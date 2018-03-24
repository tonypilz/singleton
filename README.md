# Overview
The library improves the classical singleton pattern with respect to 
 - testing
 - controlling timepoint and seqence of object construction/destruction
 - providing constructor-arguments 
 - detecting access to destroyed instances
 
This is achieved by decoupling instance access from the instance lifetime management. The following example illustrates the basic idea/usage:

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
    global::instance<A>()->foo();            // 3) access instance  'a' 
}

```

On line `1)` in the example above, instance `a` gets constructed as a regular object and is then on line `2)` made globally accessible. Then  on line `3)` instance `a` is accessed by function `bar()` via global access. On line `4)` instance `a` gets destructed, and before that happens it gets globally inaccessible due to the destruction of `reg` which is basically as a scoped registration. 

The example above does not cover all aspectes there are to it. Not covered were aspects of 
 - [testing](testing) 
 - [delayed access](delayed-access) 
 - [invalid access detection](invalid-access-detection)
 - [multiple instances of the same type](multiple-instances-of-the-same-type)
 
 which will be discussed below.

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

The second case requires some extra work. In order to test code that accesses global instances those instances usually need to be replaced for the duration of the test by mock-ups. The following example illustrates how this can be done:   

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
    return global::instance<A>()->foo() ? 77 : 66;              // 1) access to global instance of 
}  

void bar_test(){

    A_mock a_mock;                                             // 2)

    global::ReplacingInstanceRegistration<A> reg(&a_mock);     // 3) temporarily make 'a-mock' globally accessible

    assert(bar() == 66);                                       // 4) test of function bar()
    
}                                                              // 5) undo of step 2) 

```
In the example above the function `bar()` is tested in line `4)`. It accesses in line `1)` a global instance of type `A` which will therefore be replaced in line `3)` by the instance `a_mock` for the duration of the test. Therefore during the call to `bar()` in line `4)` the function `A_mock::foo()` will be used, which does not exercise the syscall and merely returns `0`. 

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
 Logger logger;                                             // 3) access to settings
 global::InstanceRegistration<Logger> regL(&logger);        
 Settings settings;
 global::InstanceRegistration<Settings> regS(&settings); 
}
```
In the example above the constructor of `Logger` uses in line `1)` the globally accessible instance of `Settings` and vice versa in line `2)`. Therefore either way one of the two instances will not be fully constructed upon access. The classical solution to this kind of problem would be to fall back to a 2 phase initialisation for global instances which is error prone und reduces readability².

²In many cases program startup is split into multiple stages and the initialization code is dispersed. 

With this library an alternative solution can chosen:
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
   if (readSettingsFile()==success)
     global::instance<Logger>().visitIfNotNull(
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
In the example above all calls to `global::instance<T>().visitIfNotNull()` are deferred until the an instance of `T` becomes globally accessible. So constructing the `Logger` in line `3)` defers the call of line `1)` until `Settings` becomes globally accessible, which is in line `6)`. The construction of `Settings` in line `5)` makes a direct call in line in `2)` to `Logger::log` since an instance of `Logger` is at that point already globally accessible due to line `4)`.

# Invalid Access Detection
The attempt to access a global instance without a prior registration will cause an exception to be thrown by default. This default behaviour can be changed as follows:
```cpp

void main(){

 struct A{};
 
 global::instance<A>().onNullPtrAccess = [](){ return new A(); };    // 1) change default behaviour
 
 global::instance<A>();                                              // 2) invalid global access

}
```
In the example above the invalid access in line `2)` causes a call to line `1)`, which returns a dummy instance to `A` instead of throwing an exception.

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
    global::instance<A,Blue>()->foo();                   // 5)
}

```

In the example above three instances are created in line `1)` and made globally accessible in lines `2)`-`4)`. The third instance is then accessed on line `5)`. 

Note: The access to instances of the same type can be extended to e.g. multidimensional array access by using as index-type a form of `template<int x, int y> struct Index{};`, which would allow to access the instances via `global::instance<A,Index<4,6>>()`


# Bad Pracices
The following practices are not recommended to do: 
 - the hook `instanceChangedHook` shouldnt be changed since its beeing used by. This would disable the delayed access.
 - do not reintroduce coupling between instance access and instance lifetime by putting the registration object into the class to be registered 
