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
#include "../IO/Log.h"
#include "../IO/FileSystem.h"
#include "../IO/File.h"

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#elif defined(__MINGW32__)
#include <windows.h>
#endif

#include <stdio.h>

namespace Urho3D
{

const String RCCppGppCompiler::makefile_ =  ""
        "-include PreMakefile\n"
        "\n"
        "CXX := g++\n"
        "\n"
#ifdef _DEBUG
        "CPP_DEBUG_FLAGS ?= -g\n"
#else
        "CPP_DEBUG_FLAGS ?= -O2\n"
#endif
#ifdef __x86_64__
        "CPPFLAGS := -m64\n"
#else
        "CPPFLAGS := -m32\n"
#endif
        "\n"
        "CPPFLAGS := -fPIC $(CPP_DEBUG_FLAGS) $(CPPFLAGS) -DURHO3D_LOGGING -DRCCPP $(EXT_DEFINES) $(EXT_CPPFLAGS) \\\n"
        "-I. \\\n"
        "-I$(URHO3D_HOME)/include \\\n"
        "-I$(URHO3D_HOME)/include/Urho3D/ThirdParty \\\n"
        "\n"
#ifdef __x86_64__
        "LDFLAGS := -m64\n"
#else
        "LDFLAGS := -m32\n"
#endif
#if defined(__APPLE__) || defined(__linux__)
        "LDFLAGS := $(LDFLAGS) -shared -L$(URHO3D_HOME)/lib -lUrho3D $(EXT_LDFLAGS)\n"
#elif defined(__MINGW32__)
        "LDFLAGS := $(LDFLAGS) -shared -L$(URHO3D_HOME)/Bin -lUrho3D"
#ifdef DEBUG
        "_d"
#endif
        " $(EXT_LDFLAGS)\n"
#endif
#if defined(__APPLE__) || defined(__linux__)
        "RMDIR := rm -rf\n"
        "RM := $(RMDIR)\n"
#elif defined(__MINGW32__)
        "RMDIR := rmdir /S /Q\n"
        "RM := del /S /Q\n"
#endif
        "TARGET_LIB := $(LIB_NAME)\n"
        "\n"
        "SUBDIRS := $(wildcard */)\n"
        "SOURCES := $(wildcard *.cpp $(addsuffix *.cpp,$(SUBDIRS)))\n"
        "OBJ_DIR := obj\n"
        "OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(SOURCES:.cpp=.o)))\n"
        "\n"
        "all: $(TARGET_LIB)\n"
        "\t@echo Compilation complete\n"
        "\n"
        "$(TARGET_LIB): $(OBJ_FILES)\n"
        "\t@echo Linking objects\n"
        "\t$(CXX) $(LDFLAGS) -o $@ $^\n"
        "\n"
        "$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)\n"
        "\t@echo Compiling file $@\n"
        "\t$(CXX) $(CPPFLAGS) -c -o $@ $^\n"
        "\n"
        "$(OBJ_DIR)/%.o: */%.cpp | $(OBJ_DIR)\n"
        "\t@echo Compiling file $@\n"
        "\t$(CXX) $(CPPFLAGS) -c -o $@ $^\n"
        "\n"
        "$(OBJ_DIR):\n"
        "\tmkdir -p $(OBJ_DIR)\n"
        "\n"
        "clean:\n"
        "\t$(RMDIR) $(OBJ_DIR)\n"
        "\t$(RM) $(TARGET_LIB)\n"
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
    return true;
}

bool RCCppGppCompiler::Compile(const RCCppFile &file, const String& libraryPath, String& output)
{
    output.Clear();
    if (file.IsMainFile())
    {
        CreateMakefile(GetParentPath(libraryPath) +  "Makefile", GetFileNameAndExtension(libraryPath));
    }

    LOGDEBUG("RCCpp: Compiling using command line: " + makeCommand_ + "\n");
    String origDir = fileSystem_->GetCurrentDir();
    fileSystem_->SetCurrentDir(GetParentPath(libraryPath).CString());
    String buildLog = "Build.log";

#ifdef __MINGW32__
#define popen _popen
#define pclose _pclose
#endif

    FILE* pipe = popen((makeCommand_ + " 2>&1").CString(), "r");
    if (!pipe)
    {
        return false;
    }
    char buffer[128];
    while (!feof(pipe))
    {
        if (fgets(buffer, sizeof(buffer), pipe) != NULL)
        {
            output.Append(buffer);
        }
    }
    pclose(pipe);
    File makefileOut(context_);
    if (makefileOut.Open(buildLog, FILE_WRITE))
    {
        makefileOut.WriteString(output);
    }
    makefileOut.Flush();
    makefileOut.Close();

    fileSystem_->SetCurrentDir(origDir);

    LOGDEBUG(output);

    if (output.Contains("error", false))
    {
        return false;
    }
    else
    {
        return true;
    }
}

}
