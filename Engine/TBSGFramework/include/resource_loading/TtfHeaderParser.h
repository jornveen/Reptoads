#pragma once
#include "memory/string.h"

namespace tbsg
{
	// @Returns empty string when no name could be found!
	ptl::string GetFontNameFromTtf(const ptl::string& pathToTtf);
}
