#ifndef __VPU_TEST_3_0_MULTICASE_H__
#define __VPU_TEST_3_0_MULTICASE_H__

#include <cstdlib>
#include <vector>
#include <string>
#include "testCaseCmn.h"

/* algo used structure */
struct syncFlagList{
    uint32_t syncFlag[32];
};

struct multicoreProperty {
    uint32_t selfIdx;
    uint32_t totalIdx;
    uint32_t syncNum;

    uint32_t syncPattern;
};

//----------------------------------------
struct ionBuffer {
    uint64_t va;
    unsigned int len;
    int shareFd;
};

class ionAllocator {
private:
    int mFd;
    std::vector<ionBuffer *> mAllocList;

    bool checkAllocated(struct ionBuffer * handle);

public:
    ionAllocator();
    ~ionAllocator();
    struct ionBuffer * memAlloc(unsigned int size);
    bool memFree(struct ionBuffer * handle);
    bool memSync(struct ionBuffer * handle);
};

class vpuTestMultiCore {
private:
    int mCoreNum;

    VpuStream * mStream;
    VpuAlgo * mAlgo ;
    std::vector<char *> mAlgoBin;
    std::vector<VpuCreateAlgo> mCreateAlgoList;

    class ionAllocator mMemAllocator;

public:
    vpuTestMultiCore(int coreNum);
    ~vpuTestMultiCore();
    bool execute(int loops, unsigned int algoDelayMs);
};

#endif
