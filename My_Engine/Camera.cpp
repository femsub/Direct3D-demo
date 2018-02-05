#include "stdafx.h"
#include "Camera.h"


Camera::Camera(HWND& hwnd)
	:hwnd(hwnd)
{
}


Camera::~Camera()
{
}

XMMATRIX Camera::Default()
{
	return XMMatrixLookAtLH(camPosition, camTarget, camUp);
}
const XMVECTOR& Camera::get_cam_pos()
{
	return camPosition;
}
const XMVECTOR& Camera::get_cam_tar()
{
	return camTarget;
}
HRESULT Camera::InitDirectInput(HINSTANCE hInstance)
{
	HRESULT hr = S_OK;
	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard,
		&DIKeyboard,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysMouse,
		&DIMouse,
		NULL);

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;
}
void Camera::DetectInput(double time)
{
	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];

	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	if (keyboardState[DIK_ESCAPE] & 0x80)
		PostMessage(hwnd, WM_DESTROY, 0, 0);

	float speed = float(15.0 * time);

	if (keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
		is_follow = false;
	}
	if (keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
		is_follow = false;
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;
		is_follow = false;
	}
	if (keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
		is_follow = false;
	}
	if (keyboardState[DIK_SPACE] & 0x80)
	{
		moveUpDown += speed*0.3f;
		is_follow = false;
	}
	if (keyboardState[DIK_LCONTROL] & 0x80)
	{
		moveUpDown -= speed*0.3f;
		is_follow = false;
	}
	if (keyboardState[DIK_F] & 0x80)
	{
		is_follow = true;
	}
	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseLastState.lX * 0.001f;

		camPitch += mouseCurrState.lY * 0.001f;

		mouseLastState = mouseCurrState;
	}

	//UpdateCamera();

	return;
}

void Camera::UpdateCamera(D3D_DRIVER_TYPE&	g_driverType, 
						XMMATRIX& camView)
{
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

	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
	camTarget = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);

	camRight = XMVector3TransformCoord(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), RotateYTempMatrix);
	//camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
	camUp = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), RotateYTempMatrix);
	camForward = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), RotateYTempMatrix);

	camPosition += moveLeftRight * camRight;
	camPosition += moveBackForward * camForward;
	camPosition += moveUpDown * camUp;
	if (is_follow) {
		camPosition = XMVectorSet(float(-4.0f*cos(t / 2.0f)),3.0f, float(-4.0f* sin(t / 2.0f)),1.0f);
	}
		

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	moveUpDown = 0.0f;

	camTarget = camPosition + camTarget;

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
}
void Camera::Release()
{
	DIKeyboard->Unacquire();
	DIMouse->Unacquire();
	DirectInput->Release();
}