#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "LightHelper.h"
using namespace DirectX;

class Box
{
	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Norm;
		XMFLOAT2 Tex;
	};

	struct CBChangesEveryFrame
	{
		XMMATRIX WorldViewProjection;
		XMMATRIX World;
		XMMATRIX WorldInvTranspose;
		XMFLOAT4 vMeshColor;
		Material gMaterial;
	};
	ID3D11Device*&				g_pd3dDevice;
	ID3D11DeviceContext*&		g_pImmediateContext;
	ID3D11VertexShader*			g_pVertexShader=nullptr;
	ID3D11PixelShader*			g_pPixelShader = nullptr;
	ID3D11InputLayout*			g_pVertexLayout = nullptr;
	ID3D11Buffer*				g_pVertexBuffer = nullptr;
	ID3D11Buffer*				g_pIndexBuffer = nullptr;
	ID3D11ShaderResourceView*	g_pTextureRV = nullptr;
	ID3D11SamplerState*			g_pSamplerLinear = nullptr;
	ID3D11Buffer*				g_pCBChangesEveryFrame = nullptr;
public:
	Box(ID3D11Device*&,
	ID3D11DeviceContext*&);
	~Box();
	HRESULT Build();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	void Render(D3D_DRIVER_TYPE&,XMMATRIX&,XMMATRIX&);
	void Release();

};

