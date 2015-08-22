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

#include "../IO/Log.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceCache.h"

#include "RCCppUnix.h"
#include "RCCppGppCompiler.h"

#include <stdio.h>
#include <dlfcn.h>

namespace Urho3D
{

RCCppUnix::RCCppUnix(Context* context) :
    RCCppImpl(context),
    library_(NULL),
    createObject_(NULL),
    destroyObject_(NULL),
    compiler_(new RCCppGppCompiler(context_)),
    fileSystem_(context_->GetSubsystem<FileSystem>())
{
}

RCCppUnix::~RCCppUnix()
{
    UnloadLib();
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
    if (library_ != NULL)
    {
        UnloadLib();
    }

#ifdef __linux__
    // Hack in case it's Linux. It seems dlopen caches the library name and in case
    // the same library name is opened again (even after closing the library), it returns
    // the same handler (memory address). Hence, every time we need to load the library we
    // use a different name to avoid that happening.
    static bool firstLoad = true;
    if (firstLoad)
    {
        String tmpLibraryPath = libraryPath + ".first";
        fileSystem_->Rename(libraryPath, tmpLibraryPath);
        library_ = dlopen(tmpLibraryPath.CString(), RTLD_LAZY);
        fileSystem_->Rename(tmpLibraryPath, libraryPath);
        firstLoad = false;
    }
    else
    {
        library_ = dlopen(libraryPath.CString(), RTLD_LAZY);
    }
#else
    library_ = dlopen(libraryPath.CString(), RTLD_LAZY);
#endif
    if (library_ != NULL)
    {
        String name = GetFileName(libraryPath);
        createObject_ = (PCreateRCCppObject)dlsym(library_, String("create" + name).CString());
        destroyObject_ = (PDestroyRCCppObject)dlsym(library_, String("destroy" + name).CString());
        return true;
    }
    else
    {
        LOGERROR("Error loading library " + libraryPath + ": " + dlerror());
        return false;
    }
}

void RCCppUnix::UnloadLib()
{
    if (library_ != NULL)
    {
        if (dlclose(library_) != 0)
        {
            LOGERROR("Error closing library: " + String(dlerror()));
        }
        library_ = NULL;
        mainObject_ = NULL;
        createObject_ = NULL;
        destroyObject_ = NULL;
    }
}

}

#endif
