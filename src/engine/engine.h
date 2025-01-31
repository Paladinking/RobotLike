#ifndef ENGINE_00_H
#define ENGINE_00_H
#include <SDL.h>
#include <SDL_ttf.h>
#include "events.h"

// Global renderer variable
extern SDL_Renderer *gRenderer;
// Global window variable
extern SDL_Window *gWindow;

extern TTF_Font *gFont;

/**
 * Struct used for putting an SDL_Surface in a smart-pointer.
 */
struct SurfaceDeleter {
    void operator()(SDL_Surface *s) { SDL_FreeSurface(s); }
};

namespace engine {
    void init();

    void shutdown();

    /**
    * Returns a random number between min (inclusive) and max (exclusive).
    */
    int random(int min, int max);
}; // namespace engine

#endif
