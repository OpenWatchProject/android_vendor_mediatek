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
#define LOG_TAG "BOKEH_HAL"

#include "bokeh_hal_imp.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <vsdof/hal/ProfileUtil.h>
#include <vsdof/hal/rapidjson/prettywriter.h> // for stringify JSON
#include <vsdof/hal/rapidjson/filewritestream.h>
#include <vsdof/hal/rapidjson/writer.h>

void
BOKEH_HAL_IMP::__dumpInitData()
{
    if(!DUMP_ENABLED) {
        return;
    }
}

void
BOKEH_HAL_IMP::__dumpExtraData(Document &document)
{
    if(!DUMP_ENABLED &&
       checkStereoProperty("vendor.STEREO.dump_json") == 0)
    {
        return;
    }

    char fileName[PATH_MAX+1];
    sprintf(fileName, "%s/ExtraData.json", StereoSettingProvider::getDumpFolder(true, __timestamp, __requestNumber).c_str());
    MY_LOGD("Write extra data to %s", fileName);
    FILE *fp = fopen(fileName, "wb");
    if(fp) {
        char writeBuffer[65536];
        FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        PrettyWriter<FileWriteStream> prettyWriter(os);
        document.Accept(prettyWriter);

        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Fail to open %s", fileName);
    }
}

void
BOKEH_HAL_IMP::__dumpImageData(BOKEH_HAL_PARAMS &param)
{
    if(!DUMP_ENABLED) {
        return;
    }

    //Dump clean image
    char dumpPath[PATH_MAX+1];
    const char *folder = StereoSettingProvider::getDumpFolder(true, __timestamp, __requestNumber).c_str();

    MSize inputSize = param.cleanImage->getImgSize();
    sprintf(dumpPath, "%s/CleanImage_%dx%d_NV21.yuv", folder, inputSize.w, inputSize.h);
    param.cleanImage->saveToFile(dumpPath);

    sprintf(dumpPath, "%s/CleanImage_%dx%d_YV12_workingImage.yuv", folder, __imgInfo.TargetWidth, __imgInfo.TargetHeight);
    __workingImage->saveToFile(dumpPath);

    sprintf(dumpPath, "%s/CleanImage_%dx%d_YV12_TargetImg.yuv", folder, __imgInfo.TargetWidth, __imgInfo.TargetHeight);
    FILE *fp = fopen(dumpPath, "wb");
    if(fp) {
        fwrite(__imgInfo.TargetImgAddr, 1, __imgInfo.TargetWidth*__imgInfo.TargetHeight*3/2, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot open %s", dumpPath);
    }

    //Dump depth buffer
    MSize depthSize = DEPTH_SIZE;
    if( 90 == __capOrientation ||
       270 == __capOrientation)
    {
        depthSize = MSize(DEPTH_SIZE.h, DEPTH_SIZE.w);
    }
    sprintf(dumpPath, "%s/DepthBuffer_%dx%dx4.yuv", folder, depthSize.w, depthSize.h);
    fp = fopen(dumpPath, "wb");
    if(fp) {
        fwrite(__imgInfo.DepthBufferAddr, 1, __imgInfo.DepthBufferSize, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot open %s", dumpPath);
    }

    //Dump split depth buffer
    const char *FILENAME_TEMPLATE[] = {
        "%s/WMI_%dx%d.yuv",
        "%s/NOC_%dx%d.yuv",
        "%s/VAR_%dx%d.yuv",
        "%s/OCC_%dx%d.yuv",
    };

    size_t PER_BUFFER_SIZE = DEPTH_SIZE.w * DEPTH_SIZE.h;
    MUINT8 *src = __imgInfo.DepthBufferAddr;
    for(int i = 0; i < 4; i++, src += PER_BUFFER_SIZE) {
        sprintf(dumpPath, FILENAME_TEMPLATE[i], folder, depthSize.w, depthSize.h);
        fp = fopen(dumpPath, "wb");
        if(fp) {
            fwrite(src, 1, PER_BUFFER_SIZE, fp);
            fflush(fp);
            fclose(fp);
        } else {
            MY_LOGE("Cannot open %s", dumpPath);
        }
    }
}

void
BOKEH_HAL_IMP::__dumpResult(BOKEH_HAL_OUTPUT &output)
{
    if(!DUMP_ENABLED) {
        return;
    }

    //Dump bokeh image
    char dumpPath[PATH_MAX+1];
    const char *folder = StereoSettingProvider::getDumpFolder(true, __timestamp, __requestNumber).c_str();

    sprintf(dumpPath, "%s/BokehResult_%dx%d_YV12_workingImage.yuv", folder, __imgInfo.TargetWidth, __imgInfo.TargetHeight);
    __workingImage->saveToFile(dumpPath);

    sprintf(dumpPath, "%s/BokehResult_%dx%d_YV12.yuv", folder, __resultInfo.RefocusImageWidth, __resultInfo.RefocusImageHeight);
    FILE *fp = fopen(dumpPath, "wb");
    if(fp) {
        size_t fileSize = __resultInfo.RefocusImageWidth * __resultInfo.RefocusImageHeight * 3/2;
        fwrite(__resultInfo.RefocusedYUVImageAddr, 1, fileSize, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot open %s", dumpPath);
    }

    MSize outputSize = output.bokehImage->getImgSize();
    sprintf(dumpPath, "%s/BokehImage_%dx%d_NV21.yuv", folder, outputSize.w, outputSize.h);
    output.bokehImage->saveToFile(dumpPath);
}
