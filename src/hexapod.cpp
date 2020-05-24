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
, _addedCoordSysHandle()
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

    _addedCoordSysHandle = b0RemoteApi::readInt(
        _cl->simxGetObjectHandle(
            ("ReferenceFrame_" + std::to_string(_hexapodNum)).c_str(),
            _cl->simxServiceCall()),
        1);

    auto pose = getPose();
    _target.setPos(pose);
}

bool Hexapod::run() {
    auto pose = getPose();
    updateTargetPos();

    bool returnValue = false;

    const float distThreshold = 0.2;
    const float headingThreshold = 5 * degToRad;
    const float inMovementTurnRation = 2.0;
    const float rotationGain = 35e-2;

    auto adjustHeading = [&]() -> bool {
        auto headingDiff = wrapPI(_targets.angle - pose.angle);

        //! @todo: Sign is inverted. But why?
        //! It works this way and not the other
        auto sign = (headingDiff < 0) ? 1 : -1;
        _walkParams.rotationMode =
            rotationGain * pow(headingDiff, 2) * sign * inMovementTurnRation;
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

void Hexapod::setPose(Pose pose) {
    float pos[3] = {pose.x, pose.y, defHeight};
    float orientation[3] = {defAlfa, pose.angle, defGamma};
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
        auto oArr = result->at(1).as<std::array<float, 3>>();
        pose.x = oArr.at(0);
        pose.y = oArr.at(1);
    }

    // Reference frame on the hexapod to measure
    // This seems to work properly
    {
        // Euler angles
        auto result = _cl->simxGetObjectOrientation(
            _addedCoordSysHandle, refFrameHandle, _cl->simxServiceCall());
        auto oArr = result->at(1).as<std::array<float, 3>>();
        pose.angle = oArr.at(2);
    }
    return pose;
}

void Hexapod::updateTargetPos() {
    auto targetPos = _target.getPos();
    _targets.x = targetPos.at(0);
    _targets.y = targetPos.at(1);
}

void Hexapod::setTargetHeading(float heading) {
    _targets.angle = heading;
}

void Hexapod::walk(float velocity, float curvature) {
    _walkParams.stepVelocity = velocity;
    _walkParams.rotationMode = curvature;
    apply(_walkParams);
}

void Hexapod::navigate(Position position, NavigationMode mode) {
    switch (mode) {
    case NavigationMode::Rotation:
        _mode = Mode::SimpleNavigate;
        break;
    case NavigationMode::Translation:
        _mode = Mode::Translate;
        break;
    }

    _targets.x = position.x;
    _targets.y = position.y;
    _target.setPos(position);
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

void Target::setPos(Position position) {
    _pos.x = position.x;
    _pos.y = position.y;
    float pos[3] = {position.x, position.y, _pos.z};
    _cl->simxSetObjectPosition(
        _handle, refFrameHandle, pos, _cl->simxServiceCall());
}

std::array<float, 2> Target::getPos() {
    auto result = _cl->simxGetObjectPosition(
        _handle, refFrameHandle, _cl->simxServiceCall());
    auto oArr = result->at(1).as<std::array<float, 3>>();
    _pos.x = oArr.at(0);
    _pos.y = oArr.at(1);
    return std::array<float, 2>({_pos.x, _pos.y});
}
