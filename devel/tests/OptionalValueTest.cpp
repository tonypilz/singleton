#include "OptionalValueTest.h"

#include <src/globalInstances.h>

using global::detail::optional;
using global::detail::bad_optional_access
;

OptionalValueTest::OptionalValueTest(QObject *parent) : QObject(parent)
{

}

void OptionalValueTest::defaultConstructedValueNotSet()
{
    optional<A> a;
    QCOMPARE(a.has_value(),false);
}

void OptionalValueTest::accessingInvalidValueThrows()
{

     try{
        optional<A> a;
        static_cast<A>(a);
     }
    catch(bad_optional_access
 const&){
        return;

    }
    catch(...){

    }

    QFAIL("");
}

void OptionalValueTest::assignedValueIsValid()
{
    optional<A> a;

    A b;
    a = b;
    QCOMPARE(a.has_value(),true);


}

void OptionalValueTest::ussettingAValueMakesItInvalid()
{
    optional<A> a;
    A b;
    a = b;
    a.reset();
    QCOMPARE(a.has_value(),false);
}
