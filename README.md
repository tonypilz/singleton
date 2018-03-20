# global-instances-access
The main goal of this library is to provide an alternative to the classical singleton pattern.
Is allows the user to make instances globally available and not to suffer from most of the drawbacks of a singleton solution.

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

## Example (singleton)

```cpp
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

void main(){
    bar();
}
```

## Example (global-instances-access)

```cpp
#include <src/InstanceRegistration.h>

struct A
{
    void foo(){std::cout<<"foo\n";}
};

void bar() {
    global::instance<A>().foo();        // access global instance of A
}

void main(){
    A a;   
    global::InstanceRegistration<A> reg(&a); // make a globally accessible
    bar();
}
```

# Comparison singleton vs this lib


| Property | Singleton | global-instances-access |
| --- | --- | --- |
| constructor arguments | ugly | no restrictions |
| instance replacablity (testing) | ugly | simple |
| object construction sequence | restricted, possibly hard to determine | manual control, no restrictions  | 
| constructor loops A::A()->B::B()->A::A() | errorprone to resolve | can easily be avoided | 
| time of object destruction | fixed, during static destruction | manual control, no restrictions | 
| object destruction sequence | fixed, hard to be determined | manual control, no restrictions | 
| access to destructed objects | hard to detect | automatic detection |
| number of globally accessible instances per type | 1 (needs modification for more) | not restricted | 

## Constructor Arguments
In [the example] (#example-global-instances-access) it can be seen that instance `a` is constructed manually. Therefore any parameters can be applied at this point. 

## Instance Replacablity (Testing) 
Global Instances can be replaced at any time. In the following example the function `testB()` is testing member `B::bar()`. And it does so by temporarily replacing the globally accessed instance of type `A`.
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
    int bar(){ return global::instance<A>().foo() ? 77 : 66;}
};

void testB(){

    A_mock a_mock;

    global::ReplacingInstanceRegistration<A> reg(&a_mock);  //temporarily make 'a-mock' globally accessible

    B b;

    assert(b.bar() == 66);                                  //b.bar() uses 'a_mock'
}

```
## Object Construction Sequence
In [the example] (#example-global-instances-access) it can be seen that instance `a` is first constructed manually and then registered to be globally accessible. The same procedure applies to any other globally accessible instance which allows any desired construction and registration sequence independent of their mutual access.

## Constructor Loops
Since using a global instance of `A` by calling `global::instance<A>()` does not invoke its constructor, there will be no constructor loops by this mechanism.

## Time of Object Destruction 


