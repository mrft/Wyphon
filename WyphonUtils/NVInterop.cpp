#include "stdafx.h"
#include "NVInterop.h"

PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV = NULL;
PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV = NULL;
PFNWGLDXSETRESOURCESHAREHANDLENVPROC wglDXSetResourceShareHandleNV = NULL;
PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV = NULL;
PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV = NULL;
PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV = NULL;
PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV = NULL;


/**
* Load the Nvidia-Extensions dynamically
*/
BOOL loadNvExt() {
	wglDXOpenDeviceNV = (PFNWGLDXOPENDEVICENVPROC)wglGetProcAddress("wglDXOpenDeviceNV");
	if(!wglDXOpenDeviceNV)
	{
//		throw TEXT("wglDXOpenDeviceNV ext is not supported by your GPU or driver.");
		return FALSE;
	}
	wglDXRegisterObjectNV = (PFNWGLDXREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXRegisterObjectNV");
	if(!wglDXRegisterObjectNV)
	{
//		throw TEXT("wglDXRegisterObjectNV ext is not supported by your GPU or driver.");
		return FALSE;
	}
	wglDXUnregisterObjectNV = (PFNWGLDXUNREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXUnregisterObjectNV");
	if(!wglDXUnregisterObjectNV)
	{
//		throw TEXT("wglDXRegisterObjectNV ext is not supported by your GPU or driver.");
		return FALSE;
	}
	wglDXSetResourceShareHandleNV = (PFNWGLDXSETRESOURCESHAREHANDLENVPROC)wglGetProcAddress("wglDXSetResourceShareHandleNV");
	if(!wglDXSetResourceShareHandleNV)
	{
//		throw TEXT("wglDXSetResourceShareHandleNV ext is not supported by your GPU or driver.");
		return FALSE;
	}
	wglDXLockObjectsNV = (PFNWGLDXLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXLockObjectsNV");
	if(!wglDXLockObjectsNV)
	{
//		throw TEXT("wglDXLockObjectsNV ext is not supported by your GPU or driver.");
		return FALSE;
	}
	wglDXUnlockObjectsNV = (PFNWGLDXUNLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXUnlockObjectsNV");
	if(!wglDXUnlockObjectsNV)
	{
//		throw TEXT("wglDXUnlockObjectsNV ext is not supported by your GPU or driver.");
		return FALSE;
	}
	wglDXCloseDeviceNV = (PFNWGLDXCLOSEDEVICENVPROC)wglGetProcAddress("wglDXCloseDeviceNV");
	if(!wglDXUnlockObjectsNV)
	{
//		throw TEXT("wglDXCloseDeviceNV ext is not supported by your GPU or driver.");
		return FALSE;
	}

	return true;
}
