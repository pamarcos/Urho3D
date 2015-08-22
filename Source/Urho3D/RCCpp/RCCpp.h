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

#ifndef RCCPP_H
#define RCCPP_H

#include "../Core/Object.h"
#include "../Core/Context.h"
#include "../Core/Thread.h"
#include "../UI/UIElement.h"
#include "../UI/Text.h"
#include "../UI/BorderImage.h"

#include "RCCppImpl.h"
#include "RCCppFile.h"

namespace Urho3D
{

class ResourceCache;
class RCCpp;
class Condition;

class URHO3D_API CompilationThread : public Object, public Thread
{
    OBJECT(CompilationThread);

public:
    CompilationThread(Context* context, RCCpp* rcCpp, RCCppFile* file);
    virtual ~CompilationThread();
    void ThreadFunction();

private:
    bool compilationSuccesful_;
    RCCpp* rcCpp_;
    RCCppFile* rcCppFile_;
};

class URHO3D_API RCCpp : public Object
{
    OBJECT(RCCpp);

    friend class CompilationThread;

public:
    RCCpp(Context* context);
    virtual ~RCCpp();

    bool ExecuteFile(const String& fileName);

    void Start();
    void Stop();
    bool LoadLib(const String& libraryPath);

    String GetLibraryName() { return libraryName_; }
    String GetLibraryPath() { return libraryPath_; }

    unsigned GetMinRcCppFileChangeTimeElapsed() { return minRcCppFileChangeTimeElapsed_; }
    void SetMincCppFileChangeTimeElapsed(unsigned value) { minRcCppFileChangeTimeElapsed_ = value; }

private:
    bool ReloadLib(const String& libraryPath);
    void SendCompilationFinishedEvent(bool successful, const RCCppFile& file);
    void SubscribeToEvents();

    void CompileAsync(const RCCppFile& file);
    bool CompileSync(const RCCppFile& file);

    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandleRCCppFileChanged(StringHash eventType, VariantMap& eventData);
    void HandleCompilationStarted(StringHash eventType, VariantMap& eventData);
    void HandleCompilationFinished(StringHash eventType, VariantMap& eventData);
    void HandleLibraryPreLoaded(StringHash eventType, VariantMap& eventData);
    void HandleLibraryPostLoaded(StringHash eventType, VariantMap& eventData);
    void HandleClassPreLoaded(StringHash eventType, VariantMap& eventData);
    void HandleClassPostLoaded(StringHash eventType, VariantMap& eventData);

    String libraryName_;
    String libraryPath_;
    RCCppFile* mainRCCppFile_;
    RCCppFile* rcCppFileCompiled_;
    SharedPtr<RCCppImpl> impl_;
    ResourceCache* cache_;
    bool compilationSuccesful_;
    SharedPtr<CompilationThread> compilationThread_;
    bool firstCompilation_;
    bool compilationFinished_;
    String compilationOutput_;
    unsigned rcCppFileChangedTimestamp_;
    unsigned minRcCppFileChangeTimeElapsed_;

    SharedPtr<BorderImage> uiBackground_;
    SharedPtr<Text> uiText_;
    SharedPtr<Text> uiCompilationText_;
};

}
#endif // RCCPP_H
