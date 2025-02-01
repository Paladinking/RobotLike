#define TURN_CHANCE 100
#define BREAK_CHANCE 100
#define STOP_CHANCE 0
#include "maze.h"
#include "engine/engine.h"
#include <algorithm>
#include <unordered_set>
#include <memory>
#include <queue>
#include <iostream>

constexpr uint32_t MIN_ROOM_COUNT = 10;
constexpr uint32_t MAX_ROOM_COUNT = 15;
constexpr uint32_t ROOM_COUNT = 10;
constexpr uint32_t ROOM_MIN_SIZE = 4;
constexpr uint32_t ROOM_MAX_SIZE = 10;
constexpr uint32_t MAX_TRIES = 100;


void Maze::generate_maze() {
    struct Room {
        int32_t x, y, w, h;

        std::pair<int32_t, int32_t> middle() {
            return {x + w / 2, y + h / 2};
        }
    };


    for (auto& row: map) {
        std::fill(row.begin(), row.end(), TileType::VOID);
    }

    auto generate_room = []() {
        int32_t w = engine::random<int32_t>(ROOM_MIN_SIZE, ROOM_MAX_SIZE);
        int32_t h = engine::random<int32_t>(ROOM_MIN_SIZE, ROOM_MAX_SIZE);
        int32_t x = engine::random<int32_t>(0, MAZE_WIDTH - w);
        int32_t y = engine::random<int32_t>(0, MAZE_HEIGHT - h);
        return Room{x, y, w, h};
    };

    auto intersects = [](Room a, Room b) {
        int32_t aw = a.x + a.w, bw = b.x + b.w;
        int32_t ah = a.y + a.h, bh = b.y + b.h;
        return ((bw < b.x || bw > a.x) &&
                (bh < b.y || bh > a.y) &&
                (aw < a.x || aw > b.x) &&
                (ah < a.y || ah > b.y));   
    };

    auto add_room = [this](Room r) {
        for (int32_t x1 = r.x; x1 <= r.x + r.w; x1++) {
            for (int y1 = r.y; y1 <= r.y + r.h; y1++) {
                if (x1 == r.x || x1 == r.x + r.w || 
                    y1 == r.y || y1 == r.y + r.h) {
                    map[x1][y1] = WALL;
                } else {
                    map[x1][y1] = OPEN;
                }
            }
        }
    };
    struct Node {
        Node(Node* p, int32_t x, int32_t y) : par{p}, x{x}, y{y} {}
        Node* par;
        int32_t x, y;
        double fscore = 0;
        double gscore = std::numeric_limits<double>::max();

        double hscore(std::pair<int32_t, int32_t> goal) {
            int32_t dx = x - goal.first, dy = y - goal.second;
            return std::abs(dx) + std::abs(dy);
        }

        std::array<Node, 4> neigbors() {
            return {
                Node {this, x - 1, y},
                Node {this, x + 1, y},
                Node {this, x, y - 1},
                Node {this, x,  y + 1}
            };
        }
    };
    auto cmpr = [](Node* a, Node* b) {
        return a->fscore > b->fscore;
    };

    auto connect_rooms = [this, &cmpr](Room a, Room b) {
        struct hash {
            std::size_t operator()(std::pair<int32_t, int32_t> p) const {
                return std::hash<int32_t>{}(p.first) ^ 
                       std::hash<int32_t>{}(p.second);
                return 0;
            }
        };
        std::priority_queue<Node*, std::vector<Node*>, decltype(cmpr)> queue{cmpr};
        std::unordered_set<std::pair<int32_t, int32_t>, hash> closed{};

        struct Defer{
            std::vector<Node*> nodes{};
            ~Defer() {
                for (Node* n: nodes) {
                    delete n;
                }
            }
            Node* add(Node n) {
                Node* n1 = new Node{n};
                nodes.push_back(n1);
                return n1;
            }
            Node* add(Node* parent, int32_t x, int32_t y) {
                Node* n = new Node(parent, x, y);
                nodes.push_back(n);
                return n;
            }
        } nodes;

        auto start = nodes.add(nullptr, a.middle().first, a.middle().second);
        auto goal = b.middle();
        start->fscore = start->hscore(goal);
        start->gscore = 0;
        queue.push(start);
        while (queue.size() > 0) {
            auto* top = queue.top();
            queue.pop();
            if (closed.count({top->x, top->y}) > 0) {
                continue;
            }
            if (top->x == goal.first && top->y == goal.second) {
                Node* node = top;
                do {
                    map[node->x][node->y] = PATH;
                    node = node->par;
                } while (node != nullptr);
                return true;
            }
            closed.insert({top->x, top->y});
            for (auto node: top->neigbors()) {
                if (node.x < 0 || node.y < 0 ||
                    node.x >= MAZE_WIDTH || node.y >= MAZE_HEIGHT) {
                    continue;
                }
                if (closed.count({node.x, node.y}) > 0) {
                    continue;
                }
                node.gscore = top->gscore;
                if (map[node.x][node.y] == VOID || map[node.x][node.y] == OPEN) {
                    node.gscore += 1;
                } else if (map[node.x][node.y] == WALL) {
                    node.gscore += 2;
                }
                node.fscore = node.hscore(goal) + node.gscore;
                queue.push(nodes.add(node));
            }
        }
        return false;
    };

    std::vector<Room> rooms{};
    uint32_t tries = 0;

    uint32_t room_count = engine::random<uint32_t>(MIN_ROOM_COUNT, MAX_ROOM_COUNT);
    for (uint32_t i = 0; i < room_count;) {
        auto room = generate_room();
        auto room_intersects = [&room, &intersects](Room r) { 
            return intersects(r, room); 
        };
        if (std::any_of(rooms.begin(), rooms.end(), room_intersects)) {
            if (tries >= MAX_TRIES) {
                break;
            }
            ++tries;
            continue;
        }
        rooms.push_back(room);
        add_room(room);
        ++i;
    }

    for (uint32_t ix = 0; ix < rooms.size() - 1; ++ix) {
        connect_rooms(rooms[ix], rooms[ix + 1]);
    }

    for (uint32_t i = 0; i < 8; ++i) {
        uint32_t ix1 = engine::random<uint32_t>(0, rooms.size());
        uint32_t ix2 = engine::random<uint32_t>(0, rooms.size());
        connect_rooms(rooms[ix1], rooms[ix2]);
    }

}

Maze::Maze() : map{MAZE_WIDTH} { generate_maze(); }

void Maze::render(float offset_x, float offset_y) {
    constexpr SDL_Color LIGHTGRAY = {0xe0, 0xe0, 0xe0, 0xff};
    constexpr SDL_Color GRAY = {0x3f, 0x3f, 0x3f, 0xff};
    constexpr SDL_Color WHITE = {0xff, 0xff, 0xff, 0xff};
    constexpr SDL_Color BLACK = {0x0, 0x0, 0x0, 0xff};

    for (int i = 0; i < map.size(); i++) {
        for (int j = 0; j < map[0].size(); j++) {
            SDL_Color color = WHITE;
            SDL_FRect rect = {offset_x + TILE_SIZE * i,
                              offset_y + TILE_SIZE * j, TILE_SIZE,
                              TILE_SIZE};
            if (is_open(i, j)) {
                color = GRAY;
                (i % 2 == 0) ? LIGHTGRAY : ((j % 2 == 0) ? GRAY : BLACK);
            }
            SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b,
                                   color.a);
            SDL_RenderFillRectF(gRenderer, &rect);
        }
    }
}
