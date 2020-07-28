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
#include "Camsv_ResMgr.h"

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
    switch(user){
    case user_camio:
        return (ResMgr*)Cam_ResMgr::Res_Attach(master,Name);
    case user_camsvio:
    case user_normalsvio:
        return (ResMgr*)Camsv_ResMgr::Res_Attach(master,Name);
    default:
        BASE_LOG_ERR("unsupported user:%d\n",user);
        return NULL;
    }

}


ResMgr* ResMgr::Res_Attach(E_USER user,char Name[32],MUINT32 input)
{
    switch(user){
    case user_camio:
        return (ResMgr*)Cam_ResMgr::Res_Attach(input,Name);
    case user_camsvio:
    case user_normalsvio:
    default:
        BASE_LOG_ERR("unsupported user:%d\n",user);
        return NULL;
    }

}

ResMgr* ResMgr::Res_Attach(E_USER user, MUINT32 mMipiPixrate, char Name[32], vector<E_INPUT>& InOutPut)
{
    switch (user) {
    case user_camsvio:
    case user_normalsvio:
        return (ResMgr*)Camsv_ResMgr::Res_Attach(mMipiPixrate, Name, InOutPut);
    case user_camio:
    default:
        BASE_LOG_ERR("unsupported user:%d\n",user);
        return NULL;
    }
}


MBOOL ResMgr::Res_AattachArbitor(Res_Arbitor_In& inParams, Res_Arbitor_Out &outParam, char Name[32])
{
    MBOOL       ret = MTRUE, bin_useful, dcif_useful, twin_useful;
    MBOOL       _twin, _bin, _dcif, _thrput;
    MUINT32     thrput_dl = 0, thrput_raw = 0, sidx = 0, clk_dl = inParams.vRawClks.size(), clk_dc = inParams.vRawClks.size();

    if (inParams.mipi_rate == 1000000) {
        thrput_dl = (MUINT32) (((MUINT64)inParams.img_crop_w)*inParams.img_crop_h*inParams.tg_fps / 10);
    }
    else {
        thrput_dl = inParams.mipi_rate;
    }
    if (thrput_dl == 0) {
        BASE_LOG_ERR("throughput error, fps(x10):%d,size:%d_%d mipi:%d\n",
            inParams.raw_fps,inParams.img_crop_w,inParams.img_crop_h,inParams.mipi_rate);
        ret = MFALSE;
        goto EXIT;
    }
    thrput_raw = (MUINT32) (((MUINT64)inParams.img_crop_w)*inParams.img_crop_h*inParams.raw_fps / 10);

    outParam.clk_level = 0;
    outParam.needFullFpsSVRawDump = MFALSE;
    outParam.resmgr_cam = NULL;
    outParam.resmgr_sv = NULL;

    // need to pass out the isOffTwin after Cam_ResMgr::Res_Attach result for queryIQTable.
    // default MTRUE; if twin is enabled, set it to MFALSE.
    outParam.isOffTwin = MTRUE;

    twin_useful = inParams.off_twin ? MFALSE : MTRUE;
    dcif_useful = ((inParams.img_crop_w * inParams.img_crop_h) <= MAX_DCIF_RESOLUTION) ? MTRUE : MFALSE;
    bin_useful  = (inParams.off_bin == MTRUE) ? MFALSE : ((inParams.raw_out_w <= (inParams.img_crop_w >> 1)) ? MTRUE : MFALSE);

    switch (inParams.pattern) {
    case eCAM_DUAL_PIX:
    case eCAM_4CELL:
    case eCAM_MONO:
    case eCAM_ZVHDR:
        bin_useful = MFALSE;
        break;
    #if 0
    case eCAM_4CELL_HDR_LSSL: // 4cell XHDR
        bin_useful = MFALSE;
        break;
    #endif
    default:
        break;
    }

    _thrput = thrput_dl;
    _twin   = twin_useful;
    _bin    = bin_useful;
    _dcif   = dcif_useful;
    clk_dl  = 0;
    while ((clk_dl < inParams.vRawClks.size()) && IS_THROUGHPUT_OVER(_thrput, inParams.vRawClks.at(clk_dl))) {
        if (_twin) {
            outParam.isOffTwin = MFALSE;
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

    if (clk_dl < inParams.vRawClks.size()) { // clk_dl == 0
        //attach cam_resMgr, direct-link
        outParam.resmgr_cam = (ResMgr*)Cam_ResMgr::Res_Attach(inParams.img_crop_w, Name);
    }
    else if (dcif_useful) {
        outParam.isOffTwin = MTRUE;
        _thrput = thrput_raw;
        _twin   = twin_useful;
        _bin    = bin_useful;
        _dcif   = dcif_useful;
        clk_dc = 0;
        while (IS_THROUGHPUT_OVER(_thrput, inParams.vRawClks.at(clk_dc)) && (clk_dc < inParams.vRawClks.size())) {
            if (_twin) {
                outParam.isOffTwin = MFALSE;
                _twin = MFALSE;
                _thrput /= 2;
            }
            else if (_bin) {
                _bin = MFALSE;
                _thrput /= 4;
            }
            else {
                clk_dc++;
            }
        }

        if (clk_dc < inParams.vRawClks.size()) /* (clk_dc < clk_dl) */ {
            //**********************************
            //attach cam_resMgr & camsv_resMgr, direct-couple
            //**********************************


            vector<E_INPUT>     resultTG;
            resultTG.push_back(TG_CAMSV_0);
            outParam.resmgr_sv = (ResMgr*)Camsv_ResMgr::Res_Attach(thrput_dl, Name, resultTG);
            outParam.resmgr_cam = (ResMgr*)Cam_ResMgr::Res_Attach(inParams.img_crop_w, Name);

            outParam.needFullFpsSVRawDump = MTRUE;
        }
    }

    outParam.clk_level = (outParam.needFullFpsSVRawDump == MTRUE) ? clk_dc : clk_dl;

EXIT:
    if(inParams.vRawClks.size() == outParam.clk_level) {
        outParam.clk_level--;
        BASE_LOG_ERR("ResMgr arb fail!!! cfgIn:mipirate=%d tg_crop=%d_%d tgfps=%d rawfps=%d raw_out_w=%d clk0=%d,off_bin:%d,off_twin:%d pattern=%d\n"
            "cfgOut:dcif=%d clk_lv=%d clk=%d throuput_dl/raw=%d/%d\n",
                inParams.mipi_rate,inParams.img_crop_w, inParams.img_crop_h,inParams.tg_fps,inParams.raw_fps,
                inParams.raw_out_w,inParams.vRawClks.at(0),inParams.off_bin,inParams.off_twin,inParams.pattern,
                outParam.needFullFpsSVRawDump,outParam.clk_level,inParams.vRawClks.at(outParam.clk_level),thrput_dl,thrput_raw);
        ret = MFALSE;
    }
    else
        BASE_LOG_INF("cfgIn:mipirate=%d tg_crop=%d_%d tgfps=%d rawfps=%d raw_out_w=%d clk0=%d,off_bin:%d,off_twin:%d pattern=%d\n"
            "cfgOut:dcif=%d clk_lv=%d clk=%d throuput_dl/raw=%d/%d\n",
                inParams.mipi_rate,inParams.img_crop_w, inParams.img_crop_h,inParams.tg_fps,inParams.raw_fps,
                inParams.raw_out_w,inParams.vRawClks.at(0),inParams.off_bin,inParams.off_twin,inParams.pattern,
                outParam.needFullFpsSVRawDump,outParam.clk_level,inParams.vRawClks.at(outParam.clk_level),thrput_dl,thrput_raw);
    return ret;
}

ISP_HW_MODULE ResMgr::Res_GetModule(void)
{
    return this->m_hwModule;
}


