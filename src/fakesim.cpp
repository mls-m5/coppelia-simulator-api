
#include "fakesim.h"

#include "fakehex.h"

namespace {

const std::vector<Pose> standardPoses = {
    {{0, 1}},
    {{0, 4}},
    {{2, 1}},
    {{4, 1}},
};
}

namespace {
class FakeSim : public ISimClient {
    std::vector<std::unique_ptr<IHexapod>> createHexapods(
        int numHexapods) override {
        std::vector<std::unique_ptr<IHexapod>> ret;
        ret.reserve(numHexapods);
        for (int i = 0; i < numHexapods; ++i) {
            ret.emplace_back(createFakehex(standardPoses.at(i)));
        }
        return ret;
    }

    //! @brief Stop sim if active
    void stop() override {}

    void start() override {}
};
} // namespace

std::unique_ptr<ISimClient> createFakeClient() {
    return std::make_unique<FakeSim>();
}
