
#pragma once

#include "ihexapod.h"
#include "path.h"

class Organizer {
public:
    Organizer(std::vector<IHexapod *> hexapods, Paths paths);
    Path getProjection(size_t index);

    //! Update projections and handle hexapods movemenets
    void run();

private:
    std::vector<Path> _paths;
    std::vector<Path> _projections;
    std::vector<IHexapod *> _hexapods;
};
