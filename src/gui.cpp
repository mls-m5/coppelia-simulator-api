// Author Mattias Larsson Sköld 2020

#include "gui.h"

#include "matgui/application.h"
#include "matgui/constants.h"
#include "matgui/paint.h"
#include "matgui/window.h"
#include <cmath>

using namespace std;
using namespace MatGui;

namespace {
Position transformToView(Position &p, float scale) {
    return {p.x * scale + 100, p.y * scale + 300};
}

} // namespace

Gui::Gui(int argc, char **argv)
: _application(argc, argv)
, _window("robot sim")
, _linePaint() {
    _linePaint.line.color(1, 1, 1);
    _targetPaint.line.color(0, 0, 1);
    _projectionPaint.line.color(0, .5, 0);
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
    constexpr auto size = 20.;

    for (auto &i : _hexapodInfos) {

        auto &target = i.target;
        auto transformedT = transformToView(target, _scale);

        _targetPaint.drawRect(transformedT.x - 2, transformedT.y - 2, 4, 4);

        auto &p = i.pose;
        auto transformedP = transformToView(p, _scale);

        _targetPaint.drawLine(
            transformedP.x, transformedP.y, transformedT.x, transformedT.y);

        _linePaint.drawEllipse(
            transformedP.x - size / 2., transformedP.y - size / 2., 20, 20);

        _linePaint.drawLine(transformedP.x,
                            transformedP.y,
                            transformedP.x + cos(p.angle) * 10.,
                            transformedP.y + sin(p.angle) * 10);

        drawProjections(i.projection);
    }

    for (auto &p : _paths) {
        for (size_t i = 1; i < p.size(); ++i) {

            auto p1 = transformToView(p.at(i - 1), _scale);
            auto p2 = transformToView(p.at(i), _scale);

            _targetPaint.drawLine(p1.x, p1.y, p2.x, p2.y);
        }
    }
}

void Gui::drawProjections(const Path &projections) {
    const float size = 30;
    for (auto p : projections) {
        auto [x, y, z] = transformToView(p, _scale);
        _projectionPaint.drawEllipse(x - size / 2, y - size / 2, size, size);
    }
}
