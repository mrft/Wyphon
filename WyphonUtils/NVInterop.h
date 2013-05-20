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


// Helper function that loads the NVidia Interop functions dynamically
BOOL loadNvExt();
