#pragma once

#include <type_traits>

namespace MockableGlobalInstance{

constexpr bool testing = true;

template<typename T_test, typename T>
using ifTest = std::conditional<testing, T_test, T>;


//this form of testability is independet of global instance access and vice versa
//it can be added at any time to a project



//this is actual implementation
class A_actual
{
public:
    A_actual();

    void foo(){} //not virtual!
};


//this is test interface
class A_testInterface
{
public:
    A_testInterface(){}

    virtual void foo() = 0;
};





//this type is actually beeing used by the class to be tested
//MockableGlobalInstance x; x.foo();
//global::instance<MockableGlobalInstance>().foo();

using A = ifTest<A_testInterface, A_actual>;





class A_someMockImpl : public A_testInterface
{
public:
    A_someMockImpl(){}

    void foo() override{}
};


void exampleMain(){}
}
