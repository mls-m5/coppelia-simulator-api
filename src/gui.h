// Author Mattias Larsson Sköld 2020

#pragma once

#include "matgui/application.h"
#include "matgui/paint.h"
#include "matgui/window.h"
#include "path.h"
#include "pose.h"
#include "position.h"

#include <memory>
#include <vector>

class Gui {
public:
    struct HexapodInfo {
        Pose pose;
        Pose target;
        Path projection;
        std::array<float, 3> color;
        size_t freeProjectionLength;
    };

    Gui(int argc, char **argv);
    ~Gui();

    //! Runs as long as the window is open and handles gui stuff
    void mainLoop();

    void setHexapodInformation(size_t index, HexapodInfo);
    void setPaths(Paths paths);

    void draw();

private:
    Position transformToView(const Position &p) const;

    void drawProjections(const Path &projections, size_t freeLength);

    MatGui::Application _application;
    MatGui::Window _window;
    MatGui::Paint _linePaint;
    MatGui::Paint _targetPaint;
    MatGui::Paint _pathPaint;
    MatGui::Paint _projectionPaint;
    MatGui::Paint _blockedProjectionPaint;

    std::vector<struct HexapodInfo> _hexapodInfos;
    Paths _paths;

    float _scale = 40;
};
