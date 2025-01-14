/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/os_interface/windows/driver_info_windows.h"

#include "shared/source/os_interface/windows/debug_registry_reader.h"
#include "shared/source/os_interface/windows/sys_calls.h"

#include <algorithm>

std::string getCurrentLibraryPath() {
    std::string returnValue;
    WCHAR pathW[MAX_PATH];
    char path[MAX_PATH];
    HMODULE handle = NULL;

    auto status = NEO::SysCalls::getModuleHandle(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                                     GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                                 reinterpret_cast<LPCWSTR>(&getCurrentLibraryPath), &handle);
    if (status != 0) {

        status = NEO::SysCalls::getModuleFileName(handle, pathW, MAX_PATH);
        if (status != 0) {
            std::wcstombs(path, pathW, MAX_PATH);
            returnValue.append(path);
        }
    }
    return returnValue;
}

namespace NEO {
DriverInfoWindows::DriverInfoWindows(const std::string &fullPath, const PhysicalDevicePciBusInfo &pciBusInfo)
    : DriverInfo(DriverInfoType::WINDOWS), path(DriverInfoWindows::trimRegistryKey(fullPath)), registryReader(createRegistryReaderFunc(path)) {
    this->pciBusInfo = pciBusInfo;
}

DriverInfoWindows::~DriverInfoWindows() = default;

std::string DriverInfoWindows::trimRegistryKey(std::string path) {
    std::string prefix("\\REGISTRY\\MACHINE\\");
    auto pos = prefix.find(prefix);
    if (pos != std::string::npos)
        path.erase(pos, prefix.length());

    return path;
}

std::string DriverInfoWindows::getDeviceName(std::string defaultName) {
    return registryReader->getSetting("HardwareInformation.AdapterString", defaultName);
}

std::string DriverInfoWindows::getVersion(std::string defaultVersion) {
    return registryReader->getSetting("DriverVersion", defaultVersion);
};

bool DriverInfoWindows::isCompatibleDriverStore() const {
    auto toLowerAndUnifyDriverStore = [](std::string &input) -> std::string {
        std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) { return std::tolower(c); });
        auto hostDriverStorePos = input.find("\\hostdriverstore\\");
        if (hostDriverStorePos != std::string::npos) {
            input.erase(hostDriverStorePos + 1, 4);
        }
        return input;
    };
    auto currentLibraryPath = toLowerAndUnifyDriverStore(getCurrentLibraryPath());
    auto openclDriverName = registryReader->getSetting("OpenCLDriverName", std::string{});
    if (openclDriverName.empty()) {
        return false;
    }

    auto driverStorePath = toLowerAndUnifyDriverStore(registryReader->getSetting("DriverStorePathForComputeRuntime", currentLibraryPath));
    return currentLibraryPath.find(driverStorePath.c_str()) == 0u;
}

bool isCompatibleDriverStore(std::string &&deviceRegistryPath) {
    DriverInfoWindows driverInfo(deviceRegistryPath, PhysicalDevicePciBusInfo(PhysicalDevicePciBusInfo::invalidValue, PhysicalDevicePciBusInfo::invalidValue, PhysicalDevicePciBusInfo::invalidValue, PhysicalDevicePciBusInfo::invalidValue));
    return driverInfo.isCompatibleDriverStore();
}

bool DriverInfoWindows::containsSetting(const char *setting) {
    return registryReader->getSetting(setting, std::string("")) != "<>";
}

decltype(DriverInfoWindows::createRegistryReaderFunc) DriverInfoWindows::createRegistryReaderFunc = [](const std::string &registryPath) -> std::unique_ptr<SettingsReader> {
    return std::make_unique<RegistryReader>(false, registryPath);
};
} // namespace NEO
