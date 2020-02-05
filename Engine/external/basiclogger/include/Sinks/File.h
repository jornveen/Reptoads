#pragma once
/**
 *  \file
 *  \brief     File Sink and FileSink Settings are defined here.
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
 **/
#include "Sinks/Sink.h"

namespace cof
{
    namespace basic_logger
    {
        /**
         *  \brief     Defines settings for the File logger
         *  \details Can set the logging file and its write more. Important for the I/O process. Default is `a+` append to an file. If not exists creates file.
         *  \ingroup  Sinks_Settings
         *  \author    Simon Renger
         *  \version   1.0
         *  \date      2018
         *  \copyright  The MIT License
         */
        struct FileSinkSettings : SinkSettings
        {
            FileSinkSettings() :FileStr{} {};
            FileSinkSettings(const char * FileStr) :FileStr{ FileStr } {};
            FileSinkSettings(FileSinkSettings&& other) noexcept : SinkSettings(std::move(other))
            {
                FileStr = other.FileStr;
            }
            std::string FileStr {};
            std::string FileMode { "a+" };
        };
        /**
         *  \brief     The actual File logger
         *  \details  Logs to a specified file with `fwrite` and `File*`. This class uses RAII. Hence `~File` is cleaning up the mess.
         *  \ingroup  Sinks
         *  \author    Simon Renger
         *  \version   1.0
         *  \date      2018
         *  \copyright  The MIT License
         */
        class File : public Sink
        {
        public:
            File(SinkSettings* settings);
            ~File();
        protected:
            virtual void Process(Level lvl, fmt::memory_buffer&& buffer) override final;
        private:
            
            FileSinkSettings settings_;
            FILE* file_;
        };
    };
};