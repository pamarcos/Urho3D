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

#include "../IO/FileWatcher.h"
#include "../IO/FileSystem.h"
#include "../Core/CoreEvents.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/ResourceEvents.h"
#include "../Engine/Engine.h"
#include "../Core/Condition.h"
#include "../UI/UI.h"
#include "../UI/Font.h"

#include "RCCppFile.h"
#include "RCCpp.h"
#include "RCCppEvents.h"

#if defined(__APPLE__)|| defined(__linux__)
#include "RCCppUnix.h"
#elif defined (WIN32)
#include "RCCppWin.h"
#endif

namespace Urho3D
{

RCCpp::RCCpp(Context* context) :
    Object(context),
    mainRCCppFile_(NULL),
    rcCppFileCompiled_(NULL),
    compilationSuccesful_(false),
    firstCompilation_(true),
    compilationFinished_(false),
    cache_(GetSubsystem<ResourceCache>()),
    rcCppFileChangedTimestamp_(0),
    minRcCppFileChangeTimeElapsed_(2000)
{
#if defined(__APPLE__) || defined(__linux__)
    impl_ = new RCCppUnix(context);
#else
    impl_ = new RCCppWin(context);
#endif

    context_->RegisterFactory<RCCppFile>();
    cache_->SetAutoReloadResources(true);

    uiBackground_ = GetSubsystem<UI>()->GetRoot()->CreateChild<BorderImage>();
    uiBackground_->SetVisible(false);
    uiBackground_->SetColor(Color::BLACK);
    uiBackground_->SetPosition(0, 0);
    uiBackground_->SetOpacity(0.75f);
    uiBackground_->SetPriority(200);
    uiBackground_->SetLayout(LM_VERTICAL, 25, IntRect(15, 15, 15, 15));
    uiText_ = uiBackground_->CreateChild<Text>();
    uiText_->SetFont(cache_->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    uiText_->SetAlignment(HA_CENTER, VA_CENTER);
}

RCCpp::~RCCpp()
{
}

bool RCCpp::ExecuteFile(const String &fileName)
{
    mainRCCppFile_ = cache_->GetResource<RCCppFile>(fileName);
    mainRCCppFile_->SetMainFile(true);
    SubscribeToEvents();

    libraryName_ = GetFileName(fileName);
    String path, name, ext;
    SplitPath(fileName, path, name, ext);
    libraryPath_ = path + name;

    // Add the source file parent path to the resource dir to ensure ResourceCache adds a FileWatch for it
    cache_->AddResourceDir(GetParentPath(fileName));

#ifdef WIN32
    if (GetExtension(libraryPath_) != ".dll")
    {
        libraryPath_.Append(".dll");
    }
#else
    if (GetExtension(libraryPath_) != ".so")
    {
        libraryPath_.Append(".so");
    }
#endif

    compilationSuccesful_ = CompileSync(*mainRCCppFile_);
    if (compilationSuccesful_)
    {
        return LoadLib(libraryPath_);
    }
    else
    {
        return false;
    }
}

void RCCpp::SubscribeToEvents()
{
    SubscribeToEvent(cache_, E_FILECHANGED, HANDLER(RCCpp, HandleRCCppFileChanged));
    SubscribeToEvent(E_POSTUPDATE, HANDLER(RCCpp, HandlePostUpdate));

    SubscribeToEvent(this, E_RCCPP_COMPILATION_STARTED, HANDLER(RCCpp, HandleCompilationStarted));
    SubscribeToEvent(this, E_RCCPP_COMPILATION_FINISHED, HANDLER(RCCpp, HandleCompilationFinished));
    SubscribeToEvent(this, E_RCCPP_LIBRARY_PRELOADED, HANDLER(RCCpp, HandleLibraryPreLoaded));
    SubscribeToEvent(this, E_RCCPP_LIBRARY_POSTLOADED, HANDLER(RCCpp, HandleLibraryPostLoaded));
    SubscribeToEvent(this, E_RCCPP_CLASS_PRELOADED, HANDLER(RCCpp, HandleClassPreLoaded));
    SubscribeToEvent(this, E_RCCPP_CLASS_POSTLOADED, HANDLER(RCCpp, HandleClassPostLoaded));
}

bool RCCpp::CompileSync(const RCCppFile &file)
{
    using namespace RCCppCompilationStarted;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_FILE] = reinterpret_cast<void*>(const_cast<RCCppFile*>(&file));
    SendEvent(E_RCCPP_COMPILATION_STARTED, eventData);

    if (!impl_->Compile(file, libraryPath_, compilationOutput_))
    {
        LOGERROR("Error compiling file " + file.GetName());
        compilationSuccesful_ = false;
        return false;
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
    uiText_->SetText("Compiling...");
    uiBackground_->RemoveChild(uiCompilationText_);
    uiBackground_->SetVisible(true);
    using namespace RCCppCompilationStarted;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_FILE] = reinterpret_cast<void*>(const_cast<RCCppFile*>(&file));
    SendEvent(E_RCCPP_COMPILATION_STARTED, eventData);

    compilationThread_ = new CompilationThread(context_, this, &(const_cast<RCCppFile&>(file)));
    compilationThread_->AddRef();
    compilationThread_->Run();
}

void RCCpp::Start()
{
    try
    {
        if (compilationSuccesful_)
        {
            impl_->Start(libraryName_);
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
        if (compilationSuccesful_)
        {
            impl_->Stop();
        }
    }
    catch (std::exception e)
    {
        LOGERROR("RCCpp exception in Stop(): " + String(e.what()));
    }
}

bool RCCpp::LoadLib(const String &libraryPath)
{
    compilationSuccesful_ = true;
    libraryPath_ = libraryPath;
    {
        using namespace RCCppLibraryPreloaded;
        VariantMap& libPreEventData = GetEventDataMap();
        libPreEventData[P_LIBRARY] = libraryPath_;
        SendEvent(E_RCCPP_LIBRARY_PRELOADED, libPreEventData);
    }

    LOGDEBUG("Loading library " + libraryPath);
    if (impl_->LoadLib(libraryPath_))
    {
        {
            using namespace RCCppLibraryPostLoaded;
            VariantMap& libPostEventData = GetEventDataMap();
            libPostEventData[P_LIBRARY] = libraryPath_;
            SendEvent(E_RCCPP_LIBRARY_POSTLOADED, libPostEventData);
        }

        return true;
    }
    else
    {
        LOGERROR("Could not load library " + libraryPath_);
        return false;
    }
}

bool RCCpp::ReloadLib(const String& libraryPath)
{
    if (!compilationSuccesful_)
    {
        return false;
    }
    String className = GetFileName(rcCppFileCompiled_->GetName());

    {
        using namespace RCCppLibraryPreloaded;
        VariantMap& libPreEventData = GetEventDataMap();
        libPreEventData[P_LIBRARY] = libraryPath;
        SendEvent(E_RCCPP_LIBRARY_PRELOADED, libPreEventData);
    }

    {
        using namespace RCCppClassPreLoaded;
        VariantMap& classPreEventData = GetEventDataMap();
        classPreEventData[P_CLASS_NAME] = className;
        SendEvent(E_RCCPP_CLASS_PRELOADED, classPreEventData);
    }

    LOGDEBUG("Unloading library ");
    impl_->UnloadLib();

    LOGDEBUG("Loading library " + libraryPath);
    if (impl_->LoadLib(libraryPath))
    {
        {
            using namespace RCCppLibraryPostLoaded;
            VariantMap& libPostEventData = GetEventDataMap();
            libPostEventData[P_LIBRARY] = libraryPath;
            SendEvent(E_RCCPP_LIBRARY_POSTLOADED, libPostEventData);
        }

        {
            using namespace RCCppClassPostLoaded;
            VariantMap& classPostEventData = GetEventDataMap();
            classPostEventData[P_CLASS_NAME] = className;
            SendEvent(E_RCCPP_CLASS_POSTLOADED, classPostEventData);
        }

        return true;
    }
    else
    {
        LOGERROR("Could not load library " + libraryPath);
        return false;
    }
}

void RCCpp::SendCompilationFinishedEvent(bool successful, const RCCppFile& file)
{
    using namespace RCCppCompilationFinished;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_SUCCESSFUL] = successful;
    eventData[P_FILE] = reinterpret_cast<void*>(const_cast<RCCppFile*>(&file));
    eventData[P_OUTPUT] = compilationOutput_;
    SendEvent(E_RCCPP_COMPILATION_FINISHED, eventData);
}

void RCCpp::HandleRCCppFileChanged(StringHash eventType, VariantMap& eventData)
{
    using namespace FileChanged;
    String fileName = eventData[P_FILENAME].GetString();
    if (GetExtension(fileName) == ".cpp" || GetExtension(fileName) == ".h")
    {
        unsigned timestmap = Time::GetSystemTime();
        if (timestmap - rcCppFileChangedTimestamp_ > minRcCppFileChangeTimeElapsed_)
        {
            rcCppFileChangedTimestamp_ = timestmap;
            LOGDEBUG("RCCpp: reloading cpp file: " + fileName);
            CompileAsync(*cache_->GetResource<RCCppFile>(fileName));
        }
        else
        {
            LOGDEBUG("RCCpp: cpp file " + fileName + " changed too fast (< " + String(minRcCppFileChangeTimeElapsed_) + " ms). Time elapsed: " +
                     String(timestmap-rcCppFileChangedTimestamp_) + " ms");
        }
    }
}

void RCCpp::HandlePostUpdate(StringHash eventType, VariantMap &eventData)
{
    if (compilationFinished_)
    {
        uiBackground_->SetVisible(false);
        SendCompilationFinishedEvent(compilationSuccesful_, *rcCppFileCompiled_);

        if (compilationThread_.Get() != NULL)
        {
            compilationThread_->ReleaseRef();
        }

        if (compilationSuccesful_)
        {
            if (!firstCompilation_)
            {
                impl_->Stop();
                ReloadLib(libraryPath_);
                impl_->Start(libraryName_);
            }
            else
            {
                firstCompilation_ = false;
            }
        }
        else
        {
            uiText_->SetText("Compilation failed");
            uiCompilationText_ = uiBackground_->CreateChild<Text>();
            uiCompilationText_->SetText(compilationOutput_);
            uiCompilationText_->SetFont(cache_->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 10);
            uiBackground_->SetVisible(true);
        }

        compilationFinished_ = false;
    }
}

void RCCpp::HandleCompilationStarted(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppCompilationStarted;
    RCCppFile* file = reinterpret_cast<RCCppFile*>(eventData[P_FILE].GetVoidPtr());

    LOGDEBUG("RCCpp: Compilation started: " + file->GetName());
}

void RCCpp::HandleCompilationFinished(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppCompilationFinished;
    RCCppFile* file = reinterpret_cast<RCCppFile*>(eventData[P_FILE].GetVoidPtr());

    LOGDEBUG("RCCpp: Compilation finished: " + file->GetName());
}

void RCCpp::HandleLibraryPreLoaded(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppLibraryPreloaded;
    String libraryName = eventData[P_LIBRARY].GetString();
    LOGDEBUG("RCCpp: Library preloaded: " + libraryName);
}

void RCCpp::HandleLibraryPostLoaded(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppLibraryPostLoaded;
    String libraryName = eventData[P_LIBRARY].GetString();
    LOGDEBUG("RCCpp: Library postloaded: " + libraryName);
}

void RCCpp::HandleClassPreLoaded(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppClassPreLoaded;
    String className = eventData[P_CLASS_NAME].GetString();
    LOGDEBUG("RCCpp: Class preloaded: " + className);
}

void RCCpp::HandleClassPostLoaded(StringHash eventType, VariantMap &eventData)
{
    using namespace RCCppClassPostLoaded;
    String className = eventData[P_CLASS_NAME].GetString();
    LOGDEBUG("RCCpp: Class postloaded: " + className);
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
    rcCpp_->rcCppFileCompiled_ = rcCppFile_;
    rcCpp_->compilationSuccesful_ = rcCpp_->impl_->Compile(*rcCppFile_, rcCpp_->libraryPath_, rcCpp_->compilationOutput_);
    if (!rcCpp_->compilationSuccesful_ )
    {
        LOGERROR("Error compiling file " + rcCppFile_->GetName());
    }
    rcCpp_->compilationFinished_ = true;
}

}
