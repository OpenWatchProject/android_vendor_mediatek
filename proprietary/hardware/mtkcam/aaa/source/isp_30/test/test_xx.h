#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_TEST_XX_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_TEST_XX_H_

// #include "ITestControl.h"
#include "test_3a_runner.h"
#include "case_include.h"

enum TEST_XX_T
{
    TEST_XX_0 = 0,
    TEST_XX_1,
    TEST_XX_2,
    TEST_XX_3,
    TEST_XX_4,
    TEST_XX_NUM
};

#define TEST_CASE_XX(case_cocde) \
    template <> \
    void TestXX::testCase<case_cocde>()

#define RUN_TEST_XX(case_cocde)  \
    case case_cocde: \
        TestXX::testCase<case_cocde>(); \
        break

class TestXX
{
public:
    TestXX(){};
    ~TestXX(){};

    template <MUINT32 case_cocde>
    static void testCase();
};

#endif