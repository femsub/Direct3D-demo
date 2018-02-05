#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <vector>
#include <tuple>
#include <functional>
#include <fstream>
#include "DDSTextureLoader.h"
#include "LightHelper.h"
using namespace std;
using namespace DirectX;


class Snowman
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
	struct rander {
		std::function < std::pair<XMMATRIX, XMFLOAT4>() > f;
		ID3D11VertexShader*			g_pVertexShader;
		ID3D11PixelShader*			g_pPixelShader;
		std::tuple<int, int, int>	t_size;
	};
	ID3D11Device*&				g_pd3dDevice;
	ID3D11DeviceContext*&		g_pImmediateContext;
	ID3D11VertexShader*			g_pVertexShader = nullptr;
	ID3D11PixelShader*			g_pPixelShader = nullptr;
	ID3D11PixelShader*			g_pPixelShader_soild = nullptr;
	ID3D11InputLayout*			g_pVertexLayout = nullptr;
	ID3D11Buffer*				g_pVertexBuffer = nullptr;
	ID3D11Buffer*				g_pIndexBuffer = nullptr;
	ID3D11ShaderResourceView*	g_pTextureRV_body = nullptr;
	
	ID3D11SamplerState*			g_pSamplerLinear = nullptr;
	ID3D11Buffer*				g_pCBChangesEveryFrame = nullptr;
	std::vector<SimpleVertex>	VBuffer;
	std::vector<DWORD>			InBuffer;
	std::vector<rander>			randerlist;

	HRESULT buildbody();
	HRESULT buildoth();
public:
	Snowman(ID3D11Device*&,
		ID3D11DeviceContext*&);
	~Snowman();
	HRESULT Build();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	void ComputeNomal(XMFLOAT3& v1, XMFLOAT3& v2, XMFLOAT3& v3, XMFLOAT3& normal);
	void Render(D3D_DRIVER_TYPE&, XMMATRIX&, XMMATRIX&, bool);
	void Release();
};

