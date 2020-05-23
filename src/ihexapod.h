#pragma once

#include "pose.h"

class IHexapod {
public:
    //! @brief This function is for use before starting the simulation
    //! @param heading yaw in degrees
    virtual void setPose(float x, float y, float heading) = 0;

    virtual Pose getPose() const = 0;

    //! @brief This function will run the control loops e.g. to reach target
    //! positions, angles etc
    //! @return true when target has been reached
    virtual bool run() = 0;

    //! @brief The hexapod will rotate until the desired heading is reached
    //!        If we are stationary rotation will be much faster
    //! @param heading in degrees
    virtual void setTargetHeading(float heading) = 0;

    virtual void navigate(float x, float y) = 0;

    virtual Pose getTarget() const = 0;
};
