#include "RCCppFile.h"

namespace Urho3D
{

RCCppFile::RCCppFile(Context* context) :
    Resource(context)
{
}

bool RCCppFile::Load(Deserializer &source)
{
    return true;
}

}
