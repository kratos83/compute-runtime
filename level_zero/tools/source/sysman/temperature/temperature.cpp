/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/helpers/basic_math.h"

#include "level_zero/tools/source/sysman/os_sysman.h"
#include "level_zero/tools/source/sysman/temperature/temperature_imp.h"

namespace L0 {

TemperatureHandleContext::~TemperatureHandleContext() {}

void TemperatureHandleContext::createHandle(const ze_device_handle_t &deviceHandle, zes_temp_sensors_t type) {
    std::unique_ptr<Temperature> pTemperature = std::make_unique<TemperatureImp>(deviceHandle, pOsSysman, type);
    if (pTemperature->initSuccess == true) {
        handleList.push_back(std::move(pTemperature));
    }
}

void TemperatureHandleContext::init(std::vector<ze_device_handle_t> &deviceHandles) {
    for (const auto &deviceHandle : deviceHandles) {
        createHandle(deviceHandle, ZES_TEMP_SENSORS_GLOBAL);
        createHandle(deviceHandle, ZES_TEMP_SENSORS_GPU);
        createHandle(deviceHandle, ZES_TEMP_SENSORS_MEMORY);
    }
}

ze_result_t TemperatureHandleContext::temperatureGet(uint32_t *pCount, zes_temp_handle_t *phTemperature) {
    std::call_once(initTemperatureOnce, [this]() {
        this->init(pOsSysman->getDeviceHandles());
    });
    uint32_t handleListSize = static_cast<uint32_t>(handleList.size());
    uint32_t numToCopy = std::min(*pCount, handleListSize);
    if (0 == *pCount || *pCount > handleListSize) {
        *pCount = handleListSize;
    }
    if (nullptr != phTemperature) {
        for (uint32_t i = 0; i < numToCopy; i++) {
            phTemperature[i] = handleList[i]->toHandle();
        }
    }
    return ZE_RESULT_SUCCESS;
}

} // namespace L0
