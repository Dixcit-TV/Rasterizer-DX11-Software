#pragma once
#include <string>
#include "Enum.h"
#include <unordered_map>
#include "Struct.h"

class Mesh;
class PerspectiveCamera;
class Texture;

class Effect
{
public:
	Effect(const Effect& other) = delete;
	Effect(Effect&& other) noexcept = delete;
	Effect& operator=(const Effect& other) = delete;
	Effect& operator=(Effect&& other) = delete;
	virtual ~Effect();

	ID3DX11EffectTechnique* const GetTechnique(FilterMode filterMode = FilterMode::POINT) const;

	virtual ID3D11BlendState* GetBlendState() const { return m_pDXDefaultBlendState; };
	virtual ID3D11DepthStencilState* GetDepthState() const { return m_pDXDefaultDepthState; };
	virtual void SetParameters(const Mesh* const pMesh, const std::unique_ptr<PerspectiveCamera>& pCam);
	virtual Elite::RGBColor PixelShading(const Vertex_Output& pixelInfo) const = 0;

	MaterialType GetType() const { return m_Type; };

	virtual void LoadToGPU(ID3D11Device* pDevice);
	virtual void ClearGPUResources();

protected:
	explicit Effect(const std::wstring& shaderPath, MaterialType matType, const Texture* pDiffuse);

	std::unordered_map<std::string, ID3DX11EffectVariable*> m_pVariables;
	std::wstring m_ShaderPath;
	ID3DX11Effect* m_pEffect;
	ID3D11BlendState* m_pDXDefaultBlendState;
	ID3D11DepthStencilState* m_pDXDefaultDepthState;
	const Texture* m_pDiffuseMap;
	MaterialType m_Type;

	void SetMatrix(const std::string& paramName, const float* pData);
	void SetResource(const std::string& paramName, ID3D11ShaderResourceView* pResourceView);

private:
	ID3DX11EffectTechnique* m_pDXPointTechnique;
	ID3DX11EffectTechnique* m_pDXLinearTechnique;
	ID3DX11EffectTechnique* m_pDXAnisotropicTechnique;
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& shaderPath);
};
