
#include "fakehex.h"
#include "matmath/pi.h"
#include "matmath/vec.h"
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
        constexpr float speed = .1;

        _pose.angle =
            -((Vecf(_target.x, _target.y) - Vecf(_pose.x, _pose.y)).angle() *
                  180 / pi -
              90);

        auto distance = _target - _pose;
        auto d = distance;
        d.normalize();

        if (distance.abs2() > .1) {
            std::tie(_pose.x, _pose.y) = _pose + d * speed;
        }

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
