#ifndef UI_00_H
#define UI_00_H
#include "game.h"
#include "engine.h"
#include "texture.h"
#include "log.h"
#include <SDL_ttf.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

enum class Alignment { LEFT, CENTRE, RIGHT };

class TextBox;
class Button;
class Dropdown;
class Components;

class TextBox {
public:
    TextBox() = default;

    TextBox(SDL_Rect rect, std::string text, const WindowState& ws);
    TextBox(int x, int y, int w, int h, std::string text,
            const WindowState &window_state);

    TextBox(int x, int y, int w, int h, std::string text, int font_size,
            const WindowState &window_state);

    /**
     * Sets the text of the textbox.
     */
    void set_text(const std::string &text);

    /**
     * Sets the font size of the textbox.
     */
    void set_font_size(int font_size);

    /**
     * Gets the text of the textbox.
     */
    const std::string &get_text() const;

    /**
     * Sets the position of the textbox (upper corner) to (x, y).
     */
    void set_position(int x, int y);

    /**
     * Sets the dimensions of the textbox to (w, h).
     */
    void set_dimensions(int w, int h);

    /**
     * Sets the color of the text.
     */
    void set_text_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

    /**
     * Sets horizontal alignment of the text.
     */
    void set_align(Alignment alignment);

    /**
     * Gets the color of the text;
     */
    [[nodiscard]] const SDL_Color &get_text_color() const;

    /**
     * Renders the textbox.
     */
    void render(int x_offset, int y_offset,
                const WindowState &window_state) const;

    /**
     * Initializes the button class, loading the font used for the button text.
     */
    static void init(TTF_Font *font_data);

    void set_dpi_ratio(double dpi);

protected:
    friend class Dropdown;
    friend class Button;

    int x{}, y{}, w{}, h{};

    int font_size{};

    int text_offset_x{};

    int text_offset_y{};

    std::string text;

    double dpi_ratio{};

private:
    Texture texture;

    SDL_Color color = {0, 0, 0, 0};

    Alignment alignment = Alignment::CENTRE;

    /**
     * Generates the texture containing the text of the button. Called by
     * constructor and set_text.
     */
    void generate_texture();

    static TTF_Font *font;
};

class Polygon {
public:
    Polygon() = default;

    Polygon(std::initializer_list<SDL_FPoint> points);

    void render(int x_offset, int y_offset) const;

    void set_points(std::initializer_list<SDL_FPoint> points);

    void set_border_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

private:
    SDL_Color color;

    int x_offset = 0, y_offset = 0;

    std::vector<SDL_Vertex> verticies{};
};

class Box {
public:
    Box() = default;

    Box(SDL_Rect rect, int stroke);
    Box(int x, int y, int w, int h, int stroke);
    Box(SDL_Rect rect);
    Box(int x, int y, int w, int h);

    void render(int x_offset, int y_offset) const;

    void set_border_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

private:
    uint8_t r{}, g{}, b{}, a{};

    SDL_Rect rect;
    int border_width;
    bool filled{};
};

class Button {
public:
    /**
     * Default initialization
     */
    Button() = default;

    /**
     * Constructs a button with given size and text positioned at given
     * location.
     */
    Button(SDL_Rect dims, std::string text, const WindowState& ws);
    Button(int x, int y, int w, int h, std::string text, const WindowState &ws);
    Button(int x, int y, int w, int h, std::string text, int font_size,
           const WindowState &ws);

    /**
     * Returns true if the button contains the point (mouseX, mousey).
     */
    [[nodiscard]] bool is_pressed(int mouseX, int mouseY) const;

    /**
     * Hides or shows the border of this button.
     */
    void set_border(bool border);

    /**
     * Handles press logic on a mouse event. Returns true when pressed.
     */
    bool handle_press(WindowState &ws, int x_offset, int y_offset, bool down);

    /**
     * Renders the button.
     */
    void render(int x_offset, int y_offset,
                const WindowState &window_state) const;

    void set_dpi_ratio(double ration);

    void set_text(const std::string &text);

    const std::string &get_text() const;

    void set_text_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    void enable_hover(bool enable);

    void enable_button(bool enable);

    event_t get_event() const;

    void set_event(event_t event);

private:
    bool handle_press(int x, int y, bool press);

    friend class Dropdown;

    event_t event = 0;

    TextBox text;

    bool border = true;
    bool down = false;
    bool hover_enabled = true;

    bool disabled = false;
};

class Dropdown {
public:
    Dropdown() = default;

    Dropdown(SDL_Rect dims, std::string text, const std::vector<std::string> &choices, WindowState& ws);

    Dropdown(int x, int y, int w, int h, std::string text,
             const std::vector<std::string> &choices,
             WindowState &window_state);

    // Sets the choice to default, choice 0 if no default exists.
    void clear_choice();

    // Gets the selected item index. If default returns -1.
    int get_choice() const;

    // Sets the selected item index. If < 0, same as clear_choice.
    void set_choice(int ix);

    void render(int x_offset, int y_offset,
                const WindowState &window_state) const;

    int handle_press(WindowState &window_state, int x_offset, int y_offset,
                     bool down);

    void set_text_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    void enable_hover(bool enable);

    void set_dpi_ratio(double dpi);

    void set_choices(const std::vector<std::string> &choices,
                     const WindowState &window_state);

    event_t get_event() const;

    void set_event(event_t event);

private:
    bool show_list = false;

    int max_w, max_h;

    int ix;
    event_t event = 0;

    std::string default_value;

    Button base;

    std::vector<Button> choices{};
};

template<class C>
class Component {
public:
    Component() {}

    C& operator*();

    C* operator->();
private:
    Component(Components* comps, std::size_t ix) : comps{comps}, ix(ix) {}

    friend class Components;
    Components* comps;
    int ix;
};

class Components {
    template<class C>
    friend class Component;

    struct WrapperBase {
        virtual ~WrapperBase() = default;
    };

    struct CallbackData {
        void (*callback)(int64_t, void*);
        std::unique_ptr<WrapperBase> aux;
    };

    std::vector<CallbackData> callbacks {};

public:
    Components() {
        auto cb = [](int64_t, void*) {};
        // Add callback at index 0 as null callback.
        callbacks.push_back({cb, nullptr});
    }

    void clear() {
        std::get<0>(comps).clear();
        std::get<1>(comps).clear();
        std::get<2>(comps).clear();
        std::get<3>(comps).clear();
        std::get<4>(comps).clear();
        callbacks.resize(1);
    }

    void set_window_state(WindowState* window_state) {
        this->window_state = window_state;
    }

    void enable_hover(bool enabled) {
        for (auto& btn: std::get<std::vector<Button>>(comps)) {
            btn.enable_hover(enabled);
        }
        for (auto& dropdown: std::get<std::vector<Dropdown>>(comps)) {
            dropdown.enable_hover(enabled);
        }
    }

    void set_dpi(double dpi_scale) {
        for (auto &text: std::get<2>(comps)) {
            text.set_dpi_ratio(dpi_scale);
        }
        for (auto &btn: std::get<3>(comps)) {
            btn.set_dpi_ratio(dpi_scale);
        }
        for (auto &dropdown: std::get<4>(comps)) {
            dropdown.set_dpi_ratio(dpi_scale);
        }
    }

    void render(int x_offset, int y_offset) const {
        for (auto &border: std::get<0>(comps)) {
            border.render(x_offset, y_offset);
        }
        for (auto &polygon: std::get<1>(comps)) {
            polygon.render(x_offset, y_offset);
        }
        for (auto &text: std::get<2>(comps)) {
            text.render(x_offset, y_offset, *window_state);
        }
        for (auto &btn: std::get<3>(comps)) {
            btn.render(x_offset, y_offset, *window_state);
        }
        for (auto &dropdown : std::get<4>(comps)) {
            dropdown.render(x_offset, y_offset, *window_state);
        }
    }

    void handle_press(int x_offset, int y_offset, bool press) {
        event_t ix = 0;
        int64_t val = 1;
        for (auto &btn : std::get<3>(comps)) {
            if (btn.handle_press(*window_state, x_offset, y_offset, press)) {
                ix = btn.get_event();
            }
        }
        for (auto &dropdown : std::get<4>(comps)) {
            int64_t res = dropdown.handle_press(*window_state, x_offset, y_offset, press);
            if (res >= 0) {
                ix = dropdown.get_event();
                val = dropdown.get_choice();
            }
        }
        if (ix != 0) {
            // Delay callback until after iteration, in case callback changes
            // this. Currently only allows one callback per press.
            callbacks[ix].callback(val, callbacks[ix].aux.get());
        }
    }

    template <class C>
    Component<C> add(C&& comp) {
        auto& vec = std::get<std::vector<C>>(comps);
        vec.push_back(std::move(comp));
        return {this, vec.size() - 1};
    }

    template<class C, class... Args>
    Component<C> add(C&& comp, void(*cb)(Args...), Args... args) {
        auto& vec = std::get<std::vector<C>>(comps);
        vec.push_back(std::move(comp));

        struct Wrapper : public WrapperBase {
            explicit Wrapper(Args... args, void(*cb)(Args...))
                : args{args...}, cb{cb} {}
            std::tuple<Args...> args;
            void (*cb)(Args...);
        };
        std::unique_ptr<WrapperBase> aux{new Wrapper{args..., cb}};
        auto call = [](int64_t i, void* aux) {
            Wrapper &w = *reinterpret_cast<Wrapper*>(aux);
            w.cb(std::get<Args>(w.args)...);
        };

        CallbackData data{call, std::move(aux)};
        callbacks.push_back(std::move(data));
        vec.back().set_event(callbacks.size() - 1);

        return {this, vec.size() - 1};
    }

    template<class C, class...Args>
    Component<C> add(C&& comp, void(*cb)(int64_t data, Args...), Args... args) {
        auto& vec = std::get<std::vector<C>>(comps);
        vec.push_back(std::move(comp));

        struct Wrapper : public WrapperBase {
            explicit Wrapper(Args... args, void(*cb)(int64_t data, Args...))
                : args{args...}, cb{cb} {}
            std::tuple<Args...> args;
            void (*cb)(int64_t, Args...);
        };
        std::unique_ptr<WrapperBase> aux{new Wrapper{args..., cb}};
        auto call = [](int64_t i, void* aux) {
            Wrapper &w = *reinterpret_cast<Wrapper*>(aux);
            w.cb(i, std::get<Args>(w.args)...);
        };

        CallbackData data{call, std::move(aux)};
        callbacks.push_back(std::move(data));
        vec.back().set_event(callbacks.size() - 1);

        return {this, vec.size() - 1};
    }

private:
    WindowState* window_state = nullptr;

    std::tuple<std::vector<Box>, std::vector<Polygon>, std::vector<TextBox>, std::vector<Button>,
               std::vector<Dropdown>>
        comps{};
};

template<class C>
C& Component<C>::operator*() {
    return std::get<std::vector<C>>(comps->comps)[ix];
}

template<class C>
C* Component<C>::operator->() {
    return &std::get<std::vector<C>>(comps->comps)[ix];
}

#endif
