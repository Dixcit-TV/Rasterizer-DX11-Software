#include "pch.h"
#include "TransparentDiffuseEffect.h"
#include "Texture.h"
#include "ProjectSettings.h"
#include "Utils.h"

TransparentDiffuseEffect::TransparentDiffuseEffect(ID3D11Device* pDevice, const std::wstring& shaderPath, const Texture* pDiffuse)
	: Effect(shaderPath, MaterialType::TRANSPARENT_MATERIAL, pDiffuse)
	, m_pDXNoBlendingState(nullptr)
	, m_pDXWriteEnabledDepthState(nullptr)
{
	if (m_pEffect)
	{
		D3D11_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].BlendEnable = FALSE;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		pDevice->CreateBlendState(&blendDesc, &m_pDXNoBlendingState);

		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		pDevice->CreateBlendState(&blendDesc, &m_pDXDefaultBlendState);

		D3D11_DEPTH_STENCIL_DESC depthDesc{};
		depthDesc.DepthEnable = true;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
		pDevice->CreateDepthStencilState(&depthDesc, &m_pDXWriteEnabledDepthState);

		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		pDevice->CreateDepthStencilState(&depthDesc, &m_pDXDefaultDepthState);
	}
}

TransparentDiffuseEffect::~TransparentDiffuseEffect()
{
	ClearGPUResources();
}

void TransparentDiffuseEffect::LoadToGPU(ID3D11Device* pDevice)
{
	Effect::LoadToGPU(pDevice);
	if (m_pEffect)
	{
		D3D11_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].BlendEnable = FALSE;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		pDevice->CreateBlendState(&blendDesc, &m_pDXNoBlendingState);

		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		pDevice->CreateBlendState(&blendDesc, &m_pDXDefaultBlendState);

		D3D11_DEPTH_STENCIL_DESC depthDesc{};
		depthDesc.DepthEnable = true;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
		pDevice->CreateDepthStencilState(&depthDesc, &m_pDXWriteEnabledDepthState);

		D3D11_DEPTH_STENCIL_DESC noWritedepthDesc{};
		noWritedepthDesc.DepthEnable = true;
		noWritedepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		noWritedepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
		pDevice->CreateDepthStencilState(&noWritedepthDesc, &m_pDXDefaultDepthState);
	}
}

void TransparentDiffuseEffect::ClearGPUResources()
{
	Effect::ClearGPUResources();
	Utils::SafeRelease(m_pDXNoBlendingState);
	Utils::SafeRelease(m_pDXWriteEnabledDepthState);
}

/// <summary>
/// Pixel shading
/// </summary>
/// <param name="pixelInfo">Interpolated pixel information</param>
/// <returns>Final pixel color</returns>
Elite::RGBColor TransparentDiffuseEffect::PixelShading(const Vertex_Output& pixelInfo) const
{
	return m_pDiffuseMap ? m_pDiffuseMap->Sample(pixelInfo.uv) : Elite::RGBColor{};
}

ID3D11BlendState* TransparentDiffuseEffect::GetBlendState() const
{
	return ProjectSettings::GetInstance()->UseTransparency() ? m_pDXDefaultBlendState : m_pDXNoBlendingState;
}

ID3D11DepthStencilState* TransparentDiffuseEffect::GetDepthState() const
{
	return ProjectSettings::GetInstance()->UseTransparency() ? m_pDXDefaultDepthState : m_pDXWriteEnabledDepthState;
}