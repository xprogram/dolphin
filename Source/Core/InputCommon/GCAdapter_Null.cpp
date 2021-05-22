// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <functional>

#include "InputCommon/GCAdapter.h"
#include "InputCommon/GCPadStatus.h"

namespace GCAdapter
{
void Init()
{
}
void ResetRumble()
{
}
void Shutdown()
{
}
void SetAdapterCallback(std::function<void(void)> func)
{
}
void StartScanThread()
{
}
void StopScanThread()
{
}
GCPadStatus Input(int chan)
{
  return {};
}
void Output(int chan, u8 rumble_command)
{
}
bool IsDetected(const char** error_message)
{
  return false;
}
bool DeviceConnected(int chan)
{
  return false;
}
void ResetDeviceType(int chan)
{
}
bool UseAdapter()
{
  return false;
}

}  // end of namespace GCAdapter
