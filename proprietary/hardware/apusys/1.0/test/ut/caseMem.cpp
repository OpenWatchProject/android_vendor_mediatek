#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>

#include "sampleEngine.h"
#include "caseMem.h"
#include "testCmn.h"
#include "apusys.h"

#define CONSTRUCT_RUN_CMD_MAX 20

caseMem::caseMem()
{
    DEBUG_TAG;
}

caseMem::~caseMem()
{
    DEBUG_TAG;
}

bool ut_access(caseMem * caseInst, uint32_t loopNum)
{

    IApusysMem * testMem;
    char *data;
    int i = 0;
    bool ret = true;

    DEBUG_TAG;

    testMem = caseInst->getEngine()->memAlloc(10);
    LOG_DEBUG("\ntestMem----\nsize: %08x\niova: %08x\nkva: %08x\nuva: %08x\n",
            testMem->size, testMem->iova, testMem->kva, testMem->va);

    data = (char*) testMem->va;
    for(i = 0; i < 10; i++)
    {
        data[i] = i;
    }

    LOG_DEBUG("Sync\n");
    caseInst->getEngine()->memSync(testMem);
    for(i = 0; i < 10; i++)
    {
        if(data[i] != i)
        {

            ret = false;
        }
    }
    if(ret)
        LOG_DEBUG("R/W Check Pass\n");
    else
        LOG_DEBUG("R/W Check Fail\n");

    caseInst->getEngine()->memFree(testMem);

    DEBUG_TAG;
    return ret;

}

bool ut_import(caseMem * caseInst, uint32_t loopNum)
{
#if 0
    IApusysMem *srcMem, *dstMem;
    apusysIonMem* shareIonMem;
    char *data;
    bool ret = true;

    DEBUG_TAG;

    srcMem = caseInst->getEngine()->memAlloc(10);
    LOG_DEBUG("\nsrcMem----\nsize: %08x\niova: %08x\nkva: %08x\nuva: %08x\n",
            srcMem->size, srcMem->iova, srcMem->kva, srcMem->va);

    shareIonMem = (apusysIonMem* )srcMem;

    dstMem = caseInst->getEngine()->memImport(shareIonMem->ion_share_fd, shareIonMem->size);

    LOG_DEBUG("\ndstMem----\nsize: %08x\niova: %08x\nkva: %08x\nuva: %08x\n",
            dstMem->size, dstMem->iova, dstMem->kva, dstMem->va);

    if((srcMem->iova != dstMem->iova) || (srcMem->kva != dstMem->kva)) {
        ret = false;
        LOG_DEBUG("Import Check Fail\n");
    } else {
        LOG_DEBUG("Import Check Pass\n");
    }

    caseInst->getEngine()->memUnImport(dstMem);
    caseInst->getEngine()->memFree(srcMem);



    return ret;
#else
    return false;
#endif
}

bool caseMem::runCase(uint32_t subCaseIdx, uint32_t loopNum)
{
    if(loopNum == 0)
    {
        LOG_ERR("case: cmdPath loop number(%d), do nothing\n", loopNum);
        return false;
    }

    LOG_DEBUG("case: caseMem (%d/%d)\n", loopNum, subCaseIdx);
    switch(subCaseIdx)
    {
        case 0:
            return ut_access(this, loopNum);
        case 1:
            return ut_import(this, loopNum);
        default:
            LOG_DEBUG("case: caseMem (%d)\n", subCaseIdx);
    }

    return false;
}

