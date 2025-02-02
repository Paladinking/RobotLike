#pragma once
#include "editbox.h"
#include "language.h"
#include <engine/game.h>
#include <engine/ui.h>
#include "maze.h"
#include <vector>
#include "slime.h"

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

    void handle_user_event(SDL_UserEvent &e) override;

    void menu_change(bool visible);

    void clock_tick();
private:
    StateStatus next_state;
    Program program;

    Maze maze;

    std::vector<Slime> slimes;

    void set_font_size();

    bool should_exit = false;

    bool mouse_down = false;

    Editbox box;
    Components comps;

    std::vector<Component<TextBox>> log;
    int log_ix = 0;

    double dpi_scale = 0.0;

    Uint32 EVT_PRINT;
};
