#include "pch.h"
#include "SceneGraph.h"
#include "Mesh.h"
#include "Utils.h"

SceneGraph::SceneGraph(SceneGraph&& other) noexcept
	: m_pMeshes{ std::move(other.m_pMeshes) }
{
	other.m_pMeshes.clear();
}

SceneGraph& SceneGraph::operator=(SceneGraph&& other) noexcept
{
	for (Mesh* pMesh : m_pMeshes)
		Utils::SafeDelete(pMesh);

	m_pMeshes.clear();

	m_pMeshes = std::move(other.m_pMeshes);

	return *this;
}

SceneGraph::~SceneGraph()
{
	for (Mesh* pMesh : m_pMeshes)
		Utils::SafeDelete(pMesh);

	m_pMeshes.clear();
}

const std::vector<Mesh*>& SceneGraph::GetMeshes() const
{
	return m_pMeshes;
}

void SceneGraph::AddMesh(Mesh* pNewMesh)
{
	m_pMeshes.push_back(pNewMesh);
}

void SceneGraph::LoadSceneOnGPU(ID3D11Device* pDevice)
{
	for (Mesh* pMesh : m_pMeshes)
		pMesh->LoadOnGPU(pDevice);
}

void SceneGraph::ClearSceneFromGPU()
{
	for (Mesh* pMesh : m_pMeshes)
		pMesh->ClearGPUResources();
}

void SceneGraph::Update(float)
{
	//for (Mesh* pMesh : m_pMeshes)
	//	pMesh->Update(deltaT);
}