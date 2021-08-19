#pragma once
#include <vector>

class Mesh;

class SceneGraph
{
public:
	explicit SceneGraph() : m_pMeshes{} {};
	SceneGraph(const SceneGraph& other) = delete;
	SceneGraph(SceneGraph&& other) noexcept;
	SceneGraph& operator=(const SceneGraph& other) = delete;
	SceneGraph& operator=(SceneGraph&& other) noexcept;
	~SceneGraph();

	const std::vector<Mesh*>& GetMeshes() const;
	void AddMesh(Mesh* pNewMesh);

	void LoadSceneOnGPU(ID3D11Device* pDevice);
	void ClearSceneFromGPU();
	void Update(float deltaT);

private:
	std::vector<Mesh*> m_pMeshes;
};

