#include "ConditionalSingleShotOperationsTest.h"

#include <src/globalInstances.h>

using global::detail::ConditionalSingleShotOperations;

ConditionalSingleShotOperationsTest::ConditionalSingleShotOperationsTest(QObject *parent) : QObject(parent)
{

}

void ConditionalSingleShotOperationsTest::argumentIsPassedToEachElement()
{
    ConditionalSingleShotOperations<A> op;

    A a;
    const A* out1 = nullptr;
    const A* out2 = nullptr;
    op.add([&](A const&e){ out1 = &e; return true; });
    op.add([&](A const&e){ out2 = &e; return true; });
    op(a);
    QCOMPARE(out1,&a);
    QCOMPARE(out2,&a);
}

void ConditionalSingleShotOperationsTest::falseConditionsAreRexecuted()
{

    ConditionalSingleShotOperations<A> op;

    A a;
    int count = 0;

    op.add([&](A const&){++count; return false; });

    op(a);
    QCOMPARE(count,1);

    op(a);
    QCOMPARE(count,2);

}

void ConditionalSingleShotOperationsTest::trueConditionsAreExecutedOnce()
{
    ConditionalSingleShotOperations<A> op;

    A a;
    int count = 0;

    op.add([&](A const&){++count; return true; });

    op(a);
    QCOMPARE(count,1);

    op(a);
    QCOMPARE(count,1);
}

void ConditionalSingleShotOperationsTest::recursiveExecutionWorks()
{
    ConditionalSingleShotOperations<A> op;

    A a1, a2;
    const A* out1 = nullptr;
    const A* out2 = nullptr;

    op.add([&](A const&e1){
        out1 = &e1;
        op.add([&](A const&e2) {
            out2 = &e2;
            return false;
        });
        return false;
    });

    op(a1);
    QCOMPARE(out1,&a1);
    QCOMPARE(out2,static_cast<A*>(nullptr));

    out1 = nullptr;
    out2 = nullptr;

    op(a2);
    QCOMPARE(out1,&a2);
    QCOMPARE(out2,&a2);


}

void ConditionalSingleShotOperationsTest::recursiveExecutionWorks1()
{
    ConditionalSingleShotOperations<A> op;

    A a1, a2;
    const A* out1 = nullptr;
    const A* out2 = nullptr;

    op.add([&](A const&e1){
        out1 = &e1;
        op.add([&](A const&e2) {
            out2 = &e2;
            return true;
        });
        return true;
    });

    op(a1);
    QCOMPARE(out1,&a1);
    QCOMPARE(out2,static_cast<A*>(nullptr));

    out1 = nullptr;
    out2 = nullptr;

    op(a2);
    QCOMPARE(out1,static_cast<A*>(nullptr));
    QCOMPARE(out2,&a2);
}




