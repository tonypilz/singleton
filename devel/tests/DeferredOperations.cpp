#include "DeferredOperations.h"

#include <src/globalInstances.h>

using global::detail::DeferredOperations;

DeferredOperationsTest::DeferredOperationsTest(QObject *parent) : QObject(parent)
{

}

void DeferredOperationsTest::argumentIsPassedToEachElement()
{
    DeferredOperations<A> op;

    auto trueCond = [](A const&){return true;};
    A a;
    const A* out1 = nullptr;
    const A* out2 = nullptr;
    op.addDeferredOperation(trueCond,[&](A const&e){ out1 = &e; });
    op.addDeferredOperation(trueCond,[&](A const&e){ out2 = &e; });
    op.conditionsChanged(a);
    QCOMPARE(out1,&a);
    QCOMPARE(out2,&a);
}

void DeferredOperationsTest::falseConditionsAreRexecuted()
{

    DeferredOperations<A> op;

    A a;
    int count = 0;

    op.addDeferredOperation([&](A const&){++count; return false;},[&](A const&){++count;});

    op.conditionsChanged(a);
    QCOMPARE(count,1);

    op.conditionsChanged(a);
    QCOMPARE(count,2);

}

void DeferredOperationsTest::trueConditionsAreExecutedOnce()
{
    DeferredOperations<A> op;

    auto trueCond = [](A const&){return true;};
    A a;
    int count = 0;

    op.addDeferredOperation(trueCond,[&](A const&){++count; });

    op.conditionsChanged(a);
    QCOMPARE(count,1);

    op.conditionsChanged(a);
    QCOMPARE(count,1);
}

void DeferredOperationsTest::recursiveExecutionWorks()
{
    DeferredOperations<A> op;

    auto trueCond = [](A const&){return true;};

    A a1, a2;
    const A* out1 = nullptr;
    const A* out2 = nullptr;

    op.addDeferredOperation(trueCond,[&](A const&e1){
        out1 = &e1;
        op.addDeferredOperation(trueCond,[&](A const&e2) {
            out2 = &e2;
        });
    });

    op.conditionsChanged(a1);
    QCOMPARE(out1,&a1);
    QCOMPARE(out2,static_cast<A*>(nullptr));

    out1 = nullptr;
    out2 = nullptr;

    op.conditionsChanged(a2);
    QCOMPARE(out1,static_cast<A*>(nullptr));
    QCOMPARE(out2,&a2);


}

void DeferredOperationsTest::recursiveExecutionWorks1()
{
    DeferredOperations<A> op;
    auto trueCond = [](A const&){return true;};

    A a1, a2;
    const A* out1 = nullptr;
    const A* out2 = nullptr;

    op.addDeferredOperation(trueCond,[&](A const&e1){
        out1 = &e1;
        op.addDeferredOperation(trueCond,[&](A const&e2) {
            out2 = &e2;
        });
    });

    op.conditionsChanged(a1);
    QCOMPARE(out1,&a1);
    QCOMPARE(out2,static_cast<A*>(nullptr));

    out1 = nullptr;
    out2 = nullptr;

    op.conditionsChanged(a2);
    QCOMPARE(out1,static_cast<A*>(nullptr));
    QCOMPARE(out2,&a2);
}




