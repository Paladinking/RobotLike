#include "game.h"
#include "engine/log.h"
#include "config.h"
#include "string.h"
#include <array>
#include "slime.h"
#include "utils.h"

GameState::GameState() : State()  { enemies.reserve(32); }

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

    // constexpr int32_t tilesize = 10;
    textures.reserve(14);
    Texture* tex = new Texture{};
    tex->load_from_file("../../assets/Tile.png", TILE_SIZE, TILE_SIZE);
    textures.emplace_back(tex);
    maze.set_texture(textures[0].get());
    tex = new Texture{};
    tex->load_from_file("../../assets/Robot.png", TILE_SIZE, TILE_SIZE);
    textures.emplace_back(tex);

    player.reset(new Player{ 0, 0, textures[1].get() });

    for (int32_t i = 0; i < 5; i++) {
        auto slime = new Slime{ engine::random(0, 20), engine::random(0, 20), i };
        enemies.emplace_back(slime);
    }
    
}

void GameState::render() {
    box.render();
    maze.render(0, 0);
    for (size_t i{0}; i < enemies.size(); ++i) {
        enemies[i]->render(0, 0);
    }
    player->render(0, 0);
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

    player.get()->tick(maze, enemies, player_mov, vec2i{});

    box.tick(delta);
}

void GameState::handle_up(SDL_Keycode key, Uint8 mouse) {
    if (mouse == SDL_BUTTON_LEFT) {
        if (!box.is_pressed(window_state->mouseX, window_state->mouseY) && mouse_down) {
            box.unselect();
            SDL_StopTextInput();
        }
        mouse_down = false;
    } else {
        switch (key) {
            case SDLK_w:
                if (player_mov == vec2i_from_dir(DIR_UP)) player_mov = vec2i{0, 0};
                break;
            case SDLK_d:
                if (player_mov == vec2i_from_dir(DIR_RIGHT)) player_mov = vec2i{0, 0};
                break;
            case SDLK_s:
                if (player_mov == vec2i_from_dir(DIR_DOWN)) player_mov = vec2i{0, 0};
                break;
            case SDLK_a:
                if (player_mov == vec2i_from_dir(DIR_LEFT)) player_mov = vec2i{0, 0};
                break;
            default:
                break;
        }
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
        }
    } else if (box.is_selected()) {
        box.handle_keypress(key);
    } else {
        switch (key) {
            case SDLK_w:
                player_mov = vec2i_from_dir(DIR_UP);
                break;
            case SDLK_d:
                player_mov = vec2i_from_dir(DIR_RIGHT);
                break;
            case SDLK_s:
                player_mov = vec2i_from_dir(DIR_DOWN);
                break;
            case SDLK_a:
                player_mov = vec2i_from_dir(DIR_LEFT);
                break;
            default:
                break;
        }
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

void GameState::handle_focus_change(bool focus) {
    if (!focus) {
        box.unselect();
        mouse_down = false;
        SDL_StopTextInput();
    }
}

