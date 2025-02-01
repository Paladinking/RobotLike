#pragma once
#include <cstdint>



enum FACTIONS {
    ROBOTS,
    CREATURES,
};


enum DIST_TYPES {
    EUCLIDEAN,
    MANHATTAN,
    MANHATTAN_DIAG,
};


enum DIR {
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
};


struct vec2i {
public:
    vec2i(int32_t x, int32_t y);

    int32_t dist_to(vec2i const& other);
    int32_t dist_sqrd_to(vec2i const& other);
    int32_t dist_manhattan_to(vec2i const& other);
    int32_t dist_manhattan_diag_to(vec2i const& other);

    int32_t x, y;
};

vec2i vec2i_from_dir(DIR dir);
