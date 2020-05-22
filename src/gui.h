// Author Mattias Larsson Sköld 2020

#pragma once

#include "matgui/application.h"
#include "matgui/paint.h"
#include "matgui/window.h"
#include "pose.h"
#include "position.h"

#include <memory>
#include <vector>

class Gui {
public:
    Gui(int argc, char **argv);
    ~Gui();

    //! Runs as long as the window is open and handles gui stuff
    void mainLoop();

    void setHexapodPosition(size_t index, Pose pose);
    void setHexapodTarget(size_t index, Pose target);

    void draw();

private:
    MatGui::Application _application;
    MatGui::Window _window;
    MatGui::Paint _linePaint;
    MatGui::Paint _targetPaint;

    std::vector<struct HexapodInfo> _hexapodInfos;

    float _scale = 40;
};
