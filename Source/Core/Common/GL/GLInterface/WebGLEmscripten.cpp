// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Common/GL/GLInterface/WebGLEmscripten.h"

#include <chrono>

#include <emscripten/emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/threading.h>

#include "Common/Assert.h"
#include "Common/Logging/Log.h"
#include "Common/MsgHandler.h"
#include "Common/WebAdapter.h"

Common::Event g_webgl_main_loop_frame_begin_event;
Common::Event g_webgl_swap_ready_event;

GLContextWebGLEmscripten::~GLContextWebGLEmscripten()
{
  if (m_context > 0)
    emscripten_webgl_destroy_context(m_context);
}

bool GLContextWebGLEmscripten::IsHeadless() const
{
  // The canvas selector is not a valid CSS selector when we are headless, as it
  // references an element that did not come from the DOM
  return m_canvas_target[0] == '!';
}

// While Emscripten does support GetProcAddress functionality, its
// OpenGL implementation is static and calling function pointers
// returned from this method only add overhead
void* GLContextWebGLEmscripten::GetFuncAddress(const std::string& name)
{
  ASSERT_MSG(VIDEO, false, "GL function '%s' must be accessed statically if required",
               name.c_str());
  return nullptr;
}

bool GLContextWebGLEmscripten::MakeCurrent()
{
  return emscripten_webgl_make_context_current(m_context) == EMSCRIPTEN_RESULT_SUCCESS;
}

bool GLContextWebGLEmscripten::ClearCurrent()
{
  return emscripten_webgl_make_context_current(0) == EMSCRIPTEN_RESULT_SUCCESS;
}

void GLContextWebGLEmscripten::Swap()
{
  if(m_context > 0 && emscripten_is_webgl_context_lost(m_context))
  {
    // On the web, the browser may decide to "lose" the WebGL context, essentially destroying
    // it, if, for instance, it deems the context too resource-intensive. This can be recovered
    // from by recreating the lost WebGL context (currently TODO).
    PanicAlertFmt("The WebGL context was lost. You must restart the application entirely.");
    emscripten_webgl_destroy_context(m_context);
    m_context = 0;
  }

  g_webgl_swap_ready_event.Set();
  g_webgl_main_loop_frame_begin_event.WaitFor(std::chrono::milliseconds(500));
}

bool GLContextWebGLEmscripten::Initialize(const WindowSystemInfo& wsi, bool stereo, bool core)
{
  m_canvas_target = static_cast<const char*>(wsi.render_surface);

  // We only support (emulated) OpenGL ES
  m_opengl_mode = Mode::OpenGLES;

  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);

  // If we can't get hardware acceleration, make a big deal out of it
  attrs.failIfMajorPerformanceCaveat = EM_TRUE;

  // OpenGL ES 3.0 corresponds to WebGL 2.0 internally
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;

  // We don't want antialiasing, a depth buffer nor a stencil buffer
  attrs.antialias = EM_FALSE;
  attrs.depth = EM_FALSE;
  attrs.stencil = EM_FALSE;

  // We are running inside a pthread, which, on Emscripten's web
  // backend, means we are inside a Web Worker. Consequently,
  // threads don't have access to WebGL unless we are in a
  // fairly modern browser that supports OffscreenCanvas, since
  // the code isn't running on the browser main thread. Fortunately,
  // Emscripten provides us with a proxying method that relays GL
  // calls to a canvas in the browser main thread.
  attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS;
  attrs.renderViaOffscreenBackBuffer = EM_TRUE;

  // This should really be true, but the web API behind this functionality has
  // been disabled because it promotes busy-looping, something that must be
  // avoided on the web. Instead, we control when we yield back to the host
  // environment so that it can automatically swap buffers; as long as we're
  // executing code, no other events can occur.
  attrs.explicitSwapControl = EM_FALSE;

  // We'll be doing a lot of rendering; it is safe to
  // assume that we'll need a powerful GPU.
  attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;

  // Enable all WebGL extensions that help with emulating OpenGL ES
  attrs.enableExtensionsByDefault = EM_TRUE;

  // Create context using supplied canvas target
  m_context = emscripten_webgl_create_context(m_canvas_target, &attrs);

  // If the returned value is negative or zero, it's an error
  // TODO: switch to https://github.com/emscripten-core/emscripten/pull/12426
  if (m_context <= 0)
  {
    ERROR_LOG_FMT(
        VIDEO,
        "Failed to create backing WebGL context: emscripten_webgl_create_context returned {}",
        m_context);
    return false;
  }

  return MakeCurrent();
}

void GLContextWebGLEmscripten::Update()
{
  int buf_width, buf_height;
  emscripten_webgl_get_drawing_buffer_size(m_context, &buf_width, &buf_height);
  m_backbuffer_width = buf_width;
  m_backbuffer_height = buf_height;
}