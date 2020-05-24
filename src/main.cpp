#include "b0RemoteApi.h"

#include "calculateprojections.h"
#include "coppeliasim.h"
#include "fakesim.h"
#include "gui.h"
#include "hexapod.h"
#include "loadpaths.h"
#include "organizer.h"
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

    std::vector<IHexapod *> rawHexapods(numHexapods);
    std::transform(hexapods.begin(),
                   hexapods.end(),
                   rawHexapods.begin(),
                   [](std::unique_ptr<IHexapod> &p) { return p.get(); });

    auto paths = loadPaths("data/paths.txt");

    for (int i = 0; i < numHexapods; ++i) {
        auto &hex = hexapods.at(i);
        auto &path = paths.at(i);

        hex->setPose(path.front());
        path.erase(path.begin(), path.begin() + 1);
    }

    Organizer organizer(std::move(rawHexapods), paths);

    // Change heading
    hexapods.at(0)->setTargetHeading(75);
    std::cout << "done" << std::endl;

    client->start();

    //    const int floorSize = 5;
    //    auto dist =
    //        std::uniform_real_distribution<float>(-floorSize / 2, floorSize /
    //        2);
    //    for (auto &hexapod : hexapods) {
    //        Position target = {
    //            dist(randomEngine),
    //            dist(randomEngine),
    //        };
    //        hexapod->navigate(target, IHexapod::NavigationMode::Rotation);
    //    }

    bool abort = false;

    typedef std::array<float, 3> ColorArr;
    std::array<ColorArr, 4> hexapodColors;
    hexapodColors[0] = {0.56, 0, 0};
    hexapodColors[1] = {1, 1, 0.23};
    hexapodColors[2] = {0.5, 0.5, 1};
    hexapodColors[3] = {1, 0.6, 0.2};
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
                    i,
                    {hexapod->getPose(),
                     hexapod->getTarget(),
                     calculateProjections(organizer.getProjection(i),
                                          0,
                                          hexapod->getPose(),
                                          10,
                                          .3),
                     hexapodColors[i]});
            }
            jobLeft = !someNotDone;
            this_thread::sleep_for(.1s);
        }
    });

    gui.setPaths(paths);

    gui.mainLoop();

    abort = true;

    jobThread.join();

    client->stop();

    return (0);
}
