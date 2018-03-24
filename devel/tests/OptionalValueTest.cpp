#include "OptionalValueTest.h"

#include <src/OptionalValue.h>

using global::detail::OptionalValue;
using global::detail::InvalidRead;

OptionalValueTest::OptionalValueTest(QObject *parent) : QObject(parent)
{

}

void OptionalValueTest::defaultConstructedValueNotSet()
{
    OptionalValue<A> a;
    QCOMPARE(a.isValueSet(),false);
}

void OptionalValueTest::accessingInvalidValueThrows()
{

     try{
        OptionalValue<A> a;
        auto x = A(a);
     }
    catch(InvalidRead const&){
        return;

    }
    catch(...){

    }

    QFAIL("");
}

void OptionalValueTest::assignedValueIsValid()
{
    OptionalValue<A> a;

    A b;
    a = b;
    QCOMPARE(a.isValueSet(),true);


}

void OptionalValueTest::ussettingAValueMakesItInvalid()
{
    OptionalValue<A> a;
    A b;
    a = b;
    a.unsetValue();
    QCOMPARE(a.isValueSet(),false);
}
