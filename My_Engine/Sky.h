#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <vector>
#include "DDSTextureLoader.h"
#include "LightHelper.h"
using namespace DirectX;
class Sky
{
	struct Vertex
	{
		XMFLOAT3 pos;
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
	ID3D11VertexShader*			g_pVertexShader = nullptr;
	ID3D11PixelShader*			g_pPixelShader = nullptr;
	ID3D11InputLayout*			g_pVertexLayout = nullptr;
	ID3D11Buffer*				g_pVertexBuffer = nullptr;
	ID3D11Buffer*				g_pIndexBuffer = nullptr;
	ID3D11ShaderResourceView*	g_pTextureRV = nullptr;
	ID3D11SamplerState*			g_pSamplerLinear = nullptr;
	ID3D11Buffer*				g_pCBChangesEveryFrame = nullptr;
	ID3D11RasterizerState*		CCWcullMode = nullptr;
	ID3D11RasterizerState*		CWcullMode = nullptr;
	ID3D11DepthStencilState*	DSLessEqual = nullptr;
	ID3D11RasterizerState*		RSCullNone = nullptr;
	XMMATRIX					g_World;
	int							NumSphereFaces;
	int							NumSphereVertices;

public:
	Sky(ID3D11Device*&, ID3D11DeviceContext*&);
	~Sky();
	HRESULT Build(int, int);
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	void UpdateScene(const XMVECTOR&);
	void Render(XMMATRIX&, XMMATRIX&);
	void Release();
};

