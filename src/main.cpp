#include "assembly.h"
#include "engine/log.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <engine/engine.h>
#include <engine/game.h>
#include <memory>
#include "registers.h"

class SDL_context {
public:
    SDL_context() {
        LOG_INFO("Initializing SDL");
        //SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
        SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            LOG_CRITICAL("Failed initializing SDL: %s", SDL_GetError());
            exit(1);
        }
        if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
            LOG_CRITICAL("Failed initializing SDL_image: %s", IMG_GetError());
            SDL_Quit();
            exit(1);
        }
        if (TTF_Init() < 0) {
            LOG_CRITICAL("Failed initializing SDL_ttf: %s", TTF_GetError());
            IMG_Quit();
            SDL_Quit();
            exit(1);
        }
        try {
            engine::init();
        } catch (base_exception& e) {
            LOG_CRITICAL("Failed initializing engine: %s", e.msg.c_str());
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            exit(1);
        }
        initialized = true;
    }

    ~SDL_context() {
        if (initialized) {
            initialized = false;
            LOG_INFO("Shutting down SDL libraries");
            engine::shutdown();
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
        }
    }
private:
    bool initialized = false;
};

std::unique_ptr<SDL_context> context;

int main(int argv, char* argc[]) {
    context = std::make_unique<SDL_context>();

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

    StateGame game {new GameState(), 1920, 1080, "Text box!!!"};
    try {
        game.create();
        game.run();
    } catch (base_exception& e) {
        LOG_CRITICAL("%s", e.msg.c_str());
        return 1;
    }
    return 0;
}
