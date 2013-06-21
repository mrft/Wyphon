// WyphonUtils.h

#pragma once

namespace WyphonUtils {

	extern "C" _declspec(dllexport)
	HANDLE InitDevice();

	extern "C" _declspec(dllexport)
	HRESULT ReleaseDevice(HANDLE wyphonDeviceHandle);

	extern "C" _declspec(dllexport)
	HRESULT CreateDX9ExTexture(unsigned __int32 width, unsigned __int32 height, DWORD usage, D3DFORMAT format, PDIRECT3DTEXTURE9 * out_pD3D9Texture, HANDLE * out_SharedTextureHandle);

	extern "C" _declspec(dllexport)
	HRESULT CreateLinkedGLTexture(unsigned __int32 width, unsigned __int32 height, DWORD usage, DWORD format, HANDLE &DXShareHandle, GLuint &out_GlTextureName, HANDLE &out_GlTextureHandle);
	extern "C" _declspec(dllexport)
	HRESULT ReleaseLinkedGLTexture(GLuint &out_GlTextureName, HANDLE &out_GlTextureHandle);

	extern "C" _declspec(dllexport)
	HRESULT LockInteropObject(HANDLE &interopObjectHandle);
	extern "C" _declspec(dllexport)
	HRESULT UnlockInteropObject(HANDLE &interopObjectHandle);

	extern "C" _declspec(dllexport)
	HRESULT CopyGLTexture( HANDLE hDevice, HANDLE hInteropObject, GLuint sourceTexture, GLuint targetTexture, int width, int height, BOOL bFlip = TRUE );

	//public ref class WyphonUtils
	//{
	//	// TODO: Add your methods for this class here.
	//};
}
