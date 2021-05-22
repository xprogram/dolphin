// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#define HTML5_SOURCE_NAME "HTML5"

namespace ciface::HTML5
{
void Init();
void PopulateDevices(void* target);
void DeInit();
}  // namespace ciface::HTML5