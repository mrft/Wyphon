// include the basic windows header files and the Direct3D header file
#include "stdafx.h"
#include "DXWyphonSender.h"

using namespace Wyphon;
using namespace WyphonUtils;

// global declarations
LPDIRECT3D9EX d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9EX d3ddev;    // the pointer to the device class
IDirect3DTexture9* pD3D9Texture;
IDirect3DSurface9* pD3D9Surface;
HANDLE DXShareHandle;
int width, height;
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw; // The transformed position for the vertex
    DWORD color;        // The vertex color
};
// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

HANDLE							hWyphonPartner;
HANDLE							hWyphonDevice;
Wyphon::WyphonD3DTextureInfo	WyphonTextureInfo;
wchar_t							WyphonApplicationNameT[WYPHON_MAX_DESCRIPTION_LENGTH+1];
wchar_t							WyphonTextureDescriptionT[WYPHON_MAX_DESCRIPTION_LENGTH+1];

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);    // closes Direct3D and releases memory

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

	width = 800;
	height = 600;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(NULL,
                          L"WindowClass",
                          L"Our First Direct3D Program",
                          WS_OVERLAPPEDWINDOW,
                          300, 300,
                          width, height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    initD3D(hWnd);

    // enter the main loop:

    MSG msg;

    while(TRUE)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(msg.message == WM_QUIT)
            break;

        render_frame();
    }

    // clean up DirectX and COM
    cleanD3D();

    return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
    HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d);    // create the Direct3D interface
	if ( hr != S_OK ) {
		return;
	}

    D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
    d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D


    // create a device class using this information and the info from the d3dpp stuct
    d3d->CreateDeviceEx(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_HARDWARE_VERTEXPROCESSING,
                      &d3dpp,
					  NULL,
                      &d3ddev);
//	D3DXCreateRenderToSurface( d3ddev, width, height, D3DFMT_A8R8G8B8, FALSE, D3DFMT_D24S8, &RtsHelper );
	HRESULT res = d3ddev->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT) D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pD3D9Texture, &DXShareHandle);
	pD3D9Texture->GetSurfaceLevel(0, &pD3D9Surface);
	d3ddev->SetRenderTarget(0,pD3D9Surface);

	lstrcpyW( WyphonApplicationNameT, _T("myApplication") );
	lstrcpyW( WyphonTextureDescriptionT, _T("myTexture") );

	hWyphonPartner = CreateWyphonPartner( WyphonApplicationNameT,NULL,NULL,NULL,NULL,NULL /* no callbacks */ );

	BOOL bRes = ShareD3DTexture(hWyphonPartner, DXShareHandle, width, height, D3DFMT_A8R8G8B8, D3DUSAGE_RENDERTARGET, WyphonTextureDescriptionT);
	if ( bRes ) {
		return;
	}

	CUSTOMVERTEX vertices[] =
    {
        { 150.0f,  50.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255, 0, 255), }, // x, y, z, rhw, color
        { 250.0f, 250.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255, 0, 255), },
        {  50.0f, 250.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 0, 255), },
    };

	// Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. We also
    // specify the FVF, so the vertex buffer knows what data it contains.
    d3ddev->CreateVertexBuffer( 3 * sizeof( CUSTOMVERTEX ),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB, NULL );

    // Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the vertices. This mechanism is required becuase vertex
    // buffers may be in device memory.
    VOID* pVertices;
    g_pVB->Lock( 0, sizeof( vertices ), ( void** )&pVertices, 0 );
    memcpy( pVertices, vertices, sizeof( vertices ) );
    g_pVB->Unlock();
}


// this is the function used to render a single frame
void render_frame(void)
{
	D3DVIEWPORT9 vp;
    vp.Width = width;
    vp.Height = height;
    vp.MaxZ = 1.0f;

    d3ddev->BeginScene();    // begins the 3D scene

    // clear the window to a deep blue
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

    // do 3D rendering on the back buffer here

	d3ddev->SetStreamSource( 0, g_pVB, 0, sizeof( CUSTOMVERTEX ) );
    d3ddev->SetFVF( D3DFVF_CUSTOMVERTEX );
    d3ddev->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1 );

    d3ddev->EndScene();    // ends the 3D scene

    d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	DestroyWyphonPartner(hWyphonPartner);

	d3ddev->Release();    // close and release the 3D device
    d3d->Release();    // close and release Direct3D
}