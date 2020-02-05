#pragma once
/**
 *  \file LoggingMacros.h
 *  \brief     A collection of logging Macros which resolve into free functions.
 *  \details   Collection of the Init/Deinit Macros and LOG,DEBUG,WARN,ERROR and INFO. If `COF_LOGGER_USE_DATE_LINE` is not set the system will not log file and
line of code.
 *  \author    Simon Renger
 *  \version   1.0
 *  \date      2018
 *  \pre       If `__FILE__` and `__LINE__` shall be logged then the macro `COF_LOGGER_USE_DATE_LINE` needs to be set.
 *  \example   example_macro.cpp
 *  \copyright  The MIT License
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

/**
 * \def LOGGER_INIT(SinkType, Settings)
 * \brief Initialization Macro for the logger. calls the logger free function. The macro: `LOGGER_DEINIT` should be called at the end of the program!
 * \param SinkType defines the sink type e.g. StdOut or File
 * \param Settings: Will be passed through to the sink. Depends on the Sink if required a extra type. Check Sink types.
 * \warning The macro: `LOGGER_DEINIT` should be called at the end of the program! To clean up memory.
 */
#define LOGGER_INIT(SinkType, Settings) cof::basic_logger::Init<SinkType>(std::move(Settings));

#define LOGGER_INIT_STDOUT() cof::basic_logger::Init<cof::basic_logger::StdOut>(cof::basic_logger::SinkSettings{});

#define LOGGER_INIT_FILE(settings) cof::basic_logger::Init<cof::basic_logger::File>(std::move(settings));
/**
 * \def LOGGER_DEINIT
 * \brief Deinitialization Macro for the logger frees the internal allocations.
 */
#define LOGGER_DEINIT cof::logger::Deint();

/**
 * \def #COF_LOGGER_USE_DATE_LINE
 * \brief  If `__FILE__` and `__LINE__` shall be logged then the macro `COF_LOGGER_USE_DATE_LINE` needs to be set.
 */
#ifdef COF_LOGGER_USE_DATE_LINE
/**
 * \def FORMAT(format)
 * \brief Formats the message in a way that it adds the Line and the File in front of it.
 *\warning Works only if `COF_LOGGER_USE_DATE_LINE` is set
 * \note could have a different approach.
 */
#define FORMAT(format) (std::string("File:{} Line:{} \nMessage:") + format).c_str(), __FILE__, __LINE__
#else
/**
 * \def FORMAT(format)
 * \brief Formats the message in a way that it adds the Line and the File in front of it.
 * \warning  Works only if `COF_LOGGER_USE_DATE_LINE` is set
 * \note could have a different approach.
 */
#define FORMAT(format) format
#endif

/**
 * \def LOG(format,...)
 * \brief Log macro calls the equivalent free functions
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param ... Variadic arguments for the replacements. e.g. time etc.
 */
#define LOG(format, ...) cof::Log(FORMAT(format), __VA_ARGS__);
/**
 * \def WARN(format,...)
 * \brief Log macro calls the equivalent free functions
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param ... Variadic arguments for the replacements. e.g. time etc.
 */
#define WARN(format, ...) cof::Warn(FORMAT(format), __VA_ARGS__);
/**
 * \def ERROR(format,...)
 * \brief Log macro calls the equivalent free functions
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param ... Variadic arguments for the replacements. e.g. time etc.
 */
#define ERR(format, ...) cof::Error(FORMAT(format), __VA_ARGS__);
/**
 * \def DEBUG(format,...)
 * \brief Log macro calls the equivalent free functions
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param ... Variadic arguments for the replacements. e.g. time etc.
 */
#define DEBUG(format, ...) cof::Debug(FORMAT(format), __VA_ARGS__);
/**
 * \def INFO(format,...)
 * \brief Log macro calls the equivalent free functions
 * \param format ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
 * \param ... Variadic arguments for the replacements. e.g. time etc.
 */
#define INFO(format, ...) cof::Info(FORMAT(format), __VA_ARGS__);
