// Example program

#include "tests/TestMain.h"
#include "examples/globalMocking.h"
#include "examples/simple.h"
#include "examples/globalDependency.h"
#include <src/staticValue.h>

//#include "tools/SingleFileTester.h"
//#include <tools/SingleFileGenerator.h>

#include <iostream>


int main(int argc, char* argv[])
{


    //dependency::main_dependency();
    globalMocking::main_mockable();
    global::tests::TestMain{argc, argv};

    std::cout<<__FILE__<<" finished\n";

    return 0;
}

