#pragma once

#include "sampleCase.h"

class caseCmdPath : public ISampleCase {
public:
    caseCmdPath();
    ~caseCmdPath();
    bool runCase(uint32_t subCaseIdx, uint32_t loopNum);
};

void caseCmdPathInfo(void);

