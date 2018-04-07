#include "InstanceTest.h"
#include <src/globalInstances.h>
#include <src/globalInstances.h>

using namespace global;

constexpr auto condition_not_met = DeferredOperationState::pending;
constexpr auto operation_executed = DeferredOperationState::finished;

InstanceTest::InstanceTest(QObject *parent) : QObject(parent)
{

}

void InstanceTest::aRegisteredInstanceIsAccessible()
{
    A a;

    detail::InstanceRegistration<A> registration;
    registration(&a);

    QVERIFY(instance<A>()==&a);
}

void InstanceTest::anUnregisteredInstanceIsNotAccessible()
{
    QVERIFY(instance<A>()==nullptr);
}

void InstanceTest::anUnregisteredSubInstanceIsNotAccessible()
{
    A a;

    detail::InstanceRegistration<A> registration;
    registration(&a);

    const bool def = instance<A,Sub>()!=nullptr;
    QCOMPARE(def,true);
}

void InstanceTest::aRegisteredSubInstanceIsAccessible()
{
    A a;
    detail::InstanceRegistration<A,Sub> registration;
    registration(&a);

    const bool def = instance<A,Sub>()!=nullptr;
    QCOMPARE(def,true);
}

void InstanceTest::aDerivedInstanceIsAccessibleWithoutSlicing()
{
    constexpr int val = 77;
    struct A{virtual ~A() = default;};
    struct B : public A{ int x = val; };
    B b;

    detail::InstanceRegistration<A> registration(&b);

    auto res = dynamic_cast<B*>(static_cast<A*>(instance<A>()));

    QCOMPARE(res->x,val);
}

void InstanceTest::aDerivedSubInstanceIsAccessibleWithoutSlicing()
{
    constexpr int val = 77;

    struct A{virtual ~A() = default;};
    struct B : public A{ int x = val; };

    B b;

    detail::InstanceRegistration<A,Sub> registration(&b);

    auto res = dynamic_cast<B*>(static_cast<A*>(instance<A,Sub>()));

    QCOMPARE(res->x,val);
}

void InstanceTest::gettingNullThrowsWithoutHandler()
{
    try{
        instance<A>();
    }
    catch(NullptrAccess const&){}
    catch(...){ QFAIL("");}
}

void InstanceTest::gettingNullInvokesInstalledUntypeHandler()
{
    class UntypedTestHandler : public std::exception {};

    onNullptrAccess() = [](){ throw UntypedTestHandler();};

    try{
        instance<A>();
    }
    catch(UntypedTestHandler const&){}
    catch(...){ QFAIL("");}

    onNullptrAccess() = std::function<void()>{}; //cleanup installed handler
}

void InstanceTest::gettingNullInvokesInstalledTypeHandlerBeforeUntyped()
{
    A a;

    class UntypedTestHandler : public std::exception {};
    class TypedTestHandler : public std::exception {};

    auto& hu = onNullptrAccess();
    auto& ht = instance<A>().onNullPtrAccess;


    hu = [](){ throw UntypedTestHandler();};
    ht = [&a](){ return &a;};

    QCOMPARE(static_cast<A*>(instance<A>()),&a);

    hu = std::function<void()>{}; //cleanup installed handler
}



void InstanceTest::functionWillBeCalledDirectlyIfInstanceDefined()
{
    A a;
    detail::InstanceRegistration<A> registration(&a);

    bool called = false;
    instance<A>().ifAvailable([&called,&a](A const&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,true);
}

void InstanceTest::functionWillBeCalledDirectlyIfSubInstanceDefined()
{
    A a;
    detail::InstanceRegistration<A,Sub> registration(&a);

    bool called = false;
    instance<A,Sub>().ifAvailable([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,true);
}

void InstanceTest::functionWillBeCalledDirectlyIfInstanceUndefined()
{
    A a;

    bool called = false;
    instance<A>().ifUnavailable([&called,&a](){ called = true;  });

    QCOMPARE(called,true);
}

void InstanceTest::functionWillBeCalledDirectlyIfSubInstanceUndefined()
{
    A a;

    bool called = false;
    instance<A,Sub>().ifUnavailable([&called,&a](){ called = true;  });

    QCOMPARE(called,true);
}

void InstanceTest::functionWillBeCalledIfInstanceIsDefined()
{
    A a;

    bool called = false;
    instance<A>().ifAvailable([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });
    QCOMPARE(called,false);

    detail::InstanceRegistration<A> registration(&a);

    QCOMPARE(called,true);
}

void InstanceTest::functionWillBeCalledIfSubInstanceIsDefined()
{
    A a;

    bool called = false;
    instance<A,Sub>().ifAvailable([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,false);

    detail::InstanceRegistration<A,Sub> registration(&a);

    QCOMPARE(called,true);
}

void InstanceTest::functionWillBeCalledIfInstanceIsUndefined()
{
    A a;

    bool called = false;
    {
        detail::InstanceRegistration<A> registration(&a);
        instance<A>().ifUnavailable([&called,&a](){ called = true; });
        QCOMPARE(called,false);
    }

    QCOMPARE(called,true);
}

void InstanceTest::functionWillBeCalledIfSubInstanceIsUndefined()
{
    A a;

    bool called = false;
    {
        detail::InstanceRegistration<A,Sub> registration(&a);
        instance<A,Sub>().ifUnavailable([&called,&a](){ called = true; });
        QCOMPARE(called,false);
    }

    QCOMPARE(called,true);
}

void InstanceTest::functionWillBeCalledOnlyOnceDirectly()
{
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
    A a;

    detail::InstanceRegistration<A> registration(&a);

    int funcCallCount = 0;

    instance<A>().addDeferredOperation(
                [&](A* const&p){
                    if (p!=&a) return condition_not_met;
                    ++funcCallCount;
                    return operation_executed;});

    QCOMPARE(funcCallCount,1);

}

void InstanceTest::conditionalFunctionWillBeCalledIfInstanceDefined()
{
    A a;

    int funcCallCount = 0;

    instance<A>().addDeferredOperation(
                [&](A* const&p){
                    if (p!=&a) return condition_not_met;
                    ++funcCallCount;
                    return operation_executed;});

    detail::InstanceRegistration<A> registration(&a);

    QCOMPARE(funcCallCount,1);
}

void InstanceTest::conditionalFunctionWillBeCalledDirectlyIfSubInstanceDefined()
{
    A a;

    detail::InstanceRegistration<A,Sub> registration(&a);

    int funcCallCount = 0;


    instance<A,Sub>().addDeferredOperation(
                [&](A* const&p){
                    if (p!=&a) return condition_not_met;
                    ++funcCallCount;
                    return operation_executed;});

    QCOMPARE(funcCallCount,1);
}

void InstanceTest::conditionalFunctionWillBeCalledIfSubInstanceDefined()
{
    A a;

    int funcCallCount = 0;

    instance<A,Sub>().addDeferredOperation(
                [&](A* const&p){
                    if (p!=&a) return condition_not_met;
                    ++funcCallCount;
                    return operation_executed;});

    detail::InstanceRegistration<A,Sub> registration(&a);

    QCOMPARE(funcCallCount,1);
}

void InstanceTest::functionsWithDifferentConditionsWillBeCalledOnSubInstanceChange()
{
    A a;

    int funcCallCount1 = 0;
    int funcCallCount2 = 0;
    bool enabled = false;

    auto func1 = [&](A* const&p){
                        if (!enabled) return condition_not_met;
                        if (p!=&a) return condition_not_met;
                        ++funcCallCount1;
                        return operation_executed;};
    auto func2 = [&](A* const&p){
                        if (!enabled) return condition_not_met;
                        if (p!=nullptr) return condition_not_met;
                        ++funcCallCount2;
                        return operation_executed;};
    const int n = 20;
    for(int i = 0;i<n;++i){
        instance<A,Sub>().addDeferredOperation(func1);
        instance<A,Sub>().addDeferredOperation(func2);
    }

    QCOMPARE(funcCallCount1,0);
    QCOMPARE(funcCallCount2,0);

    enabled = true;

    {

        detail::InstanceRegistration<A,Sub> registration(&a);

        QCOMPARE(funcCallCount1,n);
        QCOMPARE(funcCallCount2,0);

    }

    QCOMPARE(funcCallCount1,n);
    QCOMPARE(funcCallCount2,n);

}

void InstanceTest::recursiveQueuingWorks()
{
    A a;

    int funcCallCount1 = 0;
    int funcCallCount2 = 0;

    auto func2 = [&](A* const&p){
                    if (p!=nullptr) return condition_not_met;
                    ++funcCallCount2;
                    return operation_executed;};

    auto func1 = [&](A* const&p){
                    if (p!=&a) return condition_not_met;
                    instance<A,Sub>().addDeferredOperation(func2);
                    ++funcCallCount1;
                    return operation_executed;};

    instance<A,Sub>().addDeferredOperation(func1);


    QCOMPARE(funcCallCount1,0);
    QCOMPARE(funcCallCount2,0);

    {
        detail::InstanceRegistration<A,Sub> registration(&a);

        QCOMPARE(funcCallCount1,1);
        QCOMPARE(funcCallCount2,0);
    }

    QCOMPARE(funcCallCount1,1);
    QCOMPARE(funcCallCount2,1);

    {
        detail::InstanceRegistration<A,Sub> registration(&a);

        QCOMPARE(funcCallCount1,1);
        QCOMPARE(funcCallCount2,1);

    }
}

void InstanceTest::registerForDestructionWorks()
{
    A a;

    int funcCallCount = 0;

    instance<A,Sub>().ifAvailable(
                [&](A&){ instance<A,Sub>().ifUnavailable(
                    [&](){ ++funcCallCount;});
                       });

    QCOMPARE(funcCallCount,0);

    {
        detail::InstanceRegistration<A,Sub> registration(&a);
        QCOMPARE(funcCallCount,0);
    }

    QCOMPARE(funcCallCount,1);
}

void InstanceTest::instanceRefWorks()
{

    struct Sub{};

    struct A { int x; };

    using Map = std::map<std::string,A>;
    global::SubInstance<Map,Sub> a { Map{
            {"hans", A{1}},
            {"wurst",A{2}}}};



    {
        const auto val = global::instanceRef<Map,Sub>()["hans"].x;
        QCOMPARE(val,1);
    }


}

