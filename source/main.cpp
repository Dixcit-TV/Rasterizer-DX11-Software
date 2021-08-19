#include "pch.h"
//#undef main

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"
#include "PerspectiveCamera.h"
#include "ProjectSettings.h"
#include "SceneGraph.h"
#include "Mesh.h"
#include "Utils.h"
#include "TransparentDiffuseEffect.h"
#include "NormPhongEffect.h"
#include "ResourceManager.h"

void ShutDown(SDL_Window* pWindow)
{
	delete ResourceManager::GetInstance();
	delete ProjectSettings::GetInstance();
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void PrintControls();
void LoadResources(const std::unique_ptr<Elite::Renderer>& pRenderer);
void LoadScene(std::unique_ptr<SceneGraph>& pSceneGraph);
void UpdateRenderPipeline(std::unique_ptr<PerspectiveCamera>& pCamera, std::unique_ptr<SceneGraph>& pSceneGraph, const std::unique_ptr<Elite::Renderer>& pRenderer);

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;
	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - Thomas Vincent",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	int fps{};
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };
	auto pCamera{ std::make_unique<PerspectiveCamera>(Elite::FPoint3(-10.f, 5.f, 65.f), Elite::FVector3(0.f, 0.f, 1.f), float(width) / height, 60.f) };
	auto pSceneGraph{ std::make_unique<SceneGraph>() };
	LoadResources(pRenderer);
	LoadScene(pSceneGraph);

	UpdateRenderPipeline(pCamera, pSceneGraph, pRenderer);

	PrintControls();

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_F:
					ProjectSettings::GetInstance()->ToggleFiterMode();
					break;
				case SDL_SCANCODE_R:
					ProjectSettings::GetInstance()->ToggleRenderMode();
					UpdateRenderPipeline(pCamera, pSceneGraph, pRenderer);
					break;
				case SDL_SCANCODE_T:
					ProjectSettings::GetInstance()->ToggleTransparency();
					break;
				case SDL_SCANCODE_C:
					ProjectSettings::GetInstance()->ToggleCullMode();
					break;
				case SDL_SCANCODE_P:
					std::cout << "FPS: " << fps << std::endl;
					break;
				}
				break;
			case SDL_KEYUP:
				break;
			}
		}

		pCamera->Update(pTimer->GetElapsed());
		pSceneGraph->Update(pTimer->GetElapsed());

		//--------- Render ---------
		pRenderer->Render(pCamera, pSceneGraph);

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			fps = pTimer->GetFPS();
		}

	}
	pTimer->Stop();

	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}

void LoadResources(const std::unique_ptr<Elite::Renderer>& pRenderer)
{
	auto pDevice{ pRenderer->GetDevice() };
	ResourceManager::GetInstance()->Emplace_Texture("T_Vehicle_Diffuse", "Resources/vehicle_diffuse.png");
	ResourceManager::GetInstance()->Emplace_Texture("T_Vehicle_Normal", "Resources/vehicle_normal.png");
	ResourceManager::GetInstance()->Emplace_Texture("T_Vehicle_Specular", "Resources/vehicle_specular.png");
	ResourceManager::GetInstance()->Emplace_Texture("T_Vehicle_Gloss", "Resources/vehicle_gloss.png");
	ResourceManager::GetInstance()->Emplace_Texture("T_Fire_Diffuse", "Resources/fireFX_diffuse.png");


	ResourceManager::GetInstance()->AddEffect("MAT_VehicleShader", new NormPhongEffect(L"Resources/Shaders/PosCol3D.fx", ResourceManager::GetInstance()->GetTexture("T_Vehicle_Diffuse"), ResourceManager::GetInstance()->GetTexture("T_Vehicle_Normal"), ResourceManager::GetInstance()->GetTexture("T_Vehicle_Specular"), ResourceManager::GetInstance()->GetTexture("T_Vehicle_Gloss")));
	ResourceManager::GetInstance()->AddEffect("MAT_CombustionShader", new TransparentDiffuseEffect(pDevice, L"Resources/Shaders/PartCov3D.fx", ResourceManager::GetInstance()->GetTexture("T_Fire_Diffuse")));
}

void LoadScene(std::unique_ptr<SceneGraph>& pSceneGraph)
{
	Mesh* pVehicle{ new Mesh("Resources/vehicle.obj", ResourceManager::GetInstance()->GetEffect("MAT_VehicleShader"), CullMode::BACKFACE, Elite::MakeTranslation(Elite::FVector3(0.f, 0.f, 0.f))) };
	pSceneGraph->AddMesh(pVehicle);

	Mesh* pCombustion{ new Mesh("Resources/fireFX.obj", ResourceManager::GetInstance()->GetEffect("MAT_CombustionShader"), CullMode::NONE, Elite::MakeTranslation(Elite::FVector3(0.f, 0.f, 0.f))) };
	pSceneGraph->AddMesh(pCombustion);
}

void UpdateRenderPipeline(std::unique_ptr<PerspectiveCamera>& pCamera, std::unique_ptr<SceneGraph>& pSceneGraph, const std::unique_ptr<Elite::Renderer>& pRenderer)
{
	switch (ProjectSettings::GetInstance()->GetRenderMode())
	{
	case RenderMode::HARDWARE_RENDERING:
		pRenderer->InitDirectX();
		{
			auto* pDevice{ pRenderer->GetDevice() };

			ResourceManager::GetInstance()->LoadReseourcesToGPU(pDevice);
			pSceneGraph->LoadSceneOnGPU(pDevice);
			pCamera->SetCameraSystem(CameraSystem::LEFTHANDED);
		}
		break;
	case RenderMode::SOFTWARE_RENDERING:
		pCamera->SetCameraSystem(CameraSystem::RIGHTHANDED);
		pSceneGraph->ClearSceneFromGPU();
		ResourceManager::GetInstance()->ClearResourcesOnGPU();

		pRenderer->ClearDirectX();
		break;
	}
}

void PrintControls()
{
	std::cout << "----------------Rasterizer: Software/Hardware-----------------" << std::endl;
	std::cout << "Camera movements:" << std::endl;
	std::cout << "	- RMB + mouse movements: Rotate around X and Y axis" << std::endl;
	std::cout << "	- RMB + WASD: Move X/Z axis" << std::endl;
	std::cout << "	- RMB + LMB + mouse movements: Move X/Y axis" << std::endl;
	std::cout << "	- LMB + mouse movements: Rotate Y axis and Move Z axis" << std::endl;
	std::cout << "	- I - O: Zoom In - Out" << std::endl;
	std::cout << std::endl;

	std::cout << "Render Settings:" << std::endl;
	std::cout << "	- T: Toggle transparency on/off (both software and hardware)" << std::endl;
	std::cout << "	- C: Toggle culling MeshBased-NoCulling-BackCulling-FrontCulling (both software and hardware)" << std::endl;
	std::cout << "	- R: Toggle between Software and Hardware rasterizer" << std::endl;
	std::cout << "	- F: Toggle filtering mode (Hardware only)" << std::endl;
	std::cout << std::endl;

	std::cout << "Info:" << std::endl;
	std::cout << "	- P: show FPS" << std::endl;
}