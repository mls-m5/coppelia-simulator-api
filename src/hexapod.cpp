#include "hexapod.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace {
    const int refFrameHandle = -1;

    // Default orientations
    const float defHeight = 8.8e-2;
    const float defAlfa = -M_PI/2;
    const float defGamma = -M_PI/2;

}

Hexapod::Hexapod(b0RemoteApi* cl, int handle, int hexapodNum)
: _cl(cl)
, _handle(handle)
, _hexapodNum(hexapodNum)
, _isStationary(true)
{
    // print orientation from Sim
    std::vector<msgpack::object> reply;

    b0RemoteApi::print(_cl->simxGetObjectOrientation(_handle, refFrameHandle, _cl->simxServiceCall()));
}

void Hexapod::setPose(float x, float y, float w)
{
    float pos[3] = {x, y, defHeight};
    float orientation[3] = {defAlfa, w, defGamma};
    _cl->simxSetObjectPosition(_handle, refFrameHandle, pos, _cl->simxServiceCall());
    _cl->simxSetObjectOrientation(_handle, refFrameHandle, orientation, _cl->simxServiceCall());
}

void Hexapod::setTargetHeading(float heading)
{
    // Get current heading
    //auto result = _cl->simxGetObjectOrientation(_handle, refFrameHandle, _cl->simxServiceCall());
}

void Hexapod::stop()
{
    _cl->simxSetFloatSignal("stepAmplitude", 0, _cl->simxServiceCall());
    _isStationary = false;
}

void Hexapod::apply(WalkParams params)
{
    _cl->simxSetFloatSignal("stepVelocity", params.stepVelocity, _cl->simxServiceCall());
    _cl->simxSetFloatSignal("stepAmplitude", params.stepAmplitude, _cl->simxServiceCall());
    _cl->simxSetFloatSignal("movementDirection", params.movementDirection, _cl->simxServiceCall());
    _cl->simxSetFloatSignal("rotationMode", params.rotationMode, _cl->simxServiceCall());
    _cl->simxSetFloatSignal("movementStrength", params.movementStrength, _cl->simxServiceCall());
}
