#include "utils.h"
#include <string>
#include "enemy.h"



class Slime : public Enemy {
public:
    Slime(int32_t x, int32_t y, int32_t level);
    ~Slime();

    void tick();

    void render(float offset_x, float offset_y);

private:
    int32_t size, stun_time, wait_time;
};