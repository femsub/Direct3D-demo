#include "stdafx.h"
#include "Snowman.h"



HRESULT Snowman::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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

Snowman::Snowman(ID3D11Device*&				g_pd3dDevice,
	ID3D11DeviceContext*&		g_pImmediateContext) :
	g_pd3dDevice(g_pd3dDevice),
	g_pImmediateContext(g_pImmediateContext)
{
	VBuffer.clear();
	InBuffer.clear();
}


Snowman::~Snowman()
{
}
void Snowman::ComputeNomal(XMFLOAT3& v1, XMFLOAT3& v2, XMFLOAT3& v3, XMFLOAT3& normal)
{
	XMFLOAT3 f1(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
	XMFLOAT3 f2(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);
	XMVECTOR vec1 = XMLoadFloat3(&f1);
	XMVECTOR vec2 = XMLoadFloat3(&f2);
	XMVECTOR temp = XMVector3Normalize(XMVector3Cross(vec1, vec2));
	XMStoreFloat3(&normal, temp);
}
HRESULT Snowman::Build()
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

	// Create the pixel shader soild
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);

	hr = CompileShaderFromFile((WCHAR *)L"shade.fx", "PS_SOILD", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader_soild);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Create vertex buffer
	hr = buildbody();
	if (FAILED(hr))
		return hr;
	hr = buildoth();
	if (FAILED(hr))
		return hr;


	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * VBuffer.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &VBuffer[0];
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	//UINT stride = sizeof(SimpleVertex);
	//UINT offset = 0;
	//g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Create index buffer

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DWORD) * InBuffer.size();        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &InBuffer[0];
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	//g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


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

void Snowman::Release() {
	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pTextureRV_body) g_pTextureRV_body->Release();
	if (g_pCBChangesEveryFrame) g_pCBChangesEveryFrame->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pPixelShader_soild) g_pPixelShader_soild->Release();
}

void Snowman::Render(D3D_DRIVER_TYPE&				g_driverType,
	XMMATRIX&					g_View,
	XMMATRIX&					g_Projection,
	bool ismove)
{
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Update our time
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
	XMMATRIX mOrbit0 = XMMatrixRotationY(t / 2.0f);
	XMMATRIX mOrbit = XMMatrixRotationY(-t / 2.0f);
	XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 1.0f, 0.0f);
	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	XMMATRIX g_box = XMMatrixTranslation(0.0f, 1.0f, 0.0f)*mScale * mOrbit0 * mTranslate * mOrbit;
	if (!ismove)g_box = XMMatrixIdentity();

	CBChangesEveryFrame cb;
	//cb.gMaterial.ambient = XMFLOAT4(0.1f, 0.2f, 0.3f, 1.0f);
	cb.gMaterial.ambient = XMFLOAT4(Colors::White);
	cb.gMaterial.diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 1.0f);
	cb.gMaterial.specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	
	for (rander x: randerlist) {
		std::pair<XMMATRIX, XMFLOAT4> w_c = x.f();
		XMMATRIX g_World = w_c.first;
		g_World *= g_box;
		cb.World = XMMatrixTranspose(g_World);
		cb.WorldViewProjection= XMMatrixTranspose(g_World * g_View*g_Projection);
		XMVECTOR det = XMMatrixDeterminant(cb.World);
		cb.WorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&det, cb.World));
		cb.vMeshColor = w_c.second;
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->VSSetShader(x.g_pVertexShader, nullptr, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShader(x.g_pPixelShader, nullptr, 0);
		g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV_body);
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		g_pImmediateContext->DrawIndexed(get<0>(x.t_size), get<1>(x.t_size), get<2>(x.t_size));
	}

}
HRESULT Snowman::buildbody() {
	// Load the Texture
	HRESULT hr;
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Snowman.dds", nullptr, &g_pTextureRV_body);
	if (FAILED(hr))
		return hr;

	DWORD slice = 50;

	std::vector<SimpleVertex> vbuffer((slice + 1)*(slice + 1));
	std::vector<DWORD> indices(slice * slice * 6);
	int idx = 0;
	auto cal_pos = [&](int x, int y)->XMFLOAT3 {
		if (x == 0)return XMFLOAT3(0, 0, 1);
		if (x == int(slice))return XMFLOAT3(0, 0, -1);
		if (y == int(slice))y = 0;
		XMMATRIX Rotationx = XMMatrixRotationX(float(x * (3.14 / slice)));
		XMMATRIX Rotationy = XMMatrixRotationZ(float(y * (6.28 / slice)));
		XMVECTOR currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
		XMFLOAT3 ans;
		XMStoreFloat3(&ans, currVertPos);
		return ans;
	};
	auto cal_tex = [&](int x, int y)->XMFLOAT2 {
		return XMFLOAT2((float)x / slice, (float)y / slice);
	};
	for (DWORD x = 0; x < slice + 1; x++)
		for (DWORD y = 0; y < slice + 1; y++) {
			vbuffer[x*(slice + 1) + y].Pos = vbuffer[x*(slice + 1) + y].Norm = cal_pos(x, y);
			vbuffer[x*(slice + 1) + y].Tex = cal_tex(x, y);
		}

	for (DWORD x = 0; x < slice; x++)
		for (DWORD y = 0; y < slice; y++) {
			indices[idx] = x * (slice + 1) + y;
			indices[idx + 1] = (x + 1) * (slice + 1) + y;
			indices[idx + 2] = x * (slice + 1) + y + 1;

			idx += 3;

			indices[idx] = x * (slice + 1) + y + 1;
			indices[idx + 1] = (x + 1) * (slice + 1) + y;
			indices[idx + 2] = (x + 1) * (slice + 1) + y + 1;

			idx += 3;
		}
	
	std::tuple<int, int, int>t_body = make_tuple(indices.size(), InBuffer.size(), VBuffer.size());
	VBuffer.insert(VBuffer.end(), vbuffer.begin(), vbuffer.end());
	InBuffer.insert(InBuffer.end(), indices.begin(), indices.end());

	rander body;
	body.t_size = t_body;
	body.f = [] ()->std::pair<XMMATRIX, XMFLOAT4> {
		return std::make_pair(XMMatrixTranslation(0.0f, 1.0f, 0.0f), XMFLOAT4(Colors::White));
	};
	body.g_pPixelShader = g_pPixelShader;
	body.g_pVertexShader = g_pVertexShader;
	randerlist.emplace_back(body);

	rander head;
	head.t_size = t_body;
	head.f = []()->std::pair<XMMATRIX, XMFLOAT4> {
		return std::make_pair(XMMatrixScaling(0.7f, 0.7f, 0.7f)*XMMatrixTranslation(0.0f, 2.5f, 0.0f), XMFLOAT4(Colors::White));
	};
	head.g_pPixelShader = g_pPixelShader;
	head.g_pVertexShader = g_pVertexShader;
	randerlist.emplace_back(head);
	
	float scale = 0.1f, height = 2.6f, dis = 0.3f, off = -0.6f;
	rander eye1;
	eye1.t_size = t_body;
	eye1.f = [=]()->std::pair<XMMATRIX, XMFLOAT4> {
		return std::make_pair(XMMatrixScaling(scale, scale, scale)*XMMatrixTranslation(dis, height, off), XMFLOAT4(Colors::Black));
	};
	eye1.g_pPixelShader = g_pPixelShader;
	eye1.g_pVertexShader = g_pVertexShader;
	randerlist.emplace_back(eye1);

	rander eye2;
	eye2.t_size = t_body;
	eye2.f = [=]()->std::pair<XMMATRIX, XMFLOAT4> {
		return std::make_pair(XMMatrixScaling(scale, scale, scale)*XMMatrixTranslation(-dis, height, off), XMFLOAT4(Colors::Black));
	};
	eye2.g_pPixelShader = g_pPixelShader;
	eye2.g_pVertexShader = g_pVertexShader;
	randerlist.emplace_back(eye2);

	rander mouth;
	mouth.t_size = t_body;
	mouth.f = [=]()->std::pair<XMMATRIX, XMFLOAT4> {
		return std::make_pair(XMMatrixScaling(0.08f, 0.08f, 0.4f)*XMMatrixTranslation(0.0f, 2.4f, -0.8f), XMFLOAT4(Colors::OrangeRed));
	};
	mouth.g_pPixelShader = g_pPixelShader;
	mouth.g_pVertexShader = g_pVertexShader;
	randerlist.emplace_back(mouth);

	return S_OK;
}

HRESULT Snowman::buildoth() {
	XMVECTOR st = XMVectorSet(0.5f, 1.3f, 0.0f, 1.0f), p = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR pf = XMVector3Normalize(XMVector3Orthogonal(p));
	float r = 0.07f;
	DWORD slice = 20;

	std::vector<SimpleVertex> vbuffer(slice * 4 * 3);
	std::vector<DWORD> indices(slice * 4 * 3);
	for (DWORD x = 0; x < slice * 4 * 3; x++)
		indices[x] = x;
	int idx = 0;
	std::vector<XMFLOAT3> bot(slice), top(slice);

	for (DWORD x = 0; x < slice; x++) {
		XMVECTOR t = st + r * XMVector3Transform(pf, XMMatrixRotationAxis(p, x*6.28f / slice));
		XMStoreFloat3(&bot[x], t);
		t = st + p + r * XMVector3Transform(pf, XMMatrixRotationAxis(p, x*6.28f / slice));
		XMStoreFloat3(&top[x], t);
	}
	for (DWORD x = 0; x < slice; x++) {
		XMStoreFloat3(&vbuffer[idx].Pos, st);
		XMStoreFloat3(&vbuffer[idx].Norm, XMVectorNegate(p));
		vbuffer[idx].Tex = XMFLOAT2(0.0f, 0.0f);

		vbuffer[idx + 1] = vbuffer[idx + 2] = vbuffer[idx];

		vbuffer[idx + 1].Pos = bot[(x + 1) % slice];
		vbuffer[idx + 2].Pos = bot[x];
		idx += 3;
	}
	for (DWORD x = 0; x < slice; x++) {
		XMStoreFloat3(&vbuffer[idx].Pos, st + p);
		XMStoreFloat3(&vbuffer[idx].Norm, p);
		vbuffer[idx].Tex = XMFLOAT2(1.0f, 1.0f);

		vbuffer[idx + 1] = vbuffer[idx + 2] = vbuffer[idx];

		vbuffer[idx + 1].Pos = top[x];
		vbuffer[idx + 2].Pos = top[(x + 1) % slice];
		idx += 3;
	}
	for (DWORD x = 0; x < slice; x++) {
		vbuffer[idx].Pos = bot[(x + 1) % slice];
		vbuffer[idx + 1].Pos = top[(x + 1) % slice];
		vbuffer[idx + 2].Pos = top[x];
		XMFLOAT3 tem;
		ComputeNomal(vbuffer[idx].Pos, vbuffer[idx + 1].Pos, vbuffer[idx + 2].Pos, tem);
		vbuffer[idx].Norm = vbuffer[idx + 1].Norm = vbuffer[idx + 2].Norm = tem;
		vbuffer[idx].Tex = XMFLOAT2(1.0f, (float)(x + 1) / slice);
		vbuffer[idx + 1].Tex = XMFLOAT2(0.0f, (float)(x + 1) / slice);
		vbuffer[idx + 2].Tex = XMFLOAT2(0.0f, (float)(x) / slice);
		idx += 3;

		vbuffer[idx].Pos = bot[x];
		vbuffer[idx + 1].Pos = bot[(x + 1) % slice];
		vbuffer[idx + 2].Pos = top[x];
		ComputeNomal(vbuffer[idx].Pos, vbuffer[idx + 1].Pos, vbuffer[idx + 2].Pos, tem);
		vbuffer[idx].Norm = vbuffer[idx + 1].Norm = vbuffer[idx + 2].Norm = tem;
		vbuffer[idx].Tex = XMFLOAT2(1.0f, (float)(x) / slice);
		vbuffer[idx + 1].Tex = XMFLOAT2(1.0f, (float)(x + 1) / slice);
		vbuffer[idx + 2].Tex = XMFLOAT2(0.0f, (float)(x) / slice);
		idx += 3;
	}
	std::tuple<int, int, int>t_hand = make_tuple(indices.size(), InBuffer.size(), VBuffer.size());
	VBuffer.insert(VBuffer.end(), vbuffer.begin(), vbuffer.end());
	InBuffer.insert(InBuffer.end(), indices.begin(), indices.end());

	rander hand1;
	hand1.t_size = t_hand;
	hand1.f = [=]()->std::pair<XMMATRIX, XMFLOAT4> {
		return std::make_pair(XMMatrixIdentity(), XMFLOAT4(200.0f / 255, 150.0f / 255, 36.0f / 255, 1.0f));
	};
	hand1.g_pPixelShader = g_pPixelShader_soild;
	hand1.g_pVertexShader = g_pVertexShader;
	randerlist.emplace_back(hand1);

	rander hand2;
	hand2.t_size = t_hand;
	hand2.f = [=]()->std::pair<XMMATRIX, XMFLOAT4> {
		return std::make_pair(XMMatrixReflect(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)), XMFLOAT4(200.0f / 255, 150.0f / 255, 36.0f / 255, 1.0f));
	};
	hand2.g_pPixelShader = g_pPixelShader_soild;
	hand2.g_pVertexShader = g_pVertexShader;
	randerlist.emplace_back(hand2);

	return S_OK;
}