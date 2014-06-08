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

#ifndef RCCPPIMPL_H
#define RCCPPIMPL_H

#include "Object.h"
#include "Context.h"
#include "RCCppFile.h"
#include "RCCppObject.h"

namespace Urho3D
{

class RCCppMainObject;

class URHO3D_API RCCppImpl : public Object
{
    OBJECT(RCCppImpl);

public:
    RCCppImpl(Context* context);
    virtual ~RCCppImpl();

    /// Execute script file. Return true if successful.
    virtual bool Compile(const RCCppFile& file, const String& libraryPath) = 0;
    virtual void Start(const String& libraryName);
    virtual void Stop();
    virtual bool LoadLibrary(const String& libraryPath) = 0;
    virtual void UnloadLibrary() = 0;
    virtual RCCppObject* CreateObject(const String& objectName) = 0;
    virtual void DestroyObject(RCCppObject* object) = 0;

protected:
    RCCppMainObject* mainObject_;
};

}

#endif // RCCPPIMPL_H
