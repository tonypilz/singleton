#pragma once

#include <src/InstanceRegistration.h>
#include <iostream>


struct A
{
    A():reg(this){}                      // calling 'reg(this)' makes 'this' instance globally accessible

    void foo(){std::cout<<"foo\n";}

    global::InstanceRegistration<A> reg;
};


void bar() {
    global::instance<A>().foo();        //a call to member foo() of the globally accessible instance of of A
}


void main_simple(){

    A a;                               //this makes a globally accessible

    bar();
}
