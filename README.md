# Overview
The library improves the classical singleton pattern with respect to 
 - testing
 - control over timepoint and seqence of object construction/destruction
 - control over constructor-arguments 
 - access to destroyed instances detection
 
This is achieved by decoupling instance access from the instance lifetime management. The following example illustrates the basic idea/usage:

```cpp
#include <globalInstances.h>

void bar(); 

struct A
{
    void foo(){std::cout<<"foo\n";}
};

void main(){
    global::Instance<A> a;                   // 1) construct instance of A and make it globally accessible
    bar();
}                                            // 2) deregistration destruction of 'a'

void bar() {
    global::instance<A>()->foo();            // 3) prints "foo"
}

```

On line `1)` in the example above, instance `a` gets constructed as a regular object and is made globally accessible. Then on line `3)` the instance of `A` is accessed by function `bar()` via global access. Finally on line `2)` the instance of `A` gets destructed, and before that happens it is made globally inaccessible. 

The example above does not cover all aspectes there are to it. Not covered were
 - [testing](#testing) 
 - [delayed access](#delayed-access) 
 - [invalid access detection](#invalid-access-detection)
 - [multiple instances of the same type](#multiple-instances-of-the-same-type)
 - [various aspects](#various-aspects)
 
 which will be discussed in the linked sections below.

# Status
## Tested on
 - gcc-7.2.0

## Compiles Under
 - gcc-4.7 .. gcc-7.3
 - clang 3.4 .. clang 6.0
 - icc-18

The compiler flag `-std=c++11` is used
 
## Status
 - Implementation: complete (~200sloc - [see single header file](include/globalInstances.h))
 - Tests: complete
 - Documentation: complete

### Todos:
 - run static code analysis (clang static analyser, cppcheck)
 - run dynamic code analysis (valgrind.memcheck, clang adress sanitizer)

# Testing 
There are two basic cases to be considered:
 1. testing a globally accessible class and 
 2. testing code that accesses global instances.

The first case is simple. As globally accessible classes are just regular classes they can be tested like a regular class.

The second case requires some extra work. An example for the second case would be:

```cpp
struct A
{
    virtual int foo() { return system("rm /tmp/myfile"); }      // 1) remove file
};

int bar(){                                                      // 2) to be tested

    return global::instance<A>()->foo() ? 77 : 66;              // 3) access to a global instance 
}  

void bar_test(){                                                // 4) test of bar()

    assert(bar() == 66);   
}

void main(){                           
    global::Instance<A> a;
    bar_test();
}  
 
```
The example above tests function `bar()` with actually removes the file `/tmp/myfile` caused by the call to `A::foo()` in line `3)`. One way of avoiding removing the file during the test would be to replace the instance returned by `global::instance<A>()` in line `3)` which is shown below:

```cpp

struct A_mock  : public A
{
    int foo() override { return 0; }
};

void bar_test(){

    global::TestInstance<A,A_mock> a_mock;     // 5) temporarily replace the global instance
                                               //    of 'A' by an instance of 'A_mock'

    assert(bar() == 66);                       // 6) call to bar()
}                                                             

```
In the example above, `bar_test()` replaces the instance returned by `global::instance<A>()` in line `5)` before making the call to function `bar()` in line `6)`. Therefore the call to `bar()` in line `6)` will not remove the file `/tmp/myfile` which is the desired test-behaviour.    

# Delayed Access
On larger projects global instances depend on each other as shown in the following example:
```cpp
struct Logger{

  Logger(){
     m_logLevel = global::instance<Settings>()->getLogLevel();   // 1)
  }

  void log(const char* msg);

  int m_logLevel;
};


struct Settings{

 Settings(){
   if (readSettingsFile()==success)
     global::instance<Logger>()->log("settings file found");  // 2)
 }
 
 int getLogLevel();
 
};

void main(){
                
 global::Instance<Logger> logger;                             // 3) accesses settings
 global::Instance<Settings> settings; 
 
}
```
In the example above the constructor of `Logger` uses in line `1)` the globally accessible instance of `Settings` and in line `2)` vice versa. Therefore one of the two instances will not be fully constructed upon access. The solution to this kind of problem is a two phase initialisation which has a couple of problems:
 - initialization code is split onto two or more methods
 - state space about initialization state is increased  
 - class internal knowledge like 'if class A is constrcuted then do b' might leak out
 - fragility raises since implicit assumptions about initialization order are embedded into the code

All of these problems can be adressed by a queuing mechanism which allows to automatically execute initialization steps if global instances become accessible as shown in the example below: 

```cpp
struct Logger{

 Logger() {
    global::instance<Settings>().visitIfNotNull(
        [this](Settings& b){
           m_logLevel = s.getLogLevel();                       // 1)
        });             
 }

 void log(const char* msg);
 
 int m_logLevel;
};


struct Settings{

 Settings(){ 
   if (readSettingsFile())
     global::instance<Logger>().visitIfNotNull(
        [this](Logger& l){
           l.log("settings file found");                       // 2)
        });   
 }
 
 bool readSettingsFile();
 int getLogLevel();
 
};

void main(){

 global::Instance<Logger> logger;                             // 3)
 global::Instance<Settings> settings;                         // 4)
 
}
```
In the example above all calls to `global::instance<T>().visitIfNotNull()` are deferred until the an instance of `T` becomes globally accessible. So constructing the `Logger` in line `3)` defers the call of line `1)` until `Settings` becomes globally accessible, which is in line `4)`. The construction of `Settings` in line `4)` makes a direct call in line `2)` to `Logger::log` since an instance of `Logger` is at that point already globally accessible due to line `3)`.

# Invalid Access Detection
The attempt to access a global instance without a prior registration will by default cause an exception to be thrown. This default behaviour can be changed as follows:
```cpp

void main(){

 struct A{};
 
 global::instance<A>().onNullPtrAccess = [](){ return new A(); };    // 1) change default behaviour
 
 global::instance<A>();                                              // 2) invalid global access

}
```
In the example above the invalid access in line `2)` causes a call to line `1)`, which returns a dummy instance to `A` instead of throwing an exception.

Note that there can also be a customized global handler which is used if no local one (as customized in the example above) is available:
```cpp

void main(){

 struct A{};
 
 global::onNullptrAccess() = [](){ throw "this is not good"; };
 
 global::instance<A>();

}
```
Note that since this handler is used for all types the handler cannot provide an alternative instance to be used to the caller. 

# Multiple Instances of the Same Type
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

    global::SubInstance<A,Red>   ar;     // 1)
    global::SubInstance<A,Green> ag;     // 2)
    global::SubInstance<A,Blue>  ab;     // 3)
 
    bar();
}                                        

void bar() {
    global::instance<A,Blue>()->foo();   // 4)
}

```

In the example above three instances are created and made globally accessible in lines `1)`-`3)`. The third instance is then accessed on line `4)`. 

Note: The access to instances of the same type can be extended to e.g. multidimensional array access by using as index-type a form of `template<int x, int y> struct Index{}`, which would allow to access the instances via `global::instance<A,Index<4,6>>()`.

# Various Aspects
In this paragraph some minor aspects to using this library will be discussed.

## Thread Savety
This library is not threadsave. But in most cases, this is not a problem since registration/deregistration of global instances usually happens at the beginning and during shutdown which is usually done by a single thread. And in the time between calls to `global::instance<T>()` are constant and therefore thread save.

## Static destruction
Since static variables are used to provide global instance access one should keep in mind that they run out of scope during static destruction and that they should not be used anyomore at that point in time. The general recommendation is to not access global instances anymore after leaving function main.

## When to use Delayed Access
The mechanism of [Delayed Access](#delayed-access) should be used whenever access to global instances could fail due to unfavorable construction sequence of global instances. It is especially usefull in cases where a class depends on more than one global instance:

```cpp

struct Logger{
    void log(const char* msg);
};

struct Database{
    const char* readSettings();
}

struct Settings{

   Settings(){ 
     global::instance<Database>().visitIfNotNull(
       [this](Database& db){

          this->setData(db.readSettings());

          global::instance<Logger>().visitIfNotNull(
            [this](Logger& l){

              l.log("settings file found in database");

           });   
   }

};

void main(){        

   global::Instance<Logger> logger;                                                    
   global::Instance<Settings> settings;                                              
   global::Instance<Database> db;     
 
}
```
As can be seen in the example above, the constructor of `Settings` depends on more than one global instance namely `Database` and `Logger`. Resolving this by hand can be quite tedious since `Database` or `Logger` could as well be waiting for other global instances. Using [Delayed Access](#delayed-access) in a nested manner makes this an easy solvable problem.
 
 ## Customization
 Since this library is rather small (~200 sloc) with 5 relevant classes it can be customized easily. For more details see section [Under the Hood](#under-the-hood) below.
 
 # Under the Hood
 
 To understand how it works we have to take a closer look at the two central expressions from the first example:
  - `global::instance<A>()->foo();` and
  - `global::Instance<A> a;`
 
We begin with `global::instance<A>()->foo();`

The expression `global::instance<A>()` is a regular function, which returns a reference to a globally accessible static object of type `InstancePointer<A*>`. This object is created the first time the method is called and it holds the pointer to the actual instance of `A` which should be accessed globally. The expression `global::instance<A>()->foo();` can therefore also be written as

```cpp
InstancePointer<A*>& ref = global::instance<A>();
ref->foo();
```

The class `InstancePointer<A*>` has its own the `operator ->` which returns a pointer to the last registered instance of type `A`. If none was registered then the respective error handlers will be called. The class `InstancePointer<A*>` also provides means to register operations which are called after the instance pointer to `A` changes which makes delayed access to `A` possible. 

What we have not seen so far is how an instance of `A` gets registered to the static object of type `InstancePointer<A*>`. This is done by the second central expression `global::Instance<A> a;`. What happens here is that an object of type `RegisterdInstanceT`: 

```cpp
struct RegisterdInstanceT {
    A a;
    InstanceRegistration<A> reg;
    RegisterdInstanceT() : a, reg(&a) {}
};
```
is created which is an aggregate of two objects, the first beeing an instance `A` and the second beeing a scoped registration object. This object registers a given adress to an instance of `A` in its constructor and deregisteres it in its destructor. Register means here that the provided address of `A` is copied to the above mentioned globally accessible object of type `InstancePointer<A*>`. Deregister means here clearing the address copied before.   

This concludes the basic mechanism. The rest of the functionality is a detail around the just described central mechanism, eg. error handling and checking. 

