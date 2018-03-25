#pragma once

#include <type_traits>
#include <src/InstanceRegistration.h>
#include <cassert>

namespace globalMocking{


constexpr bool testing = true;


struct A
{
    virtual int foo() { return system("rm /tmp/myfile"); }
};

struct A_mock  : public A
{
    int foo() override { return 0; }
};

int bar(){

    return global::instance<A>()->foo() ? 77 : 66;

}

void testB(){

    global::TestInstance<A,A_mock> a_mock;  //temporarily make an instance of A_mock globally accessible

    assert(bar() == 66);                    //bar() uses 'a_mock'
}


void main_mockable(){

    global::Instance<A> a;                  //make 'a' globally accessible

    if (testing) testB();

    bar();                                  //bar() uses 'a' again
}


}//globalMocking
