#pragma once
/**
 *  \file
 *  \brief     A collection of logging free functions
 *  \details   This is the collection of logging free functions such as: Log,Debug,Info,Warn and Error as well as the init and deint.
 *  \author    Simon Renger
 *  \version   1.0
 *  \date      2018
 *  \pre       If used outside of Debug mode the macro `COF_USE_LOGGER`  needs to be defined. Otherweise all functions/methods will leave a zero fingerprint.
 *  \example   example_freefunction.cpp
 *  \section Examples
 *  \subsection example_freefunctions Freefunctions Approach
 *  \snippet example_freefunction.cpp general
  * \subsubsection example_easyfunc_stdout Easy to use initialization function for `stdout`
 *  \snippet example_freefunction.cpp stdout
 * \subsubsection example_easyfunc_file Easy to use initialization function for `file`
 *  \snippet example_freefunction.cpp file
 * 
 *@see `cof::logger::g_enabled` \copyright  The MIT License
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
 *
 **/
#include <cassert>
#include <type_traits>
#include "BasicLogger.h"
#include "Details/GlobalLoggerInstance.h"
namespace cof
{
namespace basic_logger
{
    //class forwarding:
class StdOut;
class File;
struct FileSinkSettings;

/**
 * \brief Initialization function
 * \param settings Setting for the sink
 * \tparam S Sink type
 * \tparam SS Sink Setting type
 * \warning static_assert if Sink is not base on `Sink`: `Error: The used Sink is not compatible with the Logger! It needs to be of the type cof::Sink`
 * \warning Only one function call is possible hence only one initialization otherwise it will cause a runtime assert.
 */
template <typename S, typename SS>
void Init(SS&& settings)
{
    static_assert(std::is_base_of_v<Sink, S>, "Error: The used Sink is not compatible with the Logger! It needs to be of the type cof::Sink");
    if (!s_logger)
    {
        s_logger = new Logger(new S{new SS{std::forward<SS>(settings)}});
    }
    else
    {
        // Only supports 1 Sink at the same time!
        assert(false);
    }
};


    void InitStdOut();

    void InitFile(cof::basic_logger::FileSinkSettings&& settings);

    void Deint();
}
/**
 * \def COF_CHECK_IF_LOGGER_IS_VALID()
 * \brief check if logger is initialized if not runtime assert!
 */
#define COF_CHECK_IF_LOGGER_IS_VALID() \
    if (!basic_logger::s_logger)       \
        assert(false);
// Logging Functions:

/**
 * \brief Logging function.
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param args Variadic arguments for the replacements. e.g. time etc.
 * \tparam Args Variadic template argument important for the format replacements.
 * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
 * \warning Will `assert` if basic_logger::s_logger is`**NOT** valid!
 * \warning if not in debug mode and `COF_USE_LOGGER` is not set the function will compile empty.
 */
template <typename... Args>
void Log(const char* format, const Args&... args)
{
    if (cof::basic_logger::g_enabled)
    {
        COF_CHECK_IF_LOGGER_IS_VALID()
        basic_logger::s_logger->Log(format, args...);
    }
}
/**
 * \brief Logging function.
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param args Variadic arguments for the replacements. e.g. time etc.
 * \tparam Args Variadic template argument important for the format replacements.
 * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
 * \warning Will `assert` if basic_logger::s_logger is`**NOT** valid!
 * \warning if not in debug mode and `COF_USE_LOGGER` is not set the function will compile empty.
 */
template <typename... Args>
void Debug(const char* format, const Args&... args)
{
    if (cof::basic_logger::g_enabled)
    {
        COF_CHECK_IF_LOGGER_IS_VALID()
        basic_logger::s_logger->Debug(format, args...);
    }
}
/**
 * \brief Logging function.
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param args Variadic arguments for the replacements. e.g. time etc.
 * \tparam Args Variadic template argument important for the format replacements.
 * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
 * \warning Will `assert` if basic_logger::s_logger is`**NOT** valid!
 * \warning if not in debug mode and `COF_USE_LOGGER` is not set the function will compile empty.
 */
template <typename... Args>
void Warn(const char* format, const Args&... args)
{
    if (cof::basic_logger::g_enabled)
    {
        COF_CHECK_IF_LOGGER_IS_VALID()
        basic_logger::s_logger->Warn(format, args...);
    }
}
/**
 * \brief Logging function.
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param args Variadic arguments for the replacements. e.g. time etc.
 * \tparam Args Variadic template argument important for the format replacements.
 * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
 * \warning Will `assert` if basic_logger::s_logger is`**NOT** valid!
 * \warning if not in debug mode and `COF_USE_LOGGER` is not set the function will compile empty.
 */
template <typename... Args>
void Info(const char* format, const Args&... args)
{
    if (cof::basic_logger::g_enabled)
    {
        COF_CHECK_IF_LOGGER_IS_VALID()
        basic_logger::s_logger->Info(format, args...);
    }
}
/**
 * \brief Logging function.
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param args Variadic arguments for the replacements. e.g. time etc.
 * \tparam Args Variadic template argument important for the format replacements.
 * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
 * \warning Will `assert` if basic_logger::s_logger is`**NOT** valid!
 * \warning if not in debug mode and `COF_USE_LOGGER` is not set the function will compile empty.
 */
template <typename... Args>
void Error(const char* format, const Args&... args)
{
    if (cof::basic_logger::g_enabled)
    {
        COF_CHECK_IF_LOGGER_IS_VALID()
        basic_logger::s_logger->Error(format, args...);
    }
}

#undef COF_CHECK_IF_LOGGER_IS_VALID
}
