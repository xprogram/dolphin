// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

// Emscripten OpenGL API function definitions
// to enable static linkage (dummy functions
// created as needed to avoid compilation errors)

#include <GL/gl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>

#include <webgl/webgl1_ext.h>
#include <webgl/webgl2_ext.h>

#define GL_PRIMITIVE_RESTART_NV 0x8558
#define GL_PRIMITIVE_RESTART_INDEX_NV 0x8559

#define GL_PIXEL_COUNTER_BITS_NV 0x8864
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV 0x8865
#define GL_PIXEL_COUNT_NV 0x8866
#define GL_PIXEL_COUNT_AVAILABLE_NV 0x8867

#define GL_OCCLUSION_TEST_HP 0x8165
#define GL_OCCLUSION_TEST_RESULT_HP 0x8166

#ifdef __cplusplus
extern "C" {
#endif

GLAPI void APIENTRY glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
GLAPI void APIENTRY glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index,
                                                  const GLchar* name);

GLAPI void APIENTRY glPrimitiveRestartIndexNV(GLuint index);
GLAPI void APIENTRY glPrimitiveRestartNV(void);
GLAPI void APIENTRY glPrimitiveRestartIndex(GLuint index);

GLAPI void APIENTRY glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data);
GLAPI void APIENTRY glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat,
                                            GLsizei width, GLsizei height, GLsizei depth,
                                            GLboolean fixedsamplelocations);
GLAPI void APIENTRY glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat,
                                              GLsizei width, GLsizei height, GLsizei depth,
                                              GLboolean fixedsamplelocations);
GLAPI void APIENTRY glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width, GLsizei height,
                                         GLsizei depth, GLenum format, GLenum type, GLsizei bufSize,
                                         void* pixels);
GLAPI void APIENTRY glBufferStorage(GLenum target, GLsizeiptr size, const void* data,
                                    GLbitfield flags);
GLAPI void APIENTRY glClipControl(GLenum origin, GLenum depth);

typedef void(APIENTRY* GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity,
                                       GLsizei length, const GLchar* message,
                                       const void* userParam);
GLAPI void APIENTRY glDebugMessageCallbackARB(GLDEBUGPROCARB callback, const void* userParam);
GLAPI void APIENTRY glDebugMessageControlARB(GLenum source, GLenum type, GLenum severity,
                                             GLsizei count, const GLuint* ids, GLboolean enabled);
GLAPI void APIENTRY glDebugMessageControl(GLenum source, GLenum type, GLenum severity,
                                          GLsizei count, const GLuint* ids, GLboolean enabled);

GLAPI void APIENTRY glGenOcclusionQueriesNV(GLsizei n, GLuint* ids);
GLAPI void APIENTRY glDeleteOcclusionQueriesNV(GLsizei n, const GLuint* ids);
GLAPI void APIENTRY glBeginOcclusionQueryNV(GLuint id);
GLAPI void APIENTRY glEndOcclusionQueryNV(void);
GLAPI void APIENTRY glGetOcclusionQueryuivNV(GLuint id, GLenum pname, GLuint* params);

#ifdef __cplusplus
}  // extern "C"
#endif