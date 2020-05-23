#include "b0RemoteApi.h"

#include "gui.h"
#include "hexapod.h"
#include "loadpaths.h"
#include "paths.h"
#include <csignal>
#include <random>
#include <thread>

namespace {

float simTime = 0.0f;
int sensorTrigger = 0;
long lastTimeReceived = 0;
std::unique_ptr<b0RemoteApi> client;

const int numHexapods = 4;

std::random_device dev;

std::mt19937 randomEngine(dev());

} // namespace

void simulationStepStarted_CB(std::vector<msgpack::object> *msg) {
    std::map<std::string, msgpack::object> data =
        msg->at(1).as<std::map<std::string, msgpack::object>>();
    std::map<std::string, msgpack::object>::iterator it =
        data.find("simulationTime");
    if (it != data.end())
        simTime = it->second.as<float>();
}

void proxSensor_CB(std::vector<msgpack::object> *msg) {
    sensorTrigger = b0RemoteApi::readInt(msg, 1);
    lastTimeReceived = client->simxGetTimeInMs();
    printf(".");
}

std::vector<std::unique_ptr<Hexapod>> createHexapods(int numHexapods) {
    std::vector<std::unique_ptr<Hexapod>> hexapods;
    int hexapodNum = 0;
    for (int i = 0; i < numHexapods; i++) {
        hexapods.push_back(
            std::make_unique<Hexapod>(client.get(), hexapodNum++));
    }
    return hexapods;
}

int main(int argc, char *argv[]) {
    Gui gui(argc, argv);

    auto stopSim = []() {
        // Stop simulation
        std::cout << "Ended!" << std::endl;
        b0RemoteApi::print(
            client->simxStopSimulation(client->simxServiceCall()));
        // exit(0);
    };

    std::atexit(stopSim);
    // std::signal(SIGINT, SIG_DFL);
    // std::signal(SIGABRT, stopSim);
    // std::signal(SIGTERM, stopSim);
    // std::signal(SIGTSTP, stopSim);

    std::cout << "program started" << std::endl;
    client = std::make_unique<b0RemoteApi>("b0RemoteClient", "b0RemoteApi");

    stopSim(); // Start with stopping the sim, if any active

    // Preparing the scene
    auto hexapods = createHexapods(numHexapods);

    hexapods.at(0)->setPose(2, 1, 90);

    // Start the simulation
    b0RemoteApi::print(client->simxStartSimulation(client->simxServiceCall()));

    char msg[20] = "testmsg to hexapod";
    client->simxSetStringSignal(
        "test", msg, sizeof(msg), client->simxServiceCall());

    // Change heading
    hexapods.at(0)->setTargetHeading(75);
    std::cout << "done" << std::endl;

    const int floorSize = 5;
    auto dist =
        std::uniform_real_distribution<float>(-floorSize / 2, floorSize / 2);
    for (auto &hexapod : hexapods) {
        float x = dist(randomEngine);
        float y = dist(randomEngine);
        hexapod->navigate(x, y);
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
        }
    });

    auto paths = loadPaths("data/paths.txt");
    gui.setPaths(paths);

    gui.mainLoop();

    abort = true;

    jobThread.join();

    return (0);
}
