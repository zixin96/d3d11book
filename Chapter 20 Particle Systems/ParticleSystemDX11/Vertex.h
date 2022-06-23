//***************************************************************************************
// Vertex.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines vertex structures and input layouts.
//***************************************************************************************

#ifndef VERTEX_H
#define VERTEX_H

#include "d3dUtil.h"

namespace Vertex
{
	// Basic 32-byte vertex structure.
	struct Basic32
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Tex;
	};

	struct Terrain
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT2 Tex;
		DirectX::XMFLOAT2          BoundsY;
	};

	struct Particle
	{
		DirectX::XMFLOAT3 InitialPos;
		DirectX::XMFLOAT3 InitialVel;
		DirectX::XMFLOAT2 Size;
		float Age;
		unsigned int Type;
	};
}

class InputLayoutDesc
{
public:
	// Init like const int A::a[4] = {0, 1, 2, 3}; in .cpp file.
	static const D3D11_INPUT_ELEMENT_DESC Pos[1];
	static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
	static const D3D11_INPUT_ELEMENT_DESC Terrain[3];
	static const D3D11_INPUT_ELEMENT_DESC Particle[5];
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* Pos;
	static ID3D11InputLayout* Basic32;
	static ID3D11InputLayout* Terrain;
	static ID3D11InputLayout* Particle;
};

#endif // VERTEX_H
