
#include "calculateprojections.h"

std::vector<Position> calculateProjections(Path path,
                                           size_t startIndex,
                                           Position hexPosition,
                                           size_t numSteps,
                                           float stepLen) {

    std::vector<Position> ret;

    std::vector<Position> pathPoints;
    pathPoints.reserve(path.size() - startIndex + 1);

    pathPoints.push_back(hexPosition);
    for (size_t i = startIndex; i < path.size(); ++i) {
        pathPoints.push_back(path.at(i));
    }

    if (pathPoints.size() <= 1) {
        return {};
    }

    float lenLeft = 0;

    for (size_t i = 1; i < pathPoints.size(); ++i) {
        auto p1 = pathPoints.at(i - 1);
        auto p2 = pathPoints.at(i);

        auto distance = p2 - p1;
        auto len = distance.abs();
        auto d = distance / len;
        lenLeft += len;

        while (lenLeft > 0 && ret.size() < numSteps) {
            auto newPosition = p2 - d * lenLeft;
            ret.push_back(newPosition);
            lenLeft -= stepLen;
        }

        if (ret.size() >= numSteps) {
            break;
        }
    }

    return ret;
}
