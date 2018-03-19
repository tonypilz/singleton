#pragma once

#include <type_traits>
#include <src/InstanceRegistration.h>

namespace globalMocking{


struct A_actual
{
    int foo(){ return system("rm /tmp/myfile"); }
};

struct A_mock
{
    int foo(){ return 0; }
};


constexpr bool testing = true;
using A = std::conditional<testing, A_mock, A_actual>::type;




struct B
{
    int bar(){ return global::instance<A>().foo() ? 77 : 66;}
};



void main_mockable(){

    A a;
    global::InstanceRegistration<A> reg(&a); //make 'a' globally accessible

    B t;

    if (testing){
        assert(t.bar() == 66);           //b uses the foo()-implementation of either A_mock or A_actual depending on the value of 'testing'
    }

}


}//globalMocking
