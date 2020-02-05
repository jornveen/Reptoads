#include "IApplication.h"
#include <Net/Client.h>
#include <core/Input.h>
#include "resource_loading/ConfigLoader.h"
#include "resource_loading/ArgumentParsing.h"

#if defined(_WIN32)
#include <rendering/dx/DX12Renderer.h>
#include <rendering/dx/DX12Application.h>
#elif defined(PLATFORM_PS)
#include <rendering/gnm/PS4Renderer.h>
#endif
/*
void ArgumentParsing(tbsg::Config& config, int argc, char** argv)
{
    
}*/

namespace tbsg {

	// Static IApplication Variables
	bool IApplication::m_ShouldStop = false;

	IApplication::~IApplication()
	{
		// m_renderer->Shutdown();
		// delete m_renderer;
		delete m_input;
	}

	void IApplication::Initialize(int argc,char *argv[])
	{
		//init config:
#if defined(PLATFORM_WINDOWS)
		LoadConfig("./config.json");
		tbsg::ParsedArguments arguments;
		ArgumentParsing(tbsg::Config::Get(), argc, argv, arguments);
		HINSTANCE hInstance = GetModuleHandle(NULL);
		m_renderer = new gfx::DX12Renderer(hInstance, true);
#elif defined(PLATFORM_PS)
		LoadConfig("/hostapp/config.json");
		m_renderer = new gfx::PS4Renderer();
#endif
		// initialize the renderer
		m_renderer->Initialize();
		m_input = new Input(tbsg::Config::Get().width, tbsg::Config::Get().height);
	}

	void IApplication::Run()
	{
	}

	void IApplication::Shutdown()
	{
		m_renderer->Shutdown();
		delete m_renderer; 
	    m_renderer = nullptr;
	}
}
