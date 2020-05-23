#include "b0RemoteApi.h"

#include "coppeliasim.h"
#include "fakesim.h"
#include "gui.h"
#include "hexapod.h"
#include "loadpaths.h"
#include "paths.h"
#include <csignal>
#include <random>
#include <thread>

namespace {

const int numHexapods = 4;

std::random_device dev;

std::mt19937 randomEngine(dev());

} // namespace

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace chrono_literals;

    bool fake = false;
    for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--fake") {
            fake = true;
        }
        else if (arg == "--help") {
            cout << "--help for help, --fake to run fake simulation" << endl;
            return 0;
        }
    }

    Gui gui(argc, argv);

    cout << "program started" << endl;

    std::unique_ptr<ISimClient> client;
    if (fake) {
        client = createFakeClient();
        cout << "Using fake client" << endl;
    }
    else {
        client = createCoppeliaClient();
    }

    client->stop();

    // Preparing the scene
    auto hexapods = client->createHexapods(numHexapods);

    hexapods.at(0)->setPose(2, 1, 90);

    // Change heading
    hexapods.at(0)->setTargetHeading(75);
    std::cout << "done" << std::endl;

    client->start();

    const int floorSize = 5;
    auto dist =
        std::uniform_real_distribution<float>(-floorSize / 2, floorSize / 2);
    for (auto &hexapod : hexapods) {
        float x = dist(randomEngine);
        float y = dist(randomEngine);
        hexapod->navigate(x, y, IHexapod::NavigationMode::Rotation);
    }

    bool abort = false;

    // Finish all jobs
    auto jobThread = std::thread([&]() {
        while (bool jobLeft = true && !abort) {
            bool someNotDone = false;
            for (size_t i = 0; i < hexapods.size(); ++i) {
                auto &hexapod = hexapods.at(i);
                if (!hexapod->run()) {
                    someNotDone = true;
                }

                gui.setHexapodInformation(
                    i, {hexapod->getPose(), hexapod->getTarget()});
            }
            jobLeft = !someNotDone;
            this_thread::sleep_for(.1s);
        }
    });

    auto paths = loadPaths("data/paths.txt");
    gui.setPaths(paths);

    gui.mainLoop();

    abort = true;

    jobThread.join();

    client->stop();

    return (0);
}
