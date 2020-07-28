
#define LOG_TAG "TestAF"

#include <mtkcam/utils/std/Log.h>
#include "test_af.h"

#define MY_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

TEST_CASE_AF(TEST_AF_0)
{
    test3ARunner.prepareMeta_Set(AF, caseAF.getPrepareMetaFunc_Set(CASE_AF_1));
    test3ARunner.setVerifyFunc_Get(AF, caseAF.getVerifyFunc_Get(CASE_AF_1));
    test3ARunner.run(10);
}

TEST_CASE_AF(TEST_AF_1)
{
}

TEST_CASE_AF(TEST_AF_2)
{
}