#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include <thread>

#include <unistd.h>

#include "caseDevAlloc.h"
#include "testCmn.h"
#include "apusys.h"

enum {
    UT_DEVALLOC_FLOW,

    UT_DEVALLOC_MAX,
};


caseDevAlloc::caseDevAlloc()
{
    return;
}

caseDevAlloc::~caseDevAlloc()
{
    return;
}

static bool devAllocFlow(caseDevAlloc * inst, uint32_t loopNum)
{
    IApusysEngine * engine = IApusysEngine::createInstance("caseDevAlloc-devAllocFlow");
    int i = 0;
    bool ret = false;
    IApusysDev * dev = nullptr;


    for (i = 0; i < loopNum; i++)
    {
        LOG_INFO("allocate device...\n");
        dev = engine->devAlloc(APUSYS_DEVICE_SAMPLE);
        if (dev == nullptr)
        {
            LOG_ERR("caseDevAlloc-devAllocFlow: loop(%d/%d) alloc fail\n", i, loopNum);
            goto out;
        }
        LOG_INFO("allocate device done\n");

        sleep(1);

        LOG_INFO("free device...\n");
        if (engine->devFree(dev) == false)
        {
            LOG_ERR("caseDevAlloc-devAllocFlow: loop(%d/%d) free fail\n", i, loopNum);
            goto out;
        }
        LOG_INFO("free device done\n");
    }

    ret = true;

out:
    IApusysEngine::deleteInstance(engine);
    return ret;
}

bool caseDevAlloc::runCase(uint32_t subCaseIdx, uint32_t loopNum)
{
    bool ret = false;

    if(loopNum == 0)
    {
        LOG_ERR("caseDevAlloc: loop number(%d), do nothing\n", loopNum);
        return ret;
    }

    switch (subCaseIdx)
    {
        case UT_DEVALLOC_FLOW:
            ret = devAllocFlow(this, loopNum);
            break;

        default:
            break;
    }

    return ret;
}

void caseDevAllocInfo(void)
{
    LOG_CON("   <testCase> = %d : command test\n", APUSYS_UT_DEVALLOC);
    LOG_CON("     <subtestIdx> = %-2d : device allocate flow (allocate and free)\n", UT_DEVALLOC_FLOW);

    return;
}

