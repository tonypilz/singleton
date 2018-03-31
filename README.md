# Overview

The library replaces the classical singleton. It allows the construction of and access to global instances. The difference is that [most of the drawbacks are avoided](#comparision-with-the-classical-singleton).

The following example illustrates the main usage of the librabary. It shows how to construct a globally accessible instance of type `A`, getting access to it and how it is destructed.

```cpp
#include <globalInstances.h>

struct A { 
    void foo(){ std::cout<<"foo\n"; } 
};

void bar() { 
    global::instance<A>()->foo(); // access (prints "foo")
} 

void main(){
    global::Instance<A> a;        // construct an make globally accessible
    bar();
}                                 // destruct

```



Besides the main usage above, there are some more advanced szenarios to be looked at: 
 - [how to do testing](#how-to-do-testing) 
 - [how to avoid two-phase initialization](#how-to-avoid-two-phase-initialization) 
 - [how to use multiple instances of the same type](#how-to-use-multiple-instances-of-the-same-type)
 - [how to handle invalid access](#how-to-handle-invalid-access)

In the remainder of the document the library is discussed in more detail.

 # Table of Contents

<!-- TOC -->

- [Overview](#overview)
- [Status](#status)
    - [Testing](#testing)
    - [Compiler Support](#compiler-support)
    - [Installation](#installation)
- [Library Aspects](#library-aspects)
    - [How to do Testing](#how-to-do-testing)
    - [How to Avoid Two-Phase Initialization](#how-to-avoid-two-phase-initialization)
    - [How to Use Multiple Instances of The Same Type](#how-to-use-multiple-instances-of-the-same-type)
    - [How to Handle Invalid Access](#how-to-handle-invalid-access)
    - [Various Aspects](#various-aspects)
        - [Thread Savety](#thread-savety)
        - [Static destruction](#static-destruction)
        - [Customization](#customization)
        - [Comparision with the classical singleton](#comparision-with-the-classical-singleton)
- [Under the Hood](#under-the-hood)

<!-- /TOC -->

# Status
## Testing

All tests pass under gcc-7.2.0.

## Compiler Support
The library compiles under
 - gcc-4.7 .. gcc-7.3
 - clang 3.4 .. clang 6.0
 - icc-18

The compiler flag `-std=c++11` is used
 
## Installation

just copy the single header file [globalInstances.h](include/globalInstances.h) (~200sloc) into the project and include it (see in the first example).

# Library Aspects
## How to do Testing 
There are two basic cases to be considered:
 1. testing a globally accessible class and 
 2. testing code that accesses global instances.

The first case is simple. As globally accessible classes are just like regular classes they can be tested like them.

The second case requires some extra work which shall be illustrated by the following example. In it a function `bar_test()` is supposed to test a function `bar()` which accesses global data:

```cpp
struct A
{
    virtual int foo() { return system("rm /tmp/myfile"); } 
};

int bar(){

    return global::instance<A>()->foo() ? 77 : 66;  // access a global instance 
}  

void bar_test(){

    assert(bar() == 66);                           // test
}

void main(){
    global::Instance<A> a;
    bar_test();
}  
 
```

For beeing able to test function `bar()` properly, the globally accessed instance must be replaced by a mock for the duration of the test:

```cpp

struct A_mock : public A
{
    int foo() override { return 0; }
};

void bar_test(){

    global::TestInstance<A,A_mock> a_mock;     // replace global instance of 'A' by 
                                               // an instance of 'A_mock'

    assert(bar() == 66);
}

```

## How to Avoid Two-Phase Initialization
On larger projects some of the global instances usually access each other during their construction. In the following example, a class `A` accesses a global instance of type `B` during its construction and vice versa:

```cpp
struct A{

  A(){
    global::instance<B>()->foo(); 
  }

  void bar();
};


struct B{

 B(){
   global::instance<A>()->bar(); 
 }
 
 void foo();
 
};

void main(){
                
 global::Instance<A> a;                           
 global::Instance<B> b; 
 
}
```
This kind of dependency loop is usually resolved by introducing some form of init-function which is called later on after the construction of the depending instances is complete. This so called two-phase initialization can get quite complex and brittle on bigger projects and is best to be avoided by using the deferred invocation mechanism of the library. With this, the example above could look like that: 

```cpp
struct A{

 A() {
    global::instance<B>().visitIfNotNull(
        [](B& b){
           b.foo();            // defered until b is available
        });
 }

 void bar();
};


struct B{

 B(){ 
    global::instance<A>().visitIfNotNull(
        [](A& a){
           a.bar();            // defered until a is available
        });   
 }

 void foo();
};

void main(){

 global::Instance<A> a;
 global::Instance<B> b;
 
}
```

Note that the order of construction of a and b can be changed without affecting the result. This is one of the advantages of this approach.

Also note that defering calls can be interlaced which allows defering until multiple instances are available e.g. if a class `C` depends on `A` and `B`, this could be expressed as:

```cpp
struct C{

 C() {
     global::instance<A>().visitIfNotNull(
        [](A& a){
             global::instance<B>().visitIfNotNull(
                [](B& b){
                   b.foo();            // defered until a AND b are available
                });
        });
 }

};

```



## How to Use Multiple Instances of The Same Type
The library supports providing access to multiple instances of the same type as shown below:

```cpp
struct A
{
    void foo(){std::cout<<"foo\n";}
};

struct Red{};
struct Green{};
struct Blue{};

void bar();

void main(){

    global::SubInstance<A,Red>   ar;     // creates first instance of type A
    global::SubInstance<A,Green> ag;     // creates second instance of type A
    global::SubInstance<A,Blue>  ab;     // creates third instance of type A
 
    bar();
}                                        

void bar() {
    global::instance<A,Blue>()->foo();   // accessing the third instance of type A
}

```

Note: The access to instances of the same type can be extended to e.g. multidimensional array access by using as index-type a form of `template<int x, int y> struct Index{}`, which would allow accessing the instances via `global::instance<A,Index<4,6>>()`.

## How to Handle Invalid Access
The attempt to access a global instance without a prior registration will by default cause an exception to be thrown. This default behaviour can be changed as follows:
```cpp

void main(){

 struct A{ void foo(){} };
 
 global::instance<A>().onNullPtrAccess =
          [](){ return new A(); };         // define a custom behaviour on invalid access
 
 global::instance<A>()->foo();             // invalid access

}
```
In the example above a new dummy-instance is created and used during the invalid access  instead of throwing an exception.

A second possiblity to change the default behaviour is shown in the following example:
```cpp

void main(){

 struct A{ void foo(){} };
 
 global::onNullptrAccess() = [](){ throw "this is not good"; };
 
 global::instance<A>()->foo();

}
```
The difference between both is that the first one is specific for type `A` and will be called first. If it is not defined the second handler is called, which handles errors for all types. So changing the second one will change the invalid access behaviour for all types which do not have a type-specific handler.

And since the second hanlder is used for all types it cannot provide an alternative instance to be used by the caller. 

## Various Aspects
In this paragraph some minor aspects to using this library will be discussed.

### Thread Savety
This library is not threadsave. But in most cases, this is not a problem since registration/deregistration of global instances usually happen at the beginning and during shutdown which is usually done by a single thread. And in the time between calls to `global::instance<T>()` are constant and therefore thread save.

### Static destruction
Since static variables are used to provide global instance access one should keep in mind that they will run out of scope during static destruction and that they should not be used anyomore at that point in time. The general recommendation is to not access global instances anymore after leaving function main.

 
### Customization
 Since this library is rather small (~200 sloc) with 5 relevant classes it can be customized easily. For more details see section [Under the Hood](#under-the-hood) below.

### Comparision with the classical singleton

The library improves the classical singleton with respect to 
 - testing
 - control over timepoint and seqence of object construction/destruction
 - control over constructor-arguments 
 - two-phase initialization

And although all four aspects could also be implemented in the context of the classical singleton, that implementation would be error prone and not easy read which makes it appear less favourable than the solutions offered by this library.
 
# Under the Hood
 
 In oder to get an idea how the library is implemented it is sufficient to take a closer look at the two main expressions:
  - `global::instance<A>()` and
  - `global::Instance<A>`
 
We begin with the accessor function, which has the signature

```cpp
detail::InstancePointer<A*>& instance();
```

It returns a reference to a static object of type `InstancePointer<A*>`. This object is created the first time the method is called and it holds the pointer to the actual instance of `A` which is accessed by calling `operator->` on it. If no instance of `A` was registered before calling `operator->` the respective error handlers will be called upon calling  `operator->`. The class `InstancePointer<A*>` also provides means to register callable objects which are called after the instance pointer to `A` changes which allows for deferred calls to instances of `A`. 

What we have not seen so far is how an instance of `A` gets registered to the static object of type `InstancePointer<A*>`. This is done by the second of the two main expressions, namely `global::Instance<A>`. This is actually just a type which looks like 

```cpp
template<typename T>
struct Instance {
    T a;
    InstanceRegistration<T> reg;
    Instance() : a, reg(&a) {}
};
```

Creating an instance of it first constructs an instance of `A` and then registers it by creating an instance of type `InstanceRegistration<T>`. Note that the destructor of `InstanceRegistration<T>` deregisters the instance of `A`. By register we mean that the address of the instance of a `A` is given to the respective static object of type `InstancePointer<A*>`. Likewise deregister means here clearing the respective address.   

This concludes the description of the basic mechanism. The rest of the functionality is a detail around the just described central mechanism, eg. error handling and checking. 

