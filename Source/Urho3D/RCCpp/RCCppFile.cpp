#include "RCCppFile.h"

namespace Urho3D
{

RCCppFile::RCCppFile(Context* context) :
    Resource(context),
    mainFile_(false)
{
}

bool RCCppFile::Load(Deserializer &source)
{
    return true;
}

bool RCCppFile::IsMainFile() const
{
    return mainFile_;
}

void RCCppFile::SetMainFile(bool value)
{
    mainFile_ = value;
}

}
