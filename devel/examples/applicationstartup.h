#pragma once


#include <src/globalInstances.h>
#include <iostream>

namespace startup2{

struct A{};
struct B{};
struct C{};

void doStuff(){}

void main_(){

    global::Instance<A> a; // Construct an instance of A and make it globally accessible
    global::Instance<B> b; // Construct an instance of B and make it globally accessible
    global::Instance<C> c; // Construct an instance of C and make it globally accessible

    doStuff();

} //on scope exit c, b and a will be deregistered before their destruction


}
