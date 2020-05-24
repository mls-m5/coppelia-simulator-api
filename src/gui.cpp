// Author Mattias Larsson Sköld 2020

#include "gui.h"

#include "hexapodconsts.h"
#include "matgui/application.h"
#include "matgui/constants.h"
#include "matgui/paint.h"
#include "matgui/window.h"
#include <cmath>

using namespace std;
using namespace MatGui;

namespace {} // namespace

Gui::Gui(int argc, char **argv)
: _application(argc, argv)
, _window("robot sim")
, _linePaint() {
    _linePaint.line.color(1, 1, 1);
    _targetPaint.line.color(0, 0, 1);
    _pathPaint.line.color(0, 0, 1, .1);
    _projectionPaint.line.color(0, .5, 0);
    _blockedProjectionPaint.line.color(0, .5, 0, .4);
    _window.frameUpdate.connect([this]() { draw(); });

    _window.scroll.connect([this](View::ScrollArgument arg) {
        _scale *= (1. + arg.y / 10);
        _window.invalidate();
    });
}

Gui::~Gui() = default;

void Gui::mainLoop() {
    _application.mainLoop();
}

void Gui::setHexapodInformation(size_t index, HexapodInfo info) {
    if (index >= _hexapodInfos.size()) {
        _hexapodInfos.resize(index + 1);
    }

    _hexapodInfos.at(index) = info;

    _window.invalidate();
}

void Gui::setPaths(Paths paths) {
    _paths = paths;
}

void Gui::draw() {
    const auto size = HexapodRadius * 2 * _scale;

    for (auto &info : _hexapodInfos) {
        _linePaint.line.color(
            info.color.at(0), info.color.at(1), info.color.at(2));
        auto &target = info.target;
        auto transformedT = transformToView(target);

        _targetPaint.drawRect(transformedT.x - 2, transformedT.y - 2, 4, 4);

        auto &p = info.pose;
        auto transformedP = transformToView(p);

        _targetPaint.drawLine(
            transformedP.x, transformedP.y, transformedT.x, transformedT.y);

        drawProjections(info.projection, info.freeProjectionLength);

        _linePaint.drawEllipse(
            transformedP.x - size / 2., transformedP.y - size / 2., size, size);

        _linePaint.drawLine(transformedP.x,
                            transformedP.y,
                            transformedP.x + cos(p.angle) * size / 2.,
                            transformedP.y - sin(p.angle) * size / 2.);
    }

    for (auto &p : _paths) {
        for (size_t i = 1; i < p.size(); ++i) {

            const auto p1 = transformToView(p.at(i - 1));
            const auto p2 = transformToView(p.at(i));

            _pathPaint.drawLine(p1.x, p1.y, p2.x, p2.y);
        }
    }
}

Position Gui::transformToView(const Position &p) const {
    return {p.x * _scale + 200, 200.f - p.y * _scale};
}

void Gui::drawProjections(const Path &projections, size_t freeLength) {
    const float size = HexapodRadius * 2 * _scale;
    for (size_t i = 0; i < projections.size(); ++i) {
        const auto &p = projections.at(i);
        auto [x, y, z] = transformToView(p);
        if (i <= freeLength) {
            _projectionPaint.drawEllipse(
                x - size / 2, y - size / 2, size, size);
        }
        else {
            _blockedProjectionPaint.drawEllipse(
                x - size / 2, y - size / 2, size, size);
        }
    }
}
