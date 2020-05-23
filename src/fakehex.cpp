
#include "fakehex.h"

#include "pose.h"

class FakeHex : public IHexapod {

public:
    FakeHex(Pose pose)
    : _pose(pose) {}

    //! @brief This function is for use before starting the simulation
    //! @param heading yaw in degrees
    void setPose(float x, float y, float heading) override {
        _pose = Pose{{x, y}, heading};
    }

    Pose getPose() const override {
        return _pose;
    }

    bool run() override {
        return false;
    }

    void setTargetHeading(float heading) override {
        _target.angle = heading;
    }

    void navigate(float x, float y) override {
        _target.x = x;
        _target.y = y;
    }

    Pose getTarget() const override {
        return _target;
    }

private:
    Pose _pose;
    Pose _target;
};

std::unique_ptr<IHexapod> createFakehex(Pose pose) {
    return std::make_unique<FakeHex>(pose);
}
