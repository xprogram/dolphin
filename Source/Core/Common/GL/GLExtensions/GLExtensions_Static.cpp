// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <sstream>
#include <unordered_set>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#include "Common/GL/GLExtensions/GLExtensions.h"

namespace GLExtensions
{
static u32 s_gl_version;
static std::unordered_set<std::string> s_extension_list;

static void InitVersion()
{
  GLint major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  if (glGetError() == GL_NO_ERROR)
    s_gl_version = major * 100 + minor * 10;
  else
    s_gl_version = 210;
}

u32 Version()
{
  return s_gl_version;
}

bool Init(GLContext* context)
{
  // Nothing much to do; all GL procedures are already linked
  // to their respective symbols, so no function pointer
  // fetching is needed. Just need to query version and fetch
  // the supported extensions.

  InitVersion();

  s_extension_list.clear();

  if (s_gl_version < 300)
  {
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    std::string tmp(extensions);
    std::istringstream buffer(tmp);

    while (buffer >> tmp)
      s_extension_list.insert(tmp);
  }
  else
  {
    GLint ext_total = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &ext_total);
    for (GLint i = 0; i < ext_total; ++i)
      s_extension_list.insert((const char*)glGetStringi(GL_EXTENSIONS, i));
  }

  return true;
}

bool Supports(const std::string& name)
{
  if (s_extension_list.count(name))
  {
#ifdef __EMSCRIPTEN__
    return emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),
                                             name.c_str()) == EM_TRUE;
#else
    return true;
#endif
  }

  return false;
}
}  // namespace GLExtensions