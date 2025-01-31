#ifndef ENGINE_MENU_H
#define ENGINE_MENU_H
#include "ui.h"
#include "input.h"
#include <memory>

class Menu : public State {
public:
    Menu();

    explicit Menu(const std::string &exit_input);

    void init(WindowState* window_state) override;

    /**
     * Handles a down-event of keyboard or mouse.
     */
    void handle_down(SDL_Keycode key, Uint8 mouse) override;

    /**
     * Handles an up-event of keyboard or mouse.
     */
    void handle_up(SDL_Keycode key, Uint8 mouse) override;

    /**
     * Renders the full menu.
     */
    void render() override;

    /**
     * Ticks the menu, deciding if to switch state.
     */
    void tick(Uint64 delta, StateStatus &res) override;

protected:
    Components comps;

    // Set by subclasses to swap state
    StateStatus next_res;

    /**
     * Called when the Menu_exit input is recieved (Typicly Escape).
     * This function allows most menu subclasses not to override handle_down or
     * handle_up.
     */
    virtual void menu_exit();

private:
    int targeted_button = 0;

    std::unique_ptr<PressInput> exit_input;
};


#endif
