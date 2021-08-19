#include "pch.h"
#include "Texture.h"
#include <SDL_image.h>
#include "Utils.h"

Texture::Texture(const std::string& texturePath)
	: m_pTexture{ nullptr }
	, m_pTextureView{ nullptr }
	, m_pTextureSurface{ IMG_Load(texturePath.c_str()) }
{}

Texture::~Texture()
{
	ClearGPUResources();
	SDL_FreeSurface(m_pTextureSurface);
}

void Texture::LoadToGPU(ID3D11Device* pDevice)
{
	//Set texture descriptor
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = m_pTextureSurface->w;
	texDesc.Height = m_pTextureSurface->h;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_pTextureSurface->pixels;
	initData.SysMemPitch = UINT(m_pTextureSurface->pitch);
	initData.SysMemSlicePitch = UINT(m_pTextureSurface->h * m_pTextureSurface->pitch);

	HRESULT result{ pDevice->CreateTexture2D(&texDesc, &initData, &m_pTexture) };
	if (FAILED(result))
	{
		std::cout << "Could not create Texture: " << std::endl;
	}
	else
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
		viewDesc.Format = texDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipLevels = texDesc.MipLevels;

		result = pDevice->CreateShaderResourceView(m_pTexture, &viewDesc, &m_pTextureView);
		if (FAILED(result))
			std::cout << "Could not create Texture View" << std::endl;
	}
}

void Texture::ClearGPUResources()
{
	Utils::SafeRelease(m_pTexture);
	Utils::SafeRelease(m_pTextureView);
}

/// <summary>
/// Clamped Sample pixel from texture
/// </summary>
/// <param name="uv">normalized pixel coordinate in space</param>
/// <returns>Pixel color</returns>
Elite::RGBColor Texture::Sample(const Elite::FVector2& uv) const
{

	uint32_t* pixels{ (uint32_t*)m_pTextureSurface->pixels };
	int col{ int(Elite::Clamp(uv.x, 0.f, 1.f) * m_pTextureSurface->w) };
	int row{ int(Elite::Clamp(uv.y, 0.f, 1.f) * m_pTextureSurface->h) };
	int pixelIndex{ row * m_pTextureSurface->w + col };
	uint8_t r, g, b, a;

	SDL_GetRGBA(pixels[pixelIndex], m_pTextureSurface->format, &r, &g, &b, &a);
	return Elite::RGBColor(float(r), float(g), float(b), float(a)) / 255.f;
}