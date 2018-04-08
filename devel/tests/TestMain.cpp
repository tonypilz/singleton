
#include "DeferredOperationsTest.h"
#include "OptionalValueTest.h"
#include "TestMain.h"

#include <ratio>
#include <iostream>
#include <cassert>
#include <src/globalInstances.h>
#include "InstanceTest.h"
#include "RegistrationTest.h"


namespace global {
namespace tests {


void testFailed(){
    detail::do_throw(std::exception{});
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
        DeferredOperationsTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }

    {
        OptionalValueTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }


}


}
}

