
#include "coppeliasim.h"

#include "b0RemoteApi.h"
#include "hexapod.h"

namespace {

// float simTime = 0.0f;
// int sensorTrigger = 0;
// long lastTimeReceived = 0;
b0RemoteApi *client = nullptr;

// void simulationStepStarted_CB(std::vector<msgpack::object> *msg) {
//    std::map<std::string, msgpack::object> data =
//        msg->at(1).as<std::map<std::string, msgpack::object>>();
//    std::map<std::string, msgpack::object>::iterator it =
//        data.find("simulationTime");
//    if (it != data.end())
//        simTime = it->second.as<float>();
//}

// void proxSensor_CB(std::vector<msgpack::object> *msg) {
//    sensorTrigger = b0RemoteApi::readInt(msg, 1);
//    lastTimeReceived = client->simxGetTimeInMs();
//    printf(".");
//}

void stopSim() {
    if (!client) {
        return;
    }
    // Stop simulation
    std::cout << "Ended!" << std::endl;
    b0RemoteApi::print(client->simxStopSimulation(client->simxServiceCall()));
    // exit(0);
}

class CoppeliaSim : public ISimClient {
public:
    CoppeliaSim()
    : _client("b0RemoteClient", "b0RemoteApi") {
        ::client = &_client;
        stopSim();
        char msg[20] = "testmsg to hexapod";
        client->simxSetStringSignal(
            "test", msg, sizeof(msg), client->simxServiceCall());
    }

    ~CoppeliaSim() {
        stop();
    }

    std::vector<std::unique_ptr<IHexapod>> createHexapods(
        int numHexapods) override {
        std::vector<std::unique_ptr<IHexapod>> hexapods;
        int hexapodNum = 0;
        for (int i = 0; i < numHexapods; i++) {
            hexapods.emplace_back(
                std::make_unique<Hexapod>(&_client, hexapodNum++));
        }
        return hexapods;
    }

    void start() override {
        b0RemoteApi::print(
            client->simxStartSimulation(client->simxServiceCall()));
    }

    void stop() override {
        stopSim();
    }

    b0RemoteApi _client = b0RemoteApi();
};

} // namespace

std::unique_ptr<ISimClient> createCoppeliaClient() {
    return std::make_unique<CoppeliaSim>();
}
