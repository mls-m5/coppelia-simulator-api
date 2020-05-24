
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
    struct HexapodData {
        Path path;
        Path projection;
        IHexapod *hexapod;
        size_t freeProjectionLength;
    };
    bool doesCollide(Position current, size_t ignore, size_t maxIndex) const;

    std::vector<HexapodData> _hexData;
};
