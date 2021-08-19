#pragma once
#include "Effect.h"

class TransparentDiffuseEffect final : public Effect
{
public:
	explicit TransparentDiffuseEffect(ID3D11Device* pDevice, const std::wstring& shaderPath, const Texture* pDiffuse);

	TransparentDiffuseEffect(const TransparentDiffuseEffect& other) = delete;
	TransparentDiffuseEffect(TransparentDiffuseEffect&& other) noexcept = delete;
	TransparentDiffuseEffect& operator=(const TransparentDiffuseEffect& other) = delete;
	TransparentDiffuseEffect& operator=(TransparentDiffuseEffect&& other) = delete;
	virtual ~TransparentDiffuseEffect() override;

	virtual void LoadToGPU(ID3D11Device* pDevice) override;
	virtual void ClearGPUResources() override;

	virtual Elite::RGBColor PixelShading(const Vertex_Output& pixelInfo) const override;
	virtual ID3D11BlendState* GetBlendState() const override;
	virtual ID3D11DepthStencilState* GetDepthState() const override;

private:
	ID3D11BlendState* m_pDXNoBlendingState;
	ID3D11DepthStencilState* m_pDXWriteEnabledDepthState;
};

