// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

extern "C" {

// Keyboard & mouse input
extern void HTML5NativeInput_SetupKeyboardMouseDevice(const char* target);
extern void HTML5NativeInput_RemoveKeyboardMouseDevice(void);
extern void HTML5NativeInput_GetKeyboardMouseInputState(float* cursor_x, float* cursor_y,
                                                        int* buttons, float* axes,
                                                        unsigned char* keyboard);
}