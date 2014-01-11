
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
// Copyright (c) 2014 Francois Beaune, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef APPLESEED_UPDATEPROJECTFILE_COMMANDLINEHANDLER_H
#define APPLESEED_UPDATEPROJECTFILE_COMMANDLINEHANDLER_H

// appleseed.foundation headers.
#include "foundation/platform/compiler.h"
#include "foundation/utility/commandlineparser.h"

// appleseed.shared headers.
#include "application/commandlinehandlerbase.h"

// Standard headers.
#include <string>

// Forward declarations.
namespace appleseed { namespace shared { class SuperLogger; } }

namespace appleseed {
namespace updateprojectfile {

//
// Command line handler.
//

class CommandLineHandler
  : public shared::CommandLineHandlerBase
{
  public:
    foundation::ValueOptionHandler<std::string> m_filename;
    foundation::ValueOptionHandler<int>         m_to_revision;

    // Constructor.
    CommandLineHandler();

    // Parse the application's command line.
    virtual void parse(
        const int               argc,
        const char*             argv[],
        shared::SuperLogger&    logger) OVERRIDE;

  private:
    // Emit usage instructions to the logger.
    virtual void print_program_usage(
        const char*             program_name,
        shared::SuperLogger&    logger) const;
};

}       // namespace updateprojectfile
}       // namespace appleseed

#endif  // !APPLESEED_UPDATEPROJECTFILE_COMMANDLINEHANDLER_H
