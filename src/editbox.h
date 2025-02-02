#ifndef PROCASM_EDITBOX_H
#define PROCASM_EDITBOX_H

#include "config.h"
#include "editlines.h"
#include "engine/game.h"
#include "engine/ui.h"
#include <vector>

void change_callback(TextPosition start, TextPosition end, int64_t size, void* box);

class Editbox {
public:
    Editbox(int x, int y, const WindowState &window_state);

    Editbox(Editbox&& other) = delete;
    Editbox(const Editbox& other) = delete;
    Editbox& operator=(Editbox&& other) = delete;
    Editbox& operator=(const Editbox& other) = delete;

    Editbox() = default;

    void render();

    void set_dpi_scale(double dpi);

    void tick(Uint64 passed);

    [[nodiscard]] bool is_pressed(int mouse_x, int mouse_y) const;

    void handle_keypress(SDL_Keycode key);

    void select();

    void unselect();

    const std::vector<std::string>& get_text() const;

    void input_char(char c);

    void set_text(std::string& text);

    void set_errors(std::vector<std::pair<std::string, int32_t>> msgs);
private:
    friend void change_callback(TextPosition, TextPosition, int64_t, void*);
    void change_callback(TextPosition start, TextPosition end, int64_t removed);

    TextPosition find_pos(int mouse_x, int mouse_y) const;

    void reset_cursor_animation();

    int x{}, y{};

    int64_t max_col {0};
    bool insert_mode {false};

    bool box_selected {false};

    EditLines lines {MAX_LINES, MAX_LINE_WIDTH, ::change_callback, this};

    std::vector<TextBox> boxes {};
    std::vector<TextBox> error_msg {};

    bool show_cursor {false};
    Sint64 ticks_remaining = 0;

    const WindowState* window_state {nullptr};
};


#endif // PROCASM_EDITBOX_H
