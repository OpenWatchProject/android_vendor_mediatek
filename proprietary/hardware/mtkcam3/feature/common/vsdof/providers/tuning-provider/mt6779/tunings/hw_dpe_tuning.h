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
#ifndef HW_DPE_TUNING_H_
#define HW_DPE_TUNING_H_

#include "stereo_tuning_base.h"
#include "hw_dpe_dvp_ctrl_tuning.h"
#include "hw_dpe_dvs_ctrl_tuning.h"
#include "hw_dpe_dvs_me_tuning.h"
#include "hw_dpe_dvs_occ_tuning.h"

class HW_DPETuning : public StereoTuningBase
{
public:
    HW_DPETuning(json &tuningJson);
    virtual ~HW_DPETuning() {}

    virtual const char *getTuningKey() { return STEREO_TUNING_NAME[E_TUNING_HW_DPE]; }

    virtual bool retrieveTuningParams(TuningQuery_T &query);
    virtual void log(FastLogger &logger, bool inJSON=false);

protected:
    virtual void _init(const json &tuningJson __attribute__((unused))) {}
    virtual void _initDefaultValues() {}
    virtual void _loadValuesFromDocument(const json &jsonObj __attribute__((unused))) {}

private:
    HW_DPE_DVP_CTRL_Tuning  __dvpCtrlTuning;
    HW_DPE_DVS_CTRL_Tuning  __dvsCtrlTuning;
    HW_DPE_DVS_ME_Tuning    __dvsMeTuning;
    HW_DPE_DVS_OCC_Tuning   __dvsOccTuning;
};

#endif