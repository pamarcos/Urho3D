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

#include "../IO/Log.h"

#include "RCCppImpl.h"
#include "RCCppMainObject.h"

namespace Urho3D
{

RCCppImpl::RCCppImpl(Context *context) :
    Object(context),
    mainObject_(NULL)
{
}

RCCppImpl::~RCCppImpl()
{
}

void RCCppImpl::Start(const String& libraryName)
{
    if (mainObject_ == NULL)
    {
        mainObject_ = static_cast<RCCppMainObject*>(CreateObject(libraryName));
    }
    if (mainObject_ != NULL)
    {
        mainObject_->Start();
    }
    else
    {
        LOGERROR("Error creating main RCCpp object " + libraryName);
    }
}

void RCCppImpl::Stop()
{
    if (mainObject_ != NULL)
    {
        mainObject_->Stop();
        DestroyObject(mainObject_);
        mainObject_ = NULL;
    }
}

}
