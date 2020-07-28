/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 **************************************************/
#ifndef _CAM_CAPIBILITY_H_
#define _CAM_CAPIBILITY_H_

#include <vector>
#include <mtkcam/def/ImageFormat.h>
#include <Cam/ICam_capibility.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

typedef struct _tXmx{
    MUINT32 sram_dmx;
    MUINT32 sram_bmx;
    MUINT32 sram_amx;
    MUINT32 sram_rmx;
    MUINT32 sram_pmx;

    MUINT32 line_dmx;
    MUINT32 line_bmx;
    MUINT32 line_amx;
    MUINT32 line_rmx;
    MUINT32 line_pmx;
    _tXmx(
        MUINT32 dmx = 0,
        MUINT32 bmx = 0,
        MUINT32 amx = 0,
        MUINT32 rmx = 0,
        MUINT32 pmx = 0
        )
        :sram_dmx(dmx)
        ,sram_bmx(bmx)
        ,sram_amx(amx)
        ,sram_rmx(rmx)
        ,sram_pmx(pmx)
        ,line_dmx(dmx)
        ,line_bmx(bmx)
        ,line_amx(amx)
        ,line_rmx(rmx)
        ,line_pmx(pmx)
        {}
}tXmx;

typedef struct _tCAM_rst{
    MUINT32 ratio;      //unit:%
    std::vector<NSCam::EImageFormat> Queue_fmt;
    MUINT32 pipelinebitdepth;
    MUINT32 pipeSize;
    MUINT32 bs_max_size;
    tXmx    xmx;
    MUINT32 HeaderSize; //unit:byte

    MUINT32 x_pix;                  //horizontal resolution, unit:pix
    MUINT32 xsize_byte;
    MUINT32 crop_x;                 //crop start point-x , unit:pix
    MUINT32 stride_pix;             //stride, uint:pix. this is a approximative value under pak mode
    MUINT32 stride_byte;            //stride, uint:byte

    MBOOL   D_TWIN;                 // 1: dynamic twin is ON, 0: dynamic twin is OFF.
    MUINT32 uni_num;                //the number of uni
    _tCAM_rst(
        MUINT32 _ratio = 100,
        MUINT32 _pipelinebitdepth = 1,
        MUINT32 _pipesize = 1,
        MUINT32 _bs_max_size = 1
        )
        :ratio(_ratio)
        ,pipelinebitdepth(_pipelinebitdepth)
        ,pipeSize(_pipesize)
        ,bs_max_size(_bs_max_size)
        ,HeaderSize(0)
        ,x_pix(0)
        ,xsize_byte(0)
        ,crop_x(0)
        ,stride_pix(0)
        ,stride_byte(0)
        ,D_TWIN(0)
        ,uni_num(1)
    {
        Queue_fmt.clear();
    }
}tCAM_rst;

typedef enum{
    E_CAM_UNKNOWNN      = 0x0,

    E_CAM_pipeline_size = 0x20,
    E_CAM_BS_Max_size   = 0x40,
    E_CAM_HEADER_size   = 0x80,

}E_CAM_Query_OP;

class capibility
{
public:
    capibility(){};
    ~capibility(){};

    MBOOL GetCapibility(MUINT32 portId,E_CAM_Query e_Op,CAM_Queryrst &QueryRst);

    MBOOL GetCapibility(MUINT32,
                        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd,
                        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo,
                        tCAM_rst &,E_CAM_Query_OP) {return MFALSE;};

private:
            MUINT32 GetFormat(MUINT32 portID, CAM_Queryrst &QueryRst);
            MUINT32 GetRatio(MUINT32 portID);
            MUINT32 GetPipelineBitdepth();

};



// 0 : cq using legacy control method. (cq r all using continuous mode)
// 1 : cq using immediate control method. (slave cam using immediate mode)
#define CQ_CTRL_METHOD   0

#endif
