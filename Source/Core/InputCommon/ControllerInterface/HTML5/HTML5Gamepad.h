// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <string>

#include <emscripten/html5.h>

#include "InputCommon/ControllerInterface/CoreDevice.h"

namespace ciface::HTML5
{
void InitControllers();
void ShutdownControllers();

class Gamepad final : public Core::Device
{
private:
  class Button final : public Input
  {
  public:
    std::string GetName() const override;
    Button(u8 index, const EM_BOOL& button) : m_button(button), m_index(index) {}
    ControlState GetState() const override;

  private:
    const EM_BOOL& m_button;
    const u8 m_index;
  };

  class Axis final : public Input
  {
  public:
    std::string GetName() const override;
    Axis(u8 index, const double& axis, const bool positive)
        : m_axis(axis), m_index(index), m_positive(positive)
    {
    }
    ControlState GetState() const override;

  private:
    const double& m_axis;
    const u8 m_index;
    const bool m_positive;
  };

public:
  void UpdateInput() override;

  Gamepad(const EmscriptenGamepadEvent& initial_state);
  ~Gamepad();

  std::string GetName() const override;
  std::string GetSource() const override;

private:
  EmscriptenGamepadEvent m_state;
};
}  // namespace ciface::HTML5