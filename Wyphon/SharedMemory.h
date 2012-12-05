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
		unsigned int semaphoreLocked;
		HANDLE hSharedMemory;
		BYTE * data;

		// These are stored in sharedmemory too:
		//unsigned int sharedMemorySize;
		//unsigned int highestByteWritten;
	};

	extern "C" _declspec(dllexport)
	bool LockSharedMemory(HANDLE sharedMemoryHandle, unsigned int timeoutInMilliseconds);
	
	extern "C" _declspec(dllexport)
	bool UnlockSharedMemory(HANDLE sharedMemoryHandle);
	
	extern "C" _declspec(dllexport)
	int WriteSharedMemory( HANDLE sharedMemoryHandle, BYTE* data, unsigned int length, unsigned int offset );

	extern "C" _declspec(dllexport)
	int ReadSharedMemory( HANDLE sharedMemoryHandle, BYTE * &pData );
	
	extern "C" _declspec(dllexport)
	int WriteStringToSharedMemory( HANDLE sharedMemoryHandle, LPTSTR data );
	
	extern "C" _declspec(dllexport)
	LPTSTR ReadStringFromSharedMemory( HANDLE sharedMemoryHandle );
	
	extern "C" _declspec(dllexport)
	bool DestroySharedMemory(HANDLE sharedMemoryHandle);
	
	extern "C" _declspec(dllexport)
	HANDLE CreateSharedMemory(LPTSTR lpName, unsigned int startSize, unsigned int maxSize);

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
