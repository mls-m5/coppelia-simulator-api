
#pragma once

#include "ihexapod.h"
#include <memory>
#include <vector>

class ISimClient {
public:
    virtual std::vector<std::unique_ptr<IHexapod>> createHexapods(
        int numHexapods) = 0;

    //! @brief Stop sim if active
    virtual void stop() = 0;

    virtual void start() = 0;
};
