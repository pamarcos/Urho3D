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

#include "RCCppImpl.h"

namespace Urho3D
{

RCCppImpl::RCCppImpl(Context* context) :
    Object(context)
{
    String URHO3D_HOME = getenv("URHO3D_HOME");

    incPaths_.Clear();
    incPaths_.Push(URHO3D_HOME + ".");
    incPaths_.Push(URHO3D_HOME + "/Build/Engine");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/RCCpp");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Audio");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Container");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Core");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Engine");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Graphics");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Input");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/IO");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Math");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Navigation");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Network");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Physics");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Resource");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Scene");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/UI");
    incPaths_.Push(URHO3D_HOME + "/Source/Engine/Urho2D");
    incPaths_.Push(URHO3D_HOME + "/Source/ThirdParty/Box2D");
    incPaths_.Push(URHO3D_HOME + "/Source/ThirdParty/Bullet/src");
    incPaths_.Push(URHO3D_HOME + "/Source/ThirdParty/kNet/include");
    incPaths_.Push(URHO3D_HOME + "/Source/ThirdParty/SDL/include");

    libPaths_.Clear();
    libPaths_.Push(URHO3D_HOME + "/Lib");
    libraries_.Clear();
    libraries_.Push("Urho3D");

    defines_.Clear();
    defines_.Push("URHO3D_LOGGING");
}

RCCppImpl::~RCCppImpl()
{
}

String RCCppImpl::GetCompilerLine()
{
    String compilerString = getCompilerSharedLib();
    for (List<String>::Iterator it = compilerFlags_.Begin(); it != compilerFlags_.End(); ++it)
    {
        compilerString += " " + *it;
    }
    for (List<String>::Iterator it = incPaths_.Begin(); it != incPaths_.End(); ++it)
    {
        compilerString += " " + getIncPathPrefix() + *it;
    }
    for (List<String>::Iterator it = libPaths_.Begin(); it != libPaths_.End(); ++it)
    {
        compilerString += " " + getLibPathPrefix() + *it;
    }
    for (List<String>::Iterator it = libraries_.Begin(); it != libraries_.End(); ++it)
    {
        compilerString += " " + getLinkLibPrefix() + *it;
    }
    for (List<String>::Iterator it = defines_.Begin(); it != defines_.End(); ++it)
    {
        compilerString += " " + getDefinePrefix() + *it;
    }
    return compilerString;
}

String RCCppImpl::GetCompilerLine(const String& fileName)
{
    return GetCompilerLine();
}

}
