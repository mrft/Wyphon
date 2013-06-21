#include "stdafx.h"
#include "GLExtensions.h"

PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV = NULL;
PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV = NULL;
PFNWGLDXSETRESOURCESHAREHANDLENVPROC wglDXSetResourceShareHandleNV = NULL;
PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV = NULL;
PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV = NULL;
PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV = NULL;
PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV = NULL;

glBindFramebufferEXTPROC glBindFramebufferEXT = NULL;
glBindRenderbufferEXTPROC glBindRenderbufferEXT = NULL;
glCheckFramebufferStatusEXTPROC glCheckFramebufferStatusEXT = NULL;
glDeleteFramebuffersEXTPROC glDeleteFramebuffersEXT = NULL;
glDeleteRenderBuffersEXTPROC glDeleteRenderBuffersEXT = NULL;
glFramebufferRenderbufferEXTPROC glFramebufferRenderbufferEXT = NULL;
glFramebufferTexture1DEXTPROC glFramebufferTexture1DEXT = NULL;
glFramebufferTexture2DEXTPROC glFramebufferTexture2DEXT = NULL;
glFramebufferTexture3DEXTPROC glFramebufferTexture3DEXT = NULL;
glGenFramebuffersEXTPROC glGenFramebuffersEXT = NULL;
glGenRenderbuffersEXTPROC glGenRenderbuffersEXT = NULL;
glGenerateMipmapEXTPROC glGenerateMipmapEXT = NULL;
glGetFramebufferAttachmentParameterivEXTPROC glGetFramebufferAttachmentParameterivEXT = NULL;
glGetRenderbufferParameterivEXTPROC glGetRenderbufferParameterivEXT = NULL;
glIsFramebufferEXTPROC glIsFramebufferEXT = NULL;
glIsRenderbufferEXTPROC glIsRenderbufferEXT = NULL;
glRenderbufferStorageEXTPROC glRenderbufferStorageEXT = NULL;

glBlitFramebufferEXTPROC glBlitFramebufferEXT = NULL;

/**
* Load the Nvidia-Extensions dynamically
*/
BOOL loadInteropExtension() {
	try {
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
	}
	catch (...) {
		return FALSE;
	}

	return TRUE;
}

BOOL loadFBOExtension() {
	try { // load additional function for FBO buffer copying
		glBindFramebufferEXT = (glBindFramebufferEXTPROC)(unsigned)wglGetProcAddress("glBindFramebufferEXT");
		glBindRenderbufferEXT = (glBindRenderbufferEXTPROC)(unsigned)wglGetProcAddress("glBindRenderbufferEXT");
		glCheckFramebufferStatusEXT = (glCheckFramebufferStatusEXTPROC)(unsigned)wglGetProcAddress("glCheckFramebufferStatusEXT");
		glDeleteFramebuffersEXT = (glDeleteFramebuffersEXTPROC)(unsigned)wglGetProcAddress("glDeleteFramebuffersEXT");
		glDeleteRenderBuffersEXT = (glDeleteRenderBuffersEXTPROC)(unsigned)wglGetProcAddress("glDeleteRenderbuffersEXT");
		glFramebufferRenderbufferEXT = (glFramebufferRenderbufferEXTPROC)(unsigned)wglGetProcAddress("glFramebufferRenderbufferEXT");
		glFramebufferTexture1DEXT = (glFramebufferTexture1DEXTPROC)(unsigned)wglGetProcAddress("glFramebufferTexture1DEXT");
		glFramebufferTexture2DEXT = (glFramebufferTexture2DEXTPROC)(unsigned)wglGetProcAddress("glFramebufferTexture2DEXT");
		glFramebufferTexture3DEXT = (glFramebufferTexture3DEXTPROC)(unsigned)wglGetProcAddress("glFramebufferTexture3DEXT");
		glGenFramebuffersEXT = (glGenFramebuffersEXTPROC)(unsigned)wglGetProcAddress("glGenFramebuffersEXT");
		glGenRenderbuffersEXT = (glGenRenderbuffersEXTPROC)(unsigned)wglGetProcAddress("glGenRenderbuffersEXT");
		glGenerateMipmapEXT = (glGenerateMipmapEXTPROC)(unsigned)wglGetProcAddress("glGenerateMipmapEXT");
		glGetFramebufferAttachmentParameterivEXT = (glGetFramebufferAttachmentParameterivEXTPROC)(unsigned)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
		glGetRenderbufferParameterivEXT = (glGetRenderbufferParameterivEXTPROC)(unsigned)wglGetProcAddress("glGetRenderbufferParameterivEXT");
		glIsFramebufferEXT = (glIsFramebufferEXTPROC)(unsigned)wglGetProcAddress("glIsFramebufferEXT");
		glIsRenderbufferEXT = (glIsRenderbufferEXTPROC)(unsigned)wglGetProcAddress("glIsRenderbufferEXT");
		glRenderbufferStorageEXT = (glRenderbufferStorageEXTPROC)(unsigned)wglGetProcAddress("glRenderbufferStorageEXT");
	}
	catch (...) {
		return FALSE;
	}
	return	glBindFramebufferEXT!=NULL && glBindRenderbufferEXT!=NULL && glCheckFramebufferStatusEXT!=NULL && glDeleteFramebuffersEXT!=NULL && glDeleteRenderBuffersEXT!=NULL &&
			glFramebufferRenderbufferEXT!=NULL && glFramebufferTexture1DEXT!=NULL && glFramebufferTexture2DEXT!=NULL && glFramebufferTexture3DEXT!=NULL && glGenFramebuffersEXT!=NULL &&
			glGenRenderbuffersEXT!=NULL && glGenerateMipmapEXT!=NULL && glGetFramebufferAttachmentParameterivEXT!=NULL && glGetRenderbufferParameterivEXT!=NULL && glIsFramebufferEXT!=NULL &&
			glIsRenderbufferEXT!=NULL && glRenderbufferStorageEXT!=NULL;
}

BOOL loadFBOBlitExtension() {
	try { // load additional function for advanced FBO buffer copying
		glBlitFramebufferEXT = (glBlitFramebufferEXTPROC) wglGetProcAddress("glBlitFramebufferEXT");
	}
	catch (...)	{
		return FALSE;
	}
	return glBlitFramebufferEXT!=NULL;
}

/**
 * Load all GL extensions at once
 *
 * @return		unsigned int		the supported extensions (can be tested with with bit-masks GLEXT_SUPPORT_XXX)
 */
unsigned int loadGLExtensions() {
	unsigned int caps = 0;
	if ( loadInteropExtension() ) {
		caps |= GLEXT_SUPPORT_NVINTEROP;
	}
	if ( loadFBOExtension() ) {
		caps |= GLEXT_SUPPORT_FBO;
	}
	if ( loadFBOBlitExtension() ) {
		caps |= GLEXT_SUPPORT_FBO_BLIT;
	}

	return caps;
}
