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

using namespace System;




	struct SharedMemoryDescriptor {	
		HANDLE hSemaphore;
		unsigned int semaphoreLocked;
		HANDLE hSharedMemory;
		BYTE * data;

		// These are stored in sharedmemory too:
		//unsigned int sharedMemorySize;
		//unsigned int highestByteWritten;
	};


namespace SharedMemory {



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
