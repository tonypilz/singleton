# Overview

The library allows the construction of and access to global instances like the classical singleton, but fixing most of its [drawbacks](#comparision-to-classical-singleton).

The following example illustrates the main usage of the library. It shows how to construct an instance of type `A`, make it globally accessible, access it and how it is destructed.

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
 - [testing](#how-to-do-testing) 
 - [avoid two-phase initialization](#how-to-avoid-two-phase-initialization) 
 - [handle invalid access](#how-to-handle-invalid-access)
 - [pass arguments to the constructor](#how-to-pass-arguments-to-the-constructor)

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
    - [How to Handle Invalid Access](#how-to-handle-invalid-access)
    - [Various Aspects](#various-aspects)
        - [Thread Savety](#thread-savety)
        - [How to Remove the Compiler Warnings About Unused Variables](#how-to-remove-the-compiler-warnings-about-unused-variables)
        - [Behaviour on Exceptions](#behaviour-on-exceptions)
        - [Static Destruction](#static-destruction)
        - [Advantages of having Full Control over Instance Construction/Destruction](#advantages-of-having-full-control-over-instance-constructiondestruction)
            - [Sequence of Construction/Destruction](#sequence-of-constructiondestruction)
            - [Timepoint of Destruction](#timepoint-of-destruction)
        - [Use on Embedded Devices](#use-on-embedded-devices)
        - [How to use Multiple Instances of the Same Type](#how-to-use-multiple-instances-of-the-same-type)
        - [Private Constructors](#private-constructors)
        - [Program Startup/Shutdown Status](#program-startupshutdown-status)
        - [Customizing the Library](#customizing-the-library)
        - [Comparision to Classical Singleton](#comparision-to-classical-singleton)
            - [Other Libraries](#other-libraries)
- [Under the Hood](#under-the-hood)

<!-- /TOC -->

# Status

[![Build Status](https://travis-ci.org/tonypilz/global-instances-access.svg?branch=master)](https://travis-ci.org/tonypilz/global-instances-access)

## Testing

All tests pass under gcc-7.2.0.

## Compiler Support
The library compiles under
 - gcc-4.7 .. gcc-7.3
 - clang 3.4 .. clang 6.0
 - icc-18

The compiler flag `-std=c++11` is used


 
## Installation

just copy the single header file [globalInstances.h](include/globalInstances.h) (~240sloc) into the project and include it (see in the first example).

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

Note that `detail::ReplacingInstanceRegistration<A> reg(nullptr);` can be used to unset temporarily the current instance of `A`.

## How to Avoid Two-Phase Initialization
On larger projects some of the global instances usually access each other during their construction. In the following example, the constructor of type `A` accesses the global instance of type `B` and vice versa:

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

This initialization loop is usually resolved by adding an init-function to `A` which is invoked externally after the construction of the instance of type `B` is completed. This is called two-phase initialization:

```cpp
struct A{

  A(){}
  void init{ global::instance<B>()->foo(); } 

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
 global::instance<A>()->init();
}
```

If projects get bigger, the concept is likely to be extended to mulitple initialization steps, usually triggered by some form of runlevels. 

As one can imagine, this gets complex rather quickly the reason beeing that the initialization logic is dispersed from the construtors to various locations in the code. In the example the intitialization is dispersed from `A::A()` to function `main()` and function `A::init()`. 

Therefore, the libray offers a queuing mechanism which allows to defer initialization steps until the condition for an initialization step is met e.g. a certain global instance becomes available. 

Using the mechanism therefore enables us to keep the initialization code together in the constructor: 

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
Although the example above might not appear to be more readable on first sight, it will be if the projects get bigger with more initialization steps and more complex conditions and some form of runlevels.

Additional notes:

Defering calls can be nested which allows deferring execution until multiple instances become available e.g. a dependence of a class `C` on `A` __and__ `B` could be expressed as:

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
Queuing calls for destruction are also possible (see example [here](#program-startupshutdown-status)).

Arbitrary conditions can also be used in conjunction with deferred calls:

```cpp
struct D{
 
 D() {
     global::instance<A>().addDeferredOperation(
        [this](A* a){
            if (a!=nullptr && this->someCondtion() && a->someOtherCondition()) {
                this->baz(); 
                return global::DeferredOperationState::finished;
            }
            return global::DeferredOperationState::pending;
        });
 }

 bool someCondition();
 void baz();
};
```
The deferred operation will be called each time the instance of `global::instance<A>()` changes until `DeferredOperationState::finished` is returned by the operation.

Also note that the pending operations for an instance `A` will be reexecuted if at least one of the executed operations returned `finished` which allows them to depend on each other.

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
In the example above a new dummy-instance of type `A` is created and used during the invalid access instead of throwing an exception.

The same is possible with an untyped handler which will be called afterwards if defined: 
```cpp
void main(){

 struct A{ void foo(){} };
 
 global::instance<A>().onNullPtrAccessUntyped =
          [](){ std::cout<<"oops\n" };   // define a custom behaviour on invalid access
 
 global::instance<A>()->foo();             // invalid access

}
```
The untyped handler can either throw or do nothing which invokes the global handler, which can also be customized:
```cpp
void main(){

 struct A{ void foo(){} };
 
 global::onNullptrAccess() = [](){ throw "this is not good"; };
 
 global::instance<A>()->foo();

}
```
The first two handlers are effective only for `global::instance<A>()`. The difference between a typed handler and an untyped one is that the typed one knows its type `A` and can therefore return an alterantive instance of `A` to the caller. It will therefore be called prior to the others. If it is not defined the untyped handlers are called, first the local one and then the global one. So customizing the global handler will change the handling of invalid access for all types which do not have a local handlers defined.


## Various Aspects
In this paragraph some minor aspects to using this library will be discussed.

### Thread Savety
This library does not employ synchnization primitives like mutexes. Therefore it is generally not thread-save. However, since the library provides full control over timepoint of construction / destruction it should almost never be necessary to synchronize global instance construction or destruction since program startup and shutdown happen in almost all cases single threaded. And after construction of all global instances is complete all calls to `global::instance<T>()` are thread-save since no data is beeing changed:

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
    static A* a = nullptr;
    if (a==nullptr) a = new global::Instance<A>();  
    mutex.unlock();
}

void main(){

 std::thread t1([](){ makeA(); global::instance<A>()->foo(); });
 std::thread t2([](){ makeA(); global::instance<A>()->foo(); });

 t1.join();
 t2.join();
}
```
### How to Remove the Compiler Warnings About Unused Variables
The compiler can be silenced by using the unused variable:

```cpp
struct A{ void foo(); };

void main(){

 global::Instance<A> a; (void)a;   //no warning

}
```

### Behaviour on Exceptions

If `A` throws an exceptions during its construction within `global::Instance<A> a;`, it will not be made globally accessible and therefore no deferred calls will be triggered.

If a deferred call throws, some of the unexecuted deferred calls will be discarded and therefore never be executed. 

### Static Destruction
Since static variables are used to provide global instance access one should keep in mind that they will be destroyed during static destruction and that they should not be used anyomore at that point in time. The general recommendation is to not access global instances anymore after leaving function main.

### Advantages of having Full Control over Instance Construction/Destruction

The approach of the library to separate instance access management from instances lifetime management gives the user full control over 
1. sequence of construction/destruction
1. timepoint of destruction

which brings following advantages compared to the singleton:

#### Sequence of Construction/Destruction
In the case of classical singeltons, instances are constructed as they are used the first time. Therefore the sequence of construction is subject to unexpected change if classes get changed which is true even if all instances are created in function `main()` since new depencencies could arise over time. In the following example the instances are not created as they are initialized in the main-function:

```cpp
template<typename T>
T& instance() {
    static T t;
    return t;
}

struct B{ void foo(); };
struct A{ A(){ instance<B>().foo(); }};

void main(){
    instance<A>();     
    instance<B>();    // instance of B created BEFORE instance of A
}
```

Additionally, people also tend to forget adding an initializing call of their globals to function main.

The same problem applies for the sequence of destruction since destruction sequence is the inverse of the construction sequence. So reasoning about it is as difficult so reliance on it is not recommended.

The library improves the classical singleton by having a defined the sequence of construction and destruction:

```cpp
#include <globalInstances.h>

struct B{ void foo(); };
struct A{ A(){ instance<B>()->foo(); }};    // throws since B is not constructed yet

void main(){
    global::Instance<A> a;     
    global::Instance<B> b;    
}
```

Having a defined sequence of initialization eventually allows for better reasoning about the code and therefore reduces the likelihood of error and might also save some code which deals with initialization uncertainty.


Note: See section [How to Avoid Two-Phase Initialization](#how-to-avoid-two-phase-initialization) for a way to avoid throwing an exception here.

#### Timepoint of Destruction

In the case of classical singeltons destructors of global instances are of limited use since they cannot savely access other singletons because they cannot determine programatically if other global instances have been destructed already and determining it manually is brittle (see section above).

A variation of the classical singleton (eg used by google) do not destruct at all and lets the operating system do the clean up:

```cpp
template<typename T>
T& instance() {
    static T* t = new T();
    return *t;
}
```

This however disables the constructor completely. 

The advantage of this library is that it allows regular usage of destructors given that they are executed before static destruction happens: (see [here](#static-destruction) for the reason) 

```cpp
struct A{ 
    
    void foo(); 

    ~A(){ 
        if (instance<B>()) instance<B>()->bar(); 
    } 
}; 

struct B{ 
    
    void bar(); 
    
    ~B(){ 
        if (instance<A>()) instance<A>()->foo(); 
    } 
};

void main(){
    global::Instance<A> a;     
    global::Instance<B> b; 

} // instance of B gets destructed first, then instance of A which is why B::bar()
  // wont be called 
```

Note that in section [How to Avoid Two-Phase Initialization](#how-to-avoid-two-phase-initialization) is shown a way to have `B::bar()` called before destructing the instance of `B`.

### Use on Embedded Devices
The library can be used in an embedded environment since it 
 - does not require runtime type information, 
 - calls operator `new`/`delete` only during startup and shutdown<sup>1</sup> and 
 - can be used with exceptions disabled<sup>2</sup>.



<sup>1</sup>After all instances have been created, calls to instances eg `global::instance<T>()->foo()` do not invoke operator `new` new or `delete`. The same applies to all deferred calls eg `global::instance<T>().ifAvailable()` if they can be executed directly. So if they can not be executed directly because e.g an instance has not been created yet, the calls will be queued which invokes the operator `new`.

<sup>2</sup>If exceptions are disabled all errors will be handled by invoking `exit()` instead of throwing an exception.


### How to use Multiple Instances of the Same Type
In order to globally access multiple instances of the same type one simply needs to add a template parameter to the type which is to be accessed:

```cpp
template<typename Instance = void>
struct A
{
    void foo(){std::cout<<"foo\n";}
};

struct Red{};
struct Green{};
struct Blue{};

void bar();

void main(){

    global::Instance<A<Red>>   ar;     // creates first instance of type A
    global::Instance<A<Green>> ag;     // creates second instance of type A
    global::Instance<A<Blue>>  ab;     // creates third instance of type A
 
    global::Instance<A>   a;           // creates default instance of type A
  
    bar();
}                                        

void bar() {
    global::instance<A<Blue>>()->foo(); // accessing the third instance of type A
}

```

Note: The access to instances of the same type can be extended to e.g. multidimensional array access by using an integral type `template<int x, int y> struct A{...};`, which would allow accessing the instances via `global::instance<A<4,6>>()`.

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

    template< template<typename>class,
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

    global::instance<GlobalInstances>().becomesUnavailable(
        [](GlobalInstances&){
           std::cout<<"Global instances about to be destructed\n";
        });

    global::instance<RunlevelX>().ifAvailable(
        [](RunlevelX&){
           std::cout<<"RunlevelX entered\n";
        });

    global::instance<RunlevelX>().becomesUnavailable(
        [](RunlevelX&){
           std::cout<<"RunlevelX left\n";
        });

    global::Instance<GlobalInstances> g;

    // all instances constructed, main loop can be entered

    mainLoop();
}

void bar() {
    bool g = global::instance<GlobalInstances>();
    bool r = global::instance<RunlevelX>();
    std::cout << "All global instances available: " << g 
              << ", runlevel X entered: " << r << "\n";
}

```
As can be seen in the example, the instance of `GlobalInstances` has two purposes. First, constructing and destructing all global instances and second, serve as indicater if all global instances have been fully constructed. It also helps to avoid cluttering the main function with the construction code of the global instances.

The indicator instance of type `RunelevelX` is used here to indicate a certain program startup and shutdown state. Therefore it can be used to trigger a two phase initialization. However, as discussed in [this section](#how-to-avoid-two-phase-initialization), a two phase initialization should be avoided if possible which is why there should rarely be the need for an indicator instance. 

### Customizing the Library
 Since this library is rather small (~240 sloc) with 5 relevant classes it can be customized fairly easy. For more details see section [Under the Hood](#under-the-hood) below.

### Comparision to Classical Singleton

By classical singleton something like the following is meant here:

```cpp
template<typename T>
T& instance() {
    static T t;
    return t;
}
```

And library adresses its drawbacks, which are:
 1. [testability](#how-to-do-testing)
 1. [requiring a two-phase initialization](#how-to-avoid-two-phase-initialization)
 1. [lack of control over the construction/destruction sequence of instances](#sequence-of-constructiondestruction)
 1. [timepoint of destruction (if any) is always during static deinitialization](#timepoint-of-destruction)
 1. [no arguments can be passed to the constructor of instances](#how-to-pass-arguments-to-the-constructor)

The linked sections explain in more detail why they are drawbacks and how they are improved by this library.


#### Other Libraries

Most of the singleton libraries found on github in April 2018 were demos/examples or random implementations within other projects. The remaining dedicated singleton implementations will be compared in the following table. The indicators are __+__ for improvement over the classical singelton, __=__ for comparable, __-__ for worsening. The numbers 1-5 refer to the drabacks listed above.

| Aspect   | 1 | 2 | 3             | 4 | 5             | automatic destruction | threadsave construction | forces virtual destructor | thread local instances | dependency   |
|----------|---|---|---------------|---|---------------|-----------------------|-------------------------|---------------------------|------------------------|--------------|
| This Lib | + | + | +             | + | +             | =                     | -                       | =                         | =                      | stl          |
| [herpe]  | = | = | =<sup>2</sup> | = | =<sup>1</sup> | =                     | =                       | -                         | =                      | stl          |
| [ugrif]  | = | = | =<sup>2</sup> | + | =             | -                     | -                       | =                         | =                      | /            |
| [xytis]  | = | = | =<sup>2</sup> | + | =             | -<sup>3</sup>         | -                       | =                         | =                      | stl          |
| [aworx]  | = | = | =<sup>2</sup> | + | =             | -                     | -<sup>5</sup>           | -                         | =                      | stl, ALib    |
| [fwolt]  | = | = | =             | = | +             | =                     | =                       | =                         | =                      | stl, fw      |
| [zyf38]  | = | = | =<sup>2</sup> | = | =             | =                     | =                       | =                         | =                      | stl          |
| [cheno]  | + | = | =<sup>2</sup> | + | =             | -<sup>4</sup>         | =                       | =                         | +                      | stl, pthread |
| [cppma]  | + | = | +<sup>2</sup> | + | +<sup>6</sup> | =                     | =                       | =                         | =                      | stl, boost   |

 <sup>1</sup> Implementation of constructor arguments incorrect

 <sup>2</sup> Possibly susceptible to static initialization order problems due to using raw static variables

 <sup>3</sup> Access after manual destruction causes undefined behaviour
 
 <sup>4</sup> Requires setting the instance manually to `nullptr` after deleting it manually

 <sup>5</sup> Implementation of manual locking/unlocking incorrect

 <sup>6</sup> Up to 4 Arguments
 



[herpe]: https://github.com/herpec-j/Singleton
[ugrif]: https://github.com/ugriffin/VSSynthesiseSingleton
[xytis]: https://github.com/xytis/typedef-singleton
[aworx]: https://github.com/AlexWorx/ALib-Singleton
[fwolt]: https://github.com/FlorianWolters/cpp-component-util-singleton
[zyf38]: https://github.com/zyf3883310/C-11-Thread-Safe-Singleton
[cheno]: https://github.com/chenokay/selib
[cppma]: https://github.com/cppmaven/Singularity

# Under the Hood
 
 In oder to get an idea how the library is implemented it is sufficient to take a closer look at the two main expressions:
  - `global::instance<A>()` and
  - `global::Instance<A>`
 
We begin with the accessor function, which has the signature

```cpp
detail::InstancePointer<A>& instance();
```

It returns a reference to a static object of type `InstancePointer<A>` which is created on the first time the method is called. The object holds the pointer to the actual instance of type `A` which is accessed by calling `operator->` on it. If no instance of type `A` was registered before and `operator->` is called the respective error handlers will be triggered. The class `InstancePointer<A>` also provides means to register callable objects which are called if the pointer to the actual instance changes. This enables the deferred calling mechanism. 

What remains to be shown is how `InstancePointer<A>` gets the actual pointer of instance of type `A`. This is done by constructing an instance of type `global::Instance<A>` which (simplified) looks like:  

```cpp
template<typename T>
struct Instance {

    T a;                               // construct the actual instance

    InstanceRegistration<T> reg;       // scoped registration
    
    Instance() : reg(&a) {}            // constructor of reg copies the adress of a to
                                       // global InstancePointer<T>-instance 
};
```

As we can see `Instance<A>` constructs an instance of type `A` (=`a`) as well as a registration object `reg` of type `InstanceRegistration<A>`. The registration object copies the given adress of `a` during its construction to the global instance of `InstancePointer<A>` and clears it during its destruction.    

This concludes the description of the basic mechanism. The rest of the functionality is a detail around the just described central mechanism, eg. error handling and checking. 

