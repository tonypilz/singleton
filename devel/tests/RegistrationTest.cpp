#include "RegistrationTest.h"
#include <src/instance.h>
#include <src/InstanceRegistration.h>

RegistrationTest::RegistrationTest(QObject *parent) : QObject(parent)
{

}

void RegistrationTest::leavingTheScopeOfASingleInstanceRegistrationDeregistersInstance()
{
    class A{};
    A a;

    QVERIFY(global::instance<A>()==nullptr);
    {
        global::detail::InstanceRegistration<A> registration(&a);
        QVERIFY(global::instance<A>()!=nullptr);
    }
    QVERIFY(global::instance<A>()==nullptr);
}

void RegistrationTest::leavingTheScopeOfASubSingleInstanceRegistrationDeregistersInstance()
{
    class A{};
    A a;

    class MySub{};

    {const bool def = global::instance<A,MySub>(); QCOMPARE(def,false);}
    {
        global::detail::InstanceRegistration<A,MySub> registration(&a);
        {const bool def = global::instance<A,MySub>(); QCOMPARE(def,true);}
    }
    {const bool def = global::instance<A,MySub>(); QCOMPARE(def,false);}
}

void RegistrationTest::SingleInstanceRegistrationAllowsOnlySingleRegistration()
{
    class A{};
    A a;

    global::detail::InstanceRegistration<A> registration(&a);
    try {
        global::detail::InstanceRegistration<A> registration(&a);
    }
    catch(global::InstanceReplacementNotAllowed const&){}
    catch(...){ QFAIL("");}

}

void RegistrationTest::singleInstanceSubRegistrationAllowsOnlySingleRegistration()
{
    class A{};
    A a;

    class MySub{};

    global::detail::InstanceRegistration<A,MySub> registration(&a);
    try {
        global::detail::InstanceRegistration<A,MySub> registration(&a);
    }
    catch(global::InstanceReplacementNotAllowed const&){}
    catch(...){ QFAIL("");}

}

void RegistrationTest::ReplacingInstanceRegistrationReplacesInstanceTemporarily()
{
    class A{};
    A a1,a2;

    global::detail::InstanceRegistration<A> registration(&a1);
    QVERIFY(global::instance<A>()==&a1);
    {
        global::detail::ReplacingInstanceRegistration<A> registration(&a2);
        QVERIFY(global::instance<A>()==&a2);
    }

   QVERIFY(global::instance<A>()==&a1);

}

void RegistrationTest::replacingInstanceSubRegistrationReplacesInstanceTemporarily()
{
    class A{};
    A a1,a2;

    class MySub{};

    global::detail::InstanceRegistration<A,MySub> registration(&a1);

    {auto same = global::instance<A,MySub>()==&a1; QVERIFY(same );}
    {
        global::detail::ReplacingInstanceRegistration<A,MySub> registration(&a2);
        {auto same = global::instance<A,MySub>()==&a2; QVERIFY(same );}
    }
    {auto same = global::instance<A,MySub>()==&a1; QVERIFY(same );}


}

void RegistrationTest::registrationsCanBeChanged()
{

    class A{};
    A a1,a2;

    global::detail::ReplacingInstanceRegistration<A> registration(&a1);
    registration.registerInstance(&a2);

    {auto same = global::instance<A>()==&a2; QVERIFY(same);}

}

void RegistrationTest::registrationsSubCanBeChanged()
{
    class A{};
    A a1,a2;

    class MySub{};

    global::detail::ReplacingInstanceRegistration<A,MySub> registration(&a1);
    registration.registerInstance(&a2);

    {auto same = global::instance<A,MySub>()==&a2; QVERIFY(same);}

}

void RegistrationTest::InstanceBasicallyWorks()
{

    class A{ };
    global::Instance<A> a;
    QVERIFY(global::instance<A>()!=nullptr);

    try {
        global::Instance<A> b;
    }
    catch(global::InstanceReplacementNotAllowed const&){

    }
    catch(...){
        QFAIL("");
    }
}

void RegistrationTest::InstanceBasicallyWorksSub()
{
    class A{ };
    class Sub {};
    global::SubInstance<A,Sub> a;

    const auto def = global::instance<A,Sub>()!=nullptr;
    QVERIFY(def);
}

void RegistrationTest::InstanceBasicallyWorksWithBaseclass()
{
    struct Base { virtual int foo()=0;};
    struct A : public Base{ int foo() override {return 4;} };
    global::Instance<Base,A> a;
    QCOMPARE(global::instance<Base>()->foo(),4);
}

void RegistrationTest::InstanceBasicallyWorksWithArgsSub()
{
    struct A{ double x; std::string y; A(const double x_, const std::string& y_):x(x_),y(y_){} };
    class Sub {};
    global::SubInstance<A,Sub> a(3,"bla");

    {
        auto eq = global::instance<A,Sub>()->x == 3;
        QVERIFY(eq);
    }

    {
        auto eq = global::instance<A,Sub>()->y == "bla";
        QVERIFY(eq);
    }

}

void RegistrationTest::TestInstanceBasicallyWorks()
{

    struct A { virtual int foo(){ return 4; }};
    global::Instance<A> a;

    {
        struct A_Mock : public A { int foo() override { return 5; }};
        global::TestInstance<A,A_Mock> a_mock;

        QCOMPARE(global::instance<A>()->foo(),5);
    }

    QCOMPARE(global::instance<A>()->foo(),4);

}

void RegistrationTest::TestInstanceBasicallyWorksSub()
{
    struct Sub{};

    struct A { virtual int foo(){ return 4; }};
    global::SubTestInstance<A,Sub> a;

    {
        struct A_Mock : public A { int foo() override { return 5; }};
        global::SubTestInstance<A,Sub, A_Mock> a_mock;

        const auto val = global::instance<A,Sub>()->foo();
        QCOMPARE(val,5);
    }

    const auto val = global::instance<A,Sub>()->foo();
    QCOMPARE(val,4);
}


