// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include "InputCommon/ControllerInterface/CoreDevice.h"

namespace ciface::HTML5
{
void SetupKeyboardMouse(const char* target);

class KeyboardMouse final : public Core::Device
{
private:
  struct State
  {
    u8 keyboard[256];
    bool left_click;
    bool right_click;
    bool middle_click;
    float cursor_x;
    float cursor_y;
    float axes[3];
  };

  class Key final : public Input
  {
  public:
    Key(u8 index, const u8& key) : m_key(key), m_index(index) {}
    std::string GetName() const override;
    ControlState GetState() const override;

  private:
    const u8& m_key;
    const u8 m_index;
  };

  class Cursor final : public Input
  {
  public:
    Cursor(u8 index, const float& axis, const bool positive)
        : m_axis(axis), m_index(index), m_positive(positive)
    {
    }
    std::string GetName() const override;
    bool IsDetectable() const override { return false; }
    ControlState GetState() const override;

  private:
    const float& m_axis;
    const u8 m_index;
    const bool m_positive;
  };

  class Axis final : public Input
  {
  public:
    Axis(u8 index, const float& axis, const float range)
        : m_axis(axis), m_range(range), m_index(index)
    {
    }
    std::string GetName() const override;
    ControlState GetState() const override;

  private:
    const float& m_axis;
    const float m_range;
    const u8 m_index;
  };

  class Button final : public Input
  {
  public:
    Button(u8 index, const bool& button) : m_button(button), m_index(index) {}
    std::string GetName() const override;
    ControlState GetState() const override;

  private:
    const bool& m_button;
    const u8 m_index;
  };

public:
  void UpdateInput() override;

  KeyboardMouse(const char* target);
  ~KeyboardMouse();

  std::string GetName() const override;
  std::string GetSource() const override;

private:
  const char* const m_target;
  State m_state_in;
};
}  // namespace ciface::HTML5