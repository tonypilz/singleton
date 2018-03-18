#include "RegistrationTest.h"
#include <src/instance.h>
#include <src/instanceRegistration.h>

RegistrationTest::RegistrationTest(QObject *parent) : QObject(parent)
{

}

void RegistrationTest::leavingTheScopeOfASingleInstanceRegistrationDeregistersInstance()
{
    class A{};
    A a;

    QCOMPARE(global::isInstanceDefined<A>(),false);
    {
        global::ExclusiveRegistration<A> registration(&a);
        QCOMPARE(global::isInstanceDefined<A>(),true);
    }
    QCOMPARE(global::isInstanceDefined<A>(),false);
}

void RegistrationTest::leavingTheScopeOfASubSingleInstanceRegistrationDeregistersInstance()
{
    class A{};
    A a;

    class MySub{};

    {const bool def = global::isInstanceDefined<A,MySub>(); QCOMPARE(def,false);}
    {
        global::ExclusiveRegistration<A,MySub> registration(&a);
        {const bool def = global::isInstanceDefined<A,MySub>(); QCOMPARE(def,true);}
    }
    {const bool def = global::isInstanceDefined<A,MySub>(); QCOMPARE(def,false);}
}

void RegistrationTest::singleInstanceRegistrationAllowsOnlySingleRegistration()
{
    class A{};
    A a;

    global::ExclusiveRegistration<A> registration(&a);
    try {
        global::ExclusiveRegistration<A> registration(&a);
    }
    catch(global::InstanceReplacementNotAllowed const&){}
    catch(...){ QFAIL("");}

}

void RegistrationTest::singleInstanceSubRegistrationAllowsOnlySingleRegistration()
{
    class A{};
    A a;

    class MySub{};

    global::ExclusiveRegistration<A,MySub> registration(&a);
    try {
        global::ExclusiveRegistration<A,MySub> registration(&a);
    }
    catch(global::InstanceReplacementNotAllowed const&){}
    catch(...){ QFAIL("");}

}

void RegistrationTest::replacingInstanceRegistrationReplacesInstanceTemporarily()
{
    class A{};
    A a1,a2;

    global::ExclusiveRegistration<A> registration(&a1);
    QCOMPARE(&global::instance<A>(),&a1);
    {
        global::ReplacingScopedRegistration<A> registration(&a2);
        QCOMPARE(&global::instance<A>(),&a2);
    }

   QCOMPARE(&global::instance<A>(),&a1);

}

void RegistrationTest::replacingInstanceSubRegistrationReplacesInstanceTemporarily()
{
    class A{};
    A a1,a2;

    class MySub{};

    global::ExclusiveRegistration<A,MySub> registration(&a1);

    {auto r = &global::instance<A,MySub>(); QCOMPARE(r,&a1);}
    {
        global::ReplacingScopedRegistration<A,MySub> registration(&a2);
        {auto r = &global::instance<A,MySub>(); QCOMPARE(r,&a2);}
    }
    {auto r = &global::instance<A,MySub>(); QCOMPARE(r,&a1);}


}

void RegistrationTest::registrationsCanBeChanged()
{

    class A{};
    A a1,a2;

    global::ReplacingScopedRegistration<A> registration(&a1);
    registration.registerInstance(&a2);

    {auto r = &global::instance<A>(); QCOMPARE(r,&a2);}

}

void RegistrationTest::registrationsSubCanBeChanged()
{
    class A{};
    A a1,a2;

    class MySub{};

    global::ReplacingScopedRegistration<A,MySub> registration(&a1);
    registration.registerInstance(&a2);

    {auto r = &global::instance<A,MySub>(); QCOMPARE(r,&a2);}

}


