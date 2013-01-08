/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 26/10/2012
 * Time: 13:10
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
// MyClass.h
#pragma once

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>



namespace Wyphon {

	#define WYPHON_MAX_DESCRIPTION_LENGTH 		127

	struct WyphonD3DTextureInfo {
		HANDLE hSharedTexture;
		unsigned int width;
		unsigned int height;
		DWORD format;	//D3DFMT_xxx constants like D3DFMT_A8R8G8B8 (http://msdn.microsoft.com/en-us/library/windows/desktop/bb172558(v=vs.85).aspx)
		DWORD usage;	//D3DUSAGE_xxx constants like D3DUSAGE_RENDERTARGET (http://msdn.microsoft.com/en-us/library/windows/desktop/bb172625(v=vs.85).aspx)
		wchar_t description[WYPHON_MAX_DESCRIPTION_LENGTH + 1];
		
		unsigned int partnerId; //id of partner that shared it with us
	};


	typedef void ( * LPWyphonPartnerJoinedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingWyphonPartnerId, LPCTSTR description, void * customData );
	typedef void ( * LPWyphonPartnerLeftCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingWyphonPartnerId, void * customData );

//	typedef void ( * LPD3DTextureSharingStartedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, WyphonD3DTextureInfo * pTextureInfo);
//	typedef void ( * LPD3DTextureSharingStoppedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, WyphonD3DTextureInfo * pTextureInfo );
	typedef void ( * LPD3DTextureSharingStartedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD format, DWORD usage, LPTSTR description );
	typedef void ( * LPD3DTextureSharingStoppedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD format, DWORD usage, LPTSTR description );





	extern "C" _declspec(dllexport)
	bool ShareD3DTexture(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD format, DWORD usage, LPTSTR description);

	extern "C" _declspec(dllexport)
	bool UnshareD3DTexture(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle);

	extern "C" _declspec(dllexport)
	bool DestroyWyphonPartner(HANDLE wyphonPartnerHandle);

	extern "C" _declspec(dllexport)
	HANDLE CreateWyphonPartner( LPTSTR applicationName
								, void * pCallbackFuncCustomData
								, LPWyphonPartnerJoinedCALLBACK pPartnerJoinedCallbackFunc
								, LPWyphonPartnerLeftCALLBACK pPartnerLeftCallbackFunc
								, LPD3DTextureSharingStartedCALLBACK pD3DTextureSharingStartedCallbackFunc 
								, LPD3DTextureSharingStoppedCALLBACK pD3DTextureSharingStoppedCallbackFunc 
	 							);

	extern "C" _declspec(dllexport)
	LPCTSTR GetWyphonPartnerName(HANDLE wyphonPartnerHandle, unsigned int wyphonPartnerId);

	extern "C" _declspec(dllexport)
	bool GetD3DTextureInfo(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle, unsigned int & wyphonPartnerId, unsigned int & width, unsigned int & height, DWORD & format, DWORD & usage, LPTSTR description, int maxDescriptionLength );

	extern "C" _declspec(dllexport)
	unsigned int GetPartnerId(HANDLE wyphonPartnerHandle);
	
	
//	extern "C" _declspec(dllexport)
//	std::array<WyphonInfo *> GetAllD3DTextureInfo(HANDLE wyphonPartnerHandle);


	/// <summary>
	///
	/// </summary>
	//public ref class WyphonPartner {
		// TODO: Add class methods here
	//};
}
