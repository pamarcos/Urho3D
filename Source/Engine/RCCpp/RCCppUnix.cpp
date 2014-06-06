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
#include "File.h"
#include "FileSystem.h"
#include "ResourceCache.h"

#include <stdio.h>
#include <unistd.h>

#include "dlfcn.h"

namespace Urho3D
{

const String RCCppUnix::makefile_ =  ""
        "-include PreMakefile\n"
        "\n"
        "CXX := g++\n"
        "\n"
        "CPPFLAGS_DEBUG ?= -g\n"
        "\n"
        "CPPFLAGS := -fPIC $(CPPFLAGS_DEBUG) -DURHO3D_LOGGING $(EXT_DEFINES) $(EXT_CPPFLAGS) \\\n"
        "-I$(URHO3D_HOME). \\\n"
        "-I$(URHO3D_HOME)/Build/Engine \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/RCCpp \\\n"
        "-I$(URHO3D_HOME)/Source/Engine \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Audio \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Container \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Core  \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Engine  \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Graphics \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Input  \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/IO \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Math \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Navigation \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Network \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Physics \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Resource \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Scene \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/UI \\\n"
        "-I$(URHO3D_HOME)/Source/Engine/Urho2D \\\n"
        "-I$(URHO3D_HOME)/Source/ThirdParty/Box2D \\\n"
        "-I$(URHO3D_HOME)/Source/ThirdParty/Bullet/src \\\n"
        "-I$(URHO3D_HOME)/Source/ThirdParty/kNet/include \\\n"
        "-I$(URHO3D_HOME)/Source/ThirdParty/SDL/include\n"
        "\n"
        "LDFLAGS := -shared -L$(URHO3D_HOME)/Lib -lUrho3D $(EXT_LDFLAGS)\n"
        "RM := rm -rf\n"
        "TARGET_LIB = $LIB_NAME\n"
        "\n"
        "SUBDIRS := $(wildcard */)\n"
        "SOURCES := $(wildcard *.cpp $(addsuffix *.cpp,$(SUBDIRS)))\n"
        "OBJ_DIR := obj\n"
        "OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(SOURCES:.cpp=.o)))\n"
        "\n"
        "all: $(TARGET_LIB)\n"
        "	@echo Compilation complete\n"
        "\n"
        "$(TARGET_LIB): $(OBJ_FILES)\n"
        "	@echo Linking objects\n"
        "	$(CXX) $(LDFLAGS) -o $@ $^\n"
        "\n"
        "$(OBJ_DIR)/%.o: %.cpp\n"
        "	@mkdir -p $(OBJ_DIR)\n"
        "	@echo Compiling file $@\n"
        "	$(CXX) $(CPPFLAGS) -c -o $@ $^\n"
        "\n"
        "$(OBJ_DIR)/%.o: */%.cpp\n"
        "	@mkdir -p $(OBJ_DIR)\n"
        "	@echo Compiling file $@\n"
        "	$(CXX) $(CPPFLAGS) -c -o $@ $^\n"
        "\n"
        "clean:\n"
        "	$(RM) $(OBJ_DIR)";

RCCppUnix::RCCppUnix(Context* context) :
    RCCppImpl(context),
    library_(NULL),
    mainObject_(NULL),
    createObject_(NULL),
    destroyObject_(NULL)
{
    fileSystem_ = context_->GetSubsystem<FileSystem>();
}

RCCppUnix::~RCCppUnix()
{
    UnloadLibrary();
}

bool RCCppUnix::CreateMakefile(const String& fileName)
{
    File makefile(context_);
    if (!makefile.Open(fileName, FILE_WRITE))
    {
        return false;
    }
    String finalMakefile = makefile_.Replaced("$LIB_NAME", libraryName_ + ".so");
    if (!makefile.WriteString(finalMakefile))
    {
        return false;
    }
    makefile.Flush();
    makefile.Close();
}

const String RCCppUnix::GetLibraryPath()
{
    return libraryPath_;
}

bool RCCppUnix::Compile(const RCCppFile& file)
{
    if (file.IsMainFile())
    {
        libraryName_ = GetFileName(file.GetName());
        ResourceCache* cache = context_->GetSubsystem<ResourceCache>();
        String mainCppFilePath = cache->GetResourceFileName(file.GetName()) ;
        String path, name, ext;
        SplitPath(mainCppFilePath, path, name, ext);
        libraryPath_ = path+ name + ".so";
        CreateMakefile(GetParentPath(libraryPath_) +  "Makefile");
    }

    int cores =  sysconf(_SC_NPROCESSORS_ONLN);
    String makeLine = "make -j" + String(cores);
    LOGDEBUG("RCCpp: Compiling using command line: " + makeLine + "\n");
    String origDir = fileSystem_->GetCurrentDir();
    fileSystem_->SetCurrentDir(GetParentPath(libraryPath_).CString());
    String buildLog = "Build.log";
    system(String(makeLine + " 1> " +  buildLog + " 2> " + buildLog).CString());
    String result;
    File makefileOut(context_);
    if (makefileOut.Open(buildLog))
    {
        result = makefileOut.ReadString();
    }
    fileSystem_->SetCurrentDir(origDir);

    LOGDEBUG(result);

    if (result.Contains("error", false))
    {
        return false;
    }
    else
    {
        return true;
    }
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

bool RCCppUnix::LoadLibrary(const String& fileName)
{
    library_ = dlopen(fileName.CString(), RTLD_LAZY);
    if (library_ != NULL)
    {
            String name = GetFileName(fileName);
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

void RCCppUnix::UnloadLibrary()
{
    if (library_ != NULL)
    {
        dlclose(library_);
        library_ = NULL;
        mainObject_ = NULL;
    }
}

void RCCppUnix::Start()
{
    if (mainObject_ == NULL)
    {
        mainObject_ = static_cast<RCCppMainObject*>(CreateObject(libraryName_));
    }
    if (mainObject_ != NULL)
    {
        mainObject_->Start();
    }
    else
    {
        LOGERROR("Error creating main RCCpp object " + libraryName_);
    }
}

void RCCppUnix::Stop()
{
    if (mainObject_ != NULL)
    {
        mainObject_->Stop();
        DestroyObject(mainObject_);
        mainObject_ = NULL;
    }
}

}
