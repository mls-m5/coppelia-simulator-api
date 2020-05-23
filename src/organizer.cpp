
#include "organizer.h"

Organizer::Organizer(std::vector<IHexapod *> hexapods, Paths paths)
: _paths(std::move(paths))
, _hexapods(std::move(hexapods)) {}

Path Organizer::getProjection(size_t index) {
    return _paths.at(index);
}
