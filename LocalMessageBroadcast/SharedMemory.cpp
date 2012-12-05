/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 21/10/2012
 * Time: 20:17
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
// MyClass.cpp
#include "SharedMemory.h"
//#include <string>
//#include <Windef.h>
#include <iostream>


//extern "C" _declspec(dllexport) std::string getSomeString( /*const std::string& data, bool encrypt*/ ) {
//	return std::string("Hello World, I am a string!!!");
//}




using namespace System;

namespace SharedMemory {


	extern "C" _declspec(dllexport)
	bool LockSharedMemory(HANDLE sharedMemoryHandle, unsigned int timeoutInMilliseconds) {
		SharedMemoryDescriptor *  pSharedMemory = (SharedMemoryDescriptor *) sharedMemoryHandle;
		if ( pSharedMemory->hSemaphore != NULL /*&& pSharedMemory->semaphoreLocked*/ ) {
//			std::wcout << "	-> Try to lock semaphore " << pSharedMemory->semaphoreLocked << "\n";
			if ( WaitForSingleObject(pSharedMemory->hSemaphore, timeoutInMilliseconds) == 0 ) {
				pSharedMemory->semaphoreLocked++;
//				std::wcout << "	-> Locked semaphore " << pSharedMemory->semaphoreLocked << "\n";
				return true;
			}
		}
		return false;
	}


	extern "C" _declspec(dllexport)
	bool UnlockSharedMemory(HANDLE sharedMemoryHandle) {
		SharedMemoryDescriptor * pSharedMemory = (SharedMemoryDescriptor *) sharedMemoryHandle;
		long previousCount;
//		std::wcout << "	-> Try to unlock semaphore with handle " << pSharedMemory->hSemaphore << " " << pSharedMemory->semaphoreLocked << "\n";
		if ( (pSharedMemory->hSemaphore != NULL) && pSharedMemory->semaphoreLocked ) {
//			std::wcout << "	-> Try to unlock semaphore " << pSharedMemory->semaphoreLocked << "\n";
			if ( ReleaseSemaphore(pSharedMemory->hSemaphore, 1, &previousCount) != 0 ) {
				pSharedMemory->semaphoreLocked--;
//				std::wcout << "	-> Unlocked semaphore " << pSharedMemory->semaphoreLocked << "\n";
				return true;
			}
		}
		return false;
	}


	extern "C" _declspec(dllexport)
	int WriteSharedMemory( HANDLE sharedMemoryHandle, BYTE* data, unsigned int length, unsigned int offset ) {
		SharedMemoryDescriptor * pSharedMemory = (SharedMemoryDescriptor *) sharedMemoryHandle;

		if ( pSharedMemory->semaphoreLocked ) {
//			std::wcout << "Writing the message to the shared buffer: " << pSharedMemory->data << "\n";

			//read highestByteWritten and maxSharedMemorySize
			unsigned int highestByteWritten = ((unsigned int *)pSharedMemory->data)[0];
			unsigned int maxSharedMemorySize = ((unsigned int *)pSharedMemory->data)[1];
			
			int nrOfBytesToBeWritten = Math::Min( maxSharedMemorySize - offset, length );
			
//			std::wcout << "== I will write " << nrOfBytesToBeWritten << "/" << length << " at offset " << offset << " since sharedMemorySize = " << maxSharedMemorySize << "\n";
			
			
			unsigned int realOffset = offset + 2 * sizeof(unsigned int);

			CopyMemory( pSharedMemory->data + realOffset, data, nrOfBytesToBeWritten );
			
			//set highest byte written !
			((unsigned int *)pSharedMemory->data)[0] = (unsigned int) Math::Max( highestByteWritten, offset + nrOfBytesToBeWritten );

			//std::wcout << "== SharedMemory will return " << pSharedMemory->highestByteWritten << " bytes now when read..." << "\n";

			return nrOfBytesToBeWritten;
		}
		else {
			return -1;
		}
	}

//	extern "C" _declspec(dllexport)
//	BYTE * ReadSharedMemory( HANDLE sharedMemoryHandle ) {
//		SharedMemoryDescriptor * pSharedMemory = (SharedMemoryDescriptor *) sharedMemoryHandle;
//
//		if ( pSharedMemory->semaphoreLocked ) {
//			//std::wcout << "reading the message from the shared buffer: " << pSharedMemory->data << "\n";
//			
//			//don't return the mapped memory but a copy...
//			BYTE * retVal = (BYTE *)malloc(pSharedMemory->sharedMemorySize);
//			CopyMemory( (PVOID)retVal, pSharedMemory->data, pSharedMemory->sharedMemorySize );			
//			return retVal;
//		}
//		return NULL;
//	}


	extern "C" _declspec(dllexport)
	int ReadSharedMemory( HANDLE sharedMemoryHandle, BYTE * &pData ) {
		SharedMemoryDescriptor * pSharedMemory = (SharedMemoryDescriptor *) sharedMemoryHandle;

		if ( pSharedMemory->semaphoreLocked ) {
			//std::wcout << "reading the message from the shared buffer: " << pSharedMemory->data << "\n";
			
			//read highestByteWritten and maxSharedMemorySize
			unsigned int highestByteWritten = ((unsigned int *)pSharedMemory->data)[0];
			unsigned int maxSharedMemorySize = ((unsigned int *)pSharedMemory->data)[1];
			int offset = 2 * sizeof(unsigned int);
			
			//don't return the mapped memory but a copy...
			int nrOfBytesToBeRead = highestByteWritten;
			pData = (BYTE *)malloc(nrOfBytesToBeRead);
			CopyMemory( (PVOID)pData, pSharedMemory->data + offset, nrOfBytesToBeRead );
			return nrOfBytesToBeRead;
		}
		else {
			return -1;
		}
	}


	extern "C" _declspec(dllexport)
	int WriteStringToSharedMemory( HANDLE sharedMemoryHandle, LPTSTR data ) {
		SharedMemoryDescriptor * pSharedMemory = (SharedMemoryDescriptor *) sharedMemoryHandle;

		//+1 ? copy \0 also??
		return WriteSharedMemory( sharedMemoryHandle, (BYTE*)data, (_tcslen(data) + 1) * sizeof(TCHAR), 0 );

//		if ( pSharedMemory->semaphoreLocked ) {
////			std::wcout << "reading the message from the shared buffer: " << pSharedMemory->data << "\n";
//			int offset = 0;
//			int nrOfBytesToBeWritten = Math::Min( pSharedMemory->sharedMemorySize, offset + (_tcslen(data) * sizeof(TCHAR) ) );
//			CopyMemory( (PVOID)pSharedMemory->data, data, nrOfBytesToBeWritten );			
//			pSharedMemory->highestByteWritten = Math::Max( pSharedMemory->highestByteWritten, offset + nrOfBytesToBeWritten );
//			return true;
//		}
//		else {
//			return false;
//		}
	}

	extern "C" _declspec(dllexport)
	LPTSTR ReadStringFromSharedMemory( HANDLE sharedMemoryHandle ) {
		SharedMemoryDescriptor * pSharedMemory = (SharedMemoryDescriptor *) sharedMemoryHandle;

		if ( pSharedMemory->semaphoreLocked ) {
//			std::wcout << "reading the message from the shared buffer: " << pSharedMemory->data << "\n";
			//read highestByteWritten and maxSharedMemorySize
			unsigned int highestByteWritten = ((unsigned int *)pSharedMemory->data)[0];
			unsigned int maxSharedMemorySize = ((unsigned int *)pSharedMemory->data)[1];
			int offset = 2 * sizeof(unsigned int);

			return (LPTSTR)pSharedMemory->data + offset;
		}
		return NULL;
	}


	extern "C" _declspec(dllexport)
	bool DestroySharedMemory(HANDLE sharedMemoryHandle) {
		SharedMemoryDescriptor * pSharedMemory = (SharedMemoryDescriptor *) sharedMemoryHandle;
		
		bool success = true;
		
		UnlockSharedMemory(sharedMemoryHandle);
		
		success = CloseHandle(pSharedMemory->hSemaphore);

		UnmapViewOfFile(pSharedMemory->data);

		success = success && CloseHandle(pSharedMemory->hSharedMemory);

		pSharedMemory->hSharedMemory = NULL;
		//pSharedMemory->sharedMemorySize = -1;		
		
		return success;
	}



	#define BUF_SIZE 255



	extern "C" _declspec(dllexport)
	HANDLE CreateSharedMemory(LPTSTR lpName, unsigned int startSize, unsigned int maxSize) {
//		if (maxSize > 2) {
//			TCHAR szName[]=TEXT("Local\\MyFileMappingObject");
//			std::wcout << "Let's try CreateSharedMemory with our own name '" << szName << "' returned " << CreateSharedMemory(szName, 1, 1) <<"\n";
//			return NULL;
//		}

		int nameForSemaphoreLength = _tcslen(lpName) + 8 * sizeof(TCHAR);
		LPTSTR nameForSemaphore = new TCHAR[nameForSemaphoreLength];
		_tcscpy_s(nameForSemaphore, nameForSemaphoreLength, lpName);
		_tcscat_s(nameForSemaphore, nameForSemaphoreLength, _TEXT("_SEM") );
		//std::wcout << "NAME for semaphore = " << nameForSemaphore << "\n";

		int nameForFileMappingLength = _tcslen(lpName) + 16 * sizeof(TCHAR);
		LPTSTR nameForFileMapping = new TCHAR[nameForFileMappingLength];
		_tcscpy_s(nameForFileMapping, nameForFileMappingLength, _TEXT("Local\\") );
		_tcscat_s(nameForFileMapping, nameForFileMappingLength, lpName);
		_tcscat_s(nameForFileMapping, nameForFileMappingLength, _TEXT("_FM") );
		//std::wcout << "NAME for filemapping = " << nameForFileMapping << "\n";
		
		
		SharedMemoryDescriptor * pSharedMemory = new SharedMemoryDescriptor();	
		
		pSharedMemory->hSemaphore = NULL;
		pSharedMemory->semaphoreLocked = 0;
		pSharedMemory->hSharedMemory = NULL;
		//pSharedMemory->sharedMemorySize = maxSize;
		//pSharedMemory->highestByteWritten = startSize;
		
		int realMaxSize = maxSize + ( 2 * sizeof(unsigned int));

		//a sempahore counts DOWN if it is blocked !!! so initalCount should be equal to maxcount (1 in our case)
		pSharedMemory->hSemaphore = CreateSemaphore( NULL, 1L, 1L, nameForSemaphore );
		
		
		bool created = false;
		pSharedMemory->hSharedMemory = OpenFileMapping(
									 FILE_MAP_WRITE,		// default security
									 false,					// read/write access
									 nameForFileMapping);	// name of mapping object

		//std::wcout << "OpenFileMapping '" << nameForFileMapping << "' returned " << pSharedMemory->hSharedMemory <<"\n";

		if ( pSharedMemory->hSharedMemory == NULL ) {
			//we are probably the first, so CREATE THE FILE
			pSharedMemory->hSharedMemory = CreateFileMapping(
										 INVALID_HANDLE_VALUE,	// use paging file
										 NULL,					// default security
										 PAGE_READWRITE,		// read/write access
										 0,						// maximum object size (high-order DWORD)
										 realMaxSize, 			// maximum object size (low-order DWORD)
										 nameForFileMapping);	// name of mapping object
			
			//std::wcout << "CreateFileMapping '" << nameForFileMapping << "' returned " << pSharedMemory->hSharedMemory <<"\n";
			created = true;
			if ( pSharedMemory->hSharedMemory == NULL ) {
				std::wcout << "FAILED to create the shared buffer: " << GetLastError() <<"\n";
			}
		}
		
		if ( pSharedMemory->hSharedMemory != NULL ) {
			pSharedMemory->data = (BYTE *) MapViewOfFile(
											pSharedMemory->hSharedMemory,	// handle to map object
											FILE_MAP_ALL_ACCESS, 		// read/write permission
											0,
											0,
											realMaxSize
										);
			
			if (created) {
				//write something to this buffer
//				TCHAR szMsg[]=TEXT("Message from first process.");
//				std::wcout << "FIRST PROCESS: writing a message to the shared buffer: " << szMsg << "\n";
//				CopyMemory((PVOID)pSharedMemory->data, szMsg, (_tcslen(szMsg) * sizeof(TCHAR)));
//				std::wcout << "FIRST PROCESS: reading the written message from the shared buffer: " << pSharedMemory->data << "\n";

				//empty the whole buffer
				memset ( (PVOID)pSharedMemory->data, 0, realMaxSize );
				
				((unsigned int *)pSharedMemory->data)[0] = startSize;
				((unsigned int *)pSharedMemory->data)[1] = maxSize;
			}
			else {
				//print what's been written in the buffer
//				std::wcout << "SECOND PROCESS: reading the message from the shared buffer: " << pSharedMemory->data << "\n";
			}
		}
		else {
//			std::wcout << "FAILED to open or create the shared buffer: " << GetLastError() <<"\n";
		}
		
		delete nameForSemaphore;
		delete nameForFileMapping;
		
		bool success = pSharedMemory->hSemaphore != NULL && pSharedMemory->hSharedMemory != NULL;
		if ( ! success ) {
			DestroySharedMemory(pSharedMemory);
		}
		return success ? pSharedMemory : NULL;
		
//		std::wcout << "CreateSharedMemory influences another pointer ???" << "\n";
//		return NULL;
	}









//	class _declspec(dllexport) SharedMemory : public ISharedMemory {
//		public:
//			char* TestMethod();
//			
//			void Release();
//	};
//	
//	
//	char* SharedMemory::TestMethod() {
//		return "String returned by SharedMemory.TestMethod";
//	}
//	
//	void SharedMemory::Release()
//	{
//		delete this;
//	}
//	
//	
//	extern "C" _declspec(dllexport) ISharedMemory* /*APIENTRY*/ GetSharedMemory() {
//		return new SharedMemory();
//	}
//	
//	
//	extern "C" _declspec(dllexport) char* getSomeCharArray() {
//		return "Hello world, I am a char array!!!";
//	}


}
