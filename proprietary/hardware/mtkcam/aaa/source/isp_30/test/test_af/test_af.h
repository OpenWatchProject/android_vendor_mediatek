#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_TEST_AF_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_TEST_AF_H_

// #include "ITestControl.h"
#include "test_3a_runner.h"
#include "case_include.h"

enum TEST_AF_T
{
    TEST_AF_0 = 0,
    TEST_AF_1,
    TEST_AF_2,
    TEST_AF_3,
    TEST_AF_4,
    TEST_AF_NUM
};

#define TEST_CASE_AF(case_cocde) \
    template <> \
    void TestAF::testCase<case_cocde>()

#define RUN_TEST_AF(case_cocde)  \
    case case_cocde: \
        TestAF::testCase<case_cocde>(); \
        break

class TestAF
{
public:
    TestAF(){};
    ~TestAF(){};

    template <MUINT32 case_cocde>
    static void testCase();
};

#endif