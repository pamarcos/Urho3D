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

#include "RCCppGppCompiler.h"
#include "Log.h"
#include "FileSystem.h"
#include "File.h"

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#elif defined(_MINGW32__)
#include <windows.h>
#endif

namespace Urho3D
{

const String RCCppGppCompiler::makefile_ =  ""
        "-include PreMakefile\n"
        "\n"
        "CXX := g++\n"
        "\n"
#ifdef DEBUG
        "CPPFLAGS ?= -g\n"
#else
        "CPPFLAGS ?= -O2\n"
#endif
        "\n"
        "CPPFLAGS := -fPIC $(CPPFLAGS) -DURHO3D_LOGGING $(EXT_DEFINES) $(EXT_CPPFLAGS) \\\n"
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
#if defined(__APPLE__) || defined(__linux__)
        "LDFLAGS := -shared -L$(URHO3D_HOME)/Lib -lUrho3D $(EXT_LDFLAGS)\n"
#elif defined(__MINGW32__)
        "LDFLAGS := -shared -L$(URHO3D_HOME)/Bin -lUrho3D"
#ifdef DEBUG
        "_d"
#endif
        " $(EXT_LDFLAGS)\n"
#endif
#if defined(__APPLE__) || defined(__linux__)
        "RM := rm -rf\n"
#elif defined(__MINGW32__)
        "RM := rmdir /S /Q\n"
#endif
        "TARGET_LIB = $(LIB_NAME)\n"
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
        "	$(RM) $(OBJ_DIR) $(LIB_NAME)\n"
        "\n"
        "PHONY: clean";

RCCppGppCompiler::RCCppGppCompiler(Context* context) :
    RCCppCompiler(context)
{
    fileSystem_ = context_->GetSubsystem<FileSystem>();

#ifdef __MINGW32__
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int cores_ = sysinfo.dwNumberOfProcessors;
    makeCommand_ = "mingw32-make -j" + String(cores_);
#else
    int cores_ = sysconf(_SC_NPROCESSORS_ONLN);
    makeCommand_ = "make -j" + String(cores_);
#endif
}

bool RCCppGppCompiler::CreateMakefile(const String& fileName, const String& libraryName)
{
    File makefile(context_);
    if (!makefile.Open(fileName, FILE_WRITE))
    {
        return false;
    }
    String finalMakefile = makefile_.Replaced("$(LIB_NAME)", libraryName);
    if (!makefile.WriteString(finalMakefile))
    {
        return false;
    }
    makefile.Flush();
    makefile.Close();
}

bool RCCppGppCompiler::Compile(const RCCppFile &file, const String& libraryPath)
{
    if (file.IsMainFile())
    {
        CreateMakefile(GetParentPath(libraryPath) +  "Makefile", GetFileNameAndExtension(libraryPath));
    }

    LOGDEBUG("RCCpp: Compiling using command line: " + makeCommand_ + "\n");
    String origDir = fileSystem_->GetCurrentDir();
    fileSystem_->SetCurrentDir(GetParentPath(libraryPath).CString());
    String buildLog = "Build.log";
    system(String(makeCommand_ + " 1> " +  buildLog + " 2> " + buildLog).CString());
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

}
