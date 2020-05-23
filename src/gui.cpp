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
std::tuple<float, float> transformToView(Position &p, float scale) {
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
    constexpr auto radconv = MatGui::pi / 180.;
    constexpr auto size = 20.;

    for (auto &i : _hexapodInfos) {

        auto &target = i.target;
        auto [tx, ty] = transformToView(target, _scale);

        _targetPaint.drawRect(tx - 2, ty - 2, 4, 4);

        auto &p = i.pose;
        auto [x, y] = transformToView(p, _scale);

        _targetPaint.drawLine(x, y, tx, ty);

        _linePaint.drawEllipse(x - size / 2., y - size / 2., 20, 20);

        _linePaint.drawLine(x,
                            y,
                            x + cos(p.angle * radconv) * 10.,
                            y + sin(p.angle * radconv) * 10);
    }

    for (auto &p : _paths) {
        for (size_t i = 1; i < p.second.size(); ++i) {

            auto [x1, y1] = transformToView(p.second.at(i - 1), _scale);
            auto [x2, y2] = transformToView(p.second.at(i), _scale);

            _targetPaint.drawLine(x1, y1, x2, y2);
        }
    }
}
