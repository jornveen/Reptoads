#pragma once
//Define the call to the debugger
#ifndef BREAK_DEBUGGER
	#if defined(PLATFORM_WINDOWS)
		#include <intrin.h>
		#define BREAK_DEBUGGER() __debugbreak()
	#elif defined(PLATFORM_PS)
		#define BREAK_DEBUGGER() __builtin_trap()
	#else
		#define BREAK_DEBUGGER()
	#endif
#endif
#ifndef BREAK_DEBUGGER_WHEN
	#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_PS)
		#define BREAK_DEBUGGER_WHEN(condition) if(condition){ BREAK_DEBUGGER(); }
	#else
		#define BREAK_DEBUGGER_WHEN(condition)
	#endif
#endif

#include <iostream>
#include "memory/Containers.h"
#include "memory/String.h"

// assert() in Jeremiah's DX12 framework is slightly bugged, and assertions won't break the debugger.
// That's why we need to define it ourselves

inline void SmallPart_PrintErrorOnLine(ptl::string message = "", const char* file = nullptr, int line = -1, const char* expressionName = nullptr)
{
	using namespace ptl::string_literals;
	ptl::string s = "Assertion failed!\nAt: "s + file + " : " + ptl::to_string(line) + " with the expression: \n'" + expressionName;
	if (!message.empty())
		s += "'\n message: '"s + message + "'";

	std::cerr << s << '\n';
}


#ifndef PLATFORM_PS 
    #ifndef BREAK_DEBUGGER
        #define BREAK_DEBUGGER()
    #endif
    #ifndef ASSERT
        #define ASSERT(expression) if(!(expression)) { SmallPart_PrintErrorOnLine("", __FILE__, __LINE__, #expression); BREAK_DEBUGGER(); }
    #endif
    #ifndef ASSERT_MSG
        #define ASSERT_MSG(expression, message) if(!(expression)) { SmallPart_PrintErrorOnLine(message, __FILE__, __LINE__, #expression); BREAK_DEBUGGER(); }
    #endif
#elif defined(PLATFORM_PS)
#include <system_service.h>
#define ASSERT(expression) if(!(expression)) { SmallPart_PrintErrorOnLine("", __FILE__, __LINE__, #expression); 	sceSystemServiceReportAbnormalTermination(nullptr); }
#define ASSERT_MSG(expression, message) if(!(expression)) { SmallPart_PrintErrorOnLine(message, __FILE__, __LINE__, #expression); 	sceSystemServiceReportAbnormalTermination(nullptr); }
#endif