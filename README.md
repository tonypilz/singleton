# Overview
This library improves the classical singleton pattern with respect to 
 - testing
 - arguments for the constructor 
 - control over seqcence and timepoint of construction/destruction
 - invalid access detection
 
by separating instance access from instance lifetime, illustrated by the following example:

```cpp
#include <src/InstanceRegistration.h>

void bar(); 

struct A
{
    void foo(){std::cout<<"foo\n";}
};

void main(){
    A a;                                     // 1) construct instance
    global::InstanceRegistration<A> reg(&a); // 2) make instance 'a' globally accessible
    bar();
}                                            // 4) 

void bar() {
    global::instance<A>().foo();             // 3) access instance  'a' 
}

```

In the example, instance `a` is constructed as a regular object in `1`, then made globally accessible in `2` and then accessed globally by function `bar()` in `3`. Note that in `4` instance `a` is deregistered first before it is destructed.   

The example covers the basic usage of this library. Not covered by the example are the aspects of [testing](#testing) and delayed access which will be shown below.

# Status
## Tested on
 - gcc-7.2.0

## Compiles Under
 - icc-18
 - gcc-4.9.0 .. gcc-7.3
 - clang 3.4 .. clang 6.0
 
## Status
 - Implementation: complete (~200sloc)
 - Tests: complete
 - Documentation: incomplete


# Testing 
There are two cases to be considered:
 1. Globally accessible classes and 
 2. Classes, that access global instances

The first case is trivial. As globally accessible instances do not know that they are global they can be tested like regular ones.

The second case requires more attention. In order to test classes that access global instances we need to replace them by mocks during the runtime of the test. How this is done is shown in the following example:  

```cpp
struct A
{
    virtual int foo() { return system("rm /tmp/myfile"); }
};

struct A_mock  : public A
{
    int foo() override { return 0; }
};

struct B
{
    int bar(){ return global::instance<A>().foo() ? 77 : 66;}  // 1) access to global instance
};

void test_B_bar(){

    A_mock a_mock;                                             // 2)

    global::ReplacingInstanceRegistration<A> reg(&a_mock);     // 3) temporarily replace global instance by 'a-mock'

    B b;

    assert(b.bar() == 66);                                     // 4) test bar b.bar() uses 'a_mock'
    
}                                                              // 5) undo of step 2) 

```
In the example the function `test_B_bar()` tests the function `B::bar()` of class `B`. Since `B::bar()` accesses in `1` a global instance of class `A` it must be replaced by a mock instance before executing the test. This is done in `3` by making an instance of `A_mock` globally accessible. The test call `b.bar()` in `4` now accesses `a_mock` which does nothing and returns `0`. 
