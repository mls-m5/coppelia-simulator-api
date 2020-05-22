// Author Mattias Larsson Sköld 2020

#include "gui.h"

#include "matgui/application.h"
#include "matgui/constants.h"
#include "matgui/paint.h"
#include "matgui/window.h"
#include <cmath>

using namespace std;
using namespace MatGui;

Gui::Gui(int argc, char **argv)
: _application(make_unique<Application>(argc, argv))
, _window(make_unique<Window>("robot sim"))
, _linePaint(make_unique<Paint>()) {
    _linePaint->line.color(1, 1, 1);
    _window->frameUpdate.connect([this]() { draw(); });
}

Gui::~Gui() = default;

void Gui::mainLoop() {
    _application->mainLoop();
}

void Gui::setHexapodPosition(size_t index, Pose pose) {
    if (index >= _hexapodPoses.size()) {
        _hexapodPoses.resize(index + 1);
    }

    _hexapodPoses.at(index) = pose;

    _window->invalidate();
}

void Gui::draw() {
    constexpr auto radconv = MatGui::pi / 180.;
    constexpr auto size = 20.;

    for (auto &p : _hexapodPoses) {
        auto x = p.x * 100.;
        auto y = p.y * 100.;

        _linePaint->drawEllipse(x - size / 2., y - size / 2., 20, 20);
        _linePaint->drawLine(x,
                             y,
                             x + cos(p.angle * radconv) * 10.,
                             y + sin(p.angle * radconv) * 10);
    }
}
