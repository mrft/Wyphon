// WyphonUtils.h

#pragma once

namespace WyphonUtils {


	extern "C" _declspec(dllexport)
	HRESULT InitDX9Ex();

	extern "C" _declspec(dllexport)
	void ReleaseDX9Ex();

	extern "C" _declspec(dllexport)
	HRESULT CreateDX9ExTexture(UINT width, UINT height, DWORD usage, D3DFORMAT format, PDIRECT3DTEXTURE9 * out_pD3D9Texture, HANDLE * out_SharedTextureHandle);


	//public ref class WyphonUtils
	//{
	//	// TODO: Add your methods for this class here.
	//};
}
