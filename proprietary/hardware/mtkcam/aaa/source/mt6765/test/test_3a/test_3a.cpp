
#define LOG_TAG "Test3A"

#include <mtkcam/utils/std/Log.h>
#include "test_3a.h"

#define MY_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

TEST_CASE_3A(TEST_3A_0)
{
    test3ARunner.prepareMeta_Set(AAA, case3A.getPrepareMetaFunc_Set(CASE_3A_1));
    test3ARunner.setVerifyFunc_Get(AAA, case3A.getVerifyFunc_Get(CASE_3A_1));
    test3ARunner.run(10);
}

TEST_CASE_3A(TEST_3A_1)
{
    test3ARunner.prepareMeta_Set(AAA, case3A.getPrepareMetaFunc_Set(PREVIEW));
    test3ARunner.setVerifyFunc_Get(AAA, case3A.getVerifyFunc_Get(CASE_3A_1));
    test3ARunner.run(10);
}

TEST_CASE_3A(TEST_3A_2)
{
}