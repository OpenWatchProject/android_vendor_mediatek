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

#include <iostream>
#include <sstream>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CDN_HAL"

#include "cdn_hal_imp.h"
#include <mtkcam/utils/std/Log.h>

using namespace StereoHAL;


static const char* sCNDLogName = "CDNLog";

template<typename T, size_t arrSize, typename = std::enable_if_t<!std::is_same<T, char>::value>>
std::ostream& operator<<(std::ostream& stream, const T (&array)[arrSize])
{
    stream << array[0];
    for(size_t i = 1; i < arrSize; i++)
    {
        stream << ", " << array[i];
    }
    return stream;
}

void
CDN_HAL_IMP::__logParams()
{
    if(!LOG_ENABLED) return;

    std::ostringstream stringStream;
    stringStream << "************************************" << std::endl;
    stringStream << "Name: Params" << std::endl;
    stringStream << "************************************" << std::endl;
    stringStream << "id: " << __runtimeParams.id << std::endl;
    stringStream << "isRotate: " << __runtimeParams.isRotate << std::endl;
    stringStream << "mode: " << __runtimeParams.mode << std::endl;
    stringStream << "var: " << __runtimeParams.var << std::endl;
    stringStream << "Trans: " << __runtimeParams.Trans << std::endl;
    stringStream << "width: " << __runtimeParams.width << std::endl;
    stringStream << "height: " << __runtimeParams.height << std::endl;
    stringStream << "************************************" << std::endl;
    __android_log_print(ANDROID_LOG_DEBUG, sCNDLogName, "%s", stringStream.str().c_str());
}

void
CDN_HAL_IMP::__logCheckProcess()
{
    if(!LOG_ENABLED) return;

    std::ostringstream stringStream;
    stringStream << "************************************" << std::endl;
    stringStream << "Name: CDNCheckProcess" << std::endl;
    stringStream << "************************************" << std::endl;
    stringStream << "width: " << __runntimeCheckProcess.width << std::endl;
    stringStream << "height: " << __runntimeCheckProcess.height << std::endl;
    stringStream << "isRotate: " << __runntimeCheckProcess.isRotate << std::endl;
    stringStream << "Trans: " << __runntimeCheckProcess.Trans << std::endl;;
    stringStream << "CInfo: " << __runntimeCheckProcess.CInfo << std::endl;
    stringStream << "SInfo: " << __runntimeCheckProcess.SInfo << std::endl;
    stringStream << "BOUNDARY_THR: " << __runntimeCheckProcess.BOUNDARY_THR << std::endl;
    stringStream << "VAR_THR: " << __runntimeCheckProcess.VAR_THR << std::endl;
    stringStream << "MACRO_BOUND_THR: " << __runntimeCheckProcess.MACRO_BOUND_THR << std::endl;
    stringStream << "************************************" << std::endl;
    __android_log_print(ANDROID_LOG_DEBUG, sCNDLogName, "%s", stringStream.str().c_str());
}

void
CDN_HAL_IMP::__logImages()
{
    if(!LOG_ENABLED) return;

    std::ostringstream stringStream;
    stringStream << "************************************" << std::endl;
    stringStream << "Name: Images" << std::endl;
    stringStream << "************************************" << std::endl;
    stringStream << "width: " << __runtimeImages.width << std::endl;
    stringStream << "height: " << __runtimeImages.height << std::endl;
    stringStream << "MonoYUV: " << (void*)__runtimeImages.MonoYUV << std::endl;
    stringStream << "BayerYUV: " << (void*)__runtimeImages.BayerYUV << std::endl;
    stringStream << "output: " << (void*)__runtimeImages.output[0] << ", ";
    stringStream << (void*)__runtimeImages.output[1] << ", ";
    stringStream << (void*)__runtimeImages.output[2] << std::endl;
    stringStream << "************************************" << std::endl;
    __android_log_print(ANDROID_LOG_DEBUG, sCNDLogName, "%s", stringStream.str().c_str());
}