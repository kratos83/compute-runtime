/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

namespace NEO {
template <>
uint64_t CompilerProductHelperHw<IGFX_METEORLAKE>::getHwInfoConfig(const HardwareInfo &hwInfo) const {
    return 0x0;
}

} // namespace NEO
