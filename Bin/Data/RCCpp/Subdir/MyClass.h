#ifndef MYCLASS_H
#define MYCLASS_H

#include "RCCppObject.h"

class MyClass : public RCCppObject
{
	OBJECT(MyClass);

public:
    MyClass(Context* context);
    ~MyClass();

    void PrintSomething();
};

#endif // MYCLASS_H
