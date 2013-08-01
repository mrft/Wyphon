// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <windowsx.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here

#include <d3d9.h>

#include <gl/gl.h>


#include <Wyphon/Wyphon.h>
#include <WyphonUtils/WyphonUtils.h>


// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
