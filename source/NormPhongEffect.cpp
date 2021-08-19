#include "pch.h"
#include "NormPhongEffect.h"
#include "Mesh.h"
#include "PerspectiveCamera.h"
#include "Texture.h"

NormPhongEffect::NormPhongEffect(const std::wstring& shaderPath, const Texture* pDiffuse, const Texture* pNormal, const Texture* pSpecular, const Texture* pGlossiness)
	: Effect(shaderPath, MaterialType::OPAQUE_MATERIAL, pDiffuse)
	, m_pNormalMap(pNormal)
	, m_pSpecularMap(pSpecular)
	, m_pGlossinessMap(pGlossiness)
{}

void NormPhongEffect::LoadToGPU(ID3D11Device* pDevice)
{
	Effect::LoadToGPU(pDevice);
	if (m_pEffect)
	{
		D3D11_BLEND_DESC noBlendDesc{};
		noBlendDesc.RenderTarget[0].BlendEnable = FALSE;
		noBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		pDevice->CreateBlendState(&noBlendDesc, &m_pDXDefaultBlendState);

		D3D11_DEPTH_STENCIL_DESC depthDesc{};
		depthDesc.DepthEnable = true;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
		pDevice->CreateDepthStencilState(&depthDesc, &m_pDXDefaultDepthState);

		m_pVariables.emplace("gCameraMatrix", m_pEffect->GetVariableByName("gCameraMatrix"));
		m_pVariables.emplace("gWorldMatrix", m_pEffect->GetVariableByName("gWorldMatrix"));
		m_pVariables.emplace("gNormalMap", m_pEffect->GetVariableByName("gNormalMap"));
		m_pVariables.emplace("gSpecularMap", m_pEffect->GetVariableByName("gSpecularMap"));
		m_pVariables.emplace("gGlossinessMap", m_pEffect->GetVariableByName("gGlossinessMap"));

		SetResource("gNormalMap", m_pNormalMap->GetTextureResourceView());
		SetResource("gSpecularMap", m_pSpecularMap->GetTextureResourceView());
		SetResource("gGlossinessMap", m_pGlossinessMap->GetTextureResourceView());
	}
}

void NormPhongEffect::SetParameters(const Mesh* const pMesh, const std::unique_ptr<PerspectiveCamera>& pCam)
{
	Effect::SetParameters(pMesh, pCam);
	SetMatrix("gCameraMatrix", pCam->GetONB().data[0]);
	SetMatrix("gWorldMatrix", pMesh->GetTransform().data[0]);
}

Elite::RGBColor NormPhongEffect::PixelShading(const Vertex_Output& pixelInfo) const
{
	Elite::RGBColor pixelColor{}, diffuseColor{}, specularColor{};
	const DirectionalLight dirLight{ Elite::RGBColor{ 1.f, 1.f, 1.f }, Elite::FVector3{.577f, -.577f, -.577f}, 5.f };
	const float shininess{ 25.f };
	Elite::FVector3 normal{ pixelInfo.normal };

	if (m_pNormalMap)
	{
		Elite::FMatrix3 tSpaceAxis{ pixelInfo.tangent, Elite::Cross(pixelInfo.tangent, pixelInfo.normal), pixelInfo.normal };
		Elite::RGBColor normalSample{ (m_pNormalMap->Sample(pixelInfo.uv)) };
		normal = tSpaceAxis * Elite::FVector3(2.f * normalSample.r - 1.f, 2.f * normalSample.g - 1.f, 2.f * normalSample.b - 1.f);
		Normalize(normal);
	}

	float nDotL{ Elite::Dot(-normal, dirLight.nDirection) };

	pixelColor = dirLight.color * dirLight.intensity * Elite::Clamp(nDotL, 0.f, 1.f);
	float diffuseStrength = Elite::Clamp(nDotL, 0.f, 1.f) * dirLight.intensity / float(E_PI);
	diffuseColor = m_pDiffuseMap ? m_pDiffuseMap->Sample(pixelInfo.uv) : diffuseColor;
	diffuseColor *= diffuseStrength;

	if (m_pSpecularMap && m_pGlossinessMap)
	{
		Elite::RGBColor specularReflectance{ m_pSpecularMap->Sample(pixelInfo.uv) };
		float rDv{ Elite::Dot(dirLight.nDirection - (nDotL + nDotL) * -normal, pixelInfo.viewVector) };
		rDv = Elite::Clamp(rDv, 0.f, 1.f);
		specularColor = specularReflectance * pow(rDv, shininess * m_pGlossinessMap->Sample(pixelInfo.uv).r);
	}

	pixelColor = diffuseColor + specularColor;

	return pixelColor;
}