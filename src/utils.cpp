#include "utils.h"
#include <cmath>
#include <utility>


vec2i::vec2i(int32_t x, int32_t y) : x(x), y(y) {}


int32_t vec2i::dist_to(vec2i const& other) {
    return std::sqrt(dist_sqrd_to(other));
}


int32_t vec2i::dist_sqrd_to(vec2i const& other) {
    return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
}


int32_t vec2i::dist_manhattan_to(vec2i const& other) {
    return std::abs(x - other.x) + std::abs(y - other.y);
}


int32_t vec2i::dist_manhattan_diag_to(vec2i const& other) {
    return std::max(std::abs(x - other.x), std::abs(y - other.y));
}

vec2i vec2i_from_dir(DIR dir) {
    switch (dir) {
    case DIR::DIR_UP:
        return vec2i{0, -1};
    case DIR::DIR_RIGHT:
        return vec2i{1, 0};
    case DIR::DIR_DOWN:
        return vec2i{0, 1};
    case DIR::DIR_LEFT:
    default:
        return vec2i{-1, 0};
    };
}
