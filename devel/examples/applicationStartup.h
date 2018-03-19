#pragma once

#include <src/InstanceRegistration.h>
#include <iostream>

namespace startup{


struct A
{
    A():reg(this){}
    global::InstanceRegistration<A> reg;
};

struct B
{
    B():reg(this){}
    global::InstanceRegistration<B> reg;
};

struct C
{
    C():reg(this){}
    global::InstanceRegistration<C> reg;
};


void doStuff(){}

void main_(){

    //instanceces make themselves globally accessible after their construction

    A a;
    B b;
    C c;

    //a,b and c are are globally accessible

    doStuff();

} //on scope exit c, b and a will be deregistered and destructed


}
