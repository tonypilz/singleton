#include "InstanceTest.h"
#include <src/instance.h>
#include <src/InstanceRegistration.h>

InstanceTest::InstanceTest(QObject *parent) : QObject(parent)
{

}

void InstanceTest::aRegisteredInstanceIsAccessible()
{
    class A{};
    A a;

    global::InstanceRegistration<A> registration;
    registration(&a);

    QVERIFY(global::instance<A>()==&a);
}

void InstanceTest::anUnregisteredInstanceIsNotAccessible()
{
    class A{};
    QVERIFY(global::instance<A>()==nullptr);
}

void InstanceTest::anUnregisteredSubInstanceIsNotAccessible()
{
    class A{};
    A a;

    global::InstanceRegistration<A> registration;
    registration(&a);

    class Sub{};

    const bool def = global::instance<A,Sub>()!=nullptr;
    QCOMPARE(def,true);

}

void InstanceTest::aRegisteredSubInstanceIsAccessible()
{
    class A{};
    A a;

    class Sub{};

    global::InstanceRegistration<A,Sub> registration;
    registration(&a);


    const bool def = global::instance<A,Sub>()!=nullptr;
    QCOMPARE(def,true);
}

void InstanceTest::aDerivedInstanceIsAccessibleWithoutSlicing()
{
    constexpr int val = 77;
    struct A{virtual ~A() = default;};
    struct B : public A{ int x = val; };
    B b;

    global::InstanceRegistration<A> registration(&b);

    auto res = dynamic_cast<B*>(static_cast<A*>(global::instance<A>()));

    QCOMPARE(res->x,val);

}

void InstanceTest::aDerivedSubInstanceIsAccessibleWithoutSlicing()
{
    class Sub{};

    constexpr int val = 77;
    struct A{virtual ~A() = default;};
    struct B : public A{ int x = val; };
    B b;

    global::InstanceRegistration<A,Sub> registration(&b);


    auto res = dynamic_cast<B*>(static_cast<A*>(global::instance<A,Sub>()));

    QCOMPARE(res->x,val);
}

void InstanceTest::gettingNullThrowsWithoutHandler()
{
    class A{};

    try{
        global::instance<A>();
    }
    catch(global::NullptrAccess const&){}
    catch(...){ QFAIL("");}
}

void InstanceTest::gettingNullInvokesInstalledUntypeHandler()
{
    class A{};

    class UntypedTestHandler : public std::exception {};

    global::onNullptrAccess() = [](){ throw UntypedTestHandler();};


    try{
        global::instance<A>();
    }
    catch(UntypedTestHandler const&){}
    catch(...){ QFAIL("");}

    global::onNullptrAccess() = std::function<void()>{}; //cleanup installed handler
}

void InstanceTest::gettingNullInvokesInstalledTypeHandlerBeforeUntyped()
{
    class A{};
    A a;

    class UntypedTestHandler : public std::exception {};
    class TypedTestHandler : public std::exception {};

    auto& hu = global::onNullptrAccess();
    auto& ht = global::instance<A>().onNullPtrAccess;


    hu = [](){ throw UntypedTestHandler();};
    ht = [&a](){ return &a;};

    QCOMPARE(static_cast<A*>(global::instance<A>()),&a);

    hu = std::function<void()>{}; //cleanup installed handler

}



