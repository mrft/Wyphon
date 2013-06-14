/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 14/11/2012
 * Time: 16:30
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
// LocalMessageBroadcast.cpp
#include "LocalMessageBroadcast.h"
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "SharedMemory.h"


// Due to problems using SharedMemory from another dll,
// I included that code in the same project 
// (see #include "SharedMemory.h")
///////////////////////////////////////////////////////

////	Define the Functions in the DLL for reuse. This is just prototyping the dll's function. 
////	A mock of it. Use "stdcall" for maximum compatibility.
//typedef bool (__stdcall * pLockSharedMemoryFUNC)(HANDLE sharedDataHandle, unsigned __int32 timeoutInMilliseconds);
//typedef bool (__stdcall * pUnlockSharedMemoryFUNC)(HANDLE sharedDataHandle);
//typedef __int32 (__stdcall * pWriteSharedMemoryFUNC)( HANDLE sharedDataHandle, BYTE* data, unsigned __int32 length, unsigned __int32 offset );
//typedef __int32 (__stdcall * pReadSharedMemoryFUNC)( HANDLE sharedDataHandle, BYTE * &pData );
//typedef __int32 (__stdcall * pWriteStringToSharedMemoryFUNC)( HANDLE sharedDataHandle, LPTSTR data );
//typedef LPTSTR (__stdcall * pReadStringFromSharedMemoryFUNC)( HANDLE sharedDataHandle );
//typedef bool (__stdcall * pDestroySharedMemoryFUNC)(HANDLE sharedDataHandle);
//typedef HANDLE (__stdcall * pCreateSharedMemoryFUNC)(LPTSTR lpName, unsigned __int32 startSize, unsigned __int32 maxSize);
//
//
//HINSTANCE hSharedMemoryDll = LoadLibrary(TEXT("SharedMemory.dll"));
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// get pointers to these functions in the dll
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	bool LockSharedMemory(HANDLE sharedDataHandle, unsigned __int32 timeoutInMilliseconds)
////	bool UnlockSharedMemory(HANDLE sharedDataHandle)
////	__int32 WriteSharedMemory( HANDLE sharedDataHandle, BYTE* data, unsigned __int32 length, unsigned __int32 offset )
////	__int32 ReadSharedMemory( HANDLE sharedDataHandle, BYTE * &pData )
////	__int32 WriteStringToSharedMemory( HANDLE sharedDataHandle, LPTSTR data )
////	LPTSTR ReadStringFromSharedMemory( HANDLE sharedDataHandle )
////	bool DestroySharedMemory(HANDLE sharedDataHandle)
////	HANDLE CreateSharedMemory(LPTSTR lpName, unsigned __int32 startSize, unsigned __int32 maxSize)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
////if ( hSharedMemoryDll != NULL ) {
//	FARPROC lpfnLockSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"LockSharedMemory");
//	FARPROC lpfnUnlockSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"UnlockSharedMemory");
//	FARPROC lpfnWriteSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"WriteSharedMemory");
//	FARPROC lpfnReadSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"ReadSharedMemory");
//	FARPROC lpfnDestroySharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"DestroySharedMemory");
//	FARPROC lpfnCreateSharedMemory = GetProcAddress(HMODULE (hSharedMemoryDll),"CreateSharedMemory");
//
//
//	pLockSharedMemoryFUNC LockSharedMemory = (pLockSharedMemoryFUNC)lpfnLockSharedMemory;
//	pUnlockSharedMemoryFUNC UnlockSharedMemory = (pUnlockSharedMemoryFUNC)lpfnUnlockSharedMemory;
//	pWriteSharedMemoryFUNC WriteSharedMemory = (pWriteSharedMemoryFUNC)lpfnWriteSharedMemory;
//	pReadSharedMemoryFUNC ReadSharedMemory = (pReadSharedMemoryFUNC)lpfnReadSharedMemory;
//	pDestroySharedMemoryFUNC DestroySharedMemory = (pDestroySharedMemoryFUNC)lpfnDestroySharedMemory;
//	pCreateSharedMemoryFUNC CreateSharedMemory = (pCreateSharedMemoryFUNC)lpfnCreateSharedMemory;
////}


/*
	The LocalMessageBroadcastPartner descriptor should hold all of the necessary information 
	to be able to send messages to all of the other LocalMessageBroadcastPartners.
	- 	The first will be the hSharedMemory, which will hold the SharedMemory handle. 
		In this sharedData, we will store the HANDLE of the MAILSLOT that should be written 
		to for each Partner.
		If a Partner disappears, he should ideally clear this field (set to 0), but if 
		he forgets it, it would be cool if another one that detects a MAILSLOT going down would fix this.
		This is only used for 'newcomers', who get a list of MAILSLOTS they should set up for writing their 
		messages too.
		Through all of these they should let everybody know that they are here, and that they 
		should be contacted through the mailslot with NAME \\mailslot\.\<SharedByAllMailslots>\xxxx 
		(xxxx being the decimal representation of the unigned integer in the shared memory).
		These mailslots are also needed to let everybody else know if we send a message, or when we are quitting.
	- 	As a consequence, the second will be a list of mailslots that we should communicate over.

	So, we are setting up mailslot communication with everyone else who is participating. Every message will 
	be sent to every partner.
	There will be 5 types of messages:
	- Hello, I am new. My name isxxx, please talk to me on this channel (mailslot handle?).
	- Welcome, new partner! My name is xxx.
	- Hey, I have a message for all of you. Here it is: [bytes]
	- Goodbye, please stop talking to me (and forget about me).
	  Whenever an application sends out this goodbye message, it should have cleaned up the shared memory first.
	
	A message will have 1 byte telling us the message_type
	and some other bytes depending on that type, with needed info:
	- HELLO (1)		|	myId (sizeof unsigned __int32)	|	name_length (unsigned __int32) 	|	name
	- WELCOME (1)	|	myId (sizeof unsigned __int32)	|	name_length (unsigned __int32) 	|	name
	- MESSAGE (1) 	|	myId		|	msg_length (sizeof unsigned __int32)		|	msg bytes
	- GOODBYE		|	myId
	
)
*/

using namespace std;
using namespace SharedMemory;

namespace LocalMessageBroadcast {



	// IF WE ASSUME that more types can be shared in the future
	// we will add message_types and structs to describe these resources
	// We'll start with textures only at this time

	#define LOCAL_MSG_BROADCAST_STOPLISTENING				0
	#define LOCAL_MSG_BROADCAST_HELLO 						1
	#define LOCAL_MSG_BROADCAST_WELCOME						2
	#define LOCAL_MSG_BROADCAST_MESSAGE		 				10
	#define LOCAL_MSG_BROADCAST_GOODBYE 					99

	#define LOCAL_MSG_BROADCAST_INITIAL_NUMBER_OF_PARTNERS 	2
	#define LOCAL_MSG_BROADCAST_MAX_NUMBER_OF_PARTNERS 		128

	#define READERMAILSLOT_TIMEOUT							1000



	//FOR INTERNAL USE ONLY
	struct LocalMessageBroadcastPartnerDescriptor {
		HANDLE hSharedMemory;

		wstring * sharedMemoryName;
		
		wstring * myName;

		unsigned __int32 myId; //used to generate out mailsot name, and shared with the other partners
		
		HANDLE hReaderMailslot; //handle to the mailslot we are listening on

		bool listenerThreadRunning;

		map<unsigned __int32, HANDLE> * writerMailslotHandlesMap;	//unsigned __int32 = the partner's myId
																//HANDLE = the handles to the mailslots we should write to

		map<unsigned __int32, wstring> * partnerNamesMap;	//unsigned __int32 = the partner's myId
														//wstring = name of the partner


		LPLocalMessageBroadcastPartnerJoinedCALLBACK partnerJoinedCallbackFunc;
		LPLocalMessageBroadcastPartnerLeftCALLBACK partnerLeftCallbackFunc;
		LPMessageReceivedCALLBACK msgReceivedCallbackFunc;

		void * callbackFuncCustomData;	//will be sent with Callback functions, and can contain data specific to the user

		//for receiving messages
		__int32 currentReadBufferSize;
		BYTE * readBuffer;

		//for locking the maps, to protect them from concurrent access
		HANDLE hMutex;
	};





	void CleanupOtherPartner( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned __int32 partnerId ); //see below for implementation



	/**
	 * These functions are used to lock the maps to avoid concurrent access (callbacks could happen at the same time as someone asks for a partnerId linked to a name for example) 
	 * INFINITE can be given as the timeout
	 */
	bool LockMaps( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned __int32 timeoutInMilliseconds ) {
		if ( pLocalMessageBroadcastPartner->hMutex != NULL ) {
//			std::wcout << "	-> Try to lock mutex " << "\n";

			DWORD waitResult = WaitForSingleObject( pLocalMessageBroadcastPartner->hMutex, timeoutInMilliseconds );

			switch ( waitResult ) {
				case WAIT_OBJECT_0:
	//				std::wcout << "	-> Locked mutex " << "\n";
					return true;
					break;
				case WAIT_ABANDONED:
				default:
					break;
			}
		}
		return false;
	}

	bool UnlockMaps( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner ) {
//		std::wcout << "	-> Try to unlock mutex with handle " << pSharedMemory->hMutex << " " << pSharedMemory->semaphoreLocked << "\n";
		if ( pLocalMessageBroadcastPartner->hMutex != NULL  ) {
//			std::wcout << "	-> Try to unlock mutex" << pSharedMemory->semaphoreLocked << "\n";
			if ( ReleaseMutex( pLocalMessageBroadcastPartner->hMutex ) ) {
//				std::wcout << "	-> Unlocked mutex "  << "\n";
				return true;
			}
		}
		return false;
	}
	



	/// since we will only send unsigned integers around, the mailslot name 
	/// will be constructed from a string and the decimal representation of this number...
	wstring CreateMailslotName(wstring baseName, unsigned __int32 identifier) {
		//wstring retVal;
		
		wstringstream ss;
    	ss << "\\\\.\\mailslot\\" << baseName << "\\" << identifier;
		
		//retVal = ss.str();
		
		return ss.str(); //retVal;
	}


	///in the list of ids (from shared memory) find an 'untaken' id that we can use, and find an empty spot 
	///to write it back to shared memory afterwards
	bool FindMailslotIdAndEmptySharedMemorySpot(unsigned __int32 * ids, __int32 nrOfIds, __int32 & emptySpotPosition, unsigned __int32 & newReaderMailslotIdentifier) {
		
		//find an empty spot (=0), if there is one
		__int32 pos = -1;
		unsigned __int32 maxId = 0; //our id should be the max + 1 from the already existing identifiers

		for ( __int32 i = 0; i < nrOfIds; i++ ) {
			if ( ids[i] == 0 && pos == -1) {
				pos = i;
			}
			if ( ids[i] > maxId ) {
				maxId = ids[i];
			}
			//wcout << "ids[" << j << "] = " << ids[j] << " AND emptySpotPosition = " << emptySpotPosition << "\n";
		}
		if ( pos == -1 && nrOfIds < LOCAL_MSG_BROADCAST_MAX_NUMBER_OF_PARTNERS ) {
			pos = nrOfIds;
		}


		if ( pos == -1 || pos >= LOCAL_MSG_BROADCAST_MAX_NUMBER_OF_PARTNERS ) {
			return false;
		}
		
		emptySpotPosition = pos;
		newReaderMailslotIdentifier = maxId + 1;
		
		return true;
	}

	/// Closes and removes 1 single file handle for writing to partner mailslots
	bool DestroyWriterMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned __int32 myId) {
		
		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {
				map<unsigned __int32, HANDLE> * handlesMap = pLocalMessageBroadcastPartner->writerMailslotHandlesMap;
				map<unsigned __int32, wstring> * namesMap = pLocalMessageBroadcastPartner->partnerNamesMap;
	
				CloseHandle( (*handlesMap)[myId] );
		
				handlesMap->erase(myId);
				namesMap->erase(myId);
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
			return true;
		}

		return false;
	}



	/// Closes and removes file handles for writing to the partner mailslots
	bool DestroyWriterMailslots(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner) {
		
		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {
				map<unsigned __int32, HANDLE>::const_iterator itr;
		
				for( itr = pLocalMessageBroadcastPartner->writerMailslotHandlesMap->begin(); itr != pLocalMessageBroadcastPartner->writerMailslotHandlesMap->end(); ++itr ) {
					//wcout << "writerMailslotId: " << (*itr).first << " Handle: " << (*itr).second;
			
					CloseHandle( itr->second );
				}
		
				pLocalMessageBroadcastPartner->writerMailslotHandlesMap->clear();
				pLocalMessageBroadcastPartner->partnerNamesMap->clear();
		
		//		while ( pLocalMessageBroadcastPartner->hWriterMailslotList.front() != NULL ) {
		//			HANDLE hFile = pLocalMessageBroadcastPartner->hWriterMailslotList.front();
		//
		//			CloseHandle(hFile);
		//			
		//			pLocalMessageBroadcastPartner->hWriterMailslotList.pop_front();
		//		}
		//		
		//		pLocalMessageBroadcastPartner->hWriterMailslotList.clear();
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
			return true;
		}

		return false;
	}


	///Create 1 single writermailslot, and put it into the list of known partners
	bool CreateWriterMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned __int32 id) {

		wstring name = CreateMailslotName(*(pLocalMessageBroadcastPartner->sharedMemoryName), id);

		//wcout << "CreateWriterMailslot named '" << name << "'\n";

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
//			wcout << "CreateFile failed with " << GetLastError() << ".\n";
						
			return false; 
		}
		
		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {
				(*pLocalMessageBroadcastPartner->writerMailslotHandlesMap)[id] = hFile; 
				//We don't know the name yet, so don't fill NamesMap yet...
				////(*pLocalMessageBroadcastPartner->partnerNamesMap)[id] = *(new wstring());
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
			return true;
		}
		
		return false;
	}


	/// Creates file handles for writing to all of the partner mailslots
	bool CreateWriterMailslots(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned __int32 * ids, __int32 nrOfIds) {
	
		bool success = true;
	
		for ( __int32 i = 0; i < nrOfIds; i++ ) {
			unsigned __int32 id = ids[i];
			
			//wcout << "Current mailslot id = " << id << " ";
			
			if ( id == 0 ) {
				//wcout << " = 0 so don't try to open it..." << "\n";
			}
			else {
				//wcout << " != 0 so try to open the mailslot ";
				if ( ! CreateWriterMailslot(pLocalMessageBroadcastPartner, id) ) {
					//wcout << "created successfully" << "\n";
					success = false;;
				}
				else {
					//wcout << " NOT CREATED !!!" << "\n";
				}
			}		
		}
		
		return success;
	}


	bool DestroyReaderMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner) {

		CloseHandle( pLocalMessageBroadcastPartner->hReaderMailslot );
		
		return true;
	}


	/// Creates a mailslot handle so we can listen for messages
	bool CreateReaderMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner) {
		wstring name = CreateMailslotName( *(pLocalMessageBroadcastPartner->sharedMemoryName), pLocalMessageBroadcastPartner->myId);
		
		pLocalMessageBroadcastPartner->hReaderMailslot = CreateMailslot( 	name.c_str(), 
													        0,                             // no maximum message size 
													        READERMAILSLOT_TIMEOUT, //MAILSLOT_WAIT_FOREVER,         // no time-out for operations 
													        (LPSECURITY_ATTRIBUTES) NULL); // default security
		
		if ( pLocalMessageBroadcastPartner->hReaderMailslot == INVALID_HANDLE_VALUE ) { 
//			wcout << "CreateMailslot (" << name << ") failed with " << GetLastError() << "\n";
			return false; 
		} 
	    else {
			//wcout << "Mailslot named " << name << " created successfully.\n";
	    }
	    
	    return true;
	}


	bool WriteToSingleMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned __int32 partnerId, BYTE * data, __int32 len) {
		
		bool success = true;

		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {
				if ( pLocalMessageBroadcastPartner->writerMailslotHandlesMap->count(partnerId) > 0 ) {
					HANDLE hFile = pLocalMessageBroadcastPartner->writerMailslotHandlesMap->at(partnerId);
					BOOL fResult;
					DWORD cbWritten; 

		//		    	wcout << "Try to write to mailsot " << hFile << " aka " << itr->second << "\n";
			
					fResult = WriteFile(hFile, data, len, &cbWritten, (LPOVERLAPPED) NULL);

					if ( ! fResult ) {
						success = false;
						printf( "WriteFile failed with %d.\n", GetLastError() ); 
					}
					else {
						//printf("Slot written to successfully.\n"); 
					}
				}
				else {
		//		    wcout << "No partner mailslots found, so do nothing..." << "\n";WriteToAllMailslots
					success = false;
				}
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
		}
		else {
			success = false;
		}
			
		return success;

	}


	bool WriteToAllMailslots(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, __int32 len) {
		
		bool success = true;
		list<unsigned __int32> failedPartnerIds;

//	    wcout << "Trying pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size()" << "\n";
//	    wcout << pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() << "\n";

		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {
				if ( pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() > 0 ) {
					map<unsigned __int32, HANDLE>::iterator itr;
			
		//		    wcout << "Found " << pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() << " partners, so get an iterator and try to send the message to all of them." << "\n";
			
					for ( itr = pLocalMessageBroadcastPartner->writerMailslotHandlesMap->begin(); itr != pLocalMessageBroadcastPartner->writerMailslotHandlesMap->end(); ++itr ) {
						//don't write to your own mailslot
						if ( itr->first != pLocalMessageBroadcastPartner->myId ) {
				
							//wcout << "Try to write to partner " << itr->first << "'s mailsot aka " << itr->second << "\n";
							if ( ! WriteToSingleMailslot(pLocalMessageBroadcastPartner, itr->first, data, len ) ) {
								failedPartnerIds.push_back( itr->first );
								success = false;
								
								//wcout << "failed !" << "\n";
							}
							else {
								//wcout << "success !" << "\n";
							}
					
						}
						else {
							//wcout << "Don't try to write to our own " << itr->first << " mailsot aka " << itr->second << "\n";
						}
					}
				}
				else {
		//		    wcout << "No partner mailslots found, so do nothing..." << "\n";			
				}
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
		}
		else {
			success = false;
		}


		if ( failedPartnerIds.size() > 0 ) {
			list<unsigned __int32>::iterator itr;

			for ( itr = failedPartnerIds.begin(); itr != failedPartnerIds.end(); ++itr ) {
			    wcout << "It seems that partner with id=" << (*itr) << " has left without saying goodbye. How rude..." << "\n";			
				wcout.flush();

				///////////////////////////////////////////////////////////
				// let everyone ekse know that this partner has left !!! //
				///////////////////////////////////////////////////////////
				CleanupOtherPartner( pLocalMessageBroadcastPartner, *itr );
			}
		}

		return success;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId ; unsigned __int32 appnamelength ; wchar_t * myName
	__int32 CreateHelloMessage(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE ** data ) {

		__int32 dataSize = 1 + sizeof(pLocalMessageBroadcastPartner->myId) + sizeof(unsigned __int32) + pLocalMessageBroadcastPartner->myName->length() * sizeof(wchar_t);
		(*data) = new BYTE[dataSize];
		(*data)[0] = LOCAL_MSG_BROADCAST_HELLO;

		BYTE * addr = (*data) + 1;
		__int32 len = sizeof(pLocalMessageBroadcastPartner->myId);
		CopyMemory( addr, (VOID *) &pLocalMessageBroadcastPartner->myId, len );
		addr += len;
		len = sizeof(unsigned __int32);
		unsigned __int32 strlen = pLocalMessageBroadcastPartner->myName->length();
		CopyMemory( addr, (VOID *) &strlen, len );
		addr += len;
		len = pLocalMessageBroadcastPartner->myName->length() * sizeof(wchar_t);
		CopyMemory( addr, (VOID *) pLocalMessageBroadcastPartner->myName->c_str(), len );
		addr += len;
		
		if ( dataSize != addr - (*data) ) {
//			wcout << "WHAT'S THIS??? dataSize = " << dataSize << " but addr - data = " << addr - (*data) << "\n";
			throw L"[CreateHelloMessage] dataSize and real length of generated message don't match"; //10;
		}
		
		return dataSize;
	}

	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId ; unsigned __int32 appnamelength ; wchar_t * myName
	__int32 CreateStopListeningMessage(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE ** data ) {

		__int32 dataSize = 1;
		(*data) = new BYTE[dataSize];
		(*data)[0] = LOCAL_MSG_BROADCAST_STOPLISTENING;
		
		return dataSize;
	}

	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId
	__int32 CreateGoodbyeMessage(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE ** data, unsigned __int32 leavingPartnerId ) {

		__int32 dataSize = 1 + sizeof( leavingPartnerId );
		(*data) = new BYTE[dataSize];
		(*data)[0] = LOCAL_MSG_BROADCAST_GOODBYE;

		BYTE * addr = (*data) + 1;
		__int32 len = sizeof( leavingPartnerId );
		CopyMemory( addr, (VOID *) &leavingPartnerId, len );
		addr += len;
		
		return dataSize;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId
	__int32 CreateGoodbyeMessage(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE ** data ) {

		return CreateGoodbyeMessage( pLocalMessageBroadcastPartner, data, pLocalMessageBroadcastPartner->myId );

		//__int32 dataSize = 1 + sizeof(pLocalMessageBroadcastPartner->myId);
		//(*data) = new BYTE[dataSize];
		//(*data)[0] = LOCAL_MSG_BROADCAST_GOODBYE;

		//BYTE * addr = (*data) + 1;
		//__int32 len = sizeof(pLocalMessageBroadcastPartner->myId);
		//CopyMemory( addr, (VOID *) &pLocalMessageBroadcastPartner->myId, len );
		//addr += len;
		//
		//return dataSize;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId ; SharedObjectInfo
	__int32 CreateMessage( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, void * pObject, unsigned __int32 objectSize, BYTE ** data ) {
		__int32 dataSize = 1 + sizeof(pLocalMessageBroadcastPartner->myId) + sizeof(objectSize) + objectSize;
		
		//if ( dataSize > maxMsgSize ) {
		//	throw "Max message size is " + maxMsgSize + "and the message you want to send would have a size of " + dataSize + " bytes.";
		//}
		
		(*data) = new BYTE[dataSize];
		(*data)[0] = LOCAL_MSG_BROADCAST_MESSAGE;
		
		BYTE * addr = (*data) + 1;
		__int32 len = sizeof(pLocalMessageBroadcastPartner->myId);
		CopyMemory( addr, (VOID *) &pLocalMessageBroadcastPartner->myId, len );

		addr += len;
		len = sizeof(objectSize);
		CopyMemory( addr, (VOID *) &objectSize, len );

		addr += len;
		len = objectSize;
		CopyMemory( addr, (VOID *) pObject, len );
		
		addr += len;
		
//		wcout << "CreateBroadcastMessage dataSize=" << dataSize << " objectSize=" << objectSize << "\n";
		
		return dataSize;
	}

	/// Add this partner to the list of known partners
	/// Then send him a hello message (so he knows our name) and send him all of the stuff we are sharing
	bool ProcessReceivedHelloMessage( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, __int32 length ) {
		bool success = true;

		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {
				//parse the message
				///////////////////
				BYTE * pData = data + 1;
				unsigned __int32 partnerId = *((unsigned __int32 *)pData);
		
				pData += sizeof(partnerId);
				unsigned __int32 name_length = *((unsigned __int32 *)pData);
		
				pData += sizeof(unsigned __int32);
				wchar_t * name = (wchar_t*)pData;

				//wcout << "The new message partnerId = " << partnerId << " and name has length " << name_length << "\n";

				wstring wstrName(L"");
				wstrName.append( name, name_length );
		
				//wcout << " and name = " << wstrName << "\n";

				// Add to the list of known partners: update the writerMailslotsMap
				///////////////////////////////////////////////////////////////////

				//find and open the mailslot and store its handle
				if ( ! CreateWriterMailslot( pLocalMessageBroadcastPartner, partnerId ) ) {
					success = false;
				}
		
				map<unsigned __int32, wstring> * namesMap = pLocalMessageBroadcastPartner->partnerNamesMap;

				//(*handlesMap)[partnerId]
				((*namesMap)[partnerId]).clear(); //clear the current name for the mailslot string
				(*namesMap)[partnerId].append( name, name_length );
		
				//wcout << "The new message partnerId = " << partnerId << " and name " << (*namesMap)[partnerId] <<  " (length=" << name_length << ")" << "\n";


				// Send back a 'welcome' message, so the new partner knows our name
				///////////////////////////////////////////////////////////////////
		
				// (a welcome message is just a hello message with a different msgType)
				map<unsigned __int32, HANDLE> * handlesMap = pLocalMessageBroadcastPartner->writerMailslotHandlesMap;
				HANDLE hPartnerMailslot = (*handlesMap)[partnerId];

				BYTE * welcomeMsgData;
				__int32 welcomeMsgDataSize = CreateHelloMessage(pLocalMessageBroadcastPartner, &welcomeMsgData);
				welcomeMsgData[0] = LOCAL_MSG_BROADCAST_WELCOME;
		

				DWORD cbWritten;
				BOOL fResult = WriteFile(hPartnerMailslot, welcomeMsgData, welcomeMsgDataSize, &cbWritten, (LPOVERLAPPED) NULL);

				if (!fResult) {
					success = false;
					printf("WriteFile failed with %d.\n", GetLastError()); 
				}
				else {
					//printf("Slot written to successfully.\n"); 
				}

				//call the callback function
				if ( pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc != NULL ) {
					try {
						(*(pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc))(pLocalMessageBroadcastPartner, partnerId, wstrName.c_str(), pLocalMessageBroadcastPartner->callbackFuncCustomData);
					} catch (exception) {
					}
				}
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
		}
		else {
			success = false;
		}

		return success;
	}

	/// Add this partner to the list of known partners
	bool ProcessReceivedWelcomeMessage( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, __int32 length ) {
		bool success = true;
		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {
				//parse the message
				///////////////////
				BYTE * pData = data + 1;
				unsigned __int32 partnerId = *((unsigned __int32 *)pData);
		
				pData += sizeof(partnerId);
				unsigned __int32 name_length = *((unsigned __int32 *)pData);
		
				pData += sizeof(unsigned __int32);
				wchar_t * name = (wchar_t*)pData;

				//wcout << "The new message partnerId = " << partnerId << " and name has length " << name_length << "\n";

				wstring wstrName(L"");
				wstrName.append( name, name_length );
		
				//wcout << " and name = " << wstrName << "\n";

				// Add to the list of known partners: update the writerMailslotsMap
				///////////////////////////////////////////////////////////////////
				map<unsigned __int32, wstring> * namesMap = pLocalMessageBroadcastPartner->partnerNamesMap;

				//find and open the mailslot and store its handle
				if ( ! CreateWriterMailslot( pLocalMessageBroadcastPartner, partnerId ) ) {
					success = false;
				}
		
				((*namesMap)[partnerId]).clear(); //clear the current name for the mailslot string
				(*namesMap)[partnerId].append( name, name_length );
		
		//		wcout << "The new message partnerId = " << partnerId << " and name " << (*namesMap)[partnerId] <<  " (length=" << name_length << ")" << "\n";

				//call the callback function
				if ( pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc != NULL ) {
					try {
						(*(pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc))(pLocalMessageBroadcastPartner, partnerId, wstrName.c_str(), pLocalMessageBroadcastPartner->callbackFuncCustomData);
					} catch (exception) {
					}
				}
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
		}
		else {
			success = false;
		}

		return success;
	}


	/// Remove this partner from the list of known partners and stop listening, also remove all of his shared resources
	bool ProcessReceivedGoodbyeMessage( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, __int32 length ) {
		bool success = true;


		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {

				//parse the message
				///////////////////
				BYTE * pData = data + 1;
				unsigned __int32 partnerId = *((unsigned __int32 *)pData);
		
		
				//call the callback function
				if ( pLocalMessageBroadcastPartner->partnerLeftCallbackFunc != NULL ) {
					try {
						(*(pLocalMessageBroadcastPartner->partnerLeftCallbackFunc))(pLocalMessageBroadcastPartner, partnerId, pLocalMessageBroadcastPartner->callbackFuncCustomData);
					} catch (exception) {
					}
				}

		
				// Remove from the list of known partners
				/////////////////////////////////////////
				DestroyWriterMailslot( pLocalMessageBroadcastPartner, partnerId );
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
		}
		else {
			success = false;
		}

		return success;
	}


	/// Add this object to the list of 'shared-by-partners' objects
	bool ProcessReceivedMessage(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, unsigned __int32 length ) {

		//wcout << "ProcessReceivedShareObjectMessage received a message of size " << length << "\n";

		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		__int32 len = sizeof(unsigned __int32);
		unsigned __int32 partnerId = *((unsigned __int32 *)pData);
		
		pData += len;
		len = sizeof(unsigned __int32);
		unsigned __int32 msgLength;
		CopyMemory(&msgLength, (VOID *) pData, len );

		pData += len;
		len = (__int32)msgLength;
		void * pMsgData = (void*) new BYTE[len];
		CopyMemory(pMsgData, (void *) pData, len );

		//wcout << "Trying to call msgReceivedCallbackFunc ... Data shared by "<< partnerId << " msgLength=" << msgLength //<< " '" << (LPTSTR) pMsgData << "'" << "\n";

		//callback that a message has been received
		if ( pLocalMessageBroadcastPartner->msgReceivedCallbackFunc != NULL ) {
			try {
				((pLocalMessageBroadcastPartner->msgReceivedCallbackFunc))(pLocalMessageBroadcastPartner, partnerId, pMsgData, msgLength, pLocalMessageBroadcastPartner->callbackFuncCustomData);
			} catch (exception) {
			}
		}

		//wcout << "msgReceivedCallbackFunc FINISHED SUCCESSFULLY " << "\n";
		
		return true;
	}


	bool RemoveIdFromSharedMemory( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned __int32 idToBeRemoved ) {
		
		bool success = false;

		if ( LockSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, 2000 ) ) {

//			wcout << "Trying to read shared memory" << "\n";
			
			BYTE * idBytes = NULL;
			__int32 nrOfIds = ReadSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned __int32);
			unsigned __int32 * ids = (unsigned __int32 *)idBytes;

			//find ourself in shareddata
			__int32 ourPosition = -1;

			for ( __int32 i = 0; i < nrOfIds; i++ ) {
				if ( ids[i] == idToBeRemoved ) {
					ourPosition = i;
				}
			}

//			wcout << "delete read bytes" << "\n";
			delete idBytes;

//			wcout << "Trying to write to shared memory at position " << ourPosition << "\n";

			unsigned __int32 zero = 0;
			//Remove yourself from the list
			WriteSharedMemory( 	pLocalMessageBroadcastPartner->hSharedMemory, 
								(BYTE*) &zero,							//data
								sizeof(unsigned __int32),				//length
								ourPosition * sizeof(unsigned __int32)	//offset								
							);
			
			success = UnlockSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory );
		}

		return success;
	}

	/**
     * Remove a partner from sharedMemory and tell all the partners that he faded away
     * (possibly after we could not deliver a message)
     */
    void CleanupOtherPartner( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned __int32 partnerId ) {
        // Cleanup shared memory
		RemoveIdFromSharedMemory( pLocalMessageBroadcastPartner, partnerId );


        BYTE * data;
        __int32 dataSize = CreateGoodbyeMessage( pLocalMessageBroadcastPartner, &data, partnerId );

		// simulate "write to own mailslot", we want to cleanup our data as well
        ProcessReceivedGoodbyeMessage( pLocalMessageBroadcastPartner, data, dataSize );

		// tell all other partners that this partner died
        WriteToAllMailslots( pLocalMessageBroadcastPartner, (BYTE *) data, dataSize );
        
    }


	//THIS THREAD WILL BE READING THE MAILSLOT, and calling callback functions when needed
	void ListenerThread( void* pLocalMessageBroadcastPartnerHandle ) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) ( * (HANDLE*) pLocalMessageBroadcastPartnerHandle );
		
//		wcout << "ListenerThread function: " << *(pLocalMessageBroadcastPartner->myName) << "\n";

		DWORD readTimeout = READERMAILSLOT_TIMEOUT;
		//SetMailslotInfo( pLocalMessageBroadcastPartner->hReaderMailslot, readTimeout );			


		while ( pLocalMessageBroadcastPartner->listenerThreadRunning ) {
		
			DWORD nextSize;
			DWORD messageCount;

			//wcout << "ListenerThread checking for new messages..." << "\n";
			//wcout << " .";
			//wcout.flush();

			//////////////////////////////////////////////////////////////////////////////////////////////////
			// Some explanation: I thought GetmailslotInfo would block but actually ReadFile blocks
			// It blocks for a maximum time as set when creating the Mailslot (could be MAILSLOT_WAIT_FOREVER)
			// So we will try to read 0 bytes, and when that returns, we can call GetMailslotInfo to retrieve 
			// the number and size of the new message(s). If we use Readfile then, we know it won't block.
			//////////////////////////////////////////////////////////////////////////////////////////////////
			
			//Block until new data arrives (or until readTimeout set when creating the mailslot is over) !!!!
			char dummy;
			DWORD dummyNrOfBytesRead;
			ReadFile( pLocalMessageBroadcastPartner->hReaderMailslot, &dummy, 0, &dummyNrOfBytesRead, NULL);


			GetMailslotInfo( pLocalMessageBroadcastPartner->hReaderMailslot, NULL, &nextSize, &messageCount, &readTimeout);

			//wcout << "readTimeout = " << readTimeout << "\n";

			if ( nextSize != MAILSLOT_NO_MESSAGE ) {
				//read the message, and call the callback function
				
				DWORD nrOfBytesRead;

				if ( pLocalMessageBroadcastPartner->currentReadBufferSize < nextSize ) {
					//allocate new, larger buffer
					delete(pLocalMessageBroadcastPartner->readBuffer);
					__int32 newReadBufferSize = nextSize + 1024;
					pLocalMessageBroadcastPartner->readBuffer = new BYTE[newReadBufferSize];
					pLocalMessageBroadcastPartner->currentReadBufferSize = newReadBufferSize;
				}

				BYTE * buffer = pLocalMessageBroadcastPartner->readBuffer;

				//WE NEED TO KNOW THE CORRECT MESSAGE SIZE = nextSize for each message
				//for ( __int32 i = 0; i < messageCount; i++ ) {
				do {
					if ( ReadFile( pLocalMessageBroadcastPartner->hReaderMailslot, buffer, nextSize, &nrOfBytesRead, NULL) ) {
						//parse message and do the appropriate callback
						if ( buffer[0] == LOCAL_MSG_BROADCAST_HELLO ) {
							//wcout << "ListenerThread received a [LOCAL_MSG_BROADCAST_HELLO] message." << "\n";
							
							ProcessReceivedHelloMessage( pLocalMessageBroadcastPartner, buffer, nrOfBytesRead);
						}
						else if ( buffer[0] == LOCAL_MSG_BROADCAST_WELCOME ) {
							//wcout << "ListenerThread received a [LOCAL_MSG_BROADCAST_WELCOME] message." << "\n";
							ProcessReceivedWelcomeMessage( pLocalMessageBroadcastPartner, buffer, nrOfBytesRead);
						}
						else if ( buffer[0] == LOCAL_MSG_BROADCAST_GOODBYE ) {
							//wcout << "ListenerThread received a [LOCAL_MSG_BROADCAST_GOODBYE] message." << "\n";
							ProcessReceivedGoodbyeMessage( pLocalMessageBroadcastPartner, buffer, nrOfBytesRead);
						}
						else if ( buffer[0] == LOCAL_MSG_BROADCAST_MESSAGE ) {
							//wcout << "ListenerThread received a [LOCAL_MSG_BROADCAST_SHARE] message." << "\n";
							ProcessReceivedMessage( pLocalMessageBroadcastPartner, buffer, nrOfBytesRead);
						}
						else if ( buffer[0] == LOCAL_MSG_BROADCAST_STOPLISTENING ) {
							//stop listening for new messages: this message should only be sent by DestroyPartner !!!
							pLocalMessageBroadcastPartner->listenerThreadRunning = false;
						}
						else {
							//unknown message type, read next
						}
					}
					
					//wcout << "CHECK if there are more messages." << "\n";
					GetMailslotInfo( pLocalMessageBroadcastPartner->hReaderMailslot, NULL, &nextSize, &messageCount, &readTimeout);
					//wcout << "CHECKED if there are more messages. " << messageCount << " messages left." << "\n";
				} while ( nextSize != MAILSLOT_NO_MESSAGE );
			}
			else {
				//wcout << "ListenerThread received NO new messages... Let's listen again." << "\n";
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
	bool DestroyLocalMessageBroadcastPartner(HANDLE localMessageBroadcastPartnerHandle) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;

//		wcout << "Trying to DestroyLocalMessageBroadcastPartner " << pLocalMessageBroadcastPartner << "\n";
		
		//Send a STOP message to our own mailslot, so it stops listening for new messages
		/////////////////////////////////////////////////////////////////////////////////
		BYTE * data;
		__int32 dataSize = CreateStopListeningMessage(pLocalMessageBroadcastPartner, &data);

		HANDLE hFile = (*pLocalMessageBroadcastPartner->writerMailslotHandlesMap)[pLocalMessageBroadcastPartner->myId];
		BOOL fResult;
		DWORD cbWritten;
		fResult = WriteFile(hFile, data, dataSize, &cbWritten, (LPOVERLAPPED) NULL);

		if (fResult) {
			//STOP essage sent, so Wait until listenerThread is done
			__int32 slept = 0;
			while ( pLocalMessageBroadcastPartner->listenerThreadRunning && slept <= READERMAILSLOT_TIMEOUT ) {
				Sleep(1);
			}
		}
		
		//Still running ???
		if ( pLocalMessageBroadcastPartner->listenerThreadRunning ) {
			//sending STOP message failed, so set 'running' to false here, and wait long enough so the thread will certainly have stopped reading
			printf("WriteFile failed with %d.\n", GetLastError());
			pLocalMessageBroadcastPartner->listenerThreadRunning = false;
			Sleep(READERMAILSLOT_TIMEOUT + 200);
		}
		
		delete pLocalMessageBroadcastPartner->readBuffer;



		//Remove yourself from sharedMemory
		///////////////////////////////////
		bool success = false;

//		wcout << "Trying to LockSharedMemory" << "\n";

//		wcout << "Trying to DestroyReaderMailslot" << "\n";

		//close readermailslot: stop listening to new messages
		DestroyReaderMailslot(pLocalMessageBroadcastPartner);

//		if ( LockSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, 2000 ) ) {

			success = RemoveIdFromSharedMemory( pLocalMessageBroadcastPartner, pLocalMessageBroadcastPartner->myId );

////			wcout << "Trying to read shared memory" << "\n";
//			
//			BYTE * idBytes = NULL;
//			__int32 nrOfIds = ReadSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned __int32);
//			unsigned __int32 * ids = (unsigned __int32 *)idBytes;
//
//			//find ourself in shareddata
//			__int32 ourPosition = -1;
//
//			for ( __int32 i = 0; i < nrOfIds; i++ ) {
//				if ( ids[i] == pLocalMessageBroadcastPartner->myId ) {
//					ourPosition = i;
//				}
//			}
//
////			wcout << "delete read bytes" << "\n";
//			delete idBytes;
//
////			wcout << "Trying to write to shared memory at position " << ourPosition << "\n";
//
//			unsigned __int32 zero = 0;
//			//Remove yourself from the list
//			WriteSharedMemory( 	pLocalMessageBroadcastPartner->hSharedMemory, 
//								(BYTE*) &zero,						//data
//								sizeof(unsigned __int32),				//length
//								ourPosition * sizeof(unsigned __int32)	//offset								
//							);
//			
//
//			success = UnlockSharedMemory(pLocalMessageBroadcastPartner->hSharedMemory);
//		}

		
		//Write a 'goodbye' message to all writermailslots
//		wcout << "Trying to create goodbye message" << "\n";
		dataSize = CreateGoodbyeMessage(pLocalMessageBroadcastPartner, &data);
		
//		wcout << "Trying to write goodbye message to all parter mailslots" << "\n";
//	    wcout << "Trying pLocalMessageBroadcastPartner = " << pLocalMessageBroadcastPartner << "\n";
//	    wcout << "Trying pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size()" << "\n";
//	    wcout << pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() << "\n";
		WriteToAllMailslots( pLocalMessageBroadcastPartner, (BYTE *) data, dataSize );
		
//		wcout << "Trying to delete goodbye message" << "\n";
		delete data;

//		wcout << "Trying to destroy writer mailslots" << "\n";

		//close all writermailslots
		DestroyWriterMailslots( pLocalMessageBroadcastPartner );
		
		pLocalMessageBroadcastPartner->hSharedMemory = NULL;
		

		success = success && ReleaseMutex( pLocalMessageBroadcastPartner->hMutex );


		//HANDLE hSharedMemory;
		//wstring * myName;
		//unsigned __int32 myId;
		//HANDLE hReaderMailslot;
		//map<HANDLE, wstring> writerMailslotsMap;

//		wcout << "Trying to delete all memory that has been allocated..." << "\n";

		delete pLocalMessageBroadcastPartner->partnerNamesMap;
		delete pLocalMessageBroadcastPartner->writerMailslotHandlesMap;
		delete pLocalMessageBroadcastPartner->myName;
		
		delete pLocalMessageBroadcastPartner;
		
//		wcout << "Done..." << "\n";
		
		return success;
	}


	extern "C" _declspec(dllexport)
	HANDLE CreateLocalMessageBroadcastPartner( LPTSTR lpSharedMemoryName, LPTSTR myName
		, void * callbackFuncCustomData
		, LPLocalMessageBroadcastPartnerJoinedCALLBACK pPartnerJoinedCallbackFunc
		, LPLocalMessageBroadcastPartnerLeftCALLBACK pPartnerLeftCallbackFunc
		, LPMessageReceivedCALLBACK pMsgReceivedCallbackFunc 
	) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = new LocalMessageBroadcastPartnerDescriptor();
 
 		//LPTSTR lpSharedMemoryName = TEXT(LOCAL_MSG_BROADCAST_SHAREDDATA_NAME);

		pLocalMessageBroadcastPartner->sharedMemoryName = new wstring( lpSharedMemoryName );

		//wcout << "Try to CreateLocalMessageBroadcast with " << pLocalMessageBroadcastPartner->sharedMemoryName << "\n";

		pLocalMessageBroadcastPartner->hSharedMemory = CreateSharedMemory( lpSharedMemoryName, LOCAL_MSG_BROADCAST_INITIAL_NUMBER_OF_PARTNERS * sizeof(unsigned __int32), LOCAL_MSG_BROADCAST_MAX_NUMBER_OF_PARTNERS * sizeof(unsigned __int32) );
		//THIS was actually overwriting LOCAL variables when called from Wyphon.cpp to 3 and 7:   pLocalMessageBroadcastPartner->hSharedMemory = CreateSharedMemory( lpSharedMemoryName, 3, 7 );
		if ( pLocalMessageBroadcastPartner->hSharedMemory == NULL ) {
			throw "[LocalMessageBroadcast] CreateSharedMemory failed.";
			wcout << "hSharedMemory = NULL: creating new shared memory failed... " << pLocalMessageBroadcastPartner->hSharedMemory << "\n";
		}
		
		
		__int32 nameForSemaphoreLength = _tcslen( myName ) + 8 * sizeof( TCHAR );
		LPTSTR nameForSemaphore = new TCHAR[ nameForSemaphoreLength ];
		_tcscpy_s( nameForSemaphore, nameForSemaphoreLength, myName );
		_tcscat_s( nameForSemaphore, nameForSemaphoreLength, _TEXT("_SEM") );

		//create an unnamed mutex
		pLocalMessageBroadcastPartner->hMutex = CreateMutex( NULL, false, NULL );


//		wcout << "hSharedMemory = " << pLocalMessageBroadcastPartner->hSharedMemory << "\n";

//		wcout << "CreateLocalMessageBroadcast returned " << pLocalMessageBroadcastPartner->hSharedMemory << "\n";

		pLocalMessageBroadcastPartner->myName = new wstring( myName );

//		wcout << "Application Name = " << myName << " or in *(pLocalMessageBroadcastPartner->myName) = " << *(pLocalMessageBroadcastPartner->myName) << "\n";
		
		pLocalMessageBroadcastPartner->writerMailslotHandlesMap = new map<unsigned __int32, HANDLE>();
		pLocalMessageBroadcastPartner->partnerNamesMap = new map<unsigned __int32, wstring>();

//		wcout << "Created new writerMailslotHandlesMap with size " << pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() << "\n";
		
		pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc = pPartnerJoinedCallbackFunc;
		pLocalMessageBroadcastPartner->partnerLeftCallbackFunc = pPartnerLeftCallbackFunc;
		pLocalMessageBroadcastPartner->msgReceivedCallbackFunc = pMsgReceivedCallbackFunc;

		pLocalMessageBroadcastPartner->callbackFuncCustomData = callbackFuncCustomData;
//		wcout << "callbackFuncCustomData = " << callbackFuncCustomData << " " <<  pLocalMessageBroadcastPartner->callbackFuncCustomData << "\n";
		
		pLocalMessageBroadcastPartner->currentReadBufferSize = 2;
		pLocalMessageBroadcastPartner->readBuffer = new BYTE[2];

		//Open mailsot handles for every-one that's already in shared memory
		//and create one of your own and share that in shared memory
		
//		wcout << "Try to open mailslots for every handle that is already in shared data..." << "\n";
//		wcout << "LockSharedMemory" << "\n";
		bool allMailslotsReady = false;
		if ( pLocalMessageBroadcastPartner->hSharedMemory != NULL && LockSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, 1000 ) ) {
			
			BYTE * idBytes = NULL;
			__int32 nrOfIds = ReadSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned __int32);
			unsigned __int32 * ids = (unsigned __int32 *)idBytes;			

			wcout << "I found " << nrOfIds << " ids in shared data..." << "\n";

			if ( nrOfIds > 0 ) {
				
				//Some mailslots may fail, they probably forgot to 'unregister' themselves from shared memory or something else went wrong, but we don't care
				CreateWriterMailslots(pLocalMessageBroadcastPartner, ids, nrOfIds);
				
				//create your own mailslot and
				//advertise your existence in the shared memory, and to all of the partner mailslots

//				wcout << "Now try to create your own mailslot to listen to." << "\n";

				//FOR A REASON I DON'T UNDERSTAND, looping twice over the ids crashes later on (when debugging)...
				// So read the shared memory again...
//				delete idBytes;
//				nrOfIds = ReadLocalMessageBroadcast( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned __int32);
//				ids = (unsigned __int32 *)idBytes;

				
				__int32 emptySpotPosition = -1;
				unsigned __int32 newReaderMailslotIdentifier = 0;
				if ( FindMailslotIdAndEmptySharedMemorySpot(ids, nrOfIds, emptySpotPosition, newReaderMailslotIdentifier) ) {
//					wcout << "Found an empty spot in shared memory at position " << emptySpotPosition << " and the first unused ID = " << newReaderMailslotIdentifier << "\n";
					
					pLocalMessageBroadcastPartner->myId = newReaderMailslotIdentifier;

					//Now advertise yourself everywhere
					if ( CreateReaderMailslot(pLocalMessageBroadcastPartner) ) {
//						wcout << "Now try to write your own ID to shared data " << pLocalMessageBroadcastPartner->myId << "\n";
						
						//Also create a WRITER for our own mailslot !!!
					    if ( CreateWriterMailslot(pLocalMessageBroadcastPartner, pLocalMessageBroadcastPartner->myId) ) {

							//Add yourself to the list (at the first empty spot) !!!
							WriteSharedMemory( 	pLocalMessageBroadcastPartner->hSharedMemory, 
												(BYTE*) &newReaderMailslotIdentifier,		//data
												sizeof(unsigned __int32),						//length
												emptySpotPosition * sizeof(unsigned __int32)	//offset
											);
	
							//DEBUG
	/*						delete idBytes;
							nrOfIds = ReadLocalMessageBroadcast( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned __int32);
							ids = (unsigned __int32 *)idBytes;
							emptySpotPosition = -1;
							newReaderMailslotIdentifier = 0;
							if ( FindMailslotIdAndEmptySharedMemorySpot(ids, nrOfIds, emptySpotPosition, newReaderMailslotIdentifier) ) {
								wcout << "TEST TEST TEST: Found an empty spot in shared memory at position " << emptySpotPosition << " and the first unused ID = " << newReaderMailslotIdentifier << "\n";
							}
	*/
	
	
							allMailslotsReady = true;
						}
					}
				}
			}
			delete idBytes;
			
//			wcout << "UnlockSharedMemory" << "\n";
			UnlockSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory );
		}
		
		
//		wcout << "Try to start the listener thread !!!" << "\n";
		
		//START the listening thread
		pLocalMessageBroadcastPartner->listenerThreadRunning = true;
		_beginthread( ListenerThread, 0, &pLocalMessageBroadcastPartner );
		
		Sleep(50L);

		if ( allMailslotsReady ) {
//			wcout << "Now try to create a 'hello' message" << "\n";
			//Write a 'hello' message to all writermailslots
			BYTE * data;
			__int32 dataSize = CreateHelloMessage(pLocalMessageBroadcastPartner, &data);

//						wcout << "'hello' message created, try to send it to all partners by writing it to their mailslots..." << "\n";

			WriteToAllMailslots( pLocalMessageBroadcastPartner, (BYTE *) data, dataSize );
			
			delete data;
		}
		else {
			//Something FAILED
			delete pLocalMessageBroadcastPartner->sharedMemoryName;
			delete pLocalMessageBroadcastPartner->myName;
			delete pLocalMessageBroadcastPartner->writerMailslotHandlesMap;
			delete pLocalMessageBroadcastPartner->partnerNamesMap;
			
//			wcout << "Finally return the handle " << 0 << " BECAUSE SOMETHING WENT WRONG..." << "\n";
			return NULL;
		}

//		wcout << "Finally return the handle " << pLocalMessageBroadcastPartner << "\n";
		
		//Sleep(20000L);
		
		return pLocalMessageBroadcastPartner;
	}

	extern "C" _declspec(dllexport)
	bool SendMessageToSinglePartner(HANDLE localMessageBroadcastPartnerHandle, unsigned __int32 partnerId, void * data, unsigned __int32 length) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;

//		wcout << "Trying to CreateMessage " << pLocalMessageBroadcastPartner << " of length " << length << "\n";

		BYTE * msgData;
		__int32 msgDataSize = CreateMessage(pLocalMessageBroadcastPartner, data, length, &msgData);
		
		bool success = msgDataSize > 0;

		if ( success ) {
//			wcout << "Trying to WriteToSingleMailslot" << pLocalMessageBroadcastPartner << "\n";

			success = WriteToSingleMailslot( pLocalMessageBroadcastPartner, partnerId, msgData, msgDataSize );
			delete msgData;
		}

//		wcout << "Shared object with handle=" << pObject->hSharedTexture << " " << pObject->width << "x" << pObject->height << "\n";
		
		return success;	
	}
	
	extern "C" _declspec(dllexport)
	bool BroadcastMessage(HANDLE localMessageBroadcastPartnerHandle, void * data, unsigned __int32 length) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;

//		wcout << "Trying to CreateMessage " << pLocalMessageBroadcastPartner << " of length " << length << "\n";

		BYTE * msgData;
		__int32 msgDataSize = CreateMessage(pLocalMessageBroadcastPartner, data, length, &msgData);
		
		bool success = msgDataSize > 0;

		if ( success ) {
//			wcout << "Trying to WriteToAllMailslots" << pLocalMessageBroadcastPartner << "\n";

			success = WriteToAllMailslots( pLocalMessageBroadcastPartner, msgData, msgDataSize );
			delete msgData;
		}

//		wcout << "Shared object with handle=" << pObject->hSharedTexture << " " << pObject->width << "x" << pObject->height << "\n";
		
		return success;
	}

//	extern "C" _declspec(dllexport)
//	void GetBroadcastPartnerName(HANDLE localMessageBroadcastPartnerHandle, unsigned __int32 partnerId, LPTSTR name) {
//		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;
//
////		wcout << "[GetBroadcastPartnerName] localMessageBroadcastPartnerHandle=" << localMessageBroadcastPartnerHandle << " partnerId=" << partnerId << "\n";
////		wcout << "[GetBroadcastPartnerName] will return " << (*(pLocalMessageBroadcastPartner->partnerNamesMap))[partnerId] << "\n";
//
//		//name = (LPTSTR) malloc( (*(pLocalMessageBroadcastPartner->partnerNamesMap))[partnerId].size() * sizeof(wchar_t) );
//		CopyMemory( name, (*(pLocalMessageBroadcastPartner->partnerNamesMap))[partnerId].c_str(), (*(pLocalMessageBroadcastPartner->partnerNamesMap))[partnerId].size() * sizeof(wchar_t) );			
//
//		//name = (*(pLocalMessageBroadcastPartner->partnerNamesMap))[partnerId].c_str();
//	}

	extern "C" _declspec(dllexport)
	LPCTSTR GetBroadcastPartnerName(HANDLE localMessageBroadcastPartnerHandle, unsigned __int32 partnerId) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;
		
		if ( LockMaps( pLocalMessageBroadcastPartner, 500 ) ) {
			try {
				return (*(pLocalMessageBroadcastPartner->partnerNamesMap))[partnerId].c_str();
			}
			finally {
				UnlockMaps( pLocalMessageBroadcastPartner );
			}
		}

		return TEXT( "[Error] Thread needed too much time to access mutexed resources." );
	}

	extern "C" _declspec(dllexport)
	unsigned __int32 GetBroadcastPartnerId(HANDLE localMessageBroadcastPartnerHandle) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;

		return pLocalMessageBroadcastPartner->myId;
	}

	/**
	 * GetBroadcastPartnerIdByName
	 *
	 * @param		HANDLE		the handle of the own message broadcast unit
	 * @param		LPCTSTR		the name of the partner whose id we want to retrieve
	 *							if empty, it will return the first partner in list (if any)
	 *
	 * @return		uint32		the broadcastPartner's id. Zero if not found.
	 *
	 * @author		Elio
	 */
	extern "C" _declspec(dllexport)
	unsigned __int32 GetBroadcastPartnerIdByName(HANDLE localMessageBroadcastPartnerHandle, LPCTSTR partnerName) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;

		unsigned __int32 partnerId = 0;

		map<unsigned __int32, wstring>* names = pLocalMessageBroadcastPartner->partnerNamesMap;
		map<unsigned __int32, wstring>::const_iterator it;
			// search map for a match
		for ( it = names->begin(); it != names->end(); it++ ) {
			BOOL bEqual = it->second == partnerName;
			BOOL bNoFilter = wcslen(partnerName) == 0;
			if ( bEqual || bNoFilter ) {
				partnerId = it->first;
				break;
			}
		}

		return partnerId;
	}

}