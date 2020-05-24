
#include "b0RemoteApi.h"
#include "ihexapod.h"
#include "matmath/vec.h"
#include "pose.h"

class Target {
public:
    Target(b0RemoteApi *cl, int targetNum);

    void setPos(Position);

    std::array<float, 2> getPos();

private:
    b0RemoteApi *_cl;
    Vecf _pos;
    int _handle;
};

class Hexapod : public IHexapod {
public:
    Hexapod(b0RemoteApi *cl, int hexapodNum);

    //! @brief This function is for use before starting the simulation
    //! @param heading yaw in degrees
    void setPose(Pose pose) override;

    Pose getPose() const override;

    //! @brief This function will run the control loops e.g. to reach target
    //! positions, angles etc
    //! @return true when target has been reached
    bool run() override;

    //! @brief The hexapod will rotate until the desired heading is reached
    //!        If we are stationary rotation will be much faster
    //! @param heading in degrees
    void setTargetHeading(float heading) override;

    void navigate(Position position, NavigationMode mode = Rotation) override;

    Pose getTarget() const override;

    bool isAtTarget() const;

    float getVelocity() const override;

private:
    enum Mode {
        None,
        AdjustHeading,
        SimpleNavigate,
        Translate,
    };

    //! @brief Walk without any goal
    //! @param velocity Speed of the robot [0, 1+]
    //! @param curvature [0, 1] where 0 will walk straight forward and 1 will
    //! rotate on the spot
    void walk(float velocity, float curvature);

    void stop();

    void setMode(Mode);

    void updateTargetPos();

    b0RemoteApi *_cl; // Non owning pointer to api
    int _handle;
    int _addedCoordSysHandle;
    int _hexapodNum;

    Pose _targets;

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

    Target _target;

    struct {
        float stepVelocity;
        float movementDirection;
        float rotationMode;
        float movementStrength;
    } _lastCoppeliaCalls;

    NavigationMode _NavMode;

    //! @brief send all configurations to Coppelia
    void apply(WalkParams params);

    void updateVelocity();

    struct {
        float lastX;
        float lastY;
        long lastMs; // time in ms
        float velocity;
    } _velocityData;
};
