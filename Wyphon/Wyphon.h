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
		DWORD usage; //rendertarget or ...
		wchar_t description[WYPHON_MAX_DESCRIPTION_LENGTH + 1];
	};


//	typedef bool ( * LPD3DTextureSharingStartedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, WyphonD3DTextureInfo * pTextureInfo);
//	typedef bool ( * LPD3DTextureSharingStoppedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, WyphonD3DTextureInfo * pTextureInfo );
	typedef bool ( * LPD3DTextureSharingStartedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD usage, LPTSTR description );
	typedef bool ( * LPD3DTextureSharingStoppedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD usage, LPTSTR description );





	extern "C" _declspec(dllexport)
	bool ShareD3DTexture(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD usage, LPTSTR description);

	extern "C" _declspec(dllexport)
	bool UnshareD3DTexture(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle);

	extern "C" _declspec(dllexport)
	bool DestroyWyphonPartner(HANDLE wyphonPartnerHandle);

	extern "C" _declspec(dllexport)
	HANDLE CreateWyphonPartner( LPTSTR applicationName
								, LPD3DTextureSharingStartedCALLBACK pD3DTextureSharingStartedCallbackFunc 
								, LPD3DTextureSharingStoppedCALLBACK pD3DTextureSharingStoppedCallbackFunc 
	 							);

	extern "C" _declspec(dllexport)
	LPCTSTR GetWyphonPartnerName(HANDLE wyphonPartnerHandle, unsigned int wyphonPartnerId);

	extern "C" _declspec(dllexport)
	bool GetD3DTextureInfo(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle, unsigned int & wyphonPartnerId, unsigned int & width, unsigned int & height, DWORD & usage, LPTSTR description, int maxDescriptionLength );



	/// <summary>
	///
	/// </summary>
	public ref class WyphonPartner {
		// TODO: Add class methods here
	};
}
