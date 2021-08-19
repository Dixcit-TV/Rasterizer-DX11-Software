#include "pch.h"
#include "ProjectSettings.h"
#include "ERenderer.h"

ProjectSettings* ProjectSettings::m_Instance = nullptr;

ProjectSettings* ProjectSettings::GetInstance()
{
	return m_Instance ? m_Instance : (m_Instance = new ProjectSettings());
}

ProjectSettings::~ProjectSettings()
{
	m_Instance = nullptr;
}