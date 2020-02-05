#pragma once
/**
 *  \file
 *  \brief     Contains the basic impl of the logger class
 *  \details   see class description
 *  \example   example_object.cpp
 *  \author    Simon Renger
 *  \version   1.0
 *  \date      2018
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
 *
 **/
#include "Sinks/Sink.h"
#include "fmt/format.h"

#include <string>
#include <memory>

namespace cof
{
/*
 *  \brief Implementation namespace
 *  \details Is the home of the basic implementation of the logger class as well as all its utilities.
 */
namespace basic_logger
{
#if defined(__DEBUG) || defined(__DEBUG__) || defined(_DEBUG_) || defined(DEBUG_) || defined(_DEBUG) || defined(COF_USE_LOGGER)
/**
 * \class Logger
 * \brief     A basic non multi threaded logger.
 * \details   This is the basic logger imnpl. See [fmt](https://github.com/fmtlib/fmt/) for more information of the formatting. **This is a move only type.**
 * \author    Simon Renger
 * \version   1.0
 * \date      2018
 * \pre       If used outside of Debug mode the macro `COF_USE_LOGGER`  needs to be defined. Otherwise all functions/methods will leave a zero fingerprint.
 * \warning Move only type.
 * \section example Example
 * \include example_object.cpp
 */
class Logger
{
   public:
    Logger() = delete;
    Logger(const Logger& other) = delete;
    Logger& operator=(const Logger& other) = delete;

    Logger(Logger&& other) noexcept : sink_(std::move(other.sink_)) {}

    Logger& operator=(Logger&& other) noexcept
    {
        sink_ = std::move(other.sink_);

        return *this;
    }

    /*
     * \param Sink a pointer of type basic_logger::Sink. Takes Ownership!
     */
    Logger(basic_logger::Sink* sink) : sink_(sink) {}
    /**
     * \brief Logging function.
     * \param formStr ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
     * \param args Variadic arguments for the replacements. e.g. time etc.
     * \tparam Args Variadic template argument important for the format replacements. 
     * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
     */
    template <typename... Args>
    void Log(const char* formStr, const Args&... args)
    {
        sink_->SinkIn(Level::LOG, std::move(fmt::vformat(formStr, fmt::make_format_args(args...))));
    }
    /**
     * \brief Logging function.
     * \param formStr ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
     * \param args Variadic arguments for the replacements. e.g. time etc.
     * \tparam Args Variadic template argument important for the format replacements.
     * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
     */
    template <typename... Args>
    void Warn(const char* formStr, const Args&... args)
    {
        sink_->SinkIn(Level::WARN, std::move(fmt::vformat(formStr, fmt::make_format_args(args...))));
    }
    /**
     * \brief Logging function.
     * \param formStr ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
     * \param args Variadic arguments for the replacements. e.g. time etc.
     * \tparam Args Variadic template argument important for the format replacements.
     * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
     */
    template <typename... Args>
    void Debug(const char* formStr, const Args&... args)
    {
        sink_->SinkIn(Level::DEBUG, std::move(fmt::vformat(formStr, fmt::make_format_args(args...))));
    }
    /**
     * \brief Logging function.
     * \param formStr ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
     * \param args Variadic arguments for the replacements. e.g. time etc.
     * \tparam Args Variadic template argument important for the format replacements.
     * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
     */
    template <typename... Args>
    void Info(const char* formStr, const Args&... args)
    {
        sink_->SinkIn(Level::INFO, std::move(fmt::vformat(formStr, fmt::make_format_args(args...))));
    }
    /**
     * \brief Logging function.
     * \param formStr ftm style format. e.g. `User #ID{} logged in at {}. with the IP{}`
     * \param args Variadic arguments for the replacements. e.g. time etc.
     * \tparam Args Variadic template argument important for the format replacements.
     * \see cof::basic_logger::Sink::SinkIn() for more information on the implementation of the actual logging
     */
    template <typename... Args>
    void Error(const char* formStr, const Args&... args)
    {
        sink_->SinkIn(Level::ERR, std::move(fmt::vformat(formStr, fmt::make_format_args(args...))));
    }

   private:
    std::unique_ptr<basic_logger::Sink> sink_;
};
#else
class Logger
{
   public:
    Logger() = delete;

    Logger(const Logger& other) = default;

    Logger(Logger&& other) = default;
    Logger& operator=(const Logger& other) = default;

    Logger& operator=(Logger&& other) noexcept = default;

    Logger(basic_logger::Sink* sink) {}
    ~Logger() = default;

    template <typename... Args>
    void Log(const char* formStr, const Args&... args)
    {
    }
    template <typename... Args>
    void Warn(const char* formStr, const Args&... args)
    {
    }
    template <typename... Args>
    void Debug(const char* formStr, const Args&... args)
    {
    }
    template <typename... Args>
    void Info(const char* formStr, const Args&... args)
    {
    }
    template <typename... Args>
    void Error(const char* formStr, const Args&... args)
    {
    }
};
#endif
};
}
