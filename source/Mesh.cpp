#include "pch.h"
#include "Mesh.h"
#include "Struct.h"
#include "Effect.h"
#include "Quaternion.h"
#include "Utils.h"

Mesh::Mesh(const std::string& objPath, Effect* const pEffect, CullMode cullMode, const Elite::FMatrix4& transform)
	: m_Transform{ transform }
	, m_pEffect{ pEffect }
	, m_pDXVertexLayout{ nullptr }
	, m_pDXVertexBuffer{ nullptr }
	, m_pDXIndexBuffer{ nullptr }
	, m_VertexBuffer{ }
	, m_IndexBuffer{ }
	, m_CullMode{ cullMode }
	, m_IsLoadedOnGpu{ false }
{
	ObjReader::LoadModel(objPath, m_VertexBuffer, m_IndexBuffer);
}

Mesh::~Mesh()
{
	ClearGPUResources();
}

void Mesh::Update(float deltaT)
{
	Rotate(m_Transform, Quaternion<float>(deltaT, Utils::GetWorldY<float>()));
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext, FilterMode filterMode) const
{
	if (!m_IsLoadedOnGpu)
		return;

	UINT stride{ sizeof(Vertex_Input) };
	UINT offset{ 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pDXVertexBuffer, &stride, &offset);

	pDeviceContext->IASetIndexBuffer(m_pDXIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	pDeviceContext->IASetInputLayout(m_pDXVertexLayout);

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pDeviceContext->OMSetBlendState(m_pEffect->GetBlendState(), blendFactor, 0xffffffff);
	pDeviceContext->OMSetDepthStencilState(m_pEffect->GetDepthState(), 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	auto pTechnique{ m_pEffect->GetTechnique(filterMode) };
	pTechnique->GetDesc(&techDesc);

	UINT indexCount{ UINT(m_IndexBuffer.size()) };
	for (UINT p{ 0 }; p < techDesc.Passes; ++p)
	{
		pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(indexCount, 0, 0);
	}
}

void Mesh::LoadOnGPU(ID3D11Device* pDevice)
{
	//Set Input Layout
	HRESULT result{ S_OK };
	const uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "NORMAL";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TANGENT";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TEXCOORD";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	D3DX11_PASS_DESC passDesc;
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(vertexDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pDXVertexLayout);
	if (FAILED(result))
		return;

	//Create VertexBuffer
	D3D11_BUFFER_DESC vBufferDesc{};
	vBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vBufferDesc.ByteWidth = sizeof(Vertex_Input) * uint32_t(m_VertexBuffer.size());
	vBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBufferDesc.CPUAccessFlags = 0;
	vBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = std::data(m_VertexBuffer);

	result = pDevice->CreateBuffer(&vBufferDesc, &initData, &m_pDXVertexBuffer);
	if (FAILED(result))
		return;

	//Create IndexBuffer
	D3D11_BUFFER_DESC iBufferDesc{};
	iBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	iBufferDesc.ByteWidth = sizeof(uint32_t) * uint32_t(m_IndexBuffer.size());
	iBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBufferDesc.CPUAccessFlags = 0;
	iBufferDesc.MiscFlags = 0;
	initData.pSysMem = std::data(m_IndexBuffer);

	result = pDevice->CreateBuffer(&iBufferDesc, &initData, &m_pDXIndexBuffer);
	if (FAILED(result))
		return;

	m_IsLoadedOnGpu = true;
}

void Mesh::ClearGPUResources()
{
	if (m_IsLoadedOnGpu)
	{
		Utils::SafeRelease(m_pDXVertexLayout);
		Utils::SafeRelease(m_pDXVertexBuffer);
		Utils::SafeRelease(m_pDXIndexBuffer);
	}
}