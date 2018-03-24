#include "InstanceOperationsTest.h"
#include <src/instance.h>
#include <src/InstanceRegistration.h>


InstanceOperationsTest::InstanceOperationsTest(QObject *parent) : QObject(parent)
{

}

void InstanceOperationsTest::functionWillBeCalledDirectlyIfInstanceDefined()
{
    class A{};
    A a;

    global::InstanceRegistration<A> registration(&a);

    bool called = false;
    global::instance<A>().visitIfNotNull([&called,&a](A const&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,true);

}

void InstanceOperationsTest::functionWillBeCalledDirectlyIfSubInstanceDefined()
{
    class A{};
    A a;

    struct MySub{};

    global::InstanceRegistration<A,MySub> registration(&a);

    bool called = false;
    global::instance<A,MySub>().visitIfNotNull([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });


    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledDirectlyIfInstanceUndefined()
{
    class A{};
    A a;

    bool called = false;
    global::instance<A>().visitIfNull([&called,&a](){ called = true;  });

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledDirectlyIfSubInstanceUndefined()
{
    class A{};
    A a;

    struct MySub{};

    bool called = false;
    global::instance<A,MySub>().visitIfNull([&called,&a](){ called = true;  });

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledIfInstanceIsDefined()
{
    class A{};
    A a;

    bool called = false;
    global::instance<A>().visitIfNotNull([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });
    QCOMPARE(called,false);

    global::InstanceRegistration<A> registration(&a);

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledIfSubInstanceIsDefined()
{
    class A{};
    A a;

    struct MySub{};


    bool called = false;
    global::instance<A,MySub>().visitIfNotNull([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,false);

    global::InstanceRegistration<A,MySub> registration(&a);

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledIfInstanceIsUndefined()
{
    class A{};
    A a;

    bool called = false;
    {
        global::InstanceRegistration<A> registration(&a);
        global::instance<A>().visitIfNull([&called,&a](){ called = true; });
        QCOMPARE(called,false);
    }

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledIfSubInstanceIsUndefined()
{
    class A{};
    A a;

    struct MySub{};


    bool called = false;
    {
        global::InstanceRegistration<A,MySub> registration(&a);
        global::instance<A,MySub>().visitIfNull([&called,&a](){ called = true; });
        QCOMPARE(called,false);
    }

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledOnlyOnceDirectly()
{
    class A{};
    A a;

    global::InstanceRegistration<A> registration(&a);


    int callCount = 0;
    global::instance<A>().visitIfNotNull([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    QCOMPARE(callCount,1);
    A b;

    global::ReplacingInstanceRegistration<A> registration1(&b);

    QCOMPARE(callCount,1);

}

void InstanceOperationsTest::functionWillBeCalledOnlyOnceIndirectly()
{
    class A{};
    A a;


    int callCount = 0;
    global::instance<A>().visitIfNotNull([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    global::InstanceRegistration<A> registration(&a);

    QCOMPARE(callCount,1);

    A b;
    global::ReplacingInstanceRegistration<A> registration1(&b);

    QCOMPARE(callCount,1);
}

void InstanceOperationsTest::conditionalFunctionWillBeCalledDirectlyIfInstanceDefined()
{
    class A{};
    A a;

    global::InstanceRegistration<A> registration(&a);

    int funcCallCount = 0;
    int condCallCount = 0;

    global::instance<A>().visitIf(
                [&](A* const&p){ ++condCallCount; return p==&a;},
                [&](A* const&r){ QCOMPARE(r,&a); ++funcCallCount; });

    QCOMPARE(funcCallCount,1);
    QVERIFY(condCallCount>0);
}

void InstanceOperationsTest::conditionalFunctionWillBeCalledIfInstanceDefined()
{
    class A{};
    A a;

    int funcCallCount = 0;
    int condCallCount = 0;

    global::instance<A>().visitIf(
                [&](A*p){ ++condCallCount; return p==&a;},
                [&](A*r){ QCOMPARE(r,&a); ++funcCallCount; });

    global::InstanceRegistration<A> registration(&a);

    QCOMPARE(funcCallCount,1);
    QVERIFY(condCallCount>0);
}

void InstanceOperationsTest::conditionalFunctionWillBeCalledDirectlyIfSubInstanceDefined()
{
    class A{};
    A a;

    struct MySub{};

    global::InstanceRegistration<A,MySub> registration(&a);

    int funcCallCount = 0;
    int condCallCount = 0;

    global::instance<A,MySub>().visitIf(
                [&](A*p){ ++condCallCount; return p==&a;},
                [&](A*r){ QCOMPARE(r,&a); ++funcCallCount; });

    QCOMPARE(funcCallCount,1);
    QVERIFY(condCallCount>0);
}

void InstanceOperationsTest::conditionalFunctionWillBeCalledIfSubInstanceDefined()
{
    class A{};
    A a;

    struct MySub{};

    int funcCallCount = 0;
    int condCallCount = 0;

    global::instance<A,MySub>().visitIf(
                [&](A*p){ ++condCallCount; return p==&a;},
                [&](A*r){ QCOMPARE(r,&a); ++funcCallCount; });

    global::InstanceRegistration<A,MySub> registration(&a);

    QCOMPARE(funcCallCount,1);
    QVERIFY(condCallCount>0);
}

void InstanceOperationsTest::functionsWithDifferentConditionsWillBeCalledOnSubInstanceChange()
{
    class A{};
    A a;

    struct MySub{};

    int funcCallCount1 = 0;
    int condCallCount1 = 0;
    int funcCallCount2 = 0;
    int condCallCount2 = 0;

    A* null = nullptr;

    bool enabled = false;

    auto c1 = [&](A*p){ ++condCallCount1; return p==&a && enabled;};
    auto c2 = [&](A*p){ ++condCallCount2; return p==nullptr && enabled;};

    auto f1 = [&](A*r){ ++funcCallCount1; QCOMPARE(r,&a); };
    auto f2 = [&](A*r){ ++funcCallCount2; QCOMPARE(r,null); };

    const int n = 20;
    for(int i = 0;i<n;++i){
        global::instance<A,MySub>().visitIf(c1,f1);
        global::instance<A,MySub>().visitIf(c2,f2);
    }



    QCOMPARE(funcCallCount1,0);
    QVERIFY(condCallCount1>=n);

    QCOMPARE(funcCallCount2,0);
    QVERIFY(condCallCount2>=n);

    enabled = true;

    {

        global::InstanceRegistration<A,MySub> registration(&a);

        QCOMPARE(funcCallCount1,n);
        QCOMPARE(funcCallCount2,0);

    }

    QCOMPARE(funcCallCount1,n);
    QCOMPARE(funcCallCount2,n);

}

void InstanceOperationsTest::recursiveQueuingWorks()
{
    class A{};
    A a;

    struct MySub{};

    int funcCallCount1 = 0;
    int funcCallCount2 = 0;

    bool cond1 = false;
    bool cond2 = false;

    global::instance<A,MySub>().visitIf(
                [&](A*){ return cond1;},
                [&](A*){ ++funcCallCount1;
                        global::instance<A,MySub>().visitIf(
                                    [&](A*){ return cond2;},
                                    [&](A*){ ++funcCallCount2;});
                       });

    cond1 = true;


    QCOMPARE(funcCallCount1,0);
    QCOMPARE(funcCallCount2,0);

    {
        global::InstanceRegistration<A,MySub> registration(&a);

        QCOMPARE(funcCallCount1,1);
        QCOMPARE(funcCallCount2,0);

        cond2 = true;
    }

    QCOMPARE(funcCallCount1,1);
    QCOMPARE(funcCallCount2,1);

}

void InstanceOperationsTest::registerForDestructionWorks()
{
    class A{};
    A a;

    struct MySub{};


    int funcCallCount = 0;


    global::instance<A,MySub>().visitIfNotNull(
                [&](A&){ global::instance<A,MySub>().visitIfNull(
                    [&](){ ++funcCallCount;});
                       });

    QCOMPARE(funcCallCount,0);

    {
        global::InstanceRegistration<A,MySub> registration(&a);
        QCOMPARE(funcCallCount,0);
    }

    QCOMPARE(funcCallCount,1);
}

