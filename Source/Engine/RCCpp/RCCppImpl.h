#ifndef RCCPPIMPL_H
#define RCCPPIMPL_H

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

#include "Object.h"
#include "Context.h"

namespace Urho3D
{

class URHO3D_API RCCppImpl : public Object
{
    OBJECT(RCCppImpl);

public:
    RCCppImpl(Context* context) : Object(context)
    {
        String URHO3D_HOME = getenv("URHO3D_HOME");
        compilerFlags_ =    "-I. "
                            "-I" + URHO3D_HOME + "/Build/Engine " +
                            "-I" + URHO3D_HOME + "/Source/Engine/RCCpp " +
                            "-I" + URHO3D_HOME + "/Source/Engine" +
                            "-I" + URHO3D_HOME + "/Source/Engine/Audio " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Container " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Core " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Engine " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Graphics " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Input " +
                            "-I" + URHO3D_HOME + "/Source/Engine/IO " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Math " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Navigation " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Network " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Physics " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Resource " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Scene " +
                            "-I" + URHO3D_HOME + "/Source/Engine/UI " +
                            "-I" + URHO3D_HOME + "/Source/Engine/Urho2D " +
                            "-I" + URHO3D_HOME + "/Source/ThirdParty/Box2D " +
                            "-I" + URHO3D_HOME + "/Source/ThirdParty/Bullet/src " +
                            "-I" + URHO3D_HOME + "/Source/ThirdParty/kNet/include " +
                            "-I" + URHO3D_HOME + "/Source/ThirdParty/SDL/include " +
                            "-L" + URHO3D_HOME + "/Lib " + "-lUrho3D " +
                            "-DURHO3D_LOGGING ";
    }
    virtual ~RCCppImpl() {}

    /// Execute script file. Return true if successful.
    virtual bool Compile(const String& fileName) = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;

protected:
    String compilerFlags_;
};

}

#endif // RCCPPIMPL_H
