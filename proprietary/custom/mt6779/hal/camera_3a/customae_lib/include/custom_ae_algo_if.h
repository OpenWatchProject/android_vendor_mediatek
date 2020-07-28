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

/**
 * @file custom_ae_algo_if.h
 * @brief Interface to custom AE algorithm library
 */
#ifndef _CUSTOM_AE_ALGO_IF_H_
#define _CUSTOM_AE_ALGO_IF_H_

using namespace NSIspTuning;

namespace NS3Av3
{
    /**
     * @brief Interface to custom AE algorithm library
     */
    class ICustomAeAlgo
    {
        public:
            ICustomAeAlgo(ESensorDev_T eSensorDev);
            virtual ~ICustomAeAlgo();
            static ICustomAeAlgo* getInstance(ESensorDev_T sensor);
            /**
             * @brief Custom AE init function
             * @param [in] in: AE init input parameters
             */
            virtual void CUS_AE_Core_Init(AE_CORE_INIT *in);
            /**
             * @brief Custom AE ctrl function for exchanging information
             * @param [in]  id      : ctrl type id
             * @param [in]  ctrl_in : AE input paramters
             * @param [out] ctrl_out: AE output
             */
            virtual void CUS_AE_Core_Ctrl(AE_CTRL_ENUM id, void *ctrl_in, void *ctrl_out);
            /**
             * @brief Custom AE get statistics function
             */
            // virtual void CUS_AE_Core_Stat(AE_CORE_STAT_IN *in, AE_CORE_STAT_OUT *out);
            /**
             * @brief Custom AE main function for AE calculation
             * @param [in]  in : AE statistics input
             * @param [out] out: AE inforamtion output
             */
            virtual void CUS_AE_Core_Main(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out);
            /**
             * @brief Custom AE reset function
             */
            virtual void CUS_AE_Core_Reset(void);			
            /**
             * @brief Custom AE uninit function
             */
            virtual void CUS_AE_Core_Uninit(void);
            /**
             * @brief Custom AE provide ISP info function
             * @param [out] out: AE inforamtion output
             */
            virtual void CUS_AE_Core_ISPInfo(AE_INFO_T &ae_info);			
            /**
             * @brief Custom AE provide Debug info function
             * @param [out] out: AE inforamtion output
             */
            virtual void CUS_AE_Core_DebugInfo(MVOID* exif_info);
    };


} // namespace NS3A

#endif
