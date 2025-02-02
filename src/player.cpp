#include "player.h"
#include "maze.h"



Player::Player(int32_t x, int32_t y, Texture *tex) : pos(x, y), direction(vec2i_from_dir(DIR_UP)), texture(tex) {}

void Player::tick(Maze const &map, std::vector<std::unique_ptr<Enemy>> &enemies,
                  vec2i move_vector, vec2i damage_vector) {
                    pos += move_vector;
                  }

void Player::render(float offset_x, float offset_y) {
    texture->render_corner(offset_x + pos.x * TILE_SIZE, offset_y + pos.y * TILE_SIZE);
}
