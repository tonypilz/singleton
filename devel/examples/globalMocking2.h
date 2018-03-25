#pragma once

#include <type_traits>
#include <src/InstanceRegistration.h>

namespace globalMocking2{


constexpr bool testing = true;


struct A_actual
{
    int foo(){ return system("rm /tmp/myfile"); }
};


struct A_mock
{
    int foo(){ return 0; }
};



using A = std::conditional<testing, A_mock, A_actual>::type;




struct B
{
    int bar(){ return global::instance<A>().foo() ? 77 : 66;}
};


void testB(){

    A_mock a_mock;

    global::detail::ReplacingInstanceRegistration<A> reg(&a_mock); //temporarily make 'a_mock' globally accessible under 'A'

    B b;

    assert(b.bar() == 66);           //b.bar() uses instance of a_mock
}


void main_mockable2(){

    A a;
    global::detail::InstanceRegistration<A> reg(&a); //make 'a' globally accessible

    B t;

    if (testing){
        assert(t.bar() == 66);               //b uses the foo()-implementation of either A_mock or A_actual depending on the value of 'testing'
    }

}


}//globalMocking
