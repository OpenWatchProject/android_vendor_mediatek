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
 ************************************************************************************************/
#define LOG_TAG "ResMgr"

#include "Cam_ResMgr.h"
////////////////////////////////////////////////////////////////////////////////

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


ResMgr* ResMgr::Res_Attach(E_USER user,ISP_HW_MODULE master,char Name[32])
{
    ResMgr* ptr = NULL;

    switch(user){
        case user_camio:
            return (ResMgr*)Cam_ResMgr::Res_Attach(master,Name);
            break;
        default:
            BASE_LOG_ERR("unsupported user:%d\n",user);
            return NULL;
            break;
    }

}


ResMgr* ResMgr::Res_Attach(E_USER user,char Name[32],MUINT32 input)
{
    ResMgr* ptr = NULL;

    switch(user){
        case user_camio:
            return (ResMgr*)Cam_ResMgr::Res_Attach(input,Name);
            break;
        default:
            BASE_LOG_ERR("unsupported user:%d\n",user);
            return NULL;
            break;
    }

}

ResMgr* ResMgr::Res_Attach(E_USER user,char Name[32],MUINT32 input, MUINT32 SensorNum)
{
    ResMgr* ptr = NULL;

    switch(user){
        case user_camio:
            return (ResMgr*)Cam_ResMgr::Res_Attach(input, Name, SensorNum);
            break;
        default:
            BASE_LOG_ERR("unsupported user:%d\n", user);
            return NULL;
            break;
    }

}

MBOOL ResMgr::Res_AattachArbitor(MUINT32 index, MUINT32 SensorNum, Res_Arbitor_In* inParams, Res_Arbitor_Out* outParams, char Name[32])
{

    MBOOL       ret = MTRUE, bin_useful, twin_useful;

    MBOOL       _twin, _bin, _thrput;
    MUINT32     thrput_dl = 0, sidx = 0, clk_dl = inParams[index].vRawClks.size();

    BASE_LOG_INF("+, index:%d\n", index);
    if (index >= SensorNum) {
        BASE_LOG_ERR("index:%d >= SensorNum:%d\n", index, SensorNum);
        return MFALSE;
    }

    if (inParams[index].mipi_rate == 1000000) {
        thrput_dl = (MUINT32) (((MUINT64)inParams[index].img_crop_w)*inParams[index].img_crop_h*inParams[index].tg_fps / 10);
    } else {
        thrput_dl = inParams[index].mipi_rate;
    }

    if (thrput_dl == 0) {
        BASE_LOG_ERR("throughput error, fps(x10):%d,size:%d_%d mipi:%d\n",
            inParams[index].raw_fps,inParams[index].img_crop_w,inParams[index].img_crop_h,inParams[index].mipi_rate);
        ret = MFALSE;
        goto EXIT;
    }

    outParams[index].clk_level = 0;
    outParams[index].needFullFpsSVRawDump = MFALSE;
    outParams[index].resmgr_cam = NULL;
    outParams[index].resmgr_sv = NULL;

    if (inParams[index].vRawClks.size()) {
        //attach cam_resMgr, direct-link
        outParams[index].resmgr_cam = (ResMgr*)Cam_ResMgr::Res_Attach(index, SensorNum, inParams, outParams, Name);
        if(outParams[index].resmgr_cam == NULL) {
            return MFALSE;
        }
    }

    twin_useful = outParams[index].isOffTwin ? MFALSE : MTRUE;
    bin_useful  = (inParams[index].off_bin == MTRUE) ? MFALSE : ((inParams[index].raw_out_w <= (inParams[index].img_crop_w >> 1)) ? MTRUE : MFALSE);

    switch (inParams[index].pattern) {
        case eCAM_DUAL_PIX:
        case eCAM_4CELL:
        case eCAM_MONO:
        case eCAM_ZVHDR:
            bin_useful = MFALSE;
            break;
        default:
            break;
    }

    _thrput = thrput_dl;
    _twin   = twin_useful;
    _bin    = bin_useful;
    clk_dl  = 0;
    while ((clk_dl < inParams[index].vRawClks.size()) &&
            IS_THROUGHPUT_OVER(_thrput, inParams[index].vRawClks.at(clk_dl))) {
        if (_twin) {
            _twin = MFALSE;
            _thrput /= 2;
        }
        else if (_bin) {
            _bin = MFALSE;
            _thrput /= 4;
        }
        else {
            clk_dl++;
        }
    }
    outParams[index].clk_level = clk_dl;

EXIT:
    if(inParams[index].vRawClks.size() == outParams[index].clk_level) {
        outParams[index].clk_level--;
        BASE_LOG_ERR("-, ResMgr arb fail!!! cfgIn:mipirate=%d tg_crop=%d_%d tgfps=%d rawfps=%d raw_out_w=%d clk0=%d,off_bin:%d,off_twin:%d pattern=%d\n"
            "cfgOut:dcif=%d clk_lv=%d clk=%d throuput_dl=%d\n",
                inParams[index].mipi_rate,inParams[index].img_crop_w, inParams[index].img_crop_h,inParams[index].tg_fps,inParams[index].raw_fps,
                inParams[index].raw_out_w,inParams[index].vRawClks.at(0),inParams[index].off_bin,inParams[index].off_twin,inParams[index].pattern,
                outParams[index].needFullFpsSVRawDump,outParams[index].clk_level,inParams[index].vRawClks.at(outParams[index].clk_level),thrput_dl);
        ret = MFALSE;
    }
    else
        BASE_LOG_INF("-, cfgIn:mipirate=%d tg_crop=%d_%d tgfps=%d rawfps=%d raw_out_w=%d clk0=%d,off_bin:%d,off_twin:%d pattern=%d\n"
            "cfgOut:dcif=%d clk_lv=%d clk=%d throuput_dl=%d, isOffTwin=%d\n",
                inParams[index].mipi_rate,inParams[index].img_crop_w, inParams[index].img_crop_h,inParams[index].tg_fps,inParams[index].raw_fps,
                inParams[index].raw_out_w,inParams[index].vRawClks.at(0),inParams[index].off_bin,inParams[index].off_twin,inParams[index].pattern,
                outParams[index].needFullFpsSVRawDump,outParams[index].clk_level,inParams[index].vRawClks.at(outParams[index].clk_level),thrput_dl,
                outParams[index].isOffTwin);

    return ret;
}

ISP_HW_MODULE ResMgr::Res_GetModule(void)
{
    return this->m_hwModule;
}


