#include "hexapod.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <sstream>

namespace {
    const int refFrameHandle = -1;

    // Default orientations
    const float defHeight = 8.8e-2;
    const float defAlfa = -M_PI/2;
    const float defGamma = -M_PI/2;

    const std::string addNumToString(const char* c, int i)
    {
        std::stringstream sstr;
        sstr << c << i;
        return sstr.str();
    }

    float wrap180(float angle)
    {
        if (angle > 180)
        {
            angle -= 360;
        }
        else if (angle < -180)
        {
            angle += 360;
        }
        return angle;
    }

}

Hexapod::Hexapod(b0RemoteApi* cl, int hexapodNum)
: _cl(cl)
, _handle()
, _refFrameHandle()
, _hexapodNum(hexapodNum+1)
, _isStationary(true)
, _targets({})
, _walkParams({.stepVelocity=0.9, .movementDirection=0, .rotationMode=0, .movementStrength=1})
, _mode(Mode::None)
{
    std::cout << addNumToString("hexapod_", _hexapodNum).c_str() << addNumToString("ReferenceFrame_", _hexapodNum).c_str() << std::endl; 
    _handle = b0RemoteApi::readInt(
        cl->simxGetObjectHandle(addNumToString("hexapod_", _hexapodNum).c_str(), 
                                _cl->simxServiceCall()), 1);

    _refFrameHandle = b0RemoteApi::readInt(
        _cl->simxGetObjectHandle(addNumToString("ReferenceFrame_", _hexapodNum).c_str(), 
                                 _cl->simxServiceCall()), 1);
    // print orientation from Sim
    std::vector<msgpack::object> reply;

    b0RemoteApi::print(_cl->simxGetObjectOrientation(_handle, refFrameHandle, _cl->simxServiceCall()));
}

bool Hexapod::run()
{
    auto pose = getPose();

    bool returnValue = false;
    
    const float distThreshold = 1;
    const float inMovementTurnRation = 0.5;
    const float rotationGain = 6e-3;

    auto adjustHeading = [&] () -> bool
    {
        const float headingThreshold = 5;

        // Adjust heading
        //! @todo check so we turn to "the closest" side. Not to turn 364 degrees
        //! @todo Handle [-180, 180]
        auto headingDiff = wrap180(wrap180(pose.at(2)) + wrap180(_targets.heading));
        //headingDiff -= 90;
        std::cout << "heading: " << pose.at(2) << ", target: " << _targets.heading << ", error:" << headingDiff << std::endl;
        //return;
        if (abs(headingDiff) > headingThreshold)
        {   // We will correct
            float sign = (headingDiff < 0) ? -1 : 1;
            if (_isStationary)
            {
                _walkParams.rotationMode = sign;
                _walkParams.stepVelocity = rotationGain*headingDiff;
            }
            else
            {
                _walkParams.rotationMode = rotationGain*headingDiff*inMovementTurnRation;
            }
            if (_walkParams.rotationMode > 1.0)
            {
                _walkParams.rotationMode = 1.0;
            }
            return false;
        }
        else
        {
            if (_isStationary)
            {
                _walkParams.rotationMode = 0;
                
            }
            return true;
        }
        
        // Adjust position - by walking
        //! @todo
        // if heading is way too off maybe we should turn first
        // Maybe we can adjust the speed depending on error in heading
    };

    auto simpleNavigate = [&]() -> bool
    {
        auto diffX = _targets.x - pose.at(0);
        auto diffY = _targets.y - pose.at(1);
        auto distToTarget = std::sqrt(pow(diffX, 2) + pow(diffY,2));
        _targets.heading = atan2(diffY, diffX)/M_PI*180;

        std::cout << "Dist to target: " << distToTarget << std::endl;
        _walkParams.movementDirection = 0;
        _walkParams.stepVelocity = 1.0;
        _isStationary = false;
        if (!adjustHeading())
        {   // We still need to adjust heading
            std::cout << "still adjusting heading" << std::endl;
            return false;
        }

        if (distToTarget < distThreshold)
        {
            return true;
        }
        return false;
    };

    switch (_mode)
    {
    case Mode::None:
        break;
    case Mode::AdjustHeading:
        returnValue = adjustHeading();
        if (returnValue)
        {
            _walkParams.stepVelocity = 0;
        }
        break;
    case Mode::SimpleNavigate:
        returnValue = simpleNavigate();
        break;
    }
    apply(_walkParams);
    return returnValue;
}

void Hexapod::setPose(float x, float y, float w)
{
    float pos[3] = {x, y, defHeight};
    float orientation[3] = {defAlfa, w, defGamma};
    _cl->simxSetObjectPosition(_handle, refFrameHandle, pos, _cl->simxServiceCall());
    _cl->simxSetObjectOrientation(_handle, refFrameHandle, orientation, _cl->simxServiceCall());
}

std::array<float, 3>  Hexapod::getPose()
{
    std::array<float, 3> pose;
    {
        auto result = _cl->simxGetObjectPosition(_handle, refFrameHandle, _cl->simxServiceCall());
        //b0RemoteApi::print(result);
        auto oArr = result->at(1).as<std::array<float, 3>>();
        pose.at(0) = oArr.at(0);
        pose.at(1) = oArr.at(1);
    }
    /*
    // Pose (Attitude) to find yaw
    //! @todo We have a problem with yaw. It goes up to pi/2 and down again to -pi/2 like a sine wave. Don't know why
    {
        // Euler angles
        auto result = _cl->simxGetObjectOrientation(_handle, refFrameHandle, _cl->simxServiceCall());
        auto oArr = result->at(1).as<std::array<float, 3>>();
        //b0RemoteApi::print(result);
        pose.at(2) = oArr.at(1)/M_PI*180;
        //pose.at(2) = oArr.at(2);
        //std::cout << "heading: " << pose.at(1) << std::endl;
    }
    {
        auto result = _cl->simxGetObjectQuaternion(_handle, refFrameHandle, _cl->simxServiceCall());
        auto oArr = result->at(1).as<std::array<float, 4>>();

        // Just messing about trying to get yaw from Quaternions
        auto mag = sqrt(oArr.at(0)*oArr.at(0) + oArr.at(2)*oArr.at(2));
        auto yaw = 2*acos(oArr.at(0) / mag);
        //auto yaw = atan2(2.0*(q.y*q.z + q.w*q.x), q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);
        //std::cout << "Q1: " << yaw << std::endl;
    }
    */

    // Reference frame (to compare)
    // I added a reference frame om the hexapod to measure instead
    // This seems to work properly
    {
        // Euler angles
        auto result = _cl->simxGetObjectOrientation(_refFrameHandle, refFrameHandle, _cl->simxServiceCall());
        auto oArr = result->at(1).as<std::array<float, 3>>();
        //std::cout << "ref heading: " << oArr.at(2)/M_PI*180 << std::endl;
        pose.at(2) = oArr.at(2)/M_PI*180;
    }
    return pose;
}

void Hexapod::setTargetHeading(float heading)
{
    _targets.heading = heading;
}

void Hexapod::walk(float velocity, float curvature)
{
    _walkParams.stepVelocity = velocity;
    _walkParams.rotationMode = curvature;
    apply(_walkParams);
}

void Hexapod::navigate(float x, float y)
{
    _mode = Mode::SimpleNavigate;
    _targets.x = x;
    _targets.y = y;
}

void Hexapod::stop()
{
    _cl->simxSetFloatSignal("stepAmplitude", 0, _cl->simxServiceCall());
    _isStationary = false;
}

void Hexapod::setMode(Mode mode)
{
    _mode = mode;
}

void Hexapod::apply(WalkParams params)
{
    //std::cout << "movement dir: " << params.movementDirection << std::endl;
    _cl->simxSetFloatSignal(addNumToString("stepVelocity", _hexapodNum).c_str(),
                            params.stepVelocity, 
                            _cl->simxServiceCall());
    _cl->simxSetFloatSignal(addNumToString("movementDirection", _hexapodNum).c_str(),
                            params.movementDirection, 
                            _cl->simxServiceCall());
    _cl->simxSetFloatSignal(addNumToString("rotationMode", _hexapodNum).c_str(),
                            params.rotationMode, 
                            _cl->simxServiceCall());
    _cl->simxSetFloatSignal(addNumToString("movementStrength", _hexapodNum).c_str(),
                            params.movementStrength, 
                            _cl->simxServiceCall());
}

