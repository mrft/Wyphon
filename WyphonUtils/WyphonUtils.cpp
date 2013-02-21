// This is the main DLL file.

#include "stdafx.h"
#include <windows.h>
#include <d3d11.h>
#include <d3d10.h>
#include <d3d10_1.h>
#include <d3d10misc.h>
#include <d3d9.h>

#include "WyphonUtils.h"

using namespace System;

namespace WyphonUtils {

	//global variables
	IDirect3D9Ex * g_pDirect3D9Ex_WyphonUtils = NULL;
	IDirect3DDevice9Ex * g_pDeviceD3D9ex_WyphonUtils = NULL;




	/// Utility function to setup DX9Ex and a device. Necessary before the other Utility functions that need DX9Ex will work.
	///
	HRESULT InitDX9Ex() {
		HRESULT hr = S_OK;

		//create Direct3D instance if necessary
		if ( g_pDirect3D9Ex_WyphonUtils == NULL ) {
			hr = Direct3DCreate9Ex( D3D_SDK_VERSION,  /*_Out_*/ &g_pDirect3D9Ex_WyphonUtils );
			if ( hr != S_OK ) {
				return hr;
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
				return hr;
			}

			DWORD dwBehaviorFlags = 0;
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
				return hr;
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
		}

		return hr;
	}


	/// Utility function to teardown DX9Ex and its device. Necessary before exiting the application.
	///
	void ReleaseDX9Ex() {
		if ( g_pDeviceD3D9ex_WyphonUtils != NULL ) g_pDeviceD3D9ex_WyphonUtils->Release();
		if ( g_pDirect3D9Ex_WyphonUtils != NULL ) g_pDirect3D9Ex_WyphonUtils->Release(); 
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
	HRESULT CreateDX9ExTexture(unsigned __int32 width, unsigned __int32 height, DWORD usage, D3DFORMAT format, PDIRECT3DTEXTURE9 * out_pD3D9Texture, HANDLE * out_SharedTextureHandle) {
		if ( g_pDirect3D9Ex_WyphonUtils == NULL || g_pDeviceD3D9ex_WyphonUtils == NULL ) {
			throw TEXT("Direct3D not properly intialized. Call InitDX9Ex() first (and don't forget to ReleaseDX9Ex() before exiting your application)");
		}
		
		HRESULT hr = S_OK;

		usage = usage | D3DUSAGE_NONSECURE;

		//create texture
		hr = g_pDeviceD3D9ex_WyphonUtils->CreateTexture(width, height, 1, usage, format, D3DPOOL_DEFAULT, out_pD3D9Texture, out_SharedTextureHandle);

		return hr;
	}

}