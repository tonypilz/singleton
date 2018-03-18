
#include "TestMain.h"

#include <ratio>
#include <iostream>
#include <cassert>
#include <src/instance.h>
#include <src/instanceRegistration.h>
#include <src/instanceCall.h>
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

}
}
}

