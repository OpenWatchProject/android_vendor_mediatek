#pragma once

#include "sampleCase.h"

class caseMem : public ISampleCase {
public:
    caseMem();
    virtual ~caseMem();
    bool runCase(uint32_t subCaseIdx, uint32_t loopNum);
};

