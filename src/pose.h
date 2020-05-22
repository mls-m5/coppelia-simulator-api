
#pragma once

struct Pose {
    Pose(float x, float y, float angle = 0)
    : x(x)
    , y(y)
    , angle(angle) {}

    template <class T>
    Pose(T arr)
    : x(arr.at(0))
    , y(arr.at(1))
    , angle(arr.at(2)) {}

    Pose() = default;
    ~Pose() = default;

    float x = 0, y = 0;
    float angle = 0;
};
