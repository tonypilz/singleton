# Overview

The library allows the construction of and access to global instances which makes it an alternative to the classical singleton. The advantage is that [most of the drawbacks of the singleton are avoided](#comparision-with-the-classical-singleton).

The following example illustrates the main usage of the library. It shows how to construct an instance of type `A`, make it globally accessible, accessing and destructing it.

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



Besides the main usage above, the following aspects are also covered by this library: 
 - [how to do testing](#how-to-do-testing) 
 - [how to avoid two-phase initialization](#how-to-avoid-two-phase-initialization) 
 - [how to use multiple instances of the same type](#how-to-use-multiple-instances-of-the-same-type)
 - [how to handle invalid access](#how-to-handle-invalid-access)
 - [how to pass arguments to the constructor](#how-to-pass-arguments-to-the-constructor)

The remainder of the document discusses the library in more detail.

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
    - [How to Pass Arguments to the Constructor](#how-to-pass-arguments-to-the-constructor)
    - [How to use Multiple Instances of the Same Type](#how-to-use-multiple-instances-of-the-same-type)
    - [How to Handle Invalid Access](#how-to-handle-invalid-access)
    - [Various Aspects](#various-aspects)
        - [Thread Savety](#thread-savety)
        - [Behaviour on Exceptions](#behaviour-on-exceptions)
        - [Static Destruction](#static-destruction)
        - [Private Constructors](#private-constructors)
        - [Program Startup/Shutdown Status](#program-startupshutdown-status)
        - [Customizing the Library](#customizing-the-library)
        - [Comparision with the Classical Singleton](#comparision-with-the-classical-singleton)
        - [Comparision to Other Libraries](#comparision-to-other-libraries)
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
 1. test a globally accessible class and 
 2. test code that accesses global instances.

The first case is simple. As globally accessible classes are just regular classes they can be tested like them.

The second case requires some extra work which is illustrated by the following example:

```cpp
struct A
{
    virtual int foo() { return system("rm /tmp/myfile"); } 
};

int bar(){
    return global::instance<A>()->foo() ? 77 : 66;  // access a global instance 
}  

void main(){
    global::Instance<A> a;
    bar();
}  
 
```
Let's assume we want to test function `bar()` in without actually deleting the file `/tmp/myfile`. This can be achieved by replacing the instance of `A` by a mock for the duration of the test. The additional test code would look like this:

```cpp
struct A_mock : public A
{
    int foo() override { return 0; }
};

void bar_test(){

    global::TestInstance<A,A_mock> a_mock;     // replace global instance of 'A' by 
                                               // an instance of 'A_mock'
    assert(bar() == 66);
}                                              // undo replace

```

So for the scope of `bar_test()` all calls to `global::instance<A>()` are directed to an instance of `A_mock`. After the end of the scope of `bar_test()` calls to  `global::instance<A>()` will end up in the original instance of `A`.

## How to Avoid Two-Phase Initialization
On larger projects some of the global instances usually access each other during their construction. In the following example, a class `A` accesses globally an instance of type `B` during its construction and vice versa:

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
                
 global::Instance<A> a;     // throws since no instance of B available yet
 global::Instance<B> b;     
 
}
```
This kind of dependency loop is usually resolved by introducing some form of init-function which is called later on after the construction of the depending instances is complete. This so called two-phase initialization can get quite complex and brittle on bigger projects and can be avoided by using the deferred invocation mechanism provided by the library. With it, a non-throwing version of the example above could look like that: 

```cpp
struct A{

 A() {
    global::instance<B>().ifAvailable(
        [](B& b){
           b.foo();            // defered until b is available
        });
 }

 void bar();
};


struct B{

 B(){ 
    global::instance<A>().ifAvailable(
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

Note that the order of construction of `a` and `b` can be changed without affecting the result. This is one of the advantages of this approach.

Also note that defering calls can be nested which allows deferring execution until multiple instances are available e.g. a dependence of a class `C` on `A` __and__ `B` could be expressed as:

```cpp
struct C{

 C() {
     global::instance<A>().ifAvailable(
        [](A& a){
             global::instance<B>().ifAvailable(
                [](B& b){
                   b.foo();            // defered until a AND b are available
                });
        });
 }

};

```

## How to Pass Arguments to the Constructor

Arguments can be passed to the constructor just like regular ones:

```cpp
struct A { 
    A(int, std::string){}
};

void main(){
    global::Instance<A> a(2,"hello");  
}
```
## How to use Multiple Instances of the Same Type
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
This library does not employ synchnization primitives like mutexes. Therefore it is generally not thread-save. However, since the library provides full control over timepoint of construction / destruction it should almost never be necessary to synchronize global instance construction or destruction since program startup and shutdown happen in almost all casses single threaded. And after construction of all global instances is complete all calls to `global::instance<T>()` are thread-save since no data is beeing changed:

```cpp
struct A{ void foo(); };

void main(){

 // single threaded construction
 global::Instance<A> a;  
 
 // multithreaded access to global::instance<A>() is fine
 std::thread t1([](){ global::instance<A>()->foo(); });
 std::thread t2([](){ global::instance<A>()->foo(); });

 t1.join();
 t2.join();

 // single threaded destruction
 
}
```

Note that this argument is not true for singleton implementations, since the timepoint of construction could unexpectedly be during multithreaded runtime, which happens eg if we forgot to construct an instance eagerly at startup which usually goes unnoticed. But since the construction of a classical singleton is threadsave by default, thread savety does not pose a problem too.

Also note, that since we have full controll over timepoint of construction synchronization can always been added manually if needed: 

```cpp
struct A{ void foo(); };

void makeA{
    // construction in a multithreaded environment
    static std::mutex mutex;
    mutex.lock();
    static auto a = new global::Instance<A>();  
    mutex.unlock();
}

void main(){

 std::thread t1([](){ makeA(); global::instance<A>()->foo(); });
 std::thread t2([](){ makeA(); global::instance<A>()->foo(); });

 t1.join();
 t2.join();
}
```

### Behaviour on Exceptions

If `A` throws an exceptions during construction of `global::Instance<A> a;`, it will not be made globally available and therefore no deferred calls will be triggered.

If a deferred call throws, some of the unexecuted deferred calls will be discarded and therefore never be executed. 

### Static Destruction
Since static variables are used to provide global instance access one should keep in mind that they will be destroyed during static destruction and that they should not be used anyomore at that point in time. The general recommendation is to not access global instances anymore after leaving function main.

### Private Constructors
In order to be able to declare constructors private, one has to declare friendship as shown in the example below:

```cpp
struct A{

private:
    A(){}
    GLOBAL_INSTANCE_IS_FRIEND;
};

void main(){
    global::Instance<A> a;  
}
```

Unfortunately we cannot avoid a macro here since `global::Instance<A>` is a type-alias which cannot appear in friend declarations.

Note that without the macro we would have to declare friendship as shown below: 

```cpp
struct A{

private:
    A(){}

    template< template<typename,typename>class,
              typename ,
              typename ,
              typename >
    friend class global::detail::RegisterdInstanceT;
};

void main(){
    global::Instance<A> a;  
}
```

### Program Startup/Shutdown Status 

On bigger projects some information about the startup and shutdown state are usually desirable. This can be achieved by adding indicator instances:

```cpp
struct A{};
struct B{};
struct C{};
struct D{};
struct RunlevelX{};

class GlobalInstances
{
    global::Instance<A> a;
    global::Instance<B> b;
    global::Instance<RunlevelX> runlevelX;
    global::Instance<C> c;
    global::Instance<D> d;
}

void main(){

    bar();

    global::instance<GlobalInstances>().ifAvailable(
        [](GlobalInstances&){
           std::cout<<"All global instances constructed\n";
        });

    global::instance<RunlevelX>().ifAvailable(
        [](RunlevelX&){
           std::cout<<"RunlevelX entered\n";
        });

    global::Instance<GlobalInstances> g;

    // all instances constructed, main loop can be entered

    bar();

    mainLoop();

    global::instance<GlobalInstances>().ifUnavailable(
        [](GlobalInstances&){
           std::cout<<"Global instances about to be destructed\n";
        });

    global::instance<RunlevelX>().ifUnavailable(
        [](RunlevelX&){
           std::cout<<"RunlevelX left\n";
        });
}

void bar() {
    bool v = global::instance<GlobalInstances>();
    bool x =  global::instance<RunlevelX>();
    std::cout << "All global instances available: " << v 
              << ", runlevel X entered: " << x << "\n";
}

```
As can be seen in the example, the instance of `GlobalInstances` has two purposes. First, constructing and destructing all global instances and second, serve as indicater if all global instances have been fully constructed. It also helps to avoid cluttering the main function with the construction code of the global instances.

The indicator instance of type `RunelevelX` is used here to indicate a certain program startup and shutdown state. Therefore it can be used to trigger a two phase initialization. However, as discussed in [this section](#how-to-avoid-two-phase-initialization), a two phase initialization should be avoided if possible which is why there should rarely be the need for one. 

### Customizing the Library
 Since this library is rather small (~200 sloc) with 5 relevant classes it can be customized fairly easy. For more details see section [Under the Hood](#under-the-hood) below.

### Comparision with the Classical Singleton

The library set out to adress the following minor and major drawbacks of the classical singleton:
 - testability
 - sometimes requires two-phase initialization
 - missing control over construction sequence of instances
 - missing control over destruction sequence of instances
 - destruction during static deinitialization
 - no arguments to the constructor of instances

So the main difference to the classical singleton lies in the improvement of these drawbacks.

Note: By classical singleton something like the following is meant:

```cpp
template<typename T>
T& instance() {
    static T t;
    return t;
}
```

### Comparision to Other Libraries

Most of the singleton libraries found on github in April 2018 were demos/examples or private implementations. The remainder will be compared in the following table: 

Feature | This Lib  | Classical Singleton | [herpe] | [ugrif] | [xytis] | [aworx] | [fwolt] | [zyf38] | [cheno]
--- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- 
supports instance replacement for testing | X | - | - | - | - | - | - | - | X
automatic 2-phase initialization | X | - | - | - | - | - | - | - | -
control over construction seqence | full | limited | limited<sup>2</sup> | limited<sup>2</sup> | limited<sup>2</sup> | limited<sup>2</sup> | limited | limited<sup>2</sup> | limited<sup>2</sup>
control over destruction seqence | full | none | none | full | full | full | none | none | full
control over destruction point in time |  full | none | none | full | full | full | none | none | full
automatic destruction | X | X | X | - | -<sup>3</sup> | - | X | X | -<sup>4</sup>
constructor arguments | X | - | X<sup>1</sup> | - | - | - | X | - | -
threadsave construction | - | X | X | - | - | X<sup>5</sup> | X | X | X
implementation pattern | indep. class | function | CRTP | macro |  indep. class  | CRTP | CRTP | indep. class | indep. class 
forces virtual destructor | - | - | X | - | - | X | - | - | -
thread local instances | - | - | - | - | - | - | - | - | X
 <sup>1</sup> Implementation of constructor arguments incorrect

 <sup>2</sup> Possibly susceptible to static initialization order problems due to using raw static variables

 <sup>3</sup> Access after manual destruction causes undefined behaviour
 
 <sup>4</sup> Requires setting the instance manually to `nullptr` after deleting it manually

 <sup>5</sup> Implementation of manual locking/unlocking incorrect

CRTP = curiously recurring template pattern

indep. class = independed free standing class(es) which do not require inheritance

[herpe]: https://github.com/herpec-j/Singleton
[ugrif]: https://github.com/ugriffin/VSSynthesiseSingleton
[xytis]: https://github.com/xytis/typedef-singleton
[aworx]: https://github.com/AlexWorx/ALib-Singleton
[fwolt]: https://github.com/FlorianWolters/cpp-component-util-singleton
[zyf38]: https://github.com/zyf3883310/C-11-Thread-Safe-Singleton
[cheno]: https://github.com/chenokay/selib

# Under the Hood
 
 In oder to get an idea how the library is implemented it is sufficient to take a closer look at the two main expressions:
  - `global::instance<A>()` and
  - `global::Instance<A>`
 
We begin with the accessor function, which has the signature

```cpp
detail::InstancePointer<A*>& instance();
```

It returns a reference to a static object of type `InstancePointer<A*>` which is created on the first time the method is called. The object holds the pointer to the actual instance of type `A` which is accessed by calling `operator->` on it. If no instance of type `A` was registered before calling `operator->` the respective error handlers will be triggered. The class `InstancePointer<A*>` also provides means to register callable objects which are called if the pointer to the actual instance changes. This enables the deferred calling mechanism. 

What we have not seen so far is how an instance of type `A` gets registered to the static object of type `InstancePointer<A*>`. This is done by the second of the two main expressions, namely `global::Instance<A>` which constructs an instance of the following type:  

```cpp
struct Instance<A> {

    A a;
    InstanceRegistration<A> reg;
    
    Instance() : a, reg(&a) {}
};
```

As we can see `Instance<A>` contains the actualy instance `a` of type `A` as well as a registration object `reg` of type `InstanceRegistration<A>`. A registration object registers a given object pointer during its construction and deregisters it during its destruction. Thus `a` gets registered by `reg` after beeing constructed by `Instance<A>`. By register we mean that the address of the instance of a `A` is given to the respective static object of type `InstancePointer<A*>`. Likewise deregister means here clearing the respective address.   

This concludes the description of the basic mechanism. The rest of the functionality is a detail around the just described central mechanism, eg. error handling and checking. 

