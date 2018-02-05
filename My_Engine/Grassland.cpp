#include "stdafx.h"
#include "Grassland.h"





HRESULT Grassland::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

Grassland::Grassland(ID3D11Device*&				g_pd3dDevice,
	ID3D11DeviceContext*&		g_pImmediateContext) :
	g_pd3dDevice(g_pd3dDevice),
	g_pImmediateContext(g_pImmediateContext)
{
}


Grassland::~Grassland()
{
}
void Grassland::ComputeNomal(XMFLOAT3& v1, XMFLOAT3& v2, XMFLOAT3& v3, XMFLOAT3& normal)
{
	XMFLOAT3 f1(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
	XMFLOAT3 f2(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);
	XMVECTOR vec1 = XMLoadFloat3(&f1);
	XMVECTOR vec2 = XMLoadFloat3(&f2);
	XMVECTOR temp = XMVector3Normalize(XMVector3Cross(vec1, vec2));
	XMStoreFloat3(&normal, temp);
}
HRESULT Grassland::Build()
{
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile((WCHAR *)L"shade.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	//g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	//顶点布局

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile((WCHAR *)L"shade.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Create vertex buffer
	const int n = 100, m = 100;
	const float Leng = 100.0f;
	std::vector<SimpleVertex> Ivbuffer(n*m);
	for (int x = 0; x < n; x++)
		for (int y = 0; y < m; y++) {
			Ivbuffer[x*m + y].Pos = XMFLOAT3(-Leng /2 + y * Leng / m, 0.0f, Leng/2 - x * Leng / n);
			XMVECTOR pos = XMLoadFloat3(&Ivbuffer[x*m + y].Pos);
			float dis = (XMVectorGetX(XMVector3Length(pos))- Leng / 8)/3;
			if (dis > 0)
				Ivbuffer[x*m + y].Pos.y = float(sin(dis)*dis*sin(x/10)*sin(y/10)*2);// float(rand() / 65535.0 * 10);
			Ivbuffer[x*m + y].Norm = XMFLOAT3(0.0f, 1.0f, 0.0f);
			Ivbuffer[x*m + y].Tex = XMFLOAT2((float)x/5 , (float)y/5 );
		}
	

	// Set vertex buffer
	//UINT stride = sizeof(SimpleVertex);
	//UINT offset = 0;
	//g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Create index buffer
	std::vector<SimpleVertex> vbuffer(n*m*6);
	idxsz = (n - 1)*(m - 1) * 2 * 3;
	std::vector<WORD> indices((n - 1)*(m - 1) * 2 * 3);
	int idx = 0;
	for (WORD x = 0; x < (n - 1)*(m - 1) * 2 * 3; x++)
		indices[x] = x;
	for (WORD x = 0; x < n - 1; x++)
		for (WORD y = 0; y < m - 1; y++) {
			vbuffer[idx] = Ivbuffer[x * m + y];
			vbuffer[idx + 1] = Ivbuffer[(x + 1)*m + (y + 1)];
			vbuffer[idx + 2] = Ivbuffer[(x + 1)*m + (y)];
			XMFLOAT3 tem;
			ComputeNomal(vbuffer[idx].Pos, vbuffer[idx+1].Pos, vbuffer[idx+2].Pos, tem);
			vbuffer[idx].Norm = vbuffer[idx+1].Norm = vbuffer[idx+2].Norm = tem;

			vbuffer[idx + 3] = Ivbuffer[(x)*m + (y)];
			vbuffer[idx + 4] = Ivbuffer[(x)*m + (y + 1)];
			vbuffer[idx + 5] = Ivbuffer[(x + 1)*m + (y + 1)];
			ComputeNomal(vbuffer[idx+3].Pos, vbuffer[idx + 4].Pos, vbuffer[idx + 5].Pos, tem);
			vbuffer[idx+3].Norm = vbuffer[idx + 4].Norm = vbuffer[idx + 5].Norm = tem;
			idx += 6;
		}
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * n * m * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &vbuffer[0];
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;


	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * idxsz;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &indices[0];
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	//g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Load the Texture
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"seafloor.dds", nullptr, &g_pTextureRV);
	if (FAILED(hr))
		return hr;

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	// Create the ChangesEveryFrame buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame);
	if (FAILED(hr))
		return hr;
	//顶点缓存
	return hr;
}

void Grassland::Release() {
	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pTextureRV) g_pTextureRV->Release();
	if (g_pCBChangesEveryFrame) g_pCBChangesEveryFrame->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
}

void Grassland::Render(D3D_DRIVER_TYPE&				g_driverType,
	XMMATRIX&					g_View,
	XMMATRIX&					g_Projection)
{
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	static float t = 0.0f;
	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;
	}

	XMMATRIX g_World = XMMatrixIdentity();

	//
	// Update variables for the first cube
	//
	CBChangesEveryFrame cb;
	cb.World = XMMatrixTranspose(g_World);
	cb.WorldViewProjection = XMMatrixTranspose(g_World * g_View*g_Projection);
	XMVECTOR det = XMMatrixDeterminant(cb.World);
	cb.WorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&det, cb.World));
	cb.vMeshColor = XMFLOAT4(Colors::White);
	cb.gMaterial.ambient = XMFLOAT4(Colors::White);
	//cb.gMaterial.ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	cb.gMaterial.diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	cb.gMaterial.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
	
	
	//
	// Render the first cube
	//
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->DrawIndexed(idxsz, 0, 0);

}
