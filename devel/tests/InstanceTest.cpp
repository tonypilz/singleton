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
    global::isInstanceDefined<A>();
    QCOMPARE(global::isInstanceDefined<A>(),true);
    QCOMPARE(global::instancePtr<A>(),&a);

}

void InstanceTest::anUnregisteredInstanceIsNotAccessible()
{
    class A{};
    QCOMPARE(global::isInstanceDefined<A>(),false);
}

void InstanceTest::anUnregisteredSubInstanceIsNotAccessible()
{
    class A{};
    A a;

    global::InstanceRegistration<A> registration;
    registration(&a);

    class Sub{};

    const bool def = global::isInstanceDefined<A,Sub>();
    QCOMPARE(def,false);

}

void InstanceTest::aRegisteredSubInstanceIsAccessible()
{
    class A{};
    A a;

    class Sub{};

    global::InstanceRegistration<A,Sub> registration;
    registration(&a);


    const bool def = global::isInstanceDefined<A,Sub>();
    QCOMPARE(def,true);
}

void InstanceTest::aDerivedInstanceIsAccessibleWithoutSlicing()
{
    constexpr int val = 77;
    struct A{virtual ~A() = default;};
    struct B : public A{ int x = val; };
    B b;

    global::InstanceRegistration<A> registration(&b);


    auto res = dynamic_cast<B&>(global::instance<A>());

    QCOMPARE(res.x,val);

}

void InstanceTest::aDerivedSubInstanceIsAccessibleWithoutSlicing()
{
    class Sub{};

    constexpr int val = 77;
    struct A{virtual ~A() = default;};
    struct B : public A{ int x = val; };
    B b;

    global::InstanceRegistration<A,Sub> registration(&b);


    auto res = dynamic_cast<B&>(global::instance<A,Sub>());

    QCOMPARE(res.x,val);
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
    auto& ht = global::onNullptrAccess<A>();


    hu = [](){ throw UntypedTestHandler();};
    ht = [&a]()->A*{ return &a;};

    QCOMPARE(&global::instance<A>(),&a);

    hu = std::function<void()>{}; //cleanup installed handler
    ht = std::function<A*()>{}; //cleanup installed handler

}


void InstanceTest::noDoubleNotifications()
{
    class A{};
    A a;

    global::InstanceRegistration<A> registration(&a);


    auto& value = global::detail::initializedInstance<A,global::detail::staticValueSubDefault>();

    int calls = 0;
    value.valueChanged = [&calls](A* const&){ ++calls; };


    QCOMPARE(calls,0);
    { global::ReplacingInstanceRegistration<A> registration(&a);}
    QCOMPARE(calls,0);


}


void InstanceTest::instanceChangedHandlersTriggered()
{
    class A{};
    A a;

    auto& value = global::detail::initializedInstance<A,global::detail::staticValueSubDefault>();

    int callst = 0;

    A* expect = nullptr;

    value.valueChanged = [&callst,&expect](A* t){ ++callst; QCOMPARE(t,expect); };

    {

        global::InstanceRegistration<A> registration;

        expect = &a;

        registration(&a);

        QCOMPARE(callst,1);


        {
            A b;
            expect = &b;

            global::ReplacingInstanceRegistration<A> registration;

            registration(&b);

            QCOMPARE(callst,2);

            expect = &a;

        }

        QCOMPARE(callst,3);

        expect = nullptr;
    }

    QCOMPARE(callst,4);

}


