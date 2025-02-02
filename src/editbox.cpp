#include "editbox.h"
#include "config.h"
#include "engine/log.h"
#include "engine/style.h"

bool valid_char(unsigned char c) { return c >= 0x20 && c <= 0xef; }

Editbox::Editbox(int x, int y, const WindowState &window_state)
    : x(x), y(y), window_state(&window_state) {}

TextPosition Editbox::find_pos(int mouse_x, int mouse_y) const {
    int row = (mouse_y - (y + BOX_TEXT_MARGIN)) / BOX_LINE_HEIGHT;
    const std::vector<std::string> &text = lines.get_lines();
    if (row < 0) {
        row = 0;
    } else if (row >= text.size()) {
        row = static_cast<int>(text.size() - 1);
    }
    int col = (mouse_x - (x + BOX_TEXT_MARGIN)) / BOX_CHAR_WIDTH;
    if (col < 0) {
        col = 0;
    } else if (col > text[row].size()) {
        col = static_cast<int>(text[row].size());
    }
    return {row, col};
}

void Editbox::input_char(char c) {
    if (!box_selected) {
        return;
    }

    if (valid_char(c)) {
        reset_cursor_animation();
        if (insert_mode && !lines.has_selection()) {
            TextPosition pos = lines.get_cursor_pos();
            if (pos.col < lines.line_size(pos.row)) {
                pos.col += 1;
                lines.move_cursor(pos, true);
            }
        }
        lines.insert_str(std::string(1, c), EditType::WRITE);
    }
}

void Editbox::tick(Uint64 passed) {
    if ((window_state->mouse_mask & SDL_BUTTON_LMASK) && box_selected) {
        TextPosition pos = find_pos(window_state->mouseX, window_state->mouseY);
        lines.move_cursor(pos, true);
    }

    ticks_remaining -= static_cast<Sint64>(passed);
    if (box_selected && ticks_remaining < 0) {
        ticks_remaining = 500;
        show_cursor = !show_cursor;
    }
}

void Editbox::select() {
    TextPosition pos = find_pos(window_state->mouseX, window_state->mouseY);
    box_selected = true;
    bool shift_pressed = window_state->keyboard_state[SDL_SCANCODE_RSHIFT] ||
                         window_state->keyboard_state[SDL_SCANCODE_LSHIFT];

    lines.move_cursor(pos, shift_pressed);
    max_col = lines.get_cursor_pos().col;

    show_cursor = true;
    ticks_remaining = 800;
}

void Editbox::unselect() {
    box_selected = false;
    show_cursor = false;
    lines.clear_action();
}

bool Editbox::is_selected() { return box_selected; }

void Editbox::reset_cursor_animation() {
    ticks_remaining = 500;
    show_cursor = true;
}

void validate_string(std::string &s) {
    while (true) {
        size_t pos = s.find("\r\n");
        if (pos == std::string::npos) {
            break;
        }
        s.erase(pos, 1);
    }

    for (int i = 0; i < s.size();) {
        if (s[i] == '\t') {
            s[i] = ' ';
            for (int j = 0; j < SPACES_PER_TAB - 1; ++j) {
                s.insert(s.begin() + i + j, ' ');
            }
            i += SPACES_PER_TAB;
        } else if (s[i] == '\r') {
            s[i] = '\n';
            ++i;
        } else if (s[i] != '\n' && !valid_char(s[i])) {
            s.erase(i, 1);
        } else {
            ++i;
        }
    }
}

void Editbox::set_text(std::string &text) {
    validate_string(text);
    lines.set_selection(
        {0, 0},
        {lines.line_count() - 1, lines.line_size(lines.line_count() - 1)},
        true);
    lines.insert_str(text);
    lines.clear_undo_stack();
}

void Editbox::set_errors(std::vector<std::pair<std::string, uint32_t>> msgs) {
    error_msg.clear();
    for (const auto &error : msgs) {
        error_msg.emplace_back(
            -8 - BOX_TEXT_MARGIN,
            BOX_TEXT_MARGIN + static_cast<int>(error.second) * BOX_LINE_HEIGHT,
            0, BOX_LINE_HEIGHT, error.first, *window_state);
        error_msg.back().set_align(Alignment::RIGHT);
        error_msg.back().set_text_color(0xf0, 0, 0, 0xff);
    }
}

enum class CharGroup { REGULAR, SYMBOL, SPACE, NEWLINE };

CharGroup get_char_group(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
        return CharGroup::REGULAR;
    }
    if (c == ' ' || c == '\t') {
        return CharGroup::SPACE;
    }
    if (c == '\n') {
        return CharGroup::NEWLINE;
    }
    return CharGroup::SYMBOL;
}

void Editbox::handle_keypress(SDL_Keycode key) {
    if (!box_selected) {
        return;
    }
    bool shift_pressed = window_state->keyboard_state[SDL_SCANCODE_RSHIFT] ||
                         window_state->keyboard_state[SDL_SCANCODE_LSHIFT];
    bool ctrl_pressed = window_state->keyboard_state[SDL_SCANCODE_LCTRL] ||
                        window_state->keyboard_state[SDL_SCANCODE_RCTRL];

    TextPosition cursor = lines.get_cursor_pos();
    if (key == SDLK_INSERT) {
        insert_mode = !insert_mode;
        LOG_DEBUG("Insert mode: %d", insert_mode);
    } else if (key == SDLK_HOME) {
        reset_cursor_animation();
        if (ctrl_pressed) {
            lines.move_cursor({0, 0}, shift_pressed);
        } else {
            lines.move_cursor({cursor.row, 0}, shift_pressed);
        }
        max_col = lines.get_cursor_pos().col;
    } else if (key == SDLK_END) {
        reset_cursor_animation();
        if (ctrl_pressed) {
            lines.move_cursor({lines.line_count() - 1,
                               lines.line_size(lines.line_count() - 1)},
                              shift_pressed);
        } else {
            lines.move_cursor({cursor.row, lines.line_size(cursor.row)},
                              shift_pressed);
        }
        max_col = lines.get_cursor_pos().col;
    } else if (key == SDLK_TAB || key == SDLK_KP_TAB) {
        reset_cursor_animation();
        lines.insert_str(std::string(SPACES_PER_TAB, ' '));
    } else if (key == 'a' && ctrl_pressed) {
        reset_cursor_animation();
        lines.set_selection({0, 0},
                            {static_cast<int>(lines.line_count() - 1),
                             lines.line_size(lines.line_count() - 1)},
                            true);
    } else if ((key == 'z' && ctrl_pressed && shift_pressed) ||
               key == 'y' && ctrl_pressed) {
        reset_cursor_animation();
        if (lines.has_selection()) {
            lines.clear_selection();
        } else {
            lines.undo_action(true);
        }
    } else if (key == 'z' && ctrl_pressed) {
        reset_cursor_animation();
        if (lines.has_selection()) {
            lines.clear_selection();
        } else {
            lines.undo_action(false);
        }
    } else if (key == 'v' && ctrl_pressed) {
        reset_cursor_animation();
        char *clip = SDL_GetClipboardText();
        std::string s{clip};
        validate_string(s);
        SDL_free(clip);
        lines.insert_str(s);
    } else if ((key == 'c' || key == 'x') && ctrl_pressed) {
        reset_cursor_animation();
        if (!lines.has_selection()) {
            return;
        }
        std::string s = lines.extract_selection();
        LOG_DEBUG("Copying '%s' to clipboard", s.c_str());
        SDL_SetClipboardText(s.c_str());
        if (key == 'x') {
            lines.insert_str("");
        }
    } else if (key == SDLK_DOWN && !ctrl_pressed) {
        reset_cursor_animation();
        if (!shift_pressed && lines.has_selection()) {
            lines.clear_selection();
        }
        lines.move_cursor({cursor.row + 1, max_col}, shift_pressed);
        if (lines.get_cursor_pos().col > max_col ||
            cursor.row == lines.line_count() - 1) {
            max_col = lines.get_cursor_pos().col;
        }
    } else if (key == SDLK_UP && !ctrl_pressed) {
        reset_cursor_animation();
        if (!shift_pressed && lines.has_selection()) {
            lines.clear_selection();
        }
        lines.move_cursor({cursor.row - 1, max_col}, shift_pressed);
        if (lines.get_cursor_pos().col > max_col || cursor.row == 0) {
            max_col = lines.get_cursor_pos().col;
        }
    } else if (key == SDLK_LEFT) {
        reset_cursor_animation();
        if (!shift_pressed && !ctrl_pressed && lines.has_selection()) {
            lines.move_cursor(lines.get_selection_start(), false);
            return;
        }
        if (!lines.move_left(cursor, 1)) {
            return;
        }
        if (ctrl_pressed) {
            CharGroup group = get_char_group(lines.char_at_pos(cursor));
            bool seen_space = false;
            do {
                CharGroup new_group = get_char_group(lines.char_at_pos(cursor));
                if (new_group == CharGroup::SPACE) {
                    seen_space = true;
                } else if (seen_space || new_group != group) {
                    lines.move_right(cursor, 1);
                    break;
                }
            } while (lines.move_left(cursor, 1));
        }
        lines.move_cursor(cursor, shift_pressed);
        max_col = lines.get_cursor_pos().col;
    } else if (key == SDLK_RIGHT) {
        reset_cursor_animation();
        if (!shift_pressed && !ctrl_pressed && lines.has_selection()) {
            lines.move_cursor(lines.get_selection_end(), false);
            return;
        }
        if (ctrl_pressed) {
            CharGroup group = get_char_group(lines.char_at_pos(cursor));
            bool seen_space = false;
            while (lines.move_right(cursor, 1)) {
                CharGroup new_group = get_char_group(lines.char_at_pos(cursor));
                if (new_group == CharGroup::SPACE) {
                    seen_space = true;
                } else if (seen_space || new_group != group) {
                    break;
                }
            }
        } else if (!lines.move_right(cursor, 1)) {
            return;
        }
        lines.move_cursor(cursor, shift_pressed);
        max_col = lines.get_cursor_pos().col;
    } else if (key == SDLK_DELETE) {
        reset_cursor_animation();
        if (!lines.has_selection()) {
            if (!lines.move_right(cursor, 1)) {
                return;
            }
            lines.set_cursor(cursor, true);
        }
        lines.insert_str("", EditType::DELETE);
    } else if (key == SDLK_BACKSPACE) {
        reset_cursor_animation();
        if (!lines.has_selection()) {
            if (!lines.move_left(cursor, 1)) {
                return;
            }
            lines.set_cursor(cursor, true);
        }
        lines.insert_str("", EditType::BACKSPACE);
    } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        reset_cursor_animation();
        lines.insert_str("\n");
    }
}

void Editbox::render() {
    SDL_SetRenderDrawColor(gRenderer, UI_BORDER_COLOR);
    SDL_Rect rect = {x, y, BOX_W, BOX_H};
    SDL_RenderFillRect(gRenderer, &rect);
    SDL_SetRenderDrawColor(gRenderer, UI_BACKGROUND_COLOR);
    rect = {rect.x + 2, rect.y + 2, rect.w - 4, rect.h - 4};
    SDL_RenderFillRect(gRenderer, &rect);

    if (lines.has_selection()) {
        SDL_SetRenderDrawColor(gRenderer, 0x50, 0x50, 0x50, 0xff);
        int start = lines.get_selection_start().col;
        for (int row = lines.get_selection_start().row;
             row < lines.get_selection_end().row; ++row) {
            int size = lines.line_size(row);
            SDL_Rect r = {x + BOX_TEXT_MARGIN + BOX_CHAR_WIDTH * start,
                          y + BOX_LINE_HEIGHT * row + BOX_TEXT_MARGIN,
                          BOX_CHAR_WIDTH * (size + 1 - start), BOX_LINE_HEIGHT};
            SDL_RenderFillRect(gRenderer, &r);
            start = 0;
        }
        int col = lines.get_selection_end().col;
        SDL_Rect r = {x + BOX_TEXT_MARGIN + BOX_CHAR_WIDTH * start,
                      static_cast<int>(
                          y + BOX_LINE_HEIGHT * lines.get_selection_end().row +
                          BOX_TEXT_MARGIN),
                      BOX_CHAR_WIDTH * (col - start), BOX_LINE_HEIGHT};
        SDL_RenderFillRect(gRenderer, &r);
    }

    for (auto &box : boxes) {
        box.render(x + BOX_TEXT_MARGIN, y + BOX_TEXT_MARGIN, *window_state);
    }
    for (auto &box : error_msg) {
        box.render(x, y, *window_state);
    }

    if (show_cursor) {
        TextPosition cursor_pos = lines.get_cursor_pos();
        SDL_SetRenderDrawColor(gRenderer, 0xf0, 0xf0, 0xf0, 0xff);
        if (!insert_mode) {
            SDL_RenderDrawLine(
                gRenderer,
                x + BOX_TEXT_MARGIN + BOX_CHAR_WIDTH * cursor_pos.col,
                y + BOX_LINE_HEIGHT * cursor_pos.row + BOX_TEXT_MARGIN,
                x + BOX_TEXT_MARGIN + BOX_CHAR_WIDTH * cursor_pos.col,
                y + BOX_LINE_HEIGHT * (cursor_pos.row + 1) + BOX_TEXT_MARGIN);
        } else {
            SDL_RenderDrawLine(
                gRenderer,
                x + BOX_TEXT_MARGIN + BOX_CHAR_WIDTH * cursor_pos.col,
                y + BOX_LINE_HEIGHT * (cursor_pos.row + 1) + BOX_TEXT_MARGIN,
                x + BOX_TEXT_MARGIN + BOX_CHAR_WIDTH * (cursor_pos.col + 1),
                y + BOX_LINE_HEIGHT * (cursor_pos.row + 1) + BOX_TEXT_MARGIN);
        }
    }
}
void Editbox::set_dpi_scale(double dpi) {
    for (auto &line : boxes) {
        line.set_dpi_ratio(dpi);
    }
    for (auto &box : error_msg) {
        box.set_dpi_ratio(dpi);
    }
}
bool Editbox::is_pressed(int mouse_x, int mouse_y) const {
    return mouse_x > x && mouse_x < x + BOX_W && mouse_y > y &&
           mouse_y < y + BOX_H;
}
void Editbox::change_callback(TextPosition start, TextPosition end,
                              int64_t removed) {
    max_col = lines.get_cursor_pos().col;
    if (boxes.size() == lines.line_count()) {
        for (int i = start.row; i <= end.row; ++i) {
            boxes[i].set_text(lines.get_lines()[i]);
        }
        return;
    }
    if (boxes.size() < lines.line_count()) {
        for (int i = boxes.size(); i < lines.line_count(); ++i) {
            boxes.emplace_back(0, 0 + BOX_LINE_HEIGHT * i,
                               BOX_W - 2 * BOX_TEXT_MARGIN, BOX_LINE_HEIGHT,
                               "", *window_state);
            boxes.back().set_align(Alignment::LEFT);
        }
    } else if (boxes.size() > lines.line_count()) {
        boxes.resize(lines.line_count());
    }
    for (int64_t i = start.row; i < lines.line_count(); ++i) {
        if (lines.get_lines()[i] != boxes[i].get_text()) {
            boxes[i].set_text(lines.get_lines()[i]);
        }
    }
}

void change_callback(TextPosition start, TextPosition end, int64_t removed,
                     void *aux) {
    static_cast<Editbox *>(aux)->change_callback(start, end, removed);
}

const std::vector<std::string> &Editbox::get_text() const {
    return lines.get_lines();
}
