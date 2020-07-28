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
#ifndef SW_BOKEH_TUNING_H_
#define SW_BOKEH_TUNING_H_

#include "stereo_tuning_base.h"
#include <stereo_tuning_provider.h>

class SW_BOKEHTuning : public StereoTuningBase
{
public:
    SW_BOKEHTuning(json &tuningJson);
    virtual ~SW_BOKEHTuning();

    virtual const char *getTuningKey() { return STEREO_TUNING_NAME[E_TUNING_SW_BOKEH]; }

    virtual bool retrieveTuningParams(TuningQuery_T &query);
    virtual void log(FastLogger &logger, bool inJSON=false);

protected:
    virtual void _initDefaultValues();
    virtual void _loadValuesFromDocument(const json &jsonObj);

private:
    MUINT32 __getDofValue(MUINT32 DofLevel, vector<MUINT32> &table);

private:
    QUERY_INT_T         __galleryTuning;
    QUERY_INT_T         __cameraTuning;
    QUERY_INT_LIST_T    __galleryClearTable;
    QUERY_INT_LIST_T    __cameraClearTable;
    vector<MUINT32>     __galleryDofTable;
    vector<MUINT32>     __cameraDofTable;

    const QUERY_INT_LIST_T __DEFAULT_CLEAR_TABLE = {3, 3, 3, 4, 6, 8, 12, 16, 20, 24, 28, 32, 32, 32, 32, 32, 32};
};

#endif