#pragma once
/**
 *  \file
 *  \brief     Sink class as well as `Level enum class`
 *  \details   if `COF_USE_LOGGER` **(Outside of Debug mode)** is used/set the variable `g_enabled` is set to true and therefor enables logging if set to false guarantees zero fingerprint.(Disabled logging)
 *  \author    Simon Renger
 *  \version   1.0
 *  \date      2018
 *  \copyright  The MIT License
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
 *
 * \defgroup Sinks
 * \brief Logger backend
 * \details Sinks are the objects that actually write the log to their target. Each sink should be responsible for only single target (e.g file, console, db).
 * 
 * \defgroup Sinks_Settings Sink Settings
 * \brief Settings for Sinks
 * \details Define how Sinks can behave. For example what file the sink should log to etc.
 *
 **/
#include <array>
#include <string>
#include "fmt/format.h"




namespace cof
{
namespace basic_logger
{

enum class Level
{
    LOG,
    INFO,
    DEBUG,
    ERR,
    WARN
};

size_t inline GetLevel(const Level& level)
{
    return static_cast<size_t>(level);
}

#if defined(COF_USE_LOGGER) || defined(__DEBUG) || defined(__DEBUG__) || defined(_DEBUG_) || defined(DEBUG_) || defined(_DEBUG)
constexpr bool g_enabled = true;
#else

constexpr bool g_enabled = false;
#endif
/**
 *  \brief     The Base SinkSettings class.
 *  \details can be used to send extra information to the Sink.
 *  \ingroup  Sinks_Settings
 *  \author    Simon Renger
 *  \version   1.0
 *  \date      2018
 *  \copyright  The MIT License
 */
struct SinkSettings
{
    SinkSettings() = default;
    SinkSettings(SinkSettings&& /*other*/) noexcept {}
    virtual ~SinkSettings() {};
};
/**
 *  \brief     The Base Sink class.
 *  \author    Simon Renger
  * \ingroup  Sinks
 *  \version   1.0
 *  \date      2018
 *  \copyright  The MIT License
 *  \see [fmt fmt::memory_buffer](https://github.com/fmtlib/fmt/)
 */
class Sink
{
   public:

    virtual ~Sink() = default;
    /**
     *  \brief     format data
     *  \details Uses fmt to format content. Adds the Date as well as the log level to the message. Calls internally `Process`.
     *  \author    Simon Renger
     *  \version   1.0
     *  \date      2018
     *  \copyright  The MIT License
     */
    void SinkIn(Level lvl, std::string&& message);

   protected:
    virtual void Process(Level lvl, fmt::memory_buffer&& message) = 0;
    std::string GetFormattedDate();
	std::array<const char*, 5> levels_{ {"LOG", "INFO", "DEBUG", "ERR", "WARN"} };
};
}
}
