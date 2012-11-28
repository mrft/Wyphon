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

using namespace std;

using namespace System;


//	Define the Functions in the DLL for reuse. This is just prototyping the dll's function. 
//	A mock of it. Use "stdcall" for maximum compatibility.

typedef bool ( * LPPartnerJoinedCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * customData );
typedef bool ( * LPPartnerLeftCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * customData);
typedef bool ( * LPMessageReceivedCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * msgData, unsigned int msgLength, void * customData );


typedef bool (__stdcall * pDestroyLocalMessageBroadcastPartnerFUNC)(HANDLE localMessageBroadcastPartnerHandle);

typedef HANDLE (__stdcall * pCreateLocalMessageBroadcastPartnerFUNC)( LPTSTR lpSharedMemoryName, LPTSTR myName
	, void * callbackFuncCustomData
	, LPPartnerJoinedCALLBACK pPartnerJoinedCallbackFunc
	, LPPartnerLeftCALLBACK pPartnerLeftCallbackFunc
	, LPMessageReceivedCALLBACK pMsgReceivedCallbackFunc 
);

typedef bool (__stdcall * pBroadcastMessageFUNC)(HANDLE localMessageBroadcastPartnerHandle, void * data, unsigned int length);

typedef LPCTSTR (__stdcall * pGetBroadcastPartnerNameFUNC)(HANDLE localMessageBroadcastPartnerHandle, unsigned int partnerId);


HINSTANCE hLocalMessageBroadcastDll = LoadLibrary(TEXT("LocalMessageBroadcast.dll"));

// get pointers to the functions in the dll
///////////////////////////////////////////

//if ( hLocalMessageBroadcastDll != NULL ) {
	FARPROC lpfnDestroyLocalMessageBroadcastPartner = GetProcAddress(HMODULE (hLocalMessageBroadcastDll),"DestroyLocalMessageBroadcastPartner");
	FARPROC lpfnCreateLocalMessageBroadcastPartner = GetProcAddress(HMODULE (hLocalMessageBroadcastDll),"CreateLocalMessageBroadcastPartner");
	FARPROC lpfnBroadcastMessage = GetProcAddress(HMODULE (hLocalMessageBroadcastDll),"BroadcastMessage");
	FARPROC lpfnGetBroadcastPartnerName = GetProcAddress(HMODULE (hLocalMessageBroadcastDll),"GetBroadcastPartnerName");

	pDestroyLocalMessageBroadcastPartnerFUNC DestroyLocalMessageBroadcastPartner = pDestroyLocalMessageBroadcastPartnerFUNC(lpfnDestroyLocalMessageBroadcastPartner);
	pCreateLocalMessageBroadcastPartnerFUNC CreateLocalMessageBroadcastPartner = pCreateLocalMessageBroadcastPartnerFUNC(lpfnCreateLocalMessageBroadcastPartner);
	pBroadcastMessageFUNC BroadcastMessage = pBroadcastMessageFUNC(lpfnBroadcastMessage);
	pGetBroadcastPartnerNameFUNC GetBroadcastPartnerName = pGetBroadcastPartnerNameFUNC(lpfnGetBroadcastPartnerName);
//}

//this code gets run whenever the dll gets loaded or unloaded, so we can do the setup/teardown in here
#pragma unmanaged
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) 
{
	switch( fdwReason ) 
	{ 
		case DLL_PROCESS_ATTACH:
		break;
		case DLL_THREAD_ATTACH:
		break;
		case DLL_THREAD_DETACH:
		break;
		case DLL_PROCESS_DETACH:
		break;
	}
	
	return TRUE;  
}



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

		//wcout << "Wyphon: Texture shared by "<< mailslotId << " handle=" << pTextureInfo->hSharedTexture << " " << pTextureInfo->width << "x" << pTextureInfo->height << " named '" << pTextureInfo->description << "'" << "\n";
		
		return AddD3DTextureSharedByPartner(pWyphonPartner, sendingPartnerId, pTextureInfo);
	}

	/// Remove this texture from the list of 'shared-by-partners' textures
	bool ProcessReceivedUnshareD3DTextureMessage(WyphonPartnerDescriptor * pWyphonPartner, unsigned int sendingPartnerId, BYTE * data, unsigned int length ) {
	
		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		int len = sizeof(HANDLE);
		HANDLE textureHandle = *((HANDLE *)pData);
		
		wcout << "Wyphon: Texture NOT shared ANYMORE  by "<< sendingPartnerId << " handle=" << textureHandle << "\n";

		return RemoveD3DTextureSharedByPartner(pWyphonPartner, sendingPartnerId, textureHandle);
	}



	bool PartnerJoinedCallback( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, HANDLE wyphonPartnerHandle ) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

		wcout << "Wyphon: PartnerJoinedCallback for wyphon handle: " << pWyphonPartner << " " << wyphonPartnerHandle << " hLMBP=" << localMessageBroadcastPartnerHandle << " sendingPartnerId=" << sendingPartnerId << "\n";
		

		//send him a list of everything we are currently sharing
//		map<HANDLE, WyphonD3DTextureInfo* > * texturesMap =  &(*(pWyphonPartner->sharedByUsD3DTexturesMap));
		map<HANDLE, WyphonD3DTextureInfo* > * texturesMap =  pWyphonPartner->sharedByUsD3DTexturesMap;
//		map<HANDLE, WyphonD3DTextureInfo*>::iterator itr;
//
//		for ( itr = (*texturesMap).begin(); itr != (*texturesMap).end(); ++itr ) {
//			BYTE * data;
//			int dataSize = CreateShareD3DTextureMessage(pWyphonPartner, itr->second, &data);
//			
//			bool success = dataSize > 0;
//	
//			if ( success ) {
//				success = BroadcastMessage( pWyphonPartner->hLocalMessageBroadcastPartner, data, dataSize );
//				delete data; 
//			}
//		}
		
		return true;
	}
	
	bool PartnerLeftCallback( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, HANDLE wyphonPartnerHandle ) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

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
	bool DestroyWyphonPartner(HANDLE wyphonPartnerHandle) {
		WyphonPartnerDescriptor * pWyphonPartner = (WyphonPartnerDescriptor *) wyphonPartnerHandle;

		bool success = true;
		wcout << "Wyphon: Trying to DestroyLocalMessageBroadcastPartner" << "\n";
		
		DestroyLocalMessageBroadcastPartner( pWyphonPartner->hLocalMessageBroadcastPartner );
		pWyphonPartner->hLocalMessageBroadcastPartner = NULL;
		
		wcout << "Wyphon: Trying to delete all memory that has been allocated..." << "\n";

		delete pWyphonPartner->applicationName;

		map<unsigned int, map<HANDLE, WyphonD3DTextureInfo*>>::iterator itr;
		for ( itr = pWyphonPartner->sharedByPartnersD3DTexturesMap->begin(); itr != pWyphonPartner->sharedByPartnersD3DTexturesMap->end(); itr = pWyphonPartner->sharedByPartnersD3DTexturesMap->begin() /*because current gets deleted !!!*/ ) {
			wcout << "Wyphon: RemoveAllD3DTexturesSharedByPartner " << itr->first << "\n";
			RemoveAllD3DTexturesSharedByPartner(pWyphonPartner, itr->first );
		}
		delete pWyphonPartner->sharedByPartnersD3DTexturesMap;

		wcout << "Wyphon: RemoveAllD3DTexturesSharedByUs " << "\n";
		RemoveAllD3DTexturesSharedByUs(pWyphonPartner);
		delete pWyphonPartner->sharedByUsD3DTexturesMap;
		
		delete pWyphonPartner;
		
		//wcout << "Wyphon: Done..." << "\n";
		
		return success;
	}



	extern "C" _declspec(dllexport)
	HANDLE CreateWyphonPartner( LPTSTR applicationName
								, LPD3DTextureSharingStartedCALLBACK pD3DTextureSharingStartedCallbackFunc 
								, LPD3DTextureSharingStoppedCALLBACK pD3DTextureSharingStoppedCallbackFunc 
	 							) {

		//For some reason I don't understand, when the function CreateSharedMemory 
		//gets called in the dll LocalMessageBroadcast, some of our local variables 
		//get OVERWRITTEN (!) by the values given to the second and third parameter (startSize and maxSize) of that function
		
		//this way, we hope to make sure an unused parameter gets overwritten
		WyphonPartnerDescriptor * pWyphonPartner01 = NULL;
		WyphonPartnerDescriptor * pWyphonPartner02 = NULL;
//		WyphonPartnerDescriptor * pWyphonPartner03 = NULL;
//		WyphonPartnerDescriptor * pWyphonPartner04 = NULL;
//		WyphonPartnerDescriptor * pWyphonPartner05 = NULL;
//		WyphonPartnerDescriptor uselessWyphonPartnerArray[128];

		WyphonPartnerDescriptor * pWyphonPartner = new WyphonPartnerDescriptor();

		pWyphonPartner->applicationName = new wstring( applicationName );

		wcout << "Wyphon: Application Name = " << applicationName << " or in *(pWyphonPartner->applicationName) = " << *(pWyphonPartner->applicationName) << "\n";


 		LPTSTR lpLocalMessageBroadcastName = TEXT(WYPHON_LOCALMESSAGEBROADCAST_NAME);

		wcout << "Wyphon: Try to CreateLocalMessageBroadcastPartner with " << lpLocalMessageBroadcastName << " and callbackFuncCustomData=" << pWyphonPartner << "\n";
	
		HANDLE hWyphonPartner = (HANDLE)pWyphonPartner;
		wcout << "Wyphon: pWyphonPartner=" << pWyphonPartner << "\n";


		wcout << "Wyphon: CreateLocalMessageBroadcastPartner returned " << pWyphonPartner->hLocalMessageBroadcastPartner << "\n";
		
		pWyphonPartner->sharedByUsD3DTexturesMap = new map<HANDLE, WyphonD3DTextureInfo* >();
		pWyphonPartner->sharedByPartnersD3DTexturesMap = new map<unsigned int, map<HANDLE, WyphonD3DTextureInfo * >>();
		wcout << "Wyphon: pWyphonPartner=" << pWyphonPartner << "\n";
		
		pWyphonPartner->pD3DtextureSharingStartedCallbackFunc = pD3DTextureSharingStartedCallbackFunc;
		pWyphonPartner->pD3DtextureSharingStoppedCallbackFunc = pD3DTextureSharingStoppedCallbackFunc;

		unsigned int * ppWyphonPartner2 = new unsigned int[1];
		ppWyphonPartner2[0] = *((unsigned int*)&pWyphonPartner);
		unsigned int pWyphonPartner2 = (unsigned int)pWyphonPartner01;
		unsigned int pWyphonPartner3 = (unsigned int)pWyphonPartner02;
		wcout << "Wyphon: pWyphonPartner=" << pWyphonPartner << " " << pWyphonPartner2 << " " << pWyphonPartner3 << "\n";

		HANDLE hLocalMessageBroadcastPartner =
			CreateLocalMessageBroadcastPartner( lpLocalMessageBroadcastName, applicationName 
													, NULL //pWyphonPartner
													, NULL //(LPPartnerJoinedCALLBACK)PartnerJoinedCallback
													, NULL //(LPPartnerJoinedCALLBACK)PartnerLeftCallback
													, NULL //(LPMessageReceivedCALLBACK)MessageReceivedCallback
												);
		wcout << "Wyphon: pWyphonPartner=" << pWyphonPartner << " " << pWyphonPartner2 << " " << pWyphonPartner3 << "\n";
		//pWyphonPartner->hLocalMessageBroadcastPartner = hLocalMessageBroadcastPartner;

//			CreateLocalMessageBroadcastPartner( lpLocalMessageBroadcastName, TEXT("JOS") //applicationName 
//													, NULL
//													, NULL //(LPPartnerJoinedCALLBACK)PartnerJoinedCallback
//													, NULL //(LPPartnerJoinedCALLBACK)PartnerLeftCallback
//													, NULL //(LPMessageReceivedCALLBACK)MessageReceivedCallback
//												);
//		wcout << "Wyphon: pWyphonPartner=" << pWyphonPartner << " " << pWyphonPartner2 << " " << pWyphonPartner3 << "\n";
//
//			CreateLocalMessageBroadcastPartner( lpLocalMessageBroadcastName, TEXT("JOS") //applicationName 
//													, NULL
//													, NULL //(LPPartnerJoinedCALLBACK)PartnerJoinedCallback
//													, NULL //(LPPartnerJoinedCALLBACK)PartnerLeftCallback
//													, NULL //(LPMessageReceivedCALLBACK)MessageReceivedCallback
//												);
//		wcout << "Wyphon: pWyphonPartner=" << pWyphonPartner << " " << pWyphonPartner2 << " " << pWyphonPartner3 << "\n";

		wcout << "Wyphon: Finally return the handle " << pWyphonPartner << " ?=(HANDLE)" << hWyphonPartner << " ?=(void*)" << (void*)pWyphonPartner << "\n";
		
		//Sleep(20000L);

		if (pWyphonPartner->pD3DtextureSharingStartedCallbackFunc != pD3DTextureSharingStartedCallbackFunc) {
			wcout << "Wyphon: pWyphonPartner->pD3DtextureSharingStartedCallbackFunc=" << pWyphonPartner->pD3DtextureSharingStartedCallbackFunc << " != " << pD3DTextureSharingStartedCallbackFunc << "\n";
		}
		wcout << "Wyphon: appName=" << pWyphonPartner->applicationName << "\n";
		
		return pWyphonPartner;
	}


	extern "C" _declspec(dllexport)
	bool ShareD3DTexture(HANDLE wyphonPartnerHandle, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD usage, LPTSTR description) {
		wcout << "Wyphon: ShareD3DTexture with handle=" << sharedTextureHandle << " " << width << "x" << height << "\n";

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


}