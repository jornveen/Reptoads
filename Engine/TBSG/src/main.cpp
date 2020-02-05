#include "ClientApplication.h"
#include "memory/default_memresource.h"
#ifdef PLATFORM_PS
#include "core/LibLoading.h"
#include <mat.h>
#define MATGROUP_FREELIST_ALLOCATOR MatGroup(1)
#define MATGROUP_POOL_ALLOCATOR MatGroup(2)
#endif

int main(int argc, char *argv[])
{
#ifdef PLATFORM_PS
    assert(core::load_library("/hostapp/libfmod.prx"));
	sceMatInitialize(0);
	sceMatRegisterGroup(MATGROUP_FREELIST_ALLOCATOR, "Freelist allocator", SCEMAT_GROUP_ROOT);
	sceMatRegisterGroup(MATGROUP_POOL_ALLOCATOR, "Pool allocator", SCEMAT_GROUP_ROOT);
	sceMatPushMarkerStatic("STATIC_MARKER", 0x0000aa00, 0);
	sceMatUpdateModuleList();
#endif

	auto  o = ptl::DefaultMemoryResource::get_default_allocator();

	printf("Initializing application...\n");
    ClientApplication app;
    app.Initialize(argc, argv);
    app.Run();
#ifdef PLATFORM_PS
	sceMatUninitialize();
#endif
    return 0;
}
