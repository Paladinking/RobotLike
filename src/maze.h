#pragma once

constexpr int MAZE_HEIGHT = 30;
constexpr int MAZE_WIDTH = 30;
constexpr float TILE_SIZE = 10.0f;

#include <array>
#include <vector>

enum Direction : int { LEFT = 0, UP = 1, RIGHT = 2, DOWN = 3 };

class Maze {
private:
    enum TileType {VOID, OPEN, WALL, PATH, PLAYER_START, ENEMY};

    std::vector<std::array<TileType, MAZE_HEIGHT>> map;

    void generate_maze();

public:
    bool is_open(int32_t x, int32_t y) { return map[x][y] == OPEN || map[x][y] == PLAYER_START || map[x][y] == PATH; }

    Maze();

    void render(float offet_x, float offset_y);
};
