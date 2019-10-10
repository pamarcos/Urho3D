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

#ifndef RCCPPGPPCOMPILER_H
#define RCCPPGPPCOMPILER_H

#include "../Core/Object.h"
#include "../Core/Context.h"
#include "RCCppFile.h"
#include "RCCppCompiler.h"

namespace Urho3D
{

class FileSystem;

class URHO3D_API RCCppGppCompiler : public RCCppCompiler
{
    OBJECT(RCCppGppCompiler);

public:
    RCCppGppCompiler(Context* context);

    bool CreateMakefile(const String& fileName, const String& libraryName);
    bool Compile(const RCCppFile& file, const String& libraryPath, String& output);

private:
    String makeCommand_;
    static const String makefile_;
    int cores_;
    FileSystem* fileSystem_;
};

}
#endif // RCCPPGPPCOMPILER_H