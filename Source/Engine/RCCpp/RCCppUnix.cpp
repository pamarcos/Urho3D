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

#if defined(__APPLE__) | defined(__linux__)

#include "RCCppUnix.h"
#include "Log.h"
#include "Urho3D.h"
#include "FileSystem.h"
#include "ResourceCache.h"
#include "RCCppGppCompiler.h"

#include <stdio.h>
#include "dlfcn.h"

namespace Urho3D
{

RCCppUnix::RCCppUnix(Context* context) :
    RCCppImpl(context),
    library_(NULL),
    createObject_(NULL),
    destroyObject_(NULL),
    compiler_(new RCCppGppCompiler(context_))
{
}

RCCppUnix::~RCCppUnix()
{
}

bool RCCppUnix::Compile(const RCCppFile& file, const String& libraryPath, String& output)
{
    return compiler_->Compile(file, libraryPath, output);
}

RCCppObject* RCCppUnix::CreateObject(const String &objectName)
{
    if (createObject_ != NULL)
    {
        return createObject_(context_);
    }
    else
    {
        LOGERROR("Couldn't find any create method for RCCpp object " + objectName);
        return NULL;
    }
}

void RCCppUnix::DestroyObject(RCCppObject *object)
{
    if (object != NULL)
    {
        if (destroyObject_ != NULL)
        {
            destroyObject_(object);
        }
        else
        {
            LOGERROR("Couldn't find any destroy method for RCCpp object " + object->GetTypeName());
        }
    }
}

bool RCCppUnix::LoadLib(const String& libraryPath)
{
    library_ = dlopen(libraryPath.CString(), RTLD_LAZY);
    if (library_ != NULL)
    {
        String name = GetFileName(libraryPath);
        createObject_ = (PCreateRCCppObject)dlsym(library_, String("create" + name).CString());
        destroyObject_ = (PDestroyRCCppObject)dlsym(library_, String("destroy" + name).CString());
    }

    if (library_ == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void RCCppUnix::UnloadLib()
{
    if (library_ != NULL)
    {
        dlclose(library_);
        library_ = NULL;
        mainObject_ = NULL;
    }
}

}

#endif
