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

#if defined(WIN32)

#include "../Core/RCCppWin.h"
#include "../IO/Log.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceCache.h"

#ifdef __MINGW32__
#include "RCCppGppCompiler.h"
#endif

#include <stdio.h>
#include <windows.h>

namespace Urho3D
{

RCCppWin::RCCppWin(Context* context) :
    RCCppImpl(context),
    library_(NULL),
    createObject_(NULL),
    destroyObject_(NULL),
    firstCompilation_(true),
    fileSystem_(context_->GetSubsystem<FileSystem>())
{
#ifdef __MINGW32__
    compiler_ = new RCCppGppCompiler(context_);
#else
#error "MSVC not supported"
#endif
}

RCCppWin::~RCCppWin()
{
    UnloadLib();
}

bool RCCppWin::Compile(const RCCppFile& file, const String& libraryPath, String& output)
{
    // Windows throws a file access error when trying to compile a loaded library.
    // It allows to change its name, though. So, we change its name, compile and then remove
    // the file when we unload the library.
    if (!firstCompilation_)
    {
        oldLibPath_ = libraryPath + ".old";
        fileSystem_->Rename(libraryPath, oldLibPath_);
    }
    firstCompilation_ = false;
    return compiler_->Compile(file, libraryPath, output);
}

RCCppObject* RCCppWin::CreateObject(const String &objectName)
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

void RCCppWin::DestroyObject(RCCppObject *object)
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

bool RCCppWin::LoadLib(const String& libraryPath)
{
    if (library_ != NULL)
    {
        UnloadLib();
    }
    library_ = LoadLibrary(libraryPath.CString());
    if (library_ != NULL)
    {
        String name = GetFileName(libraryPath);
        createObject_ = (PCreateRCCppObject)GetProcAddress(library_, String("create" + name).CString());
        destroyObject_ = (PDestroyRCCppObject)GetProcAddress(library_, String("destroy" + name).CString());
        return true;
    }
    else
    {
        return false;
    }
}

void RCCppWin::UnloadLib()
{
    if (library_ != NULL)
    {
        FreeLibrary(library_);
        library_ = NULL;
        mainObject_ = NULL;
        createObject_ = NULL;
        destroyObject_ = NULL;

        if (!oldLibPath_.Empty())
        {
            fileSystem_->Delete(oldLibPath_);
            oldLibPath_.Clear();
        }
    }
}

}

#else
// To avoid ranlib complaining about RCCppWin.cpp having no symbols
void RCCppWinDummy() {}
#endif
