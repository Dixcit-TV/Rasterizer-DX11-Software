#include "pch.h"
#include <tuple>
#include "ResourceManager.h"
#include "Effect.h"
#include "Utils.h"

ResourceManager* ResourceManager::m_Instance = nullptr;

ResourceManager* ResourceManager::GetInstance()
{
	return m_Instance ? m_Instance : (m_Instance = new ResourceManager());
}

ResourceManager::~ResourceManager()
{
	for (auto& matPair : m_Materials)
		Utils::SafeDelete(matPair.second);

	m_Materials.clear();
	m_TextureMaps.clear();

	m_Instance = nullptr;
}

void ResourceManager::AddEffect(const std::string& tag, Effect* pEffect)
{
	m_Materials.emplace(tag, pEffect);
}

void ResourceManager::Emplace_Texture(const std::string& tag, const std::string& texturePath)
{
	m_TextureMaps.emplace(std::piecewise_construct, std::forward_as_tuple(tag), std::forward_as_tuple(texturePath));
}

Effect* ResourceManager::GetEffect(const std::string& tag) const
{
	auto effectPair{ m_Materials.find(tag) };
	if (effectPair != m_Materials.end())
		return effectPair->second;

	std::cout << tag << " material doesn't exist." << std::endl;
	return nullptr;
}

const Texture* ResourceManager::GetTexture(const std::string& tag) const
{
	auto texturePair{ m_TextureMaps.find(tag) };
	if (texturePair != m_TextureMaps.end())
		return &texturePair->second;

	std::cout << tag << " texture doesn't exist." << std::endl;
	return nullptr;
}

void ResourceManager::LoadReseourcesToGPU(ID3D11Device* pDevice)
{
	for (auto& texturePair : m_TextureMaps)
		texturePair.second.LoadToGPU(pDevice);

	for (auto& matPair : m_Materials)
		matPair.second->LoadToGPU(pDevice);
}

void ResourceManager::ClearResourcesOnGPU()
{
	for (auto& matPair : m_Materials)
		matPair.second->ClearGPUResources();

	for (auto& texturePair : m_TextureMaps)
		texturePair.second.ClearGPUResources();
}