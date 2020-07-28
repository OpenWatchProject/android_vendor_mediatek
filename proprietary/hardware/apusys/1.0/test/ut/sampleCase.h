#pragma once
#include "sampleEngine.h"

enum apusysUTCase {
    APUSYS_UT_CMDPATH,
    APUSYS_UT_MEM,
    APUSYS_UT_DEVALLOC,

    APUSYS_UT_MAX,
};

class ISampleCase {
private:
    ISampleEngine * mEngine;

public:
    ISampleCase();
    virtual ~ISampleCase();
    virtual bool runCase(uint32_t subCaseIdx, uint32_t loop) = 0;
    ISampleEngine * getEngine() {return mEngine;}
};

