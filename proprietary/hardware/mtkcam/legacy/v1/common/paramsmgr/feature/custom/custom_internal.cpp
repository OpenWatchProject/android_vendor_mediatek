#define LOG_TAG "MtkCam/Custom_Internal"

//
#include <stdlib.h>
#include <stdio.h>
#include <Log.h>
#include <math.h>
//
#include <cutils/properties.h>
//
#include <vector>

using namespace std;
//
#include <mtkcam/utils/fwk/MtkCameraParameters.h>
using namespace android;
//
#include <FeatureDef.h>
using namespace NSCameraFeature;
//
#include <custom_internal.h>


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

namespace android {
    namespace camera {
        namespace v1 {

static char ** g_pic_main_arr, **g_pic_sub_arr;
static char ** g_prv_main_arr, **g_prv_sub_arr;
static char ** g_video_main_arr, **g_video_sub_arr;
static int g_dump_flag = 0;

int getDumpFlag()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.dft.dump", value, "0");
    g_dump_flag = ::atoi(value);

    return g_dump_flag;
}

int isAligned(int w, int h, int aligned)
{
    return w % aligned == 0 && h % aligned == 0;
}

int isAligned_16(int w, int h)
{
    return isAligned(w, h, 16);
}

int isSize4_3(int width, int height)
{
    if(width * 3 > INT_MAX - 1)
    {
        MY_LOGD("width: %d is too big!\n", width);
        return 0;
    }

    if(height * 3 > INT_MAX - 1)
    {
        MY_LOGD("height: %d is too big!\n", height);
        return 0;
    }

    if(width * 3 == height * 4)
        return 1;
    return 0;
}

int isSize3_2(int width, int height)
{
    if(width * 2 > INT_MAX - 1)
    {
        MY_LOGD("width:%d is too big!\n", width);
        return 0;
    }

    if(height * 3 > INT_MAX - 1)
    {
        MY_LOGD("height:%d is too big!\n", height);
        return 0;
    }

    if(width * 2 == height * 3)
        return 1;
    return 0;
}

int isSize5_3(int width, int height)
{
    if(width * 3 > INT_MAX - 1)
    {
        MY_LOGD("width:%d is too big!\n", width);
        return 0;
    }

    if(height * 5 > INT_MAX - 1)
    {
        MY_LOGD("height:%d is too big!\n", height);
        return 0;
    }

    if(width * 3 == height * 5)
        return 1;
    return 0;
}

int isSize16_9(int width, int height)
{
    if(width * 9 > INT_MAX - 1)
    {
        MY_LOGD("width:%d is too big!\n", width);
        return 0;
    }

    if(height * 16 > INT_MAX - 1)
    {
        MY_LOGD("height:%d is too big!\n", height);
        return 0;
    }

    if(width * 9 == height * 16)
        return 1;
    return 0;
}

int genBiggestSizeForRatio4_3(int max_w, int max_h, int *gen_max_w, int *gen_max_h)
{
    int quotient;
#if 0
    if(1.0 * max_w / max_h <= 4. / 3) {
        *gen_max_w = (max_w / 16) * 16;     // align with 16
        *gen_max_h = (*gen_max_w * 3 / 4);
        *gen_max_h = *gen_max_h / 16 * 16;
        *gen_max_w = *gen_max_h * 4 / 3;
    } else {
        *gen_max_h = (max_h / 16) * 16;     // align with 16
        *gen_max_w = *gen_max_h * 4 / 3;
        *gen_max_w = *gen_max_w / 16 * 16;
        *gen_max_h = *gen_max_w * 3 / 4;
    }
#else       // old algo
    int quotient_w = max_w / 64;    // 64 = 16 * 4
    int quotient_h = max_h / 48;    // 48 = 16 * 3

    if(quotient_w < quotient_h)
        quotient = quotient_w;
    else
        quotient = quotient_h;

    *gen_max_w = quotient * 64;
    *gen_max_h = quotient * 48;
#endif
    MY_LOGD_IF(getDumpFlag() > 0, "Get biggest size(4:3):%dx%d\n", *gen_max_w, *gen_max_h);
    return 1;
}

int genBiggestSizeForRatio3_2(int max_w, int max_h, int *gen_max_w, int *gen_max_h)
{
    int quotient;
#if 0
    if(1.0 * max_w / max_h <= 3. / 2) {
        *gen_max_w = (max_w / 16) * 16;     // align with 16
        *gen_max_h = (*gen_max_w * 2 / 3);
        *gen_max_h = *gen_max_h / 16 * 16;
        *gen_max_w = *gen_max_h * 3 / 2;
    } else {
        *gen_max_h = (max_h / 16) * 16;     // align with 16
        *gen_max_w = *gen_max_h * 3 / 2;
        *gen_max_w = *gen_max_w / 16 * 16;
        *gen_max_h = *gen_max_w * 2 / 3;
    }
#else       // old algo
    int quotient_w = max_w / 48;    // 48 = 16 * 3
    int quotient_h = max_h / 32;    // 32 = 16 * 2

    if(quotient_w < quotient_h)
        quotient = quotient_w;
    else
        quotient = quotient_h;

    *gen_max_w = quotient * 48;
    *gen_max_h = quotient * 32;
#endif
    MY_LOGD_IF(getDumpFlag() > 0, "Get biggest size(3:2):%dx%d\n", *gen_max_w, *gen_max_h);
    return 1;
}

int genBiggestSizeForRatio5_3(int max_w, int max_h, int *gen_max_w, int *gen_max_h)
{
    int quotient;
#if 0
    if(1.0 * max_w / max_h <= 5. / 3) {
        *gen_max_w = (max_w / 16) * 16;     // align with 16
        *gen_max_h = (*gen_max_w * 3 / 5);
        *gen_max_h = *gen_max_h / 16 * 16;
        *gen_max_w = *gen_max_h * 5 / 3;
    } else {
        *gen_max_h = (max_h / 16) * 16;     // align with 16
        *gen_max_w = *gen_max_h * 5 / 3;
        *gen_max_w = *gen_max_w / 16 * 16;
        *gen_max_h = *gen_max_w * 3 / 5;
    }
#else       // old algo
    int quotient_w = max_w / 80;    // 80 = 16 * 5
    int quotient_h = max_h / 48;    // 48 = 16 * 3

    if(quotient_w < quotient_h)
        quotient = quotient_w;
    else
        quotient = quotient_h;

    *gen_max_w = quotient * 80;
    *gen_max_h = quotient * 48;
#endif

    MY_LOGD_IF(getDumpFlag() > 0, "Get biggest size(5:3):%dx%d\n", *gen_max_w, *gen_max_h);
    return 1;
}

int genBiggestSizeForRatio16_9(int max_w, int max_h, int *gen_max_w, int *gen_max_h)
{
    int quotient;
#if 0
    if(1.0 * max_w / max_h <= 16. / 9) {
        *gen_max_w = (max_w / 16) * 16;     // align with 16
        *gen_max_h = (*gen_max_w * 9 / 16);
        *gen_max_h = *gen_max_h / 16 * 16;
        *gen_max_w = *gen_max_h * 16 / 9;
    } else {
        *gen_max_h = (max_h / 16) * 16;     // align with 16
        *gen_max_w = *gen_max_h * 16 / 9;
        *gen_max_w = *gen_max_w / 16 * 16;
        *gen_max_h = *gen_max_w * 9 / 16;
    }
#else       // old algo
    int quotient_w = max_w / 256;       // 256 = 16 * 16
    int quotient_h = max_h / 144;       // 144 = 16 * 9

    if(quotient_w < quotient_h)
        quotient = quotient_w;
    else
        quotient = quotient_h;

    *gen_max_w = quotient * 256;
    *gen_max_h = quotient * 144;
#endif
    MY_LOGD_IF(getDumpFlag() > 0, "Get biggest size(16:9):%dx%d\n", *gen_max_w, *gen_max_h);
    return 1;
}

int isExistInSizeArrList(char **arr_list, const int cnt, int w, int h)
{
    int i = 0;

    while( i < cnt)
    {
        int temp_w, temp_h;
        int sscanf_ret;
        const char *size;

        size = arr_list[i];
        MY_LOGD_IF(getDumpFlag() > 1, "size:%s", size);
        sscanf_ret = sscanf(size, "%dx%d", &temp_w, &temp_h);
        if(sscanf_ret != 2)
            return false;
        else
        {
            if(w == temp_w && h == temp_h)
                return true;
        }
        ++i;
    }
    return false;
}

int isBiggestInSizeArrList(char **arr_list, const int cnt, int w, int h)
{
    int i = 0;
    int biggest;
    int size = w * h;

    while( i < cnt)
    {
        int temp_w, temp_h;
        int sscanf_ret;
        const char *size_str;

        size_str = arr_list[i];
        sscanf_ret = sscanf(size_str, "%dx%d", &temp_w, &temp_h);
        if(sscanf_ret != 2)
        {
            MY_LOGE("Cirical: sscanf error: Size is |%s|", size_str);
            return true;
        }
        else
        {
            if(temp_w * temp_h > biggest)
                biggest = temp_w * temp_h;
            if(size <= biggest)
                return false;
        }
        ++i;
    }
    return false;
}


int getNearestRatio(int w, int h)
{
    double ratio = ((double)w) / h;

    if(fabs(ratio - (16.) / 9) <= 1e-7)
        return SIZE_RATIO_16_9;
    if(fabs(ratio - (5.) / 3) <= 1e-7)
        return SIZE_RATIO_5_3;
    if(fabs(ratio - (3.) / 2) <= 1e-7)
        return SIZE_RATIO_3_2;
    if(fabs(ratio - (4.) / 3) <= 1e-7)
        return SIZE_RATIO_4_3;
    return SIZE_RATIO_4_3;
}


int getSizeRatio(int w, int h)
{
    if(isSize4_3(w, h))
        return SIZE_RATIO_4_3;
    if(isSize3_2(w, h))
        return SIZE_RATIO_3_2;
    if(isSize5_3(w, h))
        return SIZE_RATIO_5_3;
    if(isSize16_9(w, h))
        return SIZE_RATIO_16_9;
    return SIZE_RATIO_UNKNOWN;
}

int add4_3_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW, baseH;
    int i;
    int should_calculate;
    int gen_max_w, gen_max_h;

    /*
    // if you want to customize the picture sizes, you can do as:
    if(max_w == xxx && max_h == xxx)
    {
        size_arr[currCnt++] = "320x240";
        size_arr[currCnt++] = "640x480";
        ......
        return currCnt;
    }
    */

    // the min 4:3 max_w/max_h supported picture size is 640x480
    baseW = 320, baseH = 240;
    i = 1;
    should_calculate = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        // if the size length is bigger than 128, should set it bigger
        char *temp = (char *)malloc(128);
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(4:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }
    // base on 320x240 end

    baseW = 256, baseH = 192;
    i = 1;
    should_calculate = 1;
#if 0       // advanced method
    // when max size is too big, then don't calculate these sizes based on
    // 256x192, because if we do, the picture size values is too many...
    if(max_w / baseW > 4)
        should_calculate = 0;
    while(should_calculate && baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(4:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }
#else   // simple method
    #if 0
        if(max_w >= 1024 && max_h >= 768)
            size_arr[currCnt++] = (char *)"1024x768";
        if(max_w >= 2048 && max_h >= 1536)
            size_arr[currCnt++] = (char *)"2048x1536";
    #endif
#endif


    baseW = 1600, baseH = 1200;
    i = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // todo if possible
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(4:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    // gen a biggest size for 4:3
    genBiggestSizeForRatio4_3(max_w, max_h, &gen_max_w, &gen_max_h);
    // if not exist in size arr list
    if(!isExistInSizeArrList(size_arr, currCnt, gen_max_w, gen_max_h))
     //&& isBiggestInSizeArrList(size_arr, currCnt, gen_max_w, gen_max_h))
    {
        // add the size
        char *temp = (char *)malloc(128);
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", gen_max_w, gen_max_h);
        MY_LOGD("Add a biggest size(4:3):%s", temp);
        size_arr[currCnt++] = temp;
    }

    label_fail:
        return currCnt;
}

int add3_2_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW, baseH;
    int i;
    int gen_max_w, gen_max_h;

    /*
    // if you want to customize the picture sizes, you can do as:
    if(max_w == xxx && max_h == xxx)
    {
        size_arr[currCnt++] = "480x320";
        size_arr[currCnt++] = "624x416";
        ......
        return currCnt;
    }
    */

    // for max_w/max_h <= 640x480
    if(max_w <= 640 && max_h <= 480)
    {
        size_arr[currCnt++] = (char *)"480x320";
        size_arr[currCnt++] = (char *)"624x416";
        return currCnt;
    }

    // based on: 480x320
    baseW = 480, baseH = 320;
    i = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(3:2):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    // gen a biggest size for 3:2
    genBiggestSizeForRatio3_2(max_w, max_h, &gen_max_w, &gen_max_h);
    // if not exist in size arr list
    if(!isExistInSizeArrList(size_arr, currCnt, gen_max_w, gen_max_h))
      //&& isBiggestInSizeArrList(size_arr, currCnt, gen_max_w, gen_max_h))
    {
        // add the size
        char *temp = (char *)malloc(128);
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", gen_max_w, gen_max_h);
        MY_LOGD("Add a biggest size(3:2):%s", temp);
        size_arr[currCnt++] = temp;
    }

    label_fail:
        return currCnt;
}


int add5_3_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW, baseH;
    int i;
    int gen_max_w, gen_max_h;

    /*
    // if you want to customize the picture sizes, you can do as:
    if(max_w == xxx && max_h == xxx)
    {
        size_arr[currCnt++] = "400x240";
        size_arr[currCnt++] = "800x480";
        ......
        return currCnt;
    }
    */

    // for max_w/max_h <= 640x480
    if(max_w <= 640 && max_h <= 480)
    {
        size_arr[currCnt++] = (char *)"320x192";
        size_arr[currCnt++] = (char *)"640x384";
        return currCnt;
    }

    // based on 400x240
    baseW = 400, baseH = 240;
    i = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(5:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    // based on 1280x768
    baseW = 1280, baseH = 768;
    i = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(5:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    // gen a biggest size for 5:3
    genBiggestSizeForRatio5_3(max_w, max_h, &gen_max_w, &gen_max_h);
    // if not exist in size arr list
    if(!isExistInSizeArrList(size_arr, currCnt, gen_max_w, gen_max_h))
      //&& isBiggestInSizeArrList(size_arr, currCnt, gen_max_w, gen_max_h))
    {
        // add the size
        char *temp = (char *)malloc(128);
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", gen_max_w, gen_max_h);
        MY_LOGD("Add a biggest size(5:3):%s", temp);
        size_arr[currCnt++] = temp;
    }

    label_fail:
        return currCnt;
}

int add16_9_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW, baseH;
    int i = 1;
    int gen_max_w, gen_max_h;

    /*
    // if you want to customize the picture sizes, you can do as:
    if(max_w == xxx && max_h == xxx)
    {
        size_arr[currCnt++] = "640x360";
        size_arr[currCnt++] = "1280x720";
        ......
        return currCnt;
    }
    */

    // for max_w/max_h <= 640x480
    if(max_w <= 640 && max_h <= 480)
    {
        size_arr[currCnt++] = (char *)"256x144";
        size_arr[currCnt++] = (char *)"512x288";
        return currCnt;
    }

    // based on 1280x720
    baseW = 1280, baseH = 720;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(16:9):%s\n", size_arr[currCnt]);
        ++currCnt;

        baseW *= 2;
        baseH *= 2;
    }

    // based on 1600x912
    baseW = 1600, baseH = 912;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(16:9):%s\n", size_arr[currCnt]);
        ++currCnt;

        baseW *= 2;
        baseH *= 2;
    }

    // gen a biggest size for 16:9
    genBiggestSizeForRatio16_9(max_w, max_h, &gen_max_w, &gen_max_h);
    // if not exist in size arr list
    if(!isExistInSizeArrList(size_arr, currCnt, gen_max_w, gen_max_h))
      //&& isBiggestInSizeArrList(size_arr, currCnt, gen_max_w, gen_max_h))
    {
        // add the size
        char *temp = (char *)malloc(128);
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", gen_max_w, gen_max_h);
        MY_LOGD("Add a biggest size(16:9):%s", temp);
        size_arr[currCnt++] = temp;
    }

    // if >= 1920x1080, add 1920x1088
    if(max_w >= 1920 && max_h >= 1080)
    {
        size_arr[currCnt++] = (char *)"1920x1088";
        return currCnt;
    }

    label_fail:
        return currCnt;
}


int add4_3_PrvSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW, baseH;
    int i;
    int should_calculate;
    int gen_max_w, gen_max_h;

    /*
    // if you want to customize the picture sizes, you can do as:
    if(max_w == xxx && max_h == xxx)
    {
        size_arr[currCnt++] = "320x240";
        size_arr[currCnt++] = "640x480";
        ......
        return currCnt;
    }
    */

    // the min 4:3 max_w/max_h supported picture size is 640x480
    baseW = 320, baseH = 240;
    i = 1;
    should_calculate = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        // if the size length is bigger than 128, should set it bigger
        char *temp = (char *)malloc(128);
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(4:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }
    // base on 320x240 end

label_fail:
    return currCnt;
}

int add3_2_PrvSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW, baseH;
    int i;
    int gen_max_w, gen_max_h;

    /*
    // if you want to customize the picture sizes, you can do as:
    if(max_w == xxx && max_h == xxx)
    {
        size_arr[currCnt++] = "480x320";
        size_arr[currCnt++] = "624x416";
        ......
        return currCnt;
    }
    */

    // for max_w/max_h <= 640x480
    if(max_w <= 640 && max_h <= 480)
    {
        size_arr[currCnt++] = (char *)"480x320";
        size_arr[currCnt++] = (char *)"624x416";
        return currCnt;
    }

    // based on: 480x320
    baseW = 480, baseH = 320;
    i = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(3:2):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    // if >= 640x480, then add 720x480 for compatible
    if(max_w >= 640 && max_h >= 480) {
        size_arr[currCnt++] = (char *)"720x480";
    }

label_fail:
    return currCnt;
}


int add5_3_PrvSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW, baseH;
    int i;
    int gen_max_w, gen_max_h;

    /*
    // if you want to customize the picture sizes, you can do as:
    if(max_w == xxx && max_h == xxx)
    {
        size_arr[currCnt++] = "400x240";
        size_arr[currCnt++] = "800x480";
        ......
        return currCnt;
    }
    */

    // for max_w/max_h <= 640x480
    if(max_w <= 640 && max_h <= 480)
    {
        size_arr[currCnt++] = (char *)"320x192";
        size_arr[currCnt++] = (char *)"640x384";
        return currCnt;
    }

    // based on 400x240
    baseW = 400, baseH = 240;
    i = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(5:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    // based on 1280x768
    baseW = 1280, baseH = 768;
    i = 1;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(5:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

label_fail:
    return currCnt;
}

int add16_9_PrvSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW, baseH;
    int i = 1;
    int gen_max_w, gen_max_h;

    /*
    // if you want to customize the picture sizes, you can do as:
    if(max_w == xxx && max_h == xxx)
    {
        size_arr[currCnt++] = "640x360";
        size_arr[currCnt++] = "1280x720";
        ......
        return currCnt;
    }
    */

    // for max_w/max_h <= 640x480
    if(max_w <= 640 && max_h <= 480)
    {
        size_arr[currCnt++] = (char *)"256x144";
        size_arr[currCnt++] = (char *)"512x288";
        return currCnt;
    }

    // based on 1280x720
    baseW = 1280, baseH = 720;
    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD_IF(getDumpFlag() > 0, "Add one size(16:9):%s\n", size_arr[currCnt]);
        ++currCnt;

        baseW *= 2;
        baseH *= 2;
    }

label_fail:
    return currCnt;
}



int generatePicSizeValues(int max_w, int max_h, char ***size_arr, int force_ratio)
{
    double ratio = ((double)max_w)/max_h;
    int ratioInt = getSizeRatio(max_w, max_h);
    int base;
    int i;
    int w, h;
    int ratioW, ratioH;
    int ret = 0;
    int new_ratio;
    int max_count = 4096;        // todo: need modify if max sizes count is bigger than 4096
    int addedCntAfter = 0;

    MY_LOGD_IF(getDumpFlag() > 0, "+");
    *size_arr = NULL;

    // avoid nagative max_w/max_h
    if(max_w <= 0 || max_h <= 0)
    {
        // default: QVGA
        max_w = 640;
        max_h = 480;
    }
    // if width < height, then swap them
    if(max_w < max_h)
    {
        int temp = max_w;
        max_w = max_h;
        max_h = temp;
    }

    if(force_ratio >= SIZE_RATIO_4_3 && force_ratio <= SIZE_RATIO_16_9)
    {
        ratioInt = force_ratio;
    }

label_switch_ratio:
    switch(ratioInt)
    {
        case SIZE_RATIO_4_3:
            base = max_w/16/4;
            ratioW = 4;
            ratioH = 3;
        break;
        case SIZE_RATIO_3_2:
            base = max_w/16/3;
            ratioW = 3;
            ratioH = 2;
        break;
        case SIZE_RATIO_5_3:
            base = max_w/16/5;
            ratioW = 5;
            ratioH = 3;
        break;
        case SIZE_RATIO_16_9:
            base = max_w/16/16;
            ratioW = 16;
            ratioH = 9;
        break;
        default:
            ratioInt = getNearestRatio(max_w, max_h);
            goto label_switch_ratio;
        break;
    }
    *size_arr = (char **)malloc(max_count * sizeof(char *));
    if(!*size_arr)
        goto label_fail;
    // initialize size arr, set as 0
    memset(*size_arr, 0, max_count * sizeof(char *));


    #if 0
    for(i = 1; i <= base; ++i)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;
        w = i * 16 * ratioW;
        h = i * 16 * ratioH;

        sprintf(temp, "%dx%d", w, h);
        (*size_arr)[i - 1] = temp;
        //MY_LOGD("Add one size:%s\n", (*size_arr)[i - 1]);
        ++ret;
    }
    #else

        addedCntAfter = add4_3_PictureSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add5_3_PictureSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add3_2_PictureSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add16_9_PictureSizes(*size_arr, addedCntAfter, max_w, max_h);

        // add the biggest size if possible
        if(!isExistInSizeArrList(*size_arr, addedCntAfter, max_w, max_h))
        {
            // add the size
            char *temp = (char *)malloc(128);
            if(!temp)
                goto label_fail;

            sprintf(temp, "%dx%d", max_w, max_h);
            MY_LOGD("Add a biggest picture size:%s", temp);
            (*size_arr)[addedCntAfter++] = temp;
        }

        ret = addedCntAfter;
    #endif

    MY_LOGD_IF(getDumpFlag() > 0, "-");
    return ret;

label_fail:
    if(*size_arr) {
        for(i = 0; i < addedCntAfter; ++i) {
            if((*size_arr)[i]) {
                free((*size_arr)[i]);
            }
        }
    }

    if(*size_arr) {
        free(*size_arr);
    }
    MY_LOGD_IF(getDumpFlag() > 0, "Error -");
    return ret;
}

int generatePrvSizeValues(int max_w, int max_h, char ***size_arr, int force_ratio)
{
    double ratio = ((double)max_w)/max_h;
    int ratioInt = getSizeRatio(max_w, max_h);
    int base;
    int i;
    int w, h;
    int ratioW, ratioH;
    int ret = 0;
    int new_ratio;
    int max_count = 4096;        // todo: need modify if max sizes count is bigger than 4096
    int addedCntAfter = 0;

    MY_LOGD_IF(getDumpFlag() > 0, "+");
    *size_arr = NULL;

    // avoid nagative max_w/max_h
    if(max_w <= 0 || max_h <= 0)
    {
        // default: QVGA
        max_w = 640;
        max_h = 480;
    }
    // if width < height, then swap them
    if(max_w < max_h)
    {
        int temp = max_w;
        max_w = max_h;
        max_h = temp;
    }

    if(force_ratio >= SIZE_RATIO_4_3 && force_ratio <= SIZE_RATIO_16_9)
    {
        ratioInt = force_ratio;
    }

label_switch_ratio:
    switch(ratioInt)
    {
        case SIZE_RATIO_4_3:
            base = max_w/16/4;
            ratioW = 4;
            ratioH = 3;
        break;
        case SIZE_RATIO_3_2:
            base = max_w/16/3;
            ratioW = 3;
            ratioH = 2;
        break;
        case SIZE_RATIO_5_3:
            base = max_w/16/5;
            ratioW = 5;
            ratioH = 3;
        break;
        case SIZE_RATIO_16_9:
            base = max_w/16/16;
            ratioW = 16;
            ratioH = 9;
        break;
        default:
            ratioInt = getNearestRatio(max_w, max_h);
            goto label_switch_ratio;
        break;
    }
    *size_arr = (char **)malloc(max_count * sizeof(char *));
    if(!*size_arr)
        goto label_fail;
    // initialize size arr, set as 0
    memset(*size_arr, 0, max_count * sizeof(char *));


    #if 0
    for(i = 1; i <= base; ++i)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;
        w = i * 16 * ratioW;
        h = i * 16 * ratioH;

        sprintf(temp, "%dx%d", w, h);
        (*size_arr)[i - 1] = temp;
        //MY_LOGD("Add one size:%s\n", (*size_arr)[i - 1]);
        ++ret;
    }
    #else
        // by default:
        // (1) add 176x144
        (*size_arr)[addedCntAfter++] = (char *)"176x144";
        // (1) add 352x288
        (*size_arr)[addedCntAfter++] = (char *)"352x288";

        addedCntAfter = add4_3_PrvSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add5_3_PrvSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add3_2_PrvSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add16_9_PrvSizes(*size_arr, addedCntAfter, max_w, max_h);

        // add the biggest size if possible
        if(!isExistInSizeArrList(*size_arr, addedCntAfter, max_w, max_h))
        {
            // add the size
            char *temp = (char *)malloc(128);
            if(!temp)
                goto label_fail;

            sprintf(temp, "%dx%d", max_w, max_h);
            MY_LOGD("Add a biggest preview size:%s", temp);
            (*size_arr)[addedCntAfter++] = temp;
        }

        ret = addedCntAfter;
    #endif

    MY_LOGD_IF(getDumpFlag() > 0, "-");
    return ret;

label_fail:
    if(*size_arr) {
        for(i = 0; i < addedCntAfter; ++i) {
            if((*size_arr)[i]) {
                free((*size_arr)[i]);
            }
        }
    }

    if(*size_arr) {
        free(*size_arr);
    }
    MY_LOGD_IF(getDumpFlag() > 0, "-");
    return ret;
}

int generateVideoSizeValues(int max_w, int max_h, char ***size_arr, int /* force_ratio */)
{
    double ratio = ((double)max_w)/max_h;
    int ratioInt = getSizeRatio(max_w, max_h);
    int i;
    int w, h;
    int ratioW, ratioH;
    int ret = 0;
    int max_count = 4096;        // todo: need modify if max sizes count is bigger than 4096
    int addedCntAfter = 0;

    MY_LOGD_IF(getDumpFlag() > 0, "+");
    *size_arr = NULL;

    // avoid nagative max_w/max_h
    if(max_w <= 0 || max_h <= 0)
    {
        // default: VGA
        max_w = 640;
        max_h = 480;
    }
    // if width < height, then swap them
    if(max_w < max_h)
    {
        int temp = max_w;
        max_w = max_h;
        max_h = temp;
    }

    *size_arr = (char **)malloc(max_count * sizeof(char *));
    if(!*size_arr)
        goto label_fail;
    // initialize size arr, set as 0
    memset(*size_arr, 0, max_count * sizeof(char *));


    // by default:
    // (1) add 176x144
    (*size_arr)[addedCntAfter++] = (char *)"176x144";
    // (2) add 340x240
    (*size_arr)[addedCntAfter++] = (char *)"320x240";
    // (3) add 352x288
    (*size_arr)[addedCntAfter++] = (char *)"352x288";   // VCD
    // (4) add 480x320
    (*size_arr)[addedCntAfter++] = (char *)"480x320";
    // (5) add 640x480
    (*size_arr)[addedCntAfter++] = (char *)"640x480";
    // (5) add 720x480  (for some cases: 480P is 720x480)
    (*size_arr)[addedCntAfter++] = (char *)"720x480";

    // add video sizes when possible
    if(max_w >= 864 && max_h >= 480)
        (*size_arr)[addedCntAfter++] = (char *)"864x480";
    if(max_w >= 1280 && max_h >= 720)
        (*size_arr)[addedCntAfter++] = (char *)"1280x720";
    if(max_w >= 1280 && max_h >= 736)
        (*size_arr)[addedCntAfter++] = (char *)"1280x736";
    // 1080P
    if(max_w >= 1920 && max_h >= 1080)
    {
        (*size_arr)[addedCntAfter++] = (char *)"1920x1080";
        (*size_arr)[addedCntAfter++] = (char *)"1920x1088";
    }
    // when bigger video sizes??
    // for 4K2K
    if(max_w >= 3840 && max_h >= 2160)
        (*size_arr)[addedCntAfter++] = (char *)"3840x2160";


   // add the biggest size if possible
    if(!isExistInSizeArrList(*size_arr, addedCntAfter, max_w, max_h))
    {
        // add the size
        char *temp = (char *)malloc(128);
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", max_w, max_h);
        MY_LOGD("Add a biggest video size:%s", temp);
        (*size_arr)[addedCntAfter++] = temp;
    }

    ret = addedCntAfter;

    MY_LOGD_IF(getDumpFlag() > 0, "-");
    return ret;

label_fail:
    if(*size_arr) {
        for(i = 0; i < addedCntAfter; ++i) {
            if((*size_arr)[i]) {
                free((*size_arr)[i]);
            }
        }
    }

    if(*size_arr) {
        free(*size_arr);
    }
    MY_LOGD_IF(getDumpFlag() > 0, "-");
    return ret;
}


char *getDefaultPicSize()
{
    // for test
    return (char *)"128x128";
}

int parseStrToSize(const char *s, int *w, int *h)
{
    int ret;
    *w = *h = 0;

    ret = sscanf(s, "%dx%d", w, h);
    if(ret == 2)
        goto label_exit;

    ret = sscanf(s, "\"%dx%d\"", w, h);
    if(ret < 2)
    {
        MY_LOGE("size:%s invalid!", s);
        return 0;
    }

label_exit:
    if(ret == 2)
    {
        return 1;
    }

    MY_LOGE("parseStrToSize failed, s:[%s], set default value:640x480", s);
    *w = 640;
    *h = 480;

    return 1;
}


const char **getPicSizes(char const *maxPicSize, int * pCnt, int facing)
{
    static bool HasGetMainPicSizes = false;
    static bool HasGetSubPicSizes = false;

    int max_w, max_h;

    // back facing camera
    if(facing == 0 && !HasGetMainPicSizes) {
        parseStrToSize(maxPicSize, &max_w, &max_h);

        *pCnt = generatePicSizeValues(max_w, max_h, &g_pic_main_arr, 0);

#if 0
        const char **arr = new const char *[4];
        arr[0] = "600x400";
        arr[1] = "1600x1200";
        arr[2] = "800x600";
        arr[3] = "2400x1800";
        *pCnt = 4;
#endif

        HasGetMainPicSizes = true;

        return (const char **)g_pic_main_arr;
    }
    else if(facing == 1 && !HasGetSubPicSizes) {   // front facing camera
        parseStrToSize(maxPicSize, &max_w, &max_h);
        *pCnt = generatePicSizeValues(max_w, max_h, &g_pic_sub_arr, 0);

        HasGetSubPicSizes = true;

        return (const char **)g_pic_sub_arr;
    }

    return NULL;
}

int addExtraPictureSizes( char const * * defaultSizes, int cnt, char const* *extraSizes, int extraSize)
{
    int i = 0;

    MY_LOGI_IF(getDumpFlag() > 0, "extraSize:%d", extraSize);
    while(i < extraSize)
    {
        int sscanf_ret;
        const char * size = extraSizes[i];
        int temp_w, temp_h;

        sscanf_ret = sscanf(size, "%dx%d", &temp_w, &temp_h);
        if(sscanf_ret != 2)
        {
            MY_LOGE("Critical: sscanf error:%s", size);
            goto label_continue;
        }

        if(!isExistInSizeArrList((char **)defaultSizes, cnt, temp_w, temp_h))
        {
            // add the size
            char *temp = (char *)malloc(128);
            if(!temp)
            {
                MY_LOGE("Critical: malloc error:size(128)");
                goto label_continue;
            }

            sprintf(temp, "%dx%d", temp_w, temp_h);
            MY_LOGI_IF(getDumpFlag() > 0, "Add a extra size:%s", temp);
            defaultSizes[cnt++] = temp;
        }

    label_continue:

        ++i;
    }

    return cnt;
}

const char **getPrvSizes(char const *maxPrvSize, int * pCnt, int facing)
{
    static bool HasGetMainPrvSizes = false;
    static bool HasGetSubPrvSizes = false;

    int max_w, max_h;

    // back facing camera
    if(facing == 0 && !HasGetMainPrvSizes) {
        parseStrToSize(maxPrvSize, &max_w, &max_h);
        *pCnt = generatePrvSizeValues(max_w, max_h, &g_prv_main_arr, 0);

#if 0
        const char **arr = new const char *[4];
        arr[0] = "600x400";
        arr[1] = "1600x1200";
        arr[2] = "800x600";
        arr[3] = "2400x1800";
        *pCnt = 4;
#endif

        HasGetMainPrvSizes = true;

        return (const char **)g_prv_main_arr;
    }
    else if(facing == 1 && !HasGetSubPrvSizes) {   // front facing camera
        parseStrToSize(maxPrvSize, &max_w, &max_h);
        *pCnt = generatePrvSizeValues(max_w, max_h, &g_prv_sub_arr, 0);

        HasGetSubPrvSizes = true;

        return (const char **)g_prv_sub_arr;
    }

    return NULL;
}

int addExtraPreviewSizes( char const* * defaultSizes, int cnt, char const* *extraSizes, int extraSize)
{
    int i = 0;

    MY_LOGI_IF(getDumpFlag() > 0, "defaultSize count:%d,extraSize:%d", cnt, extraSize);
    while(i < extraSize)
    {
        int sscanf_ret;
        const char * size = extraSizes[i];
        int temp_w, temp_h;

        sscanf_ret = sscanf(size, "%dx%d", &temp_w, &temp_h);
        if(sscanf_ret != 2)
        {
            MY_LOGE("Critical: sscanf error:%s", size);
            goto label_continue;
        }

        if(!isExistInSizeArrList((char **)defaultSizes, cnt, temp_w, temp_h))
        {
            // add the size
            char *temp = (char *)malloc(128);
            if(!temp)
            {
                MY_LOGE("Critical: malloc error:size(128)");
                goto label_continue;
            }

            sprintf(temp, "%dx%d", temp_w, temp_h);
            MY_LOGI_IF(getDumpFlag() > 0, "Add a extra size:%s", temp);
            defaultSizes[cnt++] = temp;
        }

    label_continue:

        ++i;
    }

    return cnt;
}


const char **getVideoSizes(char const *maxVideoSize, int * pCnt, int facing)
{
    static bool HasGetMainVideoSizes = false;
    static bool HasGetSubVideoSizes = false;

    int max_w, max_h;

    // back facing camera
    if(facing == 0 && !HasGetMainVideoSizes) {
        parseStrToSize(maxVideoSize, &max_w, &max_h);

        *pCnt = generateVideoSizeValues(max_w, max_h, &g_video_main_arr, 0);

        HasGetMainVideoSizes = true;

        return (const char **)g_video_main_arr;
    }
    else if(facing == 1 && !HasGetSubVideoSizes) {   // front facing camera
        parseStrToSize(maxVideoSize, &max_w, &max_h);
        *pCnt = generateVideoSizeValues(max_w, max_h, &g_video_sub_arr, 0);

        HasGetSubVideoSizes = true;

        return (const char **)g_video_sub_arr;
    }

    return NULL;
}

int addExtraVideoSizes( char const* * defaultSizes, int cnt, char const* *extraSizes, int extraSize)
{
    int i = 0;

    MY_LOGI_IF(getDumpFlag() > 0, "extraSize:%d", extraSize);
    while(i < extraSize)
    {
        int sscanf_ret;
        const char * size = extraSizes[i];
        int temp_w, temp_h;

        sscanf_ret = sscanf(size, "%dx%d", &temp_w, &temp_h);
        if(sscanf_ret != 2)
        {
            MY_LOGE("Critical: sscanf error:%s", size);
            goto label_continue;
        }

        if(!isExistInSizeArrList((char **)defaultSizes, cnt, temp_w, temp_h))
        {
            // add the size
            char *temp = (char *)malloc(128);
            if(!temp)
            {
                MY_LOGE("Critical: malloc error:size(128)");
                goto label_continue;
            }

            sprintf(temp, "%dx%d", temp_w, temp_h);
            MY_LOGI_IF(getDumpFlag() > 0, "Add a extra size:%s", temp);
            defaultSizes[cnt++] = temp;
        }

    label_continue:

        ++i;
    }

    return cnt;
}


/*
 { char const* szFType = "default-values"; char const* szFKey = MtkCameraParameters::KEY_PICTURE_SIZE; char const *szDefaultScene_ItemDefault[] = { getDefaultPicSize() }; int picSizesCnt; char const* *szDefaultScene_ItemList = getPicSizes(&picSizesCnt);  SceneKeyedMap sceneKeyedMap(szFType, FeatureInfo( szDefaultScene_ItemDefault[0], szDefaultScene_ItemList, picSizesCnt )); rFMap.add(FKEY_T(szFKey), sceneKeyedMap); do { if ( (0) ) { ((void)__android_log_print(ANDROID_LOG_DEBUG, "MtkCam/ParamsManager", "[%s] ""%s=%s""\r\n", __FUNCTION__, szFKey, sceneKeyedMap.getDefault().getDefaultItem().string())); } }while(0); }    \
*/

        }// v1
    }   // camera
}// android

