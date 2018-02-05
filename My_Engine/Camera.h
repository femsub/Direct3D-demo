#pragma once
#include <d3d11_1.h>
#include <dinput.h>
#include <directxmath.h>
#include "LightHelper.h"
using namespace DirectX;
class Camera
{
	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;
	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;
	HWND& hwnd;
	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	float moveUpDown = 0.0f;
	float camYaw = 0.0f;
	float camPitch = 0.0f;
	int is_follow = false;
	XMFLOAT2 pad;
	XMVECTOR camPosition = XMVectorSet(0.0f, 2.0f, -5.0f, 0.0f);
	XMVECTOR camTarget = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	
public:
	Camera(HWND&);
	~Camera();
	XMMATRIX Default();
	HRESULT InitDirectInput(HINSTANCE);
	void DetectInput(double);
	void UpdateCamera(D3D_DRIVER_TYPE&	g_driverType,XMMATRIX& camView);
	const XMVECTOR& get_cam_pos();
	const XMVECTOR& get_cam_tar();
	void Release();
};

