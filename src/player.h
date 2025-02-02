#include <cstdint>
#include "utils.h"
#include "equipment.h"
#include "maze.h"
#include <vector>
#include <memory>
#include "enemy.h"
#include "engine/texture.h"



class Player {
public:
    Player(int32_t x, int32_t y, Texture* tex);
    ~Player() = default;

    void tick(Maze const& map, std::vector<std::unique_ptr<Enemy>>& enemies, vec2i move_vector, vec2i damage_vector);

    void render(float offset_x, float offset_y);

    bool add_equipment(std::shared_ptr<Equipment> new_equipment);

    bool remove_equipment(EQUIPMENT_SLOTS slot);

private:
    vec2i pos;
    vec2i direction;
    int32_t max_hp, hp;
    Texture* texture;

    std::shared_ptr<Equipment> weapon, head, body, feet;
};