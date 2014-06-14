#include "MyClass.h"
#include "Log.h"

RCCPP_OBJECT(MyClass);

MyClass::MyClass(Context* context) :
    RCCppObject(context)
{
}

MyClass::~MyClass()
{
    LOGINFO("~MyClass()");
}

void MyClass::PrintSomething()
{
    LOGINFO("Print from MyClass!");
}
