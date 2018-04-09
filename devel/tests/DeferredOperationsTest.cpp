#include "DeferredOperationsTest.h"

#include <src/globalInstances.h>
#include "operatorNew.h"

using global::detail::DeferredOperations;
constexpr auto pending = global::DeferredOperationState::pending;
constexpr auto finished = global::DeferredOperationState::finished;

DeferredOperationsTest::DeferredOperationsTest(QObject *parent) : QObject(parent)
{

}

void DeferredOperationsTest::argumentIsPassedToEachElement()
{
    DeferredOperations<A> op;

    A a;
    const A* out1 = nullptr;
    const A* out2 = nullptr;
    op.addDeferredOperation([&](A const*e){ out1 = e; return finished; },&a);
    op.addDeferredOperation([&](A const*e){ out2 = e; return finished; },&a);
    QCOMPARE(out1,&a);
    QCOMPARE(out2,&a);
}

void DeferredOperationsTest::falseConditionsAreRexecuted()
{

    DeferredOperations<A> op;

    A a;
    int count = 0;

    op.addDeferredOperation([&](A const*){++count; return pending;},&a);
    QCOMPARE(count,1);

    op.conditionsChanged(nullptr,&a);
    QCOMPARE(count,2);

}

void DeferredOperationsTest::trueConditionsAreExecutedOnce()
{
    DeferredOperations<A> op;

    A a;
    int count = 0;

    op.addDeferredOperation([&](A const*){++count; return finished;},&a);

    QCOMPARE(count,1);

    op.conditionsChanged(nullptr,&a);
    QCOMPARE(count,1);
}

void DeferredOperationsTest::recursiveExecutionWorks()
{
    DeferredOperations<A> op;

    A a1, a2;
    const A* out1 = nullptr;
    const A* out2 = nullptr;
    const A* null = nullptr;

    op.addDeferredOperation([&](A *e1){

        if (e1==nullptr) return pending;

        out1 = e1;
        op.addDeferredOperation([&](A const*e2) {

            if (e2==nullptr) return pending;

            out2 = e2;
            return finished;
        },nullptr);
        return finished;
    },nullptr);

    op.conditionsChanged(nullptr,&a1);

    QCOMPARE(out1,&a1);
    QCOMPARE(out2,null);

    out1 = nullptr;
    out2 = nullptr;

    op.conditionsChanged(nullptr,&a2);
    QCOMPARE(out1,null);
    QCOMPARE(out2,&a2);


}

void DeferredOperationsTest::recursiveExecutionWorks1()
{
    DeferredOperations<A> op;

    A a1, a2;
    const A* out1 = nullptr;
    const A* out2 = nullptr;
    const A* null = nullptr;

    op.addDeferredOperation([&](A*e1){
        out1 = e1;
        op.addDeferredOperation([&](A const*e2) {
            out2 = e2;
            return finished;
        },e1);
        return finished;
    },&a1);

    QCOMPARE(out1,&a1);
    QCOMPARE(out2,&a1);

    out1 = nullptr;
    out2 = nullptr;

    op.conditionsChanged(&a1,&a2);
    QCOMPARE(out1,null);
    QCOMPARE(out2,null);
}

void DeferredOperationsTest::instanceBeforeIsPassedCorrectly()
{
    DeferredOperations<A> op;

    A a,b;

    const A* out1 = nullptr;
    const A* out2 = nullptr;

    op.addDeferredOperationWithArgBefore([&](A const*before,A const*current){ out1 = before; out2 = current; return pending; },nullptr);

    op.conditionsChanged(&a,&b);

    QCOMPARE(out1,&a);
    QCOMPARE(out2,&b);
}

void DeferredOperationsTest::operatorNewNotUsedOnFinishedFunctions()
{
    const int newCountBefore = newCallCount();

    DeferredOperations<A> op;

    A a;

    op.addDeferredOperationWithArgBefore([&](A const*,A const*){ return finished; },nullptr);
    op.addDeferredOperation([&](A const*){ return finished; },nullptr);
    op.ifAvailable([&](A const&){ },&a);

    QCOMPARE(newCountBefore,newCallCount());

    op.addDeferredOperationWithArgBefore([&](A const*,A const*){ return pending; },nullptr);
    op.addDeferredOperation([&](A const*){ return pending; },nullptr);
    op.ifAvailable([&](A const&){ },nullptr);
    op.becomesUnavailable([&](A&){ },&a);

    QVERIFY(newCallCount()>=(newCountBefore+4));

}




