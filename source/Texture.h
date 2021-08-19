#pragma once
#include <string>
#include <SDL_surface.h>
#include "ERGBColor.h"
#include "EMath.h"

class Texture final
{
public:
	explicit Texture(const std::string& texturePath);
	Texture(const Texture& texture) = delete;
	Texture(Texture&& texture) noexcept = delete;
	Texture& operator=(const Texture& texture) = delete;
	Texture& operator=(Texture&& texture) noexcept = delete;
	~Texture();

	ID3D11ShaderResourceView* GetTextureResourceView() const { return m_pTextureView; };

	Elite::RGBColor Sample(const Elite::FVector2& uv) const;

	void LoadToGPU(ID3D11Device* pDevice);
	void ClearGPUResources();

private:
	SDL_Surface* m_pTextureSurface;
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pTextureView;
};

