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

#include "RCCppUnix.h"
#include "Log.h"
#include "Urho3D.h"

#include "dlfcn.h"

namespace Urho3D
{

RCCppUnix::RCCppUnix(Context* context) :
    RCCppImpl(context),
    library_(NULL),
    object_(NULL),
    createObject_(NULL),
    destroyObject_(NULL)
{
}

RCCppUnix::~RCCppUnix()
{
    UnloadLibrary();
}

bool RCCppUnix::Compile(const String &fileName)
{
    UnloadLibrary();

    String libName = fileName.Substring(0, fileName.Find(".cpp")) + ".so";
    String compilerString = GetCompilerLine(fileName) + " " + fileName + " -o " + libName;

    LOGDEBUG("RCCpp: Compiling using command line:\n" + compilerString + "\n");
    system(compilerString.CString());

    return LoadLibrary(libName);
}

bool RCCppUnix::LoadLibrary(const String &libName)
{
    library_ = dlopen(libName.CString(), RTLD_LAZY);
    if (library_ != NULL)
    {
        createObject_ = (PCreateRCCppMainObject)dlsym(library_, "create");
        destroyObject_ = (PDestroyRCCppMainObject)dlsym(library_, "destroy");

        object_ = createObject_(context_);
    }

    if (library_ == NULL || createObject_ == NULL || destroyObject_ == NULL || object_ == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void RCCppUnix::UnloadLibrary()
{
    if (library_ != NULL)
    {
        if (destroyObject_ != NULL && object_ != NULL)
        {
            destroyObject_(object_);
        }
        dlclose(library_);
        library_ = NULL;
        object_ = NULL;
        createObject_ = NULL;
        destroyObject_ = NULL;
    }
}

void RCCppUnix::Start()
{
    if (object_ != NULL)
    {
        object_->Start();
    }
}

void RCCppUnix::Stop()
{
    if (object_ != NULL)
    {
        object_->Stop();
    }
}

String RCCppUnix::getCompilerSharedLib()
{
    return "g++ -g -fPIC -shared";
}

String RCCppUnix::getIncPathPrefix()
{
    return "-I";
}

String RCCppUnix::getLibPathPrefix()
{
    return "-L";
}

String RCCppUnix::getLinkLibPrefix()
{
    return "-l";
}

String RCCppUnix::getDefinePrefix()
{
    return "-D";
}

}
