#pragma once

#include <src/InstanceRegistration.h>
#include <iostream>

namespace simple {


struct A
{
    void foo(){std::cout<<"foo\n";}
};

void bar() {
    global::instance<A>().foo();        // access global instance of A
}

void main_(){
    A a;
    global::InstanceRegistration<A> reg(&a); // make a globally accessible
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
}
