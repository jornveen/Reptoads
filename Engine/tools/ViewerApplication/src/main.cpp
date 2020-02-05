#include <iostream>
#include "ViewerApplication.h"
#include "core/LibLoading.h"

int main(int argc, char* argv[])
{

auto  o = ptl::DefaultMemoryResource::get_default_allocator();
#ifdef PLATFORM_PS
	assert(core::load_library("/hostapp/libfmod.prx"));
#endif

	ViewerApplication app{};
	app.Initialize(argc, argv);
	app.Run();

	return 0;
}
