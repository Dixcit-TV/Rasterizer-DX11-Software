#pragma once
#include <vector>
#include "Texture.h"
#include "Enum.h"

class Effect;
struct Vertex_Input;

class Mesh final
{
public:
	explicit Mesh(const std::string& objPath, Effect* const pEffect, CullMode cullMode = CullMode::BACKFACE, const Elite::FMatrix4& transform = Elite::FMatrix4::Identity());
	Mesh(const Mesh& other) = delete;
	Mesh(Mesh&& other) noexcept = delete;
	Mesh& operator=(const Mesh& other) = delete;
	Mesh& operator=(Mesh&& other) noexcept = delete;
	~Mesh();

	CullMode GetCullMode() const { return m_CullMode; }
	const Elite::FMatrix4& GetTransform() const { return m_Transform; }
	Effect* const GetEffect() const { return m_pEffect; }
	const std::vector<Vertex_Input>& GetVertices() const { return m_VertexBuffer; }
	const std::vector<uint32_t>& GetIndexes() const { return m_IndexBuffer;  }

	void LoadOnGPU(ID3D11Device* pDevice);
	void ClearGPUResources();
	void Update(float deltaT);
	void Render(ID3D11DeviceContext* pDeviceContext, FilterMode filterMode) const;

private:
	Elite::FMatrix4 m_Transform;
	Effect* const m_pEffect;
	ID3D11InputLayout* m_pDXVertexLayout;
	ID3D11Buffer* m_pDXVertexBuffer;
	ID3D11Buffer* m_pDXIndexBuffer;
	std::vector<Vertex_Input> m_VertexBuffer;
	std::vector<uint32_t> m_IndexBuffer;
	CullMode m_CullMode;

	bool m_IsLoadedOnGpu;
};

