//***************************************************************************************
// BoxDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates rendering a colored box.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************

#include <unordered_map>

#include "d3dApp.h"
#include "MathHelper.h"
#include "d3dcompiler.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ObjectConstants
{
	XMFLOAT4X4 World = MathHelper::Identity4x4();
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();
	void BuildConstantBuffer();
private:
	ComPtr<ID3D11Buffer> mBoxVB;
	ComPtr<ID3D11Buffer> mBoxIB;
	ComPtr<ID3D11Buffer> mObjCb;

	ComPtr<ID3D11VertexShader> mVS;
	ComPtr<ID3D11PixelShader>  mPS;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::vector<D3D11_INPUT_ELEMENT_DESC>             mInputDesc;

	ComPtr<ID3D11InputLayout> mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
                   PSTR      cmdLine, int         showCmd)
{
	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	BoxApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), //mFX(0), mTech(0), mfxWorldViewProj(0),
	  mInputLayout(0), mTheta(1.5f * MathHelper::Pi), mPhi(0.25f * MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Box Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

BoxApp::~BoxApp()
{
}

bool BoxApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildShaders();
	BuildVertexLayout();
	BuildConstantBuffer();

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BoxApp::BuildConstantBuffer()
{
	// Fill in a buffer description.
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth           = sizeof(ObjectConstants);
	cbDesc.Usage               = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags           = 0;
	cbDesc.StructureByteStride = 0;

	// Create the buffer.
	HR(md3dDevice->CreateBuffer(&cbDesc, nullptr, &mObjCb));
}

void BoxApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void BoxApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout.Get());
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, mBoxVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mBoxIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->VSSetShader(
	                                  // Pointer to a vertex shader
	                                  mVS.Get(),
	                                  // A pointer to an array of class-instance interfaces
	                                  nullptr,
	                                  // The number of class-instance interfaces in the array
	                                  0u);

	md3dImmediateContext->PSSetShader(
	                                  // Pointer to a vertex shader
	                                  mPS.Get(),
	                                  // A pointer to an array of class-instance interfaces
	                                  nullptr,
	                                  // The number of class-instance interfaces in the array
	                                  0u);

	// Set constants
	XMMATRIX world         = XMLoadFloat4x4(&mWorld);
	XMMATRIX view          = XMLoadFloat4x4(&mView);
	XMMATRIX proj          = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	ObjectConstants cbPerObject;
	XMStoreFloat4x4(&cbPerObject.World, XMMatrixTranspose(worldViewProj));

	// Provides access to subresource data
	D3D11_MAPPED_SUBRESOURCE msr;
	// Gets a pointer to the data contained in a subresource, and denies the GPU access to that subresource
	md3dImmediateContext->Map(
	                          // A pointer to a ID3D11Resource interface
	                          mObjCb.Get(),
	                          // Index number of the subresource
	                          0u,
	                          // A D3D11_MAP-typed value that specifies the CPU's read and write permissions for a resource
	                          D3D11_MAP_WRITE_DISCARD,
	                          // Flag that specifies what the CPU does when the GPU is busy
	                          0u,
	                          // output pointer
	                          &msr
	                         );

	memcpy(msr.pData, &cbPerObject, sizeof(cbPerObject));

	// Invalidate the pointer to a resource and reenable the GPU's access to that resource
	md3dImmediateContext->Unmap(mObjCb.Get(), 0u);

	md3dImmediateContext->VSSetConstantBuffers(0, 1, mObjCb.GetAddressOf());

	md3dImmediateContext->DrawIndexed(36, 0, 0);
	HR(mSwapChain->Present(0, 0));
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BoxApp::BuildGeometryBuffers()
{
	// Create vertex buffer
	Vertex vertices[] =
	{
		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White)},
		{XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black)},
		{XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red)},
		{XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green)},
		{XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue)},
		{XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow)},
		{XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan)},
		{XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta)}
	};

	D3D11_BUFFER_DESC vbd;
	vbd.Usage               = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth           = sizeof(Vertex) * 8;
	vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags      = 0;
	vbd.MiscFlags           = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));


	// Create the index buffer

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth           = sizeof(UINT) * 36;
	ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags      = 0;
	ibd.MiscFlags           = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

void BoxApp::BuildShaders()
{
	mShaders["standardVS"] = d3dUtil::CompileShader(L"FX\\color.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["standardPS"] = d3dUtil::CompileShader(L"FX\\color.hlsl", nullptr, "PS", "ps_5_0");

	md3dDevice->CreateVertexShader(
	                               // A pointer to the compiled shader
	                               mShaders["standardVS"]->GetBufferPointer(),
	                               // Size of the compiled vertex shader
	                               mShaders["standardVS"]->GetBufferSize(),
	                               // A pointer to a class linkage interface
	                               nullptr,
	                               // Address of a pointer to a ID3D11VertexShader interface
	                               &mVS);

	md3dDevice->CreatePixelShader(
	                              // A pointer to the compiled shader
	                              mShaders["standardPS"]->GetBufferPointer(),
	                              // Size of the compiled vertex shader
	                              mShaders["standardPS"]->GetBufferSize(),
	                              // A pointer to a class linkage interface
	                              nullptr,
	                              // Address of a pointer to a ID3D11VertexShader interface
	                              &mPS);
}

void BoxApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	mInputDesc = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(md3dDevice->CreateInputLayout(
		   mInputDesc.data(),
		   (UINT)mInputDesc.size(),
		   mShaders["standardVS"]->GetBufferPointer(),
		   mShaders["standardVS"]->GetBufferSize(),
		   &mInputLayout
	   ));
}
