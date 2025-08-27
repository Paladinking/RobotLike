#include "ui.h"
#include "log.h"
#include "style.h"
#include <utility>

TTF_Font *TextBox::font;

void TextBox::init(TTF_Font *font_data) {
    TextBox::font = font_data;
    if (TextBox::font == nullptr) {
        throw game_exception(std::string(SDL_GetError()));
    }
}

TextBox::TextBox(SDL_Rect rect, std::string text, const WindowState &ws)
    : TextBox(rect.x, rect.y, rect.w, rect.h, std::move(text), ws) {}

TextBox::TextBox(int x, int y, int w, int h, std::string text,
                 const WindowState &window_state)
    : TextBox(x, y, w, h, text, 20, window_state) {}

TextBox::TextBox(int x, int y, int w, int h, std::string text, int font_size,
                 const WindowState &window_state)
    : x(x), y(y), w(w), h(h), text(std::move(text)), font_size(font_size),
      dpi_ratio(std::min(static_cast<double>(window_state.window_width) /
                             window_state.screen_width,
                         static_cast<double>(window_state.window_height) /
                             window_state.screen_height)),
      texture((SDL_Texture *)nullptr, 0, 0) {
    set_text_color(UI_TEXT_COLOR);
}

void TextBox::generate_texture() {
    if (text.empty()) {
        texture.free();
        return;
    }

    SDL_Surface *text_surface =
        TTF_RenderText_Blended_Wrapped(font, text.c_str(), 
                                       text.length(),
                                       color, w);
    if (text_surface == nullptr) {
        throw image_load_exception(std::string(SDL_GetError()));
    }
    int width = text_surface->w;
    int height = text_surface->h;
    SDL_Texture *text_texture =
        SDL_CreateTextureFromSurface(gRenderer, text_surface);
    SDL_DestroySurface(text_surface);

    if (text_texture == nullptr) {
        throw image_load_exception(std::string(SDL_GetError()));
    }
    texture = Texture(text_texture, width, height);

    TTF_GetStringSize(font, text.c_str(), text.length(), 
                      &width, &height);
    if (alignment == Alignment::LEFT) {
        text_offset_x = 0;
    } else if (alignment == Alignment::CENTRE) {
        text_offset_x = static_cast<int>((w - width / dpi_ratio) / 2);
    } else {
        text_offset_x = static_cast<int>(w - width / dpi_ratio);
    }
    text_offset_y = static_cast<int>((h - height / dpi_ratio) / 2);
}

void TextBox::set_dpi_ratio(double dpi) {
    dpi_ratio = dpi;
    generate_texture();
}

void TextBox::set_position(const int new_x, const int new_y) {
    x = new_x;
    y = new_y;
}

void TextBox::set_text(const std::string &new_text) {
    text = new_text;
    generate_texture();
}

void TextBox::set_align(Alignment align) {
    alignment = align;
    if (align == Alignment::LEFT) {
        text_offset_x = 0;
    } else if (align == Alignment::CENTRE) {
        int width, height;
        TTF_GetStringSize(font, text.c_str(), text.length(),
                          &width, &height);
        text_offset_x = static_cast<int>((w - width / dpi_ratio) / 2);
    } else {
        int width, height;
        TTF_GetStringSize(font, text.c_str(), text.length(),
                          &width, &height);
        text_offset_x = static_cast<int>(w - width / dpi_ratio);
    }
}

void TextBox::set_text_color(const Uint8 r, const Uint8 g, const Uint8 b,
                             const Uint8 a) {
    color = {r, g, b, a};
    generate_texture();
}

const SDL_Color &TextBox::get_text_color() const { return color; }

void TextBox::set_font_size(const int new_font_size) {
    font_size = new_font_size;
    generate_texture();
}

const std::string &TextBox::get_text() const { return text; }

void TextBox::set_dimensions(const int new_w, const int new_h) {
    text_offset_x = (new_w - w) / 2 + text_offset_x;
    text_offset_y = (new_h - h) / 2 + text_offset_y;
    w = new_w;
    h = new_h;
}

void TextBox::render(const int x_offset, const int y_offset,
                     const WindowState &window_state) const {
    if (text.empty()) {
        return;
    }
    // When rendering text, set logical size to the size of the window to allow
    // better quality text. Because of this, need to manually adjust for DPI.
    /*SDL_SetRenderLogicalPresentation(gRenderer, 
            dpi_ratio * window_state.screen_width,
            dpi_ratio * window_state.screen_height,
            SDL_LOGICAL_PRESENTATION_DISABLED);*/
    texture.render_corner(
        static_cast<int>((x_offset + x + text_offset_x)),
        static_cast<int>((y_offset + y + text_offset_y)));
    /*SDL_SetRenderLogicalPresentation(gRenderer, 
            window_state.screen_width,
            window_state.screen_height,
            SDL_LOGICAL_PRESENTATION_LETTERBOX);*/
}

Polygon::Polygon(std::initializer_list<SDL_FPoint> points) {
    verticies.reserve(points.size());
    SDL_Color c = {UI_BORDER_COLOR};
    color = {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f};
    for (auto p : points) {
        SDL_Vertex ver{p, {color.r, color.g, color.b, color.a}, {0.0f, 0.0f}};
        verticies.push_back(ver);
    }
}

void Polygon::render(int x_offset, int y_offset) const {
    auto* me = const_cast<Polygon*>(this);
    if (x_offset != me->x_offset) {
        for (auto& v: me->verticies) {
            v.position.x = v.position.x - me->x_offset + x_offset;
        }
        me->x_offset = x_offset;
    }
    if (x_offset != me->y_offset) {
        for (auto& v: me->verticies) {
            v.position.y = v.position.y - me->y_offset + y_offset;
        }
        me->y_offset = y_offset;
    }
    SDL_RenderGeometry(gRenderer, nullptr, verticies.data(),
                       verticies.size(), nullptr, 0);
}

void Polygon::set_points(std::initializer_list<SDL_FPoint> points) {
    verticies.clear();
    for (auto p : points) {
        SDL_Vertex v = {{p.x + x_offset, p.y + y_offset}, color, {0.0f, 0.0f}};
        verticies.push_back(v);
    }
}

void Polygon::set_border_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    color = {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    for (auto& v: verticies) {
        v.color = color;
    }
}

Box::Box(SDL_Rect rect, int stroke)
    : Box(rect.x, rect.y, rect.w, rect.h, stroke) {}

Box::Box(int x, int y, int w, int h, int stroke)
    : rect{x, y, w, h}, border_width{stroke}, filled{false} {
    set_border_color(UI_BORDER_COLOR);
}

Box::Box(SDL_Rect rect) : Box(rect.x, rect.y, rect.w, rect.h) {}

Box::Box(int x, int y, int w, int h) : rect{x, y, w, h}, border_width{0}, filled{true} {
    set_border_color(UI_BORDER_COLOR);
}

void Box::render(int x_offset, int y_offset) const {
    SDL_SetRenderDrawColor(gRenderer, r, g, b, a);
    SDL_FRect r = {(float)(rect.x + x_offset), (float)(rect.y + y_offset),
                   (float)rect.w, (float)rect.h};
    if (filled) {
        SDL_RenderFillRect(gRenderer, &r);
        return;
    }
    SDL_RenderFillRect(gRenderer, &r);
    SDL_SetRenderDrawColor(gRenderer, UI_BACKGROUND_COLOR);
    r = {(float)(r.x + border_width), (float)(r.y + border_width),
                  (float)(r.w - 2 * border_width),
                  (float)(r.h - 2 * border_width)};
    SDL_RenderFillRect(gRenderer, &r);
}

void Box::set_border_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

Button::Button(SDL_Rect dims, std::string text, const WindowState &ws)
    : Button(dims.x, dims.y, dims.w, dims.h, std::move(text), ws) {}

Button::Button(int x, int y, int w, int h, std::string text, const WindowState &ws)
    : Button(x, y, w, h, std::move(text), 20, ws) {}

Button::Button(int x, int y, int w, int h, std::string text, int fs,
               const WindowState &ws)
    : text(x, y, w, h, std::move(text), fs, ws), event{0} {
}

bool Button::is_pressed(int mouseX, int mouseY) const {
    return mouseX >= text.x && mouseX < text.x + text.w && mouseY >= text.y &&
           mouseY < text.y + text.h;
}

void Button::set_border(const bool new_border) { border = new_border; }

bool Button::handle_press(int x, int y, bool press) {
    if (disabled) {
        return false;
    }
    if (!is_pressed(x, y)) {
        down = false;
    } else if (press) {
        down = true;
    } else if (down) {
        down = false;
        return true;
    } else {
        down = false;
    }
    return false;
}

bool Button::handle_press(WindowState &ws, int x_offset, int y_offset,
                          bool press) {
    int mouseX = ws.mouseX - x_offset;
    int mouseY = ws.mouseY - y_offset;
    if (handle_press(mouseX, mouseY, press)) {
        return true;
    }
    return false;
}

const std::string &Button::get_text() const { return text.get_text(); }

void Button::set_text(const std::string &str) { text.set_text(str); }

void Button::set_text_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    text.set_text_color(r, g, b, a);
}

void Button::enable_hover(bool enable) {
    hover_enabled = enable;
}

void Button::enable_button(bool enable) {
    disabled = !enable;
    down = false;
}

event_t Button::get_event() const { return event; }

void Button::set_event(event_t new_event) {
    event = new_event;
}

void Button::set_dpi_ratio(double ratio) { text.set_dpi_ratio(ratio); }

void Button::render(const int x_offset, const int y_offset,
                    const WindowState &window_state) const {
    bool hover = hover_enabled && is_pressed(window_state.mouseX - x_offset,
                                             window_state.mouseY - y_offset);
    SDL_FRect r = {(float)(text.x + x_offset),
                   (float)(text.y + y_offset), (float)text.w, (float)text.h};
    if (border) {
        SDL_SetRenderDrawColor(gRenderer, UI_BORDER_COLOR);
        SDL_RenderFillRect(gRenderer, &r);
        r.x += 2;
        r.y += 2;
        r.w -= 4;
        r.h -= 4;
    }
    if (disabled) {
        SDL_SetRenderDrawColor(gRenderer, UI_BUTTON_HOVER_COLOR);
    } else if (down) {
        SDL_SetRenderDrawColor(gRenderer, UI_BUTTON_PRESSED_COLOR);
    } else if (hover) {
        SDL_SetRenderDrawColor(gRenderer, UI_BUTTON_HOVER_COLOR);
    } else {
        SDL_SetRenderDrawColor(gRenderer, UI_BUTTON_COLOR);
    }
    SDL_RenderFillRect(gRenderer, &r);

    text.render(x_offset, y_offset, window_state);
}

Dropdown::Dropdown(SDL_Rect dims, std::string text,
                   const std::vector<std::string> &choices,
                   WindowState &window_state)
    : Dropdown(dims.x, dims.y, dims.w, dims.h, std::move(text), choices,
               window_state) {}

Dropdown::Dropdown(int x, int y, int w, int h, std::string text,
                   const std::vector<std::string> &choices,
                   WindowState &window_state)
    : base(x, y, w, h, text, window_state), default_value(text), ix{-1}, event{0} {
    set_choices(choices, window_state);
}

void Dropdown::render(int x_offset, int y_offset,
                      const WindowState &window_state) const {
    base.render(x_offset, y_offset, window_state);
    float x_base = base.text.x + x_offset + base.text.w - 20.0f;
    float y_base = base.text.y + y_offset + base.text.h / 2.0f - 4;
    if (show_list) {
        for (auto &btn : choices) {
            btn.render(x_offset, y_offset, window_state);
        }
        SDL_FRect r = {(float)(x_offset + base.text.x),
                      (float)(y_offset + base.text.y + base.text.h),
                      (float)(max_w + 8),
                      (float)(choices.size()) * (max_h + 16)};
        SDL_SetRenderDrawColor(gRenderer, UI_BORDER_COLOR);
        SDL_RenderRect(gRenderer, &r);
        SDL_Vertex verticies[3] = {
            {{x_base, y_base + 10}, {UI_TEXT_COLOR}, {0.0f, 0.0f}},
            {{x_base + 10, y_base + 10}, {UI_TEXT_COLOR}, {0.0f, 0.0f}},
            {{x_base + 5, y_base}, {UI_TEXT_COLOR}, {0.0f, 0.0f}}};
        SDL_RenderGeometry(gRenderer, nullptr, verticies, 3, nullptr, 0);
    } else {
        SDL_Vertex verticies[3] = {
            {{x_base, y_base}, {UI_TEXT_COLOR}, {0.0f, 0.0f}},
            {{x_base + 10, y_base}, {UI_TEXT_COLOR}, {0.0f, 0.0f}},
            {{x_base + 5, y_base + 10}, {UI_TEXT_COLOR}, {0.0f, 0.0f}}};
        SDL_RenderGeometry(gRenderer, nullptr, verticies, 3, nullptr, 0);
    }
}

void Dropdown::enable_hover(bool enable) {
    base.enable_hover(enable);
    for (auto& btn : choices) {
        btn.enable_hover(enable);
    }
}

void Dropdown::set_choices(const std::vector<std::string> &choices,
                           const WindowState &window_state) {
    this->choices.clear();
    max_w = base.text.w - 8, max_h = 0;
    int tw, th;
    const std::string base_str = " -";
    if (default_value != "") {
        TTF_GetStringSize(gFont, base_str.c_str(), base_str.length(),
                          &tw, &th);
        if (tw > max_w) {
            max_w = tw;
        }
        if (th > max_h) {
            max_h = th;
        }
    }
    for (int i = 0; i < choices.size(); ++i) {
        const std::string str = " " + choices[i];
        TTF_GetStringSize(gFont, str.c_str(), str.length(), &tw, &th);
        if (tw > max_w) {
            max_w = tw;
        }
        if (th > max_h) {
            max_h = th;
        }
    }
    SDL_Rect r;
    int ix_offset = 0;
    if (default_value != "") {
        ix_offset = 1;
        r = {base.text.x, base.text.y + base.text.h, max_w + 8,
                  max_h + 16};
        this->choices.push_back(Button(r, base_str, window_state));
        this->choices[0].text.set_align(Alignment::LEFT);
        this->choices[0].set_border(false);
    }
    for (int i = 0; i < choices.size(); ++i) {
        const std::string &str = choices[i];
        r = {base.text.x, base.text.y + base.text.h + (max_h + 16) * (i + ix_offset),
             max_w + 8, max_h + 16};
        this->choices.push_back(
            Button(r, " " + choices[i], window_state));
        this->choices.back().text.set_align(Alignment::LEFT);
        this->choices.back().set_border(false);
    }
    this->clear_choice();
}

event_t Dropdown::get_event() const { return event; }

void Dropdown::set_event(event_t new_event) {
    event = new_event;
}

void Dropdown::set_dpi_ratio(double dpi) {
    base.set_dpi_ratio(dpi);
    for (auto &btn : choices) {
        btn.set_dpi_ratio(dpi);
    }
}

int Dropdown::handle_press(WindowState &window_state, int x_offset,
                           int y_offset, bool press) {
    int mouseX = window_state.mouseX - x_offset;
    int mouseY = window_state.mouseY - y_offset;
    if (base.handle_press(mouseX, mouseY, press)) {
        show_list = !show_list;
    }
    if (show_list) {
        int pressed = -1;
        for (int i = 0; i < choices.size(); ++i) {
            auto &btn = choices[i];
            if (btn.handle_press(mouseX, mouseY, press)) {
                pressed = i;
                show_list = false;
            }
        }
        if (pressed != -1) {
            ix = pressed - (default_value == "" ? 0 : 1);
            if (pressed == 0 && default_value != "") {
                base.set_text(default_value);
            } else {
                base.set_text(choices[pressed].get_text().substr(1));
            }
        }
        return pressed;
    }
    return -1;
}

void Dropdown::clear_choice() {
    if (default_value != "") {
        ix = -1;
        base.set_text(default_value);
    } else {
        ix = 0;
        base.set_text(choices[0].get_text().substr(1));
    }
}

int Dropdown::get_choice() const { return ix; }

void Dropdown::set_choice(int new_ix) {
    if (new_ix < 0) {
        clear_choice();
        return;
    }
    ix = new_ix;
    if (default_value == "") {
        base.set_text(choices[ix].get_text().substr(1));
    } else {
        base.set_text(choices[ix + 1].get_text().substr(1));
    }
}

void Dropdown::set_text_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {

    base.text.set_text_color(r, g, b, a);
    for (auto &btn : choices) {
        btn.text.set_text_color(r, g, b, a);
    }
}
