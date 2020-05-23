#include "hexapod.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <sstream>

namespace {
const int refFrameHandle = -1;

// Default orientations
const float defHeight = 8.8e-2;
const float defAlfa = -M_PI / 2;
const float defGamma = -M_PI / 2;
const float degToRad = M_PI / 180;

const std::string addNumToString(const char *c, int i) {
    std::stringstream sstr;
    sstr << c << i;
    return sstr.str();
}

float wrapPI(float angle) {
    while (angle > M_PI) {
        angle -= 2 * M_PI;
    }
    while (angle < -M_PI) {
        angle += 2 * M_PI;
    }
    return angle;
}

void constrain(float &value, float min, float max) {
    if (value > max) {
        value = max;
    }
    if (value < min) {
        value = min;
    }
}

void constrain(float &value, float minMax) {
    constrain(value, -minMax, minMax);
}

} // namespace

Hexapod::Hexapod(b0RemoteApi *cl, int hexapodNum)
: _cl(cl)
, _handle()
, _refFrameHandle()
, _hexapodNum(hexapodNum + 1)
, _targets({})
, _walkParams({.stepVelocity = 0.9,
               .movementDirection = 0,
               .rotationMode = 0,
               .movementStrength = 1})
, _mode(Mode::None)
, _target(cl, hexapodNum)
, _lastCoppeliaCalls({})
, _NavMode(NavigationMode::Rotation) {
    std::cout << "hexapod_" << _hexapodNum << "ReferenceFrame_" << _hexapodNum
              << std::endl;
    _handle = b0RemoteApi::readInt(
        cl->simxGetObjectHandle(
            ("hexapod_" + std::to_string(_hexapodNum)).c_str(),
            _cl->simxServiceCall()),
        1);

    _refFrameHandle = b0RemoteApi::readInt(
        _cl->simxGetObjectHandle(
            ("ReferenceFrame_" + std::to_string(_hexapodNum)).c_str(),
            _cl->simxServiceCall()),
        1);

    auto pose = getPose();
    _target.setPos(pose.x, pose.y);
}

bool Hexapod::run() {
    auto pose = getPose();

    bool returnValue = false;

    const float distThreshold = 0.2;
    const float headingThreshold = 5 * degToRad;
    const float inMovementTurnRation = 2.0;
    const float rotationGain = 35e-2;

    auto adjustHeading = [&]() -> bool {
        auto headingDiff = wrapPI(_targets.angle - pose.angle);

        _walkParams.rotationMode =
            rotationGain * pow(headingDiff, 2) * inMovementTurnRation;
        constrain(_walkParams.rotationMode, 1.0);

        if (headingDiff < headingThreshold) {
            return true;
        }
        return false;
    };

    auto simpleNavigate = [&]() -> bool {
        auto diffX = _targets.x - pose.x;
        auto diffY = _targets.y - pose.y;
        auto distToTarget = std::sqrt(pow(diffX, 2) + pow(diffY, 2));
        _targets.angle = atan2(diffY, diffX);

        _walkParams.movementDirection = 0; // We don't translate
        _walkParams.stepVelocity = 1.0;

        if (distToTarget < distThreshold) {
            _walkParams.rotationMode = 0;
            _walkParams.stepVelocity = 0;
            return true;
        }
        adjustHeading();
        return false;
    };

    auto translate = [&]() -> bool {
        auto diffX = _targets.x - pose.x;
        auto diffY = _targets.y - pose.y;
        // std::cout << "pose z: " << pose.angle << std::endl;
        _walkParams.movementDirection = atan2(diffY, diffX) - pose.angle;
        _targets.angle = 0;
        _walkParams.stepVelocity = 1.0;

        auto distToTarget = std::sqrt(pow(diffX, 2) + pow(diffY, 2));

        if (distToTarget < distThreshold) {
            _walkParams.rotationMode = 0;
            _walkParams.stepVelocity = 0;
            return true;
        }
        return false;
    };

    switch (_mode) {
    case Mode::None:
        break;
    case Mode::AdjustHeading:
        returnValue = adjustHeading();
        if (returnValue) {
            _walkParams.stepVelocity = 0;
        }
        break;
    case Mode::SimpleNavigate:
        returnValue = simpleNavigate();
        break;
    case Mode::Translate:
        returnValue = translate();
        break;
    }
    apply(_walkParams);
    return returnValue;
}

void Hexapod::setPose(float x, float y, float w) {
    float pos[3] = {x, y, defHeight};
    float orientation[3] = {defAlfa, w, defGamma};
    _cl->simxSetObjectPosition(
        _handle, refFrameHandle, pos, _cl->simxServiceCall());
    _cl->simxSetObjectOrientation(
        _handle, refFrameHandle, orientation, _cl->simxServiceCall());
}

Pose Hexapod::getPose() const {
    Pose pose;
    {
        auto result = _cl->simxGetObjectPosition(
            _handle, refFrameHandle, _cl->simxServiceCall());
        // b0RemoteApi::print(result);
        auto oArr = result->at(1).as<std::array<float, 3>>();
        pose.x = oArr.at(0);
        pose.y = oArr.at(1);
    }

    // Reference frame on the hexapod to measure
    // This seems to work properly
    {
        // Euler angles
        auto result = _cl->simxGetObjectOrientation(
            _refFrameHandle, refFrameHandle, _cl->simxServiceCall());
        auto oArr = result->at(1).as<std::array<float, 3>>();
        // std::cout << "ref heading: " << oArr.at(2) << std::endl;
        pose.angle = oArr.at(2);
    }
    return pose;
}

void Hexapod::setTargetHeading(float heading) {
    _targets.angle = heading;
}

void Hexapod::walk(float velocity, float curvature) {
    _walkParams.stepVelocity = velocity;
    _walkParams.rotationMode = curvature;
    apply(_walkParams);
}

void Hexapod::navigate(float x, float y, NavigationMode mode) {
    switch (mode) {
    case NavigationMode::Rotation:
        _mode = Mode::SimpleNavigate;
        break;
    case NavigationMode::Translation:
        _mode = Mode::Translate;
        break;
    }

    _targets.x = x;
    _targets.y = y;
    _target.setPos(x, y);
}

Pose Hexapod::getTarget() const {
    return _targets;
}

void Hexapod::stop() {
    _cl->simxSetFloatSignal("stepAmplitude", 0, _cl->simxServiceCall());
}

void Hexapod::setMode(Mode mode) {
    _mode = mode;
}

void Hexapod::apply(WalkParams params) {
    // std::cout << "movement dir: " << params.movementDirection << std::endl;
    if (params.stepVelocity != _lastCoppeliaCalls.stepVelocity) {
        _cl->simxSetFloatSignal(
            ("stepVelocity" + std::to_string(_hexapodNum)).c_str(),
            params.stepVelocity,
            _cl->simxServiceCall());
        _lastCoppeliaCalls.stepVelocity = params.stepVelocity;
    }
    if (params.movementDirection != _lastCoppeliaCalls.movementDirection) {
        _cl->simxSetFloatSignal(
            ("movementDirection" + std::to_string(_hexapodNum)).c_str(),
            params.movementDirection,
            _cl->simxServiceCall());
        _lastCoppeliaCalls.movementDirection = params.movementDirection;
    }
    if (params.rotationMode != _lastCoppeliaCalls.rotationMode) {
        _cl->simxSetFloatSignal(
            ("rotationMode" + std::to_string(_hexapodNum)).c_str(),
            params.rotationMode,
            _cl->simxServiceCall());
        _lastCoppeliaCalls.rotationMode = params.rotationMode;
    }
    if (params.movementStrength != _lastCoppeliaCalls.movementStrength) {

        _cl->simxSetFloatSignal(
            ("movementStrength" + std::to_string(_hexapodNum)).c_str(),
            params.movementStrength,
            _cl->simxServiceCall());
        _lastCoppeliaCalls.movementStrength = params.movementStrength;
    }
}

Target::Target(b0RemoteApi *cl, int targetNum)
: _cl(cl)
, _pos({0, 0, 0.1})
, _handle() {
    _handle = b0RemoteApi::readInt(
        cl->simxGetObjectHandle(
            addNumToString("target_", targetNum + 1).c_str(),
            cl->simxServiceCall()),
        1);
}

void Target::setPos(float x, float y) {
    _pos.at(0) = x;
    _pos.at(1) = y;
    float pos[3] = {x, y, _pos.at(2)};
    _cl->simxSetObjectPosition(
        _handle, refFrameHandle, pos, _cl->simxServiceCall());
}

std::array<float, 2> Target::getPos() {
    return std::array<float, 2>({_pos.at(0), _pos.at(1)});
}
