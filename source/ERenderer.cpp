#include "pch.h"

//Project includes
#include "ERenderer.h"
#include "Mesh.h"
#include <vector>
#include "PerspectiveCamera.h"
#include "ProjectSettings.h"
#include "SceneGraph.h"
#include "Effect.h"
#include "Utils.h"

Elite::Renderer::Renderer(SDL_Window* pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_Renderers{}
	, m_IsDXInitialized{false}
{
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	m_pFrontBuffer = SDL_GetWindowSurface(m_pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_DepthBuffer = std::vector<float>(size_t(width) * size_t(height));

	m_Renderers.emplace(RenderMode::HARDWARE_RENDERING, std::bind(&Renderer::RenderDirectX, this, std::placeholders::_1, std::placeholders::_2));
	m_Renderers.emplace(RenderMode::SOFTWARE_RENDERING, std::bind(&Renderer::RenderSoftware, this, std::placeholders::_1, std::placeholders::_2));
}

Elite::Renderer::~Renderer()
{
	ClearDirectX();
}

void Elite::Renderer::ClearDirectX()
{
	for (auto& rasterStatePair : m_RasterizerStates)
		Utils::SafeRelease(rasterStatePair.second);

	Utils::SafeRelease(m_pDXRenderBufferTarget);
	Utils::SafeRelease(m_pDXRenderTargetView);
	Utils::SafeRelease(m_pDXDepthStencilBuffer);
	Utils::SafeRelease(m_pDXDepthStencilView);
	Utils::SafeRelease(m_pDXSwapChain);
	Utils::SafeRelease(m_pDXFactory);
	Utils::SafeRelease(m_pDXDevice);

	if (m_pDXDeviceContext)
	{
		m_pDXDeviceContext->ClearState();
		m_pDXDeviceContext->Flush();
		m_pDXDeviceContext->Release();
		m_pDXDeviceContext = nullptr;
	}

	m_IsDXInitialized = false;
}

void Elite::Renderer::Render(const std::unique_ptr<PerspectiveCamera>& pCamera, const std::unique_ptr<SceneGraph>& pSceneGraph)
{
	m_Renderers[ProjectSettings::GetInstance()->GetRenderMode()](pCamera, pSceneGraph);
}

void Elite::Renderer::RenderDirectX(const std::unique_ptr<PerspectiveCamera>& pCamera, const std::unique_ptr<SceneGraph>& pSceneGraph)
{
	if (!m_IsDXInitialized)
		return;

	//Clear Old Data
	RGBColor clearColor{ 0.f, 0.f, 0.3f };
	m_pDXDeviceContext->ClearRenderTargetView(m_pDXRenderTargetView, &clearColor.r);
	m_pDXDeviceContext->ClearDepthStencilView(m_pDXDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	//Render
	const std::vector<Mesh*>& pSceneMeshes{ pSceneGraph->GetMeshes() };
	FilterMode filter{ ProjectSettings::GetInstance()->GetFilterMode() };
	CullMode cullModeSettings{ ProjectSettings::GetInstance()->GetCullMode() };
	
	for (const Mesh* pMesh : pSceneMeshes)
	{
		CullMode cullMode = cullModeSettings == CullMode::MESHBASED ? pMesh->GetCullMode() : cullModeSettings;

		auto rasterState = m_RasterizerStates.find(cullMode);
		if (rasterState != m_RasterizerStates.end())
			m_pDXDeviceContext->RSSetState(rasterState->second);

		pMesh->GetEffect()->SetParameters(pMesh, pCamera);
		pMesh->Render(m_pDXDeviceContext, filter);
	}

	//Present
	m_pDXSwapChain->Present(0, 0);
}

void Elite::Renderer::RenderSoftware(const std::unique_ptr<PerspectiveCamera>& pCamera, const std::unique_ptr<SceneGraph>& pSceneGraph)
{
	m_DepthBuffer.assign(m_DepthBuffer.size(), FLT_MAX);
	SDL_FillRect(m_pBackBuffer, NULL, 0x606060);
	SDL_LockSurface(m_pBackBuffer);

	Elite::FMatrix4 worldProjectionViewMatrix{ };
	Elite::FMatrix4 projectionViewMatrix{ pCamera->GetProjectionMatrix() * pCamera->GetViewMatrix() };
	Elite::FPoint3 cameraPos{ pCamera->GetPosition() };
	const std::vector<Mesh*>& pSceneMeshes{ pSceneGraph->GetMeshes() };
	Vertex_Input triangleVertices[TRI_VERTEX_COUNT];
	Vertex_Output screenVertices[TRI_VERTEX_COUNT];
	CullMode cullModeSettings{ ProjectSettings::GetInstance()->GetCullMode() };

	for (const Mesh* const pMesh : pSceneMeshes)
	{
		CullMode cullMode = cullModeSettings == CullMode::MESHBASED ? pMesh->GetCullMode() : cullModeSettings;
		const Elite::FMatrix4& worldMatrix{ pMesh->GetTransform() };
		const auto& vertices{ pMesh->GetVertices() };
		const auto& indexes{ pMesh->GetIndexes() };
		const Effect* pMaterial{ pMesh->GetEffect() };
		bool useTransparency{ pMaterial->GetType() == MaterialType::TRANSPARENT_MATERIAL && ProjectSettings::GetInstance()->UseTransparency() };
		PrimitiveTopology topology{ PrimitiveTopology::TRIANGLELIST };
		worldProjectionViewMatrix = projectionViewMatrix * worldMatrix;

		const size_t indexCount{ indexes.size() };
		const size_t step{ size_t(topology) };

		for (size_t idx{}; idx <= indexCount - TRI_VERTEX_COUNT; idx += step)
		{
			//check if the triangle generated is valid (i.e. degenerate triangle aren't valid) or if frustrum culling applies, if it's the case, jump to the next triangle
			if (!Rasterizer::CreateTriangle(topology, vertices, indexes, idx, triangleVertices)
				|| !Rasterizer::ConvertVerticesWorldToScreenSpace(triangleVertices, screenVertices, worldProjectionViewMatrix, worldMatrix, cameraPos, m_Width, m_Height))
				continue;

			Aabb2D aabb{ Rasterizer::GetAabb2D(screenVertices, m_Width, m_Height) };
			//Loop over all the pixels in the aabb
			for (uint32_t r = aabb.bot; r < aabb.top; ++r)
			{
				for (uint32_t c = aabb.left; c < aabb.right; ++c)
				{
					Elite::FPoint2 pixelPosition{ float(c), float(r) };
					float w0, w1, w2;
					//Do the inside check for the current triangle and current pixel, returns the vertices weight as output parameters
					//if false is returned, we aren't inside the triangle
					if (!Rasterizer::IsPixelInTriangle(screenVertices, pixelPosition, cullMode, w0, w1, w2))
						continue;

					//interpolate z coordinates for depth testing
					uint32_t pixelIdx{ c + (r * m_Width) };
					float z = 1.f / (1.f / screenVertices[0].position.z * w0 + 1.f / screenVertices[1].position.z * w1 + 1.f / screenVertices[2].position.z * w2);
					if (z < m_DepthBuffer[pixelIdx])
					{
						Vertex_Output pixelInfo{ Rasterizer::GetInterpolatedPixelInfo(pixelPosition, screenVertices, z, w0, w1, w2) };
						Elite::RGBColor pixelColor{ pMaterial->PixelShading(pixelInfo) };

						//Hard coded transparency blending mode following DirectX setup: src_Color * src_alpha + dest_Color * inv_src_alpha
						if (useTransparency)
							pixelColor = pixelColor * pixelColor.a + GetColorFromSDL_ARGB(m_pBackBufferPixels[pixelIdx]) * (1 - pixelColor.a);
						else
							m_DepthBuffer[pixelIdx] = z;

						pixelColor.MaxToOne();
						m_pBackBufferPixels[pixelIdx] = GetSDL_ARGBColor(pixelColor);
					}
				}
			}
		}
	}

	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Elite::Renderer::InitDirectX()
{
	if (m_IsDXInitialized)
		return true;

	D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_0 };
	uint32_t createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	//Init device (Create resources) and deviceContext (set-up pipeline)
	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDXDevice, &featureLevel, &m_pDXDeviceContext);
	if (FAILED(result))
	{
		std::cout << "Couldn't create Device." << std::endl;
		return false;
	}

	//Init DXGIFactory to setup the swapChain
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXFactory));
	if (FAILED(result))
	{
		std::cout << "Couldn't create Factory." << std::endl;
		return false;
	}

	//Init buffer swapChain properties, create 2 buffers swap chain output
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate = { 1, 60 };
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;

	//Get handle to the render window from SDL
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//Init SwapChain
	result = m_pDXFactory->CreateSwapChain(m_pDXDevice, &swapChainDesc, &m_pDXSwapChain);
	if (FAILED(result))
	{
		std::cout << "Couldn't create SwapChain." << std::endl;
		return false;
	}

	//Init depth buffer properties as Read-Write 2D array 
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	//Init Depth Buffer 
	result = m_pDXDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDXDepthStencilBuffer);
	if (FAILED(result))
	{
		std::cout << "Couldn't create DepthBuffer." << std::endl;
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	//Init DepthBuffer view
	result = m_pDXDevice->CreateDepthStencilView(m_pDXDepthStencilBuffer , &depthStencilViewDesc, &m_pDXDepthStencilView);
	if (FAILED(result))
	{
		std::cout << "Couldn't create DepthBuffer View." << std::endl;
		return false;
	}

	//Get and cache swap chain buffer
	result = m_pDXSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pDXRenderBufferTarget));
	if (FAILED(result))
	{
		std::cout << "Couldn't get Render Target." << std::endl;
		return false;
	}

	//Init render view and bind Render buffer
	result = m_pDXDevice->CreateRenderTargetView(m_pDXRenderBufferTarget, 0, &m_pDXRenderTargetView);
	if (FAILED(result))
	{
		std::cout << "Couldn't create Render Target View." << std::endl;
		return false;
	}

	//Set Rasterizer State for counterClockwise Front faces
	D3D11_RASTERIZER_DESC rasterDesc{};
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = TRUE;
	result = m_pDXDevice->CreateRasterizerState(&rasterDesc, &m_RasterizerStates[CullMode::BACKFACE]);
	if (FAILED(result))
	{
		std::cout << "Couldn't create Rasterizer state for BackFace Culling." << std::endl;
		return false;
	}

	rasterDesc.CullMode = D3D11_CULL_NONE;
	result = m_pDXDevice->CreateRasterizerState(&rasterDesc, &m_RasterizerStates[CullMode::NONE]);
	if (FAILED(result))
	{
		std::cout << "Couldn't create Rasterizer state for No-Culling." << std::endl;
		return false;
	}

	rasterDesc.CullMode = D3D11_CULL_FRONT;
	result = m_pDXDevice->CreateRasterizerState(&rasterDesc, &m_RasterizerStates[CullMode::FRONTFACE]);
	if (FAILED(result))
	{
		std::cout << "Couldn't create Rasterizer state for FrontFace Culling." << std::endl;
		return false;
	}

	//Bind views to the output merger stage
	m_pDXDeviceContext->OMSetRenderTargets(1, &m_pDXRenderTargetView, m_pDXDepthStencilView);

	//Set-up view port, 1 taking the entire outputwindow (can be several, i.e. Co-Op)
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = float(m_Width);
	viewPort.Height = float(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDXDeviceContext->RSSetViewports(1, &viewPort);

	std::cout << "DirectX is ready\n";
	m_IsDXInitialized = true;

	return true;
}
