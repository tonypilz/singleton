# Overview
This library improves the classical singleton pattern by separating the concerns of global access from the object lifetime management. It therefore avoids the drawbacks created by the combination of both namely:
 - testability
 - constructor arguments
 - object construction/destruction sequence
 - timepoint of object destruction
 - detection of access to destructed objects
 
 ## Example (classical singleton)

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
    A::instance().foo();                // instance construction and access
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
    global::instance<A>().foo();              // instance access
}

void main(){
    A a;                                     // instance construction
    global::InstanceRegistration<A> reg(&a); // establish globally accessibility
    bar();
}
```
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

# Details 

## Testability
The globally accessible instance can be replaced at any time by eg. a mocked version. In the following example the instance accessible by `global::instance<A>()` will be temporarily replaced by an instance of type `A_mock` during the scope of `testB()`. This allows testing of class B which uses a global instance of type `A` for its computation.
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
## Constructor Arguments
Since the library is only concerned with the access to instances and not with their lifetime, there are no restrictions to the arguments of the constructor.

## Object Construction/Destruction Sequence
As before, since the library is only concerned with providing access to instances and not with their lifetime, there are no restrictions to the sequence of construction and destruction.

## Timepoint of Object Destruction
As before, since the library is only concerned with providing access to instances and not with their lifetime, there are no restrictions to the point of time for their destruction.

## Detection of Accessing Destructed Objects
On the destruction of global instances they will be deregistered for global access. Any global access afterwards will be handled as if no instance was registered. The behaviour for that case can be customized.

# Library Usage / Best practices

## Registration to be Globally Accessible
In order to make an instance `a` of type `A` globally accessible via `global::instance<A>()` it must be registered. This registration can be done by using the class `global::InstanceRegistration<A>`:
```cpp

struct A{
 void foo(){}
};

void main(){
 A a;
 global::InstanceRegistration<A> reg(&a); 
 // from this point to till the end of the scope 'a' can be accessed via global::instance<A>()
 
 global::instance<A>().foo();
 
}
```

Note: A registration can also be done inside the constructor of `A`:

```cpp

struct A{
 A(){reg(this);} 
 void foo(){}
 global::InstanceRegistration<A> reg(&a); 
};

void main(){
 A a;  // from this point to till the end of the scope 'a' can be accessed via global::instance<A>()
 global::instance<A>().foo();
}
```
However, this form is not recommended since it combines global accessiblity with lifetime management again which is the exact opposite of what we set forth to achive. Not surprisingly this approach reintroduces some of the drawbacks of the classical singletons! Also note that on this approach the registering call `reg(this);` must be made after class construction is complete or otherwise an incomplete instance could be accessd.

## Instance Lifetime Management 
Global instances should have a predictable construction and destruction. Additionally, the timepoint of destruction should not be during static destruction since debugging there can be quite challenging. A fulfilling approach would be:
```cpp
struct A{};
struct B{};
struct C{};

void doStuff(){}

void main_(){

    A a;
    global::InstanceRegistration<A> regA(&a); 
    B b;
    global::InstanceRegistration<B> regB(&b); 
    C c;
    global::InstanceRegistration<C> regC(&c); 

    doStuff();

} //on scope exit c, b and a will be deregistered before their destruction
```
Note that on large scale systems construction and registration of a,b and c can be moved to one or more helper classes:  
```cpp
struct A{};
struct B{};
struct C{};

struct Helper {
    A a;
    global::InstanceRegistration<A> regA; 
    B b;
    global::InstanceRegistration<B> regB; 
    C c;
    global::InstanceRegistration<C> regC; 
    
    Helper():regA(&a),regB(&b),regC(&c){}
};

void doStuff(){}

void main_(){
    Helper h;
    doStuff();
} 
```

## Invalid Access Detection
If `global::instance<A>()` is called without prior instance registration an exception will be thrown by default. This behviour can be customized. By calling `global::isInstanceDefined<A>()`a caller can check if an instance is registered. 

## Circular Dependcies
On most applications global instances depend on each other, eg:
```cpp
struct Logger{

 Logger(){
  m_logLevel = global::instance<Settings>().getLogLevel();
 }
 
 void log(const char* msg);
 
 int m_logLevel;
};


struct Settings{

 Settings(){
   if (readSettingsFile()==success)
     global::instance<Logger>().log("settings file found");
 }
 
 int getLogLevel();
 
};

void main(){
 Logger logger;
 global::InstanceRegistration<Logger> regL(&logger); 
 Settings settings;
 global::InstanceRegistration<Settings> regS(&settings); 
}
```

In this case the `Logger` needs `Settings` and vice versa. Therefore irrespective of the order of construction this will not work. The classical solution would be to perform a 2 phase initialisation on at least one of the 2 classes which leads to
 - code bloat (class invariant cannot be established in the constructor so this must be handled all over the class) and it makes it 
 - errorprone on bigger projects (someone will eventually forget about the second init function or the broken class-invariant).

The proposed solution for this problem is the following:

```cpp
struct Logger{

 Logger() {
     global::onInstanceDefine<Settings>([this](Settings& s){
         m_logLevel = s.getLogLevel();});
 }
 
 void log(const char* msg);
 
 int m_logLevel;
};


struct Settings{

 Settings(){ 
   if (readSettingsFile()==success)
     global::onInstanceDefine<Logger>([this](Logger& l){
           m_logLevel = l.log("settings file found");});
 }
 
 int getLogLevel();
 
};

void main(){
 Logger logger;
 global::InstanceRegistration<Logger> regL(&logger); 
 Settings settings;
 global::InstanceRegistration<Settings> regS(&settings); 
}
```
Here the calls to `Logger::log` and `Settings::getLogLevel` are executed if the respective instance becomes available. So in this concrete example the call to `Logger::log` is done immediately in the constructor of `Settings` since the logger is already globally accessible. The call to `Settings::getLogLevel` is done during the construction of `regS`.

## Testing
