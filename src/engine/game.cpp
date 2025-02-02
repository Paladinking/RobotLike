#include "game.h"
#include "style.h"
#include "log.h"
#include "engine.h"
#include <utility>

SDL_Renderer *gRenderer;
SDL_Window *gWindow;

/*
 * Base Game class
 *
 */
Game::Game(const int window_width, const int window_height, std::string title)
    : initial_width(window_width), initial_height(window_height),
      initial_title(std::move(title)) {
    if (initial_width <= 0 || initial_height <= 0)
        throw game_exception("Invalid window dimensions");
}

void Game::create() {
    if (gWindow != nullptr || gRenderer != nullptr) {
        throw game_exception("Previous game still alive!");
    }
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        throw SDL_exception("STL is not initialized!");
    }

    gWindow = SDL_CreateWindow(initial_title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, initial_width,
                               initial_height, SDL_WINDOW_HIDDEN | SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (gWindow == nullptr) {
        throw SDL_exception("Window could not be created, " +
                            std::string(SDL_GetError()));
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == nullptr) {
        SDL_DestroyWindow(gWindow);
        throw SDL_exception("Renderer could not be created, " +
                            std::string(SDL_GetError()));
    }

    SDL_RenderSetVSync(gRenderer, 1);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

    SDL_GetWindowSize(gWindow, &window_state.window_width,
                      &window_state.window_height);
    SDL_GetRendererOutputSize(gRenderer, &window_state.screen_width,
                              &window_state.screen_height);
    destroyed = false;

    window_state.keyboard_state = SDL_GetKeyboardState(nullptr);
    init();
}

void Game::run() {
    if (destroyed) {
        return;
    }
    running = true;
    Uint64 last_time = SDL_GetTicks64();

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                exit_game();
                break;
            case SDL_KEYDOWN:
                handle_keydown(e.key);
                break;
            case SDL_KEYUP:
                handle_keyup(e.key);
                break;
            case SDL_MOUSEMOTION:
                window_state.mouseX = e.motion.x;
                window_state.mouseY = e.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                handle_mousedown(e.button);
                break;
            case SDL_MOUSEBUTTONUP:
                handle_mouseup(e.button);
                break;
            case SDL_MOUSEWHEEL:
                handle_mousewheel(e.wheel);
                break;
            case SDL_TEXTINPUT:
                handle_textinput(e.text);
                break;
            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
                    e.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED) {
                    handle_size_change();
                } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    handle_focus_change(false);
                } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    handle_focus_change(true);
                }
            default:
                if (e.type >= SDL_USEREVENT) {
                    handle_user_event(e.user);
                }
            }

        }
        window_state.mouse_mask = SDL_GetMouseState(nullptr, nullptr);

        Uint64 cur_time = SDL_GetTicks64();
        this->tick(cur_time - last_time);
        if (!running)
            break;
        last_time = cur_time;

        render();
    }
    shutdown();
}

void Game::exit_game() { running = false; }

Game::~Game() {
    if (!destroyed) {
        SDL_DestroyRenderer(gRenderer);
        gRenderer = nullptr;

        SDL_DestroyWindow(gWindow);
        gWindow = nullptr;
    }
}

/*
 * State class
 *
 */
void State::init(WindowState *state) { window_state = state; }

int State::get_preferred_width() const { return -1; }

int State::get_preferred_height() const { return -1; }

/*
 * StateGame class
 *
 */

StateGame::StateGame(State *initial_state, const int w, const int h,
                     const std::string &title)
    : Game(w, h, title) {
    states.emplace(initial_state);
}

void StateGame::init() {
    update_window(states.top().get());
    states.top()->init(&window_state);
}

void StateGame::render() { 
    SDL_SetRenderDrawColor(gRenderer, UI_BACKGROUND_COLOR);
    SDL_RenderClear(gRenderer);
    states.top()->render();

    SDL_RenderPresent(gRenderer);
}

void StateGame::tick(Uint64 delta) {
    StateStatus status = {StateStatus::NONE, nullptr};
    states.top()->tick(delta, status);

    switch (status.action) {
    case StateStatus::PUSH:
        states.emplace(status.new_state);
        update_window(status.new_state);
        states.top()->init(&window_state);
        break;
    case StateStatus::SWAP:
        states.top().reset(status.new_state);
        update_window(status.new_state);
        states.top()->init(&window_state);
        break;
    case StateStatus::POP:
        states.pop();
        LOG_DEBUG("Popped state");
        if (states.empty()) {
            exit_game();
        } else {
            update_window(states.top().get());
            states.top()->resume();
        }
        break;
    case StateStatus::EXIT:
        exit_game();
        break;
    case StateStatus::NONE:
        break;
    }
}

void StateGame::update_window(const State *const state) {
    int w = state->get_preferred_width(), h = state->get_preferred_height();
    if (w == -1)
        w = window_state.screen_width;
    if (h == -1)
        h = window_state.screen_height;
    if (w != window_state.screen_width || h != window_state.screen_height) {
        SDL_SetWindowSize(gWindow, w, h);
        window_state.screen_width = w;
        window_state.screen_height = h;
    }
}

void StateGame::handle_keydown(SDL_KeyboardEvent &e) {
    states.top()->handle_down(e.keysym.sym, 0);
}

void StateGame::handle_keyup(SDL_KeyboardEvent &e) {
    states.top()->handle_up(e.keysym.sym, 0);
}

void StateGame::handle_mousedown(SDL_MouseButtonEvent &e) {
    states.top()->handle_down(SDLK_UNKNOWN, e.button);
}

void StateGame::handle_mouseup(SDL_MouseButtonEvent &e) {
    states.top()->handle_up(SDLK_UNKNOWN, e.button);
}

void StateGame::handle_mousewheel(SDL_MouseWheelEvent &e) {
    states.top()->handle_wheel(e);
}

void StateGame::handle_textinput(SDL_TextInputEvent &e) {
    states.top()->handle_textinput(e);
}

void StateGame::handle_size_change() {
    states.top()->handle_size_change();
}

void StateGame::handle_focus_change(bool focus) {
    states.top()->handle_focus_change(focus);
}

void StateGame::handle_user_event(SDL_UserEvent &e) {
    states.top()->handle_user_event(e);
}

void StateGame::shutdown() {
    if (!states.empty()) {
        states.top()->shutdown();
    }
}
