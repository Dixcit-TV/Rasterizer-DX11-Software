/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>
#include "Enum.h"
#include "Struct.h"
#include <unordered_map>
#include <functional>

struct SDL_Window;
struct SDL_Surface;
class PerspectiveCamera;
class SceneGraph;

namespace Elite
{
	class Renderer final
	{
	public:
		explicit Renderer(SDL_Window* pWindow);
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;
		~Renderer();

		ID3D11Device* GetDevice() const { return m_pDXDevice; };
		void Render(const std::unique_ptr<PerspectiveCamera>& pCamera, const std::unique_ptr<SceneGraph>& pSceneGraph);

		bool InitDirectX();
		void ClearDirectX();

	private:
		std::unordered_map<RenderMode, std::function<void(const std::unique_ptr<PerspectiveCamera>&, const std::unique_ptr<SceneGraph>&)>> m_Renderers;

		//DirectX resources
		std::unordered_map<CullMode, ID3D11RasterizerState*> m_RasterizerStates{};
		ID3D11Device* m_pDXDevice;
		ID3D11DeviceContext* m_pDXDeviceContext;
		IDXGIFactory* m_pDXFactory;
		IDXGISwapChain* m_pDXSwapChain;
		ID3D11Texture2D* m_pDXDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDXDepthStencilView;
		ID3D11Resource* m_pDXRenderBufferTarget;
		ID3D11RenderTargetView* m_pDXRenderTargetView;

		//Software rasterizer resources
		std::vector<float> m_DepthBuffer{};
		SDL_Surface* m_pFrontBuffer;
		SDL_Surface* m_pBackBuffer;
		uint32_t* m_pBackBufferPixels;

		//Common Resources
		SDL_Window* m_pWindow;
		uint32_t m_Width;
		uint32_t m_Height;
		bool m_IsDXInitialized;

		void RenderDirectX(const std::unique_ptr<PerspectiveCamera>& pCamera, const std::unique_ptr<SceneGraph>& pSceneGraph);

		void RenderSoftware(const std::unique_ptr<PerspectiveCamera>& pCamera, const std::unique_ptr<SceneGraph>& pSceneGraph);
	};
}

#endif