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

#ifndef RCCPPMAINOBJECT_H
#define RCCPPMAINOBJECT_H

#include "../Core/Object.h"
#include "../Core/Context.h"
#include "../Engine/Engine.h"

#include "RCCppObject.h"

using namespace Urho3D;

namespace Urho3D
{

class RCCppMainObject : public RCCppObject
{
    OBJECT(RCCppMainObject);

public:
    RCCppMainObject(Context* context) :
        RCCppObject(context),
        engine_(GetSubsystem<Engine>())
    {}

    virtual void Start() = 0;
    virtual void Stop() = 0;

protected:
    SharedPtr<Engine> engine_;
};

typedef RCCppMainObject* (*PCreateRCCppMainObject)(Context*);
typedef void (*PDestroyRCCppMainObject)(RCCppMainObject*);

}

#endif // RCCPPMAINOBJECT_H