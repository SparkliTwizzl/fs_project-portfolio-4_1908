// ---------- INCLUDES ----------
//#include "stdafx.h"
#include "Project.h"

#include <iostream>

// DirectX
#include <d3d11.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")
#include "DDSTextureLoader.h"

#include "OBJDataLoader.h"

// shaders
#include "VS.csh"
#include "VS_Distort.csh"
#include "GeometryShaderDefault.csh"
#include "GeometryShaderDistort.csh"
#include "PS.csh"
#include "PS_CubeMap.csh"
#include "PS_Distort.csh"
#include "PS_InputColor.csh"
#include "PS_InputColorLights.csh"
#include "PS_SolidColor.csh"
#include "PS_SolidColorLights.csh"

using namespace DirectX;
// ---------- INCLUDES ----------

// ---------- MACROS ----------
#define MAX_LOADSTRING 100

#define MAX_INSTANCES 5
#define MAX_DIRECTIONAL_LIGHTS 3
#define MAX_POINT_LIGHTS 3
#define MAX_SPOT_LIGHTS 3

#define DEGREES_TO_RADIANS(degrees) (degrees * (XM_PI / 180.0f))
// ---------- MACROS ----------

/* KEY
g_	: global
p_	: pointer
pp_	: double pointer
S_	: struct
*/

// ---------- STRUCTS ----------
struct Vertex
{
	XMFLOAT4 position;
	XMFLOAT3 normal;
	XMFLOAT3 texel;
	XMFLOAT4 color;
};
struct DirectionalLight
{
	XMFLOAT4 direction;
	XMFLOAT4 color;
};
struct PointLight
{
	XMFLOAT4 position;
	FLOAT range;
	XMFLOAT3 attenuation;
	XMFLOAT4 color;
};
struct SpotLight
{
	XMFLOAT4 position;
	XMFLOAT4 direction;
	FLOAT range;
	FLOAT cone;
	XMFLOAT3 attenuation;
	XMFLOAT4 color;
};
struct VertexShaderConstantBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMMATRIX instanceOffsets[MAX_INSTANCES];
	FLOAT time;
	XMFLOAT3 padding;
};
struct PixelShaderConstantBuffer
{
	XMFLOAT4 ambientColor;
	XMFLOAT4 instanceColors[MAX_INSTANCES];
	DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight pointLights[MAX_POINT_LIGHTS];
	//SpotLight spotLights[MAX_SPOT_LIGHTS];
	FLOAT time;
	XMFLOAT3 padding;
};
// ---------- STRUCTS ----------

// ---------- GLOBAL VARS ----------
// ----- WIN32 VARS -----
HINSTANCE HInstance = nullptr; // current instance
HWND HWindow = nullptr; // the window
WCHAR TitleBarText[MAX_LOADSTRING]; // the title bar text
WCHAR MainWindowClassName[MAX_LOADSTRING]; // the main window class name
// ----- WIN32 VARS -----

// ----- D3D VARS -----
D3D_DRIVER_TYPE DXDriverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL DXFeatureLevel = D3D_FEATURE_LEVEL_11_0;
// --- DEVICE / SWAP CHAIN ---
ID3D11Device* DXDevice = nullptr;
IDXGISwapChain* DXSwapChain = nullptr;
ID3D11DeviceContext* DXDeviceContext
= nullptr;
// --- DEVICE / SWAP CHAIN ---
// --- RENDER TARGET VIEWS ---
ID3D11RenderTargetView* MainDXRenderTargetView = nullptr;
ID3D11RenderTargetView* RenderToTextureDXRenderTargetView = nullptr;
// --- RENDER TARGET VIEWS ---
// --- DEPTH STENCILS ---
ID3D11Texture2D*			MainDXDepthStencil = nullptr;
ID3D11DepthStencilView*		MainDXDepthStencilView = nullptr;
ID3D11Texture2D*			g_p_depthStencil_RTT = nullptr;
ID3D11DepthStencilView*		g_p_depthStencilView_RTT = nullptr;
// --- DEPTH STENCILS ---
// --- VIEWPORTS ---
D3D11_VIEWPORT				g_viewport0;
D3D11_VIEWPORT				g_viewport1;
// --- VIEWPORTS ---
// --- INPUT LAYOUT ---
ID3D11InputLayout*			g_p_vertexLayout = nullptr;
// --- INPUT LAYOUT ---
// --- VERT / IND BUFFERS ---
// SKYBOX
ID3D11Buffer*				g_p_vBuffer_Skybox = nullptr;
ID3D11Buffer*				g_p_iBuffer_Skybox = nullptr;
UINT						g_numVerts_Skybox = 0;
UINT						g_numInds_Skybox = 0;
// CUBE
ID3D11Buffer*				g_p_vBuffer_Cube = nullptr;
ID3D11Buffer*				g_p_iBuffer_Cube = nullptr;
UINT						g_numVerts_Cube = 0;
UINT						g_numInds_Cube = 0;
// GROUND PLANE
ID3D11Buffer*				g_p_vBuffer_GroundPlane = nullptr;
ID3D11Buffer*				g_p_iBuffer_GroundPlane = nullptr;
UINT						g_numVerts_GroundPlane = 0;
UINT						g_numInds_GroundPlane = 0;
UINT						g_numDivisions_GroundPlane = 100;
FLOAT						g_scale_GroundPlane = 10.0f;
// BRAZIER01
ID3D11Buffer*				g_p_vBuffer_Brazier01 = nullptr;
ID3D11Buffer*				g_p_iBuffer_Brazier01 = nullptr;
UINT						g_numVerts_Brazier01 = 0;
UINT						g_numInds_Brazier01 = 0;
// SPACESHIP
ID3D11Buffer*				g_p_vBuffer_Spaceship = nullptr;
ID3D11Buffer*				g_p_iBuffer_Spaceship = nullptr;
UINT						g_numVerts_Spaceship = 0;
UINT						g_numInds_Spaceship = 0;
// PLANET
ID3D11Buffer*				g_p_vBuffer_Planet = nullptr;
ID3D11Buffer*				g_p_iBuffer_Planet = nullptr;
UINT						g_numVerts_Planet = 0;
UINT						g_numInds_Planet = 0;
// --- VERT / IND BUFFERS ---
// --- CONSTANT BUFFERS ---
ID3D11Buffer*				g_p_cBufferVS = nullptr;
ID3D11Buffer*				g_p_cBufferPS = nullptr;
// --- CONSTANT BUFFERS ---
// --- TEXTURES / SHADER RESOURCE VIEWS ---
ID3D11ShaderResourceView*	g_p_SRV_Skybox = nullptr;
ID3D11ShaderResourceView*	g_p_SRV_Brazier01 = nullptr;
ID3D11ShaderResourceView*	g_p_SRV_Spaceship = nullptr;
ID3D11ShaderResourceView*	g_p_SRV_Sun = nullptr;
ID3D11ShaderResourceView*	g_p_SRV_Earth = nullptr;
ID3D11ShaderResourceView*	g_p_SRV_Moon = nullptr;
ID3D11ShaderResourceView*	g_p_SRV_Mars = nullptr;
ID3D11ShaderResourceView*	g_p_SRV_Jupiter = nullptr;
ID3D11Texture2D*			g_p_tex_RTT = nullptr;
ID3D11ShaderResourceView*	g_p_SRV_RTT = nullptr;
// --- TEXTURES / SHADER RESOURCE VIEWS ---
// --- SAMPLER STATES ---
ID3D11SamplerState*			g_p_samplerLinear = nullptr;
// --- SAMPLER STATES ---
// --- SHADERS ---
// VERTEX
ID3D11VertexShader*			g_p_VS = nullptr;
ID3D11VertexShader*			g_p_VS_Distort = nullptr;
// GEOMETRY
ID3D11GeometryShader*		g_p_GS = nullptr;
ID3D11GeometryShader*		g_p_GS_Distort = nullptr;
// PIXEL
ID3D11PixelShader*			g_p_PS = nullptr;
ID3D11PixelShader*			g_p_PS_CubeMap = nullptr;
ID3D11PixelShader*			g_p_PS_Distort = nullptr;
ID3D11PixelShader*			g_p_PS_InputColor = nullptr;
ID3D11PixelShader*			g_p_PS_InputColorLights = nullptr;
ID3D11PixelShader*			g_p_PS_SolidColor = nullptr;
ID3D11PixelShader*			g_p_PS_SolidColorLights = nullptr;
// --- SHADERS ---
// ----- D3D vars -----

// ----- MATRICES -----
XMFLOAT4X4					g_wrld;
XMFLOAT4X4					g_view;
XMFLOAT4X4					g_proj;
XMFLOAT4X4					g_proj_RTT;
XMFLOAT4X4					g_wrld_Skybox;
XMFLOAT4X4					g_wrld_Cube;
XMFLOAT4X4					g_wrld_GroundPlane;
XMFLOAT4X4					g_wrld_Brazier01;
XMFLOAT4X4					g_wrld_Spaceship;
XMFLOAT4X4					g_wrld_Sun;
XMFLOAT4X4					g_wrld_Earth;
XMFLOAT4X4					g_wrld_Moon;
XMFLOAT4X4					g_wrld_Mars;
XMFLOAT4X4					g_wrld_Jupiter;
// ----- MATRICES -----

// ----- CAMERA VALUES -----
FLOAT						g_camMoveSpeed = 4.0f;		// units per second
FLOAT						g_camRotSpeed = 40.0f;		// degrees per second
FLOAT						g_camZoomSpeed = 0.01f;		// zoom level per second
FLOAT						g_camZoom = 1.0f;			// current zoom level
const FLOAT					g_camZoomMin = 0.5f;
const FLOAT					g_camZoomMax = 2.0f;
FLOAT						g_camNearSpeed = 1.0f;		// near plane change per second
FLOAT						g_camNearPlane = 0.01f;		// current near plane
const FLOAT					g_camNearMin = 0.01f;
const FLOAT					g_camNearMax = 9.0f;
FLOAT						g_camFarSpeed = 10.0f;		// far plane change per second
FLOAT						g_camFarPlane = 100.0f;		// current far plane
const FLOAT					g_camFarMin = 10.0f;
const FLOAT					g_camFarMax = 100.0f;
// ----- CAMERA VALUES -----

// ----- TOGGLES -----
bool						g_freelook = true;
bool						g_defaultVS = true;
bool						g_defaultGS = true;
bool						g_defaultPS = true;
// ----- TOGGLES -----
// ---------- GLOBAL VARS ----------

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				CreateProceduralGrid(Vertex, UINT, FLOAT, Vertex**, UINT&, UINT**, UINT&);
void				ProcessOBJData(const char*, Vertex**, UINT&, UINT**, UINT&);
HRESULT				InitDepthStencilView(UINT, UINT, ID3D11Texture2D**, ID3D11DepthStencilView**);
HRESULT				InitVertexBuffer(UINT, Vertex**, ID3D11Buffer**);
HRESULT				InitIndexBuffer(UINT, UINT**, ID3D11Buffer**);
HRESULT				InitConstantBuffer(UINT, ID3D11Buffer**);
HRESULT				InitSamplerState(ID3D11SamplerState**);
void				Render();
void				Cleanup();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, TitleBarText, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PROJECT, MainWindowClassName, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT));

	MSG msg;

	// Main message loop:
	while (true) //(GetMessage(&msg, nullptr, 0, 0))
	{
		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			break;

		Render();
	}

	Cleanup();

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PROJECT);
	wcex.lpszClassName = MainWindowClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HInstance = hInstance; // Store instance handle in our global variable

	HWindow = CreateWindowW(MainWindowClassName, TitleBarText, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	// XOR with thickframe prevents click & drag resize, XOR with maximizebox prevents control button resize

	if (!HWindow)
	{
		return FALSE;
	}

	ShowWindow(HWindow, nCmdShow);
	UpdateWindow(HWindow);

	HRESULT hr;

	// get window dimensions
	RECT windowRect;
	GetClientRect(HWindow, &windowRect);
	UINT windowWidth = windowRect.right - windowRect.left;
	UINT windowHeight = windowRect.bottom - windowRect.top;

	// --------------------------------------------------
	// ATTACH D3D TO WINDOW

	// ---------- D3D DEVICE AND SWAP CHAIN ----------
	// ----- SWAP CHAIN DESCRIPTOR -----
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1; // number of buffers in swap chain
	swapChainDesc.OutputWindow = HWindow;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // pixel format
	swapChainDesc.BufferDesc.Width = windowWidth;
	swapChainDesc.BufferDesc.Height = windowHeight;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // buffer usage flag; specifies what swap chain's buffer will be used for
	swapChainDesc.SampleDesc.Count = 1; // samples per pixel while drawing
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// ----- SWAP CHAIN DESCRIPTOR -----
	// create device and swap chain
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		createDeviceFlags, &DXFeatureLevel, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &DXSwapChain, &DXDevice, 0, &DXDeviceContext);
	// ---------- D3D DEVICE AND SWAP CHAIN ----------

	// ---------- SHADER RESOURCE VIEWS ----------
	// --- RTT ---
	// texture
	D3D11_TEXTURE2D_DESC texDesc_RTT = {};
	texDesc_RTT.Width = windowWidth;
	texDesc_RTT.Height = windowHeight;
	texDesc_RTT.MipLevels = 1;
	texDesc_RTT.ArraySize = 1;
	texDesc_RTT.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc_RTT.SampleDesc.Count = 1;
	texDesc_RTT.Usage = D3D11_USAGE_DEFAULT;
	texDesc_RTT.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc_RTT.CPUAccessFlags = 0;
	texDesc_RTT.MiscFlags = 0;
	hr = DXDevice->CreateTexture2D(&texDesc_RTT, NULL, &g_p_tex_RTT);
	// shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc_RTT;
	SRVDesc_RTT.Format = texDesc_RTT.Format;
	SRVDesc_RTT.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc_RTT.Texture2D.MostDetailedMip = 0;
	SRVDesc_RTT.Texture2D.MipLevels = 1;
	hr = DXDevice->CreateShaderResourceView(g_p_tex_RTT, &SRVDesc_RTT, &g_p_SRV_RTT);
	// --- RTT ---

	// skybox
	hr = CreateDDSTextureFromFile(DXDevice, L"Assets/skybox.dds", nullptr, &g_p_SRV_Skybox);

	// mesh textures
	hr = CreateDDSTextureFromFile(DXDevice, L"Assets/Brazier01map.dds", nullptr, &g_p_SRV_Brazier01);
	hr = CreateDDSTextureFromFile(DXDevice, L"Assets/spaceshipmap.dds", nullptr, &g_p_SRV_Spaceship);
	hr = CreateDDSTextureFromFile(DXDevice, L"Assets/sunmap.dds", nullptr, &g_p_SRV_Sun);
	hr = CreateDDSTextureFromFile(DXDevice, L"Assets/earthmap.dds", nullptr, &g_p_SRV_Earth);
	hr = CreateDDSTextureFromFile(DXDevice, L"Assets/moonmap.dds", nullptr, &g_p_SRV_Moon);
	hr = CreateDDSTextureFromFile(DXDevice, L"Assets/marsmap.dds", nullptr, &g_p_SRV_Mars);
	hr = CreateDDSTextureFromFile(DXDevice, L"Assets/jupitermap.dds", nullptr, &g_p_SRV_Jupiter);
	// ---------- SHADER RESOURCE VIEWS ----------

	// ---------- SAMPLER STATES ----------
	hr = InitSamplerState(&g_p_samplerLinear);
	// ---------- SAMPLER STATES ----------

	// ---------- RENDER TARGET VIEWS ----------
	// --- MAIN ---
	// get back buffer from swap chain
	ID3D11Resource* p_backBuffer = nullptr;
	hr = DXSwapChain->GetBuffer(0, __uuidof(p_backBuffer), (void**)&p_backBuffer);
	// use back buffer to create render target view
	hr = DXDevice->CreateRenderTargetView(p_backBuffer, nullptr, &MainDXRenderTargetView);
	// release back buffer
	p_backBuffer->Release();
	// --- MAIN ---
	// --- RTT ---
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc_RTT;
	RTVDesc_RTT.Format = texDesc_RTT.Format;
	RTVDesc_RTT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc_RTT.Texture2D.MipSlice = 0;
	hr = DXDevice->CreateRenderTargetView(g_p_tex_RTT, &RTVDesc_RTT, &RenderToTextureDXRenderTargetView);
	// --- RTT ---
	// ---------- RENDER TARGET VIEWS ----------

	// ---------- DEPTH STENCILS ----------
	hr = InitDepthStencilView(windowWidth, windowHeight, &MainDXDepthStencil, &MainDXDepthStencilView);
	hr = InitDepthStencilView(windowWidth, windowHeight, &g_p_depthStencil_RTT, &g_p_depthStencilView_RTT);
	// ---------- DEPTH STENCILS ----------

	// ---------- VIEWPORTS ----------
	// setup main viewport
	g_viewport0.Width = (FLOAT)windowWidth;
	g_viewport0.Height = (FLOAT)windowHeight;
	g_viewport0.TopLeftX = 0;
	g_viewport0.TopLeftX = 0;
	g_viewport0.MinDepth = 0.0f; // exponential depth; near/far planes are handled in projection matrix
	g_viewport0.MaxDepth = 1.0f;

	// setup minimap viewport
	g_viewport1.Width = (FLOAT)windowWidth / 4.0f;
	g_viewport1.Height = (FLOAT)windowHeight / 4.0f;
	g_viewport1.TopLeftX = 0;
	g_viewport1.TopLeftX = 0;
	g_viewport1.MinDepth = 0.0f; // exponential depth; near/far planes are handled in projection matrix
	g_viewport1.MaxDepth = 1.0f;
	// ---------- VIEWPORTS ----------

	// ---------- SHADERS ----------
	// vertex
	hr = DXDevice->CreateVertexShader(VS_Distort, sizeof(VS_Distort), nullptr, &g_p_VS_Distort);
	hr = DXDevice->CreateVertexShader(VS, sizeof(VS), nullptr, &g_p_VS);
	// geometry
	hr = DXDevice->CreateGeometryShader(GeometryShaderDefault, sizeof(GeometryShaderDefault), nullptr, &g_p_GS);
	hr = DXDevice->CreateGeometryShader(GeometryShaderDistort, sizeof(GeometryShaderDistort), nullptr, &g_p_GS_Distort);
	// pixel
	hr = DXDevice->CreatePixelShader(PS, sizeof(PS), nullptr, &g_p_PS);
	hr = DXDevice->CreatePixelShader(PS_CubeMap, sizeof(PS_CubeMap), nullptr, &g_p_PS_CubeMap);
	hr = DXDevice->CreatePixelShader(PS_Distort, sizeof(PS_Distort), nullptr, &g_p_PS_Distort);
	hr = DXDevice->CreatePixelShader(PS_InputColor, sizeof(PS_InputColor), nullptr, &g_p_PS_InputColor);
	hr = DXDevice->CreatePixelShader(PS_InputColorLights, sizeof(PS_InputColorLights), nullptr, &g_p_PS_InputColorLights);
	hr = DXDevice->CreatePixelShader(PS_SolidColor, sizeof(PS_SolidColor), nullptr, &g_p_PS_SolidColor);
	hr = DXDevice->CreatePixelShader(PS_SolidColorLights, sizeof(PS_SolidColorLights), nullptr, &g_p_PS_SolidColorLights);
	// ---------- SHADERS ----------

	// ---------- INPUT LAYOUT ----------
	// input layout data
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numInputElements = ARRAYSIZE(inputElementDesc);
	// create input layout
	hr = DXDevice->CreateInputLayout(inputElementDesc, numInputElements, VS, sizeof(VS), &g_p_vertexLayout);
	// set input layout
	DXDeviceContext->IASetInputLayout(g_p_vertexLayout);
	// ---------- INPUT LAYOUT ----------

	// ---------- MESHES ----------
	// ----- SKYBOX -----
	// load vertex / index data
	Vertex* p_verts_Skybox = nullptr;
	UINT* p_inds_Skybox = nullptr;
	ProcessOBJData("Assets/skybox.obj", &p_verts_Skybox, g_numVerts_Skybox, &p_inds_Skybox, g_numInds_Skybox);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Skybox, &p_verts_Skybox, &g_p_vBuffer_Skybox);
	hr = InitIndexBuffer(g_numInds_Skybox, &p_inds_Skybox, &g_p_iBuffer_Skybox);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_Skybox, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Skybox;
	delete[] p_inds_Skybox;
	// ----- SKYBOX -----

	// ----- CUBE -----
	// load vertex / index data
	Vertex* p_verts_Cube = nullptr;
	UINT* p_inds_Cube = nullptr;
	ProcessOBJData("Assets/cube.obj", &p_verts_Cube, g_numVerts_Cube, &p_inds_Cube, g_numInds_Cube);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Cube, (Vertex**)&p_verts_Cube, &g_p_vBuffer_Cube);
	hr = InitIndexBuffer(g_numInds_Cube, (UINT**)&p_inds_Cube, &g_p_iBuffer_Cube);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_Cube, XMMatrixIdentity());
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	// generate vertex / index data
	Vertex gridOrigin = { { 0, 0, 0, 1 }, { 0, 1, 0 }, { 0, 0, 0 }, {} };
	Vertex* p_verts_GroundPlane = nullptr;
	UINT* p_inds_GroundPlane = nullptr;
	CreateProceduralGrid(gridOrigin, g_numDivisions_GroundPlane, g_scale_GroundPlane,
		&p_verts_GroundPlane, g_numVerts_GroundPlane, &p_inds_GroundPlane, g_numInds_GroundPlane);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_GroundPlane, &p_verts_GroundPlane, &g_p_vBuffer_GroundPlane);
	hr = InitIndexBuffer(g_numInds_GroundPlane, &p_inds_GroundPlane, &g_p_iBuffer_GroundPlane);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_GroundPlane, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_GroundPlane;
	delete[] p_inds_GroundPlane;
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	// load vertex / index data
	Vertex* p_verts_Brazier01 = nullptr;
	UINT* p_inds_Brazier01 = nullptr;
	ProcessOBJData("Assets/Brazier01.obj", &p_verts_Brazier01, g_numVerts_Brazier01,
		&p_inds_Brazier01, g_numInds_Brazier01);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Brazier01, &p_verts_Brazier01, &g_p_vBuffer_Brazier01);
	hr = InitIndexBuffer(g_numInds_Brazier01, &p_inds_Brazier01, &g_p_iBuffer_Brazier01);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_Brazier01, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Brazier01;
	delete[] p_inds_Brazier01;
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	// load vertex / index data
	Vertex* p_verts_Spaceship = nullptr;
	UINT* p_inds_Spaceship = nullptr;
	ProcessOBJData("Assets/spaceship.obj", &p_verts_Spaceship, g_numVerts_Spaceship,
		&p_inds_Spaceship, g_numInds_Spaceship);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Spaceship, &p_verts_Spaceship, &g_p_vBuffer_Spaceship);
	hr = InitIndexBuffer(g_numInds_Spaceship, &p_inds_Spaceship, &g_p_iBuffer_Spaceship);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_Spaceship, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Spaceship;
	delete[] p_inds_Spaceship;
	// ----- SPACESHIP -----

	// ----- PLANET -----
	// load vertex / index data
	Vertex* p_verts_Planet = nullptr;
	UINT* p_inds_Planet = nullptr;
	ProcessOBJData("Assets/planet.obj", &p_verts_Planet, g_numVerts_Planet,
		&p_inds_Planet, g_numInds_Planet);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Planet, &p_verts_Planet, &g_p_vBuffer_Planet);
	hr = InitIndexBuffer(g_numInds_Planet, &p_inds_Planet, &g_p_iBuffer_Planet);
	// set initial world matrices
	XMStoreFloat4x4(&g_wrld_Sun, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_Earth, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_Moon, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_Mars, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_Jupiter, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Planet;
	delete[] p_inds_Planet;
	// ----- PLANET -----
	// ---------- MESHES ----------

	// set type of topology to draw
	DXDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ---------- CONSTANT BUFFERS ----------
	hr = InitConstantBuffer(sizeof(VertexShaderConstantBuffer), &g_p_cBufferVS);
	hr = InitConstantBuffer(sizeof(PixelShaderConstantBuffer), &g_p_cBufferPS);
	// ---------- CONSTANT BUFFERS ----------

	// ---------- MATRICES ----------
	// world
	XMStoreFloat4x4(&g_wrld, XMMatrixIdentity());

	// view
	XMVECTOR eye = XMVectorSet(0, 10, -15, 0);
	XMVECTOR at = XMVectorSet(0, 2, 0, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
	XMVECTOR determinant = XMMatrixDeterminant(view);
	XMStoreFloat4x4(&g_view, XMMatrixInverse(&determinant, view));

	// projection
	// main
	XMStoreFloat4x4(&g_proj, XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
	// RTT
	XMStoreFloat4x4(&g_proj_RTT, XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
	// ---------- MATRICES ----------

	// ATTACH D3D TO WINDOW
	// --------------------------------------------------

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(HInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	//case WM_PAINT:
	//    {
	//        PAINTSTRUCT ps;
	//        HDC hdc = BeginPaint(hWnd, &ps);
	//        // TODO: Add any drawing code that uses hdc here...
	//        EndPaint(hWnd, &ps);
	//    }
	//    break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void CreateProceduralGrid(Vertex _origin, UINT _numDivisions, FLOAT _scale,
	Vertex** _pp_verts, UINT& _numVerts, UINT** _pp_inds, UINT& _numInds)
{
	// calculate number of vertices / indices
	_numVerts = _numDivisions * _numDivisions;
	_numInds = 6 * (_numDivisions - 1) * (_numDivisions - 1);
	// set vertex data
	Vertex* p_verts = new Vertex[_numVerts];
	for (UINT z = 0; z < _numDivisions; z++)
		for (UINT x = 0; x < _numDivisions; x++)
		{
			UINT index = x + (z * _numDivisions);
			assert(index < _numVerts);
			// calculate offset amount
			FLOAT offsetX = (_scale * -0.5f) + (_scale / (_numDivisions - 1)) * x;
			FLOAT offsetZ = (_scale * -0.5f) + (_scale / (_numDivisions - 1)) * z;
			// offset position
			p_verts[index].position = _origin.position;
			p_verts[index].position.x += offsetX;
			p_verts[index].position.z += offsetZ;
			// copy normal
			p_verts[index].normal = { 0, 1, 0 };
			// offset tex coord
			p_verts[index].texel = _origin.texel;
			p_verts[index].texel.x += offsetX;
			p_verts[index].texel.y += offsetZ;
			// randomize color
			p_verts[index].color = {};
			p_verts[index].color.x = (rand() % 1000) / 1000.0f;
			p_verts[index].color.y = (rand() % 1000) / 1000.0f;
			p_verts[index].color.z = (rand() % 1000) / 1000.0f;
			p_verts[index].color.w = 1;
		}
	*_pp_verts = p_verts;
	// set indices
	UINT* p_inds = new UINT[_numInds];
	for (UINT z = 0; z < _numDivisions - 1; z++)
		for (UINT x = 0; x < _numDivisions - 1; x++)
		{
			UINT vertIndex = x + (z * _numDivisions);
			assert(vertIndex < _numVerts);
			UINT index = 6 * (x + (z * (_numDivisions - 1)));
			assert(index < _numInds);
			p_inds[index + 0] = vertIndex;
			p_inds[index + 1] = vertIndex + _numDivisions + 1;
			p_inds[index + 2] = vertIndex + 1;
			p_inds[index + 3] = vertIndex;
			p_inds[index + 4] = vertIndex + _numDivisions;
			p_inds[index + 5] = vertIndex + _numDivisions + 1;

			//_RPTN(0, "%d, %d, %d,\n%d, %d, %d\n\n", p_inds[index + 0], p_inds[index + 1], p_inds[index + 2], p_inds[index + 3], p_inds[index + 4], p_inds[index + 5]);
		}
	*_pp_inds = p_inds;
}

void ProcessOBJData(const char* _filepath, Vertex** _pp_verts, UINT& vertexCount, UINT** _pp_inds, UINT& indexCount)
{
	OBJMesh RawMeshData = LoadOBJMesh(_filepath);
	vertexCount = RawMeshData.VertexCount;
	indexCount = RawMeshData.IndexCount;
	// copy vertex data
	Vertex* vertices = new Vertex[vertexCount];
	for (UINT i = 0; i < vertexCount; i++)
	{
		// copy position
		vertices[i].position.x = RawMeshData.Vertices[i].Position[0];
		vertices[i].position.y = RawMeshData.Vertices[i].Position[1];
		vertices[i].position.z = RawMeshData.Vertices[i].Position[2];
		vertices[i].position.w = 1;
		// copy normal
		vertices[i].normal.x = RawMeshData.Vertices[i].Normal[0];
		vertices[i].normal.y = RawMeshData.Vertices[i].Normal[1];
		vertices[i].normal.z = RawMeshData.Vertices[i].Normal[2];
		// copy texcoord
		vertices[i].texel.x = RawMeshData.Vertices[i].Texel[0];
		vertices[i].texel.y = RawMeshData.Vertices[i].Texel[1];
		vertices[i].texel.z = RawMeshData.Vertices[i].Texel[2];
		// set color
		vertices[i].color = { 1, 1, 1, 1 };
	}
	*_pp_verts = vertices;
	// copy index data
	UINT* indices = new UINT[indexCount];
	for (UINT i = 0; i < indexCount; i++)
	{
		indices[i] = RawMeshData.Indices[i];
	}
	*_pp_inds = indices;
}

HRESULT InitDepthStencilView(UINT _width, UINT _height, ID3D11Texture2D** _pp_depthStencil,
	ID3D11DepthStencilView** _pp_depthStencilView)
{
	HRESULT hr;
	// create depth stencil texture
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = _width;
	depthStencilDesc.Height = _height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	hr = DXDevice->CreateTexture2D(&depthStencilDesc, nullptr, _pp_depthStencil);
	if (FAILED(hr))
		return hr;

	// create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	return DXDevice->CreateDepthStencilView(*_pp_depthStencil, &depthStencilViewDesc, _pp_depthStencilView);
}

HRESULT InitVertexBuffer(UINT _numVerts, Vertex** _pp_verts, ID3D11Buffer** _pp_vBuffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(Vertex) * _numVerts;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = *_pp_verts;
	return DXDevice->CreateBuffer(&bufferDesc, &subData, _pp_vBuffer);
}

HRESULT InitIndexBuffer(UINT _numInds, UINT** _pp_inds, ID3D11Buffer** _pp_iBuffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(UINT) * _numInds;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = *_pp_inds;
	return DXDevice->CreateBuffer(&bufferDesc, &subData, _pp_iBuffer);
}

HRESULT InitConstantBuffer(UINT _bufferSize, ID3D11Buffer** _pp_cBuffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = _bufferSize;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	return DXDevice->CreateBuffer(&bufferDesc, nullptr, _pp_cBuffer);
}

HRESULT InitSamplerState(ID3D11SamplerState** _pp_samplerState)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	return DXDevice->CreateSamplerState(&samplerDesc, _pp_samplerState);
}

void Render()
{
	// --------------------------------------------------
	// UPDATES / DRAW SETUP

	// ----- UPDATE TIME -----
	static ULONGLONG timeStart = 0;
	static ULONGLONG timePrev = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
		timeStart = timeCur;
	float t = (timeCur - timeStart) / 1000.0f;
	float dt = (timeCur - timePrev) / 1000.0f;
	timePrev = timeCur;
	// ----- UPDATE TIME -----

	// ----- GET WINDOW DIMENSIONS -----
	RECT windowRect;
	GetClientRect(HWindow, &windowRect);
	UINT windowWidth = windowRect.right - windowRect.left;
	UINT windowHeight = windowRect.bottom - windowRect.top;
	// ----- GET WINDOW DIMENSIONS -----

	// ----- SET BYTE STRIDES / OFFSETS -----
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	// ----- SET BYTE STRIDES / OFFSETS -----

	// ----- CREATE CONSTANT BUFFER STRUCT INSTANCES -----
	VertexShaderConstantBuffer cBufferVS = {};
	PixelShaderConstantBuffer cBufferPS = {};
	// ----- CREATE CONSTANT BUFFER STRUCT INSTANCES -----

	// ----- GENERAL PURPOSE VARS -----
	// color to clear render targets to
	FLOAT clearColor[4] = { 0, 0, 0.25f, 1 };
	// matrices
	XMMATRIX translate = XMMatrixIdentity();
	XMMATRIX rotate = XMMatrixIdentity();
	XMMATRIX scale = XMMatrixIdentity();
	// ----- GENERAL PURPOSE VARS -----

	// ----- RETRIEVE MATRICES -----
	XMMATRIX wrld = XMLoadFloat4x4(&g_wrld);
	XMMATRIX view = XMLoadFloat4x4(&g_view);
	XMMATRIX proj = XMLoadFloat4x4(&g_proj);
	XMMATRIX proj_RTT = XMLoadFloat4x4(&g_proj_RTT);
	XMMATRIX wrld_Skybox = XMLoadFloat4x4(&g_wrld_Skybox);
	XMMATRIX wrld_Cube = XMLoadFloat4x4(&g_wrld_Cube);
	XMMATRIX wrld_GroundPlane = XMLoadFloat4x4(&g_wrld_GroundPlane);
	XMMATRIX wrld_Brazier01 = XMLoadFloat4x4(&g_wrld_Brazier01);
	XMMATRIX wrld_Spaceship = XMLoadFloat4x4(&g_wrld_Spaceship);
	XMMATRIX wrld_Sun = XMLoadFloat4x4(&g_wrld_Sun);
	XMMATRIX wrld_Earth = XMLoadFloat4x4(&g_wrld_Earth);
	XMMATRIX wrld_Moon = XMLoadFloat4x4(&g_wrld_Moon);
	XMMATRIX wrld_Mars = XMLoadFloat4x4(&g_wrld_Mars);
	XMMATRIX wrld_Jupiter = XMLoadFloat4x4(&g_wrld_Jupiter);
	// ----- RETRIEVE MATRICES -----

	// ----- LIGHTS -----
	// directional
#define LIGHTS_DIR 1
	DirectionalLight dLights[MAX_DIRECTIONAL_LIGHTS] =
	{
		// dir, color
		{ { 0, 1, 0, 0 }, { 0, 0, 1, 1 } },
		{},
		{}
	};
	// point
#define LIGHTS_PNT 1
	PointLight pLights[MAX_POINT_LIGHTS] =
	{
		// pos, range, atten, color
		{ { 1.5f, 0.5, 0, 1 }, 10, { 0, 0, 0.5f}, { 0, 1, 0, 1 } },
		{},
		{}
	};
	// spot
	SpotLight sLights[MAX_SPOT_LIGHTS] = {};
	// ----- LIGHTS -----

	// ----- UPDATE WORLD POSITIONS -----
	// --- CUBE ---
	// orbit about origin
	rotate = XMMatrixRotationY(0.5f * t);
	wrld_Cube = XMMatrixTranslation(2.5f, 6, 0) * rotate;
	// --- CUBE ---
	// --- GROUND PLANE ---
	wrld_GroundPlane = XMMatrixTranslation(0, -1, 0);
	// --- GROUND PLANE ---
	// --- SPACESHIP ---
	// orbit about origin
	rotate = XMMatrixRotationY(-1.3f * t);
	wrld_Spaceship = XMMatrixTranslation(5, 2, 0) * rotate;
	// --- SPACESHIP ---
	// --- SUN ---
	// spin
	scale = XMMatrixScaling(1.2f, 1.2f, 1.2f);
	rotate = XMMatrixRotationY(0.1f * t);
	wrld_Sun = scale * rotate * XMMatrixTranslation(0, 3, 0);
	// --- SUN ---
	// --- EARTH ---
	// orbit about origin
	scale = XMMatrixScaling(0.4f, 0.4f, 0.4f);
	rotate = XMMatrixRotationY(0.74f * t);
	wrld_Earth = scale * XMMatrixTranslation(3, 3, -2) * rotate;
	// spin
	rotate = XMMatrixRotationY(2.1f * t);
	wrld_Earth = rotate * wrld_Earth;
	// --- EARTH ---
	// --- MOON ---
	// orbit about earth
	scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	rotate = XMMatrixRotationY(0.4f * t);
	wrld_Moon = scale * XMMatrixTranslation(2, 0, 0) * rotate * wrld_Earth;
	// --- MOON ---
	// --- MARS ---
	// orbit about origin
	scale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	rotate = XMMatrixRotationY(0.53f * t);
	wrld_Mars = scale * XMMatrixTranslation(4, 3, 3) * rotate;
	// spin
	rotate = XMMatrixRotationY(1.6f * t);
	wrld_Mars = rotate * wrld_Mars;
	// --- MARS ---
	// --- JUPITER ---
	// orbit about origin
	scale = XMMatrixScaling(0.7f, 0.7f, 0.7f);
	rotate = XMMatrixRotationY(0.31f * t);
	wrld_Jupiter = scale * XMMatrixTranslation(6, 3, -3) * rotate;
	// spin
	rotate = XMMatrixRotationY(1.2f * t);
	wrld_Jupiter = rotate * wrld_Jupiter;
	// --- JUPITER ---
	// --- LIGHTS ---
	// DLIGHT 0
	XMMATRIX lightMatrix = XMMatrixTranslation(dLights[0].direction.x, dLights[0].direction.y, dLights[0].direction.z);
	rotate = XMMatrixRotationZ(0.4f * t);
	XMStoreFloat4(&dLights[0].direction, (lightMatrix * rotate).r[3]);
	// PLIGHT 0
	lightMatrix = XMMatrixTranslation(pLights[0].position.x, pLights[0].position.y, pLights[0].position.z);
	rotate = XMMatrixRotationY(0.7f * t);
	XMStoreFloat4(&pLights[0].position, (lightMatrix * rotate).r[3]);
	// --- LIGHTS ---
	// ----- UPDATE WORLD POSITIONS -----

	// ----- HANDLE TOGGLES -----
	// camera
	static bool keyHeld_freelook = false;
	bool keyPressed_freelook = GetAsyncKeyState('C');
	if (!keyHeld_freelook && keyPressed_freelook) // toggle freelook
	{
		keyHeld_freelook = true;
		g_freelook = !g_freelook;
	}
	if (keyHeld_freelook && !keyPressed_freelook) // reset freelook held flag
	{
		keyHeld_freelook = false;
	}
	// vertex shader
	static bool keyHeld_defaultVS = false;
	bool keyPressed_defaultVS = GetAsyncKeyState('1');
	if (!keyHeld_defaultVS && keyPressed_defaultVS) // toggle defaultVS
	{
		keyHeld_defaultVS = true;
		g_defaultVS = !g_defaultVS;
	}
	if (keyHeld_defaultVS && !keyPressed_defaultVS) // reset defaultVS held flag
	{
		keyHeld_defaultVS = false;
	}
	// geometry shader
	static bool keyHeld_defaultGS = false;
	bool keyPressed_defaultGS = GetAsyncKeyState('2');
	if (!keyHeld_defaultGS && keyPressed_defaultGS) // toggle defaultGS
	{
		keyHeld_defaultGS = true;
		g_defaultGS = !g_defaultGS;
	}
	if (keyHeld_defaultGS && !keyPressed_defaultGS) // reset defaultGS held flag
	{
		keyHeld_defaultGS = false;
	}
	// pixel shader
	static bool keyHeld_defaultPS = false;
	bool keyPressed_defaultPS = GetAsyncKeyState('3');
	if (!keyHeld_defaultPS && keyPressed_defaultPS) // toggle defaultPS
	{
		keyHeld_defaultPS = true;
		g_defaultPS = !g_defaultPS;
	}
	if (keyHeld_defaultPS && !keyPressed_defaultPS) // reset defaultPS held flag
	{
		keyHeld_defaultPS = false;
	}
	// ----- HANDLE TOGGLES -----

	// ----- UPDATE CAMERA -----
	// -- POSITION --
	FLOAT x, y, z;
	x = y = z = 0.0f;
	if (GetAsyncKeyState('A'))			x -= g_camMoveSpeed * dt; // move left
	if (GetAsyncKeyState('D'))			x += g_camMoveSpeed * dt; // move right
	if (GetAsyncKeyState(VK_LSHIFT))	y -= g_camMoveSpeed * dt; // move down
	if (GetAsyncKeyState(VK_SPACE))		y += g_camMoveSpeed * dt; // move up
	if (GetAsyncKeyState('S'))			z -= g_camMoveSpeed * dt; // move backward
	if (GetAsyncKeyState('W'))			z += g_camMoveSpeed * dt; // move forward
	// apply offset
	//view = (XMMatrixTranslation(x, 0, z) * view) * XMMatrixTranslation(0, y, 0);
	view = XMMatrixTranslation(x, y, z) * view;
	// -- POSITION --
	// -- ROTATION --
	FLOAT xr, yr;
	xr = yr = 0.0f;
	if (GetAsyncKeyState(VK_UP))	xr -= DEGREES_TO_RADIANS(g_camRotSpeed) * dt; // rotate upward
	if (GetAsyncKeyState(VK_DOWN))	xr += DEGREES_TO_RADIANS(g_camRotSpeed) * dt; // rotate downward
	if (GetAsyncKeyState(VK_LEFT))	yr -= DEGREES_TO_RADIANS(g_camRotSpeed) * dt; // rotate left
	if (GetAsyncKeyState(VK_RIGHT))	yr += DEGREES_TO_RADIANS(g_camRotSpeed) * dt; // rotate right
	// apply rotation
	XMVECTOR camPos = view.r[3];
	view = view * XMMatrixTranslationFromVector(-1 * camPos);
	view = XMMatrixRotationX(xr) * (view * XMMatrixRotationY(yr));
	view = view * XMMatrixTranslationFromVector(camPos);
	// -- ROTATION --
	// -- ZOOM --
	if (GetAsyncKeyState(VK_OEM_MINUS)) // zoom out
	{
		g_camZoom -= g_camMoveSpeed * dt;
		if (g_camZoom < g_camZoomMin)
			g_camZoom = g_camZoomMin;
	}
	if (GetAsyncKeyState(VK_OEM_PLUS)) // zoom in
	{
		g_camZoom += g_camMoveSpeed * dt;
		if (g_camZoom > g_camZoomMax)
			g_camZoom = g_camZoomMax;
	}
	// -- ZOOM --
	// -- NEAR / FAR PLANES --
	if (GetAsyncKeyState(VK_OEM_4)) // far plane closer
	{
		g_camFarPlane -= g_camFarSpeed * dt;
		if (g_camFarPlane < g_camFarMin)
			g_camFarPlane = g_camFarMin;
	}
	if (GetAsyncKeyState(VK_OEM_6)) // far plane farther
	{
		g_camFarPlane += g_camFarSpeed * dt;
		if (g_camFarPlane > g_camFarMax)
			g_camFarPlane = g_camFarMax;
	}
	if (GetAsyncKeyState(VK_OEM_1)) // near plane closer
	{
		g_camNearPlane -= g_camNearSpeed * dt;
		if (g_camNearPlane < g_camNearMin)
			g_camNearPlane = g_camNearMin;
	}
	if (GetAsyncKeyState(VK_OEM_7)) // near plane farther
	{
		g_camNearPlane += g_camNearSpeed * dt;
		if (g_camNearPlane > g_camNearMax)
			g_camNearPlane = g_camNearMax;
	}
	// -- NEAR / FAR PLANES --
	// reset camera
	if (GetAsyncKeyState(VK_BACK)) // reset zoom, near / far planes
	{
		g_camZoom = 1.0f;
		g_camNearPlane = 0.01f;
		g_camFarPlane = 100.0f;
	}
	// update projection matrix with current zoom level and near/far planes
	proj = XMMatrixPerspectiveFovLH(XM_PIDIV4 / g_camZoom, windowWidth / (FLOAT)windowHeight, g_camNearPlane, g_camFarPlane);
	// ----- UPDATE CAMERA -----

	// ----- PER-INSTANCE DATA -----
	XMMATRIX instanceOffsets[MAX_INSTANCES] = {};
	XMFLOAT4 instanceColors[MAX_INSTANCES] = {};
	// ----- PER-INSTANCE DATA -----

	// UPDATES / DRAW SETUP
	// --------------------------------------------------
	// DRAWING

	// ---------- RENDER-TO-TEXTURE PASS -----------
	// ----- SET SHARED CONSTANT BUFFER VALUES -----
	// vertex
	XMVECTOR determinant = XMMatrixDeterminant(view);
	cBufferVS.viewMatrix = XMMatrixInverse(&determinant, view);
	cBufferVS.projectionMatrix = proj_RTT;
	cBufferVS.time = t;
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 1, 1, 1, 1 };
	cBufferPS.directionalLights[0] = dLights[0];
	cBufferPS.pointLights[0] = pLights[0];
	cBufferPS.time = t;
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	DXDeviceContext->RSSetViewports(1, &g_viewport0);
	// set render target view
	DXDeviceContext->OMSetRenderTargets(1, &RenderToTextureDXRenderTargetView, g_p_depthStencilView_RTT);
	// set shader constant buffers
	DXDeviceContext->VSSetConstantBuffers(0, 1, &g_p_cBufferVS);
	DXDeviceContext->PSSetConstantBuffers(1, 1, &g_p_cBufferPS);
	// clear render target view
	DXDeviceContext->ClearRenderTargetView(RenderToTextureDXRenderTargetView, clearColor);
	// clear depth stencil view to 1.0 (max depth)
	DXDeviceContext->ClearDepthStencilView(g_p_depthStencilView_RTT, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Skybox, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixTranslationFromVector(view.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS_CubeMap, 0, 0);
	DXDeviceContext->PSSetShaderResources(1, 1, &g_p_SRV_Skybox);
	DXDeviceContext->PSSetSamplers(0, 1, &g_p_samplerLinear);
	DXDeviceContext->DrawIndexed(g_numInds_Skybox, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	DXDeviceContext->ClearDepthStencilView(g_p_depthStencilView_RTT, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_GroundPlane, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_GroundPlane, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	DXDeviceContext->DrawIndexed(g_numInds_GroundPlane, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Brazier01, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Brazier01, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Brazier01);
	DXDeviceContext->DrawIndexed(g_numInds_Brazier01, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Spaceship, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Spaceship, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Spaceship);
	DXDeviceContext->DrawIndexed(g_numInds_Spaceship, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Sun);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Earth);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Moon);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Mars);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Jupiter);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[1] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[2] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[3] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[4] = XMMatrixIdentity();
	// size scaling
	FLOAT sizeScale = 1.5f;
	// distance scaling
	FLOAT distScale = 1.0f;
	// --- DIRECTIONAL ---
	sizeScale = 2.0f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	distScale = 10.0f;
	for (UINT i = 0; i < LIGHTS_DIR; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(distScale * dLights[i].direction.x,
			distScale * dLights[i].direction.y, distScale * dLights[i].direction.z);
		DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(pLights[i].position.x, pLights[i].position.y, pLights[i].position.z);
		DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- POINT ---
	// ----- VISUAL LIGHTS -----
	// ---------- RENDER-TO-TEXTURE PASS -----------

	// ---------- MAIN RENDER PASS ----------
	// ----- SET SHARED CONSTANT BUFFER VALUES -----
	// vertex
	if (g_freelook)
	{
		XMVECTOR determinant = XMMatrixDeterminant(view);
		cBufferVS.viewMatrix = XMMatrixInverse(&determinant, view);
	}
	else
	{
		XMVECTOR eye = XMVectorSet(0, 10, -10, 1);
		XMVECTOR at = wrld_Cube.r[3];
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);
		cBufferVS.viewMatrix = XMMatrixLookAtLH(eye, at, up);
	}
	cBufferVS.projectionMatrix = proj;
	cBufferVS.time = t;
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 0.5f, 0.5f, 0.5f, 1 };
	cBufferPS.directionalLights[0] = dLights[0];
	cBufferPS.pointLights[0] = pLights[0];
	cBufferPS.time = t;
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	DXDeviceContext->RSSetViewports(1, &g_viewport0);
	// set render target view
	DXDeviceContext->OMSetRenderTargets(1, &MainDXRenderTargetView, MainDXDepthStencilView);
	// set shader constant buffers
	DXDeviceContext->VSSetConstantBuffers(0, 1, &g_p_cBufferVS);
	DXDeviceContext->PSSetConstantBuffers(1, 1, &g_p_cBufferPS);
	// clear render target view
	DXDeviceContext->ClearRenderTargetView(MainDXRenderTargetView, clearColor);
	// clear depth stencil view to 1.0 (max depth)
	DXDeviceContext->ClearDepthStencilView(MainDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Skybox, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixTranslationFromVector(view.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS_CubeMap, 0, 0);
	DXDeviceContext->PSSetShaderResources(1, 1, &g_p_SRV_Skybox);
	DXDeviceContext->PSSetSamplers(0, 1, &g_p_samplerLinear);
	DXDeviceContext->DrawIndexed(g_numInds_Skybox, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	DXDeviceContext->ClearDepthStencilView(MainDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_RTT);
	DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_GroundPlane, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_GroundPlane, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	DXDeviceContext->DrawIndexed(g_numInds_GroundPlane, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Brazier01, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Brazier01, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	if (g_defaultVS) DXDeviceContext->VSSetShader(g_p_VS, 0, 0);	// default shader
	else DXDeviceContext->VSSetShader(g_p_VS_Distort, 0, 0);		// fancy shader
	if (g_defaultGS) DXDeviceContext->GSSetShader(g_p_GS, 0, 0);	// default shader
	else DXDeviceContext->GSSetShader(g_p_GS_Distort, 0, 0);		// fancy shader
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	if (g_defaultPS) DXDeviceContext->PSSetShader(g_p_PS, 0, 0);	// default shader
	else DXDeviceContext->PSSetShader(g_p_PS_Distort, 0, 0);		// fancy shader
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Brazier01);
	DXDeviceContext->DrawIndexed(g_numInds_Brazier01, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Spaceship, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Spaceship, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Spaceship);
	DXDeviceContext->DrawIndexed(g_numInds_Spaceship, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Sun);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Earth);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Moon);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Mars);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Jupiter);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[1] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[2] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[3] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[4] = XMMatrixIdentity();
	// size scaling
	sizeScale = 1.5f;
	// distance scaling
	distScale = 1.0f;
	// --- DIRECTIONAL ---
	sizeScale = 2.0f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	distScale = 10.0f;
	for (UINT i = 0; i < LIGHTS_DIR; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(distScale * dLights[i].direction.x,
			distScale * dLights[i].direction.y, distScale * dLights[i].direction.z);
		DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(pLights[i].position.x, pLights[i].position.y, pLights[i].position.z);
		DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- POINT ---
	// ----- VISUAL LIGHTS -----
	// ---------- MAIN RENDER PASS -----------

	// ---------- MINIMAP RENDER PASS ----------
	// ----- SET SHARED CONSTANT BUFFER VALUES -----
	// vertex
	XMVECTOR eye = XMVectorSet(0, 15, 0, 1);
	XMVECTOR at = XMVectorSet(0, 0, 0, 1);
	XMVECTOR up = XMVectorSet(0, 0, 1, 0);
	XMMATRIX view1 = XMMatrixLookAtLH(eye, at, up);
	determinant = XMMatrixDeterminant(view1);
	view1 = XMMatrixInverse(&determinant, view1);
	determinant = XMMatrixDeterminant(view1);
	cBufferVS.viewMatrix = XMMatrixInverse(&determinant, view1);
	cBufferVS.projectionMatrix = proj;
	cBufferVS.time = t;
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 0.5f, 0.5f, 0.5f, 1 };
	cBufferPS.directionalLights[0] = dLights[0];
	cBufferPS.pointLights[0] = pLights[0];
	cBufferPS.time = t;
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	DXDeviceContext->RSSetViewports(1, &g_viewport1);
	// set render target view
	DXDeviceContext->OMSetRenderTargets(1, &MainDXRenderTargetView, MainDXDepthStencilView);
	// set shader constant buffers
	DXDeviceContext->VSSetConstantBuffers(0, 1, &g_p_cBufferVS);
	DXDeviceContext->PSSetConstantBuffers(1, 1, &g_p_cBufferPS);
	// clear depth stencil view to 1.0 (max depth)
	DXDeviceContext->ClearDepthStencilView(MainDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Skybox, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixTranslationFromVector(view1.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS_CubeMap, 0, 0);
	DXDeviceContext->PSSetShaderResources(1, 1, &g_p_SRV_Skybox);
	DXDeviceContext->PSSetSamplers(0, 1, &g_p_samplerLinear);
	DXDeviceContext->DrawIndexed(g_numInds_Skybox, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	DXDeviceContext->ClearDepthStencilView(MainDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_RTT);
	DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_GroundPlane, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_GroundPlane, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	DXDeviceContext->DrawIndexed(g_numInds_GroundPlane, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Brazier01, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Brazier01, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Brazier01);
	DXDeviceContext->DrawIndexed(g_numInds_Brazier01, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Spaceship, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Spaceship, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Spaceship);
	DXDeviceContext->DrawIndexed(g_numInds_Spaceship, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Sun);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Earth);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Moon);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Mars);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
	DXDeviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(g_p_PS, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Jupiter);
	DXDeviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[1] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[2] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[3] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[4] = XMMatrixIdentity();
	// size scaling
	sizeScale = 1.5f;
	// distance scaling
	distScale = 1.0f;
	// --- DIRECTIONAL ---
	sizeScale = 2.0f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	distScale = 10.0f;
	for (UINT i = 0; i < LIGHTS_DIR; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(distScale * dLights[i].direction.x,
			distScale * dLights[i].direction.y, distScale * dLights[i].direction.z);
		DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(pLights[i].position.x, pLights[i].position.y, pLights[i].position.z);
		DXDeviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		DXDeviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- POINT ---
	// ----- VISUAL LIGHTS -----
	// ---------- MINIMAP RENDER PASS -----------

	// present back buffer; change args to limit/sync framerate
	DXSwapChain->Present(1, 0);

	// DRAWING
	// --------------------------------------------------
	// STORE VARS

	// ----- STORE MATRICES -----
	XMStoreFloat4x4(&g_wrld, wrld);
	XMStoreFloat4x4(&g_view, view);
	XMStoreFloat4x4(&g_proj, proj);
	XMStoreFloat4x4(&g_proj_RTT, proj_RTT);
	XMStoreFloat4x4(&g_wrld_Skybox, wrld_Skybox);
	XMStoreFloat4x4(&g_wrld_Cube, wrld_Cube);
	XMStoreFloat4x4(&g_wrld_GroundPlane, wrld_GroundPlane);
	XMStoreFloat4x4(&g_wrld_Brazier01, wrld_Brazier01);
	// ----- STORE MATRICES -----

	// STORE VARS
	// --------------------------------------------------
}

void Cleanup()
{
	// --- SHADERS ---
	if (g_p_PS_SolidColorLights) g_p_PS_SolidColorLights->Release();
	if (g_p_PS_SolidColor) g_p_PS_SolidColor->Release();
	if (g_p_PS_InputColorLights) g_p_PS_InputColorLights->Release();
	if (g_p_PS_InputColor) g_p_PS_InputColor->Release();
	if (g_p_PS_Distort) g_p_PS_Distort->Release();
	if (g_p_PS_CubeMap) g_p_PS_CubeMap->Release();
	if (g_p_PS) g_p_PS->Release();
	if (g_p_GS_Distort) g_p_GS_Distort->Release();
	if (g_p_GS) g_p_GS->Release();
	if (g_p_VS_Distort) g_p_VS_Distort->Release();
	if (g_p_VS) g_p_VS->Release();
	// --- SAMPLER STATES ---
	if (g_p_samplerLinear) g_p_samplerLinear->Release();
	// --- SHADER RESOURCE VIEWS ---
	if (g_p_SRV_RTT) g_p_SRV_RTT->Release();
	if (g_p_tex_RTT) g_p_tex_RTT->Release();
	if (g_p_SRV_Jupiter) g_p_SRV_Jupiter->Release();
	if (g_p_SRV_Mars) g_p_SRV_Mars->Release();
	if (g_p_SRV_Moon) g_p_SRV_Moon->Release();
	if (g_p_SRV_Earth) g_p_SRV_Earth->Release();
	if (g_p_SRV_Sun) g_p_SRV_Sun->Release();
	if (g_p_SRV_Spaceship) g_p_SRV_Spaceship->Release();
	if (g_p_SRV_Brazier01) g_p_SRV_Brazier01->Release();
	if (g_p_SRV_Skybox) g_p_SRV_Skybox->Release();
	// --- CONSTANT BUFFERS ---
	if (g_p_cBufferPS) g_p_cBufferPS->Release();
	if (g_p_cBufferVS) g_p_cBufferVS->Release();
	// --- VERT / IND BUFFERS ---
	// PLANET
	if (g_p_iBuffer_Planet) g_p_iBuffer_Planet->Release();
	if (g_p_vBuffer_Planet) g_p_vBuffer_Planet->Release();
	// SPACESHIP
	if (g_p_iBuffer_Spaceship) g_p_iBuffer_Spaceship->Release();
	if (g_p_vBuffer_Spaceship) g_p_vBuffer_Spaceship->Release();
	// BRAZIER01
	if (g_p_iBuffer_Brazier01) g_p_iBuffer_Brazier01->Release();
	if (g_p_vBuffer_Brazier01) g_p_vBuffer_Brazier01->Release();
	// GROUND PLANE
	if (g_p_iBuffer_GroundPlane) g_p_iBuffer_GroundPlane->Release();
	if (g_p_vBuffer_GroundPlane) g_p_vBuffer_GroundPlane->Release();
	// CUBE
	if (g_p_iBuffer_Cube) g_p_iBuffer_Cube->Release();
	if (g_p_vBuffer_Cube) g_p_vBuffer_Cube->Release();
	// SKYBOX
	if (g_p_iBuffer_Skybox) g_p_iBuffer_Skybox->Release();
	if (g_p_vBuffer_Skybox) g_p_vBuffer_Skybox->Release();
	// --- VERTEX LAYOUT ---
	if (g_p_vertexLayout) g_p_vertexLayout->Release();
	// --- DEPTH STENCILS ---
	if (g_p_depthStencilView_RTT) g_p_depthStencilView_RTT->Release();
	if (g_p_depthStencil_RTT) g_p_depthStencil_RTT->Release();
	if (MainDXDepthStencilView) MainDXDepthStencilView->Release();
	if (MainDXDepthStencil) MainDXDepthStencil->Release();
	// --- RENDER TARGET VIEWS ---
	if (RenderToTextureDXRenderTargetView) RenderToTextureDXRenderTargetView->Release();
	if (MainDXRenderTargetView) MainDXRenderTargetView->Release();
	// --- DEVICE / SWAP CHAIN ---
	if (DXDeviceContext) DXDeviceContext->Release();
	if (DXSwapChain) DXSwapChain->Release();
	if (DXDevice) DXDevice->Release();
}
