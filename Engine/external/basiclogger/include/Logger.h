#pragma once
/**
 *  \file
 *  \brief     Easy to include file
 *  \author    Simon Renger
 *  \version   1.0
 *  \date      2018
 *  \copyright  The MIT License
 *  \section Examples
 *  \subsection  example_object Object Approach
 *  \include   example_object.cpp
  * \subsection  example_macro Macro Approach
 *  \include   example_macro.cpp
 *  \subsection example_freefunctions Freefunctions Approach
 *  \snippet example_freefunction.cpp general
  * \subsubsection example_easyfunc_stdout Easy to use initialization function for `stdout`
 *  \snippet example_freefunction.cpp stdout
 * \subsubsection example_easyfunc_file Easy to use initialization function for `file`
 *  \snippet example_freefunction.cpp file
 *
 * The MIT License
 *
 * Copyright (c) 2018 Simon Renger, https://simonrenger.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **/

#include "LoggingFunction.h"
#include "LoggingMacros.h"
#include "Sinks/File.h"
#include "Sinks/StdOut.h"
