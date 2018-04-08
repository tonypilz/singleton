#include "RegistrationTest.h"
#include <src/globalInstances.h>
#include <src/globalInstances.h>

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



void RegistrationTest::SingleInstanceRegistrationAllowsOnlySingleRegistration()
{
#ifdef __cpp_exceptions
    class A{};
    A a;

    global::detail::InstanceRegistration<A> registration(&a);
    try {
        global::detail::InstanceRegistration<A> registration(&a);
    }
    catch(global::InstanceReplacementNotAllowed const&){}
    catch(...){ QFAIL("");}


#endif

}


void RegistrationTest::ReplacingInstanceRegistrationReplacesInstanceTemporarily()
{
    class A{};
    A a1,a2;

    global::detail::InstanceRegistration<A> registration(&a1);
    QVERIFY(static_cast<A*>(global::instance<A>())==&a1);
    {
        global::detail::ReplacingInstanceRegistration<A> registration(&a2);
        QVERIFY(static_cast<A*>(global::instance<A>())==&a2);
    }

   QVERIFY(static_cast<A*>(global::instance<A>())==&a1);

}



void RegistrationTest::registrationsCanBeChanged()
{

    class A{};
    A a1,a2;

    global::detail::ReplacingInstanceRegistration<A> registration(&a1);
    registration.registerInstance(&a2);

    {auto same = static_cast<A*>(global::instance<A>())==&a2; QVERIFY(same);}

}


void RegistrationTest::InstanceBasicallyWorks()
{

#ifdef __cpp_exceptions
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

#endif

}



void RegistrationTest::InstanceBasicallyWorksWithBaseclass()
{
    struct Base { virtual int foo()=0;};
    struct A : public Base{ int foo() override {return 4;} };
    global::Instance<Base,A> a;
    QCOMPARE(global::instance<Base>()->foo(),4);
}

void RegistrationTest::InstanceBasicallyWorksWithArgs()
{
    struct A{ double x; std::string y; A(const double x_, const std::string& y_):x(x_),y(y_){} };

    global::Instance<A> a(3,"bla");

    {
        auto eq = global::instance<A>()->x == 3;
        QVERIFY(eq);
    }

    {
        auto eq = global::instance<A>()->y == "bla";
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


void RegistrationTest::InstanceWorksWithStdMap()
{
    

    struct A { int x; };

    using Map = std::map<std::string,A>;
    global::Instance<Map> a { Map{
            {"hans", A{1}},
            {"wurst",A{2}}}};

    {
        const auto val = global::instance<Map>()->find("hans")->second.x;
        QCOMPARE(val,1);
    }

    {
        const auto val = global::instance<Map>()->find("wurst")->second.x;
        QCOMPARE(val,2);
    }


}


struct AX{
private:
    AX(){}

    template<
            template<typename, typename> class,
            typename ,
            typename ,
            typename >
    friend class global::detail::RegisterdInstanceT;
};


void RegistrationTest::privateConstructorsCanBeUsed(){

    
    global::Instance<AX> a;
}

struct AY{
private:
    AY(){}
    GLOBAL_INSTANCE_IS_FRIEND;
};


void RegistrationTest::privateConstructorsCanBeUsedWithMacro()
{
    
    global::Instance<AY> a;
}



