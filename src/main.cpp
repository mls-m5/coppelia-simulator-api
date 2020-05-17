#include "b0RemoteApi.h"

#include "gui.h"
#include "hexapod.h"

namespace {

float simTime = 0.0f;
int sensorTrigger = 0;
long lastTimeReceived = 0;
b0RemoteApi *cl = nullptr;

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
    lastTimeReceived = cl->simxGetTimeInMs();
    printf(".");
}

std::array<int, numHexapods> getHexapodHandles() {
    //! @todo: Get all handles from strings hexapod_[1, numHexapods]

    std::array<int, numHexapods> handles = {};
    handles[0] = b0RemoteApi::readInt(
        cl->simxGetObjectHandle("hexapod_1", cl->simxServiceCall()), 1);
    // handles[1] = b0RemoteApi::readInt(cl->simxGetObjectHandle("hexapod_2",
    // cl->simxServiceCall()), 1);
    return handles;
}

std::vector<Hexapod> createHexapods(std::array<int, numHexapods> handles) {
    std::vector<Hexapod> hexapods;
    int hexapodNum = 0;
    for (int handle : handles) {
        hexapods.push_back(Hexapod(cl, handle, hexapodNum++));
    }
    return hexapods;
}

int main(int argc, char *argv[]) {
    Gui gui(argc, argv);

    std::cout << "program started" << std::endl;
    b0RemoteApi client("b0RemoteClient", "b0RemoteApi");
    cl = &client;

    // Preparing the scene
    auto hexapods = createHexapods(getHexapodHandles());

    hexapods[0].setPose(2, 1, 90);

    // Start the simulation
    b0RemoteApi::print(client.simxStartSimulation(client.simxServiceCall()));

    auto result = client.simxGetObjectOrientation(
        getHexapodHandles()[0], -1, client.simxServiceCall());

    b0RemoteApi::print(result);
    // for (auto o : result)
    //{
    //    std::cout << o << std::endl;
    //}

    char msg[10] = "testmsg";
    client.simxSetStringSignal(
        "test", msg, sizeof(msg), client.simxServiceCall());
    // double floatMsg[2] = {2.0, 1.5};
    // client.simxSetStringSignal("floats", (char*)floatMsg, sizeof(floatMsg),
    // client.simxServiceCall()));
    client.simxSetFloatSignal("speed", 0.2, client.simxServiceCall());

    client.simxSetFloatSignal("rotation", 1, client.simxServiceCall());

    gui.mainLoop();

    // Stop simulation
    b0RemoteApi::print(client.simxStopSimulation(client.simxServiceCall()));
    std::cout << "Ended!" << std::endl;
    return (0);
}
