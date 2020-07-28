#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_TEST_3A_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_TEST_3A_H_

// #include "ITestControl.h"
#include "test_3a_runner.h"
#include "case_include.h"

enum TEST_3A_T
{
    TEST_3A_0 = 0,
    TEST_3A_1,
    TEST_3A_2,
    TEST_3A_3,
    TEST_3A_4,
    TEST_3A_NUM
};

// static Test3A test3A;

#define TEST_CASE_3A(case_cocde) \
    template <> \
    void Test3A::testCase<case_cocde>()

#define RUN_TEST_3A(case_cocde)  \
    case case_cocde: \
        Test3A::testCase<case_cocde>(); \
        break

class Test3A
{
public:
    Test3A(){};
    ~Test3A(){};

    template <MUINT32 case_cocde>
    static void testCase();

    //Test3ARunner* pTest3ARunner;
};

#endif