#pragma once

#include <src/InstanceRegistration.h>
#include <iostream>


struct A
{
    A():reg(this){}                      // calling 'reg(this)' makes 'this' globally accessible
    void foo(){std::cout<<"foo\n";}
    global::InstanceRegistration<A> reg;
};


void bar() {
    global::instance<A>().foo();        // a call to foo() of the globally accessible instance of A
}


void main_simple(){
    A a;                               // makes 'a' globally accessible
    bar();
}

namespace withSingleton{


struct A
{
    static A& instance(){
        static A i;
        return i;
    }
    void foo(){std::cout<<"foo\n";}
};

void bar() {
    A::instance().foo();
}

void main_simple(){
    bar();
}

}
