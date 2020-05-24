
#include "organizer.h"
#include "calculateprojections.h"
#include "hexapodconsts.h"

namespace {} // namespace

Organizer::Organizer(std::vector<IHexapod *> hexapods, Paths paths)
: _hexData(hexapods.size()) {
    if (hexapods.size() > paths.size()) {
        throw std::runtime_error("not enough paths");
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

size_t Organizer::getFreeProjectionLength(size_t index) {
    return _hexData.at(index).freeProjectionLength;
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
        data.isStillFree = true;
        data.freeProjectionLength = data.projection.size();
    }

    for (size_t projectionIndex = 1; projectionIndex < longestProjection;
         ++projectionIndex) {
        for (size_t i = 0; i < _hexData.size(); ++i) {
            auto &data = _hexData.at(i);
            auto &projection = data.projection;
            if (projection.size() <= projectionIndex) {
                continue;
            }
            if (!data.isStillFree) {
                continue;
            }
            if (doesCollide(
                    projection.at(projectionIndex), i, projectionIndex)) {
                data.freeProjectionLength = projectionIndex - 1;
                data.isStillFree = false;
            }
        }
    }
}

bool Organizer::doesCollide(Position current,
                            size_t ignore,
                            size_t maxIndex) const {

    const auto r = HexapodRadius * 2;
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
            if (pi >= data.freeProjectionLength) {
                break;
            }
            if ((projection.at(pi) - current).abs2() < r * r) {
                return true;
            }
        }
    }

    return false;
}
