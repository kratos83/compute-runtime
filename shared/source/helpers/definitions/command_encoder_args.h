/*
 * Copyright (C) 2021-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

namespace NEO {
struct RootDeviceEnvironment;

struct EncodeDummyBlitWaArgs {
    bool isWaRequired = false;
    RootDeviceEnvironment *rootDeviceEnvironment = nullptr;
};

struct MiFlushArgs {
    bool timeStampOperation = false;
    bool commandWithPostSync = false;
    bool notifyEnable = false;
    bool tlbFlush = false;

    EncodeDummyBlitWaArgs &waArgs;
    MiFlushArgs(EncodeDummyBlitWaArgs &args) : waArgs(args) {}
};
} // namespace NEO
