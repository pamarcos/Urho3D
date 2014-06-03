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

#include "RCCpp.h"
#include "FileWatcher.h"
#include "FileSystem.h"
#include "CoreEvents.h"
#include "Log.h"
#include "ResourceCache.h"
#include "ResourceEvents.h"
#include "RCCppFile.h"

#if defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE_CC__) || defined(BSD) || defined(__linux__)
#include "RCCppUnix.h"
#endif

namespace Urho3D
{

RCCpp::RCCpp(Context* context) :
    Object(context)
{
#if defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE_CC__) || defined(BSD)
    impl_ = new RCCppUnix(context);
#endif

    context_->RegisterFactory<RCCppFile>();
    cache_ = GetSubsystem<ResourceCache>();
    cache_->SetAutoReloadResources(true);
}

bool RCCpp::ExecuteFile(const String &fileName)
{
    mainRCCppFile_ = cache_->GetResource<RCCppFile>(fileName);
    SubscribeToEvent(cache_, E_FILECHANGED, HANDLER(RCCpp, HandleRCCppFileChanged));
    return Compile(fileName);
}

bool RCCpp::Compile(const String &fileName)
{
    bool ret = impl_->Compile(fileName);
    if (!ret)
    {
        LOGERROR("Error compiling file " + fileName);
    }
    return ret;
}

void RCCpp::Start()
{
    impl_->Start();
}

void RCCpp::Stop()
{
    impl_->Stop();
}

void RCCpp::HandleRCCppFileChanged(StringHash eventType, VariantMap& eventData)
{
    String fileName = eventData[FileChanged::P_FILENAME].ToString();
    if (GetExtension(fileName) == ".cpp")
    {
        if (cache_->SanitateResourceName(fileName) == mainRCCppFile_->GetName())
        {
            impl_->Stop();
        }
        LOGDEBUG("Reloading cpp file: " + fileName);
        Compile(fileName);

        if (cache_->SanitateResourceName(fileName) == mainRCCppFile_->GetName())
        {
            impl_->Start();
        }
    }
}

}
