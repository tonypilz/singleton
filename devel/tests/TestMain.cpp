
#include "ConditionalSingleShotOperationsTest.h"
#include "OptionalValueTest.h"
#include "TestMain.h"

#include <ratio>
#include <iostream>
#include <cassert>
#include <src/instance.h>
#include <src/InstanceRegistration.h>
#include "InstanceTest.h"
#include "RegistrationTest.h"


namespace global {
namespace tests {


void testFailed(){
    throw 12345;
}

TestMain::TestMain(int argc, char* argv[])
{

    QApplication app(argc, argv);



    {
        InstanceTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }


    {
        RegistrationTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }

    {
        ConditionalSingleShotOperationsTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }

    {
        OptionalValueTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }


}


}
}

