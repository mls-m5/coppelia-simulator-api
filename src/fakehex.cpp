
#include "fakehex.h"
#include "matmath/pi.h"
#include "matmath/vec.h"
#include "pose.h"

#include <iostream>
using namespace std;

class FakeHex : public IHexapod {

public:
    FakeHex(Pose pose)
    : _pose(pose) {}

    //! @brief This function is for use before starting the simulation
    //! @param heading yaw in degrees
    void setPose(Pose pose) override {
        _pose = pose;
    }

    Pose getPose() const override {
        return _pose;
    }

    bool run() override {
        constexpr float speed = .1 / 2;

        _pose.angle =
            -((Vecf(_target.x, _target.y) - Vecf(_pose.x, _pose.y)).angle() *
              -pi / 2);

        auto distance = _target - _pose;
        auto d = distance;
        d.normalize();

        if (distance.abs2() > .1) {
            std::tie(_pose.x, _pose.y) = _pose + d * speed * _targetVelocity;
        }

        return false;
    }

    void setTargetHeading(float heading) override {
        _target.angle = heading;
    }

    void navigate(Position target, NavigationMode mode = Rotation) override {
        _target = target;
        cout << "fake navigating hexapod to " << target << endl;
    }

    Pose getTarget() const override {
        return _target;
    }

    float getVelocity() const override {
        return 0; //! @todo return something useful
    }

    bool isAtTarget() const override {
        return (_pose - _target).abs2() < .2;
    };

    void setVelocity(float value) override {
        _targetVelocity = value;
    }

private:
    Pose _pose;
    Pose _target;
    float _targetVelocity = 1;
};

std::unique_ptr<IHexapod> createFakehex(Pose pose) {
    return std::make_unique<FakeHex>(pose);
}
