#ifndef OVERLAY_MENU_H
#define OVERLAY_MENU_H
#include "engine/menu.h"

class OverlayMenu : public Menu {
public:
    OverlayMenu(State* parent);

    void render() override;
protected:
    State* parent;
};


#endif
