
#include "b0RemoteApi.h"

class Hexapod 
{
public:
    Hexapod(b0RemoteApi* cl, int handle, int hexapodNum);

    //! @brief This function is for use before starting the simulation
    //! @param heading yaw in degrees
    void setPose(float x, float y, float heading);

    //! @brief This function will run the control loops e.g. to reach target positions, angles etc
    void run();

    //! @brief The hexapod will rotate until the desired heading is reached
    //!        If we are stationary rotation will be much faster
    //! @param heading in degrees
    void setTargetHeading(float heading);

    void stop(); 

private:

    b0RemoteApi* _cl;
    int _handle;
    int _hexapodNum;
    bool _isStationary;

    struct WalkParams {
        float stepVelocity;        // Speed of the robot [0, 1+]
        float stepAmplitude;       // Size of the steps [0, 1+]
        float movementDirection;   // Translation towards direction in deg. Rotation and translation are not possible to combine
        float rotationMode;        // [0, 1] where 0 will walk straight forward and 1 will rotate on the spot
        float movementStrength;    // Acceleration [0, 1]
    } _walkParams;

    //! @brief send all configurations to Coppelia
    void apply(WalkParams params);
};