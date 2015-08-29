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

#ifndef RCCPPOBJECT_H
#define RCCPPOBJECT_H

#include "../Core/Object.h"
#include "../Core/Context.h"
#include "../Core/CoreEvents.h"

using namespace Urho3D;

namespace Urho3D
{

class RCCppObject : public Object
{
    OBJECT(RCCppObject);

public:
    RCCppObject(Context* context) :
            Object(context)
    {}

    virtual ~RCCppObject() {}

    virtual void Start() {}
    virtual void Stop() {}
};

typedef RCCppObject* (*PCreateRCCppObject)(Context*);
typedef void (*PDestroyRCCppObject)(RCCppObject*);

#define RCCPP_OBJECT(typeName) \
    extern "C" RCCppObject* create##typeName(Context* context) { return new typeName(context); } \
    extern "C" void destroy##typeName(RCCppObject* object) { delete object; }

}

#endif // RCCPPOBJECT_H
