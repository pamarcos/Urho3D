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

#include "Object.h"
#include "Context.h"
#include "CoreEvents.h"

using namespace Urho3D;

namespace Urho3D
{

class RCCppMainObject : public Object
{
    OBJECT(RCCppMainObject);

    RCCppMainObject(Context* context) : Object(context)
    {
        SubscribeToEvent(E_UPDATE, HANDLER(RCCppMainObject, Update));
    }
    virtual ~RCCppMainObject() {}

    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Update(StringHash eventType, VariantMap& eventData) {}
};

typedef RCCppMainObject* (*PCreateRCCppMainObject)(Context*);
typedef void (*PDestroyRCCppMainObject)(RCCppMainObject*);

#define RCCP_OBJECT(typeName) \
    extern "C" RCCppMainObject* create(Context* context) { return new typeName(context); } \
    extern "C" void destroy(RCCppMainObject* object) { delete object; }

}

#endif // RCCPPOBJECT_H
