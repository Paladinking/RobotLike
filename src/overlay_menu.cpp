#include "overlay_menu.h"
#include "config.h"

OverlayMenu::OverlayMenu(State* parent) : parent{parent} {}

void OverlayMenu::render() {
    parent->render();
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0x5f);
    SDL_Rect r = {0, 0, WIDTH, HEIGHT};
    SDL_RenderFillRect(gRenderer, &r);
    Menu::render();
}
