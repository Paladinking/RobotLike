#include "slime.h"
#include "engine/engine.h"
#include "maze.h"



Slime::Slime(int32_t x, int32_t y, int32_t level) : Enemy(x, y, level*3, level, level, 1, FACTIONS::CREATURES, "Slime", DIST_TYPES::MANHATTAN, DIST_TYPES::MANHATTAN), size(level * 10), stun_time(0), wait_time(0) {

}


Slime::~Slime() {}


void Slime::tick() {
    if (stun_time > 0) {
        --stun_time;
        return;
    } else if (wait_time > 0) {
        --wait_time;
        return;
    }
}

void Slime::render(float offset_x, float offset_y) {
    SDL_Color color { 0x0, 0xff, 0x0, 0xff };
    SDL_FRect rect = {offset_x + (pos.x * TILE_SIZE),
                        offset_y + (pos.y * TILE_SIZE),
                        TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b,
                            color.a);
    SDL_RenderFillRect(gRenderer, &rect);
}
