// Copyright 2019 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerInterface/HTML5/HTML5KeyboardMouse.h"

#include <cstring>
#include <string>

#include <emscripten/key_codes.h>

#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/ControllerInterface/HTML5/HTML5.h"
#include "InputCommon/ControllerInterface/HTML5/HTML5NativeInput.h"

namespace ciface::HTML5
{
constexpr struct
{
  const int code;
  const char* const name;
} named_keys[] = {{DOM_VK_1, "1"},
                  {DOM_VK_2, "2"},
                  {DOM_VK_3, "3"},
                  {DOM_VK_4, "4"},
                  {DOM_VK_5, "5"},
                  {DOM_VK_6, "6"},
                  {DOM_VK_7, "7"},
                  {DOM_VK_8, "8"},
                  {DOM_VK_9, "9"},
                  {DOM_VK_0, "0"},
                  {DOM_VK_A, "A"},
                  {DOM_VK_B, "B"},
                  {DOM_VK_C, "C"},
                  {DOM_VK_D, "D"},
                  {DOM_VK_E, "E"},
                  {DOM_VK_F, "F"},
                  {DOM_VK_G, "G"},
                  {DOM_VK_H, "H"},
                  {DOM_VK_I, "I"},
                  {DOM_VK_J, "J"},
                  {DOM_VK_K, "K"},
                  {DOM_VK_L, "L"},
                  {DOM_VK_M, "M"},
                  {DOM_VK_N, "N"},
                  {DOM_VK_O, "O"},
                  {DOM_VK_P, "P"},
                  {DOM_VK_Q, "Q"},
                  {DOM_VK_R, "R"},
                  {DOM_VK_S, "S"},
                  {DOM_VK_T, "T"},
                  {DOM_VK_U, "U"},
                  {DOM_VK_V, "V"},
                  {DOM_VK_W, "W"},
                  {DOM_VK_X, "X"},
                  {DOM_VK_Y, "Y"},
                  {DOM_VK_Z, "Z"},
                  {DOM_VK_F1, "F1"},
                  {DOM_VK_F2, "F2"},
                  {DOM_VK_F3, "F3"},
                  {DOM_VK_F4, "F4"},
                  {DOM_VK_F5, "F5"},
                  {DOM_VK_F6, "F6"},
                  {DOM_VK_F7, "F7"},
                  {DOM_VK_F8, "F8"},
                  {DOM_VK_F9, "F9"},
                  {DOM_VK_F10, "F10"},
                  {DOM_VK_F11, "F11"},
                  {DOM_VK_F12, "F12"},
                  {DOM_VK_F13, "F13"},
                  {DOM_VK_F14, "F14"},
                  {DOM_VK_F15, "F15"},
                  {DOM_VK_F16, "F16"},
                  {DOM_VK_F17, "F17"},
                  {DOM_VK_F18, "F18"},
                  {DOM_VK_F19, "F19"},
                  {DOM_VK_F20, "F20"},
                  {DOM_VK_F21, "F21"},
                  {DOM_VK_F22, "F22"},
                  {DOM_VK_F23, "F23"},
                  {DOM_VK_F24, "F24"},
                  {DOM_VK_SPACE, "Space"},
                  {DOM_VK_TAB, "Tab"},
                  {DOM_VK_DIVIDE, "Keypad /"},
                  {DOM_VK_MULTIPLY, "Keypad *"},
                  {DOM_VK_SUBTRACT, "Keypad -"},
                  {DOM_VK_ADD, "Keypad +"},
                  {DOM_VK_NUMPAD1, "Keypad 1"},
                  {DOM_VK_NUMPAD2, "Keypad 2"},
                  {DOM_VK_NUMPAD3, "Keypad 3"},
                  {DOM_VK_NUMPAD4, "Keypad 4"},
                  {DOM_VK_NUMPAD5, "Keypad 5"},
                  {DOM_VK_NUMPAD6, "Keypad 6"},
                  {DOM_VK_NUMPAD7, "Keypad 7"},
                  {DOM_VK_NUMPAD8, "Keypad 8"},
                  {DOM_VK_NUMPAD9, "Keypad 9"},
                  {DOM_VK_NUMPAD0, "Keypad 0"},
                  {DOM_VK_HOME, "Home"},
                  {DOM_VK_RIGHT, "Right Arrow"},
                  {DOM_VK_LEFT, "Left Arrow"},
                  {DOM_VK_DOWN, "Down Arrow"},
                  {DOM_VK_UP, "Up Arrow"},
                  {DOM_VK_SEMICOLON, ";"},
                  {DOM_VK_COMMA, ","},
                  {DOM_VK_PERIOD, "."},
                  {DOM_VK_SLASH, "/"},
                  {DOM_VK_ESCAPE, "Escape"},
                  {DOM_VK_QUOTE, "'"},
                  {DOM_VK_BACK_QUOTE, "Tilde"},
                  {DOM_VK_BACK_SLASH, "\\"},
                  {DOM_VK_META, "Meta"},  // Windows key, Command key, etc.
                  {DOM_VK_CONTROL, "Left Control"},
                  {DOM_VK_SHIFT, "Left Shift"},
                  {DOM_VK_ALT, "Left Alt"},
                  {DOM_VK_CAPS_LOCK, "Caps Lock"}};

// Sensitivity of axis input for normalized cursor X/Y
constexpr float NORM_MOUSE_AXIS_SENSITIVITY = 0.04f;

// Prevent duplicate keyboard/mouse devices.
static bool s_keyboard_mouse_exists = false;

enum class MouseEventButton : int
{
  Left = 1 << 0,
  Right = 1 << 1,
  Middle = 1 << 2
};

void SetupKeyboardMouse(const char* target)
{
  if (s_keyboard_mouse_exists)
    return;

  g_controller_interface.AddDevice(std::make_shared<KeyboardMouse>(target));
  s_keyboard_mouse_exists = true;
}

KeyboardMouse::~KeyboardMouse()
{
  HTML5NativeInput_RemoveKeyboardMouseDevice();
  s_keyboard_mouse_exists = false;
}

KeyboardMouse::KeyboardMouse(const char* target) : m_target(target)
{
  HTML5NativeInput_SetupKeyboardMouseDevice(m_target);

  // Add keys
  for (u8 i = 0; i < sizeof(named_keys) / sizeof(*named_keys); ++i)
    AddInput(new Key(i, m_state_in.keyboard[named_keys[i].code]));

  // Add mouse buttons
  AddInput(new Button(0, m_state_in.left_click));
  AddInput(new Button(1, m_state_in.right_click));
  AddInput(new Button(2, m_state_in.middle_click));

  // Setup the cursor
  AddInput(new Cursor(0, m_state_in.cursor_x, false));
  AddInput(new Cursor(0, m_state_in.cursor_x, true));
  AddInput(new Cursor(1, m_state_in.cursor_y, false));
  AddInput(new Cursor(1, m_state_in.cursor_y, true));

  // Setup the axes for the cursor
  for (u8 i = 0; i < 4; ++i)
  {
    AddInput(new Axis(i / 2, m_state_in.axes[i / 2],
                      i % 2 ? NORM_MOUSE_AXIS_SENSITIVITY : -NORM_MOUSE_AXIS_SENSITIVITY));
  }

  // Setup the scrollwheel axes
  AddInput(new Axis(2, m_state_in.axes[2], -1));
  AddInput(new Axis(2, m_state_in.axes[2], 1));
}

void KeyboardMouse::UpdateInput()
{
  int buttons;

  HTML5NativeInput_GetKeyboardMouseInputState(&m_state_in.cursor_x, &m_state_in.cursor_y, &buttons,
                                              m_state_in.axes, m_state_in.keyboard);

  m_state_in.left_click = buttons & static_cast<int>(MouseEventButton::Left);
  m_state_in.right_click = buttons & static_cast<int>(MouseEventButton::Right);
  m_state_in.middle_click = buttons & static_cast<int>(MouseEventButton::Middle);
}

std::string KeyboardMouse::GetName() const
{
  return "Keyboard Mouse";
}

std::string KeyboardMouse::GetSource() const
{
  return HTML5_SOURCE_NAME;
}

// names
std::string KeyboardMouse::Key::GetName() const
{
  return named_keys[m_index].name;
}

std::string KeyboardMouse::Button::GetName() const
{
  return "Click " + std::to_string(m_index);
}

std::string KeyboardMouse::Axis::GetName() const
{
  static char tmpstr[] = "Axis ..";
  tmpstr[5] = char('X' + m_index);
  tmpstr[6] = (m_range < 0 ? '-' : '+');
  return tmpstr;
}

std::string KeyboardMouse::Cursor::GetName() const
{
  static char tmpstr[] = "Cursor ..";
  tmpstr[7] = char('X' + m_index);
  tmpstr[8] = (m_positive ? '+' : '-');
  return tmpstr;
}

// get state
ControlState KeyboardMouse::Key::GetState() const
{
  return m_key;
}

ControlState KeyboardMouse::Button::GetState() const
{
  return m_button;
}

ControlState KeyboardMouse::Axis::GetState() const
{
  return ControlState(m_axis) / m_range;
}

ControlState KeyboardMouse::Cursor::GetState() const
{
  return m_axis / (m_positive ? 1.0 : -1.0);
}
}  // namespace ciface::HTML5