#pragma once

//-----------------------------------------------------------------------------
// GL consts that are needed and aren't present in GL.h
//-----------------------------------------------------------------------------
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define WGL_ACCESS_READ_ONLY_NV 0x0000
#define WGL_ACCESS_READ_WRITE_NV 0x0001
#define WGL_ACCESS_WRITE_DISCARD_NV 0x0002

//-----------------------------------------------------------------------------
// NVIDIA GL ext that allow for dx/gl interop
//-----------------------------------------------------------------------------
typedef HANDLE (WINAPI * PFNWGLDXOPENDEVICENVPROC) (void* dxDevice);
extern PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV;
typedef BOOL (WINAPI * PFNWGLDXCLOSEDEVICENVPROC) (HANDLE hDevice);
extern PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV;
typedef HANDLE (WINAPI * PFNWGLDXREGISTEROBJECTNVPROC) (HANDLE hDevice, void* dxObject, GLuint name, GLenum type, GLenum access);
extern PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV;
typedef BOOL (WINAPI * PFNWGLDXUNREGISTEROBJECTNVPROC) (HANDLE hDevice, HANDLE hObject);
extern PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV;
typedef BOOL (WINAPI * PFNWGLDXSETRESOURCESHAREHANDLENVPROC) (void *dxResource, HANDLE shareHandle);
extern PFNWGLDXSETRESOURCESHAREHANDLENVPROC wglDXSetResourceShareHandleNV;
typedef BOOL (WINAPI * PFNWGLDXLOCKOBJECTSNVPROC) (HANDLE hDevice, GLint count, HANDLE *hObjects);
extern PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV;
typedef BOOL (WINAPI * PFNWGLDXUNLOCKOBJECTSNVPROC) (HANDLE hDevice, GLint count, HANDLE *hObjects);
extern PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV;

//-----------------------------------------------------------------------------
// Declaration of WGL FBO extension
//-----------------------------------------------------------------------------
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT                0x0506
#define GL_MAX_RENDERBUFFER_SIZE_EXT                        0x84E8
#define GL_FRAMEBUFFER_BINDING_EXT                          0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT                         0x8CA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT           0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT           0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT         0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT    0x8CD4
#define GL_FRAMEBUFFER_COMPLETE_EXT                         0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT            0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT    0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT  0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT            0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT               0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT           0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT           0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                      0x8CDD
#define GL_FRAMEBUFFER_STATUS_ERROR_EXT                     0x8CDE
#define GL_MAX_COLOR_ATTACHMENTS_EXT                        0x8CDF
#define GL_COLOR_ATTACHMENT0_EXT                            0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT                            0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT                            0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT                            0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT                            0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT                            0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT                            0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT                            0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT                            0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT                            0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT                           0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT                           0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT                           0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT                           0x8CED
#define GL_COLOR_ATTACHMENT14_EXT                           0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT                           0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT                             0x8D00
#define GL_STENCIL_ATTACHMENT_EXT                           0x8D20
#define GL_FRAMEBUFFER_EXT                                  0x8D40
#define GL_RENDERBUFFER_EXT                                 0x8D41
#define GL_RENDERBUFFER_WIDTH_EXT                           0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT                          0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT                 0x8D44
#define GL_STENCIL_INDEX_EXT                                0x8D45
#define GL_STENCIL_INDEX1_EXT                               0x8D46
#define GL_STENCIL_INDEX4_EXT                               0x8D47
#define GL_STENCIL_INDEX8_EXT                               0x8D48
#define GL_STENCIL_INDEX16_EXT                              0x8D49

typedef void   (APIENTRY *glBindFramebufferEXTPROC) (GLenum target, GLuint framebuffer);
typedef void   (APIENTRY *glBindRenderbufferEXTPROC) (GLenum target, GLuint renderbuffer);
typedef GLenum (APIENTRY *glCheckFramebufferStatusEXTPROC) (GLenum target);
typedef void   (APIENTRY *glDeleteFramebuffersEXTPROC) (GLsizei n, const GLuint* framebuffers);
typedef void   (APIENTRY *glDeleteRenderBuffersEXTPROC) (GLsizei n, const GLuint* renderbuffers);
typedef void   (APIENTRY *glFramebufferRenderbufferEXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void   (APIENTRY *glFramebufferTexture1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void   (APIENTRY *glFramebufferTexture2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void   (APIENTRY *glFramebufferTexture3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void   (APIENTRY *glGenFramebuffersEXTPROC) (GLsizei n, GLuint* framebuffers);
typedef void   (APIENTRY *glGenRenderbuffersEXTPROC) (GLsizei n, GLuint* renderbuffers);
typedef void   (APIENTRY *glGenerateMipmapEXTPROC) (GLenum target);
typedef void   (APIENTRY *glGetFramebufferAttachmentParameterivEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
typedef void   (APIENTRY *glGetRenderbufferParameterivEXTPROC) (GLenum target, GLenum pname, GLint* params);
typedef GLboolean (APIENTRY *glIsFramebufferEXTPROC) (GLuint framebuffer);
typedef GLboolean (APIENTRY *glIsRenderbufferEXTPROC) (GLuint renderbuffer);
typedef void (APIENTRY *glRenderbufferStorageEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

extern glBindFramebufferEXTPROC glBindFramebufferEXT;
extern glBindRenderbufferEXTPROC glBindRenderbufferEXT;
extern glCheckFramebufferStatusEXTPROC glCheckFramebufferStatusEXT;
extern glDeleteFramebuffersEXTPROC glDeleteFramebuffersEXT;
extern glDeleteRenderBuffersEXTPROC glDeleteRenderBuffersEXT;
extern glFramebufferRenderbufferEXTPROC glFramebufferRenderbufferEXT;
extern glFramebufferTexture1DEXTPROC glFramebufferTexture1DEXT;
extern glFramebufferTexture2DEXTPROC glFramebufferTexture2DEXT;
extern glFramebufferTexture3DEXTPROC glFramebufferTexture3DEXT;
extern glGenFramebuffersEXTPROC glGenFramebuffersEXT;
extern glGenRenderbuffersEXTPROC glGenRenderbuffersEXT;
extern glGenerateMipmapEXTPROC glGenerateMipmapEXT;
extern glGetFramebufferAttachmentParameterivEXTPROC glGetFramebufferAttachmentParameterivEXT;
extern glGetRenderbufferParameterivEXTPROC glGetRenderbufferParameterivEXT;
extern glIsFramebufferEXTPROC glIsFramebufferEXT;
extern glIsRenderbufferEXTPROC glIsRenderbufferEXT;
extern glRenderbufferStorageEXTPROC glRenderbufferStorageEXT;

//-----------------------------------------------------------------------------
// Blit FBO extension (to extension)
//-----------------------------------------------------------------------------

#define READ_FRAMEBUFFER_EXT                0x8CA8
#define DRAW_FRAMEBUFFER_EXT                0x8CA9

typedef void   (APIENTRY *glBlitFramebufferEXTPROC) (GLint srcX0,GLint srcY0,GLint srcX1,GLint srcY1,GLint dstX0,GLint dstY0,GLint dstX1,GLint dstY1,GLbitfield mask,GLenum filter);
extern glBlitFramebufferEXTPROC glBlitFramebufferEXT;


//-----------------------------------------------------------------------------
// EXTENSION SUPPORT FLAGS
//-----------------------------------------------------------------------------
#define GLEXT_SUPPORT_NVINTEROP		0x0001
#define GLEXT_SUPPORT_FBO			0x0002
#define GLEXT_SUPPORT_FBO_BLIT		0x0004

//-----------------------------------------------------------------------------
// Helper functions that load the extension functions dynamically
//-----------------------------------------------------------------------------
BOOL loadInteropExtension();
BOOL loadFBOExtension();
BOOL loadFBOBlitExtension();
	// load all extensions at once
unsigned int loadGLExtensions();

