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


	typedef bool ( * LPD3DTextureSharingStartedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, WyphonD3DTextureInfo textureInfo);
	typedef bool ( * LPD3DTextureSharingStoppedCALLBACK)( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, HANDLE hD3DTexture );


	/// <summary>
	///
	/// </summary>
	public ref class WyphonPartner {
		// TODO: Add class methods here
	};
}
