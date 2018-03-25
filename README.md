# Overview
The library improves the classical singleton pattern with respect to 
 - testing
 - control over timepoint and seqence of object construction/destruction
 - control over constructor-arguments 
 - access to destroyed instances detection
 
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
    global::instance<A>()->foo();            // 3) prints "foo"
}

```

On line `1)` in the example above, instance `a` gets constructed as a regular object and is then on line `2)` made globally accessible. Then  on line `3)` instance `a` is accessed by function `bar()` via global access. On line `4)` instance `a` gets destructed, and before that happens its made globally inaccessible by the destructor of `reg`.  In other words `InstanceRegistration` is in effect from its construction until the end of the scope. 

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
 - icc-18
 - gcc-4.9.0 .. gcc-7.3
 - clang 3.4 .. clang 6.0
 
## Status
 - Implementation: complete (~200sloc)
 - Tests: complete
 - Documentation: incomplete


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
    A a;                                    
    global::InstanceRegistration<A> reg(&a); 
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

    A_mock a_mock;                                           

    global::ReplacingInstanceRegistration<A> reg(&a_mock); // 5) temporarily redirect calls to global::instance<A>()

    assert(bar() == 66);                                   // 6) call to bar()
    
}                                                             

```
In the example above, `bar_test()` replaces the instance returned by `global::instance<A>()` in line `5)` before making the call to function `bar()` in line `6)`. Therefore the call to `bar()` in line `6)` will not remove the file `/tmp/myfile` which is the desired behaviour.    

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
 Logger logger;                                                // 3)
 global::InstanceRegistration<Logger> regL(&logger);           // 4)
 Settings settings;                                            // 5)
 global::InstanceRegistration<Settings> regS(&settings);       // 6)
}
```
In the example above all calls to `global::instance<T>().visitIfNotNull()` are deferred until the an instance of `T` becomes globally accessible. So constructing the `Logger` in line `3)` defers the call of line `1)` until `Settings` becomes globally accessible, which is in line `6)`. The construction of `Settings` in line `5)` makes a direct call in line in `2)` to `Logger::log` since an instance of `Logger` is at that point already globally accessible due to line `4)`.

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
Note that since this handler is used for all types the handler cannot provide an alternative instance. 

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

Note: The access to instances of the same type can be extended to e.g. multidimensional array access by using as index-type a form of `template<int x, int y> struct Index{}`, which would allow to access the instances via `global::instance<A,Index<4,6>>()`.

# Various Aspects
In this paragraph, some minor aspects to using this library are discussed.

## Thread Savety
This library is not threadsave. But in most cases, this is not a problem since registration/deregistration of global instances usually happens at the beginning and during shutdown which is usually done by a single thread. And in the time between calls to `global::instance<T>()` are constant and therefore thread save.

## Static destruction
Since static variables are used to provide global instance access one should keep in mind that they run out of scope during static destruction and that they should not be used anyomore at that point in time. 

## Where to Perform the Registrations
In general there is no restriction where to store a registration object and when to make the call to it to make an instance globally accessible. So it can e.g. be located inside the instance to be registered:

```cpp

struct A{
 A(){reg(this);} 
 void foo(){}
 global::InstanceRegistration<A> reg(&a); 
};

void main(){
 A a;                                             // from this point to till the end of the 
                                                  // scope 'a' can be accessed via global::instance<A>()
 global::instance<A>().foo();
}
```
While this looks more elegant and less errorprone than the manual registration seen in the other examples it is not recommended for the following 2 reasons:
 1. It is errorprone, since the registration in the constructor must be done AFTER the class invariant ist established. Failing in doing so would allow global access to an invalid instance which happens more often then one would think.
 2. It combines accessibility management with lifetime management which is exactly what we find in the classical singleton and therefore what brings back most of its drawbacks.
 
 For this reason there is a helper class.
 
 ## When to use Delayed Access
The mechanism of [Delayed Access](#delayed-access) should be used wherever access to global instances could fail due to unfavorable construction sequence of global instances. It is especially usefull in cases where a class depends on more than one one global instance:

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
 Logger logger;                                                
 global::InstanceRegistration<Logger> regL(&logger);           
 Settings settings;                                            
 global::InstanceRegistration<Settings> regS(&settings);       
 Database db;                                                
 global::InstanceRegistration<Database> regS(&db);       
}
```
As can be seen in the example above, the constructor of `Settings` depends on more than one global instance namely `Database` and `Logger`. Resolving this by hand can be quite tedious since `Database` or `Logger` could as well be waiting for other global instances. Using [Delayed Access](#delayed-access) in a nested manner makes this problem rather elegant to solve.
 
 ## Customization
 Since this library is rather small (~200 sloc) with 5 relevant classes it can be customized rather easily. For this see section [Under the Hood](#under-the-hood) below.
 
 # Under the Hood
