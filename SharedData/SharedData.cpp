/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 26/10/2012
 * Time: 13:10
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
// SharedData.cpp
#include "SharedData.h"
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <d3d9types.h>
//#include <thread>
#include <process.h>

using namespace std;

//	Define the Functions in the DLL for reuse. This is just prototyping the dll's function. 
//	A mock of it. Use "stdcall" for maximum compatibility.
typedef bool (__stdcall * pLockSharedMemoryFUNC)(HANDLE sharedDataHandle, unsigned int timeoutInMilliseconds);
typedef bool (__stdcall * pUnlockSharedMemoryFUNC)(HANDLE sharedDataHandle);
typedef int (__stdcall * pWriteSharedMemoryFUNC)( HANDLE sharedDataHandle, BYTE* data, unsigned int length, unsigned int offset );
typedef int (__stdcall * pReadSharedMemoryFUNC)( HANDLE sharedDataHandle, BYTE * &pData );
typedef int (__stdcall * pWriteStringToSharedMemoryFUNC)( HANDLE sharedDataHandle, LPTSTR data );
typedef LPTSTR (__stdcall * pReadStringFromSharedMemoryFUNC)( HANDLE sharedDataHandle );
typedef bool (__stdcall * pDestroySharedMemoryFUNC)(HANDLE sharedDataHandle);
typedef HANDLE (__stdcall * pCreateSharedMemoryFUNC)(LPTSTR lpName, unsigned int startSize, unsigned int maxSize);


HINSTANCE hSharedMemoryDll = LoadLibrary(TEXT("SharedMemory.dll"));

// get pointers to the functions in the dll
///////////////////////////////////////////
//	bool LockSharedMemory(HANDLE sharedDataHandle, unsigned int timeoutInMilliseconds) {
//	bool UnlockSharedMemory(HANDLE sharedDataHandle) {
//	int WriteSharedMemory( HANDLE sharedDataHandle, BYTE* data, unsigned int length, unsigned int offset ) {
//	int ReadSharedMemory( HANDLE sharedDataHandle, BYTE * &pData ) {
//	int WriteStringToSharedMemory( HANDLE sharedDataHandle, LPTSTR data ) {
//	LPTSTR ReadStringFromSharedMemory( HANDLE sharedDataHandle ) {
//	bool DestroySharedMemory(HANDLE sharedDataHandle) {
//	HANDLE CreateSharedMemory(LPTSTR lpName, unsigned int startSize, unsigned int maxSize) {

//if ( hSharedMemoryDll != NULL ) {
	FARPROC lpfnLockSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"LockSharedMemory");
	FARPROC lpfnUnlockSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"UnlockSharedMemory");
	FARPROC lpfnWriteSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"WriteSharedMemory");
	FARPROC lpfnReadSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"ReadSharedMemory");
	FARPROC lpfnDestroySharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"DestroySharedMemory");
	FARPROC lpfnCreateSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"CreateSharedMemory");


	pLockSharedMemoryFUNC LockSharedMemory = pLockSharedMemoryFUNC(lpfnLockSharedMemory);
	pUnlockSharedMemoryFUNC UnlockSharedMemory = pUnlockSharedMemoryFUNC(lpfnUnlockSharedMemory);
	pWriteSharedMemoryFUNC WriteSharedMemory = pWriteSharedMemoryFUNC(lpfnWriteSharedMemory);
	pReadSharedMemoryFUNC ReadSharedMemory = pReadSharedMemoryFUNC(lpfnReadSharedMemory);
	pDestroySharedMemoryFUNC DestroySharedMemory = pDestroySharedMemoryFUNC(lpfnDestroySharedMemory);
	pCreateSharedMemoryFUNC CreateSharedMemory = pCreateSharedMemoryFUNC(lpfnCreateSharedMemory);
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
	The SharedDataPartner descriptor should hold all of the necessary information 
	to be able to send the data to all of the other SharedDataPartners.
	- 	The first will be the hSharedMemory, which will hold the SharedMemory handle. 
		In this sharedData, we will store the HANDLE of the MAILSLOT that should be written 
		to for each Partner.
		If a Partner disappears, he should ideally clear this field (set to 0), but if 
		he forgets it, it would be cool if another one that detects a MAILSLOT going down would fix this.
		This is only used for 'newcomers', who get a list of MAILSLOTS they should set up for writing their 
		messages too.
		Through all of these they should let everybody know that they are here, and that they 
		should be contacted through the mailslot with NAME \\mailslot\.\Wyphon_xxxx 
		(xxxx being the decimal representation of the unisgned integer in the shared memory).
		These mailslots are also needed to let 
		everybody else know when we are sharing a new texture, or when we stop sharing a certain texture
		, or when we are quitting.
	- 	As a consequence, the second will be a list of mailslots that we should communicate over.

	So, we are setting up mailslot communication with everyone else who is participating. Every message will 
	be sent to every partner.
	There will be 5 types of messages:
	- Hello, I am new. My name isxxx, please talk to me on this channel (mailslot handle?).
	- Welcome, new partner! My name is xxx.
	- Hey, I have a texture to share with all of you. Here are its coördinates. (width, height, handle, usage, ...)
	- Hey, the following texture will no longer be valid, here's the handle.
	- Goodbye, please stop talking to me (and forget about all my textures, but I should have told you so before).
	  Whenever an application sends out this goodbye message, it should have cleaned up the shared memory first.
	
	A message will have 1 byte telling us the message_type
	and some other bytes depending on that type, with needed info:
	- WYPHON_HELLO (1)					|	partnerIdentifier (sizeof unsigned int)	|	name_length (size_t) 	| name
	- WYPHON_WELCOME (1)				|	partnerIdentifier (sizeof unsigned int)	|	name_length (size_t) 	| name
	- WYPHON_START_SHARING_TEXTURE (1) 	|	SharedObjectInfo (sizeof SharedObjectInfo)
	- WYPHON_STOP_SHARING_TEXTURE (1)	|	sharedHandle (sizeof HANDLE)
	- WYPHON_GOODBYE					|	partnerIdentifier
	
)
*/


namespace SharedData {



	// IF WE ASSUME that more types can be shared in the future
	// we will add message_types and structs to describe these resources
	// We'll start with textures only at this time

	#define SHARED_DATA_HELLO 						1
	#define SHARED_DATA_WELCOME						2
	#define SHARED_DATA_SHARE		 				10
	#define SHARED_DATA_UNSHARE				 		11
	#define SHARED_DATA_GOODBYE 					99

//	#define SHARED_DATA_SHAREDDATA_NAME				"Wyphon"

	#define SHARED_DATA_INITIAL_NUMBER_OF_PARTNERS 	2
	#define SHARED_DATA_MAX_NUMBER_OF_PARTNERS 		128


	//FOR INTERNAL USE ONLY
	struct SharedObjectInfo {
		HANDLE hObject; // we need a simple identifier for each object
		unsigned int dataSize; //rendertarget or ...
		void * pData;
	};


	//FOR INTERNAL USE ONLY
	struct SharedDataPartnerDescriptor {
		HANDLE hSharedMemory;

		wstring * sharedMemoryName;
		
		wstring * partnerName;

		unsigned int partnerIdentifier; //used to generate out mailsot name, and shared with the other partners
		
		HANDLE hReaderMailslot; //handle to the mailslot we are listening on

		bool listenerThreadRunning;

		map<unsigned int, HANDLE> * writerMailslotHandlesMap;	//unsigned int = the partner's partnerIdentifier
																//HANDLE = the handles to the mailslots we should write to

		map<unsigned int, wstring> * writerMailslotNamesMap;	//unsigned int = the partner's partnerIdentifier
																//wstring = name of the partner

		map<HANDLE, SharedObjectInfo * > * sharedByUsObjectsMap; //HANDLE = SHARED TEXTURE HANDLE

		//unsigned int = the partner's partnerIdentifier (so we know which partner shared this object)
		//HANDLE = the shared object handle pointing to a sharedTextureInfo struct
		map<unsigned int, map<HANDLE, SharedObjectInfo * >> * sharedByPartnersObjectsMap;								

	};
	




	
	



	/// since we will only send unsigned integers around, the mailslot name 
	/// will be constructed from a string and the decimal representation of this number...
	wstring CreateMailslotName(unsigned int identifier) {
		//wstring retVal;
		
		wstringstream ss;
    	ss << "\\\\.\\mailslot\\Wyphon\\" << identifier;
		
		//retVal = ss.str();
		
		return ss.str(); //retVal;
	}


	///in the list of ids (from shared memory) find an 'untaken' id that we can use, and find an empty spot 
	///to write it back to shared memory afterwards
	bool FindMailslotIdAndEmptySharedMemorySpot(unsigned int * ids, int nrOfIds, int & emptySpotPosition, unsigned int & newReaderMailslotIdentifier) {
		
		//find an empty spot (=0), if there is one
		int pos = -1;
		unsigned int maxId = 0; //our id should be the max + 1 from the already existing identifiers

		for ( int i = 0; i < nrOfIds; i++ ) {
			if ( ids[i] == 0 && pos == -1) {
				pos = i;
			}
			if ( ids[i] > maxId ) {
				maxId = ids[i];
			}			
			//wcout << "ids[" << j << "] = " << ids[j] << " AND emptySpotPosition = " << emptySpotPosition << "\n";
		}
		if ( pos == -1 && nrOfIds < SHARED_DATA_MAX_NUMBER_OF_PARTNERS ) {
			pos = nrOfIds;
		}


		if ( pos == -1 || pos >= SHARED_DATA_MAX_NUMBER_OF_PARTNERS ) {
			return false;
		}
		
		emptySpotPosition = pos;
		newReaderMailslotIdentifier = maxId + 1;
		
		return true;
	}

	/// Closes and removes 1 single file handle for writing to partner mailslots
	bool DestroyWriterMailslot(SharedDataPartnerDescriptor * pSharedDataPartner, unsigned int partnerIdentifier) {
		
		map<unsigned int, HANDLE> * handlesMap = pSharedDataPartner->writerMailslotHandlesMap;
		map<unsigned int, wstring> * namesMap = pSharedDataPartner->writerMailslotNamesMap;
	
		CloseHandle( (*handlesMap)[partnerIdentifier] );
		
		handlesMap->erase(partnerIdentifier);
		namesMap->erase(partnerIdentifier);
		
		return true;
	}

	/// Remove 1 object shared by a partner given its partnerId and the objectHandle
	bool RemoveObjectSharedByPartner(SharedDataPartnerDescriptor * pSharedDataPartner, unsigned int partnerId, HANDLE objectHandle) {
		map<HANDLE, SharedObjectInfo * > * objectsMap =  &(*(pSharedDataPartner->sharedByPartnersObjectsMap))[partnerId];
		
		if ( objectsMap->count(objectHandle) > 0 ) {
			delete (*objectsMap)[objectHandle]->pData;
			delete (*objectsMap)[objectHandle];
			objectsMap->erase(objectHandle);
		}
		
		return true;
	}

	/// Add 1 object shared by a partner given its partnerId and the SharedObjectInfo struct
	bool AddObjectSharedByPartner(SharedDataPartnerDescriptor * pSharedDataPartner, unsigned int partnerId, SharedObjectInfo * pObject, int length) {
		map<HANDLE, SharedObjectInfo * > * objectsMap =  &(*(pSharedDataPartner->sharedByPartnersObjectsMap))[partnerId];
		
		(*objectsMap)[pObject->hObject] = pObject;
		
		return true;
	}


	/// cleans up the resources that are shared with us by the given partner's partnerId
	bool RemoveAllObjectsSharedByPartner(SharedDataPartnerDescriptor * pSharedDataPartner, unsigned int partnerId) {	
		map<HANDLE, SharedObjectInfo * > * objectsMap =  &(*(pSharedDataPartner->sharedByPartnersObjectsMap))[partnerId];
		map<HANDLE, SharedObjectInfo*>::iterator itr;

		for ( itr = (*objectsMap).begin(); itr != (*objectsMap).end(); ++itr ) {
			delete itr->second->pData;
			delete itr->second;
		}
		objectsMap->clear();
	
		pSharedDataPartner->sharedByPartnersObjectsMap->erase(partnerId);
		
		return true;
	}

	/// Remove 1 object shared by us given the objectHandle
	bool RemoveObjectSharedByUs(SharedDataPartnerDescriptor * pSharedDataPartner, HANDLE objectHandle) {
		map<HANDLE, SharedObjectInfo* > * objectsMap =  &(*(pSharedDataPartner->sharedByUsObjectsMap));
		
		if ( objectsMap->count(objectHandle) > 0 ) {
			delete (*objectsMap)[objectHandle]->pData;
			delete (*objectsMap)[objectHandle];
			objectsMap->erase(objectHandle);
		}
		
		return true;
	}

	/// cleans up all of the resources that are shared by us
	bool RemoveAllObjectsSharedByUs(SharedDataPartnerDescriptor * pSharedDataPartner) {	
		map<HANDLE, SharedObjectInfo* > * objectsMap =  &(*(pSharedDataPartner->sharedByUsObjectsMap));
		map<HANDLE, SharedObjectInfo*>::iterator itr;

		for ( itr = (*objectsMap).begin(); itr != (*objectsMap).end(); ++itr ) {
			delete itr->second->pData;
			delete itr->second;
		}
		objectsMap->clear();
		
		return true;
	}

	/// Closes and removes file handles for writing to the partner mailslots
	bool DestroyWriterMailslots(HANDLE SharedDataPartnerHandle) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;
		
		map<unsigned int, HANDLE>::const_iterator itr;
		
		for( itr = pSharedDataPartner->writerMailslotHandlesMap->begin(); itr != pSharedDataPartner->writerMailslotHandlesMap->end(); ++itr ) {
			wcout << "writerMailslotId: " << (*itr).first << " Handle: " << (*itr).second;
			
			CloseHandle( itr->second );
		}
		
		pSharedDataPartner->writerMailslotHandlesMap->clear();
		pSharedDataPartner->writerMailslotNamesMap->clear();
		
//		while ( pSharedDataPartner->hWriterMailslotList.front() != NULL ) {
//			HANDLE hFile = pSharedDataPartner->hWriterMailslotList.front();
//
//			CloseHandle(hFile);
//			
//			pSharedDataPartner->hWriterMailslotList.pop_front();
//		}
//		
//		pSharedDataPartner->hWriterMailslotList.clear();
		
		return TRUE;
	}


	///Create 1 single writermailslot, and put it into the list of known partners
	bool CreateWriterMailslot(SharedDataPartnerDescriptor * pSharedDataPartner, unsigned int id) {

		wstring name = CreateMailslotName(id);

		wcout << "CreateWriterMailslot named '" << name << "'\n";

		//now open the mailslot
		HANDLE hFile;

		hFile = CreateFile( name.c_str(), 
							GENERIC_WRITE, 
							FILE_SHARE_READ,
							(LPSECURITY_ATTRIBUTES) NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL, 
							(HANDLE) NULL); 
	
		if (hFile == INVALID_HANDLE_VALUE) { 
			wcout << "CreateFile failed with " << GetLastError() << ".\n";
						
			return false; 
		}
		
		(*pSharedDataPartner->writerMailslotHandlesMap)[id] = hFile; 
		
		//We don't know the name yet, so don't fill NamesMap yet...
		////(*pSharedDataPartner->writerMailslotNamesMap)[id] = *(new wstring());
		
		return true;
	}


	/// Creates file handles for writing to all of the partner mailslots
	BOOL CreateWriterMailslots(HANDLE SharedDataPartnerHandle, unsigned int * ids, int nrOfIds) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;
	
		bool success = true;
	
		for ( int i = 0; i < nrOfIds; i++ ) {
			unsigned int id = ids[i];
			
			wcout << "Current mailslot id = " << id;
			
			if ( id == 0 ) {
				wcout << " = 0 so don't try to open it..." << "\n";
			}
			else {
				wcout << " != 0 so try to open the mailslot ";
			
				success = success && CreateWriterMailslot(pSharedDataPartner, id);				
			}		
		}
		
		return success;
	}


	BOOL DestroyReaderMailslot(HANDLE SharedDataPartnerHandle) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;

		CloseHandle( pSharedDataPartner->hReaderMailslot );
		
		return TRUE;
	}


	/// Creates a mailslot handle so we can listen for messages
	BOOL CreateReaderMailslot(SharedDataPartnerDescriptor * pSharedDataPartner) {
		wstring name = CreateMailslotName(pSharedDataPartner->partnerIdentifier);
		
		pSharedDataPartner->hReaderMailslot = CreateMailslot( 	name.c_str(), 
													        0,                             // no maximum message size 
													        5000, //MAILSLOT_WAIT_FOREVER,         // no time-out for operations 
													        (LPSECURITY_ATTRIBUTES) NULL); // default security
		
		if ( pSharedDataPartner->hReaderMailslot == INVALID_HANDLE_VALUE ) { 
			wcout << "CreateMailslot (" << name << ") failed with " << GetLastError() << "\n";
			return FALSE; 
		} 
	    else {
	    	wcout << "Mailslot named " << name << " created successfully.\n";
	    }
	    return TRUE; 
	}


	BOOL WriteToAllMailslots(HANDLE SharedDataPartnerHandle, BYTE * data, int len) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;
		
		BOOL success = true;

		if ( pSharedDataPartner->writerMailslotHandlesMap->size() > 0 ) {
			map<unsigned int, HANDLE>::iterator itr;
			
		    wcout << "Found " << pSharedDataPartner->writerMailslotHandlesMap->size() << " partners, so get an iterator and try to send the message to all of them." << "\n";
			
			for ( itr = pSharedDataPartner->writerMailslotHandlesMap->begin(); itr != pSharedDataPartner->writerMailslotHandlesMap->end(); ++itr ) {
				HANDLE hFile = itr->second;
				BOOL fResult;
				DWORD cbWritten; 
	
		    	//wcout << "Try to write to mailsot " << hFile << " aka " << itr->second << "\n";
				
				fResult = WriteFile(hFile, data, len, &cbWritten, (LPOVERLAPPED) NULL);
	
				if (!fResult) {
					success = false;
					printf("WriteFile failed with %d.\n", GetLastError()); 
				}
				else {
					//printf("Slot written to successfully.\n"); 
				}
			}
		}
		else {
		    wcout << "No partner mailslots found, so do nothing..." << "\n";			
		}
		
		return success;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId ; size_t appnamelength ; wchar_t * partnerName
	int CreateHelloMessage(HANDLE SharedDataPartnerHandle, BYTE ** data ) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;

		int dataSize = 1 + sizeof(pSharedDataPartner->partnerIdentifier) + sizeof(size_t) + pSharedDataPartner->partnerName->length() * sizeof(wchar_t);
		(*data) = new BYTE[dataSize];
		(*data)[0] = SHARED_DATA_HELLO;

		BYTE * addr = (*data) + 1;
		int len = sizeof(pSharedDataPartner->partnerIdentifier);
		CopyMemory( addr, (VOID *) &pSharedDataPartner->partnerIdentifier, len );
		addr += len;
		len = sizeof(size_t);
		size_t strlen = pSharedDataPartner->partnerName->length();
		CopyMemory( addr, (VOID *) &strlen, len );
		addr += len;
		len = pSharedDataPartner->partnerName->length() * sizeof(wchar_t);
		CopyMemory( addr, (VOID *) pSharedDataPartner->partnerName->c_str(), len );
		addr += len;
		
		return dataSize;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId
	int CreateGoodbyeMessage(HANDLE SharedDataPartnerHandle, BYTE ** data ) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;

		int dataSize = 1 + sizeof(pSharedDataPartner->partnerIdentifier);
		(*data) = new BYTE[dataSize];
		(*data)[0] = SHARED_DATA_GOODBYE;

		BYTE * addr = (*data) + 1;
		int len = sizeof(pSharedDataPartner->partnerIdentifier);
		CopyMemory( addr, (VOID *) &pSharedDataPartner->partnerIdentifier, len );
		addr += len;
		
		return dataSize;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId ; SharedObjectInfo
	int CreateShareObjectMessage(SharedDataPartnerDescriptor * pSharedDataPartner, SharedObjectInfo * pObject, BYTE ** data ) {
		int dataSize = 1 + sizeof(pSharedDataPartner->partnerIdentifier) + + sizeof(SharedObjectInfo) + pObject->dataSize;
		
		(*data) = new BYTE[dataSize];
		(*data)[0] = SHARED_DATA_SHARE;
		
		BYTE * addr = (*data) + 1;
		int len = sizeof(pSharedDataPartner->partnerIdentifier);
		CopyMemory( addr, (VOID *) &pSharedDataPartner->partnerIdentifier, len );
		
		addr += len;
		len = sizeof(SharedObjectInfo);
		CopyMemory( addr, (VOID *) pObject, len );

		addr += len;
		len = pObject->dataSize;
		CopyMemory( addr, pObject->pData, len );
		
		addr += len;
		
		//wcout << "CreateShareObjectMessage " << pObject->hSharedTexture << " " << pObject->width << "x" << pObject->height << " named '" << pObject->description << "' created a message of size " << dataSize << "=" << (addr - *data) << "\n";
		
		return dataSize;
	}


	/// data should be deleted afterwards by the caller !!!
	int CreateUnshareObjectMessage(SharedDataPartnerDescriptor * pSharedDataPartner, HANDLE sharedTextureHandle, BYTE ** data) {
		int dataSize = 1 + sizeof(pSharedDataPartner->partnerIdentifier) + sizeof(sharedTextureHandle);
		(*data) = new BYTE[dataSize];
		(*data)[0] = SHARED_DATA_UNSHARE;

		BYTE * addr = (*data) + 1;
		int len = sizeof(pSharedDataPartner->partnerIdentifier);
		CopyMemory( addr, (VOID *) &pSharedDataPartner->partnerIdentifier, len );
		
		addr += len;
		len = sizeof(sharedTextureHandle);
		CopyMemory( addr, (VOID *) &sharedTextureHandle, len );
		
		return dataSize;
	}


	/// Add this partner to the list of known partners
	/// Then send him a hello message (so he knows our name) and send him all of the stuff we are sharing
	bool ProcessReceivedHelloMessage( SharedDataPartnerDescriptor * pSharedDataPartner, BYTE * data, int length ) {
		bool success = true;

		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		unsigned int partnerId = *((unsigned int *)pData);
		
		pData += sizeof(partnerId);
		size_t name_length = *((size_t *)pData);
		
		pData += sizeof(size_t);
		WCHAR * name = (WCHAR*)pData;

		//wcout << "The new message partnerId = " << partnerId << " and name has length " << name_length << "\n";

		wstring wstrName;
		wstrName.append( name, name_length );
		
		//wcout << " and name = " << wstrName << "\n";

		// Add to the list of known partners: update the writerMailslotsMap
		///////////////////////////////////////////////////////////////////

		//find and open the mailslot and store its handle
		success = success && CreateWriterMailslot( pSharedDataPartner, partnerId );
		
		map<unsigned int, wstring> * namesMap = pSharedDataPartner->writerMailslotNamesMap;

		//(*handlesMap)[partnerId]
		((*namesMap)[partnerId]).clear(); //clear the current name for the mailslot string
		(*namesMap)[partnerId].append( name, name_length );
		
		//wcout << "The new message partnerId = " << partnerId << " and name " << (*namesMap)[partnerId] <<  " (length=" << name_length << ")" << "\n";


		// Send back a 'welcome' message, so the new partner knows our name
		///////////////////////////////////////////////////////////////////
		
		// (a welcome message is just a hello message with a different msgType)
		map<unsigned int, HANDLE> * handlesMap = pSharedDataPartner->writerMailslotHandlesMap;
		HANDLE hPartnerMailslot = (*handlesMap)[partnerId];

		BYTE * welcomeMsgData;
		int welcomeMsgDataSize = CreateHelloMessage(pSharedDataPartner, &welcomeMsgData);
		welcomeMsgData[0] = SHARED_DATA_WELCOME;
		

		DWORD cbWritten;
		BOOL fResult = WriteFile(hPartnerMailslot, welcomeMsgData, welcomeMsgDataSize, &cbWritten, (LPOVERLAPPED) NULL);

		if (!fResult) {
			success = false;
			printf("WriteFile failed with %d.\n", GetLastError()); 
		}
		else {
			//printf("Slot written to successfully.\n"); 
		}

		
		//this is a new partner, so send him all of my shared stuff
		//map<HANDLE, SharedObjectInfo* > * sharedByUsObjectsMap;
		
		map<HANDLE, SharedObjectInfo* >::iterator itr;
			
		for ( itr = pSharedDataPartner->sharedByUsObjectsMap->begin(); itr != pSharedDataPartner->sharedByUsObjectsMap->end(); ++itr ) {
			BYTE * data;
			int dataSize = CreateShareObjectMessage(pSharedDataPartner, itr->second, &data);

			cbWritten;
			fResult = WriteFile(hPartnerMailslot, data, dataSize, &cbWritten, (LPOVERLAPPED) NULL);
	
			if (!fResult) {
				success = false;
				printf("WriteFile failed with %d.\n", GetLastError()); 
			}
			else {
				//printf("Slot written to successfully.\n"); 
			}
		}
		
		/*
		HANDLE hFile = itr->first;
		BOOL fResult;
		DWORD cbWritten; 

		fResult = WriteFile(hFile, data, len, &cbWritten, (LPOVERLAPPED) NULL);

		if (!fResult) {
			succes = false;
			printf("WriteFile failed with %d.\n", GetLastError()); 
		}
		else {
			printf("Slot written to successfully.\n"); 
		}
		*/
		return success;
	}

	/// Add this partner to the list of known partners
	bool ProcessReceivedWelcomeMessage( SharedDataPartnerDescriptor * pSharedDataPartner, BYTE * data, int length ) {
		bool success = true;

		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		unsigned int partnerId = *((unsigned int *)pData);
		
		pData += sizeof(partnerId);
		size_t name_length = *((size_t *)pData);
		
		pData += sizeof(size_t);
		WCHAR * name = (WCHAR*)pData;

		//wcout << "The new message partnerId = " << partnerId << " and name has length " << name_length << "\n";

		wstring wstrName;
		wstrName.append( name, name_length );
		
		//wcout << " and name = " << wstrName << "\n";

		// Add to the list of known partners: update the writerMailslotsMap
		///////////////////////////////////////////////////////////////////
		map<unsigned int, wstring> * namesMap = pSharedDataPartner->writerMailslotNamesMap;

		//find and open the mailslot and store its handle
		success = success && CreateWriterMailslot( pSharedDataPartner, partnerId );
		
		((*namesMap)[partnerId]).clear(); //clear the current name for the mailslot string
		(*namesMap)[partnerId].append( name, name_length );
		
		wcout << "The new message partnerId = " << partnerId << " and name " << (*namesMap)[partnerId] <<  " (length=" << name_length << ")" << "\n";


		return success;
	}


	/// Remove this partner from the list of known partners and stop listening, also remove all of his shared resources
	bool ProcessReceivedGoodbyeMessage( SharedDataPartnerDescriptor * pSharedDataPartner, BYTE * data, int length ) {
		bool success = true;

		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		unsigned int partnerId = *((unsigned int *)pData);

		// Remove all resources shared by this partner
		RemoveAllObjectsSharedByPartner(pSharedDataPartner, partnerId);
		
		// Remove from the list of known partners
		/////////////////////////////////////////

		DestroyWriterMailslot( pSharedDataPartner, partnerId );

		return success;
	}


	/// Add this object to the list of 'shared-by-partners' objects
	bool ProcessReceivedShareObjectMessage(SharedDataPartnerDescriptor * pSharedDataPartner, BYTE * data, int length ) {

		//wcout << "ProcessReceivedShareObjectMessage received a message of size " << length << "\n";

		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		int len = sizeof(unsigned int);
		unsigned int partnerId = *((unsigned int *)pData);
		
		pData += len;
		len = sizeof(SharedObjectInfo);
		SharedObjectInfo * pSharedObjectInfo = new SharedObjectInfo(); //(VOID *)pData;
		CopyMemory(pSharedObjectInfo, (VOID *) pData, len );

		pData += len;
		len = pSharedObjectInfo->dataSize;
		pSharedObjectInfo->pData = (VOID*) new BYTE[len];
		CopyMemory(pSharedObjectInfo->pData, (VOID *) pData, len );

		wcout << "Data shared by "<< partnerId << " handle=" << pSharedObjectInfo->hObject << " " << pSharedObjectInfo->dataSize << " '" << (LPTSTR) pSharedObjectInfo->pData << "'" << "\n";
		
		return AddObjectSharedByPartner(pSharedDataPartner, partnerId, pSharedObjectInfo, len);
	}

	/// Remove this object from the list of 'shared-by-partners' objects
	bool ProcessReceivedUnshareObjectMessage(SharedDataPartnerDescriptor * pSharedDataPartner, BYTE * data, int length ) {
	
		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		int len = sizeof(unsigned int);
		unsigned int partnerId = *((unsigned int *)pData);
		
		pData += len;
		len = sizeof(HANDLE);
		HANDLE hSharedObjectInfo = *((HANDLE *)pData);

		wcout << "Object NOT shared ANYMORE  by "<< partnerId << " handle=" << hSharedObjectInfo << "\n";

		return RemoveObjectSharedByPartner(pSharedDataPartner, partnerId, hSharedObjectInfo);
	}






	//THIS THREAD WILL BE READING THE MAILSLOT, and calling callback functions when needed
	void WyphonThread( void* pSharedDataPartnerHandle ) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) ( * (HANDLE*) pSharedDataPartnerHandle );
		
		wcout << "WyphonThread function: " << *(pSharedDataPartner->partnerName) << "\n";

		DWORD readTimeout = 5000;
		//SetMailslotInfo( pSharedDataPartner->hReaderMailslot, readTimeout );			


		while ( pSharedDataPartner->listenerThreadRunning ) {
		
			DWORD nextSize;
			DWORD messageCount;

			//wcout << "WyphonThread checking for new messages..." << "\n";
			wcout << " .";
			wcout.flush();

			//////////////////////////////////////////////////////////////////////////////////////////////////
			// Some explanation: I thought GetmailslotInfo would block but actually ReadFile blocks
			// It blocks for a maximum time as set when creating the Mailslot (could be MAILSLOT_WAIT_FOREVER)
			// So we will try to read 0 bytes, and when that returns, we can call GetMailslotInfo to retrieve 
			// the number and size of the new message(s). If we use Readfile then, we know it won't block.
			//////////////////////////////////////////////////////////////////////////////////////////////////
			
			//Block until new data arrives (or until readTimeout set when creating the mailslot is over) !!!!
			char dummy;
			DWORD dummyNrOfBytesRead;
			ReadFile( pSharedDataPartner->hReaderMailslot, &dummy, 0, &dummyNrOfBytesRead, NULL);


			GetMailslotInfo( pSharedDataPartner->hReaderMailslot, NULL, &nextSize, &messageCount, &readTimeout);

			//wcout << "readTimeout = " << readTimeout << "\n";

			if ( nextSize != MAILSLOT_NO_MESSAGE ) {
				//read the message, and call the callback function
				
				DWORD nrOfBytesRead;
				BYTE buffer[1024];
				//WE NEED TO KNOW THE CORRECT MESSAGE SIZE = nextSize for each message
				//for ( int i = 0; i < messageCount; i++ ) {
				do {
					if ( ReadFile( pSharedDataPartner->hReaderMailslot, &buffer, nextSize, &nrOfBytesRead, NULL) ) {
						//parse message and do the appropriate callback
						if ( buffer[0] == SHARED_DATA_HELLO ) {
							wcout << "WyphonThread received a [SHARED_DATA_HELLO] message." << "\n";
							
							ProcessReceivedHelloMessage( pSharedDataPartner, buffer, nrOfBytesRead);
						}
						else if ( buffer[0] == SHARED_DATA_WELCOME ) {
							wcout << "WyphonThread received a [SHARED_DATA_WELCOME] message." << "\n";
							ProcessReceivedWelcomeMessage( pSharedDataPartner, buffer, nrOfBytesRead);
						}
						else if ( buffer[0] == SHARED_DATA_GOODBYE ) {
							wcout << "WyphonThread received a [SHARED_DATA_GOODBYE] message." << "\n";
							ProcessReceivedGoodbyeMessage( pSharedDataPartner, buffer, nrOfBytesRead);
						}
						else if ( buffer[0] == SHARED_DATA_SHARE ) {
							wcout << "WyphonThread received a [SHARED_DATA_SHARE] message." << "\n";
							ProcessReceivedShareObjectMessage( pSharedDataPartner, buffer, nrOfBytesRead);
						}
						else if ( buffer[0] == SHARED_DATA_UNSHARE ) {
							wcout << "WyphonThread received a [SHARED_DATA_UNSHARE] message." << "\n";
							ProcessReceivedUnshareObjectMessage( pSharedDataPartner, buffer, nrOfBytesRead);
						}
					}
					
					GetMailslotInfo( pSharedDataPartner->hReaderMailslot, NULL, &nextSize, &messageCount, &readTimeout);
				} while ( nextSize != MAILSLOT_NO_MESSAGE );
			}
			else {
				//wcout << "WyphonThread received NO new messages... Let's listen again." << "\n";
				//wcout << " .";
				//wcout.flush();

				//DEBUG
				//Sleep(1000L);
			}
			//DEBUG
			//Sleep(1000L);
		}
	}


	
	
	extern "C" _declspec(dllexport)
	bool DestroySharedDataPartner(HANDLE SharedDataPartnerHandle) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;

		//Stop listening for new messages
		pSharedDataPartner->listenerThreadRunning = false;


		//Remove yourself from sharedMemory
		bool success = false;

		if ( LockSharedMemory( pSharedDataPartner->hSharedMemory, 2000 ) ) {

			//close readermailslot: stop listening to new messages
			DestroyReaderMailslot(pSharedDataPartner);


			//wcout << "Trying to read shared data" << "\n";
			
			BYTE * idBytes = NULL;
			int nrOfIds = ReadSharedMemory( pSharedDataPartner->hSharedMemory, idBytes ) / sizeof(unsigned int);
			unsigned int * ids = (unsigned int *)idBytes;

			//find ourself in shareddata
			int ourPosition = -1;

			for ( int i = 0; i < nrOfIds; i++ ) {
				if ( ids[i] == pSharedDataPartner->partnerIdentifier ) {
					ourPosition = i;
				}
			}

			//wcout << "delete read bytes" << "\n";
			delete idBytes;

			//wcout << "Trying to write to shared data at position " << ourPosition << "\n";

			unsigned int zero = 0;
			//Remove yourself from the list
			WriteSharedMemory( 	pSharedDataPartner->hSharedMemory, 
								(BYTE*) &zero,						//data
								sizeof(unsigned int),				//length
								ourPosition * sizeof(unsigned int)	//offset								
							);
			

			success = UnlockSharedMemory(pSharedDataPartner->hSharedMemory);
		}

		//wcout << "Trying to write goodbye message to all parter mailslots" << "\n";
		
		//Write a 'goodbye' message to all writermailslots
		BYTE * data;
		int dataSize = CreateGoodbyeMessage(pSharedDataPartner, &data);
		
		WriteToAllMailslots( pSharedDataPartner, (BYTE *) data, dataSize );
		
		delete data;

		//wcout << "Trying to destroy writer mailslots" << "\n";

		//close all writermailslots
		DestroyWriterMailslots(pSharedDataPartner);
		
		pSharedDataPartner->hSharedMemory = NULL;
		

		//HANDLE hSharedMemory;
		//wstring * partnerName;
		//unsigned int partnerIdentifier;
		//HANDLE hReaderMailslot;
		//map<HANDLE, wstring> writerMailslotsMap;

		//wcout << "Trying to delete all memory that has been allocated..." << "\n";

		delete pSharedDataPartner->writerMailslotNamesMap;
		delete pSharedDataPartner->writerMailslotHandlesMap;
		delete pSharedDataPartner->partnerName;

		map<unsigned int, map<HANDLE, SharedObjectInfo*>>::iterator itr;
		for ( itr = pSharedDataPartner->sharedByPartnersObjectsMap->begin(); itr != pSharedDataPartner->sharedByPartnersObjectsMap->end(); itr = pSharedDataPartner->sharedByPartnersObjectsMap->begin() /*because current gets deleted !!!*/ ) {
			RemoveAllObjectsSharedByPartner(pSharedDataPartner, itr->first );
		}
		delete pSharedDataPartner->sharedByPartnersObjectsMap;

		RemoveAllObjectsSharedByUs(pSharedDataPartner);
		delete pSharedDataPartner->sharedByUsObjectsMap;
		
		delete pSharedDataPartner;
		
		//wcout << "Done..." << "\n";
		
		return success;
	}
//
//
//
//	#define BUF_SIZE 255
//
//
//
	extern "C" _declspec(dllexport)
	HANDLE CreateSharedDataPartner( LPTSTR lpSharedMemoryName, LPTSTR partnerName ) {
		SharedDataPartnerDescriptor * pSharedDataPartner = new SharedDataPartnerDescriptor();
 
 		//LPTSTR lpSharedMemoryName = TEXT(SHARED_DATA_SHAREDDATA_NAME);

		pSharedDataPartner->sharedMemoryName = new wstring( lpSharedMemoryName );

		wcout << "Try to CreateSharedData with " << pSharedDataPartner->sharedMemoryName << "\n";

		pSharedDataPartner->hSharedMemory = CreateSharedMemory( lpSharedMemoryName, SHARED_DATA_INITIAL_NUMBER_OF_PARTNERS * sizeof(unsigned int), SHARED_DATA_MAX_NUMBER_OF_PARTNERS * sizeof(unsigned int) );

		wcout << "CreateSharedData returned " << pSharedDataPartner->hSharedMemory << "\n";

		pSharedDataPartner->partnerName = new wstring( partnerName );

		wcout << "Application Name = " << partnerName << " or in *(pSharedDataPartner->partnerName) = " << *(pSharedDataPartner->partnerName) << "\n";
		
		pSharedDataPartner->writerMailslotHandlesMap = new map<unsigned int, HANDLE>();
		pSharedDataPartner->writerMailslotNamesMap = new map<unsigned int, wstring>();

		pSharedDataPartner->sharedByUsObjectsMap = new map<HANDLE, SharedObjectInfo* >();
		pSharedDataPartner->sharedByPartnersObjectsMap = new map<unsigned int, map<HANDLE, SharedObjectInfo* >>();
		

		//Open mailsot handles for every-one that's already in shared data
		//and create one of your own and share that in shared data
		
		wcout << "Try to open mailslots for every handle that is already in shared data..." << "\n";
		wcout << "LockSharedMemory" << "\n";
		if ( LockSharedMemory( pSharedDataPartner->hSharedMemory, 1000 ) ) {
			
			BYTE * idBytes = NULL;
			int nrOfIds = ReadSharedMemory( pSharedDataPartner->hSharedMemory, idBytes ) / sizeof(unsigned int);
			unsigned int * ids = (unsigned int *)idBytes;			

			wcout << "I found " << nrOfIds << " ids in shared data..." << "\n";

			if ( nrOfIds > 0 && CreateWriterMailslots(pSharedDataPartner, ids, nrOfIds) ) {
				//create your own mailslot and
				//advertise your existence in the shared memory, and to all of the partner mailslots

				wcout << "Now try to create your own mailslot to listen to." << "\n";

				//FOR A REASON I DON'T UNDERSTAND, looping twice over the ids crashes later on...
				// So read the shared memory again...
//				delete idBytes;
//				nrOfIds = ReadSharedData( pSharedDataPartner->hSharedMemory, idBytes ) / sizeof(unsigned int);
//				ids = (unsigned int *)idBytes;

				
				int emptySpotPosition = -1;
				unsigned int newReaderMailslotIdentifier = 0;
				if ( FindMailslotIdAndEmptySharedMemorySpot(ids, nrOfIds, emptySpotPosition, newReaderMailslotIdentifier) ) {
					wcout << "Found an empty spot in shared memory at position " << emptySpotPosition << " and the first unused ID = " << newReaderMailslotIdentifier << "\n";
					
					pSharedDataPartner->partnerIdentifier = newReaderMailslotIdentifier;

					//Now advertise yourself everywhere
					if ( CreateReaderMailslot(pSharedDataPartner) ) {
						wcout << "Now try to write your own ID to shared data " << pSharedDataPartner->partnerIdentifier << "\n";
						
						//Add yourself to the list (at the first empty spot) !!!
						WriteSharedMemory( 	pSharedDataPartner->hSharedMemory, 
											(BYTE*) &newReaderMailslotIdentifier,		//data
											sizeof(unsigned int),						//length
											emptySpotPosition * sizeof(unsigned int)	//offset
										);

						//DEBUG
/*						delete idBytes;
						nrOfIds = ReadSharedData( pSharedDataPartner->hSharedMemory, idBytes ) / sizeof(unsigned int);
						ids = (unsigned int *)idBytes;
						emptySpotPosition = -1;
						newReaderMailslotIdentifier = 0;
						if ( FindMailslotIdAndEmptySharedMemorySpot(ids, nrOfIds, emptySpotPosition, newReaderMailslotIdentifier) ) {
							wcout << "TEST TEST TEST: Found an empty spot in shared memory at position " << emptySpotPosition << " and the first unused ID = " << newReaderMailslotIdentifier << "\n";
						}
*/

						wcout << "Now try to create a 'hello' message" << "\n";

						//Write a 'hello' message to all writermailslots
						BYTE * data;
						int dataSize = CreateHelloMessage(pSharedDataPartner, &data);
	
						wcout << "'hello' message created, try to send it to all partners by writing it to their mailslots..." << "\n";
	
						WriteToAllMailslots( pSharedDataPartner, (BYTE *) data, dataSize );
						
						delete data;
					}
				}
				else {
					//TODO Destroy everything that's been created so far and return NULL
				}
			}
			delete idBytes;
			
			wcout << "UnlockSharedMemory" << "\n";
			UnlockSharedMemory( pSharedDataPartner->hSharedMemory );
		}
		
		
		wcout << "Try to start the listener thread !!!" << "\n";
		
		//START the listening thread
		pSharedDataPartner->listenerThreadRunning = true;
		_beginthread( WyphonThread, 0, &pSharedDataPartner );
		
		Sleep(1000L);
		
		wcout << "Finally return the handle " << pSharedDataPartner << "\n";
		
		//Sleep(20000L);
		
		return pSharedDataPartner;
	}


	extern "C" _declspec(dllexport)
	bool ShareData(HANDLE SharedDataPartnerHandle, HANDLE sharedDataHandle, void * data, unsigned int length) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;


		SharedObjectInfo * pSharedObjectInfo = new SharedObjectInfo();
		pSharedObjectInfo->hObject = sharedDataHandle;
		pSharedObjectInfo->dataSize = length;
		pSharedObjectInfo->pData = new BYTE[length];
		CopyMemory( pSharedObjectInfo->pData, data, length);
		
		BYTE * msgData;
		int msgDataSize = CreateShareObjectMessage(pSharedDataPartner, pSharedObjectInfo, &msgData);
		
		bool success = msgDataSize > 0;

		if ( success ) {
			success = WriteToAllMailslots( pSharedDataPartner, msgData, msgDataSize );
			(*(pSharedDataPartner->sharedByUsObjectsMap))[sharedDataHandle] = pSharedObjectInfo;
			delete msgData;
		}

		//wcout << "Shared object with handle=" << pObject->hSharedTexture << " " << pObject->width << "x" << pObject->height << "\n";
		
		return success;
	}

	extern "C" _declspec(dllexport)
	bool UnshareData(HANDLE SharedDataPartnerHandle, HANDLE sharedDataHandle) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;

		BYTE * msgData;
		int msgDataSize = CreateUnshareObjectMessage(pSharedDataPartner, sharedDataHandle, &msgData);
		
		bool success = msgDataSize > 0;
		if ( success ) {
			success = WriteToAllMailslots( pSharedDataPartner, msgData, msgDataSize );
			delete msgData;
			
			RemoveObjectSharedByUs(pSharedDataPartner, sharedDataHandle);
//			delete (*(pSharedDataPartner->sharedByUsObjectsMap))[sharedDataHandle];
//			pSharedDataPartner->sharedByUsObjectsMap->erase(sharedDataHandle);
		}
		
		return success;	
	}



/*
	extern "C" _declspec(dllexport)
	bool ShareTexture(HANDLE SharedDataPartnerHandle, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD usage, LPTSTR description) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;


		VOID * pObject = CreateWyphonTextureInfo( sharedTextureHandle, width, height, usage, _tcslen(description), description );
		
		BYTE * data;
		int dataSize = CreateShareObjectMessage(pSharedDataPartner, pObject, &data);
		
		bool success = dataSize > 0;

		if ( success ) {
			success = WriteToAllMailslots( pSharedDataPartner, data, dataSize );
			(*(pSharedDataPartner->sharedByUsObjectsMap))[sharedTextureHandle] = pObject;
			delete data; 
		}

		//wcout << "Shared object with handle=" << pObject->hSharedTexture << " " << pObject->width << "x" << pObject->height << "\n";
		
		return success;
	}

	extern "C" _declspec(dllexport)
	bool UnshareTexture(HANDLE SharedDataPartnerHandle, HANDLE sharedTextureHandle) {
		SharedDataPartnerDescriptor * pSharedDataPartner = (SharedDataPartnerDescriptor *) SharedDataPartnerHandle;

		BYTE * data;
		int dataSize = CreateUnshareObjectMessage(pSharedDataPartner, sharedTextureHandle, &data);
		
		bool success = dataSize > 0;
		if ( success ) {
			success = WriteToAllMailslots( pSharedDataPartner, data, dataSize );
			delete data;
			
			delete (*(pSharedDataPartner->sharedByUsObjectsMap))[sharedTextureHandle];
			pSharedDataPartner->sharedByUsObjectsMap->erase(sharedTextureHandle);
		}
		
		return success;	
	}
*/


//
//		DWORD err;
//	HINSTANCE hDLL = LoadLibrary("DllProctTest.dll");               // Handle to DLL
//	
//	if(hDLL != NULL)
//	{
//		printf("Library has been loaded\n");
//        }
//	else
//	{
//                err = GetLastError();
//		printf("Couldn't load dll\n");
//	}
//	DisplayHelloFromDLL();
//	return 0;







//	public class SharedDataPartner
//	{
//		
//		
//		/// <summary>
//		/// This function will set up everything to start communicating with the other Wyphon partners
//		/// </summary>
//		public void Start() {
//			
//		}
//		
//		/// <summary>
//		/// Unregisters all shared resources and stops sending "I'm alive" updates
//		/// </summary>
//		public void Stop() {
//			
//		}
//		
//		/// <summary>
//		/// Returns a list of currently shared resources
//		/// </summary>
//		/// <returns></returns>
//		public List<WyphonInfo> GetCurrentlySharedResources() {
//			return null;
//		}
//		
//		/// <summary>
//		/// Broadcast a new shared resource
//		/// </summary>
//		/// <param name="w"></param>
//		public void RegisterWyphonInfo(WyphonInfo w) {
//			
//		}
//		
//		/// <summary>
//		/// Stop sharing a shared resource. Tell everyone else it has become invalid
//		/// </summary>
//		/// <param name="w"></param>
//		public void UnregisterWyphonInfo(WyphonInfo w) {
//			
//		}
//	}

}