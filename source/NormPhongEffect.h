#pragma once
#include "Effect.h"

class NormPhongEffect final : public Effect
{
public:
	explicit NormPhongEffect(const std::wstring& shaderPath, const Texture* pDiffuse, const Texture* pNormal, const Texture* pSpecular, const Texture* pGlossiness);

	NormPhongEffect(const NormPhongEffect& other) = delete;
	NormPhongEffect(NormPhongEffect&& other) noexcept = delete;
	NormPhongEffect& operator=(const NormPhongEffect& other) = delete;
	NormPhongEffect& operator=(NormPhongEffect&& other) = delete;
	virtual ~NormPhongEffect() override {};

	virtual void LoadToGPU(ID3D11Device* pDevice) override;

	void SetParameters(const Mesh* const pMesh, const std::unique_ptr<PerspectiveCamera>& pCam) override;

	virtual Elite::RGBColor PixelShading(const Vertex_Output& pixelInfo) const override;

private:
	const Texture* m_pNormalMap;
	const Texture* m_pSpecularMap;
	const Texture* m_pGlossinessMap;
};

