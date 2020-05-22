#include "b0RemoteApi.h"

#include "gui.h"
#include "hexapod.h"
#include <csignal>
#include <cstdlib>
#include <thread>
#include <unistd.h>

namespace {

float simTime = 0.0f;
int sensorTrigger = 0;
long lastTimeReceived = 0;
std::unique_ptr<b0RemoteApi> client;

const int numHexapods = 1;

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

std::vector<Hexapod *> createHexapods(int numHexapods) {
    std::vector<Hexapod *> hexapods;
    int hexapodNum = 0;
    for (int i = 0; i < numHexapods; i++) {
        hexapods.push_back(new Hexapod(client.get(), hexapodNum++));
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
    /*
    while(hexapods[0].run())
    {
        usleep(100);
    }
    hexapods[0].setTargetHeading(-75);
    while(hexapods[0].run())
    {
        usleep(100);
    }

    hexapods[0].walk(0.2, 0.0);
    hexapods[1].walk(0.5, 0.3);
    sleep(5);
    */

    std::cout << "done" << std::endl;

    for (auto hexapod : hexapods) {
        hexapod->navigate(0, 0);
    }

    bool abort = false;

    // Finish all jobs
    auto jobThread = std::thread([&]() {
        while (bool jobLeft = true && !abort) {
            bool someNotDone = false;
            for (auto hexapod : hexapods) {
                if (!hexapod->run()) {
                    someNotDone = true;
                }
            }
            jobLeft = !someNotDone;
        }
    });

    gui.mainLoop();

    abort = true;

    jobThread.join();

    return (0);
}
