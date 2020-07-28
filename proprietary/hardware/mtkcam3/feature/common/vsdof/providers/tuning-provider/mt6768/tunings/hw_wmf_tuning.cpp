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
#define LOG_TAG "StereoTuningProvider_HWWMF"

#include "hw_wmf_tuning.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

const char *QUERY_KEY_WMF_ROUND_NAME[] =
{
    "WMF",
    "Hole Filling 1",
    "Hole Filling 2"
};

HW_WMFTuning::HW_WMFTuning(json &tuningJson)
{
    for(int i = 0; i < TOTAL_WMF_ROUND; ++i) {
        __tblis[i] = new TABLE_T[TBLI_SIZE];
    }

    _init(tuningJson);
}

HW_WMFTuning::~HW_WMFTuning()
{
    for(int i = 0; i < TOTAL_WMF_ROUND; ++i) {
        delete [] __tblis[i];
        __tblis[i] = NULL;
    }
}

bool
HW_WMFTuning::retrieveTuningParams(TuningQuery_T &query)
{
    NSCam::NSIoPipe::WMFECtrl *ctrl = (NSCam::NSIoPipe::WMFECtrl *)query.results[QUERY_KEY_WMFECTRL];
    void *table =  query.results[QUERY_KEY_WMF_TABLE];

    if(NULL == ctrl ||
       NULL == table)
    {
        MY_LOGE("Cannot get WMFECtrl(%p) or table(%p)", ctrl, table);
        return false;
    }

    int queryRound = query.intParams[QUERY_KEY_WMF_ROUND];
    ::memcpy(ctrl, &__ctrls[queryRound], sizeof(NSCam::NSIoPipe::WMFECtrl));
    ::memcpy(table, __tblis[queryRound], sizeof(TABLE_T) * TBLI_SIZE);

    return true;
}

void
HW_WMFTuning::_initDefaultValues()
{
    const short DEFAULT_TBLI[TBLI_SIZE] = {
        1000,   920,    846,    778,    716,    659,    606,    558,    //  8
        513,    472,    434,    399,    367,    338,    311,    286,    // 16
        263,    242,    223,    205,    188,    173,    159,    147,    // 24
        135,    124,    114,    105,    96,     89,     82,     75,     // 32
        69,     63,     58,     54,     49,     45,     42,     38,     // 40
        35,     32,     30,     27,     25,     23,     21,     19,     // 48
        18,     16,     15,     14,     13,     12,     11,     10,     // 56
        9,      8,      7,      7,      6,      6,      5,      5,      // 64
        4,      4,      4,      3,      3,      3,      2,      2,      // 72
        2,      2,      2,      1,      1,      1,      1,      1,      // 80
        1,      1,      1,      1,      1,      1,      1,      1,      // 88
        1,      1,      1,      1,      1,      1,      1,      1,      // 96
        1,      1,      1,      1,      1,      1,      1,      1,      //104
        1,      1,      1,      1,      1,      1,      1,      1,      //112
        1,      1,      1,      1,      1,      1,      1,      1,      //120
        1,      1,      1,      1,      1,      1,      1,      1       //128
    };

    for(int i = 0; i < TOTAL_WMF_ROUND; ++i) {
        ::memcpy(__tblis[i], DEFAULT_TBLI, sizeof(DEFAULT_TBLI));
    }

    //WMF round
    __ctrls[E_WMF_ROUND].Wmfe_Enable                = 0;
    __ctrls[E_WMF_ROUND].WmfeFilterSize             = WMFE_FILTER_SIZE_7x7;
    __ctrls[E_WMF_ROUND].Wmfe_Dpnd_En               = 0;
    __ctrls[E_WMF_ROUND].Wmfe_Mask_En               = 0;
    __ctrls[E_WMF_ROUND].WmfeHorzScOrd              = WMFE_HORZ_RIGHT2LEFT;
    __ctrls[E_WMF_ROUND].WmfeVertScOrd              = WMFE_VERT_TOP2BOTTOM;
    __ctrls[E_WMF_ROUND].Wmfe_Mask_Value            = 0;
    __ctrls[E_WMF_ROUND].Wmfe_Mask_Mode             = 0;

    //Hole filling round 1
    __ctrls[E_HOLE_FILLING_ROUND_1].WmfeFilterSize  = WMFE_FILTER_SIZE_7x7;
    __ctrls[E_HOLE_FILLING_ROUND_1].Wmfe_Dpnd_En    = 1;
    __ctrls[E_HOLE_FILLING_ROUND_1].Wmfe_Mask_En    = 1;
    __ctrls[E_HOLE_FILLING_ROUND_1].WmfeHorzScOrd   = (StereoSettingProvider::getSensorRelativePosition())
                                                      ? WMFE_HORZ_LEFT2RIGHT
                                                      : WMFE_HORZ_RIGHT2LEFT;
    __ctrls[E_HOLE_FILLING_ROUND_1].WmfeVertScOrd   = WMFE_VERT_TOP2BOTTOM;
    __ctrls[E_HOLE_FILLING_ROUND_1].Wmfe_Mask_Value = 0;
    __ctrls[E_HOLE_FILLING_ROUND_1].Wmfe_Mask_Mode  = 1;

    //Hole filling round 2
    __ctrls[E_HOLE_FILLING_ROUND_2].WmfeFilterSize  = WMFE_FILTER_SIZE_7x7;
    __ctrls[E_HOLE_FILLING_ROUND_2].Wmfe_Dpnd_En    = 1;
    __ctrls[E_HOLE_FILLING_ROUND_2].Wmfe_Mask_En    = 1;
    __ctrls[E_HOLE_FILLING_ROUND_2].WmfeHorzScOrd   = (StereoSettingProvider::getSensorRelativePosition())
                                                      ? WMFE_HORZ_RIGHT2LEFT
                                                      : WMFE_HORZ_LEFT2RIGHT;
    __ctrls[E_HOLE_FILLING_ROUND_2].WmfeVertScOrd   = WMFE_VERT_BOTTOM2TOP;
    __ctrls[E_HOLE_FILLING_ROUND_2].Wmfe_Mask_Value = 0;
    __ctrls[E_HOLE_FILLING_ROUND_2].Wmfe_Mask_Mode  = 1;
}

void
HW_WMFTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("================ WMF Parameters ================");
    for(int i = 0; i < TOTAL_WMF_ROUND; i++) {
        NSCam::NSIoPipe::WMFECtrl *ctrl = &__ctrls[i];
        TABLE_T *table = __tblis[i];

        logger
        .FastLogD("Round: %s", QUERY_KEY_WMF_ROUND_NAME[i])
        .FastLogD("WmfeFilterSize:  %d", ctrl->WmfeFilterSize)
        .FastLogD("Wmfe_Dpnd_En:    %d", ctrl->Wmfe_Dpnd_En)
        .FastLogD("Wmfe_Mask_En:    %d", ctrl->Wmfe_Mask_En)
        .FastLogD("WmfeHorzScOrd:   %d", ctrl->WmfeHorzScOrd)
        .FastLogD("WmfeVertScOrd:   %d", ctrl->WmfeVertScOrd)
        .FastLogD("Wmfe_Mask_Value: %d", ctrl->Wmfe_Mask_Value)
        .FastLogD("Wmfe_Mask_Mode:  %d", ctrl->Wmfe_Mask_Mode);

        if (E_WMF_ROUND == i) {
            logger.FastLogD("Wmfe_Enable:     %d", ctrl->Wmfe_Enable);
        }

        logger.FastLogD("Table: ");
        size_t element_per_line = 8;
        size_t table_lines = TBLI_SIZE/element_per_line;
        TABLE_T *src = table;
        for(size_t line = 0; line < table_lines; line++) {
            logger.FastLogD("%5d %5d %5d %5d %5d %5d %5d %5d",
                            *src, *(src+1),*(src+2),*(src+3),
                            *(src+4),*(src+5),*(src+6),*(src+7));
            src += element_per_line;
        }

        if(i < TOTAL_WMF_ROUND-1) {
            logger.FastLogD("------------------------------------------------");
        } else {
            logger.FastLogD("================================================");
        }
    }

    logger.print();
}

void
HW_WMFTuning::_loadValuesFromDocument(const json& wmfValues)
{
    NSCam::NSIoPipe::WMFECtrl *ctrl = NULL;
    TABLE_T *table = NULL;

    for(auto &wmfValue : wmfValues) {
        const json &tuningValues = wmfValue[VALUE_KEY_VALUES];
        for(int round = 0; round < TOTAL_WMF_ROUND; round++) {
            if(wmfValue[VALUE_KEY_PARAMETERS][QUERY_KEY_WMF_ROUND] == QUERY_KEY_WMF_ROUND_NAME[round])
            {
                ctrl = &__ctrls[round];
                table = __tblis[round];
                if (E_WMF_ROUND == round) {
                    ctrl->Wmfe_Enable     = _getInt(tuningValues, "Wmfe_Enable");
                }

                switch(round) {
                    case E_WMF_ROUND:
                    default:
                        ctrl->WmfeHorzScOrd = WMFE_HORZ_LEFT2RIGHT;
                        ctrl->WmfeVertScOrd = WMFE_VERT_TOP2BOTTOM;
                    break;
                    case E_HOLE_FILLING_ROUND_1:
                        ctrl->WmfeHorzScOrd = (StereoSettingProvider::getSensorRelativePosition())
                                              ? WMFE_HORZ_LEFT2RIGHT
                                              : WMFE_HORZ_RIGHT2LEFT;
                        ctrl->WmfeVertScOrd = WMFE_VERT_TOP2BOTTOM;
                    break;
                    case E_HOLE_FILLING_ROUND_2:
                        ctrl->WmfeHorzScOrd = (StereoSettingProvider::getSensorRelativePosition())
                                              ? WMFE_HORZ_RIGHT2LEFT
                                              : WMFE_HORZ_LEFT2RIGHT;
                        ctrl->WmfeVertScOrd = WMFE_VERT_BOTTOM2TOP;
                    break;
                }
                break;
            }
        }

        ctrl->WmfeFilterSize  = static_cast<WMFEFILTERSIZE>(_getInt(tuningValues, "WmfeFilterSize"));
        ctrl->Wmfe_Dpnd_En    = _getInt(tuningValues, "Wmfe_Dpnd_En");
        ctrl->Wmfe_Mask_En    = _getInt(tuningValues, "Wmfe_Mask_En");
        ctrl->Wmfe_Mask_Value = _getInt(tuningValues, "Wmfe_Mask_Value");
        ctrl->Wmfe_Mask_Mode  = _getInt(tuningValues, "Wmfe_Mask_Mode");

        const json &tableValues = tuningValues["Table"];
        TABLE_T *dst = table;
        for (auto &n : tableValues) {
            *dst++ = n.get<int>();
        }
    }
}
