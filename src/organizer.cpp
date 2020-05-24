
#include "organizer.h"
#include "calculateprojections.h"

Organizer::Organizer(std::vector<IHexapod *> hexapods, Paths paths)
: _paths(std::move(paths))
, _hexapods(std::move(hexapods)) {
    if (_hexapods.size() != _paths.size()) {
        throw std::runtime_error(
            "paths and hexapod vectors does not have the same size");
    }
    _projections.resize(_paths.size());
    for (size_t i = 0; i < _hexapods.size(); ++i) {
        _hexapods.at(i)->navigate(_paths.at(i).front(), IHexapod::Translation);
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
}
