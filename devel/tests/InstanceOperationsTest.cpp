#include "InstanceOperationsTest.h"
#include <src/instance.h>
#include <src/instanceRegistration.h>
#include <src/instanceOperations.h>

InstanceOperationsTest::InstanceOperationsTest(QObject *parent) : QObject(parent)
{

}

void InstanceOperationsTest::functionWillBeCalledDirectlyIfInstanceDefined()
{
    class A{};
    A a;

    global::ExclusiveRegistration<A> registration(&a);

    bool called = false;
    global::onInstance<A>([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,true);

}

void InstanceOperationsTest::functionWillBeCalledDirectlyIfSubInstanceDefined()
{
    class A{};
    A a;

    struct MySub{};

    global::ExclusiveRegistration<A,MySub> registration(&a);

    bool called = false;
    global::onInstance<A,MySub>([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledIfInstanceIsDefined()
{
    class A{};
    A a;

    bool called = false;
    global::onInstance<A>([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });
    QCOMPARE(called,false);

    global::ExclusiveRegistration<A> registration(&a);

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledIfSubInstanceIsDefined()
{
    class A{};
    A a;

    struct MySub{};


    bool called = false;
    global::onInstance<A,MySub>([&called,&a](A&r){ called = true; QCOMPARE(&r,&a); });

    QCOMPARE(called,false);

    global::ExclusiveRegistration<A,MySub> registration(&a);

    QCOMPARE(called,true);
}

void InstanceOperationsTest::functionWillBeCalledOnlyOnceDirectly()
{
    class A{};
    A a;

    global::ExclusiveRegistration<A> registration(&a);


    int callCount = 0;
    global::onInstance<A>([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    QCOMPARE(callCount,1);
    A b;

    global::ReplacingScopedRegistration<A> registration1(&b);

    QCOMPARE(callCount,1);

}

void InstanceOperationsTest::functionWillBeCalledOnlyOnceIndirectly()
{
    class A{};
    A a;


    int callCount = 0;
    global::onInstance<A>([&callCount,&a](A&r){ ++callCount; QCOMPARE(&r,&a); });

    global::ExclusiveRegistration<A> registration(&a);

    QCOMPARE(callCount,1);

    A b;
    global::ReplacingScopedRegistration<A> registration1(&b);

    QCOMPARE(callCount,1);
}

void InstanceOperationsTest::conditionalFunctionWillBeCalledDirectlyIfInstanceDefined()
{
    class A{};
    A a;

    global::ExclusiveRegistration<A> registration(&a);

    int funcCallCount = 0;
    int condCallCount = 0;

    global::onInstance<A>(
                [&](A*p){ ++condCallCount; return p==&a;},
                [&](A*r){ QCOMPARE(r,&a); ++funcCallCount; });

    QCOMPARE(funcCallCount,1);
    QVERIFY(condCallCount>0);
}

void InstanceOperationsTest::conditionalFunctionWillBeCalledIfInstanceDefined()
{
    class A{};
    A a;

    int funcCallCount = 0;
    int condCallCount = 0;

    global::onInstance<A>(
                [&](A*p){ ++condCallCount; return p==&a;},
                [&](A*r){ QCOMPARE(r,&a); ++funcCallCount; });

    global::ExclusiveRegistration<A> registration(&a);

    QCOMPARE(funcCallCount,1);
    QVERIFY(condCallCount>0);
}

void InstanceOperationsTest::conditionalFunctionWillBeCalledDirectlyIfSubInstanceDefined()
{
    class A{};
    A a;

    struct MySub{};

    global::ExclusiveRegistration<A,MySub> registration(&a);

    int funcCallCount = 0;
    int condCallCount = 0;

    global::onInstance<A,MySub>(
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

    global::onInstance<A,MySub>(
                [&](A*p){ ++condCallCount; return p==&a;},
                [&](A*r){ QCOMPARE(r,&a); ++funcCallCount; });

    global::ExclusiveRegistration<A,MySub> registration(&a);

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
        global::onInstance<A,MySub>(c1,f1);
        global::onInstance<A,MySub>(c2,f2);
    }



    QCOMPARE(funcCallCount1,0);
    QVERIFY(condCallCount1>=n);

    QCOMPARE(funcCallCount2,0);
    QVERIFY(condCallCount2>=n);

    enabled = true;

    {

        global::ExclusiveRegistration<A,MySub> registration(&a);

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

    global::onInstance<A,MySub>(
                [&](A*){ return cond1;},
                [&](A*){ ++funcCallCount1;
                        global::onInstance<A,MySub>(
                                    [&](A*){ return cond2;},
                                    [&](A*){ ++funcCallCount2;});
                       });

    cond1 = true;


    QCOMPARE(funcCallCount1,0);
    QCOMPARE(funcCallCount2,0);

    {
        global::ExclusiveRegistration<A,MySub> registration(&a);

        QCOMPARE(funcCallCount1,1);
        QCOMPARE(funcCallCount2,0);

        cond2 = true;
    }

    QCOMPARE(funcCallCount1,1);
    QCOMPARE(funcCallCount2,1);


}

