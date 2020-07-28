/*********************************************************************************************
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
#include "n3d_hal_imp.h"         // For N3D_HAL class.

using namespace StereoHAL;

void
N3D_HAL_IMP::__dumpNVRAM(bool isInput)
{
    if( !IS_DUMP_ENALED ||
        NULL == __spVoidGeoData )
    {
        return;
    }

    string bufferName;
    char dumpPath[PATH_MAX];
    if(isInput) {
        genFileName_VSDOF_BUFFER(dumpPath, PATH_MAX, __dumpHint, "N3D_NVRAM_IN");
    } else {
        genFileName_VSDOF_BUFFER(dumpPath, PATH_MAX, __dumpHint, "N3D_NVRAM_OUT");

        // Extract NVRAM content from N3D
        __n3dKernel.updateNVRAM(__spVoidGeoData);
    }

    FILE *fpNVRAM = NULL;
    fpNVRAM = fopen(dumpPath, "wb");
    if(fpNVRAM) {
        fwrite(__spVoidGeoData->StereoNvramData.StereoData, 1, sizeof(MUINT32)*MTK_STEREO_KERNEL_NVRAM_LENGTH, fpNVRAM);
        fflush(fpNVRAM);
        fclose(fpNVRAM);
        fpNVRAM = NULL;
    } else {
        MY_LOGE("Cannot dump NVRAM to %s, error: %s", dumpPath, strerror(errno));
    }
}

void
N3D_HAL_IMP::__dumpDebugBuffer(MUINT8 *debugBuffer)
{
    if(NULL == debugBuffer ||
       !IS_DUMP_ENALED)
    {
        MY_LOGD("Skip to dump debug buffer");
        return;
    }

    AutoProfileUtil profile(LOG_TAG, "Dump debug buffer");

    const size_t DEBUG_BUFFER_SIZE = (size_t)*((int*)debugBuffer);
    char dumpPath[PATH_MAX];
    genFileName_VSDOF_BUFFER(dumpPath, PATH_MAX, __dumpHint, "N3D_DEBUG_BUFFER");

    FILE *fp = fopen(dumpPath, "wb");
    if(fp) {
        fwrite(debugBuffer, 1, DEBUG_BUFFER_SIZE, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot open %s", dumpPath);
    }
}

void
N3D_HAL_IMP::__dumpLDC()
{
    if(!StereoSettingProvider::LDCEnabled() ||
       StereoSettingProvider::getLDCTable().size() <= 1 ||
       !IS_DUMP_ENALED)
    {
        return;
    }

    if(__ldcString.size() < 1) {
        //Prepare dump string
        float *element = &(StereoSettingProvider::getLDCTable()[0]);
        std::ostringstream oss;
        oss.precision(10);
        oss << std::fixed;

        for(int k = 0; k < 2; k++) {
            int line = (int)*element++;
            int sizePerLine = (int)*element++;
            oss << line << " " << sizePerLine << endl;

            for(int row = 1; row <= line; row++) {
                for(int col = 0; col < sizePerLine; col++) {
                    oss << *element++ << " ";
                }
                oss << (int)*element++ << " ";
                oss << (int)*element++ << endl;
            }
        }

        __ldcString = oss.str();
    }

    char dumpPath[PATH_MAX];
    genFileName_VSDOF_BUFFER(dumpPath, PATH_MAX, __dumpHint, "N3D_LDC");
    FILE *fp = fopen(dumpPath, "wb");
    if(fp) {
        fwrite(__ldcString.c_str(), 1, __ldcString.size(), fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot open %s", dumpPath);
    }
}