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
    global::detail::InstanceRegistration<A> regA(&a); // make 'a' globally accessible
    B b;
    global::detail::InstanceRegistration<B> regB(&b); // make 'b' globally accessible
    C c;
    global::detail::InstanceRegistration<C> regC(&c); // make 'c' globally accessible

    doStuff();

} //on scope exit c, b and a will be deregistered before their destruction


}
