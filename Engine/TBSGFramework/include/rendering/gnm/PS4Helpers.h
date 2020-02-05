#pragma once
#ifdef PLATFORM_PS
#include <exception>
#include <iostream>

inline void ThrowIfFailed(bool a_failed, const char* a_msg)
{
#if __DEBUG 
	if (a_failed)
	{
		std::cout<< a_msg << std::endl;
		throw std::exception(a_msg);
	}
#endif




}
#endif