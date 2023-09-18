#include "stdafx.h"
#include "Project.h"

#include <iostream>

// DirectX
#include <d3d11.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")
#include "DDSTextureLoader.h"

// OBJ
#include "OBJDataLoader.h"

// shaders
#include "VertexShaderDefault.csh"
#include "VertexShaderDistort.csh"
#include "GeometryShaderDefault.csh"
#include "GeometryShaderDistort.csh"
#include "PixelShaderDefault.csh"
#include "PixelShaderCubeMap.csh"
#include "PixelShaderDistort.csh"
#include "PixelShaderInputColor.csh"
#include "PixelShaderInputColorLights.csh"
#include "PixelShaderSolidColor.csh"
#include "PixelShaderSolidColorLights.csh"

using namespace DirectX;


#define MAX_LOADSTRING 100

#define MAX_INSTANCES 5
#define MAX_DIRECTIONAL_LIGHTS 3
#define MAX_POINT_LIGHTS 3
#define MAX_SPOT_LIGHTS 3

#define DEGREES_TO_RADIANS(degrees) (degrees * (XM_PI / 180.0f))


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


HINSTANCE CurrentHInstance = nullptr;
HWND HWindow = nullptr;
WCHAR TitleBarText[MAX_LOADSTRING];
WCHAR MainWindowClassName[MAX_LOADSTRING];


D3D_DRIVER_TYPE DXDriverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL DXFeatureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device* DXDevice = nullptr;
IDXGISwapChain* DXSwapChain = nullptr;

ID3D11DeviceContext* DXDeviceContext = nullptr;
ID3D11RenderTargetView* MainDXRenderTargetView = nullptr;
ID3D11RenderTargetView* RenderToTextureDXRenderTargetView = nullptr;

ID3D11Texture2D* MainDXDepthStencil = nullptr;
ID3D11DepthStencilView* MainDXDepthStencilView = nullptr;
ID3D11Texture2D* RenderToTextureDXDepthStencil = nullptr;
ID3D11DepthStencilView* RenderToTextureDXDepthStencilView = nullptr;

D3D11_VIEWPORT MainDXViewport;
D3D11_VIEWPORT AlternateDXViewport;

ID3D11InputLayout* DXVertexLayout = nullptr;

ID3D11Buffer* SkyboxDXVertexBuffer = nullptr;
ID3D11Buffer* SkyboxDXIndexBuffer = nullptr;
UINT SkyboxVertexCount = 0;
UINT SkyboxIndexCount = 0;

ID3D11Buffer* CubeDXVertexBuffer = nullptr;
ID3D11Buffer* CubeDXIndexBuffer = nullptr;
UINT CubeVertexCount = 0;
UINT CubeIndexCount = 0;

ID3D11Buffer* GroundPlaneDXVertexBuffer = nullptr;
ID3D11Buffer* GroundPlaneDXIndexBuffer = nullptr;
UINT GroundPlaneVertexCount = 0;
UINT GroundPlaneIndexCount = 0;
UINT GroundPlaneDivisionCount = 100;
FLOAT GroundPlaneScale = 10.0f;

ID3D11Buffer* Brazier01DXVertexBuffer = nullptr;
ID3D11Buffer* Brazier01DXIndexBuffer = nullptr;
UINT Brazier01VertexCount = 0;
UINT Brazier01IndexCount = 0;

ID3D11Buffer* SpaceshipDXVertexBuffer = nullptr;
ID3D11Buffer* SpaceshipDXIndexBuffer = nullptr;
UINT SpaceshipVertexCount = 0;
UINT SpaceshipIndexCount = 0;

ID3D11Buffer* PlanetDXVertexBuffer = nullptr;
ID3D11Buffer* PlanetDXIndexBuffer = nullptr;
UINT PlanetVertexCount = 0;
UINT PlanetIndexCount = 0;

ID3D11Buffer* DXVertexShaderConstantBuffer = nullptr;
ID3D11Buffer* DXPixelShaderConstantBuffer = nullptr;

ID3D11ShaderResourceView* SkyboxDXShaderResourceView = nullptr;
ID3D11ShaderResourceView* Brazier01DXShaderResourceView = nullptr;
ID3D11ShaderResourceView* SpaceshipDXShaderResourceView = nullptr;
ID3D11ShaderResourceView* SunDXShaderResourceView = nullptr;
ID3D11ShaderResourceView* EarthDXShaderResourceView = nullptr;
ID3D11ShaderResourceView* MoonDXShaderResourceView = nullptr;
ID3D11ShaderResourceView* MarsDXShaderResourceView = nullptr;
ID3D11ShaderResourceView* JupiterDXShaderResourceView = nullptr;
ID3D11Texture2D* RenderToTextureDXTexture = nullptr;
ID3D11ShaderResourceView* RenderToTextureDXShaderResourceView = nullptr;

ID3D11SamplerState* DXLinearSampler = nullptr;

ID3D11GeometryShader* DXGeometryShaderDefault = nullptr;
ID3D11GeometryShader* DXGeometryShaderDistort = nullptr;
ID3D11PixelShader* DXPixelShaderDefault = nullptr;
ID3D11PixelShader* DXPixelShaderCubeMap = nullptr;
ID3D11PixelShader* DXPixelShaderDistort = nullptr;
ID3D11PixelShader* DXPixelShaderInputColor = nullptr;
ID3D11PixelShader* DXPixelShaderInputColorLights = nullptr;
ID3D11PixelShader* DXPixelShaderSolidColor = nullptr;
ID3D11PixelShader* DXPixelShaderSolidColorLights = nullptr;
ID3D11VertexShader* DXVertexShaderDefault = nullptr;
ID3D11VertexShader* DXVertexShaderDistort = nullptr;

XMFLOAT4X4 WorldMatrix;
XMFLOAT4X4 ViewMatrix;
XMFLOAT4X4 ProjectionMatrix;
XMFLOAT4X4 RenderToTextureProjectionMatrix;
XMFLOAT4X4 SkyboxWorldMatrix;
XMFLOAT4X4 CubeWorldMatrix;
XMFLOAT4X4 GroundPlaneWorldMatrix;
XMFLOAT4X4 Brazier01WorldMatrix;
XMFLOAT4X4 SpaceshipWorldMatrix;
XMFLOAT4X4 SunWorldMatrix;
XMFLOAT4X4 EarthWorldMatrix;
XMFLOAT4X4 MoonWorldMatrix;
XMFLOAT4X4 MarsWorldMatrix;
XMFLOAT4X4 JupiterWorldMatrix;

FLOAT CameraMoveSpeed = 4.0f; // units per second
FLOAT CameraRotationSpeed = 40.0f; // degrees per second
FLOAT CameraZoomSpeed = 0.01f; // zoom level per second
FLOAT CameraZoomLevel = 1.0f;
const FLOAT CameraMinZoom = 0.5f;
const FLOAT CameraMaxZoom = 2.0f;
FLOAT CameraNearPlaneMoveSpeed = 1.0f; // near plane change per second
FLOAT CameraNearPlaneDistance = 0.01f; // current near plane
const FLOAT CameraNearPlaneMinDistance = 0.01f;
const FLOAT CameraNearPlaneMaxDistance = 9.0f;
FLOAT CameraFarPlaneMoveSpeed = 10.0f; // far plane change per second
FLOAT CameraFarPlaneDistance = 100.0f; // current far plane
const FLOAT CameraFarPlaneMinDistance = 10.0f;
const FLOAT CameraFarPlaneMaxDistance = 100.0f;

bool IsFreelookEnabled = true;
bool ShouldUsedDefaultVertexShader = true;
bool ShouldUseDefaultGeometryShader = true;
bool ShouldUseDefaultPixelShader = true;

// function forward definitions, aka Pain™
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void CreateProceduralGrid(Vertex, UINT, FLOAT, Vertex**, UINT&, UINT**, UINT&);
void ProcessOBJMesh(const char*, Vertex**, UINT&, UINT**, UINT&);
HRESULT InitializeDepthStencilView(UINT, UINT, ID3D11Texture2D**, ID3D11DepthStencilView**);
HRESULT InitializeVertexBuffer(UINT, Vertex**, ID3D11Buffer**);
HRESULT InitializeIndexBuffer(UINT, UINT**, ID3D11Buffer**);
HRESULT InitializeConstantBuffer(UINT, ID3D11Buffer**);
HRESULT InitializeSamplerState(ID3D11SamplerState**);
void Render();
void Cleanup();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, TitleBarText, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PROJECT, MainWindowClassName, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// initialize application
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT));
	MSG msg;

	// main message loop
	while (true)
	{
		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}
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
//		In this function, we save the instance handle in a global variable and
//		create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	CurrentHInstance = hInstance; // Store instance handle in our global variable

	HWindow = CreateWindowW(MainWindowClassName, TitleBarText, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	// XOR with thickframe prevents click & drag resize, XOR with maximizebox prevents control button resize

	if (!HWindow)
	{
		return FALSE;
	}

	ShowWindow(HWindow, nCmdShow);
	UpdateWindow(HWindow);

	HRESULT hresult;

	// get window dimensions
	RECT windowRect;
	GetClientRect(HWindow, &windowRect);
	UINT windowWidth = windowRect.right - windowRect.left;
	UINT windowHeight = windowRect.bottom - windowRect.top;

	// --------------------------------------------------
	// ATTACH D3D TO WINDOW

	// ---------- D3D DEVICE AND SWAP CHAIN ----------
	// ----- SWAP CHAIN DESCRIPTOR -----
	DXGI_SWAP_CHAIN_DESC swapChainDescriptor = {};
	swapChainDescriptor.BufferCount = 1; // number of buffers in swap chain
	swapChainDescriptor.OutputWindow = HWindow;
	swapChainDescriptor.Windowed = true;
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // pixel format
	swapChainDescriptor.BufferDesc.Width = windowWidth;
	swapChainDescriptor.BufferDesc.Height = windowHeight;
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // buffer usage flag; specifies what swap chain's buffer will be used for
	swapChainDescriptor.SampleDesc.Count = 1; // samples per pixel while drawing
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// ----- SWAP CHAIN DESCRIPTOR -----
	// create device and swap chain
	hresult = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		createDeviceFlags, &DXFeatureLevel, 1, D3D11_SDK_VERSION,
		&swapChainDescriptor, &DXSwapChain, &DXDevice, 0, &DXDeviceContext);
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
	hresult = DXDevice->CreateTexture2D(&texDesc_RTT, NULL, &RenderToTextureDXTexture);
	// shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc_RTT;
	SRVDesc_RTT.Format = texDesc_RTT.Format;
	SRVDesc_RTT.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc_RTT.Texture2D.MostDetailedMip = 0;
	SRVDesc_RTT.Texture2D.MipLevels = 1;
	hresult = DXDevice->CreateShaderResourceView(RenderToTextureDXTexture, &SRVDesc_RTT, &RenderToTextureDXShaderResourceView);
	// --- RTT ---

	// skybox
	hresult = CreateDDSTextureFromFile(DXDevice, L"Assets/skybox.dds", nullptr, &SkyboxDXShaderResourceView);

	// mesh textures
	hresult = CreateDDSTextureFromFile(DXDevice, L"Assets/Brazier01map.dds", nullptr, &Brazier01DXShaderResourceView);
	hresult = CreateDDSTextureFromFile(DXDevice, L"Assets/spaceshipmap.dds", nullptr, &SpaceshipDXShaderResourceView);
	hresult = CreateDDSTextureFromFile(DXDevice, L"Assets/sunmap.dds", nullptr, &SunDXShaderResourceView);
	hresult = CreateDDSTextureFromFile(DXDevice, L"Assets/earthmap.dds", nullptr, &EarthDXShaderResourceView);
	hresult = CreateDDSTextureFromFile(DXDevice, L"Assets/moonmap.dds", nullptr, &MoonDXShaderResourceView);
	hresult = CreateDDSTextureFromFile(DXDevice, L"Assets/marsmap.dds", nullptr, &MarsDXShaderResourceView);
	hresult = CreateDDSTextureFromFile(DXDevice, L"Assets/jupitermap.dds", nullptr, &JupiterDXShaderResourceView);
	// ---------- SHADER RESOURCE VIEWS ----------

	// ---------- SAMPLER STATES ----------
	hresult = InitializeSamplerState(&DXLinearSampler);
	// ---------- SAMPLER STATES ----------

	// ---------- RENDER TARGET VIEWS ----------
	// --- MAIN ---
	// get back buffer from swap chain
	ID3D11Resource* p_backBuffer = nullptr;
	hresult = DXSwapChain->GetBuffer(0, __uuidof(p_backBuffer), (void**)&p_backBuffer);
	// use back buffer to create render target view
	hresult = DXDevice->CreateRenderTargetView(p_backBuffer, nullptr, &MainDXRenderTargetView);
	// release back buffer
	p_backBuffer->Release();
	// --- MAIN ---
	// --- RTT ---
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc_RTT;
	RTVDesc_RTT.Format = texDesc_RTT.Format;
	RTVDesc_RTT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc_RTT.Texture2D.MipSlice = 0;
	hresult = DXDevice->CreateRenderTargetView(RenderToTextureDXTexture, &RTVDesc_RTT, &RenderToTextureDXRenderTargetView);
	// --- RTT ---
	// ---------- RENDER TARGET VIEWS ----------

	// ---------- DEPTH STENCILS ----------
	hresult = InitializeDepthStencilView(windowWidth, windowHeight, &MainDXDepthStencil, &MainDXDepthStencilView);
	hresult = InitializeDepthStencilView(windowWidth, windowHeight, &RenderToTextureDXDepthStencil, &RenderToTextureDXDepthStencilView);
	// ---------- DEPTH STENCILS ----------

	// ---------- VIEWPORTS ----------
	// setup main viewport
	MainDXViewport.Width = (FLOAT)windowWidth;
	MainDXViewport.Height = (FLOAT)windowHeight;
	MainDXViewport.TopLeftX = 0;
	MainDXViewport.TopLeftX = 0;
	MainDXViewport.MinDepth = 0.0f; // exponential depth; near/far planes are handled in projection matrix
	MainDXViewport.MaxDepth = 1.0f;

	// setup minimap viewport
	AlternateDXViewport.Width = (FLOAT)windowWidth / 4.0f;
	AlternateDXViewport.Height = (FLOAT)windowHeight / 4.0f;
	AlternateDXViewport.TopLeftX = 0;
	AlternateDXViewport.TopLeftX = 0;
	AlternateDXViewport.MinDepth = 0.0f; // exponential depth; near/far planes are handled in projection matrix
	AlternateDXViewport.MaxDepth = 1.0f;
	// ---------- VIEWPORTS ----------

	// ---------- SHADERS ----------
	hresult = DXDevice->CreateGeometryShader(GeometryShaderDefault, sizeof(GeometryShaderDefault), nullptr, &DXGeometryShaderDefault);
	hresult = DXDevice->CreateGeometryShader(GeometryShaderDistort, sizeof(GeometryShaderDistort), nullptr, &DXGeometryShaderDistort);

	hresult = DXDevice->CreatePixelShader(PixelShaderDefault, sizeof(PixelShaderDefault), nullptr, &DXPixelShaderDefault);
	hresult = DXDevice->CreatePixelShader(PixelShaderCubeMap, sizeof(PixelShaderCubeMap), nullptr, &DXPixelShaderCubeMap);
	hresult = DXDevice->CreatePixelShader(PixelShaderDistort, sizeof(PixelShaderDistort), nullptr, &DXPixelShaderDistort);
	hresult = DXDevice->CreatePixelShader(PixelShaderInputColor, sizeof(PixelShaderInputColor), nullptr, &DXPixelShaderInputColor);
	hresult = DXDevice->CreatePixelShader(PixelShaderInputColorLights, sizeof(PixelShaderInputColorLights), nullptr, &DXPixelShaderInputColorLights);
	hresult = DXDevice->CreatePixelShader(PixelShaderSolidColor, sizeof(PixelShaderSolidColor), nullptr, &DXPixelShaderSolidColor);
	hresult = DXDevice->CreatePixelShader(PixelShaderSolidColorLights, sizeof(PixelShaderSolidColorLights), nullptr, &DXPixelShaderSolidColorLights);

	hresult = DXDevice->CreateVertexShader(VertexShaderDefault, sizeof(VertexShaderDefault), nullptr, &DXVertexShaderDefault);
	hresult = DXDevice->CreateVertexShader(VertexShaderDistort, sizeof(VertexShaderDistort), nullptr, &DXVertexShaderDistort);
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
	hresult = DXDevice->CreateInputLayout(inputElementDesc, numInputElements, VertexShaderDefault, sizeof(VertexShaderDefault), &DXVertexLayout);
	// set input layout
	DXDeviceContext->IASetInputLayout(DXVertexLayout);
	// ---------- INPUT LAYOUT ----------

	// ---------- MESHES ----------
	// ----- SKYBOX -----
	// load vertex / index data
	Vertex* p_verts_Skybox = nullptr;
	UINT* p_inds_Skybox = nullptr;
	ProcessOBJMesh("Assets/skybox.obj", &p_verts_Skybox, SkyboxVertexCount, &p_inds_Skybox, SkyboxIndexCount);
	// create vertex / index buffers
	hresult = InitializeVertexBuffer(SkyboxVertexCount, &p_verts_Skybox, &SkyboxDXVertexBuffer);
	hresult = InitializeIndexBuffer(SkyboxIndexCount, &p_inds_Skybox, &SkyboxDXIndexBuffer);
	// set initial world matrix
	XMStoreFloat4x4(&SkyboxWorldMatrix, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Skybox;
	delete[] p_inds_Skybox;
	// ----- SKYBOX -----

	// ----- CUBE -----
	// load vertex / index data
	Vertex* p_verts_Cube = nullptr;
	UINT* p_inds_Cube = nullptr;
	ProcessOBJMesh("Assets/cube.obj", &p_verts_Cube, CubeVertexCount, &p_inds_Cube, CubeIndexCount);
	// create vertex / index buffers
	hresult = InitializeVertexBuffer(CubeVertexCount, (Vertex**)&p_verts_Cube, &CubeDXVertexBuffer);
	hresult = InitializeIndexBuffer(CubeIndexCount, (UINT**)&p_inds_Cube, &CubeDXIndexBuffer);
	// set initial world matrix
	XMStoreFloat4x4(&CubeWorldMatrix, XMMatrixIdentity());
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	// generate vertex / index data
	Vertex gridOrigin = { { 0, 0, 0, 1 }, { 0, 1, 0 }, { 0, 0, 0 }, {} };
	Vertex* p_verts_GroundPlane = nullptr;
	UINT* p_inds_GroundPlane = nullptr;
	CreateProceduralGrid(gridOrigin, GroundPlaneDivisionCount, GroundPlaneScale,
		&p_verts_GroundPlane, GroundPlaneVertexCount, &p_inds_GroundPlane, GroundPlaneIndexCount);
	// create vertex / index buffers
	hresult = InitializeVertexBuffer(GroundPlaneVertexCount, &p_verts_GroundPlane, &GroundPlaneDXVertexBuffer);
	hresult = InitializeIndexBuffer(GroundPlaneIndexCount, &p_inds_GroundPlane, &GroundPlaneDXIndexBuffer);
	// set initial world matrix
	XMStoreFloat4x4(&GroundPlaneWorldMatrix, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_GroundPlane;
	delete[] p_inds_GroundPlane;
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	// load vertex / index data
	Vertex* p_verts_Brazier01 = nullptr;
	UINT* p_inds_Brazier01 = nullptr;
	ProcessOBJMesh("Assets/Brazier01.obj", &p_verts_Brazier01, Brazier01VertexCount,
		&p_inds_Brazier01, Brazier01IndexCount);
	// create vertex / index buffers
	hresult = InitializeVertexBuffer(Brazier01VertexCount, &p_verts_Brazier01, &Brazier01DXVertexBuffer);
	hresult = InitializeIndexBuffer(Brazier01IndexCount, &p_inds_Brazier01, &Brazier01DXIndexBuffer);
	// set initial world matrix
	XMStoreFloat4x4(&Brazier01WorldMatrix, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Brazier01;
	delete[] p_inds_Brazier01;
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	// load vertex / index data
	Vertex* p_verts_Spaceship = nullptr;
	UINT* p_inds_Spaceship = nullptr;
	ProcessOBJMesh("Assets/spaceship.obj", &p_verts_Spaceship, SpaceshipVertexCount,
		&p_inds_Spaceship, SpaceshipIndexCount);
	// create vertex / index buffers
	hresult = InitializeVertexBuffer(SpaceshipVertexCount, &p_verts_Spaceship, &SpaceshipDXVertexBuffer);
	hresult = InitializeIndexBuffer(SpaceshipIndexCount, &p_inds_Spaceship, &SpaceshipDXIndexBuffer);
	// set initial world matrix
	XMStoreFloat4x4(&SpaceshipWorldMatrix, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Spaceship;
	delete[] p_inds_Spaceship;
	// ----- SPACESHIP -----

	// ----- PLANET -----
	// load vertex / index data
	Vertex* p_verts_Planet = nullptr;
	UINT* p_inds_Planet = nullptr;
	ProcessOBJMesh("Assets/planet.obj", &p_verts_Planet, PlanetVertexCount,
		&p_inds_Planet, PlanetIndexCount);
	// create vertex / index buffers
	hresult = InitializeVertexBuffer(PlanetVertexCount, &p_verts_Planet, &PlanetDXVertexBuffer);
	hresult = InitializeIndexBuffer(PlanetIndexCount, &p_inds_Planet, &PlanetDXIndexBuffer);
	// set initial world matrices
	XMStoreFloat4x4(&SunWorldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&EarthWorldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&MoonWorldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&MarsWorldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&JupiterWorldMatrix, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Planet;
	delete[] p_inds_Planet;
	// ----- PLANET -----
	// ---------- MESHES ----------

	// set type of topology to draw
	DXDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ---------- CONSTANT BUFFERS ----------
	hresult = InitializeConstantBuffer(sizeof(VertexShaderConstantBuffer), &DXVertexShaderConstantBuffer);
	hresult = InitializeConstantBuffer(sizeof(PixelShaderConstantBuffer), &DXPixelShaderConstantBuffer);
	// ---------- CONSTANT BUFFERS ----------

	// ---------- MATRICES ----------
	// world
	XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());

	// view
	XMVECTOR eye = XMVectorSet(0, 10, -15, 0);
	XMVECTOR at = XMVectorSet(0, 2, 0, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
	XMVECTOR determinant = XMMatrixDeterminant(view);
	XMStoreFloat4x4(&ViewMatrix, XMMatrixInverse(&determinant, view));

	// projection
	// main
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
	// RTT
	XMStoreFloat4x4(&RenderToTextureProjectionMatrix, XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
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
//  WM_PAINT	- Paint the main window
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
			DialogBox(CurrentHInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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
	//	{
	//		PAINTSTRUCT ps;
	//		HDC hdc = BeginPaint(hWnd, &ps);
	//		// TODO: Add any drawing code that uses hdc here...
	//		EndPaint(hWnd, &ps);
	//	}
	//	break;
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

void ProcessOBJMesh(const char* _filepath, Vertex** _pp_verts, UINT& vertexCount, UINT** _pp_inds, UINT& indexCount)
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

HRESULT InitializeDepthStencilView(UINT _width, UINT _height, ID3D11Texture2D** _pp_depthStencil,
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

HRESULT InitializeVertexBuffer(UINT _numVerts, Vertex** _pp_verts, ID3D11Buffer** _pp_vBuffer)
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

HRESULT InitializeIndexBuffer(UINT _numInds, UINT** _pp_inds, ID3D11Buffer** _pp_iBuffer)
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

HRESULT InitializeConstantBuffer(UINT _bufferSize, ID3D11Buffer** _pp_cBuffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = _bufferSize;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	return DXDevice->CreateBuffer(&bufferDesc, nullptr, _pp_cBuffer);
}

HRESULT InitializeSamplerState(ID3D11SamplerState** _pp_samplerState)
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
	XMMATRIX wrld = XMLoadFloat4x4(&WorldMatrix);
	XMMATRIX view = XMLoadFloat4x4(&ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&ProjectionMatrix);
	XMMATRIX proj_RTT = XMLoadFloat4x4(&RenderToTextureProjectionMatrix);
	XMMATRIX wrld_Skybox = XMLoadFloat4x4(&SkyboxWorldMatrix);
	XMMATRIX wrld_Cube = XMLoadFloat4x4(&CubeWorldMatrix);
	XMMATRIX wrld_GroundPlane = XMLoadFloat4x4(&GroundPlaneWorldMatrix);
	XMMATRIX wrld_Brazier01 = XMLoadFloat4x4(&Brazier01WorldMatrix);
	XMMATRIX wrld_Spaceship = XMLoadFloat4x4(&SpaceshipWorldMatrix);
	XMMATRIX wrld_Sun = XMLoadFloat4x4(&SunWorldMatrix);
	XMMATRIX wrld_Earth = XMLoadFloat4x4(&EarthWorldMatrix);
	XMMATRIX wrld_Moon = XMLoadFloat4x4(&MoonWorldMatrix);
	XMMATRIX wrld_Mars = XMLoadFloat4x4(&MarsWorldMatrix);
	XMMATRIX wrld_Jupiter = XMLoadFloat4x4(&JupiterWorldMatrix);
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
		IsFreelookEnabled = !IsFreelookEnabled;
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
		ShouldUsedDefaultVertexShader = !ShouldUsedDefaultVertexShader;
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
		ShouldUseDefaultGeometryShader = !ShouldUseDefaultGeometryShader;
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
		ShouldUseDefaultPixelShader = !ShouldUseDefaultPixelShader;
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
	if (GetAsyncKeyState('A'))			x -= CameraMoveSpeed * dt; // move left
	if (GetAsyncKeyState('D'))			x += CameraMoveSpeed * dt; // move right
	if (GetAsyncKeyState(VK_LSHIFT))	y -= CameraMoveSpeed * dt; // move down
	if (GetAsyncKeyState(VK_SPACE))		y += CameraMoveSpeed * dt; // move up
	if (GetAsyncKeyState('S'))			z -= CameraMoveSpeed * dt; // move backward
	if (GetAsyncKeyState('W'))			z += CameraMoveSpeed * dt; // move forward
	// apply offset
	//view = (XMMatrixTranslation(x, 0, z) * view) * XMMatrixTranslation(0, y, 0);
	view = XMMatrixTranslation(x, y, z) * view;
	// -- POSITION --
	// -- ROTATION --
	FLOAT xr, yr;
	xr = yr = 0.0f;
	if (GetAsyncKeyState(VK_UP))	xr -= DEGREES_TO_RADIANS(CameraRotationSpeed) * dt; // rotate upward
	if (GetAsyncKeyState(VK_DOWN))	xr += DEGREES_TO_RADIANS(CameraRotationSpeed) * dt; // rotate downward
	if (GetAsyncKeyState(VK_LEFT))	yr -= DEGREES_TO_RADIANS(CameraRotationSpeed) * dt; // rotate left
	if (GetAsyncKeyState(VK_RIGHT))	yr += DEGREES_TO_RADIANS(CameraRotationSpeed) * dt; // rotate right
	// apply rotation
	XMVECTOR camPos = view.r[3];
	view = view * XMMatrixTranslationFromVector(-1 * camPos);
	view = XMMatrixRotationX(xr) * (view * XMMatrixRotationY(yr));
	view = view * XMMatrixTranslationFromVector(camPos);
	// -- ROTATION --
	// -- ZOOM --
	if (GetAsyncKeyState(VK_OEM_MINUS)) // zoom out
	{
		CameraZoomLevel -= CameraMoveSpeed * dt;
		if (CameraZoomLevel < CameraMinZoom)
			CameraZoomLevel = CameraMinZoom;
	}
	if (GetAsyncKeyState(VK_OEM_PLUS)) // zoom in
	{
		CameraZoomLevel += CameraMoveSpeed * dt;
		if (CameraZoomLevel > CameraMaxZoom)
			CameraZoomLevel = CameraMaxZoom;
	}
	// -- ZOOM --
	// -- NEAR / FAR PLANES --
	if (GetAsyncKeyState(VK_OEM_4)) // far plane closer
	{
		CameraFarPlaneDistance -= CameraFarPlaneMoveSpeed * dt;
		if (CameraFarPlaneDistance < CameraFarPlaneMinDistance)
			CameraFarPlaneDistance = CameraFarPlaneMinDistance;
	}
	if (GetAsyncKeyState(VK_OEM_6)) // far plane farther
	{
		CameraFarPlaneDistance += CameraFarPlaneMoveSpeed * dt;
		if (CameraFarPlaneDistance > CameraFarPlaneMaxDistance)
			CameraFarPlaneDistance = CameraFarPlaneMaxDistance;
	}
	if (GetAsyncKeyState(VK_OEM_1)) // near plane closer
	{
		CameraNearPlaneDistance -= CameraNearPlaneMoveSpeed * dt;
		if (CameraNearPlaneDistance < CameraNearPlaneMinDistance)
			CameraNearPlaneDistance = CameraNearPlaneMinDistance;
	}
	if (GetAsyncKeyState(VK_OEM_7)) // near plane farther
	{
		CameraNearPlaneDistance += CameraNearPlaneMoveSpeed * dt;
		if (CameraNearPlaneDistance > CameraNearPlaneMaxDistance)
			CameraNearPlaneDistance = CameraNearPlaneMaxDistance;
	}
	// -- NEAR / FAR PLANES --
	// reset camera
	if (GetAsyncKeyState(VK_BACK)) // reset zoom, near / far planes
	{
		CameraZoomLevel = 1.0f;
		CameraNearPlaneDistance = 0.01f;
		CameraFarPlaneDistance = 100.0f;
	}
	// update projection matrix with current zoom level and near/far planes
	proj = XMMatrixPerspectiveFovLH(XM_PIDIV4 / CameraZoomLevel, windowWidth / (FLOAT)windowHeight, CameraNearPlaneDistance, CameraFarPlaneDistance);
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
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 1, 1, 1, 1 };
	cBufferPS.directionalLights[0] = dLights[0];
	cBufferPS.pointLights[0] = pLights[0];
	cBufferPS.time = t;
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	DXDeviceContext->RSSetViewports(1, &MainDXViewport);
	// set render target view
	DXDeviceContext->OMSetRenderTargets(1, &RenderToTextureDXRenderTargetView, RenderToTextureDXDepthStencilView);
	// set shader constant buffers
	DXDeviceContext->VSSetConstantBuffers(0, 1, &DXVertexShaderConstantBuffer);
	DXDeviceContext->PSSetConstantBuffers(1, 1, &DXPixelShaderConstantBuffer);
	// clear render target view
	DXDeviceContext->ClearRenderTargetView(RenderToTextureDXRenderTargetView, clearColor);
	// clear depth stencil view to 1.0 (max depth)
	DXDeviceContext->ClearDepthStencilView(RenderToTextureDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	DXDeviceContext->IASetVertexBuffers(0, 1, &SkyboxDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(SkyboxDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixTranslationFromVector(view.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderCubeMap, 0, 0);
	DXDeviceContext->PSSetShaderResources(1, 1, &SkyboxDXShaderResourceView);
	DXDeviceContext->PSSetSamplers(0, 1, &DXLinearSampler);
	DXDeviceContext->DrawIndexed(SkyboxIndexCount, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	DXDeviceContext->ClearDepthStencilView(RenderToTextureDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &CubeDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(CubeDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderInputColorLights, 0, 0);
	DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &GroundPlaneDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(GroundPlaneDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderInputColorLights, 0, 0);
	DXDeviceContext->DrawIndexed(GroundPlaneIndexCount, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &Brazier01DXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(Brazier01DXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &Brazier01DXShaderResourceView);
	DXDeviceContext->DrawIndexed(Brazier01IndexCount, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &SpaceshipDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(SpaceshipDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &SpaceshipDXShaderResourceView);
	DXDeviceContext->DrawIndexed(SpaceshipIndexCount, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &SunDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &EarthDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &MoonDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &MarsDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &JupiterDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &CubeDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(CubeDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
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
		DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(DXPixelShaderSolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(pLights[i].position.x, pLights[i].position.y, pLights[i].position.z);
		DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(DXPixelShaderSolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
	}
	// --- POINT ---
	// ----- VISUAL LIGHTS -----
	// ---------- RENDER-TO-TEXTURE PASS -----------

	// ---------- MAIN RENDER PASS ----------
	// ----- SET SHARED CONSTANT BUFFER VALUES -----
	// vertex
	if (IsFreelookEnabled)
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
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 0.5f, 0.5f, 0.5f, 1 };
	cBufferPS.directionalLights[0] = dLights[0];
	cBufferPS.pointLights[0] = pLights[0];
	cBufferPS.time = t;
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	DXDeviceContext->RSSetViewports(1, &MainDXViewport);
	// set render target view
	DXDeviceContext->OMSetRenderTargets(1, &MainDXRenderTargetView, MainDXDepthStencilView);
	// set shader constant buffers
	DXDeviceContext->VSSetConstantBuffers(0, 1, &DXVertexShaderConstantBuffer);
	DXDeviceContext->PSSetConstantBuffers(1, 1, &DXPixelShaderConstantBuffer);
	// clear render target view
	DXDeviceContext->ClearRenderTargetView(MainDXRenderTargetView, clearColor);
	// clear depth stencil view to 1.0 (max depth)
	DXDeviceContext->ClearDepthStencilView(MainDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	DXDeviceContext->IASetVertexBuffers(0, 1, &SkyboxDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(SkyboxDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixTranslationFromVector(view.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderCubeMap, 0, 0);
	DXDeviceContext->PSSetShaderResources(1, 1, &SkyboxDXShaderResourceView);
	DXDeviceContext->PSSetSamplers(0, 1, &DXLinearSampler);
	DXDeviceContext->DrawIndexed(SkyboxIndexCount, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	DXDeviceContext->ClearDepthStencilView(MainDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &CubeDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(CubeDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &RenderToTextureDXShaderResourceView);
	DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &GroundPlaneDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(GroundPlaneDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderInputColorLights, 0, 0);
	DXDeviceContext->DrawIndexed(GroundPlaneIndexCount, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &Brazier01DXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(Brazier01DXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	if (ShouldUsedDefaultVertexShader) DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);	// default shader
	else DXDeviceContext->VSSetShader(DXVertexShaderDistort, 0, 0);		// fancy shader
	if (ShouldUseDefaultGeometryShader) DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);	// default shader
	else DXDeviceContext->GSSetShader(DXGeometryShaderDistort, 0, 0);		// fancy shader
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	if (ShouldUseDefaultPixelShader) DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);	// default shader
	else DXDeviceContext->PSSetShader(DXPixelShaderDistort, 0, 0);		// fancy shader
	DXDeviceContext->PSSetShaderResources(0, 1, &Brazier01DXShaderResourceView);
	DXDeviceContext->DrawIndexed(Brazier01IndexCount, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &SpaceshipDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(SpaceshipDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &SpaceshipDXShaderResourceView);
	DXDeviceContext->DrawIndexed(SpaceshipIndexCount, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &SunDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &EarthDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &MoonDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &MarsDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &JupiterDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &CubeDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(CubeDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
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
		DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(DXPixelShaderSolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(pLights[i].position.x, pLights[i].position.y, pLights[i].position.z);
		DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(DXPixelShaderSolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
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
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 0.5f, 0.5f, 0.5f, 1 };
	cBufferPS.directionalLights[0] = dLights[0];
	cBufferPS.pointLights[0] = pLights[0];
	cBufferPS.time = t;
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	DXDeviceContext->RSSetViewports(1, &AlternateDXViewport);
	// set render target view
	DXDeviceContext->OMSetRenderTargets(1, &MainDXRenderTargetView, MainDXDepthStencilView);
	// set shader constant buffers
	DXDeviceContext->VSSetConstantBuffers(0, 1, &DXVertexShaderConstantBuffer);
	DXDeviceContext->PSSetConstantBuffers(1, 1, &DXPixelShaderConstantBuffer);
	// clear depth stencil view to 1.0 (max depth)
	DXDeviceContext->ClearDepthStencilView(MainDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	DXDeviceContext->IASetVertexBuffers(0, 1, &SkyboxDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(SkyboxDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixTranslationFromVector(view1.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderCubeMap, 0, 0);
	DXDeviceContext->PSSetShaderResources(1, 1, &SkyboxDXShaderResourceView);
	DXDeviceContext->PSSetSamplers(0, 1, &DXLinearSampler);
	DXDeviceContext->DrawIndexed(SkyboxIndexCount, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	DXDeviceContext->ClearDepthStencilView(MainDXDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &CubeDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(CubeDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &RenderToTextureDXShaderResourceView);
	DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &GroundPlaneDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(GroundPlaneDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderInputColorLights, 0, 0);
	DXDeviceContext->DrawIndexed(GroundPlaneIndexCount, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &Brazier01DXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(Brazier01DXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &Brazier01DXShaderResourceView);
	DXDeviceContext->DrawIndexed(Brazier01IndexCount, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &SpaceshipDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(SpaceshipDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &SpaceshipDXShaderResourceView);
	DXDeviceContext->DrawIndexed(SpaceshipIndexCount, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &SunDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &EarthDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &MoonDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &MarsDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &PlanetDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(PlanetDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.worldMatrix = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
	DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
	DXDeviceContext->GSSetShader(DXGeometryShaderDefault, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
	DXDeviceContext->PSSetShader(DXPixelShaderDefault, 0, 0);
	DXDeviceContext->PSSetShaderResources(0, 1, &JupiterDXShaderResourceView);
	DXDeviceContext->DrawIndexed(PlanetIndexCount, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	DXDeviceContext->IASetVertexBuffers(0, 1, &CubeDXVertexBuffer, strides, offsets);
	DXDeviceContext->IASetIndexBuffer(CubeDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
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
		DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(DXPixelShaderSolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.worldMatrix = scale * XMMatrixTranslation(pLights[i].position.x, pLights[i].position.y, pLights[i].position.z);
		DXDeviceContext->UpdateSubresource(DXVertexShaderConstantBuffer, 0, nullptr, &cBufferVS, 0, 0);
		DXDeviceContext->VSSetShader(DXVertexShaderDefault, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		DXDeviceContext->UpdateSubresource(DXPixelShaderConstantBuffer, 0, nullptr, &cBufferPS, 0, 0);
		DXDeviceContext->PSSetShader(DXPixelShaderSolidColor, 0, 0);
		DXDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
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
	XMStoreFloat4x4(&WorldMatrix, wrld);
	XMStoreFloat4x4(&ViewMatrix, view);
	XMStoreFloat4x4(&ProjectionMatrix, proj);
	XMStoreFloat4x4(&RenderToTextureProjectionMatrix, proj_RTT);
	XMStoreFloat4x4(&SkyboxWorldMatrix, wrld_Skybox);
	XMStoreFloat4x4(&CubeWorldMatrix, wrld_Cube);
	XMStoreFloat4x4(&GroundPlaneWorldMatrix, wrld_GroundPlane);
	XMStoreFloat4x4(&Brazier01WorldMatrix, wrld_Brazier01);
	// ----- STORE MATRICES -----

	// STORE VARS
	// --------------------------------------------------
}

void Cleanup()
{
	// the fact that all this shit is necessary is why i use C#
	if (DXPixelShaderSolidColorLights) DXPixelShaderSolidColorLights->Release();
	if (DXPixelShaderSolidColor) DXPixelShaderSolidColor->Release();
	if (DXPixelShaderInputColorLights) DXPixelShaderInputColorLights->Release();
	if (DXPixelShaderInputColor) DXPixelShaderInputColor->Release();
	if (DXPixelShaderDistort) DXPixelShaderDistort->Release();
	if (DXPixelShaderCubeMap) DXPixelShaderCubeMap->Release();
	if (DXPixelShaderDefault) DXPixelShaderDefault->Release();
	if (DXGeometryShaderDistort) DXGeometryShaderDistort->Release();
	if (DXGeometryShaderDefault) DXGeometryShaderDefault->Release();
	if (DXVertexShaderDistort) DXVertexShaderDistort->Release();
	if (DXVertexShaderDefault) DXVertexShaderDefault->Release();
	if (DXLinearSampler) DXLinearSampler->Release();
	if (RenderToTextureDXShaderResourceView) RenderToTextureDXShaderResourceView->Release();
	if (RenderToTextureDXTexture) RenderToTextureDXTexture->Release();
	if (JupiterDXShaderResourceView) JupiterDXShaderResourceView->Release();
	if (MarsDXShaderResourceView) MarsDXShaderResourceView->Release();
	if (MoonDXShaderResourceView) MoonDXShaderResourceView->Release();
	if (EarthDXShaderResourceView) EarthDXShaderResourceView->Release();
	if (SunDXShaderResourceView) SunDXShaderResourceView->Release();
	if (SpaceshipDXShaderResourceView) SpaceshipDXShaderResourceView->Release();
	if (Brazier01DXShaderResourceView) Brazier01DXShaderResourceView->Release();
	if (SkyboxDXShaderResourceView) SkyboxDXShaderResourceView->Release();
	if (DXPixelShaderConstantBuffer) DXPixelShaderConstantBuffer->Release();
	if (DXVertexShaderConstantBuffer) DXVertexShaderConstantBuffer->Release();
	if (PlanetDXIndexBuffer) PlanetDXIndexBuffer->Release();
	if (PlanetDXVertexBuffer) PlanetDXVertexBuffer->Release();
	if (SpaceshipDXIndexBuffer) SpaceshipDXIndexBuffer->Release();
	if (SpaceshipDXVertexBuffer) SpaceshipDXVertexBuffer->Release();
	if (Brazier01DXIndexBuffer) Brazier01DXIndexBuffer->Release();
	if (Brazier01DXVertexBuffer) Brazier01DXVertexBuffer->Release();
	if (GroundPlaneDXIndexBuffer) GroundPlaneDXIndexBuffer->Release();
	if (GroundPlaneDXVertexBuffer) GroundPlaneDXVertexBuffer->Release();
	if (CubeDXIndexBuffer) CubeDXIndexBuffer->Release();
	if (CubeDXVertexBuffer) CubeDXVertexBuffer->Release();
	if (SkyboxDXIndexBuffer) SkyboxDXIndexBuffer->Release();
	if (SkyboxDXVertexBuffer) SkyboxDXVertexBuffer->Release();
	if (DXVertexLayout) DXVertexLayout->Release();
	if (RenderToTextureDXDepthStencilView) RenderToTextureDXDepthStencilView->Release();
	if (RenderToTextureDXDepthStencil) RenderToTextureDXDepthStencil->Release();
	if (MainDXDepthStencilView) MainDXDepthStencilView->Release();
	if (MainDXDepthStencil) MainDXDepthStencil->Release();
	if (RenderToTextureDXRenderTargetView) RenderToTextureDXRenderTargetView->Release();
	if (MainDXRenderTargetView) MainDXRenderTargetView->Release();
	if (DXDeviceContext) DXDeviceContext->Release();
	if (DXSwapChain) DXSwapChain->Release();
	if (DXDevice) DXDevice->Release();
}
