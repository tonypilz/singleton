# global-instances-access
The main goal of this library is to provide an alternative to the classical singleton pattern.
Is allows the user to make instances globally available and not to suffer from most of the drawbacks a singleton solution.

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
    A():reg(this){}                      // calling 'reg(this)' makes 'this' globally accessible
    void foo(){std::cout<<"foo\n";}
    global::InstanceRegistration<A> reg;
};

void bar() {
    global::instance<A>().foo();        // a call to member foo() of the globally accessible instance of A
}

void main(){
    A a;                               // makes 'a' globally accessible
    bar();
}
```

# Comparison singleton vs this lib


| Property | Singleton | global-instances-access |
| --- | --- | --- |
| constructor arguments | ugly | no restrictions |
| instance replacablity (testing) | ugly | simple |
| object construction sequence | fixed, determined by first call to instance() | manual control, no restrictions  | 
| constructor loops A::A()->B::B()->A::A() | hard to detect, errorprone to resolve | can easily be avoided | 
| time of object destruction | fixed, during static destruction | manual control, no restrictions | 
| object destruction sequence | fixed, hard to be determined | manual control, no restrictions | 
| access to destructed objects | hard to detect | automatic detection |
| number of globally accessible instances per type | 1 (needs modtification for more) | not restricted | 
