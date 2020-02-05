#include "glm/gtx/euler_angles.hpp"
#include "glm/glm.hpp"
#include "ClientApplication.h"
// rendering:
#include "resource_loading/UILoader.h"
#include "resource_loading/GLTFLoader.h"
#include "rendering/Shader.h"
#if defined(PLATFORM_WINDOWS)
#include "rendering/dx/DX12Renderer.h"
#include "rendering/dx/DX12Application.h"
#include "brofiler/Brofiler.h"
#include "rendering/dx/render_passes/DxForwardPass.h"
#include "rendering/unified_shader_passes/ParticlePass.h"
#include "rendering/dx/render_passes/DxSkyboxPass.h"
#include "rendering/dx/render_passes/DxVignettePass.h"
#include "rendering/dx/render_passes/DxMipmapPass.h"
#include "rendering/dx/DxParticleRenderer.h"
#include "rendering/dx/render_passes/DxBlurPass.h"
#include "rendering/dx/render_passes/DxMergeBloomMipsPass.h"
#elif defined(PLATFORM_PS)
#include "rendering/gnm/PS4Renderer.h"
#include "Rendering/gnm/RenderPasses/PS4MainPass.h"
#include "rendering/gnm/RenderPasses/PS4VignettePass.h"
#include "rendering/gnm/RenderPasses/PS4MipMapPass.h"
#include "rendering/gnm/RenderPasses/PS4BlurPass.h"
#include "rendering/gnm/RenderPasses/PS4BloomPass.h"
#include "rendering/gnm/RenderPasses/PS4ParticlePass.h"
#include "rendering/gnm/RenderPasses/PS4SkyboxPass.h"
#include <kernel.h>
#include <fios2.h>
#include <mat.h>
#endif
#include "rendering/Camera.h"
// other systems:
#include "utils/EncryptUtils.h"
#include "ui/UISystem.h"
#include "resource_loading/RestApiParsing.h"

#include "gameplay/WaypointSystem.h"
#include "EffectChangeHandler.h"
#include "resource_loading/ArgumentParsing.h"
#include "rendering/dx/DxParticleRenderer.h"
#include "rendering/unified_shader_passes/ParticlePass.h"
#include "rendering/gnm/RenderPasses/PS4UIPass.h"
#include "rendering/GeometricShapes.h"

#ifdef PLATFORM_PS
size_t sceLibcHeapSize = 256 * 1024 * 1024;

/*E The FIOS2 default maximum path is 1024, games can normally use a much smaller value. */
#define MAX_PATH_LENGTH 256


/*E Buffers for FIOS2 initialization.
 * These are typical values that a game might use, but adjust them as needed. They are
 * of type int64_t to avoid alignment issues. */

 /* 64 ops: */
int64_t g_OpStorage[SCE_FIOS_DIVIDE_ROUNDING_UP(SCE_FIOS_OP_STORAGE_SIZE(64, MAX_PATH_LENGTH), sizeof(int64_t))];
/* 1024 chunks, 64KiB: */
int64_t g_ChunkStorage[SCE_FIOS_DIVIDE_ROUNDING_UP(SCE_FIOS_CHUNK_STORAGE_SIZE(1024), sizeof(int64_t))];
/* 16 file handles: */
int64_t g_FHStorage[SCE_FIOS_DIVIDE_ROUNDING_UP(SCE_FIOS_FH_STORAGE_SIZE(16, MAX_PATH_LENGTH), sizeof(int64_t))];
/* 1 directory handle: */
int64_t g_DHStorage[SCE_FIOS_DIVIDE_ROUNDING_UP(SCE_FIOS_DH_STORAGE_SIZE(1, MAX_PATH_LENGTH), sizeof(int64_t))];
#endif

void ClientApplication::Initialize(int argc, char *argv[])
{
	skyboxFile = "OutputCube_1024.dds";
#ifdef PLATFORM_PS
	SceFiosParams params = SCE_FIOS_PARAMS_INITIALIZER;

	/*E Provide required storage buffers. */
	params.opStorage.pPtr = g_OpStorage;
	params.opStorage.length = sizeof(g_OpStorage);
	params.chunkStorage.pPtr = g_ChunkStorage;
	params.chunkStorage.length = sizeof(g_ChunkStorage);
	params.fhStorage.pPtr = g_FHStorage;
	params.fhStorage.length = sizeof(g_FHStorage);
	params.dhStorage.pPtr = g_DHStorage;
	params.dhStorage.length = sizeof(g_DHStorage);

	params.pathMax = MAX_PATH_LENGTH;

	params.pVprintf = vprintf;
	params.pMemcpy = memcpy;

	int fiosInitStatus = sceFiosInitialize(&params);
	ASSERT(fiosInitStatus == SCE_OK);
#endif

	IApplication::Initialize(argc, argv);
	loginHandler->Initialize(&database, &m_renderer->GetGfxResourceManager());
	game.Initialize(gameEQ, lobbyEQ);
	gameClient.Initialize(gameEQ);
	lobbyClient.Initialize(lobbyEQ, this);

	audioSystem.Initialize();
	uiSystem = new ui::UISystem(m_renderer->GetUIRenderer(), m_renderer->GetGfxResourceManager(), audioSystem, [this](ptl::shared_ptr<ui::UIText> text)
	{
		m_input->OpenInputDialog(text->ObsecureText(), text->GetRawText());
	});

	ui.Initialize(lobbyEQ, uiEQ, uiSystem);

	deltaTime.Initialize();
	SetUpRenderingPipeline();
	camera.set_Projection(32.5, 1280.f / 720.f, 1.0f, 2000.f);

	sceneHandler.Initialize();
	sceneHandler.SetRenderer(m_renderer);

	BindUIControlls();
	BindLoginEvents();

	uiEQ->appendListener(UIInteractionEvent::Quit, [](std::function<void* ()>)
	{
		IApplication::m_ShouldStop = true;
	});

	uiEQ->appendListener(UIInteractionEvent::OnAudioToggle,[this](std::function<void* ()>)
	{
		if(audioSystem.IsPaused())
		{
			uiSystem->SetImageForElementsOfTag("MusicButton.png", "MusicButton");
			audioSystem.ResumeMusic();
		}else
		{
			uiSystem->SetImageForElementsOfTag("MusicButton-Clicked.png", "MusicButton");
			audioSystem.PauseMusic();
		}
	});

}

void ClientApplication::BindUIControlls()
{
	m_input->AddListener(BINDACTION(&ui::UISystem::OnClick, uiSystem), tbsg::InputType::EMouse_Left);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnDown, uiSystem), tbsg::InputType::Down);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnUp, uiSystem), tbsg::InputType::Up);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnLeft, uiSystem), tbsg::InputType::Left);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnRight, uiSystem), tbsg::InputType::Right);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnClick, uiSystem), tbsg::InputType::Enter);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnBackspace, uiSystem), tbsg::InputType::Back);

	m_input->AddListener(BINDACTION(&UserInterface::SkipSplashScreen, &ui), tbsg::InputType::Space);
	m_input->AddListener(BINDACTION(&UserInterface::SkipSplashScreen, &ui), tbsg::InputType::EGamepad_Face_Button_Down);
	m_input->AddListener(BINDACTION(&UserInterface::OnPauseScreen, &ui), tbsg::InputType::Escape);
	m_input->AddListener(BINDACTION(&UserInterface::OnPauseScreen, &ui), tbsg::InputType::EGamepad_Special_Right);

	m_input->AddListener(BINDACTION(&ui::UISystem::OnDown, uiSystem), tbsg::InputType::EGamepad_DPad_Down);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnUp, uiSystem), tbsg::InputType::EGamepad_DPad_Up);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnLeft, uiSystem), tbsg::InputType::EGamepad_DPad_Left);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnRight, uiSystem), tbsg::InputType::EGamepad_DPad_Right);
	m_input->AddListener(BINDACTION(&ui::UISystem::OnClick, uiSystem), tbsg::InputType::EGamepad_Face_Button_Down);

	m_input->AddListener(BINDAXIS(&ui::UISystem::OnAxisHorizontal, uiSystem), tbsg::InputType::EGamepad_Left_Thumb_X);
	m_input->AddListener(BINDAXIS(&ui::UISystem::OnAxisVertical, uiSystem), tbsg::InputType::EGamepad_Left_Thumb_Y);

	m_input->AddListener(BINDCHARACTER(&ui::UISystem::OnCharacterTyped, uiSystem));
	m_input->AddListener(BINDTEXTDIALOG(&ui::UISystem::OnTextDialogResult, uiSystem));

}

void ClientApplication::BindLoginEvents()
{
	//On login screen load, check for local access token
	if (loginHandler->CheckForLocalAccessToken())
	{
		if (loginHandler->CheckIfAccessTokenValid())
		{
			lobbyClient.SetCurrentUserName(loginHandler->GetCurrentUserName());
			uiEQ->enqueue(UIInteractionEvent::OnValidTokenFound, nullptr);
			ui.SetCurrentUserName(loginHandler->GetCurrentUserName());
			uiEQ->enqueue(UIInteractionEvent::OnValidTokenFound, nullptr);
		}
	}


	// EVENTS 
	uiEQ->appendListener(UIInteractionEvent::OnLoginStatus, [this](std::function<void* ()> data)
		{
			auto pair = *static_cast<std::pair<ptl::string, ptl::string>*>(data());

			if (loginHandler->LoginUser(pair.first, pair.second))
			{
				lobbyClient.SetCurrentUserName(pair.first);
				ui.SetCurrentUserName(pair.first);
				loginHandler->WriteNewAccessToken();
				uiEQ->enqueue(UIInteractionEvent::OnLoadGameData, nullptr);
			}
			else
			{

				uiEQ->enqueue(UIInteractionEvent::OnToggleLoadingScreen, nullptr);
				uiSystem->Dialog("Invalid email or password.\nPlease try again!");
				uiSystem->PushUI("Login.ui");
			}
		});

	// EVENTS 
	uiEQ->appendListener(UIInteractionEvent::OnLoadGameData, [this](std::function<void* ()>)
		{
			lobbyClient.SetNetworkAccessToken(loginHandler->GetCurrentAccessToken());
			gameClient.SetNetworkAccessToken(loginHandler->GetCurrentAccessToken());
			if (loginHandler->LoadGameData())
			{
				if (loginHandler->GetUsersDecks())
				{
					this->ConnectToLobbyServer();
					uiEQ->enqueue(UIInteractionEvent::OnSwitchToMainMenu, nullptr);
#ifdef _WIN32

					SetConsoleTitleA(("(User: " + loginHandler->GetCurrentUserName() + ")").c_str());
#endif
					
				}
				else
				{
					uiSystem->PushUI("Login.ui");
					uiEQ->enqueue(UIInteractionEvent::OnToggleLoadingScreen, nullptr);

					uiSystem->Dialog("Failed to process required Game Data, check your internet connection");
				}
			}
			else
			{
				uiSystem->PushUI("Login.ui");
				uiEQ->enqueue(UIInteractionEvent::OnToggleLoadingScreen, nullptr);

				uiSystem->Dialog("Failed to process required Game Data, check your internet connection");
			}
		});

	uiEQ->appendListener(UIInteractionEvent::OnRequestDeckData, [this](std::function<void* ()>)
		{
			auto decks = database.GetDeckList();
			ptl::vector<ui::DeckDisplayInformation> deckDisplayItems{};
			for (auto& deck : decks)
			{
				if (!deck.cards.empty())
				{
					ui::DeckDisplayInformation deckInfo;
					deckInfo.deckName = deck.name;
					deckInfo.deckID = deck.id;
					deckInfo.coverCardId = deck.cards[0]->meta.name;
					deckDisplayItems.push_back(deckInfo);
				}
			}
			//No check for empty here as this should be done at UI to display messages.
			uiEQ->enqueue(UIInteractionEvent::OnParseDecksData, [deckDisplayItems]() mutable
				{
					return static_cast<void*>(&deckDisplayItems);
				});
		});


		gameEQ->appendListener(GameEvent::OnServerConnect,[this](Packet)
		{
			if (lobbyClient.IsConnected())
			{
				printf("Disconnecting from LobbyServer...\n");
				lobbyClient.Disconnect();
			}

		});

	lobbyEQ->appendListener(LobbyEvent::OnServerConnect,[this](std::function<void* ()>)
	{
		if (gameClient.IsConnected())
		{
			printf("Disconnecting from GameServer...\n");
			gameClient.Disconnect();
		}
	});

}

void ClientApplication::Run()
{
	utils::InitEncrypt();
	IApplication::Run();

	std::function<void()> frameIndepenedentUpdate = [this]()
	{
		//memes
	};

	static int counter = 0;
	glm::ivec2 cursorPos;
	// *** Frame Loop *** //


	while (!IApplication::m_ShouldStop)
	{
#ifdef PLATFORM_WINDOWS
		BROFILER_FRAME("Frame");
#elif PLATFORM_PS
		sceMatNewFrame();
#endif

		deltaTime.Update();
		audioSystem.Update();
		deltaTime.FrameIndependentUpdate(frameIndepenedentUpdate);

		// *** Process Events *** //
		gameEQ->process();
		lobbyEQ->process();

		uiEQ->process();

		m_input->Update();
		m_input->GetCursorPosition(cursorPos.x, cursorPos.y);

		ui.Update(deltaTime.GetDeltaTime());
		uiSystem->Update(deltaTime, cursorPos);

		gameClient.Update();
		lobbyClient.Update();
		game.Update(deltaTime.GetDeltaTime());
        particleRenderer->UpdateParticleEmitters(static_cast<float>(deltaTime.GetTotalTime()), static_cast<float>(deltaTime.GetDeltaTime()));

		// *** Update Rendering *** //
		m_renderer->RenderWorld(camera);
		m_renderer->RenderUI(camera);
		m_renderer->Present();


	}

	Shutdown();
}

void ClientApplication::ConnectToLobbyServer()
{
	printf("Connecting to LobbyServer...\n");
	tbsg::Config config = tbsg::Config::Get();
	ptl::string ip = config.GetAddress();
	unsigned int port = config.GetPort();
	lobbyClient.Connect(ip.c_str(), port);

}

void ClientApplication::ConnectToGameServer(const ptl::string& ip, unsigned port)
{
	printf("Connecting to GameServer...\n");
	gameClient.Connect(ip.c_str(), port);

}

void ClientApplication::Shutdown()
{
	lobbyClient.Disconnect();
	gameClient.Disconnect();

	IApplication::Shutdown();

#ifdef PLATFORM_WINDOWS
	DX12Application::Destroy();
#endif
	utils::DeinitEncrypt();
	delete uiSystem;

#ifdef PLATFORM_PS
	sceFiosTerminate();
#endif
}

void ClientApplication::SetUpRenderingPipeline()
{
	const float vignetteRadius = 1;//.95f;
	const float vignetteSmoothness = 0;// 0.4f;

	// *** Shader Creation *** //
	auto& rm = m_renderer->GetGfxResourceManager();
	//uint8_t bitmask = gfx::vertexPosition + gfx::vertexNormal + gfx::vertexUv;	// unused?
#if defined(PLATFORM_PS)
	auto vertexShaderPath = ptl::string("shader_v.sb");
	auto pixelShaderPath = ptl::string("shader_p.sb");
	const ShaderId mainVertexId = rm.CreateShader("ForwardPass_v.sb");
	const ShaderId mainPixelId = rm.CreateShader("ForwardPass_p.sb");
	const ShaderId particleVertexId = rm.CreateShader("ParticleShader_v.sb");
	const ShaderId particlePixelId = rm.CreateShader("ParticleShader_p.sb");
	const ShaderId vignetteVertexId = rm.CreateShader("Vignette_v.sb");
	const ShaderId vignettePixelId = rm.CreateShader("Vignette_p.sb");
	const ShaderId bloomVertexId = rm.CreateShader("Bloom_v.sb");
	const ShaderId bloomPixelId = rm.CreateShader("Bloom_p.sb");
	const ShaderId blurComputeId = rm.CreateShader("Blur_c.sb");
	const ShaderId mipMapComputeId = rm.CreateShader("MipMap_c.sb");
	const ShaderId uiOverlayVertexId = rm.CreateShader("UIOverlayShader_v.sb");
	const ShaderId uiOverlayPixelId = rm.CreateShader("UIOverlayShader_p.sb");
	const ShaderId skyboxVertexId = rm.CreateShader("SkyboxShader_v.sb");
	const ShaderId skyboxPixelId = rm.CreateShader("SkyboxShader_p.sb");
#elif defined(PLATFORM_WINDOWS)
	auto vertexShaderPath = tbsg::Config::Get().MakeShaderPath("ForwardPass_v.cso", 1);
	auto pixelShaderPath = tbsg::Config::Get().MakeShaderPath("ForwardPass_p.cso", 1);
#endif

	auto vsId = rm.CreateShader(vertexShaderPath.c_str());
	auto psId = rm.CreateShader(pixelShaderPath.c_str());
	ptl::vector<gfx::IShader*> shaders
	{
		&rm.GetShader(vsId),
		&rm.GetShader(psId)
	};

	// ** Adding Render Pass to Shader ** //
#ifdef PLATFORM_WINDOWS
	{
		auto dxRenderer = static_cast<DX12Renderer*>(m_renderer);
		auto fwdPass = new DxForwardPass{ dxRenderer, shaders };
		m_renderer->AddRenderPass(new DxSkyboxPass{ dxRenderer, skyboxFile});
		m_renderer->AddRenderPass(fwdPass);
		m_renderer->AddRenderPass(new DxVignettePass{ dxRenderer , vignetteRadius, vignetteSmoothness });
		auto mipmapPass = new DxMipmapPass{ dxRenderer, &fwdPass->GetRwBloomTexture() };
		m_renderer->AddRenderPass(mipmapPass);
		m_renderer->AddRenderPass(new DxBlurPass{ dxRenderer, *mipmapPass });
		m_renderer->AddRenderPass(new DxMergeBloomMipsPass{ dxRenderer, mipmapPass->GetNonMsaaTextureToMipmap() });
		{
			const auto dxParticleRenderer = new DxParticleRenderer(dxRenderer, fwdPass->GetBloomTexture(), fwdPass->GetBloomTextureFormat());
			particleRenderer = ptl::unique_ptr<IParticleRenderer>(dxParticleRenderer);
			m_renderer->AddRenderPass(new ParticlePass(dxParticleRenderer));
			sceneHandler.SetParticleRenderer(particleRenderer.get());
		}
	}
#elif PLATFORM_PS

	const ptl::vector<gfx::IShader*> blurShader{ &rm.GetShader(blurComputeId) };
	const ptl::vector<gfx::IShader*> mipMapShader{ &rm.GetShader(mipMapComputeId) };
	const ptl::vector<gfx::IShader*> mainPassShaders{ &rm.GetShader(mainVertexId), &rm.GetShader(mainPixelId) };
	const ptl::vector<gfx::IShader*> particlePassShaders{ &rm.GetShader(particleVertexId), &rm.GetShader(particlePixelId) };
	const ptl::vector<gfx::IShader*> bloomShaders{ &rm.GetShader(bloomVertexId), &rm.GetShader(bloomPixelId) };
	const ptl::vector<gfx::IShader*> vignetteShaders{ &rm.GetShader(vignetteVertexId), &rm.GetShader(vignettePixelId) };
	const ptl::vector<gfx::IShader*> uiOverlayShaders{ &rm.GetShader(uiOverlayVertexId), &rm.GetShader(uiOverlayPixelId) };
	const ptl::vector<gfx::IShader*> skyboxShaders{ &rm.GetShader(skyboxVertexId), &rm.GetShader(skyboxPixelId) };

	const auto texWhite = TextureId(1);
	const auto matId = rm.CreateMaterial(BlendMode(), { 1.0f, 1.0f, 1.0f, 1.0 }, texWhite, { 1.0f, 0.0f, 0.0f }, texWhite);

	// create a bilinear sampler
	sce::Gnm::Sampler bilinearSampler;
	bilinearSampler.init();
	bilinearSampler.setMipFilterMode(sce::Gnm::kMipFilterModeNone);
	bilinearSampler.setXyFilterMode(sce::Gnm::kFilterModeBilinear, sce::Gnm::kFilterModeBilinear);
	{
		ptl::vector<Vertex> skyboxVertices;
		ptl::vector<uint32_t> skyboxIndices;
		auto skyboxTextureId = rm.CreateTexture("Space_OutputCube_1024.dds");
		CreateCube(25, skyboxIndices, skyboxVertices, true);
		auto skyboxMeshId = rm.CreateMesh(skyboxVertices.data(), skyboxVertices.size(), skyboxIndices.data(), skyboxIndices.size());
		PS4SkyBoxPass* skyboxPass = new PS4SkyBoxPass(vertexPosition + vertexNormal + vertexUv, skyboxShaders, bilinearSampler, m_renderer->GenerateRenderPassId(), rm.GetMesh(skyboxMeshId), rm.GetTexture(skyboxTextureId));
		m_renderer->AddRenderPass(skyboxPass);
	}
	{
		PS4MainPass* mainPass = new PS4MainPass(vertexPosition + vertexNormal + vertexUv, mainPassShaders, bilinearSampler, m_renderer->GenerateRenderPassId());
		auto passId = m_renderer->AddRenderPass(mainPass);
	}
	{
		PS4ParticlePass* particlePass = new PS4ParticlePass(vertexPosition + vertexUv, particlePassShaders, bilinearSampler, *static_cast<PS4Renderer*>(m_renderer), m_renderer->GenerateRenderPassId());
		const auto passId = m_renderer->AddRenderPass(particlePass);
		auto pr = static_cast<IParticleRenderer*>(&particlePass->GetParticleRenderer());
		particleRenderer = ptl::unique_ptr<IParticleRenderer>(pr);
	}
	{
		PS4VignettePass* vignettePass = new PS4VignettePass(0, vignetteShaders, vignetteRadius, vignetteSmoothness, m_renderer->GetScreenSize(), m_renderer->GenerateRenderPassId());
		m_renderer->AddRenderPass(vignettePass);
	}
	{
		PS4MipMapPass* mipMapPass = new PS4MipMapPass(0, mipMapShader, m_renderer->GenerateRenderPassId());
		m_renderer->AddRenderPass(mipMapPass);
	}
	{
		PS4BlurPass* blurPass = new PS4BlurPass(0, blurShader, m_renderer->GenerateRenderPassId());
		m_renderer->AddRenderPass(blurPass);
	}
	{
		PS4BloomPass* bloomPass = new PS4BloomPass(0, bloomShaders, m_renderer->GenerateRenderPassId());
		m_renderer->AddRenderPass(bloomPass);
	}
#endif
}
