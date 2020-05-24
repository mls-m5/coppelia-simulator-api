
#include "organizer.h"
#include "calculateprojections.h"
#include "hexapodconsts.h"

namespace {

bool doesCollide(const std::vector<Path> projections,
                 Position current,
                 size_t ignore,
                 size_t maxIndex) {

    for (size_t i = 0; i < projections.size(); ++i) {
        if (ignore == i) {
            continue;
        }

        auto &projection = projections.at(i);
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

} // namespace

Organizer::Organizer(std::vector<IHexapod *> hexapods, Paths paths)
: _paths(std::move(paths))
, _hexapods(std::move(hexapods)) {
    if (_hexapods.size() != _paths.size()) {
        throw std::runtime_error(
            "paths and hexapod vectors does not have the same size");
    }
    _projections.resize(_paths.size());
    for (size_t i = 0; i < _hexapods.size(); ++i) {
        //        _hexapods.at(i)->navigate(_paths.at(i).front(),
        //        IHexapod::Translation);
        _hexapods.at(i)->navigate(_paths.at(i).front(), IHexapod::Rotation);
    }
}

Path Organizer::getProjection(size_t index) {
    return _projections.at(index);
}

void Organizer::run() {
    for (size_t i = 0; i < _hexapods.size(); ++i) {
        _projections.at(i) = calculateProjections(
            _paths.at(i), 0, _hexapods.at(i)->getPose(), 10, .3);
    }

    size_t longestProjection = 0;
    for (auto &projection : _projections) {
        longestProjection = std::max(longestProjection, projection.size());
    }

    for (size_t p = 0; p < longestProjection; ++p) {
        for (size_t i = 0; i < _hexapods.size(); ++i) {
            auto &projection = _projections.at(i);
            if (projection.size() <= p) {
                continue;
            }
            if (doesCollide(_projections, projection.at(p), i, p)) {
                /// ?
            }
        }
    }
}
