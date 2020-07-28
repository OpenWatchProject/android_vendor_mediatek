#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>

#include "sampleCase.h"
#include "caseCmdPath.h"
#include "caseMem.h"
#include "caseDevAlloc.h"
#include "testCmn.h"

#define APUSYS_UT_LOOP_MAX 1000000000

static void testInfo(void)
{
    LOG_CON("=============================================================\n");
    LOG_CON(" apusys sample test \n");
    LOG_CON("=============================================================\n");
    LOG_CON(" prototype: apusys_test64 <testCase> <subtestIdx> <#loopNum>\n");
    LOG_CON("   *sample.ko for apusys is nesscary\n");
    LOG_CON("-----------------------------------------------\n");
    caseCmdPathInfo();
    LOG_CON("\n");
    LOG_CON("   <testCase> = 1 : memory test\n");
    LOG_CON("\n");
    caseDevAllocInfo();
    LOG_CON("=============================================================\n");
    return;
}

// argument: testcase loop
int main(int argc, char * argv[])
{
    uint32_t testCase = 0;
    uint32_t subTestCase = 0;
    uint32_t loopNum = 0;
    int ret = 0;

    void * testEngine;

    /* check argument number */
    if (argc != 4)
    {
        testInfo();
        return -1;
    }

    /* get argument */
    testCase = atoi(argv[1]);
    subTestCase = atoi(argv[2]);
    loopNum = atoi(argv[3]);

    LOG_INFO("arguement: testcase(%d-%d), loopNum = %d\n", testCase, subTestCase, loopNum);

    if (loopNum > APUSYS_UT_LOOP_MAX)
    {
        loopNum = APUSYS_UT_LOOP_MAX;
    }

    if (testCase >= APUSYS_UT_MAX)
    {
        LOG_ERR("invalid test case idx(%d/%d) \n", testCase, APUSYS_UT_MAX-1);
        return -1;
    }

    switch (testCase)
    {
        case APUSYS_UT_CMDPATH:

            testEngine = new caseCmdPath;
            if (testEngine == nullptr)
            {
                LOG_ERR("allocate cmd path inst fail\n");
                ret = -1;
            }

            if (((caseCmdPath *)testEngine)->runCase(subTestCase, loopNum) == false)
            {
                LOG_ERR("run case(%d-%d) fail\n", testCase, subTestCase);
                ret = -1;
            }
            else
            {
                LOG_INFO("run case(%d-%d) ok\n", testCase, subTestCase);
            }

            delete testEngine;

            break;

        case APUSYS_UT_MEM:
            testEngine = new caseMem;
            if (testEngine == nullptr)
            {
                LOG_ERR("test Mem fail\n");
                ret = -1;
            }
            if (((caseMem *)testEngine)->runCase(subTestCase, loopNum) == false)
            {
                LOG_ERR("run case(%d-%d) fail\n", testCase, subTestCase);
                ret = -1;
            }
            else
            {
                LOG_INFO("run case(%d-%d) ok, should check kernel log\n", testCase, subTestCase);
            }

            delete (caseMem *)testEngine;
            break;

        case APUSYS_UT_DEVALLOC:
            testEngine = new caseDevAlloc;
            if (testEngine == nullptr)
            {
                LOG_ERR("alloc deviceAlloc inst fail\n");
                ret = -1;
            }

            if (((caseDevAlloc *)testEngine)->runCase(subTestCase, loopNum) == false)
            {
                LOG_ERR("run case(%d-%d) fail\n", testCase, subTestCase);
                ret = -1;
            }
            else
            {
                LOG_INFO("run case(%d-%d) ok\n", testCase, subTestCase);
            }

            delete (caseDevAlloc *)testEngine;

            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}

