
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
    detail::throwImpl(std::exception{});
}

TestMain::TestMain(int argc, char* argv[])
{

    QCoreApplication app(argc, argv);



    {
        InstanceTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }


    {
        RegistrationTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }

    {
        OptionalValueTest tc;
        if (QTest::qExec(&tc, argc, argv)!=0) testFailed();
    }


}


}
}

