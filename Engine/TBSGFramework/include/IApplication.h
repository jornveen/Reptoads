#pragma once
#include "core/Config.h"
namespace gfx
{
    class IRenderer;
}
class Client;
namespace tbsg {
    class Input;

    class IApplication
    {
    public:
        virtual ~IApplication();
        virtual void Initialize(int argc,char *argv[]);
		virtual void Run();

    public:
		/// An override you can set to true, to force the game to quit.
		/// Used by DX12 Impl to quit when window is closed.
		static bool m_ShouldStop/* = false*/;

    protected:
		virtual void Shutdown();

    protected:
        gfx::IRenderer* m_renderer{ nullptr };
        Input* m_input{ nullptr };
    };
}