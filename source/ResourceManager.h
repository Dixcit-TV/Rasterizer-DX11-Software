#pragma once
#include <unordered_map>
#include "Texture.h"

class Effect;

class ResourceManager
{
public:
	static ResourceManager* GetInstance();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager(ResourceManager&&) noexcept = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;
	ResourceManager& operator=(ResourceManager&&) noexcept = delete;
	~ResourceManager();

	void AddEffect(const std::string& tag, Effect* pEffect);
	void Emplace_Texture(const std::string& tag, const std::string& texturePath);

	Effect* GetEffect(const std::string& tag) const;
	const Texture* GetTexture(const std::string& tag) const;

	void LoadReseourcesToGPU(ID3D11Device* pDevice);
	void ClearResourcesOnGPU();

private:
	ResourceManager() : m_Materials{}, m_TextureMaps{}{};

	static ResourceManager* m_Instance;

	std::unordered_map<std::string, Effect*> m_Materials;
	std::unordered_map<std::string, Texture> m_TextureMaps;
};

