// include the basic windows header files and the Direct3D header file
#include "stdafx.h"
#include "DXWyphonSender.h"

using namespace Wyphon;
using namespace WyphonUtils;

// global declarations
LPDIRECT3D9EX d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9EX d3ddev;    // the pointer to the device class
IDirect3DTexture9* pD3D9Texture;
IDirect3DSurface9 *pD3D9Surface, *pBackBuffer;
HANDLE DXShareHandle;
int width, height;
LPDIRECT3DVERTEXBUFFER9 vbTriangle = NULL;
LPDIRECT3DVERTEXBUFFER9 vbFullscreenQuad = NULL;

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z; // The transformed position for the vertex
    DWORD color;        // The vertex color
};

DWORD startupTime;

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

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

	width = 1920;
	height = 1080;

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
                          800, 600,
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
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferCount = 1;


    // create a device class using this information and the info from the d3dpp stuct
    d3d->CreateDeviceEx(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_HARDWARE_VERTEXPROCESSING,
                      &d3dpp,
					  NULL,
                      &d3ddev);
	HRESULT res = d3ddev->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT) D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pD3D9Texture, &DXShareHandle);
	pD3D9Texture->GetSurfaceLevel(0, &pD3D9Surface);
	d3ddev->GetRenderTarget(0,&pBackBuffer);

	lstrcpyW( WyphonApplicationNameT, _T("myApplication") );
	lstrcpyW( WyphonTextureDescriptionT, _T("myTexture") );

	hWyphonPartner = CreateWyphonPartner( WyphonApplicationNameT,NULL,NULL,NULL,NULL,NULL /* no callbacks */ );

	BOOL bRes = ShareD3DTexture(hWyphonPartner, DXShareHandle, width, height, D3DFMT_A8R8G8B8, D3DUSAGE_RENDERTARGET, WyphonTextureDescriptionT);
	if ( !bRes ) {
		return;
	}

	CUSTOMVERTEX triangle[] = {
        { 2.5f, -3.0f, 0.0f, D3DCOLOR_ARGB(70, 255, 0, 255), }, // x, y, z, rhw, color
        { 0.0f, 3.0f, 0.0f, D3DCOLOR_ARGB(70, 255, 255, 0), },
        { -2.5f, -3.0f, 0.0f, D3DCOLOR_ARGB(70, 0, 0, 255), },
    };
	CUSTOMVERTEX fullscreenQuad[] = {
        { 0.0f,  0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), }, // x, y, z, rhw, color
        { 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), },
        { 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), },
        { 1.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), },
    };

    d3ddev->CreateVertexBuffer( 3 * sizeof( CUSTOMVERTEX ),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &vbTriangle, NULL );
    d3ddev->CreateVertexBuffer( 4 * sizeof( CUSTOMVERTEX ),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &vbFullscreenQuad, NULL );

    // Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the vertices. This mechanism is required becuase vertex
    // buffers may be in device memory.
    VOID *pTriangle, *pFullscreenQuad;
    vbTriangle->Lock( 0, sizeof( triangle ), ( void** )&pTriangle, 0 );
    memcpy( pTriangle, triangle, sizeof( triangle ) );
    vbTriangle->Unlock();

    vbFullscreenQuad->Lock( 0, sizeof( fullscreenQuad ), ( void** )&pFullscreenQuad, 0 );
    memcpy( pFullscreenQuad, fullscreenQuad, sizeof( fullscreenQuad ) );
    vbFullscreenQuad->Unlock();

	d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);    // turn off the 3D lighting
	d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	startupTime = timeGetTime();
}


// this is the function used to render a single frame
void render_frame(void)
{
	d3ddev->SetRenderTarget(0,pD3D9Surface);
	d3ddev->BeginScene();    // begins the 3D scene

		// clear the window to a deep blue
		d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

		d3ddev->SetTexture(0,NULL);
		d3ddev->SetFVF( D3DFVF_CUSTOMVERTEX );
		d3ddev->SetStreamSource( 0, vbTriangle, 0, sizeof( CUSTOMVERTEX ) );

		D3DXMATRIX matView;    // the view transform matrix
		D3DXMatrixLookAtLH(&matView,
					   &D3DXVECTOR3 (0.0f, 0.0f, 10.0f),    // the camera position
					   &D3DXVECTOR3 (0.0f, 0.0f, 0.0f),    // the look-at position
					   &D3DXVECTOR3 (0.0f, 1.0f, 0.0f));    // the up direction
		d3ddev->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

		D3DXMATRIX matProjection;    // the projection transform matrix
		D3DXMatrixPerspectiveFovLH(&matProjection,
							   D3DXToRadian(45),    // the horizontal field of view
							   (FLOAT)width / (FLOAT)height,    // aspect ratio
							   1.0f,    // the near view-plane
							   100.0f);    // the far view-plane
		d3ddev->SetTransform(D3DTS_PROJECTION, &matProjection);    // set the projection transform

		D3DXMATRIX matTranslate, matRotate, matScale;
		DWORD timeDiff = timeGetTime() - startupTime;
		D3DXMatrixRotationZ(&matRotate, 3.14 * float(timeDiff) / 10000);
		D3DXMatrixScaling(&matScale, 0.2f, 0.2f, 1.0f );
		// setting this to 0.1 / 0.5 creates first flickering ("preferably" in fullscreen mode)
		for ( float y = -10; y < 10; y+=0.1f ) {
			for ( float x = -10; x < 10; x+=0.2f ) {
				D3DXMatrixTranslation(&matTranslate, x, y, 0);
				d3ddev->SetTransform(D3DTS_WORLD, &(matScale*matRotate*matTranslate));
				d3ddev->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1 );
			}
		}

    d3ddev->EndScene();


	// render to screen
	d3ddev->SetRenderTarget(0,pBackBuffer);
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
	d3ddev->BeginScene();
		d3ddev->SetTexture(0,pD3D9Texture);
		d3ddev->SetFVF( D3DFVF_CUSTOMVERTEX );
		d3ddev->SetStreamSource(0,vbFullscreenQuad,0,sizeof(CUSTOMVERTEX));
		d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);
	d3ddev->EndScene();

    d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	DestroyWyphonPartner(hWyphonPartner);
	pBackBuffer->Release();
	pD3D9Surface->Release();
	vbTriangle->Release();
	vbFullscreenQuad->Release();
	d3ddev->Release();    // close and release the 3D device
    d3d->Release();    // close and release Direct3D
}