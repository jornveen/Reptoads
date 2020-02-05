#include "ViewerApplication.h"
#include "gameplay/Transform.h"
#include "rendering/IRenderer.h"
#include "scene/SceneGraph.h"
#include <brofiler/Brofiler.h>
#include "core/Input.h"
#include "rendering/dx/DX12Application.h"
#include "resource_loading/GLTFLoader.h"
#include "rendering/gnm/RenderPasses/PS4MainPass.h"
#include "rendering/dx/render_passes/DxForwardPass.h"
#ifdef PLATFORM_PS
#include <fios2.h>
#elif PLATFORM_WINDOWS
#include "rendering/dx/render_passes/DxVignettePass.h"
#include "rendering/dx/DxParticleRenderer.h"
#include "rendering/dx/render_passes/DxMipmapPass.h"
#include "rendering/dx/render_passes/DxSkyboxPass.h"
#endif
#include "rendering/gnm/RenderPasses/PS4VignettePass.h"
#include "rendering/gnm/RenderPasses/PS4MipMapPass.h"
#include "rendering/gnm/RenderPasses/PS4BlurPass.h"
#include "rendering/gnm/RenderPasses/PS4BloomPass.h"
#include "rendering/gnm/RenderPasses/PS4SkyboxPass.h"
#include "rendering/GeometricShapes.h"
#include "resource_loading/ArgumentParsing.h"
#include "rendering/unified_shader_passes/ParticlePass.h"
#include "resource_loading/ParticleParser.h"
#include "ui/UIText.h"
#include "ui/UISystem.h"
#include "resource_loading/TtfHeaderParser.h"
//TMP
#include "rendering/IUIRenderer.h"
#include "rendering/gnm/RenderPasses/PS4ParticlePass.h"

#include "core/DeltaTime.h"
#include "rendering/dx/render_passes/DxBlurPass.h"
#include "rendering/dx/render_passes/DxMergeBloomMipsPass.h"
#ifdef PLATFORM_PS
#include <kernel.h>
#include <fios2.h>

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



void ViewerApplication::Initialize(int argc, char* argv[])
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


	startTime = std::chrono::high_resolution_clock::now();

	tbsg::ArgumentParsing(tbsg::Config::Get(), argc, argv, arguments);

	// set the default tex
	m_renderer->GetGfxResourceManager().CreateTexture("DefaultWhite.bmp");
    if (!arguments.HasResultStr("--load-model")) {
		modelPathToLoad = "TestSunday7/T7.gltf";
		std::cout << "No model specified with '--load-model MyModel.gltf'. Loading default scene.gltf.\n";
	} else {
        modelPathToLoad = arguments["--load-model"];
		FILE *file;
		ptl::string fullModelPath = tbsg::Config::Get().MakeModelPath(modelPathToLoad);
		if ((file = fopen(fullModelPath.c_str(), "r"))) {
			fclose(file);
		} else {
			std::cerr << "No file exists model with name: '" << modelPathToLoad << "'. Full path which is attempted to be loaded is: '" << fullModelPath << "'.\n";
		}
	}

    // ** Initilize UI System ** //
    uiSystem = new ui::UISystem{m_renderer->GetUIRenderer(), m_renderer->GetGfxResourceManager(), audioSystem, [this](ptl::shared_ptr<ui::UIText> textField)
	{
		m_input->OpenInputDialog(textField->ObsecureText(), textField->GetRawText());
	}};
    if(arguments.HasResultStr("--ui")){
        uiSystem->PushUI(arguments["--ui"]);
    }

	if (arguments.HasResultStr("--preview-particle")) {

        particlePathToLoad = arguments["--preview-particle"];
		FILE *file;
		ptl::string fullModelPath = tbsg::Config::Get().MakeParticlePath(particlePathToLoad);
		if ((file = fopen(fullModelPath.c_str(), "r"))) {
			fclose(file);
	} else {
			std::cerr << "No file exists! particle with name: '" << particlePathToLoad << "'. Full path which is attempted to be loaded is: '" << fullModelPath << "'.\n";
		}
	}

#ifdef PLATFORM_PS
	m_input->AddListener(BINDAXIS(&ViewerApplication::Vert, this), tbsg::InputType::EGamepad_Left_Thumb_Y);
	m_input->AddListener(BINDAXIS(&ViewerApplication::Hori, this), tbsg::InputType::EGamepad_Left_Thumb_X);
	m_input->AddListener(BINDAXIS(&ViewerApplication::RotVert, this), tbsg::InputType::EGamepad_Right_Thumb_Y);
	m_input->AddListener(BINDAXIS(&ViewerApplication::RotHori, this), tbsg::InputType::EGamepad_Right_Thumb_X);
	m_input->AddListener(BINDACTION(&ViewerApplication::ScaleDownHor, this), tbsg::InputType::EGamepad_DPad_Left);
	m_input->AddListener(BINDACTION(&ViewerApplication::ScaleUpHor, this), tbsg::InputType::EGamepad_DPad_Right);
	m_input->AddListener(BINDACTION(&ViewerApplication::ScaleUpVert, this), tbsg::InputType::EGamepad_DPad_Up);
	m_input->AddListener(BINDACTION(&ViewerApplication::ScaleDownVert, this), tbsg::InputType::EGamepad_DPad_Down);
#endif
#ifdef PLATFORM_WINDOWS
	m_input->AddListener(BINDACTION(&ViewerApplication::ReloadParticleEmitter, this), tbsg::InputType::R);
#endif
}
static int powWidth = 1;
#ifdef PLATFORM_PS
bool ViewerApplication::ScaleUpHor(tbsg::InputAction ia)
{
	if (ia != tbsg::InputAction::EPressed)
		return false;

	++powWidth;
	width += 8;// glm::pow(2.0f, (float)powWidth);
	TMP_TXT.SetSize({ width, height });
	m_renderer->GetUIRenderer()->RemoveText(&TMP_TXT);
	m_renderer->GetUIRenderer()->AddText(&TMP_TXT, glm::orthoLH(0.0f, 1920.0f, 0.0f, 1080.0f, 0.0f, 1.0f));
	std::cout << std::to_string(width) + " " + std::to_string(height) << std::endl;
	return false;
}

bool ViewerApplication::ScaleDownHor(tbsg::InputAction ia)
{
	if (ia != tbsg::InputAction::EPressed)
		return false;

	--powWidth;
	width -= 8;// glm::pow(2.0f, (float)powWidth);
	TMP_TXT.SetSize({ width, height });
	m_renderer->GetUIRenderer()->RemoveText(&TMP_TXT);
	m_renderer->GetUIRenderer()->AddText(&TMP_TXT, glm::orthoLH(0.0f, 1920.0f, 0.0f, 1080.0f, 0.0f, 1.0f));
	std::cout << std::to_string(width) + " " + std::to_string(height) << std::endl;
	return false;
}

bool ViewerApplication::ScaleUpVert(tbsg::InputAction ia)
{
	if (ia != tbsg::InputAction::EPressed)
		return false;

	height +=8;
	TMP_TXT.SetSize({ width, height });
	m_renderer->GetUIRenderer()->RemoveText(&TMP_TXT);
	m_renderer->GetUIRenderer()->AddText(&TMP_TXT, glm::orthoLH(0.0f, 1920.0f, 0.0f, 1080.0f, 0.0f, 1.0f));
	std::cout << std::to_string(width) + " " + std::to_string(height) << std::endl;
	return false;
}

bool ViewerApplication::ScaleDownVert(tbsg::InputAction ia)
{
	if (ia != tbsg::InputAction::EPressed)
		return false;

	height -= 8;
	TMP_TXT.SetSize({ width, height });
	m_renderer->GetUIRenderer()->RemoveText(&TMP_TXT);
	m_renderer->GetUIRenderer()->AddText(&TMP_TXT, glm::orthoLH(0.0f, 1920.0f, 0.0f, 1080.0f, 0.0f, 1.0f));
	std::cout << std::to_string(width) + " " + std::to_string(height) << std::endl;
	return false;
}




bool ViewerApplication::Vert(float val)
{
	camera.Translate(glm::vec3(0, 0, val * -0.1));

	return false;
}
bool ViewerApplication::Hori(float val)
{
	camera.Translate(glm::vec3(val * 0.1, 0, 0));
	return false;
}
#endif

void ViewerApplication::Run()
{
	IApplication::Run();

	core::Transform lightTr{ glm::vec3{-7.0, 0.0, -0.0 }, glm::quat({0.f, 0.f, 0.f}), {.5,.5,.5} };
	core::Transform lightTr2{ glm::vec3{0.0, 0.0, -0.0 }, glm::quat({0.f, 0.f, 0.f}), {.5,.5,.5} };
	core::Transform lightTr3{ glm::vec3{0.0, 0.0, -0.0 }, glm::quat({0.f, 0.f, 0.f}), {.5,.5,.5} };
	core::Transform box3Tr{ glm::vec3{0, 0, 0 }, glm::quat({0.f, 0.f, 0.f}), {1,1,1} };

	ptl::string ps4Hostapp = ""; //On pc
#ifdef PLATFORM_PS
	ps4Hostapp = "/hostapp/";
#endif

	//ptl::string fontNameTest = tbsg::GetFontNameFromTtf(ps4Hostapp + "data/fonts/ZCOOLKuaiLe-Regular.ttf");
	//std::cout << "Name of ttf file is: " << fontNameTest << "\n";

	auto& rm = m_renderer->GetGfxResourceManager();

	glm::vec3 camera_eye{ 0, 6, 0 };
	glm::vec3 camera_target{ 0, 0, 1 };
	glm::vec3 camera_up{ 0, 1, 0 };

	camera.set_LookAt(camera_eye, camera_target, camera_up);
	camera.set_Projection(60.f, 1280.f / 720.f, 1.f, 2500.f);

	const float vignetteRadius = 1;//.95f;
	const float vignetteSmoothness = 0;// 0.4f;

#if defined(PLATFORM_WINDOWS)
	auto vertexShaderPath = tbsg::Config::Get().MakeShaderPath("ForwardPass_v.cso", 1);
	auto pixelShaderPath = tbsg::Config::Get().MakeShaderPath("ForwardPass_p.cso", 1);

	ShaderId vertexId = rm.CreateShader(vertexShaderPath);
	ShaderId pixelId = rm.CreateShader(pixelShaderPath);
	ptl::vector<gfx::IShader*> shaders
	{
		&rm.GetShader(vertexId),
		&rm.GetShader(pixelId)
	};

	{
		const auto dxRenderer = static_cast<DX12Renderer*>(m_renderer);
		auto fwdPass = new DxForwardPass{ static_cast<DX12Renderer*>(m_renderer), shaders };
		auto mipmapPass = new DxMipmapPass{ dxRenderer, &fwdPass->GetRwBloomTexture() };
		const auto dxParticleRenderer = new DxParticleRenderer(dxRenderer, fwdPass->GetBloomTexture(), fwdPass->GetBloomTextureFormat());
		particleRenderer = ptl::unique_ptr<IParticleRenderer>{ dxParticleRenderer };
		m_renderer->AddRenderPass(new DxSkyboxPass{ dxRenderer, skyboxFile });
		m_renderer->AddRenderPass(fwdPass);
		m_renderer->AddRenderPass(new ParticlePass{ dxParticleRenderer });
		m_renderer->AddRenderPass(new DxVignettePass{ dxRenderer , vignetteRadius, vignetteSmoothness });
		m_renderer->AddRenderPass(mipmapPass);
		m_renderer->AddRenderPass(new DxBlurPass{ dxRenderer, *mipmapPass });
		m_renderer->AddRenderPass(new DxMergeBloomMipsPass{ dxRenderer, mipmapPass->GetNonMsaaTextureToMipmap() });
	}

#endif

#ifdef PLATFORM_PS
	// create render passes
	{
		const ShaderId mainVertexId = rm.CreateShader("ForwardPass_v.sb");
		const ShaderId mainPixelId = rm.CreateShader("ForwardPass_p.sb");


		const ShaderId particleVertexId = rm.CreateShader("ParticleShader_v.sb");
		const ShaderId particlePixelId = rm.CreateShader("ParticleShader_p.sb");
		const ShaderId skyboxVertexId = rm.CreateShader("SkyboxShader_v.sb");
		const ShaderId skyboxPixelId = rm.CreateShader("SkyboxShader_p.sb");

		const ShaderId vignetteVertexId = rm.CreateShader("Vignette_v.sb");
		const ShaderId vignettePixelId = rm.CreateShader("Vignette_p.sb");
		const ShaderId bloomVertexId = rm.CreateShader("Bloom_v.sb");
		const ShaderId bloomPixelId = rm.CreateShader("Bloom_p.sb");
		const ShaderId blurComputeId = rm.CreateShader("Blur_c.sb");
		const ShaderId mipMapComputeId = rm.CreateShader("MipMap_c.sb");
		const ptl::vector<gfx::IShader*> blurShader{ &rm.GetShader(blurComputeId) };
		const ptl::vector<gfx::IShader*> mipMapShader{ &rm.GetShader(mipMapComputeId) };
		const ptl::vector<gfx::IShader*> mainPassShaders { &rm.GetShader(mainVertexId), &rm.GetShader(mainPixelId)};
		const ptl::vector<gfx::IShader*> particlePassShaders { &rm.GetShader(particleVertexId), &rm.GetShader(particlePixelId)};
		const ptl::vector<gfx::IShader*> bloomShaders{ &rm.GetShader(bloomVertexId), &rm.GetShader(bloomPixelId)};
		const ptl::vector<gfx::IShader*> vignetteShaders{ &rm.GetShader(vignetteVertexId), &rm.GetShader(vignettePixelId)};
		const ptl::vector<gfx::IShader*> skyboxShaders{&rm.GetShader(skyboxVertexId), &rm.GetShader(skyboxPixelId)};


		ptl::vector<Vertex> vertices;
		ptl::vector<uint32_t> indices;
		CreateCube(1, indices, vertices, true);
		auto meshId = rm.CreateMesh(vertices.data(), vertices.size(), indices.data(), indices.size());
		const auto texWhite = TextureId(1);
		const auto matId = rm.CreateMaterial(BlendMode(), { 1.0f, 1.0f, 1.0f, 1.0 }, texWhite, { 1.0f, 0.0f, 0.0f }, texWhite);
		core::Transform* tr = new core::Transform();
		tr->SetPos({ 0,0,-3 });
		core::Transform* tr2 = new core::Transform();
		tr->SetPos({ 0,0,0 });
		core::Transform* tr3 = new core::Transform();
		tr->SetPos({ 0,0,3 });
		m_renderer->AddRenderable(tr, meshId, matId);
		//m_renderer->AddRenderable(tr2, meshId, matId);

		// create a bilinear sampler
		sce::Gnm::Sampler bilinearSampler;
		bilinearSampler.init();
		bilinearSampler.setMipFilterMode(sce::Gnm::kMipFilterModeNone);
		bilinearSampler.setXyFilterMode(sce::Gnm::kFilterModeBilinear, sce::Gnm::kFilterModeBilinear);

		sce::Gnm::Sampler cubemapSampler;
		cubemapSampler.init();
		cubemapSampler.setXyFilterMode(sce::Gnm::kFilterModeBilinear, sce::Gnm::kFilterModeBilinear);
		cubemapSampler.setMipFilterMode(sce::Gnm::kMipFilterModeLinear);

		ptl::vector<Vertex> skyboxVertices;
		ptl::vector<uint32_t> skyboxIndices;
		CreateCube(25, skyboxIndices, skyboxVertices, true);
		auto skyboxMeshId = rm.CreateMesh(skyboxVertices.data(), skyboxVertices.size(), skyboxIndices.data(), skyboxIndices.size());

		auto skyboxTextureId = rm.CreateTexture(skyboxFile);

		{
			PS4SkyBoxPass* skyboxPass = new PS4SkyBoxPass(vertexPosition + vertexNormal + vertexUv, skyboxShaders, cubemapSampler, m_renderer->GenerateRenderPassId(), rm.GetMesh(skyboxMeshId), rm.GetTexture(skyboxTextureId));
			m_renderer->AddRenderPass(skyboxPass);
		}
		{
			PS4MainPass* mainPass = new PS4MainPass(vertexPosition + vertexNormal + vertexUv, mainPassShaders, bilinearSampler, m_renderer->GenerateRenderPassId());
			auto passId = m_renderer->AddRenderPass(mainPass);
		}
		{
			PS4ParticlePass* particlePass = new PS4ParticlePass(vertexPosition + vertexUv, particlePassShaders, bilinearSampler, *static_cast<PS4Renderer*>(m_renderer), m_renderer->GenerateRenderPassId());
			auto passId = m_renderer->AddRenderPass(particlePass);
			IParticleRenderer* pr = static_cast<IParticleRenderer*>(&particlePass->GetParticleRenderer());
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
	}

	TMP_TXT.SetText("TEST!");
	TMP_TXT.SetFontSize(32);
	TMP_TXT.SetSize({ width, height });
	//TMP_TXT.Set(0.1f);
	TMP_TXT.SetHorizontalAlignment(gfx::UIText::HorizontalAlignment::Center);
	TMP_TXT.SetVerticalAlignment(gfx::UIText::VerticalAlignment::Top);
	TMP_TXT.SetColor({ 1.0f,1.0f,1.0f,1.0f });

	//m_renderer->GetUIRenderer()->AddText(&TMP_TXT, glm::orthoLH(0.0f, 1920.0f, 0.0f, 1080.0f, 0.0f, 1.0f));
	// END TMP
#endif

	scene::SceneNode root{};

	GLTFLoader gltfLoader{ *m_renderer };
	gltfLoader.LoadModelFromGltf(tbsg::Config::Get().MakeModelPath(modelPathToLoad), &root);

	for (auto& child : root) {
		for (auto& renderable : child.renderables) {
			renderable.renderableRegisteredId = m_renderer->AddRenderable(&child.transform, renderable.meshId, renderable.materialId);
		}
	}

//#ifdef PLATFORM_WINDOWS
	if (!particlePathToLoad.empty()) {
		auto& uiRenderer = *m_renderer->GetUIRenderer();
		uiRenderer.AddText({ 100, 100 }, { 200, 200 }, "Currently previewing: '" + particlePathToLoad + "'", 
			UIText::HorizontalAlignment::Leading, UIText::VerticalAlignment::Top, 16, { 1, 1, 1, 1 }, nullptr);

		ReloadParticleEmitter({});
	}
//#endif

	auto& uiRenderer = *m_renderer->GetUIRenderer();
	gfx::UITextOptionsForTexture options{};
	options.text = "Another Test String";
	options.font = "Verdana";
	options.fontSize = 32;
	options.size = { width, height };
	scene::SceneNode newRoot;

	glm::vec3 pos{};

	ptl::vector<Vertex> vertices;
	ptl::vector<uint32_t> indices;
	gfx::CreateCube(1, indices, vertices, true);
	auto meshId = rm.CreateMesh(vertices.data(), vertices.size(), indices.data(), indices.size());
	const auto texWhite = TextureId(1);
	const auto matId = rm.CreateMaterial(BlendMode(), { 1.0f, 1.0f, 1.0f, 1.0 }, texWhite, { 1.0f, 0.0f, 0.0f }, texWhite);
	core::Transform* tr = new core::Transform();
	tr->SetPos({ 0,0,-3 });
	core::Transform* tr2 = new core::Transform();
	tr->SetPos({ 0,0,0 });
	core::Transform* tr3 = new core::Transform();
	tr->SetPos({ 0,0,3 });
	m_renderer->AddRenderable(tr, meshId, matId);


	lastTime = std::chrono::high_resolution_clock::now();

	auto maxTimeStep = tbsg::Config::Get().maxSteptime;
	auto maxSteps = tbsg::Config::Get().maxSteps;
	using ms = std::chrono::duration<float, std::milli>;
	using s = std::chrono::duration<float, std::chrono::seconds>;

	double timeLeft;
	double deltaTime;

	scene::SceneNode myRoot;
	myRoot.transform.SetPos({ 0, 1, 0 });

	gfx::UITextOptionsForTexture txtoptions{};
	txtoptions.text = "MEMEMES";
	txtoptions.size = { 200, 200 };

	static int counter = 0;
	core::DeltaTime dt{};
	dt.Initialize();
	while (!IApplication::m_ShouldStop) {
		BROFILER_FRAME("Framee");
		dt.Update();
		auto t = std::chrono::high_resolution_clock::now();
		auto time = std::chrono::duration_cast<ms>(t - lastTime).count() / 1000.0;
		lastTime = t;

		float totalTime = std::chrono::duration_cast<ms>(t - startTime).count() / 1000.0;

		rm.GetMaterial(matId).GetShaderMaterialData().materialEmissiveColor.x = glm::sin(totalTime) * 0.5 + 0.5;

		int steps = (time / maxTimeStep) + 1;
		if (steps > maxSteps)
			steps = maxSteps;

		if (steps == 1) {
			deltaTime = time;
			timeLeft = 0.0;
		}
		else {
			deltaTime = time / steps;
			timeLeft = std::remainder(time, steps);
		}

		m_input->Update();
		if(particleRenderer)
			particleRenderer->UpdateParticleEmitters(totalTime, static_cast<float>(deltaTime));

		while (steps > 0) {
			steps--;
			if (steps == 0) {
				deltaTime += timeLeft;
			}

			//std::cout << "DeltaTime: " << deltaTime << "ms\n";

			/*
			 *****************************************************************
			 *
			 * All game logic that needs deltatime goes here
			 *
			 ******************************************************************
			 */

			float value = (lastTime - startTime).count() * 0.000001;
			pos.x = sin(value) * 15;
			pos.z = cos(value) * 15;

			glm::ivec2 cursorPos;
			m_input->GetCursorPosition(cursorPos.x, cursorPos.y);

			auto text = uiSystem->GetChildElementOfTag("main_menu_sp_button_text").lock();

			if (text)
			{
				static_cast<ui::UIText*>(text.get())->SetText(ptl::to_string(value));
			}
		}


		glm::ivec2 cursorPos;
		m_input->GetCursorPosition(cursorPos.x, cursorPos.y);
		uiSystem->Update(dt, cursorPos);
		
		gfx::UITextOptionsForTexture txtoptions{};
		txtoptions.text = "MEMEMES" + ptl::to_string(++counter);
		txtoptions.size = { 200, 200 };

		m_renderer->RenderWorld(camera);
		m_renderer->RenderUI(camera);
		m_renderer->Present();
	}

	Shutdown();
}

void ViewerApplication::Shutdown()
{
	IApplication::Shutdown();
    delete uiSystem;
#ifdef PLATFORM_WINDOWS
	//particleRenderer->Shutdown();
	particleRenderer.reset();
	DX12Application::Destroy();

#endif

#ifdef PLATFORM_PS
	sceFiosTerminate();
#endif
}

bool ViewerApplication::ReloadParticleEmitter(tbsg::InputAction action)
{
	if (action != tbsg::InputAction::EPressed) {
		return true;
	}

	if(particlePathToLoad.empty()) {
		return true;
	}

	if(previewParticleEmitterId.IsNotNull()) {
		particleRenderer->RemoveEmitter(previewParticleEmitterId);
	}

	ParticleParser particleParser{ m_renderer->GetGfxResourceManager() };

	bool succesfullyLoaded;
	ParticleEmitter emitter;
	std::tie(emitter, succesfullyLoaded) = particleParser.DeserializeParticleFromFile(tbsg::Config::Get().MakeParticlePath(particlePathToLoad));

	if (succesfullyLoaded) {
		core::Transform emitterPos{ glm::vec3{0.0, 10.0f, 30.0f}, glm::quat{}, glm::vec3{1.0f, 1.0f, 1.0f} };
		emitter.particleEmitterPosition = emitterPos.GetPos();
		previewParticleEmitterId = particleRenderer->AddEmitter(emitterPos, std::move(emitter));
	} else {
		std::cerr << "ERROR Could not load particle emitter from json, see previous messages for more detail" << std::endl;
	}

	return true;
}
