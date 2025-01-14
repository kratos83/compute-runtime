/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <string>

namespace NEO {
class SettingsReader;
bool checkDefaultCacheDirSettings(std::string &cacheDir, SettingsReader *reader);
} // namespace NEO