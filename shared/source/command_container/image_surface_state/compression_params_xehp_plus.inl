/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/gmm_helper/resource_info.h"

#include "gmm_client_context.h"

namespace NEO {

template <typename Family>
void EncodeSurfaceState<Family>::appendImageCompressionParams(R_SURFACE_STATE *surfaceState, GraphicsAllocation *allocation, GmmHelper *gmmHelper, bool imageFromBuffer) {
    const auto ccsMode = R_SURFACE_STATE::AUXILIARY_SURFACE_MODE::AUXILIARY_SURFACE_MODE_AUX_CCS_E;
    if ((ccsMode == surfaceState->getAuxiliarySurfaceMode() || surfaceState->getMemoryCompressionEnable())) {
        uint8_t compressionFormat;
        auto gmmResourceInfo = allocation->getDefaultGmm()->gmmResourceInfo.get();
        if (gmmResourceInfo->getResourceFlags()->Info.MediaCompressed) {
            compressionFormat = gmmHelper->getClientContext()->getMediaSurfaceStateCompressionFormat(gmmResourceInfo->getResourceFormat());
        } else {
            compressionFormat = gmmHelper->getClientContext()->getSurfaceStateCompressionFormat(gmmResourceInfo->getResourceFormat());
        }

        if (imageFromBuffer) {
            if (DebugManager.flags.ForceBufferCompressionFormat.get() != -1) {
                compressionFormat = DebugManager.flags.ForceBufferCompressionFormat.get();
            }
            appendParamsForImageFromBuffer(surfaceState);
        }

        surfaceState->setCompressionFormat(compressionFormat);
    }
}
} // namespace NEO
