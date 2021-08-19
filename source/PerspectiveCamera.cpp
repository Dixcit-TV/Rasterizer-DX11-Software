#include "pch.h"
#include "PerspectiveCamera.h"
#include "SDL.h"
#include "Utils.h"
#include "Quaternion.h"
#include "ProjectSettings.h"

PerspectiveCamera::PerspectiveCamera(const Elite::FPoint3& position, const Elite::FVector3& nForward, float aspectRatio, float fovAngleDeg, float nearClip, float farClip)
	: m_LookAtMatrix()
	, m_ProjectionMatrix()
	, m_AspectRatio(aspectRatio)
	, m_FOV(tanf(fovAngleDeg* float(E_TO_RADIANS) / 2))
	, m_NearClip(nearClip)
	, m_FarClip(farClip)
	, m_pCamSystem(CameraSystem::RIGHTHANDED)
	, m_Orientation()
{
	m_Orientation = (m_pCamSystem == CameraSystem::RIGHTHANDED) * 1 + (m_pCamSystem == CameraSystem::LEFTHANDED) * -1;
	Elite::FVector3 camForward{ nForward.x * m_Orientation, nForward.y * m_Orientation, nForward.z };
	Elite::FVector3 right{ Cross(Utils::GetWorldY<float>(), camForward) };
	Normalize(right);
	Elite::FVector3 up{ Cross(camForward, right) };

	m_LookAtMatrix = Elite::FMatrix4(right, up, camForward, Elite::FPoint4(position.x, position.y, position.z * m_Orientation));

	UpdateProjectionMatrix();
}

void PerspectiveCamera::Update(float deltaT)
{
	Elite::FVector3 offset{};
	const float linearVelocity{ 50.f };
	const float zommSensitivity{ 1.f };
	const float mouseSensitivity{ 0.3f };

	float roll{ 0.f }, pitch{ 0.f };
	int zoom{};
	bool updated{ false };

	int mouseOffsetX{ 0 }, mouseOffsetY{ 0 };
	Uint32 mouseRelativeSate{ SDL_GetRelativeMouseState(&mouseOffsetX, &mouseOffsetY) };
	const Uint8* pScanCode{ SDL_GetKeyboardState(NULL) };
	bool mouseLeft{ bool(mouseRelativeSate & SDL_BUTTON(SDL_BUTTON_LEFT)) };
	bool mouseRight{ bool(mouseRelativeSate & SDL_BUTTON(SDL_BUTTON_RIGHT)) };

	if (pScanCode[SDL_SCANCODE_I] || pScanCode[SDL_SCANCODE_O])
	{
		updated = true;
		zoom = pScanCode[SDL_SCANCODE_O] * 1 + pScanCode[SDL_SCANCODE_I] * -1;
	}

	if (mouseLeft || mouseRight)
	{
		if (mouseOffsetY || mouseOffsetX)
		{
			float offsetAngle{ float(atan2(mouseOffsetY, mouseOffsetX)) };
			if (mouseLeft && mouseRight)
			{
				updated = true;
				offset.x += cosf(offsetAngle) * linearVelocity;
				offset.y += sinf(offsetAngle) * -linearVelocity;
			}
			else if (mouseLeft)
			{
				updated = true;
				offset.z += sin(offsetAngle) * linearVelocity;
				pitch -= mouseOffsetX;
			}
			else if (mouseRight)
			{
				updated = true;
				roll -= mouseOffsetY;
				pitch -= mouseOffsetX;
			}
		}

		Elite::FVector3 direction{ (pScanCode[SDL_SCANCODE_A] || pScanCode[SDL_SCANCODE_LEFT]) * -1.f + (pScanCode[SDL_SCANCODE_D] || pScanCode[SDL_SCANCODE_RIGHT]) * 1.f
								, (pScanCode[SDL_SCANCODE_S] || pScanCode[SDL_SCANCODE_DOWN]) * -1.f + (pScanCode[SDL_SCANCODE_W] || pScanCode[SDL_SCANCODE_UP]) * 1.f };

		if (direction != Elite::FVector3::ZeroVector())
		{
			updated = true;
			float offsetAngle{ atan2f(direction.y, direction.x) };
			offset.x += cosf(offsetAngle) * linearVelocity;
			offset.z += sinf(offsetAngle) * -linearVelocity;
		}
	}

	if (updated)
	{
		if (zoom != 0)
		{
			Zoom(zoom * zommSensitivity * deltaT);
		}

		if (roll || pitch)
		{
			RotateCamera(roll * m_Orientation * mouseSensitivity, pitch * m_Orientation * mouseSensitivity, 0.f);
		}

		if (offset != Elite::FVector3::ZeroVector())
		{
			offset *= deltaT;
			offset.z *= m_Orientation;
			Translate(offset);
		}
	}
}

void PerspectiveCamera::Zoom(float zoomOffset)
{
	//10º fov angle
	const float minFOV{ 5.f * float(E_TO_RADIANS) };
	const float maxFOV{ float(E_PI) };

	m_FOV += zoomOffset;
	if (m_FOV < minFOV)
		m_FOV = minFOV;

	if (m_FOV > maxFOV)
		m_FOV = maxFOV;

	UpdateProjectionMatrix();
}

inline void PerspectiveCamera::Translate(const Elite::FVector3& offset)
{
	m_LookAtMatrix *= MakeTranslation(offset);
}

inline void PerspectiveCamera::RotateCamera(float roll, float pitch, float)
{
	float rollRad{ roll * float(E_TO_RADIANS) }, pitchRad{ pitch * float(E_TO_RADIANS) };

	Rotate(m_LookAtMatrix, Quaternion<float>(pitchRad, Utils::GetWorldY<float>()) * Quaternion<float>(rollRad, Elite::FVector3(m_LookAtMatrix[0])));
}

inline void PerspectiveCamera::UpdateProjectionMatrix()
{
	m_ProjectionMatrix[0] = Elite::FVector4(1.f / (m_AspectRatio * m_FOV), 0.f, 0.f, 0.f);
	m_ProjectionMatrix[1] = Elite::FVector4(0.f, 1.f / m_FOV, 0.f, 0.f);

	if (m_pCamSystem == CameraSystem::RIGHTHANDED)
	{
		m_ProjectionMatrix[2] = Elite::FVector4(0.f, 0.f, m_FarClip / (m_NearClip - m_FarClip), -1.f);
		m_ProjectionMatrix[3] = Elite::FVector4(0.f, 0.f, (m_FarClip * m_NearClip) / (m_NearClip - m_FarClip), 0.f);
	}
	else
	{
		m_ProjectionMatrix[2] = Elite::FVector4(0.f, 0.f, m_FarClip / (m_FarClip - m_NearClip), 1.f);
		m_ProjectionMatrix[3] = Elite::FVector4(0.f, 0.f, -(m_FarClip * m_NearClip) / (m_FarClip - m_NearClip), 0.f);
	}
}

void PerspectiveCamera::SetCameraSystem(CameraSystem newSystem)
{
	if (newSystem == m_pCamSystem)
		return;

	//Flip camera + orientation (used to determine x/y input axis)
	m_pCamSystem = newSystem;
	m_Orientation *= -1;
	auto mI = Elite::FMatrix4::Identity();
	mI(2, 2) = -1;
	m_LookAtMatrix = mI * m_LookAtMatrix * mI;

	UpdateProjectionMatrix();
}