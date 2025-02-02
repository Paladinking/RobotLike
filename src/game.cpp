#include "game.h"
#include "engine/log.h"
#include "config.h"
#include "parser.h"

GameState::GameState() : State()  {}

void GameState::set_font_size() {
    double new_dpi_scale = std::min(static_cast<double>(window_state->window_width) /
                                              window_state->screen_width,
                                          static_cast<double>(window_state->window_height) /
                                              window_state->screen_height);
    if (new_dpi_scale != dpi_scale) {
        return;
    }
    LOG_DEBUG("Recalculating font size");
    dpi_scale = new_dpi_scale;
    int hpdi = 72, vdpi = 72;
    TTF_SetFontSizeDPI(gFont, 20, hpdi, vdpi);
    return;
}

void GameState::handle_size_change() {
    SDL_RenderGetLogicalSize(gRenderer, &window_state->screen_width,
                             &window_state->screen_height);
    SDL_GetRendererOutputSize(gRenderer, &window_state->window_width,
                              &window_state->window_height);
    set_font_size();
}

void GameState::init(WindowState *window_state) {
    SDL_ShowWindow(gWindow);
    SDL_RenderSetVSync(gRenderer, 1);
    SDL_RenderSetLogicalSize(gRenderer, WIDTH, HEIGHT);
    State::init(window_state);

    SDL_RenderGetLogicalSize(gRenderer, &window_state->screen_width,
                             &window_state->screen_height);
    SDL_GetRendererOutputSize(gRenderer, &window_state->window_width,
                              &window_state->window_height);

    LOG_INFO("Physical size: %d, %d\n", window_state->window_width, window_state->window_height);
    LOG_INFO("Logical size: %d, %d\n", window_state->screen_width, window_state->screen_height);

    box.~Editbox();
    new (&box)Editbox{BOX_X, BOX_Y, *window_state };

    next_state.action = StateStatus::NONE;

    set_font_size();

    SDL_RWops* file = SDL_RWFromFile("program.txt", "r");
    if (file != nullptr) {
        Sint64 size = SDL_RWsize(file);
        char *data = new char[size];
        if (SDL_RWread(file, data, 1, size) == size) {
            std::string str{data, static_cast<std::size_t>(size)};
            if (str.back() == '\n') {
                str.pop_back();
            }
            box.set_text(str);
        }
        delete[] data;
        SDL_RWclose(file);
    }
    EVT_PRINT = SDL_RegisterEvents(1);
    program.set_events(EVT_PRINT);

    comps.set_window_state(window_state);
    int log_size = 8;
    int log_w = 500;
    int log_h = 2 * BOX_TEXT_MARGIN + log_size * BOX_LINE_HEIGHT;
    int log_x = WIDTH / 2 - log_w / 2;
    int log_y = HEIGHT - log_h;
    for (int i = 0; i < log_size; ++i) {
        log.push_back(comps.add(TextBox(log_x + BOX_TEXT_MARGIN,
                                        log_y + BOX_LINE_HEIGHT * i + BOX_TEXT_MARGIN,
                                        log_w - 2 * BOX_TEXT_MARGIN, BOX_LINE_HEIGHT,
                                        "", *window_state)));
        log.back()->set_align(Alignment::LEFT);
    }
    comps.add(Box(log_x, log_y, log_w, log_h, 4));

}

void GameState::render() {
    box.render();
    maze.render(0, 0);
    comps.render(0, 0);
}
void GameState::tick(const Uint64 delta, StateStatus &res) {
    res = next_state;
    if (next_state.will_leave()) {
        LOG_DEBUG("Saving...");
        SDL_RWops* file = SDL_RWFromFile("program.txt", "w");
        if (file != nullptr) {
            const auto& lines = box.get_text();
            for (const std::string& s : lines) {
                SDL_RWwrite(file, s.c_str(), 1, s.size());
                SDL_RWwrite(file, "\n", 1, 1);
            }
            SDL_RWclose(file);
        }
        return;
    }

    box.tick(delta);
}

void GameState::handle_up(SDL_Keycode key, Uint8 mouse) {
    if (mouse == SDL_BUTTON_LEFT) {
        if (!box.is_pressed(window_state->mouseX, window_state->mouseY) && mouse_down) {
            box.unselect();
            SDL_StopTextInput();
        }
        mouse_down = false;
    }
}

void GameState::handle_down(const SDL_Keycode key, const Uint8 mouse) {
    if (key == SDLK_ESCAPE) {
        next_state.action = StateStatus::POP;
    } else if (mouse == SDL_BUTTON_LEFT) {
        if (box.is_pressed(window_state->mouseX, window_state->mouseY)) {
            SDL_StartTextInput();
            box.select();
        } else {
            mouse_down = true;
            box.unselect();
            SDL_StopTextInput();
            const auto& lines = box.get_text();
            Parser p{};
            box.set_errors({});
            if (!p.parse_lines(lines)) {
                box.set_errors(p.errors);
            } else {
                log_ix = 0;
                for(auto& b: log) {
                    b->set_text("");
                }
                program.load_program(std::move(p.all_statements),
                                  std::move(p.all_expressions),
                                  std::move(p.all_functions),
                                  p.entry);
                p.entry = nullptr;
                p.all_statements.clear();
                p.all_expressions.clear();
                p.all_functions.clear();
                program.start();
            }
            

        }
    } else {
        box.handle_keypress(key);
    }
}

void GameState::resume() {

}

void GameState::handle_textinput(const SDL_TextInputEvent &e) {
    if (e.text[0] == '\0' || e.text[1] != '\0') {
        LOG_DEBUG("Invalid input received");
        return;
    }
    char c = e.text[0];
    box.input_char(c);
}

void GameState::handle_user_event(SDL_UserEvent &e) {
    std::cout << "User event" << std::endl;
    if (e.type == EVT_PRINT) {
        auto* s = static_cast<std::string*>(e.data1);
        if (log_ix == log.size()) {
            for (int i = 0; i < log.size() - 1; ++i) {
                log[i]->set_text(log[i + 1]->get_text());
            }
        } else {
            ++log_ix;
        }
        log[log_ix - 1]->set_text(*s);
        for (int i = 1; i< log.size(); ++i) {

        }
        std::cout << *s << std::flush;
        delete s;
    }
}

void GameState::handle_focus_change(bool focus) {
    if (!focus) {
        box.unselect();
        mouse_down = false;
        SDL_StopTextInput();
    }
}

