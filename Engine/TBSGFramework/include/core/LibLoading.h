#pragma once
#ifdef PLATFORM_PS
#include <sys/_defines/_sce_ok.h>
#include <_kernel.h>

namespace core
{

	static int load_library(const char * module_file_name)
	{
		SceKernelModule module_id = sceKernelLoadStartModule(module_file_name, 0, 0, 0, nullptr, nullptr);
		switch(module_id)
		{
		case SCE_KERNEL_ERROR_EINVAL: printf("load_library failed: SCE_KERNEL_ERROR_EINVAL"); break;
		case SCE_KERNEL_ERROR_ENOENT: printf("load_library failed: SCE_KERNEL_ERROR_ENOENT (File does not exist)"); break;
		case SCE_KERNEL_ERROR_ENOEXEC: printf("load_library failed: SCE_KERNEL_ERROR_ENOEXEC (Abnormal file format)"); break;
		case SCE_KERNEL_ERROR_ENOMEM: printf("load_library failed: SCE_KERNEL_ERROR_ENOMEM (Unable to allocate memory)"); break;
		case SCE_KERNEL_ERROR_EACCES: printf("load_library failed: SCE_KERNEL_ERROR_EACCES (No access to file)"); break;
		case SCE_KERNEL_ERROR_EFAULT: printf("load_library failed: SCE_KERNEL_ERROR_EFAULT (Pointing to invalid memory)"); break;
		case SCE_KERNEL_ERROR_EAGAIN: printf("load_library failed: SCE_KERNEL_ERROR_EAGAIN (Insufficient resources)"); break;
		case SCE_KERNEL_ERROR_ESDKVERSION: printf("load_library failed: SCE_KERNEL_ERROR_ESDKVERSION (Library built against a newer version than system version is)"); break;
		default: break;
		}
		return module_id >= SCE_OK;
	}
}
#endif
