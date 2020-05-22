
#include "b0RemoteApi.h"

class Hexapod {
public:
    enum Mode {
        None,
        AdjustHeading,
        SimpleNavigate,
    };

    Hexapod(b0RemoteApi *cl, int hexapodNum);

    //! @brief This function is for use before starting the simulation
    //! @param heading yaw in degrees
    void setPose(float x, float y, float heading);

    std::array<float, 3> getPose();

    //! @brief This function will run the control loops e.g. to reach target
    //! positions, angles etc
    //! @return true when target has been reached
    bool run();

    Pose getTarget();

    //! @brief The hexapod will rotate until the desired heading is reached
    //!        If we are stationary rotation will be much faster
    //! @param heading in degrees
    void setTargetHeading(float heading);

    //! @brief Walk without any goal
    //! @param velocity Speed of the robot [0, 1+]
    //! @param curvature [0, 1] where 0 will walk straight forward and 1 will
    //! rotate on the spot
    void walk(float velocity, float curvature);

    void navigate(float x, float y);

    void setMode(Mode);

    void stop();

private:
    b0RemoteApi *_cl;
    int _handle;
    int _refFrameHandle;
    int _hexapodNum;
    bool _isStationary;

    struct Targets {
        float heading;
        float x;
        float y;
    } _targets;

    typedef struct {
        float stepVelocity; // Speed of the robot [0, 1+]
        float
            movementDirection; // Translation towards direction in deg. Rotation
                               // and translation are not possible to combine
        float rotationMode; // [0, 1] where 0 will walk straight forward and 1
                            // will rotate on the spot
        float movementStrength; // Acceleration [0, 1]
    } WalkParams;

    WalkParams _walkParams;

    Mode _mode;

    //! @brief send all configurations to Coppelia
    void apply(WalkParams params);
};
