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
#include "RCCppEvents.h"
#include "FileWatcher.h"
#include "FileSystem.h"
#include "CoreEvents.h"
#include "Log.h"
#include "ResourceCache.h"
#include "ResourceEvents.h"
#include "RCCppFile.h"
#include "Engine.h"
#include "Condition.h"

#if defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE_CC__) || defined(BSD) || defined(__linux__)
#include "RCCppUnix.h"
#endif

namespace Urho3D
{

RCCpp::RCCpp(Context* context) :
    Object(context),
    mainRCCppFile_(NULL),
    rcCppFileCompiled_(NULL),
    compilationSuccesful_(false),
    firstCompilation_(true),
    compilationFinished_(false)
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

RCCpp::~RCCpp()
{
}

bool RCCpp::ExecuteFile(const String &fileName)
{
    mainRCCppFile_ = cache_->GetResource<RCCppFile>(fileName);
    mainRCCppFile_->SetMainFile(true);

    SubscribeToEvent(cache_, E_FILECHANGED, HANDLER(RCCpp, HandleRCCppFileChanged));
    SubscribeToEvent(E_POSTUPDATE, HANDLER(RCCpp, HandlePostUpdate));
    SubscribeToEvent(this, E_RCCPP_COMPILATION_FINISHED, HANDLER(RCCpp, HandleCompilationFinished));
    SubscribeToEvent(this, E_RCCPP_LIBRARY_LOADED, HANDLER(RCCpp, HandleLibraryLoaded));
    SubscribeToEvent(this, E_RCCPP_CLASS_LOADED, HANDLER(RCCpp, HandleClassLoaded));

    compilationSuccesful_ = CompileSync(*mainRCCppFile_);
    return ReloadLibrary();
}

bool RCCpp::CompileSync(const RCCppFile &file)
{
    if (!impl_->Compile(file))
    {
        LOGERROR("Error compiling file " + file.GetName());
        compilationSuccesful_ = false;
    }
    else
    {
        compilationSuccesful_ = true;
    }
    rcCppFileCompiled_ = const_cast<RCCppFile*>(&file);
    compilationFinished_ = true;
    SendCompilationFinishedEvent(compilationFinished_, file);
    return compilationSuccesful_;
}

void RCCpp::CompileAsync(const RCCppFile& file)
{
    compilationThread_ = new CompilationThread(context_, this, &(const_cast<RCCppFile&>(file)));
    compilationThread_->AddRef();
    compilationThread_->Run();

    rcCppFileCompiled_ = const_cast<RCCppFile*>(&file);
}

void RCCpp::Start()
{
    try
    {
        if (compilationSuccesful_)
        {
            impl_->Start();
        }
        else
        {
            LOGINFO("Exiting because compilation failed");
            context_->GetSubsystem<Engine>()->Exit();
        }
    }
    catch (std::exception e)
    {
        LOGERROR("RCCpp exception in Start(): " + String(e.what()));
    }
}

void RCCpp::Stop()
{
    try
    {
        if (!compilationSuccesful_)
        {
            impl_->Stop();
        }
    }
    catch (std::exception e)
    {
        LOGERROR("RCCpp exception in Stop(): " + String(e.what()));
    }
}

bool RCCpp::ReloadLibrary()
{
    impl_->UnloadLibrary();
    if (impl_->LoadLibrary(impl_->GetLibraryPath()))
    {
        using namespace RCCppLibraryLoaded;
        VariantMap& libEventData = GetEventDataMap();
        libEventData[P_LIBRARY] = impl_->GetLibraryPath();
        SendEvent(E_RCCPP_LIBRARY_LOADED, libEventData);

        using namespace RCCppClassLoaded;
        VariantMap& classEventData = GetEventDataMap();
        classEventData[P_CLASS_NAME] = GetFileName(rcCppFileCompiled_->GetName());
        SendEvent(E_RCCPP_CLASS_LOADED, classEventData);

        return true;
    }
    else
    {
        return false;
    }
}

void RCCpp::SendCompilationFinishedEvent(bool successful, const RCCppFile& file)
{
    using namespace RCCppCompilationFinished;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_SUCCESSFUL] = successful;
    eventData[P_FILE] = reinterpret_cast<void*>(const_cast<RCCppFile*>(&file));
    SendEvent(E_RCCPP_COMPILATION_FINISHED, eventData);
}

void RCCpp::HandleRCCppFileChanged(StringHash eventType, VariantMap& eventData)
{
    using namespace FileChanged;
    String fileName = eventData[P_FILENAME].GetString();
    if (GetExtension(fileName) == ".cpp")
    {
        LOGDEBUG("Reloading cpp file: " + fileName);
        CompileAsync(*cache_->GetResource<RCCppFile>(fileName));
    }
}

void RCCpp::HandlePostUpdate(StringHash eventType, VariantMap &eventData)
{
    if (compilationFinished_)
    {
        if (compilationThread_.Get() != NULL)
        {
            compilationThread_->ReleaseRef();
        }

        if (!firstCompilation_)
        {
            if (rcCppFileCompiled_->IsMainFile())
            {
                impl_->Stop();
            }

            ReloadLibrary();

            if (rcCppFileCompiled_->IsMainFile())
            {
                impl_->Start();
            }
        }
        else
        {
            firstCompilation_ = false;
        }

        SendCompilationFinishedEvent(compilationSuccesful_, *rcCppFileCompiled_);
        compilationFinished_ = false;
    }
}

void RCCpp::HandleCompilationFinished(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppCompilationFinished;
    RCCppFile* file = reinterpret_cast<RCCppFile*>(eventData[P_FILE].GetVoidPtr());

    LOGDEBUG("RCCpp: Compilation finished: " + file->GetName());
}

void RCCpp::HandleLibraryLoaded(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppLibraryLoaded;
    String libraryName = eventData[P_LIBRARY].GetString();
    LOGDEBUG("RCCpp: Library loaded: " + libraryName);
}

void RCCpp::HandleClassLoaded(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppClassLoaded;
    String className = eventData[P_CLASS_NAME].GetString();
    LOGDEBUG("RCCpp: Class loaded: " + className);
}

CompilationThread::CompilationThread(Context* context, RCCpp* rcCpp, RCCppFile* file) :
    Object(context),
    rcCpp_(rcCpp),
    rcCppFile_(file)
{
}

CompilationThread::~CompilationThread()
{
}

void CompilationThread::ThreadFunction()
{
    if (!rcCpp_->impl_->Compile(*rcCppFile_))
    {
        LOGERROR("Error compiling file " + rcCppFile_->GetName());
        rcCpp_->compilationSuccesful_ = false;
    }
    rcCpp_->compilationFinished_ = true;
    rcCpp_->compilationSuccesful_ = true;
}

}
