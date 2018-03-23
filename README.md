# Overview
This library improves the classical singleton pattern with respect to 
 - testing
 - control over seqcence and timepoint of construction/destruction
 - arguments for the constructor 
 - invalid access detection
 
It is achieved by decoupling instance access from the instance lifetime. The following example illustrates the basic usage of the library:

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

In the example above, instance `a` is constructed as a regular object in `1`, then made globally accessible in `2` which is then accessed by function `bar()` in `3`. On line `4` instance `a` is made globally inaccessible prior to its destruction. 

The example above showed the basic usage of this library. Not shown by the example were the aspects of [testing](#testing) and [delayed access](delayed-access) which will be discussed below.

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
There are two basic cases to be considered:
 1. Testing a globally accessible class and 
 2. Testing code that acesses global instances

The first case is simple. As globally accessible classes are just regular classes they can be tested like a regular class.

The second case requires more attention. In order to test code that accesses global instances those instances usually need to be replaced by mocks during the test. How this can be achieved is illustrated in the following example:   

```cpp
struct A
{
    virtual int foo() { return system("rm /tmp/myfile"); }
};

struct A_mock  : public A
{
    int foo() override { return 0; }
};

int bar(){ 
    return global::instance<A>().foo() ? 77 : 66;              // 1) access to global instance
}  

void bar_test(){

    A_mock a_mock;                                             // 2)

    global::ReplacingInstanceRegistration<A> reg(&a_mock);     // 3) temporarily replace global instance by 'a-mock'

    assert(bar() == 66);                                       // 4) test bar b.bar() uses 'a_mock'
    
}                                                              // 5) undo of step 2) 

```
In the example above the function the function `bar()` is tested by the call in `4`. The function `bar()` accesses in `1` a global instance of type `A` which is therefore be replaced in `3` by the instance `a_mock` before executing the test. So the call to `bar()` in `4` leads to the call of `A_mock::foo()` which skips the syscall and returns `0`. 

# Delayed Access
On larger projects global instances depend on each other as shown in the following example:
```cpp
struct Logger{

 Logger(){
  m_logLevel = global::instance<Settings>().getLogLevel();   // 1)
 }
 
 void log(const char* msg);
 
 int m_logLevel;
};


struct Settings{

 Settings(){
   if (readSettingsFile()==success)
     global::instance<Logger>().log("settings file found");  // 2)
 }
 
 int getLogLevel();
 
};

void main(){
 Logger logger;                                             // 3) access to settings
 global::InstanceRegistration<Logger> regL(&logger);        
 Settings settings;
 global::InstanceRegistration<Settings> regS(&settings); 
}
```
In the example above the constructor of `Logger` uses in `1` the globally accessible instance of `Settings` and vice versa in `2`. Therefore one instance will not have been fully constructed yet upon access. The classical solution to this kind of problem would be to fall back to a 2 phase initialisation for global instances which is error prone und reduces readability².

²In many cases program startup is split into multiple stages and initialization code is dispersed. 

However, this library allows for a better solution:
```cpp
struct Logger{

 Logger() {
    global::onInstanceDefine<Settings>(
        [this](Settings& s){
           m_logLevel = s.getLogLevel();                       // 1)
        });             
 }
 
 void log(const char* msg);
 
 int m_logLevel;
};


struct Settings{

 Settings(){ 
   if (readSettingsFile()==success)
     global::onInstanceDefine<Logger>(
        [this](Logger& l){
           m_logLevel = l.log("settings file found");          // 2)
        });   
 }
 
 int getLogLevel();
 
};

void main(){
 Logger logger;                                                // 3)
 global::InstanceRegistration<Logger> regL(&logger);           // 4)
 Settings settings;                                            // 5)
 global::InstanceRegistration<Settings> regS(&settings);       // 6)
}
```
In the example above all calls to `global::onInstanceDefine<T>()` are deferred until the an instance of `T` becomes globally accessible. So constructing the `Logger` in `3` defers the call in `1` until `Settings` becomes globally accessible in `6`. And constructing `Settings` in `5` calls directly `Logger::log` in `2` since an instance of `Logger` was already made globally accessible by `4`.

# Invalid Access Detection
The attempt to access a global instance without a prior registration will cause an exception to be thrown by default. This default behaviour can be changed as follows:
```cpp

void main(){

 struct A{};
 
 global::instanceHooks::nullptrAccessHook<A>() = [](){ return new A(); };  // 1) change default behaviour
 
 global::instance<Logger>();                                               // 2) invalid global access

}
```
In the example above the invalid access in `2` causes a call to `1`, which returns a dummy instance to A instead of throwing an exception.

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
    A a1,a2,a3;                                          // 1)

    global::InstanceRegistration<A,Red>   reg1(&a1);     // 2)
    global::InstanceRegistration<A,Green> reg2(&a2);     // 3)
    global::InstanceRegistration<A,Blue>  reg3(&a3);     // 4)
    
    bar();
}                                        

void bar() {
    global::instance<A,Blue>().foo();                    // 5)
}

```

In the example above three instances are created in `1` and made globally accessible in `2`-`4`. The third instance is then accessed in `5`. This can also be extended to e.g. multidimensional array access by using as index-type a form of `template<int x, int y> struct Index{};` would lead to accessing instances via `global::instance<A,Index<4,6>>().foo();`


# Bad Pracices
 - do not change the `instanceChangedHook` unless you know what your are doing. This would disable the delayed access.
 - do not reintroduce coupling between instance access and instance lifetime by putting the registration object into the class to be registered 
