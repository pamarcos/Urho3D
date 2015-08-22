//
// Copyright (c) 2008-2014 the Urho3D project.
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

#ifndef RCCPPEVENTS_H
#define RCCPPEVENTS_H

#include "../Core/Object.h"

namespace Urho3D
{

EVENT(E_RCCPP_COMPILATION_STARTED, RCCppCompilationStarted)
{
    PARAM(P_FILE, CompileFile);
}

EVENT(E_RCCPP_COMPILATION_FINISHED, RCCppCompilationFinished)
{
    PARAM(P_SUCCESSFUL, CompilationSuccessful);
    PARAM(P_FILE, CompiledFile);
    PARAM(P_OUTPUT, CompilationOutput);
}

EVENT(E_RCCPP_LIBRARY_PRELOADED, RCCppLibraryPreloaded)
{
    PARAM(P_LIBRARY, LibraryName);
}

EVENT(E_RCCPP_LIBRARY_POSTLOADED, RCCppLibraryPostLoaded)
{
    PARAM(P_LIBRARY, LibraryName);
}

EVENT(E_RCCPP_CLASS_PRELOADED, RCCppClassPreLoaded)
{
    PARAM(P_CLASS_NAME, ClassName);
}

EVENT(E_RCCPP_CLASS_POSTLOADED, RCCppClassPostLoaded)
{
    PARAM(P_CLASS_NAME, ClassName);
}

}

#endif // RCCPPEVENTS_H
