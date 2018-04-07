#include "DeferredOperationsTest.h"

#include <src/globalInstances.h>

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
    op.addDeferredOperation([&](A const*e){ out1 = e; return finished; });
    op.addDeferredOperation([&](A const*e){ out2 = e; return finished; });
    op.conditionsChanged(nullptr,&a);
    QCOMPARE(out1,&a);
    QCOMPARE(out2,&a);
}

void DeferredOperationsTest::falseConditionsAreRexecuted()
{

    DeferredOperations<A> op;

    A a;
    int count = 0;

    op.addDeferredOperation([&](A const*){++count; return pending;});

    op.conditionsChanged(nullptr,&a);
    QCOMPARE(count,1);

    op.conditionsChanged(nullptr,&a);
    QCOMPARE(count,2);

}

void DeferredOperationsTest::trueConditionsAreExecutedOnce()
{
    DeferredOperations<A> op;

    A a;
    int count = 0;

    op.addDeferredOperation([&](A const*){++count; return finished;});

    op.conditionsChanged(nullptr,&a);
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

    op.addDeferredOperation([&](A const*e1){
        out1 = e1;
        op.addDeferredOperation([&](A const*e2) {
            out2 = e2;
            return finished;
        });
        return finished;
    });

    op.conditionsChanged(nullptr,&a1);
    QCOMPARE(out1,&a1);
    QCOMPARE(out2,static_cast<A*>(nullptr));

    out1 = nullptr;
    out2 = nullptr;

    op.conditionsChanged(nullptr,&a2);
    QCOMPARE(out1,static_cast<A*>(nullptr));
    QCOMPARE(out2,&a2);


}

void DeferredOperationsTest::recursiveExecutionWorks1()
{
    DeferredOperations<A> op;

    A a1, a2;
    const A* out1 = nullptr;
    const A* out2 = nullptr;

    op.addDeferredOperation([&](A const*e1){
        out1 = e1;
        op.addDeferredOperation([&](A const*e2) {
            out2 = e2;
            return finished;
        });
        return finished;
    });

    op.conditionsChanged(nullptr,&a1);
    QCOMPARE(out1,&a1);
    QCOMPARE(out2,static_cast<A*>(nullptr));

    out1 = nullptr;
    out2 = nullptr;

    op.conditionsChanged(nullptr,&a2);
    QCOMPARE(out1,static_cast<A*>(nullptr));
    QCOMPARE(out2,&a2);
}

void DeferredOperationsTest::instanceBeforeIsPassedCorrectly()
{
    DeferredOperations<A> op;

    A a,b;

    const A* out1 = nullptr;
    const A* out2 = nullptr;

    op.addDeferredOperationWithArgBefore([&](A const*before,A const*current){ out1 = before; out2 = current; return pending; });

    op.conditionsChanged(&a,&b);

    QCOMPARE(out1,&a);
    QCOMPARE(out2,&b);
}




