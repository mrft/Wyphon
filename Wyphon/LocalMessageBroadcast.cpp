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
//typedef bool (__stdcall * pLockSharedMemoryFUNC)(HANDLE sharedDataHandle, unsigned int timeoutInMilliseconds);
//typedef bool (__stdcall * pUnlockSharedMemoryFUNC)(HANDLE sharedDataHandle);
//typedef int (__stdcall * pWriteSharedMemoryFUNC)( HANDLE sharedDataHandle, BYTE* data, unsigned int length, unsigned int offset );
//typedef int (__stdcall * pReadSharedMemoryFUNC)( HANDLE sharedDataHandle, BYTE * &pData );
//typedef int (__stdcall * pWriteStringToSharedMemoryFUNC)( HANDLE sharedDataHandle, LPTSTR data );
//typedef LPTSTR (__stdcall * pReadStringFromSharedMemoryFUNC)( HANDLE sharedDataHandle );
//typedef bool (__stdcall * pDestroySharedMemoryFUNC)(HANDLE sharedDataHandle);
//typedef HANDLE (__stdcall * pCreateSharedMemoryFUNC)(LPTSTR lpName, unsigned int startSize, unsigned int maxSize);
//
//
//HINSTANCE hSharedMemoryDll = LoadLibrary(TEXT("SharedMemory.dll"));
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// get pointers to these functions in the dll
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	bool LockSharedMemory(HANDLE sharedDataHandle, unsigned int timeoutInMilliseconds)
////	bool UnlockSharedMemory(HANDLE sharedDataHandle)
////	int WriteSharedMemory( HANDLE sharedDataHandle, BYTE* data, unsigned int length, unsigned int offset )
////	int ReadSharedMemory( HANDLE sharedDataHandle, BYTE * &pData )
////	int WriteStringToSharedMemory( HANDLE sharedDataHandle, LPTSTR data )
////	LPTSTR ReadStringFromSharedMemory( HANDLE sharedDataHandle )
////	bool DestroySharedMemory(HANDLE sharedDataHandle)
////	HANDLE CreateSharedMemory(LPTSTR lpName, unsigned int startSize, unsigned int maxSize)
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
	- HELLO (1)		|	myId (sizeof unsigned int)	|	name_length (size_t) 	|	name
	- WELCOME (1)	|	myId (sizeof unsigned int)	|	name_length (size_t) 	|	name
	- MESSAGE (1) 	|	myId		|	msg_length (sizeof unsigned int)		|	msg bytes
	- GOODBYE		|	myId
	
)
*/

using namespace std;
using namespace SharedMemory;

namespace LocalMessageBroadcast {



	// IF WE ASSUME that more types can be shared in the future
	// we will add message_types and structs to describe these resources
	// We'll start with textures only at this time

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

		unsigned int myId; //used to generate out mailsot name, and shared with the other partners
		
		HANDLE hReaderMailslot; //handle to the mailslot we are listening on

		bool listenerThreadRunning;

		map<unsigned int, HANDLE> * writerMailslotHandlesMap;	//unsigned int = the partner's myId
																//HANDLE = the handles to the mailslots we should write to

		map<unsigned int, wstring> * partnerNamesMap;	//unsigned int = the partner's myId
														//wstring = name of the partner


		LPLocalMessageBroadcastPartnerJoinedCALLBACK partnerJoinedCallbackFunc;
		LPLocalMessageBroadcastPartnerLeftCALLBACK partnerLeftCallbackFunc;
		LPMessageReceivedCALLBACK msgReceivedCallbackFunc;

		void * callbackFuncCustomData;	//will be sent with Callback functions, and can contain data specific to the user
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
	bool DestroyWriterMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned int myId) {
		
		map<unsigned int, HANDLE> * handlesMap = pLocalMessageBroadcastPartner->writerMailslotHandlesMap;
		map<unsigned int, wstring> * namesMap = pLocalMessageBroadcastPartner->partnerNamesMap;
	
		CloseHandle( (*handlesMap)[myId] );
		
		handlesMap->erase(myId);
		namesMap->erase(myId);
		
		return true;
	}



	/// Closes and removes file handles for writing to the partner mailslots
	bool DestroyWriterMailslots(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner) {
		
		map<unsigned int, HANDLE>::const_iterator itr;
		
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
		
		return TRUE;
	}


	///Create 1 single writermailslot, and put it into the list of known partners
	bool CreateWriterMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned int id) {

		wstring name = CreateMailslotName(id);

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
			wcout << "CreateFile failed with " << GetLastError() << ".\n";
						
			return false; 
		}
		
		(*pLocalMessageBroadcastPartner->writerMailslotHandlesMap)[id] = hFile; 
		
		//We don't know the name yet, so don't fill NamesMap yet...
		////(*pLocalMessageBroadcastPartner->partnerNamesMap)[id] = *(new wstring());
		
		return true;
	}


	/// Creates file handles for writing to all of the partner mailslots
	BOOL CreateWriterMailslots(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, unsigned int * ids, int nrOfIds) {
	
		bool success = true;
	
		for ( int i = 0; i < nrOfIds; i++ ) {
			unsigned int id = ids[i];
			
			//wcout << "Current mailslot id = " << id;
			
			if ( id == 0 ) {
				//wcout << " = 0 so don't try to open it..." << "\n";
			}
			else {
				//wcout << " != 0 so try to open the mailslot ";
			
				success = success && CreateWriterMailslot(pLocalMessageBroadcastPartner, id);				
			}		
		}
		
		return success;
	}


	BOOL DestroyReaderMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner) {

		CloseHandle( pLocalMessageBroadcastPartner->hReaderMailslot );
		
		return TRUE;
	}


	/// Creates a mailslot handle so we can listen for messages
	BOOL CreateReaderMailslot(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner) {
		wstring name = CreateMailslotName(pLocalMessageBroadcastPartner->myId);
		
		pLocalMessageBroadcastPartner->hReaderMailslot = CreateMailslot( 	name.c_str(), 
													        0,                             // no maximum message size 
													        READERMAILSLOT_TIMEOUT, //MAILSLOT_WAIT_FOREVER,         // no time-out for operations 
													        (LPSECURITY_ATTRIBUTES) NULL); // default security
		
		if ( pLocalMessageBroadcastPartner->hReaderMailslot == INVALID_HANDLE_VALUE ) { 
			wcout << "CreateMailslot (" << name << ") failed with " << GetLastError() << "\n";
			return FALSE; 
		} 
	    else {
	    	//wcout << "Mailslot named " << name << " created successfully.\n";
	    }
	    return TRUE; 
	}


	BOOL WriteToAllMailslots(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, int len) {
		
		BOOL success = true;

//	    wcout << "Trying pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size()" << "\n";
//	    wcout << pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() << "\n";

		if ( pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() > 0 ) {
			map<unsigned int, HANDLE>::iterator itr;
			
		    wcout << "Found " << pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() << " partners, so get an iterator and try to send the message to all of them." << "\n";
			
			for ( itr = pLocalMessageBroadcastPartner->writerMailslotHandlesMap->begin(); itr != pLocalMessageBroadcastPartner->writerMailslotHandlesMap->end(); ++itr ) {
				HANDLE hFile = itr->second;
				BOOL fResult;
				DWORD cbWritten; 
	
		    	wcout << "Try to write to mailsot " << hFile << " aka " << itr->second << "\n";
				
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
	/// BYTE msgType ; HANDLE partnerId ; size_t appnamelength ; wchar_t * myName
	int CreateHelloMessage(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE ** data ) {

		int dataSize = 1 + sizeof(pLocalMessageBroadcastPartner->myId) + sizeof(size_t) + pLocalMessageBroadcastPartner->myName->length() * sizeof(wchar_t);
		(*data) = new BYTE[dataSize];
		(*data)[0] = LOCAL_MSG_BROADCAST_HELLO;

		BYTE * addr = (*data) + 1;
		int len = sizeof(pLocalMessageBroadcastPartner->myId);
		CopyMemory( addr, (VOID *) &pLocalMessageBroadcastPartner->myId, len );
		addr += len;
		len = sizeof(size_t);
		size_t strlen = pLocalMessageBroadcastPartner->myName->length();
		CopyMemory( addr, (VOID *) &strlen, len );
		addr += len;
		len = pLocalMessageBroadcastPartner->myName->length() * sizeof(wchar_t);
		CopyMemory( addr, (VOID *) pLocalMessageBroadcastPartner->myName->c_str(), len );
		addr += len;
		
		return dataSize;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId
	int CreateGoodbyeMessage(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE ** data ) {

		int dataSize = 1 + sizeof(pLocalMessageBroadcastPartner->myId);
		(*data) = new BYTE[dataSize];
		(*data)[0] = LOCAL_MSG_BROADCAST_GOODBYE;

		BYTE * addr = (*data) + 1;
		int len = sizeof(pLocalMessageBroadcastPartner->myId);
		CopyMemory( addr, (VOID *) &pLocalMessageBroadcastPartner->myId, len );
		addr += len;
		
		return dataSize;
	}


	/// data should be deleted afterwards by the caller !!!
	/// BYTE msgType ; HANDLE partnerId ; SharedObjectInfo
	int CreateMessage( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, void * pObject, unsigned int objectSize, BYTE ** data ) {
		int dataSize = 1 + sizeof(pLocalMessageBroadcastPartner->myId) + sizeof(objectSize) + objectSize;
		
		(*data) = new BYTE[dataSize];
		(*data)[0] = LOCAL_MSG_BROADCAST_MESSAGE;
		
		BYTE * addr = (*data) + 1;
		int len = sizeof(pLocalMessageBroadcastPartner->myId);
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
	bool ProcessReceivedHelloMessage( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, int length ) {
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
		success = success && CreateWriterMailslot( pLocalMessageBroadcastPartner, partnerId );
		
		map<unsigned int, wstring> * namesMap = pLocalMessageBroadcastPartner->partnerNamesMap;

		//(*handlesMap)[partnerId]
		((*namesMap)[partnerId]).clear(); //clear the current name for the mailslot string
		(*namesMap)[partnerId].append( name, name_length );
		
		//wcout << "The new message partnerId = " << partnerId << " and name " << (*namesMap)[partnerId] <<  " (length=" << name_length << ")" << "\n";


		// Send back a 'welcome' message, so the new partner knows our name
		///////////////////////////////////////////////////////////////////
		
		// (a welcome message is just a hello message with a different msgType)
		map<unsigned int, HANDLE> * handlesMap = pLocalMessageBroadcastPartner->writerMailslotHandlesMap;
		HANDLE hPartnerMailslot = (*handlesMap)[partnerId];

		BYTE * welcomeMsgData;
		int welcomeMsgDataSize = CreateHelloMessage(pLocalMessageBroadcastPartner, &welcomeMsgData);
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
			(*(pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc))(pLocalMessageBroadcastPartner, partnerId, wstrName.c_str(), pLocalMessageBroadcastPartner->callbackFuncCustomData);
		}
		
		return success;
	}

	/// Add this partner to the list of known partners
	bool ProcessReceivedWelcomeMessage( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, int length ) {
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
		map<unsigned int, wstring> * namesMap = pLocalMessageBroadcastPartner->partnerNamesMap;

		//find and open the mailslot and store its handle
		success = success && CreateWriterMailslot( pLocalMessageBroadcastPartner, partnerId );
		
		((*namesMap)[partnerId]).clear(); //clear the current name for the mailslot string
		(*namesMap)[partnerId].append( name, name_length );
		
//		wcout << "The new message partnerId = " << partnerId << " and name " << (*namesMap)[partnerId] <<  " (length=" << name_length << ")" << "\n";

		//call the callback function
		if ( pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc != NULL ) {
			(*(pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc))(pLocalMessageBroadcastPartner, partnerId, wstrName.c_str(), pLocalMessageBroadcastPartner->callbackFuncCustomData);
		}

		return success;
	}


	/// Remove this partner from the list of known partners and stop listening, also remove all of his shared resources
	bool ProcessReceivedGoodbyeMessage( LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, int length ) {
		bool success = true;

		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		unsigned int partnerId = *((unsigned int *)pData);
		
		
		//call the callback function
		if ( pLocalMessageBroadcastPartner->partnerLeftCallbackFunc != NULL ) {
			(*(pLocalMessageBroadcastPartner->partnerLeftCallbackFunc))(pLocalMessageBroadcastPartner, partnerId, pLocalMessageBroadcastPartner->callbackFuncCustomData);
		}

		
		// Remove from the list of known partners
		/////////////////////////////////////////
		DestroyWriterMailslot( pLocalMessageBroadcastPartner, partnerId );

		return success;
	}


	/// Add this object to the list of 'shared-by-partners' objects
	bool ProcessReceivedMessage(LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner, BYTE * data, unsigned int length ) {

		//wcout << "ProcessReceivedShareObjectMessage received a message of size " << length << "\n";

		//parse the message
		///////////////////
		BYTE * pData = data + 1;
		int len = sizeof(unsigned int);
		unsigned int partnerId = *((unsigned int *)pData);
		
		pData += len;
		len = sizeof(unsigned int);
		unsigned int msgLength;
		CopyMemory(&msgLength, (VOID *) pData, len );

		pData += len;
		len = (int)msgLength;
		void * pMsgData = (void*) new BYTE[len];
		CopyMemory(pMsgData, (void *) pData, len );

		//wcout << "Trying to call msgReceivedCallbackFunc ... Data shared by "<< partnerId << " msgLength=" << msgLength //<< " '" << (LPTSTR) pMsgData << "'" << "\n";

		//callback that a message has been received
		if ( pLocalMessageBroadcastPartner->msgReceivedCallbackFunc != NULL ) {
			((pLocalMessageBroadcastPartner->msgReceivedCallbackFunc))(pLocalMessageBroadcastPartner, partnerId, pMsgData, msgLength, pLocalMessageBroadcastPartner->callbackFuncCustomData);
		}

		//wcout << "msgReceivedCallbackFunc FINISHED SUCCESSFULLY " << "\n";
		
		return true;
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
				BYTE buffer[1024];
				//WE NEED TO KNOW THE CORRECT MESSAGE SIZE = nextSize for each message
				//for ( int i = 0; i < messageCount; i++ ) {
				do {
					if ( ReadFile( pLocalMessageBroadcastPartner->hReaderMailslot, &buffer, nextSize, &nrOfBytesRead, NULL) ) {
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

		wcout << "Trying to DestroyLocalMessageBroadcastPartner " << pLocalMessageBroadcastPartner << "\n";

		//Stop listening for new messages
		pLocalMessageBroadcastPartner->listenerThreadRunning = false;
		//Wait until listenerThread is done
		Sleep(READERMAILSLOT_TIMEOUT + 200);

		//Remove yourself from sharedMemory
		bool success = false;

		wcout << "Trying to LockSharedMemory" << "\n";

		if ( LockSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, 2000 ) ) {
			wcout << "Trying to DestroyReaderMailslot" << "\n";

			//close readermailslot: stop listening to new messages
			DestroyReaderMailslot(pLocalMessageBroadcastPartner);


			wcout << "Trying to read shared memory" << "\n";
			
			BYTE * idBytes = NULL;
			int nrOfIds = ReadSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned int);
			unsigned int * ids = (unsigned int *)idBytes;

			//find ourself in shareddata
			int ourPosition = -1;

			for ( int i = 0; i < nrOfIds; i++ ) {
				if ( ids[i] == pLocalMessageBroadcastPartner->myId ) {
					ourPosition = i;
				}
			}

			wcout << "delete read bytes" << "\n";
			delete idBytes;

			wcout << "Trying to write to shared memory at position " << ourPosition << "\n";

			unsigned int zero = 0;
			//Remove yourself from the list
			WriteSharedMemory( 	pLocalMessageBroadcastPartner->hSharedMemory, 
								(BYTE*) &zero,						//data
								sizeof(unsigned int),				//length
								ourPosition * sizeof(unsigned int)	//offset								
							);
			

			success = UnlockSharedMemory(pLocalMessageBroadcastPartner->hSharedMemory);
		}

		
		//Write a 'goodbye' message to all writermailslots
		wcout << "Trying to create goodbye message" << "\n";
		BYTE * data;
		int dataSize = CreateGoodbyeMessage(pLocalMessageBroadcastPartner, &data);
		
		wcout << "Trying to write goodbye message to all parter mailslots" << "\n";
//	    wcout << "Trying pLocalMessageBroadcastPartner = " << pLocalMessageBroadcastPartner << "\n";
//	    wcout << "Trying pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size()" << "\n";
//	    wcout << pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() << "\n";
		WriteToAllMailslots( pLocalMessageBroadcastPartner, (BYTE *) data, dataSize );
		
		wcout << "Trying to delete goodbye message" << "\n";
		delete data;

		wcout << "Trying to destroy writer mailslots" << "\n";

		//close all writermailslots
		DestroyWriterMailslots(pLocalMessageBroadcastPartner);
		
		pLocalMessageBroadcastPartner->hSharedMemory = NULL;
		

		//HANDLE hSharedMemory;
		//wstring * myName;
		//unsigned int myId;
		//HANDLE hReaderMailslot;
		//map<HANDLE, wstring> writerMailslotsMap;

		wcout << "Trying to delete all memory that has been allocated..." << "\n";

		delete pLocalMessageBroadcastPartner->partnerNamesMap;
		delete pLocalMessageBroadcastPartner->writerMailslotHandlesMap;
		delete pLocalMessageBroadcastPartner->myName;
		
		delete pLocalMessageBroadcastPartner;
		
		wcout << "Done..." << "\n";
		
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

//		wcout << "Try to CreateLocalMessageBroadcast with " << pLocalMessageBroadcastPartner->sharedMemoryName << "\n";

		pLocalMessageBroadcastPartner->hSharedMemory = CreateSharedMemory( lpSharedMemoryName, LOCAL_MSG_BROADCAST_INITIAL_NUMBER_OF_PARTNERS * sizeof(unsigned int), LOCAL_MSG_BROADCAST_MAX_NUMBER_OF_PARTNERS * sizeof(unsigned int) );
		//THIS was actually overwriting LOCAL variables when called from Wyphon.cpp to 3 and 7:   pLocalMessageBroadcastPartner->hSharedMemory = CreateSharedMemory( lpSharedMemoryName, 3, 7 );
		
//		wcout << "hSharedMemory = " << pLocalMessageBroadcastPartner->hSharedMemory << "\n";

//		wcout << "CreateLocalMessageBroadcast returned " << pLocalMessageBroadcastPartner->hSharedMemory << "\n";

		pLocalMessageBroadcastPartner->myName = new wstring( myName );

//		wcout << "Application Name = " << myName << " or in *(pLocalMessageBroadcastPartner->myName) = " << *(pLocalMessageBroadcastPartner->myName) << "\n";
		
		pLocalMessageBroadcastPartner->writerMailslotHandlesMap = new map<unsigned int, HANDLE>();
		pLocalMessageBroadcastPartner->partnerNamesMap = new map<unsigned int, wstring>();

//		wcout << "Created new writerMailslotHandlesMap with size " << pLocalMessageBroadcastPartner->writerMailslotHandlesMap->size() << "\n";
		
		pLocalMessageBroadcastPartner->partnerJoinedCallbackFunc = pPartnerJoinedCallbackFunc;
		pLocalMessageBroadcastPartner->partnerLeftCallbackFunc = pPartnerLeftCallbackFunc;
		pLocalMessageBroadcastPartner->msgReceivedCallbackFunc = pMsgReceivedCallbackFunc;

		pLocalMessageBroadcastPartner->callbackFuncCustomData = callbackFuncCustomData;
//		wcout << "callbackFuncCustomData = " << callbackFuncCustomData << " " <<  pLocalMessageBroadcastPartner->callbackFuncCustomData << "\n";
		
		
		//Open mailsot handles for every-one that's already in shared memory
		//and create one of your own and share that in shared memory
		
//		wcout << "Try to open mailslots for every handle that is already in shared data..." << "\n";
//		wcout << "LockSharedMemory" << "\n";
		bool allMailslotsReady = false;
		if ( LockSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, 1000 ) ) {
			
			BYTE * idBytes = NULL;
			int nrOfIds = ReadSharedMemory( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned int);
			unsigned int * ids = (unsigned int *)idBytes;			

//			wcout << "I found " << nrOfIds << " ids in shared data..." << "\n";

			if ( nrOfIds > 0 && CreateWriterMailslots(pLocalMessageBroadcastPartner, ids, nrOfIds) ) {
				//create your own mailslot and
				//advertise your existence in the shared memory, and to all of the partner mailslots

//				wcout << "Now try to create your own mailslot to listen to." << "\n";

				//FOR A REASON I DON'T UNDERSTAND, looping twice over the ids crashes later on (when debugging)...
				// So read the shared memory again...
//				delete idBytes;
//				nrOfIds = ReadLocalMessageBroadcast( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned int);
//				ids = (unsigned int *)idBytes;

				
				int emptySpotPosition = -1;
				unsigned int newReaderMailslotIdentifier = 0;
				if ( FindMailslotIdAndEmptySharedMemorySpot(ids, nrOfIds, emptySpotPosition, newReaderMailslotIdentifier) ) {
//					wcout << "Found an empty spot in shared memory at position " << emptySpotPosition << " and the first unused ID = " << newReaderMailslotIdentifier << "\n";
					
					pLocalMessageBroadcastPartner->myId = newReaderMailslotIdentifier;

					//Now advertise yourself everywhere
					if ( CreateReaderMailslot(pLocalMessageBroadcastPartner) ) {
//						wcout << "Now try to write your own ID to shared data " << pLocalMessageBroadcastPartner->myId << "\n";
						
						//Add yourself to the list (at the first empty spot) !!!
						WriteSharedMemory( 	pLocalMessageBroadcastPartner->hSharedMemory, 
											(BYTE*) &newReaderMailslotIdentifier,		//data
											sizeof(unsigned int),						//length
											emptySpotPosition * sizeof(unsigned int)	//offset
										);

						//DEBUG
/*						delete idBytes;
						nrOfIds = ReadLocalMessageBroadcast( pLocalMessageBroadcastPartner->hSharedMemory, idBytes ) / sizeof(unsigned int);
						ids = (unsigned int *)idBytes;
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
			int dataSize = CreateHelloMessage(pLocalMessageBroadcastPartner, &data);

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
			
			wcout << "Finally return the handle " << 0 << " BECAUSE SOMETHING WENT WRONG..." << "\n";
			return NULL;
		}

		wcout << "Finally return the handle " << pLocalMessageBroadcastPartner << "\n";
		
		//Sleep(20000L);
		
		return pLocalMessageBroadcastPartner;
	}


	extern "C" _declspec(dllexport)
	bool BroadcastMessage(HANDLE localMessageBroadcastPartnerHandle, void * data, unsigned int length) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;

		wcout << "Trying to CreateMessage " << pLocalMessageBroadcastPartner << " of length " << length << "\n";

		BYTE * msgData;
		int msgDataSize = CreateMessage(pLocalMessageBroadcastPartner, data, length, &msgData);
		
		bool success = msgDataSize > 0;

		if ( success ) {
			wcout << "Trying to WriteToAllMailslots" << pLocalMessageBroadcastPartner << "\n";

			success = WriteToAllMailslots( pLocalMessageBroadcastPartner, msgData, msgDataSize );
			delete msgData;
		}

//		wcout << "Shared object with handle=" << pObject->hSharedTexture << " " << pObject->width << "x" << pObject->height << "\n";
		
		return success;
	}

//	extern "C" _declspec(dllexport)
//	void GetBroadcastPartnerName(HANDLE localMessageBroadcastPartnerHandle, unsigned int partnerId, LPTSTR name) {
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
	LPCTSTR GetBroadcastPartnerName(HANDLE localMessageBroadcastPartnerHandle, unsigned int partnerId) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;

		return (*(pLocalMessageBroadcastPartner->partnerNamesMap))[partnerId].c_str();
	}

	extern "C" _declspec(dllexport)
	unsigned int GetBroadcastPartnerId(HANDLE localMessageBroadcastPartnerHandle) {
		LocalMessageBroadcastPartnerDescriptor * pLocalMessageBroadcastPartner = (LocalMessageBroadcastPartnerDescriptor *) localMessageBroadcastPartnerHandle;

		return pLocalMessageBroadcastPartner->myId;
	}

}