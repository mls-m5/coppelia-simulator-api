#pragma once

#include "path.h"
#include "position.h"
#include <vector>

std::vector<Position> calculateProjections(Path path,
                                           size_t startIndex,
                                           Position hexPosition,
                                           size_t numSteps,
                                           float stepLen);
