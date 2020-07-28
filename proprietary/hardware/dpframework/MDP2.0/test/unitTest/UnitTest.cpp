#include "DpDataType.h"
#include "TestCase.h"

int main(int argc, char** argv)
{
    int32_t         argIndex;
    TestReporter    reporter;
    char            *pTestName;
    TestCase        *pCurTest;

    // Default value
    pTestName = NULL;

    if (argc >= 3)
    {
        for (argIndex = 1; argIndex < argc; argIndex++)
        {
            if (!strcmp(argv[argIndex], "-testName"))
            {
                pTestName = argv[argIndex + 1];
                argIndex++;
            }
        }
    }

    const TestRegistry *pRegistry = TestRegistry::Head();

    while(pRegistry)
    {
        pCurTest = pRegistry->factory()(pTestName);
        if(pCurTest)  
        {
            pCurTest->setReporter(&reporter);

            DPLOGI("Test %s begin\n", pCurTest->getName());
            pCurTest->runTest();
            DPLOGI("Test %s end\n", pCurTest->getName());

            delete pCurTest;
        }

        if ((NULL != pTestName) &&
            (NULL != pCurTest))
        {
            // We have run the specified test case.
            break;
        }

        pRegistry = pRegistry->next();
    }

    //printf("Total cases: %d\n", reporter.countTests());
    //printf("Fail cases:  %d\n", reporter.countResults(TestReporter::kFailed));

    return 0;
}
