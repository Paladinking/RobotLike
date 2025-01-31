#pragma once
#include "editbox.h"
#include <engine/game.h>
#include <engine/ui.h>

class GameState : public State {
public:
    GameState();

    GameState(const GameState& other) = delete;
    GameState(GameState&& other) = delete;

    void init(WindowState* state) override;

    void resume() override;

    void render() override;

    void tick(Uint64 delta, StateStatus &res) override;

    void handle_down(SDL_Keycode key, Uint8 mouse) override;

    void handle_up(SDL_Keycode key, Uint8 mouse) override;

    void handle_textinput(const SDL_TextInputEvent &e) override;

    void handle_size_change() override;

    void handle_focus_change(bool focus) override;

    void menu_change(bool visible);

    void clock_tick();
private:
    StateStatus next_state;

    void set_font_size();

    bool should_exit = false;

    bool mouse_down = false;

    Editbox box;
    double dpi_scale = 0.0;
};
