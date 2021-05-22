// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <emscripten/html5_webgl.h>

#include "Common/Event.h"
#include "Common/GL/GLContext.h"

extern Common::Event g_webgl_main_loop_frame_begin_event;
extern Common::Event g_webgl_swap_ready_event;

class GLContextWebGLEmscripten final : public GLContext
{
public:
  ~GLContextWebGLEmscripten() override;

  bool IsHeadless() const override;

  bool MakeCurrent() override;
  bool ClearCurrent() override;

  void Update() override;

  void Swap() override;

  void* GetFuncAddress(const std::string& name) override;

protected:
  bool Initialize(const WindowSystemInfo& wsi, bool stereo, bool core) override;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE m_context;
  const char* m_canvas_target;
};