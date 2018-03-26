#pragma once

#include <type_traits>
#include <src/globalInstances.h>

namespace globalMocking2{


struct A_actual
{
    int foo(){ return system("rm /tmp/myfile"); }
};


struct A_mock : public A_actual
{
    int foo(){ return 0; }
};


constexpr bool testing = true;
using A = std::conditional<testing, A_mock, A_actual>::type;



int bar(){

    return global::instance<A>()->foo() ? 77 : 66;

}


void testB(){

    assert(bar() == 66);                   // bar() uses instance of a_mock
}


void main_mockable2(){

    global::Instance<A> a;                 // make 'a' globally accessible

    if (testing){
        testB();               // bar uses the foo()-implementation of either A_mock or A_actual depending on the value of 'testing'
    }

    bar();

}


}//globalMocking
