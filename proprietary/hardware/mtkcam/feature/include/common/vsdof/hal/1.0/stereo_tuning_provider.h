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
#ifndef _TUNING_PROVIDER_H_
#define _TUNING_PROVIDER_H_

#include <stereo_tuning_def.h>

namespace StereoHAL {

class StereoTuningProvider
{
public:
    /**
     * \brief Get tuning info of DPE
     * \details If user set vendor.STEREO.tuning.dpe to 1,
     *          we will dump tuning parameters to /sdcard/tuning/dpe_params in JSON format.
     *          User can edit this file to change tuning parameter and take effect in runtime.
     *
     * \param tuningBuffer Tuning parameters will be saved in the buffer
     * \param eScenario Current scenario, default is capture
     * \return true if success
     */
    static bool getDPETuningInfo(NSCam::NSIoPipe::DVEConfig *tuningBuffer, StereoHAL::ENUM_STEREO_SCENARIO eScenario=StereoHAL::eSTEREO_SCENARIO_CAPTURE);

    /**
     * \brief Get tuning info of WMF
     * \param ctrls Vector of WMFECtrl
     * \param tblis vector of tbli buffer
     *
     * \return true if success
     */
    static bool getWMFTuningInfo(std::vector<NSCam::NSIoPipe::WMFECtrl> &ctrls, std::vector<void *> &tblis);

    /**
     * \brief Get input channel of WMF image
     * \details the input channel setting will affect sizes related to MY_S
     *
     * \return channel Use Y/U/V channel for WMF input
     */
    static ENUM_WMF_CHANNEL getWMFInputChannel();

    /**
     * \brief Get GF tunings
     *
     * \param coreNumber Core number
     * \param tuningTable The count of element in the table
     * \param tuningParams key-value of tuning parameters
     * \param scenario Scenario to query
     *
     * \return true if success
     */
    static bool getGFTuningInfo(MUINT32 &coreNumber, GF_TUNING_T &tuningTable, std::vector<std::pair<std::string, int>> &tuningParams, ENUM_STEREO_SCENARIO eScenario);

    /**
     * \brief Get OCC tunings
     *
     * \param coreNumber Core number
     * \param tuningParams key-value of tuning parameters
     * \param scenario Scenario to query
     *
     * \return true if success
     */
    static bool getOCCTuningInfo(MUINT32 &coreNumber, std::vector<std::pair<std::string, int>> &tuningParams, ENUM_STEREO_SCENARIO eScenario);

    /**
     * \brief Get SW Bokeh tunings
     *
     * \param tuningParams key-value of tuning parameters
     *
     * \return true if success
     */
    static bool getSWBokehTuningInfo(std::vector<std::pair<std::string, int>> &tuningParams, std::vector<int> &clearTable);

    /**
     * \brief Get tuning info of Bokeh
     * \details If user set vendor.STEREO.tuning.bokeh to 1,
     *          we will dump tuning parameters to /sdcard/tuning/bokeh_params in JSON format.
     *          User can edit this file to change tuning parameter and take effect in runtime.
     *
     * \param tuningBuffer Ouptut of tuning buffer, type: dip_x_reg_t*
     * \param eBokehStrength Strength of bokeh
     *
     * \return true if success
     */
    static bool getBokehTuningInfo(void *tuningBuffer, ENUM_BOKEH_STRENGTH eBokehStrength=E_BOKEH_STRENGTH_NORMAL);

    /**
     * \brief Get tuning info of FM
     *
     * \param direction FM direction, left to right or right to left.
     *                  Note that sensor relative position may affect the result
     * \param fmInfo Output FM tuning info
     * \return true if success
     */
    static bool getFMTuningInfo(ENUM_FM_DIRECTION direction, NSCam::NSIoPipe::FMInfo& fmInfo);

    /**
     * \brief Get tuning info of FE
     *
     * \param feInfo Tuning info will be saved here
     * \param BLOCK_SIZE FE block size, we support block size of 8 and 16 now
     *
     * \return true if success
     * \see StereoSettingProvider::fefmBlockSize
     */
    static bool getFETuningInfo(NSCam::NSIoPipe::FEInfo& feInfo, const int BLOCK_SIZE);

    /**
     * \brief Get VSDOFParam for MDP PQ parameter configuration
     *
     * \param round Specified P2 round
     * \param vsdofParam Structure used to store VSDoF parameters
     *
     * \return true if success
     */
    static bool getPass2MDPPQParam(ENUM_PASS2_ROUND round, VSDOFParam &vsdofParam);

    /**
     * \brief Get VSDOFParam for MDP PQ parameter configuration
     *
     * \param queryName Name to query
     * \param vsdofParam Structure used to store VSDoF parameters
     *
     * \return true if success
     */
    static bool getMDPPQParam(const char *queryName, VSDOFParam &vsdofParam);
};

};  //namespace StereoHAL
#endif