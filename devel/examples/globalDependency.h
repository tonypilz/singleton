#pragma once

#include <src/globalInstances.h>
#include <iostream>

namespace dependency {


struct A
{
    A();
    int val = 4;
};


struct B
{
    B();
    int val = 5;
};


A::A(){
    global::instance<B>().visitIfNotNull([this](B& b){
        std::cout<<"a is using value of b:" << b.val << "\n";});
}

B::B(){
    global::instance<A>().visitIfNotNull([this](A& a){
        std::cout<<"b is using value of a:" << a.val << "\n";});
}

void main_dependency(){

    global::Instance<A> a;
    global::Instance<B> b;        //prints: "a is using value of b:5"
                                  //        "b is using value of a:4"

}

}
