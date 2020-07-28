#pragma once

#include "sampleCase.h"

class caseDevAlloc : public ISampleCase {
public:
    caseDevAlloc();
    ~caseDevAlloc();
    bool runCase(uint32_t subCaseIdx, uint32_t loopNum);
};

void caseDevAllocInfo(void);

