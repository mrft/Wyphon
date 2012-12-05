/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 26/10/2012
 * Time: 13:10
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
// MyClass.cpp
#include "Wyphon.h"
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <d3d9types.h>
//#include <thread>
#include <process.h>
#include "LocalMessageBroadcast.h"


using namespace std;
using namespace System;
using namespace LocalMessageBroadcast;


//	Define the Functions in the DLL for reuse. This is just prototyping the dll's function. 
//	A mock of it. Use "stdcall" for maximum compatibility.

//typedef bool ( * LPPartnerJoinedCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * customData );
//typedef bool ( * LPPartnerLeftCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * customData);
//typedef bool ( * LPMessageReceivedCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * msgData, unsigned int msgLength, void * customData );
//
//
//typedef bool (__stdcall * pDestroyLocalMessageBroadcastPartnerFUNC)(HANDLE localMessageBroadcastPartnerHandle);
//
//typedef HANDLE (__stdcall * pCreateLocalMessageBroadcastPartnerFUNC)( LPTSTR lpSharedMemoryName, LPTSTR myName
//	, void * callbackFuncCustomData
//	, LPPartnerJoinedCALLBACK pPartnerJoinedCallbackFunc
//	, LPPartnerLeftCALLBACK pPartnerLeftCallbackFunc
//	, LPMessageReceivedCALLBACK pMsgReceivedCallbackFunc 
//);
//
//typedef bool (__stdcall * pBroadcastMessageFUNC)(HANDLE localMessageBroadcastPartnerHandle, void * data, unsigned int length);
//
//typedef LPCTSTR (__stdcall * pGetBroadcastPartnerNameFUNC)(HANDLE localMessageBroadcastPartnerHandle, unsigned int partnerId);
//
//
//HINSTANCE hLocalMessageBroadcastDll = LoadLibrary(TEXT("LocalMessageBroadcast.dll"));
//
//// get pointers to the functions in the dll
/////////////////////////////////////////////
//
////if ( hLocalMessageBroadcastDll != NULL ) {
//	FARPROC lpfnDestroyLocalMessageBroadcastPartner = GetProcAddress(HMODULE (hLocalMessageBroadcastDll),"DestroyLocalMessageBroadcastPartner");
//	FARPROC lpfnCreateLocalMessageBroadcastPartner = GetProcAddress(HMODULE (hLocalMessageBroadcastDll),"CreateLocalMessageBroadcastPartner");
//	FARPROC lpfnBroadcastMessage = GetProcAddress(HMODULE (hLocalMessageBroadcastDll),"BroadcastMessage");
//	FARPROC lpfnGetBroadcastPartnerName = GetProcAddress(HMODULE (hLocalMessageBroadcastDll),"GetBroadcastPartnerName");
//
//	pDestroyLocalMessageBroadcastPartnerFUNC DestroyLocalMessageBroadcastPartner = pDestroyLocalMessageBroadcastPartnerFUNC(lpfnDestroyLocalMessageBroadcastPartner);
//	pCreateLocalMessageBroadcastPartnerFUNC CreateLocalMessageBroadcastPartner = pCreateLocalMessageBroadcastPartnerFUNC(lpfnCreateLocalMessageBroadcastPartner);
//	pBroadcastMessageFUNC BroadcastMessage = pBroadcastMessageFUNC(lpfnBroadcastMessage);
//	pGetBroadcastPartnerNameFUNC GetBroadcastPartnerName = pGetBroadcastPartnerNameFUNC(lpfnGetBroadcastPartnerName);
////}




/*
	The WyphonPartner descriptor should hold all of the necessary information 
	about what the other WyphonPartners are sharing with us, and what we are sharing with them.

	So, we are setting up communication with everyone else who is participating. Every message will 
	be sent to every partner.
	There will be multiple types of messages:
	- Hey, I have a texture to share with all of you. Here are its coördinates. (width, height, handle, usage, ...)
	- Hey, the following texture will no longer be valid, here's the handle.
	- Hey, I have a ... to share with all of you.
	- etc.

	A message will have 1 byte telling us the message_type
	and some other bytes depending on that type, with needed info:
	- WYPHON_START_SHARING_D3DTEXTURE (1) 	|	WyphonD3DTextureInfo (sizeof WyphonD3DTextureInfo)
	- WYPHON_STOP_SHARING_D3DTEXTURE (1)	|	sharedHandle (sizeof HANDLE)	
)
*/


namespace Wyphon {


	// IF WE ASSUME that more types can be shared in the future
	// we will add message_types and structs to describe these resources
	// We'll start with textures only at this time

	#define WYPHON_LOCALMESSAGEBROADCAST_NAME				"Wyphon"

	#define WYPHON_START_SHARING_D3DTEXTURE		11
	#define WYPHON_STOP_SHARING_D3DTEXTURE		12
	

	//FOR INTERNAL USE ONLY
	struct WyphonPartnerDescriptor {
		HANDLE hLocalMessageBroadcastPartner; //LocalMessageBroadcast is the communication sub-layer
		
		wstring * applicationName;
		
		map<HANDLE, WyphonD3DTextureInfo* > * sharedByUsD3DTexturesMap;
		
		map<unsigned int, map<HANDLE, WyphonD3DTextureInfo* >> * sharedByPartnersD3DTexturesMap;


		LPD3DTextureSharingStartedCALLBACK pD3DtextureSharingStartedCallbackFunc;
		LPD3DTextureSharingStoppedCALLBACK pD3DtextureSharingStoppedCallbackFunc;
	};


	/// creates a wyphontextureinfo struct, given some parameters
	/// should be deleted afterwards by the caller !!!
	WyphonD3DTextureInfo * CreateWyphonD3DTextureInfo( HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD usage, size_t descriptionLength, LPTSTR description ) {
		//struct WyphonD3DTextureInfo { HANDLE sharedHandle; unsigned int width; unsigned int height; int usage; //rendertarget or ... };
		WyphonD3DTextureInfo * info = new WyphonD3DTextureInfo();
		info->hSharedTexture = sharedTextureHandle;
		info->width = width;
		info->height = height;
		info->usage = usage;
		int copyLength = sizeof(wchar_t) * (descriptionLength > WYPHON_MAX_DESCRIPTION_LENGTH ? WYPHON_MAX_DESCRIPTION_LENGTH : descriptionLength);
		CopyMemory( info->description, (VOID *) description, copyLength );
		info->description[copyLength] = '\0';
		
		return info;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE mailslotId ; WyphonD3DTextureInfo
	int CreateShareD3DTextureMessage(WyphonPartnerDescriptor * pWyphonPartner, WyphonD3DTextureInfo * pTextureInfo, BYTE ** data ) {
		int dataSize = 1 + sizeof(*pTextureInfo);
		
		(*data) = new BYTE[dataSize];
		(*data)[0] = WYPHON_START_SHARING_D3DTEXTURE;
				
		BYTE * addr = (*data) + 1;
		int len = sizeof(WyphonD3DTextureInfo);
		CopyMemory( addr, (VOID *) pTextureInfo, len );

		addr += len;

		//wcout << "Wyphon: CreateShareD3DTextureMessage " << pTextureInfo->hSharedTexture << " " << pTextureInfo->width << "x" << pTextureInfo->height << " named '" << pTextureInfo->description << "' created a message of size " << dataSize << "=" << (addr - *data) << "\n";
		
		return dataSize;
	}


	/// data should be deleted afterwards by the caller !!!
	int CreateUnshareTextureMessage(WyphonPartnerDescriptor * pWyphonPartner, HANDLE sharedTextureHandle, BYTE ** data) {
		int dataSize = 1 + sizeof(sharedTextureHandle);
		(*data) = new BYTE[dataSize];
		(*data)[0] = WYPHON_STOP_SHARING_D3DTEXTURE;

		BYTE * addr = (*data) + 1;
		int len = sizeof(sharedTextureHandle);
		CopyMemory( addr, (VOID *) &sharedTextureHandle, len );
		
		return dataSize;
	}



	/// Add 1 object shared by a partner given its partnerId and the WyphonD3DTextureInfo struct
	bool AddD3DTextureSharedByPartner(WyphonPartnerDescriptor * pWyphonPartner, unsigned int partnerId, WyphonD3DTextureInfo * pTexture) {
		map<HANDLE, WyphonD3DTextureInfo * > * texturesD3DMap =  &(*(pWyphonPartner->sharedByPartnersD3DTexturesMap))[partnerId];
		
		(*texturesD3DMap)[pTexture->hSharedTexture] = pTexture;
		
		return true;
	}

	/// cleans up the resources that are shared with us by the given partner's partnerId
	bool RemoveAllD3DTexturesSharedByPartner(WyphonPartnerDescriptor * pWyphonPartner, unsigned int partnerId) {	
		map<HANDLE, WyphonD3DTextureInfo * > * texturesMap =  &(*(pWyphonPartner->sharedByPartnersD3DTexturesMap))[partnerId];
		map<HANDLE, WyphonD3DTextureInfo*>::iterator itr;

		for ( itr = (*texturesMap).begin(); itr != (*texturesMap).end(); ++itr ) {
			//delete itr->second->description;
			delete itr->second;
		}
		texturesMap->clear();
		
		pWyphonPartner->sharedByPartnersD3DTexturesMap->erase(partnerId);
		
		return true;
	}

	/// Remove 1 object shared by us given the objectHandle
	bool RemoveD3DTextureSharedByPartner(WyphonPartnerDescriptor * pWyphonPartner, unsigned int partnerId, HANDLE textureHandle) {
		map<HANDLE, WyphonD3DTextureInfo * > * texturesMap =  &(*(pWyphonPartner->sharedByPartnersD3DTexturesMap))[partnerId];
		
		if ( texturesMap->count(textureHandle) > 0 ) {
			delete (*texturesMap)[textureHandle];
			texturesMap->erase(textureHandle);
		}
		
		return true;
	}


	/// Remove 1 object shared by us given the objectHandle
	bool RemoveD3DTextureSharedByUs(WyphonPartnerDescriptor * pWyphonPartner, HANDLE textureHandle) {
		map<HANDLE, WyphonD3DTextureInfo* > * texturesMap =  &(*(pWyphonPartner->sharedByUsD3DTexturesMap));
		
		if ( texturesMap->count(textureHandle) > 0 ) {
			//delete (*objectsMap)[texture]->description;
			delete (*texturesMap)[textureHandle];
			texturesMap->erase(textureHandle);
		}
		
		return true;
	}

	/// cleans up all of the resources that are shared by us
	bool RemoveAllD3DTexturesSharedByUs(WyphonPartnerDescriptor * pWyphonPartner) {
		map<HANDLE, WyphonD3DTextureInfo* > * texturesMap =  &(*(pWyphonPartner->sharedByUsD3DTexturesMap));
		map<HANDLE, WyphonD3DTextureInfo*>::iterator itr;

		for ( itr = (*texturesMap).begin(); itr != (*texturesMap).end(); ++itr ) {
			//delete itr->second->description;
			delete itr->second;
		}
		texturesMap->clear();
		
		return true;
	}


	/// Add this texture to the list of 'shared-by-partners' textures
	bool ProcessReceivedShareD3DTextureMessage(WyphonPartnerDescriptor * pWyphonPartner, unsigned int sendingPartnerId, BYTE * data, unsigned int length ) {

		//wcout << "Wyphon: ProcessReceivedShareD3DTextureMessage received a message of size " << length << "\n";

		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		int len = sizeof(WyphonD3DTextureInfo);
		WyphonD3DTextureInfo * pTextureInfo = new WyphonD3DTextureInfo(); //(WyphonD3DTextureInfo *)pData;		
		CopyMemory(pTextureInfo, (VOID *) pData, len );

		wcout << "Wyphon: Texture shared by " <<  GetBroadcastPartnerName(pWyphonPartner->hLocalMessageBroadcastPartner, sendingPartnerId) << "(" << sendingPartnerId << ")" << " handle=" << pTextureInfo->hSharedTexture << " " << pTextureInfo->width << "x" << pTextureInfo->height << " named '" << pTextureInfo->description << "'" << "\n";
		
		bool success = AddD3DTextureSharedByPartner(pWyphonPartner, sendingPartnerId, pTextureInfo);
		
		if ( pWyphonPartner->pD3DtextureSharingStartedCallbackFunc != NULL ) {
			pWyphonPartner->pD3DtextureSharingStartedCallbackFunc(pWyphonPartner, sendingPartnerId, pTextureInfo->hSharedTexture, pTextureInfo->width, pTextureInfo->height, pTextureInfo->usage, pTextureInfo->description );
		}
		
		return success;
	}

	/// Remove this texture from the list of 'shared-by-partners' textures
	bool ProcessReceivedUnshareD3DTextureMessage(WyphonPartnerDescriptor * pWyphonPartner, unsigned int sendingPartnerId, BYTE * data, unsigned int length ) {
	
		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		int len = sizeof(HANDLE);
		HANDLE textureHandle = *((HANDLE *)pData);
		
		wcout << "Wyphon: Texture NOT shared ANYMORE  by " <<  GetBroadcastPartnerName(pWyphonPartner->hLocalMessageBroadcastPartner, sendingPartnerId) << "(" << sendingPartnerId << ")" << " handle=" << textureHandle << "\n";

		if ( pWyphonPartner->pD3DtextureSharingStoppedCallbackFunc != NULL ) {
			WyphonD3DTextureInfo* pTextureInfo = ( ( * ( pWyphonPartner->sharedByPartnersD3DTexturesMap ) )[sendingPartnerId])[textureHandle];
			//HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD usage, LPTSTR description
			pWyphonPartner->pD3DtextureSharingStoppedCallbackFunc( pWyphonPartner, sendingPartnerId, pTextureInfo->hSharedTexture, pTextureInfo->width, pTextureInfo->height, pTextureInfo->usage, pTextureInfo->description );
		}

		bool success = RemoveD3DTextureSharedByPartner(pWyphonPartner, sendingPartnerId, textureHandle);
		
		return success;
	}



	bool PartnerJoinedCallback( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, HANDLE wyphonPartnerHandle ) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;
		
		wcout << "Wyphon: PartnerJoinedCallback: " <<  GetBroadcastPartnerName(localMessageBroadcastPartnerHandle, sendingPartnerId) << "(" << sendingPartnerId << ")" << " for wyphon handle: " << pWyphonPartner << " " << wyphonPartnerHandle << " hLMBP=" << localMessageBroadcastPartnerHandle << "\n";
		
		
		//send him a list of everything we are currently sharing
//		map<HANDLE, WyphonD3DTextureInfo* > * texturesMap =  &(*(pWyphonPartner->sharedByUsD3DTexturesMap));
		map<HANDLE, WyphonD3DTextureInfo* > * texturesMap =  pWyphonPartner->sharedByUsD3DTexturesMap;
		map<HANDLE, WyphonD3DTextureInfo*>::iterator itr;

		for ( itr = (*texturesMap).begin(); itr != (*texturesMap).end(); ++itr ) {
			BYTE * data;
			int dataSize = CreateShareD3DTextureMessage(pWyphonPartner, itr->second, &data);
			
			bool success = dataSize > 0;
	
			if ( success ) {
				success = BroadcastMessage( pWyphonPartner->hLocalMessageBroadcastPartner, data, dataSize );
				delete data; 
			}
		}
		
		return true;
	}
	
	bool PartnerLeftCallback( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, HANDLE wyphonPartnerHandle ) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

		wcout << "Wyphon: PartnerLeftCallback: " <<  GetBroadcastPartnerName(localMessageBroadcastPartnerHandle, sendingPartnerId) << "(" << sendingPartnerId << ")" << " for wyphon handle: " << pWyphonPartner << " " << wyphonPartnerHandle << " hLMBP=" << localMessageBroadcastPartnerHandle << "\n";

		//forget about all textures shared by this partner
		return RemoveAllD3DTexturesSharedByPartner(pWyphonPartner, sendingPartnerId);		
	}
	
	bool MessageReceivedCallback( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * msgData, unsigned int msgLength, HANDLE wyphonPartnerHandle ) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

		//check the message type, and process the message
		switch ( *((BYTE*)msgData) ) {
			case WYPHON_START_SHARING_D3DTEXTURE:
				return ProcessReceivedShareD3DTextureMessage( pWyphonPartner, sendingPartnerId, (BYTE*)msgData, msgLength );
				break;
			case WYPHON_STOP_SHARING_D3DTEXTURE:
				return ProcessReceivedUnshareD3DTextureMessage( pWyphonPartner, sendingPartnerId, (BYTE*)msgData, msgLength );
				break;
			default:
				wcout << "Wyphon: Unknown message type." << "\n";
				return false;
		}
	}




	extern "C" _declspec(dllexport)
	bool ShareD3DTexture(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD usage, LPTSTR description) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

		wcout << "Wyphon: ShareD3DTexture with handle=" << sharedTextureHandle << " " << width << "x" << height << "\n";

		WyphonD3DTextureInfo * pTextureInfo = CreateWyphonD3DTextureInfo( sharedTextureHandle, width, height, usage, _tcslen(description), description );
		
		BYTE * data;
		int dataSize = CreateShareD3DTextureMessage(pWyphonPartner, pTextureInfo, &data);

		wcout << "Wyphon: CreateShareD3DTextureMessage done..." << "\n";
		
		bool success = dataSize > 0;

		if ( success ) {
			success = BroadcastMessage( pWyphonPartner->hLocalMessageBroadcastPartner, data, dataSize );
			(*(pWyphonPartner->sharedByUsD3DTexturesMap))[sharedTextureHandle] = pTextureInfo;
			delete data; 
		}

		wcout << "Wyphon: Shared texture with handle=" << pTextureInfo->hSharedTexture << " " << pTextureInfo->width << "x" << pTextureInfo->height << "\n";
		
		return success;
	}

	extern "C" _declspec(dllexport)
	bool UnshareD3DTexture(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

		BYTE * data;
		int dataSize = CreateUnshareTextureMessage(pWyphonPartner, sharedTextureHandle, &data);
		
		bool success = dataSize > 0;
		if ( success ) {
			success = BroadcastMessage( pWyphonPartner->hLocalMessageBroadcastPartner, data, dataSize );
			delete data;
			
			delete (*(pWyphonPartner->sharedByUsD3DTexturesMap))[sharedTextureHandle];
			pWyphonPartner->sharedByUsD3DTexturesMap->erase(sharedTextureHandle);
		}
		
		return success;	
	}



	/// unshare all of the resources that are shared by us
	bool UnshareAllD3DTexturesSharedByUs(WyphonPartnerDescriptor * pWyphonPartner) {
		map<HANDLE, WyphonD3DTextureInfo* > * texturesMap =  &(*(pWyphonPartner->sharedByUsD3DTexturesMap));
		map<HANDLE, WyphonD3DTextureInfo*>::iterator itr;

		for ( itr = (*texturesMap).begin(); itr != (*texturesMap).end(); itr = (*texturesMap).begin() /*because current gets deleted !!!*/ ) {
			UnshareD3DTexture( pWyphonPartner, itr->first );
		}
		
		return true;
	}


	
	
	extern "C" _declspec(dllexport)
	bool DestroyWyphonPartner(HANDLE wyphonPartnerHandle) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

		bool success = true;

		wcout << "Wyphon: RemoveAllD3DTexturesSharedByUs " << "\n";
		success = success && UnshareAllD3DTexturesSharedByUs(pWyphonPartner);
//		success = success && RemoveAllD3DTexturesSharedByUs(pWyphonPartner);

		wcout << "Wyphon: RemoveAllD3DTexturesSharedBy ANY Partner " << "\n";
		map<unsigned int, map<HANDLE, WyphonD3DTextureInfo*>>::iterator itr;
		for ( itr = pWyphonPartner->sharedByPartnersD3DTexturesMap->begin(); itr != pWyphonPartner->sharedByPartnersD3DTexturesMap->end(); itr = pWyphonPartner->sharedByPartnersD3DTexturesMap->begin() /*because current gets deleted !!!*/ ) {
			wcout << "Wyphon: RemoveAllD3DTexturesSharedByPartner " << itr->first << "\n";
			success = success && RemoveAllD3DTexturesSharedByPartner(pWyphonPartner, itr->first );
		}


		wcout << "Wyphon: Trying to DestroyLocalMessageBroadcastPartner" << "\n";
		
		success = success && DestroyLocalMessageBroadcastPartner( pWyphonPartner->hLocalMessageBroadcastPartner );
		pWyphonPartner->hLocalMessageBroadcastPartner = NULL;
		

		wcout << "Wyphon: Trying to delete all memory that has been allocated..." << "\n";

		delete pWyphonPartner->sharedByUsD3DTexturesMap;
		delete pWyphonPartner->sharedByPartnersD3DTexturesMap;
		delete pWyphonPartner->applicationName;
		
		delete pWyphonPartner;
		
		wcout << "Wyphon: Done..." << "\n";
		return success;
	}



	extern "C" _declspec(dllexport)
	HANDLE CreateWyphonPartner( LPTSTR applicationName
								, LPD3DTextureSharingStartedCALLBACK pD3DTextureSharingStartedCallbackFunc 
								, LPD3DTextureSharingStoppedCALLBACK pD3DTextureSharingStoppedCallbackFunc 
	 							) {

		WyphonPartnerDescriptor * pWyphonPartner = new WyphonPartnerDescriptor();

		pWyphonPartner->applicationName = new wstring( applicationName );

		wcout << "Wyphon: Application Name = " << applicationName << " or in *(pWyphonPartner->applicationName) = " << *(pWyphonPartner->applicationName) << "\n";


 		LPTSTR lpLocalMessageBroadcastName = TEXT(WYPHON_LOCALMESSAGEBROADCAST_NAME);

			
		pWyphonPartner->sharedByUsD3DTexturesMap = new map<HANDLE, WyphonD3DTextureInfo* >();
		pWyphonPartner->sharedByPartnersD3DTexturesMap = new map<unsigned int, map<HANDLE, WyphonD3DTextureInfo * >>();
		
		pWyphonPartner->pD3DtextureSharingStartedCallbackFunc = pD3DTextureSharingStartedCallbackFunc;
		pWyphonPartner->pD3DtextureSharingStoppedCallbackFunc = pD3DTextureSharingStoppedCallbackFunc;

		wcout << "Wyphon: pWyphonPartner=" << pWyphonPartner << "\n";

//		wcout << "Wyphon: Try to CreateLocalMessageBroadcastPartner with " << lpLocalMessageBroadcastName << " and callbackFuncCustomData=" << pWyphonPartner << "\n";
		pWyphonPartner->hLocalMessageBroadcastPartner =
			CreateLocalMessageBroadcastPartner( lpLocalMessageBroadcastName, applicationName 
													, pWyphonPartner
													, (LPPartnerJoinedCALLBACK)PartnerJoinedCallback
													, (LPPartnerJoinedCALLBACK)PartnerLeftCallback
													, (LPMessageReceivedCALLBACK)MessageReceivedCallback
												);
		wcout << "Wyphon: CreateLocalMessageBroadcastPartner returned " << pWyphonPartner->hLocalMessageBroadcastPartner << "\n";

		wcout << "Wyphon: Finally return the handle " << pWyphonPartner << " for partner " << *(pWyphonPartner->applicationName) << "\n";
		
		//Sleep(20000L);

		if (pWyphonPartner->pD3DtextureSharingStartedCallbackFunc != pD3DTextureSharingStartedCallbackFunc) {
			wcout << "Wyphon: PROBLEM pWyphonPartner->pD3DtextureSharingStartedCallbackFunc=" << pWyphonPartner->pD3DtextureSharingStartedCallbackFunc << " != " << pD3DTextureSharingStartedCallbackFunc << "\n";
		}
		
		return pWyphonPartner;
	}

	extern "C" _declspec(dllexport)
	LPCTSTR GetWyphonPartnerName(HANDLE wyphonPartnerHandle, unsigned int wyphonPartnerId) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

		return GetBroadcastPartnerName(pWyphonPartner->hLocalMessageBroadcastPartner, wyphonPartnerId);
	}

	extern "C" _declspec(dllexport)
	bool GetD3DTextureInfo(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle, unsigned int & wyphonPartnerId, unsigned int & width, unsigned int & height, DWORD & usage, LPTSTR description, int maxDescriptionLength ) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;
		
		bool found = false;
		
		map<unsigned int, map<HANDLE, WyphonD3DTextureInfo*>> * tMap = pWyphonPartner->sharedByPartnersD3DTexturesMap;
		map<unsigned int, map<HANDLE, WyphonD3DTextureInfo*>>::iterator itr;
		for ( itr = tMap->begin(); itr != tMap->end(); itr++ ) {
			wcout << "Wyphon: GetD3DTextureInfo " << itr->first << "\n";
						
			if ( (itr->second)[sharedTextureHandle] != NULL ) {
				wyphonPartnerId = itr->first;
				WyphonD3DTextureInfo * pTextureInfo = (itr->second)[sharedTextureHandle];
				width = pTextureInfo->width;
				height = pTextureInfo->height;
				usage = pTextureInfo->usage;
				
				found = true;
				break;
			}
		}

		return found;
	}
}