#include "MyClass.h"
#include "Log.h"

MyClass::MyClass(Context* context) :
    RCCppObject(context)
{
}

void MyClass::PrintSomething()
{
    LOGINFO("Print from MyClass eeeee");
}
