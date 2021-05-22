// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerInterface/HTML5/HTML5.h"
#include "InputCommon/ControllerInterface/HTML5/HTML5Gamepad.h"
#include "InputCommon/ControllerInterface/HTML5/HTML5KeyboardMouse.h"

namespace ciface::HTML5
{
// TODO: implement touchscreen
void Init()
{
  InitControllers();
}

void PopulateDevices(void* target)
{
  const char* elem_target = static_cast<const char*>(target);

  if (!elem_target)
    return;

  SetupKeyboardMouse(elem_target);
}

void DeInit()
{
  ShutdownControllers();
}
}  // namespace ciface::HTML5