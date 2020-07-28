/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_UTILS_DEBUG_COMMANDTABLE_H_
#define _MTK_HARDWARE_MTKCAM_UTILS_DEBUG_COMMANDTABLE_H_

#include <map>
#include <string>

namespace {
static auto& getDebuggableMap()
{
    static const std::map<std::string, std::string>
    sInst = {
        /**********************************************************************
         *  utils/
         **********************************************************************/
        {// debug
            "debug",
            "--module debug [--backtrace] \n"
            "       --backtrace: dump the currnet backtrace of this process.\n"
        },
        {// imgbuf: image buffer heap
            "NSCam::IImageBufferHeapManager",
            "--module NSCam::IImageBufferHeapManager [options] \n"
            "       Print debug information of image buffer heap.\n"
        },
        {// imgbuf: ion devices
            "NSCam::IIonDeviceProvider",
            "--module NSCam::IIonDeviceProvider [options] \n"
            "       Print debug information of ion devices.\n"
        },
        {// imgbuf: ion image buffer heap allocator
            "NSCam::IIonImageBufferHeapAllocator",
            "--module NSCam::IIonImageBufferHeapAllocator [options] \n"
            "       Print debug information of ion image buffer heap allocator.\n"
        },
        {// IGenericContainerManager
            "NSCam::IGenericContainerManager",
            "--module NSCam::IGenericContainerManager [options] \n"
            "       Print debug information of IGenericContainerManager.\n"
        },
        {// ulog: unified log flush
            "NSCam::ULog",
            "--module NSCam::ULog [options] \n"
            "       Flush the buffer in ULog.\n"
        },

        /**********************************************************************
         *  main/
         **********************************************************************/
        {// HAL3: camera device
            "android.hardware.camera.device@3.5::ICameraDevice",
            "--module android.hardware.camera.device@3.5::ICameraDevice [options] \n"
            "       Print debug information of camera device HAL3.\n"
        },
        {// HAL3: app stream manager
            "NSCam::v3::IAppStreamManager",
            "--module NSCam::v3::IAppStreamManager [options] \n"
            "       Print debug information of IAppStreamManager.\n"
        },

        /**********************************************************************
         *  middleware/
         **********************************************************************/

        /**********************************************************************
         *  pipeline
         **********************************************************************/
        {// IPipelineModelManager
            "NSCam::v3::pipeline::model::IPipelineModelManager",
            "--module NSCam::v3::pipeline::model::IPipelineModelManager [options] \n"
            "       Print debug information of IPipelineModelManager.\n"
        },

        {// IPipelineContextManager
            "NSCam::v3::NSPipelineContext::IPipelineContextManager",
            "--module NSCam::v3::NSPipelineContext::IPipelineContextManager [options] \n"
            "       Print debug information of IPipelineContextManager.\n"
        },

        {// IStreamInfoPluginManager
            "NSCam::v3::IStreamInfoPluginManager",
            "--module NSCam::v3::IStreamInfoPluginManager [options] \n"
            "       Print debug information of IStreamInfoPluginManager.\n"
        },

        {// IFrameBufferManager
            "NSCam::v3::NSPipelineContext::IFrameBufferManager",
            "--module NSCam::v3::NSPipelineContext::IFrameBufferManager [options] \n"
            "       Print debug information of IFrameBufferManager.\n"
        },

        {// IZslProcessor
            "NSCam::v3::pipeline::model::IZslProcessor",
            "--module NSCam::v3::pipeline::model::IZslProcessor [options] \n"
            "       Print debug information of IZslProcessor.\n"
        },

        {// CaptureFeaturePipe
            "NSCam::NSCamFeature::NSFeaturePipe::NSCapture::CaptureFeaturePipe",
            "--module NSCam::NSCamFeature::NSFeaturePipe::NSCapture::CaptureFeaturePipe [options] \n"
            "       Print debug information of CaptureFeaturePipe.\n"
        },

        {// FeaturePipeDebugee
            "NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeDebugee",
            "--module NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeDebugee [options] \n"
            "       Print debug information of FeaturePipeDebugee (FeaturePipe Node Status) .\n"
        },

        /**********************************************************************
         *  aaa
         **********************************************************************/

        /**********************************************************************
         *  drv
         **********************************************************************/

    };
    //
    return sInst;
}
};//namespace
#endif  //_MTK_HARDWARE_MTKCAM_UTILS_DEBUG_COMMANDTABLE_H_

