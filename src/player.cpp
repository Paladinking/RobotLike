#include "player.h"
#include "maze.h"
#include <cassert>
#include <iostream>


Player::Player(int32_t x, int32_t y, Texture *tex) : pos(x, y), direction(vec2i_from_dir(DIR_LEFT)), texture(tex) {}

void Player::tick(Maze const &map, std::vector<std::unique_ptr<Enemy>> &enemies,
                  vec2i move_vector, vec2i damage_vector) {
    pos += move_vector;
}

void Player::forward(Maze& map) {

}

void Player::rotate_left() {
    if (direction.x == 0 && direction.y == 1) {
        direction = vec2i_from_dir(DIR_RIGHT);
    } else if (direction.x == 0 && direction.y == -1) {
        direction = vec2i_from_dir(DIR_LEFT);
    } else if (direction.x == 1) {
        direction = vec2i_from_dir(DIR_UP);
    } else {
        direction = vec2i_from_dir(DIR_DOWN);
    }
}

void Player::rotate_right() {
    if (direction.x == 0 && direction.y == 1) {
        direction = vec2i_from_dir(DIR_LEFT);
    } else if (direction.x == 0 && direction.y == -1) {
        direction = vec2i_from_dir(DIR_RIGHT);
    } else if (direction.x == 1) {
        direction = vec2i_from_dir(DIR_DOWN);
    } else {
        direction = vec2i_from_dir(DIR_UP);
    }
}

void Player::move(Maze& map, int32_t dx, int32_t dy) {
    assert(std::abs(dx) <= 1 && std::abs(dy) <= 1);
    if (map.is_open(pos.x + dx, pos.y + dy)) {
        pos.x += dx;
        pos.y += dy;
    }
}

void Player::render(float offset_x, float offset_y) {
    std::cout << direction.angle() << std::endl;
    texture->render(offset_x + pos.x * TILE_SIZE - TILE_SIZE / 2, offset_y + pos.y * TILE_SIZE - TILE_SIZE / 2,
                             direction.angle() + M_PI_2, SDL_FLIP_NONE
                             );
    //texture->render_corner(offset_x + pos.x * TILE_SIZE, offset_y + pos.y * TILE_SIZE);
}
