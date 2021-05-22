// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Common/GL/GLExtensions/gl_emscripten.h"
#include "Common/Logging/Log.h"

#define DUMMY_GLFUNC(name, p)                                                                      \
  GLAPI void APIENTRY name p { ERROR_LOG_FMT(VIDEO, #name ": dummy GL function called"); }

// clang-format off
DUMMY_GLFUNC(glLogicOp, (GLenum opcode))

DUMMY_GLFUNC(glDebugMessageCallbackARB, (GLDEBUGPROCARB callback, const void* userParam))
DUMMY_GLFUNC(glDebugMessageCallback, (GLDEBUGPROCARB callback, const void* userParam))
DUMMY_GLFUNC(glDebugMessageControlARB, (GLenum source, GLenum type, GLenum severity, GLsizei count,
                             const GLuint* ids, GLboolean enabled))
DUMMY_GLFUNC(glDebugMessageControl, (GLenum source, GLenum type, GLenum severity, GLsizei count,
                          const GLuint* ids, GLboolean enabled))

DUMMY_GLFUNC(glPrimitiveRestartIndexNV, (GLuint index))
DUMMY_GLFUNC(glPrimitiveRestartNV, (void))
DUMMY_GLFUNC(glPrimitiveRestartIndex, (GLuint index))

DUMMY_GLFUNC(glGetBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, void* data))
DUMMY_GLFUNC(glTexImage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                            GLsizei height, GLsizei depth, GLboolean fixedsamplelocations))
DUMMY_GLFUNC(glTexStorage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat,
                              GLsizei width, GLsizei height, GLsizei depth,
                              GLboolean fixedsamplelocations))
DUMMY_GLFUNC(glTexBuffer, (GLenum target, GLenum internalformat, GLuint buffer))
DUMMY_GLFUNC(glDispatchCompute, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z))
DUMMY_GLFUNC(glMemoryBarrier, (GLbitfield barriers))
DUMMY_GLFUNC(glFramebufferTexture, (GLenum target, GLenum attachment, GLuint texture, GLint level))
DUMMY_GLFUNC(glBindImageTexture, (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer,
                       GLenum access, GLenum format))
DUMMY_GLFUNC(glEnableClientState, (GLenum cap))
DUMMY_GLFUNC(glDrawBuffer, (GLenum mode))
DUMMY_GLFUNC(glClipControl, (GLenum origin, GLenum depth))
DUMMY_GLFUNC(glCopyImageSubData, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY,
                       GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX,
                       GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight,
                       GLsizei srcDepth))
DUMMY_GLFUNC(glDrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void* indices,
                             GLint basevertex))
DUMMY_GLFUNC(glBufferStorage, (GLenum target, GLsizeiptr size, const void* data, GLbitfield flags))
DUMMY_GLFUNC(glGetTextureSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                         GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,
                         GLsizei bufSize, void* pixels))

DUMMY_GLFUNC(glGenOcclusionQueriesNV, (GLsizei n, GLuint* ids))
DUMMY_GLFUNC(glDeleteOcclusionQueriesNV, (GLsizei n, const GLuint* ids))
DUMMY_GLFUNC(glBeginOcclusionQueryNV, (GLuint id))
DUMMY_GLFUNC(glEndOcclusionQueryNV, (void))
DUMMY_GLFUNC(glGetOcclusionQueryuivNV, (GLuint id, GLenum pname, GLuint* params))

DUMMY_GLFUNC(glBindFragDataLocationIndexed, (GLuint program, GLuint colorNumber, GLuint index,
                                  const GLchar* name))
DUMMY_GLFUNC(glViewportIndexedf, (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h))
// clang-format on