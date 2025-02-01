#include "utils.h"
#include <string>



enum ENEMY_STATE {
    IDLE,
    COMBAT,
};


class Enemy {
public:
    Enemy(int32_t x, int32_t y, int32_t max_hp, int32_t damage, int32_t range, FACTIONS faction, std::string name, DIST_TYPES move_dist_type, DIST_TYPES attack_dist_type) : pos(vec2i{x, y}), max_hp(max_hp), hp(max_hp), damage(damage), range(range), faction(faction), move_dist_type(move_dist_type), attack_dist_type(attack_dist_type) {}
    virtual ~Enemy() = default;

    virtual void tick() = 0;

    virtual void render(float offset_x, float offset_y) = 0;



protected:
    vec2i pos;
    int32_t max_hp;
    int32_t hp;
    int32_t damage;
    int32_t range;
    FACTIONS const faction;
    std::string name;
    DIST_TYPES const move_dist_type;
    DIST_TYPES const attack_dist_type;
};