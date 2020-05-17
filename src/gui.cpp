// Author Mattias Larsson Sköld 2020

#include "gui.h"

#include "matgui/application.h"
#include "matgui/paint.h"
#include "matgui/window.h"

using namespace std;
using namespace MatGui;

Gui::Gui(int argc, char **argv)
: _application(make_unique<Application>(argc, argv))
, _window(make_unique<Window>("robot sim"))
, _linePaint(make_unique<Paint>()) {
    _linePaint->line.color(1, 1, 1);
    _window->frameUpdate.connect([this]() {
        _linePaint->drawLine(0, 0, _window->width(), _window->height());
    });
}

Gui::~Gui() = default;

void Gui::mainLoop() {
    _application->mainLoop();
}
