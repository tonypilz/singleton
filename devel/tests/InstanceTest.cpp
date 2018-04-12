#include "InstanceTest.h"
#include <src/globalInstances.h>
#include "operatorNew.h"

using namespace global;

constexpr auto pending = DeferredOperationState::pending;
constexpr auto finished = DeferredOperationState::finished;

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
    instance<A>().ifAvailable([&called,&a](A const&r){ called = true; QCOMPARE(&r,&a); });

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



void InstanceTest::functionWillBeCalledOnlyOnceDirectly()
{
    class A{};

    A a;

    detail::InstanceRegistration<A> registration(&a);

    int callCount = 0;
    instance<A>().ifAvailable([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    QCOMPARE(callCount,1);
    A b;

    detail::ReplacingInstanceRegistration<A> registration1(&b);

    QCOMPARE(callCount,1);
}

void InstanceTest::functionWillBeCalledOnlyOnceIndirectly()
{
    class A{};

    A a;

    int callCount = 0;
    instance<A>().ifAvailable([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    detail::InstanceRegistration<A> registration(&a);

    QCOMPARE(callCount,1);

    A b;
    detail::ReplacingInstanceRegistration<A> registration1(&b);

    QCOMPARE(callCount,1);
}

void InstanceTest::conditionalFunctionWillBeCalledDirectlyIfInstanceDefined()
{
    class A{};

    A a;

    detail::InstanceRegistration<A> registration(&a);

    int funcCallCount = 0;

    instance<A>().addDeferredOperation(
                [&](A* const&p){
                    if (p!=&a) return pending;
                    ++funcCallCount;
                    return finished;});

    QCOMPARE(funcCallCount,1);

}

void InstanceTest::conditionalFunctionWillBeCalledIfInstanceDefined()
{
    class A{};

    A a;

    int funcCallCount = 0;

    instance<A>().addDeferredOperation(
                [&](A* const&p){
                    if (p!=&a) return pending;
                    ++funcCallCount;
                    return finished;});

    detail::InstanceRegistration<A> registration(&a);

    QCOMPARE(funcCallCount,1);
}



void InstanceTest::functionsWithDifferentConditionsWillBeCalledOnInstanceChange()
{
    class A{};

    A a;

    int funcCallCount1 = 0;
    int funcCallCount2 = 0;
    bool enabled = false;

    auto func1 = [&](A* const&p){
                        if (!enabled) return pending;
                        if (p!=&a) return pending;
                        ++funcCallCount1;
                        return finished;};
    auto func2 = [&](A* const&p){
                        if (!enabled) return pending;
                        if (p!=nullptr) return pending;
                        ++funcCallCount2;
                        return finished;};
    const int n = 20;
    for(int i = 0;i<n;++i){
        instance<A>().addDeferredOperation(func1);
        instance<A>().addDeferredOperation(func2);
    }

    QCOMPARE(funcCallCount1,0);
    QCOMPARE(funcCallCount2,0);

    enabled = true;

    {

        detail::InstanceRegistration<A> registration(&a);

        QCOMPARE(funcCallCount1,n);
        QCOMPARE(funcCallCount2,0);

    }

    QCOMPARE(funcCallCount1,n);
    QCOMPARE(funcCallCount2,n);

}

void InstanceTest::recursiveQueuingWorks()
{
    class A{};

    A a;

    int funcCallCount1 = 0;
    int funcCallCount2 = 0;

    auto func2 = [&](A* const&p){
                    if (p!=nullptr) return pending;
                    ++funcCallCount2;
                    return finished;};

    auto func1 = [&](A* const&p){
                    if (p!=&a) return pending;
                    instance<A>().addDeferredOperation(func2);
                    ++funcCallCount1;
                    return finished;};

    instance<A>().addDeferredOperation(func1);


    QCOMPARE(funcCallCount1,0);
    QCOMPARE(funcCallCount2,0);

    {
        detail::InstanceRegistration<A> registration(&a);

        QCOMPARE(funcCallCount1,1);
        QCOMPARE(funcCallCount2,0);
    }

    QCOMPARE(funcCallCount1,1);
    QCOMPARE(funcCallCount2,1);

    {
        detail::InstanceRegistration<A> registration(&a);

        QCOMPARE(funcCallCount1,1);
        QCOMPARE(funcCallCount2,1);

    }
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
void InstanceTest::instanceBeforeIsAvailableToDefferedOperations()
{
    class A{};

    A a,b;

    A* before = nullptr;
    A* current = nullptr;

    constexpr A* null = nullptr;

    instance<A>().addDeferredOperationWithArgBefore(
                [&](A* b, A* c){
                    before = b;
                    current = c;
                    return pending;
    });

    QCOMPARE(before,null);
    QCOMPARE(current,null);

    {
        detail::InstanceRegistration<A> registration(&a);

        QCOMPARE(before,null);
        QCOMPARE(current,&a);

        detail::ReplacingInstanceRegistration<A> registration1(&b);

        QCOMPARE(before,&a);
        QCOMPARE(current,&b);
    }

    QCOMPARE(before,&a);
    QCOMPARE(current,null);


}

void InstanceTest::operatorNewNotUsedOnFinishedFunctions()
{
    struct A{};

    const int newCountBefore = newCallCount();

    instance<A>().addDeferredOperationWithArgBefore([&](A const*,A const*){ return finished; });
    instance<A>().addDeferredOperation([&](A const*){ return finished; });

    QCOMPARE(newCountBefore,newCallCount());

    instance<A>().addDeferredOperationWithArgBefore([&](A const*,A const*){ return pending; });
    instance<A>().addDeferredOperation([&](A const*){ return pending; });
    instance<A>().ifAvailable([&](A const&){ });

    QVERIFY(newCallCount()>=(newCountBefore+3));
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
