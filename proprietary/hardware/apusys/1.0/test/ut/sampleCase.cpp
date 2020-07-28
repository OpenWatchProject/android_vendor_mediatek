#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include <assert.h>

#include "sampleCase.h"
#include "testCmn.h"


ISampleCase::ISampleCase()
{
    mEngine = ISampleEngine::createInstance("sample case test");
    if (mEngine == nullptr)
    {
        LOG_ERR("create sample engine fail\n");
        assert(mEngine != nullptr);
    }
}

ISampleCase::~ISampleCase()
{
    if (ISampleEngine::deleteInstance(mEngine) == false)
    {
        LOG_ERR("delete sample engine fail\n");
    }
}

