#pragma once


#include <src/InstanceRegistration.h>
#include <src/instanceOperations.h>
#include <iostream>

namespace dependency {


struct A
{
    A();
    int val = 4;
    global::InstanceRegistration<A> reg;
};


struct B
{
    B();
    int val = 5;
    global::InstanceRegistration<B> reg;
};


A::A():reg(this){
    global::onInstanceDefine<B>([this](B& b){
        std::cout<<"a is using value of b:" << b.val << "\n";});
}

B::B():reg(this){
    global::onInstanceDefine<A>([this](A& a){
        std::cout<<"b is using value of a:" << a.val << "\n";});
}

void main_dependency(){

    A a;
    B b;        //prints: "a is using value of b:5"
                //        "b is using value of a:4"


}

}
