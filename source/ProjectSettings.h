#pragma once
#include "Enum.h"
#include "Utils.h"

class ProjectSettings
{
public:
	static ProjectSettings* GetInstance();

	ProjectSettings(const ProjectSettings&) = delete;
	ProjectSettings(ProjectSettings&&) noexcept = delete;
	ProjectSettings& operator=(const ProjectSettings&) = delete;
	ProjectSettings& operator=(ProjectSettings&&) noexcept = delete;
	~ProjectSettings();

	void ToggleFiterMode() { m_FilterMode = Utils::ToggleEnum(m_FilterMode); };
	void ToggleRenderMode() { m_RenderMode = Utils::ToggleEnum(m_RenderMode); };
	void ToggleCullMode() { m_CullMode = Utils::ToggleEnum(m_CullMode); };
	void ToggleTransparency() { m_UseTransparency = !m_UseTransparency; };
	FilterMode GetFilterMode() const { return m_FilterMode; };
	RenderMode GetRenderMode() const { return m_RenderMode; };
	CullMode GetCullMode() const { return m_CullMode; };
	bool UseTransparency() const { return m_UseTransparency; };

private:
	ProjectSettings()
		: m_FilterMode()
		, m_RenderMode(RenderMode::SOFTWARE_RENDERING)
		, m_CullMode(CullMode::MESHBASED)
		, m_UseTransparency(true)
	{};

	static ProjectSettings* m_Instance;
	RenderMode m_RenderMode;
	FilterMode m_FilterMode;
	CullMode m_CullMode;
	bool m_UseTransparency;
};