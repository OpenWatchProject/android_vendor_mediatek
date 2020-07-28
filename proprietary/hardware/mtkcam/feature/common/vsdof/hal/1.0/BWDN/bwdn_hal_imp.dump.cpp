/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "BWDN_HAL"

#include <sys/stat.h>

#include "bwdn_hal_imp.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include "../inc/stereo_dp_util.h"
#include <vsdof/hal/ProfileUtil.h>

using namespace StereoHAL;

char *
BWDN_HAL_IMP::__getDumpFolderName(int folderNumber, char path[])
{
    const char *DIRECTION = (StereoSettingProvider::stereoProfile() == STEREO_SENSOR_PROFILE_FRONT_FRONT) ? "Front" : "Rear";
    sprintf(path, "/sdcard/BWDN/%s/%d/", DIRECTION, folderNumber);
    return path;
}

void
BWDN_HAL_IMP::__mkdir()
{
    if(!DUMP_BUFFER) {
        return;
    }

    static int dumpNumber = 0;
    __requestNumber = dumpNumber++;
    char path[PATH_MAX+1] = {0};
    __getDumpFolderName(__requestNumber, path);
    #define DELIM "/"

    struct stat st;
    ::memset(&st, 0, sizeof(struct stat));
    if(stat(path, &st) == -1) {
        char *folder = strtok(path, DELIM);
        char createPath[PATH_MAX+1] = {0};
        createPath[0] = '/';
        while(folder) {
            strcat(createPath, folder);
            if (::mkdir (createPath, 0755) != 0 && errno != EEXIST) {
                MY_LOGE("Create %s failed, error: %s", createPath, strerror(errno));
                break;
            }

            folder = strtok(NULL, DELIM);
            strcat(createPath, DELIM);
        }
    }
}

void
BWDN_HAL_IMP::__dumpInputImage(BWDN_HAL_PARAMS &in)
{
    if(!DUMP_BUFFER) {
        return;
    }

    char fileName[PATH_MAX+1];
    void *prVA = NULL;

    //MonoProcessed__4208x2368_12_0.raw
    sprintf(fileName, "MonoProcessed__%dx%d_12_0.raw", __monoProcessedRawImage.i4Width, __monoProcessedRawImage.i4Height);
    __lockAHardwareBuffer(in.monoProcessedRaw, prVA);
    __dumpImage(fileName, prVA, __monoProcessedRawImage.i4Size);
    __unlockAHardwareBuffer(in.monoProcessedRaw);

    //BayerProcessed__4208x2368_12_0.raw
    sprintf(fileName, "BayerProcessed__%dx%d_12_0.raw", __bayerProcessedRawImage.i4Width, __bayerProcessedRawImage.i4Height);
    __lockAHardwareBuffer(in.bayerProcessedRaw, prVA);
    __dumpImage(fileName, prVA, __bayerProcessedRawImage.i4Size);
    __unlockAHardwareBuffer(in.bayerProcessedRaw);

    //BayerW__4208x2368_8.raw
    sprintf(fileName, "BayerW__%dx%d_8.raw", __bayerWImage.i4Width, __bayerWImage.i4Height);
    __lockAHardwareBuffer(in.bayerW, prVA);
    __dumpImage(fileName, prVA, __bayerWImage.i4Size);
    __unlockAHardwareBuffer(in.bayerW);

    //Depth__480x272_16_1_544x288_16.raw
    const StereoArea BASE_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_DMP_H, eSTEREO_SCENARIO_CAPTURE);
    const MSize CONTENT_SIZE = BASE_SIZE.contentSize();
    sprintf(fileName, "Depth__%dx%d_16_1_%dx%d_16.raw", __depthImage.i4Width, __depthImage.i4Height,
            __depthImage.i4Pitch, __depthImage.i4Size/__depthImage.i4Pitch/2);
    __lockAHardwareBuffer(in.depth, prVA);
    __dumpImage(fileName, prVA, __depthImage.i4Size);
    __unlockAHardwareBuffer(in.depth);
}

void
BWDN_HAL_IMP::__dumpOutputImage(BWDN_HAL_OUTPUT &out)
{
    if(!DUMP_BUFFER) {
        return;
    }

    char fileName[PATH_MAX+1];
    void *prVA = NULL;

    //S_Output__4208x2368_14_0.raw
    sprintf(fileName, "S_Output__%dx%d_14_0.raw", __outputImage.i4Width, __outputImage.i4Height);
    __lockAHardwareBuffer(out.outBuffer, prVA);
    __dumpImage(fileName, prVA, __outputImage.i4Size);
    __unlockAHardwareBuffer(out.outBuffer);
}

void
BWDN_HAL_IMP::__dumpImage(char *fileName, void *data, size_t size)
{
    if(!DUMP_BUFFER) {
        return;
    }

    char path[PATH_MAX+1];
    char folder[PATH_MAX+1] = {0};
    __getDumpFolderName(__requestNumber, folder);

    sprintf(path, "%s%s", folder, fileName);
    FILE * pFile = fopen( path , "wb" );
    size_t wroteSize = fwrite(data, 1, size, pFile);
    fflush(pFile);
    fclose (pFile);

    if(wroteSize != size) {
        MY_LOGE("Write %s failed, write size %zu, expect %zu", path, wroteSize, size);
    }
}