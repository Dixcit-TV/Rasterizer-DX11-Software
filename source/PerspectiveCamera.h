#pragma once
#include "EMath.h"
#include "Enum.h"

class PerspectiveCamera
{
public:
	explicit PerspectiveCamera() = default;
	explicit PerspectiveCamera(const Elite::FPoint3& position, const Elite::FVector3& nForward, float aspectRatio, float fovAngleDeg = 45.f, float nearClip = 0.1f, float farClip = 1000.f);
	~PerspectiveCamera() = default;

	const Elite::FMatrix4& GetONB() const { return m_LookAtMatrix; };
	Elite::FMatrix4 GetViewMatrix() const { return Elite::Inverse(m_LookAtMatrix); }
	const Elite::FMatrix4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	float GetFOV() const { return m_FOV; }
	float GetAspectRatio() const { return m_AspectRatio; }
	Elite::FPoint3 GetPosition() const { return Elite::FPoint3(m_LookAtMatrix[3].xyz); }
	void Update(float deltaT);

	void SetCameraSystem(CameraSystem newSystem);

private:
	Elite::FMatrix4 m_LookAtMatrix;
	Elite::FMatrix4 m_ProjectionMatrix;
	float m_AspectRatio;
	float m_FOV;
	float m_NearClip;
	float m_FarClip;
	int m_Orientation;
	CameraSystem m_pCamSystem;

	void Zoom(float zoomOffset);
	void Translate(const Elite::FVector3& offset);
	void RotateCamera(float yaw, float pitch, float roll);

	void UpdateProjectionMatrix();
};

