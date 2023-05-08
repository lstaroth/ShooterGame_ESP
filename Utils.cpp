#include "pch.h"
#include "Utils.h"

#define M_PI 3.14159265358979323846

extern GameScreen ScreenInfo;

FVector::FVector(float* v)
{
	x = v[0];
	y = v[1];
	z = v[2];
}

FVector::FVector(float f_x, float f_y, float f_z)
{
	x = f_x;
	y = f_y;
	z = f_z;
}

float FVector::distance(FVector v)
{
	return float(sqrt(pow((v.x - x), 2) + pow((v.y - y), 2) + pow((v.z - z), 2)));
}

float FVector::Dot(const FVector& v) const
{
	return (x * v.x + y * v.y + z * v.z);
}

FVector FVector::operator-(FVector v) const
{
	return FVector(x - v.x, y - v.y, z - v.z);
}

FMatrix RotatorToMatrix(FRotator rotation) {
	float radPitch = rotation.Pitch * ((float)M_PI / 180.0f);
	float radYaw = rotation.Yaw * ((float)M_PI / 180.0f);
	float radRoll = rotation.Roll * ((float)M_PI / 180.0f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	FMatrix matrix;

	matrix.M[0][0] = (CP * CY);
	matrix.M[0][1] = (CP * SY);
	matrix.M[0][2] = (SP);
	matrix.M[0][3] = 0;

	matrix.M[1][0] = (SR * SP * CY - CR * SY);
	matrix.M[1][1] = (SR * SP * SY + CR * CY);
	matrix.M[1][2] = (-SR * CP);
	matrix.M[1][3] = 0;

	matrix.M[2][0] = (-(CR * SP * CY + SR * SY));
	matrix.M[2][1] = (CY * SR - CR * SP * SY);
	matrix.M[2][2] = (CR * CP);
	matrix.M[2][3] = 0;

	matrix.M[3][0] = 0;
	matrix.M[3][1] = 0;
	matrix.M[3][2] = 0;
	matrix.M[3][3] = 1;

	return matrix;
}

std::tuple<float, float> WorldToScreen(FVector* world, FMinimalViewInfo& camera)
{
	FMatrix tempMatrix = RotatorToMatrix(camera.Rotation);

	FVector vAxisX = FVector(tempMatrix.M[0][0], tempMatrix.M[0][1], tempMatrix.M[0][2]);
	FVector vAxisY = FVector(tempMatrix.M[1][0], tempMatrix.M[1][1], tempMatrix.M[1][2]);
	FVector vAxisZ = FVector(tempMatrix.M[2][0], tempMatrix.M[2][1], tempMatrix.M[2][2]);

	FVector vDelta = *world - camera.Location;
	FVector vTransformed = FVector(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.0f) {
		vTransformed.z = 1.0f;
	}

	float ScreenCenterX = ScreenInfo.windowWidth / 2.0f;
	float ScreenCenterY = ScreenInfo.windowHeight / 2.0f;

	return std::make_tuple(
		(ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(camera.FOV * ((float)M_PI / 360.0f))) / vTransformed.z),
		(ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(camera.FOV * ((float)M_PI / 360.0f))) / vTransformed.z)
	);
}

HWND FindTopGameWindow()
{
	HWND FirstHwnd = nullptr;
	BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
		{
			if (DWORD processId; GetWindowThreadProcessId(hwnd, &processId) && processId == GetCurrentProcessId())
			{
				*reinterpret_cast<HWND*>(lParam) = hwnd;
				return FALSE;
			}
			return TRUE;
		}, (LPARAM)&FirstHwnd);

	return FirstHwnd;
}