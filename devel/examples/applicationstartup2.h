#pragma once

#include <src/InstanceRegistration.h>
#include <iostream>

namespace startup2{

struct A{};
struct B{};
struct C{};

void doStuff(){}

void main_(){

    A a;
    global::InstanceRegistration<A> regA(&a); // make 'a' globally accessible
    B b;
    global::InstanceRegistration<B> regB(&b); // make 'b' globally accessible
    C c;
    global::InstanceRegistration<C> regC(&c); // make 'c' globally accessible

    doStuff();

} //on scope exit c, b and a will be deregistered before their destruction


}
