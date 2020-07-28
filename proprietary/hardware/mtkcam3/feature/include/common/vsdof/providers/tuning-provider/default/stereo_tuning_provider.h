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
#ifndef STEREO_TUNING_PROVIDER_H_
#define STEREO_TUNING_PROVIDER_H_

#include <stereo_tuning_def.h>

namespace StereoHAL {

class StereoTuningProvider
{
public:
    /**
     * \brief Get input channel of WMF image
     * \details the input channel setting will affect sizes related to MY_S
     *
     * \return channel Use Y/U/V channel for WMF input
     */
    static ENUM_WMF_CHANNEL getWMFInputChannel();

    //=========================================================
    //      Software tunings
    //=========================================================
    /**
     * \brief Get GF tunings
     *
     * \param coreNumber Core number
     * \param tuningTable The count of element in the table
     * \param dispCtrlPoints Ctrl points
     * \param blurGainTable Blur gain table
     * \param tuningParams key-value of tuning parameters
     * \param scenario Scenario to query
     *
     * \return true if success
     */
    static bool getGFTuningInfo(MUINT32 &coreNumber,
                                GF_TUNING_T &tuningTable,
                                GF_TUNING_T &dispCtrlPoints,
                                GF_TUNING_T &blurGainTable,
                                TUNING_PAIR_LIST_T &tuningParams,
                                ENUM_STEREO_SCENARIO eScenario);

    /**
     * \brief Get re-mapped DoF value for algo from DoF level of AP
     *
     * \param dofLevel DoF of AP
     * \param eScenario query scenario
     *
     * \return Re-mapped DoF value for algo from DoF level of AP
     */
    static MUINT32 getGFDoFValue(const MUINT32 dofLevel, ENUM_STEREO_SCENARIO eScenario);

    /**
     * \brief Get OCC tunings
     *
     * \param coreNumber Core number
     * \param tuningParams key-value of tuning parameters
     * \param scenario Scenario to query
     *
     * \return true if success
     */
    static bool getOCCTuningInfo(MUINT32 &coreNumber, TUNING_PAIR_LIST_T &tuningParams, ENUM_STEREO_SCENARIO eScenario);

    /**
     * \brief Get SW Bokeh tunings
     *
     * \param tuningParams key-value of tuning parameters
     *
     * \return true if success
     */
    static bool getSWBokehTuningInfo(TUNING_PAIR_LIST_T &tuningParams, std::vector<int> &clearTable);

    /**
     * \brief Get re-mapped DoF value for algo from DoF level of AP
     *
     * \param dofLevel DoF of AP
     *
     * \return Re-mapped DoF value for algo from DoF level of AP
     */
    static MUINT32 getSWBokehDoFValue(const MUINT32 dofLevel);

    /**
     * \brief Get N3D tuning params
     *
     * \param scenario Scenario to get
     * \param maskMargin Margin of mask
     * \param tuningParams Other tuning params
     * \return true if success
     */
    static bool getN3DTuningInfo(ENUM_STEREO_SCENARIO scenario, TUNING_PAIR_LIST_T &tuningParams);

    /**
     * \brief Get N3D mask margin
     *
     * \param scenario Scenario to get
     * \param maskMargin Margin of mask
     *
     * \return true if success
     */
    static bool getN3DMaskMargin(ENUM_STEREO_SCENARIO scenario, StereoImageMargin &maskMargin);
};

};  //namespace StereoHAL
#endif