// Author Mattias Larsson Sköld 2020

#include "gui.h"

#include "matgui/application.h"
#include "matgui/constants.h"
#include "matgui/paint.h"
#include "matgui/window.h"
#include <cmath>

using namespace std;
using namespace MatGui;

struct HexapodInfo {
    Pose pose;
    Pose target;
};

namespace {
std::tuple<float, float> transformToView(Pose &p, float scale) {
    return {p.x * scale + 100, p.y * scale + 300};
}

} // namespace

Gui::Gui(int argc, char **argv)
: _application(argc, argv)
, _window("robot sim")
, _linePaint() {
    _linePaint.line.color(1, 1, 1);
    _targetPaint.line.color(0, 0, 1);
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

void Gui::setHexapodPosition(size_t index, Pose pose) {
    if (index >= _hexapodInfos.size()) {
        _hexapodInfos.resize(index + 1);
    }

    _hexapodInfos.at(index).pose = pose;

    _window.invalidate();
}

void Gui::setHexapodTarget(size_t index, Pose target) {
    _hexapodInfos.at(index).target = target;

    _window.invalidate();
}

void Gui::draw() {
    constexpr auto radconv = MatGui::pi / 180.;
    constexpr auto size = 20.;

    for (auto &i : _hexapodInfos) {
        auto &p = i.pose;
        auto [x, y] = transformToView(p, _scale);

        auto &target = i.target;

        _linePaint.drawEllipse(x - size / 2., y - size / 2., 20, 20);
        _linePaint.drawLine(x,
                            y,
                            x + cos(p.angle * radconv) * 10.,
                            y + sin(p.angle * radconv) * 10);

        auto [tx, ty] = transformToView(target, _scale);

        _targetPaint.drawRect(tx - 2, ty - 2, 4, 4);
        _targetPaint.drawLine(x, y, tx, ty);
    }
}
