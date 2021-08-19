#include "pch.h"
#include "Effect.h"
#include "Mesh.h"
#include "PerspectiveCamera.h"
#include <sstream>
#include "Utils.h"
#include "Texture.h"

Effect::Effect(const std::wstring& shaderPath, MaterialType matType, const Texture* pDiffuse)
	: m_ShaderPath(shaderPath)
	, m_pEffect(nullptr)
	, m_pDXPointTechnique(nullptr)
	, m_pDXLinearTechnique(nullptr)
	, m_pDXAnisotropicTechnique(nullptr)
	, m_pDXDefaultBlendState(nullptr)
	, m_pDXDefaultDepthState(nullptr)
	, m_pVariables()
	, m_pDiffuseMap(pDiffuse)
	, m_Type(matType)
{}

Effect::~Effect()
{
	ClearGPUResources();
}

void Effect::LoadToGPU(ID3D11Device* pDevice)
{
	m_pEffect = LoadEffect(pDevice, m_ShaderPath);

	if (m_pEffect)
	{
		m_pDXPointTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
		if (!m_pDXPointTechnique->IsValid())
			std::cout << L"Technique not valid: PointTechnique" << std::endl;

		m_pDXLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
		if (!m_pDXLinearTechnique->IsValid())
			std::cout << L"Technique not valid: LinearTechnique" << std::endl;

		m_pDXAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
		if (!m_pDXAnisotropicTechnique->IsValid())
			std::cout << L"Technique not valid: AnisotropicTechnique" << std::endl;

		m_pVariables.emplace("gWorldViewProjection", m_pEffect->GetVariableByName("gWorldViewProjection"));
		m_pVariables.emplace("gDiffuseMap", m_pEffect->GetVariableByName("gDiffuseMap"));

		SetResource("gDiffuseMap", m_pDiffuseMap->GetTextureResourceView());
	}
}

void Effect::ClearGPUResources()
{
	for (auto& pVarPair : m_pVariables)
	{
		Utils::SafeRelease(pVarPair.second);
	}

	m_pVariables.clear();

	Utils::SafeRelease(m_pDXDefaultDepthState);
	Utils::SafeRelease(m_pDXDefaultBlendState);
	Utils::SafeRelease(m_pDXPointTechnique);
	Utils::SafeRelease(m_pDXLinearTechnique);
	Utils::SafeRelease(m_pDXAnisotropicTechnique);
	Utils::SafeRelease(m_pEffect);
}

ID3DX11EffectTechnique* const Effect::GetTechnique(FilterMode filterMode) const
{
	switch (filterMode)
	{
	case FilterMode::POINT:
		return m_pDXPointTechnique;
	case FilterMode::LINEAR:
		return m_pDXLinearTechnique;
	case FilterMode::ANISOTROPIC:
		return m_pDXAnisotropicTechnique;
	}

	return m_pDXPointTechnique;
}

void Effect::SetParameters(const Mesh* const pMesh, const std::unique_ptr<PerspectiveCamera>& pCam)
{
	SetMatrix("gWorldViewProjection", (pCam->GetProjectionMatrix() * pCam->GetViewMatrix() * pMesh->GetTransform()).data[0]);
}

void Effect::SetMatrix(const std::string& paramName, const float* pData)
{
	auto paramPair = m_pVariables.find(paramName);
	if (paramPair != m_pVariables.end() && paramPair->second->IsValid())
		paramPair->second->AsMatrix()->SetMatrix(pData);
}

void Effect::SetResource(const std::string& paramName, ID3D11ShaderResourceView* pResourceView)
{
	auto paramPair = m_pVariables.find(paramName);
	if (paramPair != m_pVariables.end() && paramPair->second->IsValid())
		paramPair->second->AsShaderResource()->SetResource(pResourceView);
}

/// <summary>
/// Load and compile Effect for GPU rendering
/// </summary>
/// <param name="pDevice">Current DX device</param>
/// <param name="shaderPath">Path of shader file (.fx)</param>
/// <returns>Pointer to Effect</returns>
ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& shaderPath)
{
	HRESULT result{ S_OK };
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif 

	result = D3DX11CompileEffectFromFile(shaderPath.c_str()
		, nullptr
		, nullptr
		, shaderFlags
		, 0
		, pDevice
		, &pEffect
		, &pErrorBlob
	);

	if (FAILED(result))
	{
		if (pErrorBlob)
		{
			char* pErrors{ (char*)pErrorBlob->GetBufferPointer() };

			std::wstringstream ss;
			size_t eBufferSize{ pErrorBlob->GetBufferSize() };
			for (size_t i{}; i < eBufferSize; ++i)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			Utils::SafeRelease(pErrorBlob);

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath:" << shaderPath;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect;
}