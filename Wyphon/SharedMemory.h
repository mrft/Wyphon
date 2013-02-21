/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 21/10/2012
 * Time: 20:17
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
// MyClass.h
#pragma once

#include "Windows.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

//using namespace System;


namespace SharedMemory {

	struct SharedMemoryDescriptor {	
		HANDLE hSemaphore;
		unsigned __int32 semaphoreLocked;
		HANDLE hSharedMemory;
		BYTE * data;

		// These are stored in sharedmemory too:
		//unsigned __int32 sharedMemorySize;
		//unsigned __int32 highestByteWritten;
	};

	extern "C" _declspec(dllexport)
	bool LockSharedMemory(HANDLE sharedMemoryHandle, unsigned __int32 timeoutInMilliseconds);
	
	extern "C" _declspec(dllexport)
	bool UnlockSharedMemory(HANDLE sharedMemoryHandle);
	
	extern "C" _declspec(dllexport)
	__int32 WriteSharedMemory( HANDLE sharedMemoryHandle, BYTE* data, unsigned __int32 length, unsigned __int32 offset );

	extern "C" _declspec(dllexport)
	__int32 ReadSharedMemory( HANDLE sharedMemoryHandle, BYTE * &pData );
	
	extern "C" _declspec(dllexport)
	__int32 WriteStringToSharedMemory( HANDLE sharedMemoryHandle, LPTSTR data );
	
	extern "C" _declspec(dllexport)
	LPTSTR ReadStringFromSharedMemory( HANDLE sharedMemoryHandle );
	
	extern "C" _declspec(dllexport)
	bool DestroySharedMemory(HANDLE sharedMemoryHandle);
	
	extern "C" _declspec(dllexport)
	HANDLE CreateSharedMemory(LPTSTR lpName, unsigned __int32 startSize, unsigned __int32 maxSize);

//	struct ISharedMemory {
//		virtual char* TestMethod() = 0;
//		virtual void Release() = 0;
//	};

	/// <summary>
	///
	/// </summary>
//	public ref class CSharedMemory {
//		public: 
//		static char * GetSomeString()  {
//			return "hello, here's a string!!!";
//		}
//	};
	
}
