#include "InstanceTest.h"
#include <src/globalInstances.h>
#include "operatorNew.h"

using namespace global;


InstanceTest::InstanceTest(QObject *parent) : QObject(parent)
{

}

void InstanceTest::aRegisteredInstanceIsAccessible()
{
    class A{};
    A a;

    detail::InstanceRegistration<A> registration;
    registration(&a);

    QVERIFY(instance<A>()==&a);
}

void InstanceTest::anUnregisteredInstanceIsNotAccessible()
{
    class A{};
    QVERIFY(instance<A>()==nullptr);
}



void InstanceTest::aDerivedInstanceIsAccessibleWithoutSlicing()
{
    constexpr int val = 77;
    struct A{virtual ~A() = default;};
    struct B : public A{ int x = val; };
    B b;

    detail::InstanceRegistration<A> registration(&b);

    auto res = static_cast<B*>(static_cast<A*>(instance<A>()));

    QCOMPARE(res->x,val);
}

namespace {
    int customOnNullPtrAccessCount = 0;

}

namespace global {
template<> void onNullPtrAccess<>(){
    ++customOnNullPtrAccessCount;
    global::onNullPtrAccess<int>();

}
}
void InstanceTest::gettingNullInvokesCustomHandler()
{
#ifdef __cpp_exceptions
    class A{};

    try{
        customOnNullPtrAccessCount = 0;
        instance<A>();
    }
    catch(NullptrAccess const&){
        QCOMPARE(1,customOnNullPtrAccessCount);
    }
    catch(...){
        QFAIL("");
    }
#else
    QSKIP("skipped due to disabled exceptions", SkipAll);
#endif
}

void InstanceTest::functionWillBeCalledDirectlyIfInstanceDefined()
{
    class A{};

    A a;
    detail::InstanceRegistration<A> registration(&a);

    bool called = false;
    instance<A>().ifAvailable([&](A&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,true);
}


void InstanceTest::unavailableFunctionWillNotBeCalledDirectlyIfInstanceUndefined()
{
    class A{};

    A a;

    bool called = false;
    instance<A>().becomesUnavailable([&called,&a](A&){ called = true;  });

    QCOMPARE(called,false);
}


void InstanceTest::functionWillBeCalledIfInstanceIsDefined()
{
    class A{};

    A a;

    bool called = false;
    instance<A>().ifAvailable([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });
    QCOMPARE(called,false);

    detail::InstanceRegistration<A> registration(&a);

    QCOMPARE(called,true);
}



void InstanceTest::undefinedFunctionWillBeCalledIfInstanceGetsUndefined()
{

    class A{};

    A a;

    bool called = false;
    {
        instance<A>().becomesUnavailable([&called,&a](A&){ called = true; });
        QCOMPARE(called,false);
        detail::InstanceRegistration<A> registration(&a);
        QCOMPARE(called,false);
    }

    QCOMPARE(called,true);

}



void InstanceTest::ifAvailable_willBeCalledDirectlyOnce()
{
    class A{};

    A a;

    detail::InstanceRegistration<A> registration(&a);

    int callCount = 0;
    instance<A>().ifAvailable([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    QCOMPARE(callCount,1);
    {
        A b;

        detail::ReplacingInstanceRegistration<A> registration1(&b);

        QCOMPARE(callCount,1);
    }
    QCOMPARE(callCount,1);
}

void InstanceTest::ifAvailable_willBeCalledIndirectlyOnce()
{
    class A{};

    A a;

    int callCount = 0;
    instance<A>().ifAvailable([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    QCOMPARE(callCount,0);

    detail::InstanceRegistration<A> registration(&a);

    QCOMPARE(callCount,1);

    {
        A b;
        detail::ReplacingInstanceRegistration<A> registration1(&b);

        QCOMPARE(callCount,1);
    }
    QCOMPARE(callCount,1);
}

void InstanceTest::becomesUnavailable_willNotBeCalledDirectlyOnce()
{
    class A{};

    A a;

    detail::InstanceRegistration<A> registration(&a);

    int callCount = 0;
    instance<A>().becomesUnavailable([&](A&r){ ++callCount; QCOMPARE(&r,&a); });
}

void InstanceTest::becomesUnavailable_willBeCalledIndirectlyOnce()
{
    class A{};

    A a;

    int callCount = 0;
    instance<A>().becomesUnavailable([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    QCOMPARE(callCount,0);

    detail::InstanceRegistration<A> registration(&a);

    QCOMPARE(callCount,0);

    {
        A b;
        detail::ReplacingInstanceRegistration<A> registration1(&b);

        QCOMPARE(callCount,0);
    }
    QCOMPARE(callCount,0);
}


void InstanceTest::recursiveQueuingWorks()
{
    class A{};

    A a;

    int callCount1 = 0;
    int callCount2 = 0;


    auto func2 = [&](A&){ ++callCount2; };
    auto func1 = [&](A&){ ++callCount1; instance<A>().ifAvailable(func2); };

    instance<A>().ifAvailable(func1);

    QCOMPARE(callCount1,0);
    QCOMPARE(callCount2,0);

    detail::InstanceRegistration<A> registration(&a);

    QCOMPARE(callCount1,1);
    QCOMPARE(callCount2,1);

    {
        A b;
        detail::ReplacingInstanceRegistration<A> registration1(&b);

        QCOMPARE(callCount1,1);
        QCOMPARE(callCount2,1);
    }
    QCOMPARE(callCount1,1);
    QCOMPARE(callCount2,1);

}

void InstanceTest::registerForDestructionWorks()
{
    class A{};

    A a;

    int funcCallCount = 0;

    instance<A>().ifAvailable(
                [&](A&){ instance<A>().becomesUnavailable(
                    [&](A&){ ++funcCallCount;});
                       });

    QCOMPARE(funcCallCount,0);

    {
        detail::InstanceRegistration<A> registration(&a);
        QCOMPARE(funcCallCount,0);
    }

    QCOMPARE(funcCallCount,1);
}

void InstanceTest::instanceRefWorks()
{
    struct A { int x; };

    using Map = std::map<std::string,A>;
    global::Instance<Map> a { Map{
            {"hans", A{1}},
            {"wurst",A{2}}}};

    {
        const auto val = global::instanceRef<Map>()["hans"].x;
        QCOMPARE(val,1);
    }

    {
        const auto val = (*global::instance<Map>())["hans"].x;
        QCOMPARE(val,1);
    }


}


void InstanceTest::operatorNewNotUsedOnFinishedFunctions()
{
    struct A{};

    const int newCountBefore = newCallCount();

    auto noop = [](A&){ };

    A a;
    detail::InstanceRegistration<A> registration(&a);
    instance<A>().ifAvailable(noop);

    QCOMPARE(newCountBefore,newCallCount());

    instance<A>().becomesUnavailable(noop);

    QVERIFY(newCallCount()>=(newCountBefore+1));
}

void InstanceTest::registeredInstanceAccessDoesNotInvokeOperatorNew()
{
    struct A{void foo(){}};
    global::Instance<A> a;

    const int newCountBefore = newCallCount();

    instance<A>()->foo();

    QCOMPARE(newCountBefore,newCallCount());

}

void InstanceTest::unregisteredInstanceAccessDoesNotInvokeOperatorNew()
{

#ifdef __cpp_exceptions
    const int newCountBefore = newCallCount();

    try{
        struct A{void foo(){}};
        instance<A>()->foo();
    }
    catch(...){

    }

    QCOMPARE(newCountBefore,newCallCount());

#else
    QSKIP("skipped due to disabled exceptions", SkipAll);
#endif

}
