/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 29/11/2012
 * Time: 9:57
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */

#include <windows.h>
//#include <stdio.h>
//#include <conio.h>
//#include <tchar.h>
//#include <iostream>
//#include <list>
//#include <map>
//#include <sstream>
//#include <d3d9types.h>
//#include <thread>
//#include <process.h>

#include "Wyphon.h"

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
