
#pragma once

#include "position.h"

struct Pose : public Position {
    Pose(float x = 0, float y = 0, float angle = 0)
    : Position(x, y)
    , angle(angle) {}

    Pose(const Position &position)
    : Position(position)
    , angle(0) {}

    float angle = 0;
};
