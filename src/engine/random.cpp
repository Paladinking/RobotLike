#include <chrono>
#include <random>
#include "engine.h"

std::minstd_rand generator(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));

int engine::random(const int min, const int max) {
    if (min >= max) return min;
    return (static_cast<int>(generator()) % (max - min)) + min;
}
