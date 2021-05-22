// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerInterface/HTML5/HTML5Gamepad.h"

#include <mutex>
#include <string>
#include <unordered_map>

#include <emscripten/html5.h>
#include <emscripten/threading.h>

#include "Common/Event.h"
#include "Common/Flag.h"
#include "Common/Logging/Log.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/ControllerInterface/HTML5/HTML5.h"

namespace ciface::HTML5
{
namespace
{
// Each active Emscripten gamepad index has a valid mapping here
std::unordered_map<long, const Gamepad*> s_index_to_gp;

// This lock must be held whenever handling s_index_to_gp.
// It is recursive because its scope is just a bit annoying...
std::recursive_mutex s_index_to_gp_mutex;

// Controls the lifetime of the gamepad polling loop
Common::Flag s_stop_polling_flag;
Common::Event s_polling_stopped_event;

bool s_gamepads_supported;

EM_BOOL GamepadEventCallback(int ev_type, const EmscriptenGamepadEvent* gamepad_event, void* unused)
{
  if (ev_type == EMSCRIPTEN_EVENT_GAMEPADCONNECTED)
  {
    std::lock_guard lock{s_index_to_gp_mutex};
    auto gamepad = std::make_shared<Gamepad>(*gamepad_event);
    s_index_to_gp[gamepad_event->index] = gamepad.get();
    g_controller_interface.AddDevice(gamepad);
    return EM_TRUE;
  }

  if (ev_type == EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED)
  {
    std::lock_guard lock{s_index_to_gp_mutex};
    if (const auto* target = s_index_to_gp[gamepad_event->index])
    {
      g_controller_interface.RemoveDevice(
          [&target](const auto* device) { return device == target; });
    }
    return EM_TRUE;
  }

  return EM_FALSE;
}

void StartGamepadScanning()
{
  emscripten_set_gamepadconnected_callback(nullptr, EM_FALSE, GamepadEventCallback);
  emscripten_set_gamepaddisconnected_callback(nullptr, EM_FALSE, GamepadEventCallback);

  emscripten_request_animation_frame_loop(
      [](double time, void* unused) {
        if (s_stop_polling_flag.TestAndClear())
        {
          s_polling_stopped_event.Set();
          return EM_FALSE;
        }

        emscripten_sample_gamepad_data();
        return EM_TRUE;
      },
      nullptr);
}
}  // namespace

void InitControllers()
{
  s_gamepads_supported = emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS;
  if (!s_gamepads_supported)
  {
    NOTICE_LOG(SERIALINTERFACE,
               "HTML5 Gamepad API not supported, disabling controller input interface");
    return;
  }

  emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_V, StartGamepadScanning);
}

void ShutdownControllers()
{
  if (!s_gamepads_supported)
    return;

  s_stop_polling_flag.Set();
  s_polling_stopped_event.Wait();
}

// TODO: support haptic feedback once the Emscripten API exposes it
Gamepad::Gamepad(const EmscriptenGamepadEvent& initial_state) : m_state(initial_state)
{
  // setup buttons
  for (int i = 0; i < m_state.numButtons; ++i)
    AddInput(new Button(i, m_state.digitalButton[i]));

  // setup axes
  for (int i = 0; i < m_state.numAxes; ++i)
    AddAnalogInputs(new Axis(i, m_state.axis[i], false), new Axis(i, m_state.axis[i], true));
}

Gamepad::~Gamepad()
{
  std::lock_guard lock{s_index_to_gp_mutex};
  s_index_to_gp[m_state.index] = nullptr;
}

void Gamepad::UpdateInput()
{
  emscripten_get_gamepad_status(m_state.index, &m_state);
}

std::string Gamepad::GetName() const
{
  return m_state.id;
}

std::string Gamepad::GetSource() const
{
  return HTML5_SOURCE_NAME;
}

std::string Gamepad::Button::GetName() const
{
  return "Button " + std::to_string(m_index);
}

std::string Gamepad::Axis::GetName() const
{
  return "Axis " + std::to_string(m_index) + (m_positive ? '+' : '-');
}

ControlState Gamepad::Button::GetState() const
{
  return m_button;
}

ControlState Gamepad::Axis::GetState() const
{
  return m_axis / (m_positive ? 1.0 : -1.0);
}

}  // namespace ciface::HTML5