
#include "organizer.h"
#include "calculateprojections.h"
#include "hexapodconsts.h"

namespace {} // namespace

Organizer::Organizer(std::vector<IHexapod *> hexapods, Paths paths)
: _hexData(hexapods.size()) {
    if (hexapods.size() != paths.size()) {
        throw std::runtime_error(
            "paths and hexapod vectors does not have the same size");
    }
    for (size_t i = 0; i < _hexData.size(); ++i) {
        auto &data = _hexData.at(i);
        data.hexapod = hexapods.at(i);
        data.path = std::move(paths.at(i));
    }
}

Path Organizer::getProjection(size_t index) {
    return _hexData.at(index).projection;
}

void Organizer::run() {
    for (size_t i = 0; i < _hexData.size(); ++i) {
        auto &data = _hexData.at(i);
        auto &path = data.path;
        auto &hexapod = data.hexapod;
        if (path.size() <= 1) {
            continue;
        }
        if (hexapod->isAtTarget()) {
            path.erase(path.begin());
            hexapod->navigate(path.front(), IHexapod::Rotation);
        }
    }

    for (size_t i = 0; i < _hexData.size(); ++i) {
        auto &data = _hexData.at(i);
        auto &projection = data.projection;
        auto &path = data.path;
        auto &hexapod = data.hexapod;
        projection = calculateProjections(path, 0, hexapod->getPose(), 10, .3);
    }

    size_t longestProjection = 0;
    for (auto &data : _hexData) {
        longestProjection = std::max(longestProjection, data.projection.size());
    }

    for (size_t p = 0; p < longestProjection; ++p) {
        for (size_t i = 0; i < _hexData.size(); ++i) {
            auto &data = _hexData.at(i);
            auto &projection = data.projection;
            if (projection.size() <= p) {
                continue;
            }
            if (doesCollide(projection.at(p), i, p)) {
            }
        }
    }
}

bool Organizer::doesCollide(Position current,
                            size_t ignore,
                            size_t maxIndex) const {

    for (size_t i = 0; i < _hexData.size(); ++i) {
        if (ignore == i) {
            continue;
        }

        auto &data = _hexData.at(i);

        auto &projection = data.projection;
        for (size_t pi = 0; pi < projection.size(); ++pi) {
            if (pi >= maxIndex) {
                break;
            }
            if ((projection.at(pi) - current).abs2() <
                HexapodRadius * HexapodRadius) {
                return true;
            }
        }
    }

    return false;
}
