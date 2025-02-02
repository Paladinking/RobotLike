#pragma once

constexpr int MAZE_HEIGHT = 30;
constexpr int MAZE_WIDTH = 30;
constexpr float TILE_SIZE = 32.0f;

#include <array>
#include <vector>
#include "engine/texture.h"

enum Direction : int { LEFT = 0, UP = 1, RIGHT = 2, DOWN = 3 };

class Maze {
private:
    enum TileType {VOID, OPEN, WALL, PATH, PLAYER_START, ENEMY};

    std::vector<std::array<TileType, MAZE_HEIGHT>> map;

    void generate_maze();

    Texture* texture_tile;

public:
    std::pair<int32_t, int32_t> start, goal;

    bool is_open(int32_t x, int32_t y) {
        if (x < 0 || x >= map.size()) {
            return false;
        }
        if (y < 0 || y >= map[0].size()) {
            return false;
        }
        return map[x][y] == OPEN || map[x][y] == PLAYER_START || map[x][y] == PATH;
    }

    Maze();

    void set_texture(Texture* texture);

    void render(float offet_x, float offset_y);
};
