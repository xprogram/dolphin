// Copyright 2019 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cstdlib>
#include <string>
#include <thread>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "Common/GL/GLInterface/WebGLEmscripten.h"
#include "Core/Config/MainSettings.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "DolphinNoGUI/Platform.h"
#include "VideoCommon/RenderBase.h"

int ExitPlatformWebMainLoopCallback();

namespace
{
class PlatformWeb : public Platform
{
public:
  PlatformWeb(bool headless);

  void SetTitle(const std::string& title) override;
  bool Init() override;
  void MainLoop() override;

  WindowSystemInfo GetWindowSystemInfo() const override;

private:
  bool RequestFullscreen() const;
  bool RequestPointerLock() const;
  void DoMainLoopTick();

  // Technically this selector can potentially refer
  // to another element than intended if somebody is
  // messing around with element IDs in the DOM,
  // but seeing as that shouldn't happen this can
  // be considered a unique handle to a 'window'.
  std::string m_surface_selector;
};

PlatformWeb::PlatformWeb(bool headless)
{
  // TODO
  if (headless)
  {
    m_surface_selector = "!surface";
    return;
  }

  // Make the surface selector overridable without too much hassle
  const char* provided_selector = std::getenv("DOLPHIN_EMU_SURFACE_SELECTOR");
  if (provided_selector)
    m_surface_selector = provided_selector;
  else
    m_surface_selector = "#surface";
}

void PlatformWeb::SetTitle(const std::string& title)
{
  emscripten_set_window_title(title.c_str());
}

bool PlatformWeb::RequestFullscreen() const
{
  EmscriptenFullscreenStrategy fullscreen_strategy;

  // Maintain aspect ratio when going into fullscreen, since
  // without this browsers will tend to squash or stretch the element
  fullscreen_strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;

  // Resize the canvas drawing buffer size while handling high DPI
  fullscreen_strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;

  // Do not differ from default canvas filter behaviour (which should be none)
  fullscreen_strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;

  // Attempt to go fullscreen (will not do anything until the user interacts with the page)
  EMSCRIPTEN_RESULT r = emscripten_request_fullscreen_strategy(m_surface_selector.c_str(), EM_TRUE,
                                                               &fullscreen_strategy);
  return r == EMSCRIPTEN_RESULT_SUCCESS || r == EMSCRIPTEN_RESULT_DEFERRED;
}

bool PlatformWeb::RequestPointerLock() const
{
  EMSCRIPTEN_RESULT r = emscripten_request_pointerlock(m_surface_selector.c_str(), EM_TRUE);
  return r == EMSCRIPTEN_RESULT_SUCCESS || r == EMSCRIPTEN_RESULT_DEFERRED;
}

bool PlatformWeb::Init()
{
  if (SConfig::GetInstance().bHideCursor)
    RequestPointerLock();

  if (Config::Get(Config::MAIN_FULLSCREEN))
    m_window_fullscreen = RequestFullscreen();

  return true;
}

void PlatformWeb::DoMainLoopTick()
{
  if (m_running.IsSet())
  {
    UpdateRunningFlag();
    Core::HostDispatchJobs();

    g_webgl_main_loop_frame_begin_event.Set();
    g_webgl_swap_ready_event.WaitFor(std::chrono::milliseconds(100));

    // When we return from this function, we let the environment process
    // the global event queue, which updates user input, swap buffers, etc.
    return;
  }

  // Clear all DOM callbacks
  emscripten_set_visibilitychange_callback(nullptr, EM_FALSE, nullptr);
  emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE,
                                           nullptr);
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);

  emscripten_cancel_main_loop();

  std::thread([] { emscripten_force_exit(ExitPlatformWebMainLoopCallback()); }).detach();
}

void PlatformWeb::MainLoop()
{
  emscripten_set_visibilitychange_callback(
      this, EM_FALSE,
      [](int ev_type, const EmscriptenVisibilityChangeEvent* visibility_event, void* user_data) {
        static_cast<PlatformWeb*>(user_data)->m_window_focus =
            visibility_event->visibilityState == EMSCRIPTEN_VISIBILITY_VISIBLE;
        return EM_FALSE;
      });

  emscripten_set_fullscreenchange_callback(
      EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_FALSE,
      [](int ev_type, const EmscriptenFullscreenChangeEvent* fullscreen_event, void* user_data) {
        static_cast<PlatformWeb*>(user_data)->m_window_fullscreen =
            fullscreen_event->isFullscreen == EM_TRUE;
        return EM_FALSE;
      });

  emscripten_set_resize_callback(
      EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE,
      [](int ev_type, const EmscriptenUiEvent* ui_event, void* user_data) {
        g_renderer->ResizeSurface();
        return EM_FALSE;
      });

  emscripten_set_main_loop_arg([](void* arg) { static_cast<PlatformWeb*>(arg)->DoMainLoopTick(); },
                               this, 0, EM_TRUE);
}

WindowSystemInfo PlatformWeb::GetWindowSystemInfo() const
{
  WindowSystemInfo wsi;
  wsi.type = WindowSystemType::EmscriptenWeb;
  wsi.display_connection = nullptr;
  wsi.render_surface = (void*)m_surface_selector.c_str();
  return wsi;
}

}  // namespace

std::unique_ptr<Platform> Platform::CreateWebPlatform(bool headless)
{
  return std::make_unique<PlatformWeb>(headless);
}