#include "menu.h"

Menu::Menu() : State() {
    // Since get_press_input can throw exceptions...
    exit_input = std::unique_ptr<PressInput>(new KeyPressInput(SDLK_ESCAPE));
}

Menu::Menu(const std::string &exit_input) : State() {
    this->exit_input = get_press_input(exit_input, "Escape");
}

void Menu::init(WindowState* window_state) {
    State::init(window_state);
    comps.set_window_state(window_state);
}

void Menu::handle_down(const SDL_Keycode key, const Uint8 mouse) {
    if (exit_input->is_targeted(key, mouse)) {
        menu_exit();
        return;
    }
    if (mouse == SDL_BUTTON_LEFT) {
        comps.handle_press(0, 0, true);
    }
}

void Menu::handle_up(const SDL_Keycode key, const Uint8 mouse) {
    if (mouse == SDL_BUTTON_LEFT) {
        comps.handle_press(0, 0, false);
    }
}

void Menu::render() {
    comps.render(0, 0);
}

void Menu::tick(const Uint64 delta, StateStatus &res) {
    res = next_res;
    next_res.action = StateStatus::NONE;
    next_res.new_state = nullptr;
}

void Menu::menu_exit() { next_res.action = StateStatus::POP; }
