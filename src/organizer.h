
#pragma once

#include "ihexapod.h"
#include "path.h"

class Organizer {
public:
    Organizer(std::vector<IHexapod *> hexapods, Paths paths);
    Path getProjection(size_t index);

private:
    std::vector<Path> _paths;
    std::vector<IHexapod *> _hexapods;
};
