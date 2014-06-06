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

#ifndef RCCPPUNIX_H
#define RCCPPUNIX_H

#include "Object.h"
#include "Context.h"
#include "RCCppImpl.h"
#include "RCCppMainObject.h"

namespace Urho3D
{

class RCCppObject;
class FileSystem;

class URHO3D_API RCCppUnix : public RCCppImpl
{
    OBJECT(RCCppUnix);

public:
    RCCppUnix(Context* context);
    ~RCCppUnix();

    /// Execute script file. Return true if successful.
    bool ExecuteFile(const String& fileName);
    void Start();
    void Stop();

    bool Compile(const RCCppFile& file);
    bool LoadLibrary(const String& fileName);
    void UnloadLibrary();

    virtual RCCppObject* CreateObject(const String& objectName);
    virtual void DestroyObject(RCCppObject* object);

    const String GetLibraryPath();

private:
    bool CreateMakefile(const String& fileName);

    void* library_;
    String libraryName_;
    String libraryPath_;
    RCCppMainObject* mainObject_;
    FileSystem* fileSystem_;

    PCreateRCCppObject createObject_;
    PDestroyRCCppObject destroyObject_;

    static const String makefile_;
};

}

#endif // RCCPPUNIX_H
