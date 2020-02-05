#pragma once
/**
 *  \file
 *  \brief     Terminal Sink.
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
         *  \brief    Terminal logging.
         *  \details  Loggs with `stdout` to the terminal unless the log level is `Level::ERROR` then it is using `stderr`.
         *  \ingroup  Sinks
         *  \author    Simon Renger
         *  \version   1.0
         *  \date      2018
         *  \copyright  The MIT License
         */
        class StdOut : public Sink
        {
        public:
            StdOut(SinkSettings* settings) { delete settings; }
        protected:
            /// Uses `fwrite` and `fflush`
            void Process(Level lvl, fmt::memory_buffer&& message) override;
        };
    }
}
