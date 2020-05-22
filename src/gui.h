// Author Mattias Larsson Sköld 2020

#pragma once

#include "matgui/matguifwd.h"
#include "pose.h"

#include <memory>
#include <vector>

class Gui {
public:
    Gui(int argc, char **argv);
    ~Gui();

    //! Runs as long as the window is open and handles gui stuff
    void mainLoop();

    void setHexapodPosition(size_t index, Pose pose);

    void draw();

private:
    std::unique_ptr<MatGui::Application> _application;
    std::unique_ptr<MatGui::Window> _window;
    std::unique_ptr<MatGui::Paint> _linePaint;

    std::vector<Pose> _hexapodPoses;
};
