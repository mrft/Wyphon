// This is the main DLL file.

#include "stdafx.h"
#include <set>
#include <map>
#include "GLExtensions.h"

using namespace System;

namespace WyphonUtils {

	//global variables
	IDirect3D9Ex * g_pDirect3D9Ex_WyphonUtils = NULL;
	IDirect3DDevice9Ex * g_pDeviceD3D9ex_WyphonUtils = NULL;
	// the DX/GL interop device's handle
	HANDLE g_GLDXInteropHandle = NULL;

	std::set<HANDLE> g_wyphonDeviceHandles;

		// The Framebuffers for copying the texture
	std::map<HANDLE, GLuint > g_FBO;

	unsigned int m_GLExtSupported;

	/**
	 * Creates a new handle and adds it to the list of handles
	 *
	 * @return		HANDLE		the handle's value
	 */
	HANDLE GenerateNewHandle() {
		HANDLE wyphonDeviceHandle = (HANDLE) 1; // first handle
		if ( g_wyphonDeviceHandles.size() > 0 ) { // increment handles
			wyphonDeviceHandle = *(g_wyphonDeviceHandles.rbegin());
			wyphonDeviceHandle = (HANDLE) ((int) wyphonDeviceHandle + 1);
		}
		g_wyphonDeviceHandles.insert(wyphonDeviceHandle);
		return wyphonDeviceHandle;
	}

	/**
	 * Removes a handle from the list of handles
	 *
	 * @param		HANDLE		the handle to close
	 * @return		BOOLEAN		TRUE if successful, FALSE on error
	 */
	BOOL CloseHandle(HANDLE handle) {
		std::set<HANDLE>::iterator iter = g_wyphonDeviceHandles.find(handle);
		if ( iter == g_wyphonDeviceHandles.end() ) {
			return FALSE;
		} else {
			g_wyphonDeviceHandles.erase( iter );
			return TRUE;
		}
	}

	/**
	 * Create GL/DX Interop Instance
	 *
	 * @return		HRESULT		S_FALSE if wglDXInterop is not supported or failed to initialize, S_OK otherwise.
	 *
	 * @author		Elio
	 */
	HRESULT InitGLDXInterop() {
		if ( g_GLDXInteropHandle != NULL ) {
			return S_FALSE;
		}
			// load WGL extension functions
		m_GLExtSupported = loadGLExtensions();
		if ( !(m_GLExtSupported & GLEXT_SUPPORT_NVINTEROP) ) {
			// NVidia wglDXInterop is not supported
			// - this is not critical (we do not throw an exception)
			return S_FALSE;
		}
		g_GLDXInteropHandle = wglDXOpenDeviceNV(g_pDeviceD3D9ex_WyphonUtils);
		if ( g_GLDXInteropHandle == NULL ) {
			// NVidia wglDXInterop failed to initialize
			// - this is not critical (we do not throw an exception)
			return S_FALSE;
		}
		return S_OK;
	}

	/**
	 * Release GL/DX Interop Instance
	 *
	 * @return		HRESULT		S_FALSE, if the handle is NULL, S_OK otherwise
	 *
	 * @author		Elio
	 */
	HRESULT ReleaseGLDXInterop() {
		if ( g_GLDXInteropHandle == NULL ) { // already closed
			return S_FALSE;
		}
		BOOL success = wglDXCloseDeviceNV(g_GLDXInteropHandle);
		if ( success ) {
			g_GLDXInteropHandle = NULL;
		} else {
			throw TEXT("NVidia wglDXInterop failed to close.");
		}
		
		return S_OK;
	}

	/// Utility function to setup DX9Ex and a device. Also sets up the DX/GL Interop Device, if possible.
	/// Necessary before the other Utility functions that need DX9Ex will work.
	///
	/// @return		HANDLE		a handle to the Wyphon device (which stands for the DirectX and the GL/DX interop device)
	///
	/// @author		Frederik
	/// @author		Elio
	///
	extern "C" _declspec(dllexport)
	HANDLE InitDevice() {
		HRESULT hr;
		//create Direct3D instance if necessary
		if ( g_pDirect3D9Ex_WyphonUtils == NULL ) {
			hr = Direct3DCreate9Ex( D3D_SDK_VERSION,  /*_Out_*/ &g_pDirect3D9Ex_WyphonUtils );
			if ( hr != S_OK ) {
				return NULL;
			}
		}
		//create device if necessary
		if ( g_pDeviceD3D9ex_WyphonUtils == NULL ) {

			// Do we support hardware vertex processing? if so, use it. 
			// If not, downgrade to software.
			D3DCAPS9 d3dCaps;
			hr = g_pDirect3D9Ex_WyphonUtils->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps );
			if ( hr != S_OK ) {
				// TO DO: Respond to failure of GetDeviceCaps
				return NULL;
			}

			DWORD dwBehaviorFlags = D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED;
			if ( d3dCaps.VertexProcessingCaps != 0 ) {
				dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;

				//usage = usage XOR D3DUSAGE_SOFTWAREPROCESSING;
			}
			else {
				dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

				//usage = usage | D3DUSAGE_SOFTWAREPROCESSING;
			}
			
			D3DDISPLAYMODE displayMode;

			hr = g_pDirect3D9Ex_WyphonUtils->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &displayMode);

			if ( hr != S_OK ) {
				return NULL;
			}

			//D3DPRESENT_PARAMETERS * presentParameters
			D3DPRESENT_PARAMETERS presentParameters = {0};
			ZeroMemory( &presentParameters, sizeof(presentParameters) );
			presentParameters.Windowed = true;
			presentParameters.hDeviceWindow = NULL;
			presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentParameters.BackBufferWidth = 64;
			presentParameters.BackBufferHeight = 64;
			presentParameters.BackBufferFormat = displayMode.Format; //D3DFMT_A8R8G8B8;
			presentParameters.EnableAutoDepthStencil = FALSE;
			presentParameters.AutoDepthStencilFormat = D3DFMT_D24S8;
			presentParameters.BackBufferCount = 1;
			//present_parameters.Flags = 0;
			//present_parameters.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

			hr = g_pDirect3D9Ex_WyphonUtils->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, dwBehaviorFlags, &presentParameters, NULL, &g_pDeviceD3D9ex_WyphonUtils);
			if ( hr != S_OK ) {
				return NULL;
			}
		}
		hr = InitGLDXInterop();
		if ( hr != S_OK ) {
			// GL/DX Interop failed or not supported
			// - this is not critical
		}
		HANDLE wyphonDeviceHandle = GenerateNewHandle();
		g_FBO[wyphonDeviceHandle] = NULL;
		glGenFramebuffersEXT(1, &g_FBO[wyphonDeviceHandle]);
			// everything ok -> return new handle
		return wyphonDeviceHandle;
	}


	/// Utility function to teardown DX9Ex and its device and also the GL/DX Interop device.
	/// Necessary before exiting the application.
	///
	/// @return		HRESULT		S_FALSE, if the handle is NULL, S_OK otherwise
	///
	/// @author		Frederik
	/// @author		Elio
	///
	extern "C" _declspec(dllexport)
	HRESULT ReleaseDevice(HANDLE wyphonDeviceHandle) {
		if ( CloseHandle(wyphonDeviceHandle) ) { // handle really existed
				// free the fbo and remove it from list
			glDeleteFramebuffersEXT(1, &g_FBO[wyphonDeviceHandle]);
			g_FBO.erase(wyphonDeviceHandle);

			if ( g_wyphonDeviceHandles.size() == 0 ) { // the last user left
				if ( g_pDeviceD3D9ex_WyphonUtils != NULL ) {
					g_pDeviceD3D9ex_WyphonUtils->Release();
					g_pDeviceD3D9ex_WyphonUtils = NULL;
				}
				if ( g_pDirect3D9Ex_WyphonUtils != NULL ) {
					g_pDirect3D9Ex_WyphonUtils->Release();
					g_pDirect3D9Ex_WyphonUtils = NULL;
				}
				ReleaseGLDXInterop();
			}
			return S_OK;
		} else { // the handle did not exist
			return S_FALSE;
		}
	}


	/// Utility function that takes a given hWnd as a parameter. It will create a D3D9ex device first 
	/// and then will try to create a D3D9Ex shared texture. the returned handle can be used 
	/// by D3D10 or higher to open the texture and do whatever needs to be done
	///
	/// This way D3D10 or higher apps can make sure their texture can be opened by D3D9Ex applications
	///
	/// Usage:	if you need a texture for constantly copying data from main memory to, use D3DUSAGE_DYNAMIC (will be in AGP memory instead of GPU memory)
	///			if you need a texture te render another 3D scene to, use D3DUSAGE_RENDERTARGET (will be in GPU memory)
	///
	/// @param	unsigned __int32			width of the new/shared texture
	/// @param	unsigned __int32			height of the new/shared texture
	/// @param	DWORD						usage of the new/shared texture (see MSDN IDirect3D9::CreateTexture)
	/// @param	DWORD						format of the new/shared texture (see MSDN IDirect3D9::CreateTexture)
	/// @param	PDIRECT3DTEXTURE9			Will be set to the new DirectX Texture Object
	/// @param	HANDLE						If it points to NULL: create a new resource (returns a valid share handle)
	///										If it points to a valid DirectX share handle: link to an existing texture
	///
	/// @author		Frederik
	/// @author		Elio
	///
	extern "C" _declspec(dllexport)
	HRESULT CreateDX9ExTexture(unsigned __int32 width, unsigned __int32 height, DWORD usage, D3DFORMAT format, PDIRECT3DTEXTURE9 * out_pD3D9Texture, HANDLE * out_SharedTextureHandle) {
		if ( g_pDirect3D9Ex_WyphonUtils == NULL || g_pDeviceD3D9ex_WyphonUtils == NULL ) {
			throw TEXT("Direct3D not properly intialized. Call InitDX9Ex() first (and don't forget to ReleaseDX9Ex() before exiting your application)");
		}
		
		HRESULT hr = S_OK;

		if ( *out_SharedTextureHandle == NULL ) { // we're about to create a new texture (rather than connecting to an existing one)
			usage = usage | D3DUSAGE_NONSECURE;
		}

		//create texture
		*out_pD3D9Texture = NULL;
		hr = g_pDeviceD3D9ex_WyphonUtils->CreateTexture(width, height, 1, usage, format, D3DPOOL_DEFAULT, out_pD3D9Texture, out_SharedTextureHandle);

		return hr;
	}

	/// Creates an openGL texture that point to the same ressource as a shared DirectX texture
	/// If DXShareHandle is supplied, it links to an existing DirectX texture resource
	/// If DXShareHandle is NULL, it creates a new one.
	/// 
	/// @param	unsigned __int32	width of the new/shared texture
	/// @param	unsigned __int32	height of the new/shared texture
	/// @param	DWORD				usage of the new/shared texture (see MSDN IDirect3D9::CreateTexture)
	/// @param	DWORD				format of the new/shared texture (see MSDN IDirect3D9::CreateTexture)
	/// @param	HANDLE				NULL to create a new resource, a valid DirectX share handle to link to an existing texture
	/// @param	GLuint				Will be set to the new OpenGl texture name
	/// @param	HANDLE				Will be set to the new interop object's handle
	/// 
	/// @author		Elio
	/// 
	extern "C" _declspec(dllexport)
	HRESULT CreateLinkedGLTexture(unsigned __int32 width, unsigned __int32 height, DWORD usage, DWORD format, HANDLE &DXShareHandle, GLuint &out_GlTextureName, HANDLE &out_interopObjectHandle) {
		HRESULT hr = S_OK;

		if ( wglDXCloseDeviceNV == NULL ) {
			return S_FALSE;
		}

		LPDIRECT3DTEXTURE9 pD3D9Texture;
		HRESULT res = CreateDX9ExTexture(width, height, usage, (D3DFORMAT) format, &pD3D9Texture, &DXShareHandle);
		if ( res != S_OK ) {
			throw TEXT("Cannot create DX9Ex Texture.");
		}

		glGenTextures(1, &out_GlTextureName);

			// prepare shared resource
		if (!wglDXSetResourceShareHandleNV(pD3D9Texture, DXShareHandle) ) {
			throw TEXT("Cannot prepare shared DX Texture for OpenGl. wglDXSetResourceShareHandleNV() failed.");
		}

		GLenum access = WGL_ACCESS_READ_ONLY_NV;
		if ( DXShareHandle == NULL ) { // we're about to create a new texture, so we will want to write to it
			access = WGL_ACCESS_READ_WRITE_NV;
		}

			// register for interop and associate with dx texture
		out_interopObjectHandle = wglDXRegisterObjectNV(g_GLDXInteropHandle, pD3D9Texture,
			out_GlTextureName,
			GL_TEXTURE_2D,
			access);
			/* dx texture seems to be released automatically by interop device, so we do not need to take care of this
			 * this is not documented, but when carefully watching the variables, the pointer gets reset some time after without us doing anything
			 */
		DWORD e = GetLastError();
		if ( !out_interopObjectHandle ) {
			throw TEXT("Cannot link OpenGl to DX Texture. wglDXRegisterObjectNV() failed.");
		}
		
		return hr;
	}

	/// Release the shared GL-Texture previously created by CreateLinkedGLTexture
	/// 
	/// @param	GLuint		OpenGl Texture's Name
	/// @param	HANDLE		Interop Object's Handle
	/// 
	/// @author		Elio
	/// 
	extern "C" _declspec(dllexport)
	HRESULT ReleaseLinkedGLTexture(GLuint &out_GlTextureName, HANDLE &interopObjectHandle) {
		HRESULT hr = S_OK;

		if ( wglDXCloseDeviceNV == NULL ) {
			return S_FALSE;
		}

		if ( interopObjectHandle != NULL ) {
			wglDXUnregisterObjectNV(g_GLDXInteropHandle, interopObjectHandle);
			interopObjectHandle = NULL;
		}

		glDeleteTextures(1, &out_GlTextureName);

		return hr;
	}

	/// Lock the shared OpenGL Texture prior to accessing it (reading or writing)
	/// 
	/// @param	HANDLE		Interop Object's Handle to lock
	///
	/// @author		Elio
	/// 
	extern "C" _declspec(dllexport)
	HRESULT LockInteropObject(HANDLE &interopObjectHandle) {
		HRESULT hr = S_OK;

		if ( !wglDXLockObjectsNV(g_GLDXInteropHandle, 1, &interopObjectHandle) ) {
			throw TEXT("Cannot lock texture. wglDXLockObjectsNV() failed.");
		}

		return hr;
	}

	/// Unlock the shared OpenGL Texture after accessing it (reading or writing)
	/// 
	/// @param	HANDLE		Interop Object's Handle to unlock
	///
	/// @author		Elio
	/// 
	extern "C" _declspec(dllexport)
	HRESULT UnlockInteropObject(HANDLE &interopObjectHandle) {
		HRESULT hr = S_OK;

		if ( !wglDXUnlockObjectsNV(g_GLDXInteropHandle, 1, &interopObjectHandle) ) {
			throw TEXT("Cannot lock texture. wglDXUnlockObjectsNV() failed.");
		}

		return hr;
	}

	/// Copy a non-shared OpenGL Texture to a shared texture or vice versa
	///
	/// Flipping the texture upside down is necessary in order to have coherent
	/// texture origin in both, OpenGL and DirectX. Flipping is only supported,
	/// if the graphics driver supports glBlitFramebufferEXT. 
	/// 
	/// @param	GLuint		the source texture's OpenGL handle
	/// @param	GLuint		the target texture's OpenGL handle
	/// @param	HANDLE		the interop object of the shared texture
	/// @param	int			texture width
	/// @param	int			texture height
	/// @param	BOOL		indicated that the texture shall be flipped upside down
	///						this accomodates for the different origin in DX and OpenGL Textures
	///						TRUE (flip) by default
	///
	/// @author		Elio
	/// 
	extern "C" _declspec(dllexport)
	HRESULT CopyGLTexture( HANDLE hDevice, HANDLE hInteropObject, GLuint sourceTexture, GLuint targetTexture, int width, int height, BOOL bFlip = TRUE ) {
		HRESULT hr = S_OK;
		if ( ! (m_GLExtSupported & GLEXT_SUPPORT_FBO) ) {
			return S_FALSE;
		}

		WyphonUtils::LockInteropObject(hInteropObject);
			// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_FBO[hDevice]);

			// attach source texture to first attachment point
		glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_2D, sourceTexture, 0);
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

		if ( m_GLExtSupported & GLEXT_SUPPORT_FBO_BLIT ) { // blitting supported: we may flip the texture while copying
				// attach target texture to second attachment point
			glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT,
				GL_TEXTURE_2D, targetTexture, 0);
			glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
				// copy one texture buffer to the other while flipping upside down (OpenGL and DirectX have different texture origins)
			glBlitFramebufferEXT(0, 0, width, height,
						   0, bFlip ? height : 0, width, bFlip ? 0 : height,
						   GL_COLOR_BUFFER_BIT,
						   GL_NEAREST);
		} else { // fallback: no blitting supported - directly copy to texture - in this case, texture cannot be flipped
			glBindTexture(GL_TEXTURE_2D, targetTexture);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,width, height);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
			// unbind FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			// unlock interop object
		WyphonUtils::UnlockInteropObject(hInteropObject);
		return hr;
	}
}
