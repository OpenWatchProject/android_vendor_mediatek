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

/**
 * @brief Use this interface to control some isp_mgr and do tuning adjust.
 */

#ifndef __IISP_MGR_IF_H__
#define __IISP_MGR_IF_H__

#include <mtkcam/def/common.h>
#include <mtkcam/utils/module/module.h>
#include <mtkcam/aaa/aaa_hal_common.h>

namespace NS3Av3
{
using namespace NSCam;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct RMG_Config_Param
{
    MBOOL       iEnable;
    MUINT32     leFirst;
    MBOOL       zEnable;
    MUINT32     zPattern;
};

struct RMM_Config_Param
{
};

struct LCSO_Param
{
    MBOOL   bSupport;
    MSize   size;
    MINT    format;
    size_t  stride;
    MUINT32 bitDepth;

    LCSO_Param()
        : bSupport(0)
        , size(MSize(0,0))
        , format(0)
        , stride(0)
        , bitDepth(0)
        {};
};

struct NR3D_Config_Param
{
    MBOOL       enable;
    MRect       onRegion;       // region modified by GMV
    MRect       fullImg;        // image full size for demo mode calculation
    MUINT32     vipiOffst;      // in byte
    MSize       vipiReadSize;   // image size for vipi in pixel

    NR3D_Config_Param()
    : enable(MFALSE)
    , onRegion(MRect(0,0))
    , fullImg(MRect(0,0))
    , vipiOffst(0)
    , vipiReadSize(MSize(0,0))
    {};
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief Interface of IspMgr Interface
 */
class IIspMgr {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //          Ctor/Dtor.
    virtual             ~IIspMgr() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    /**
     * @brief get instance of IIspMgr
     */
    static IIspMgr*     getInstance();
    /**
    * @brief enable/disable PDC
    */
    virtual MVOID       setPDCEnable(MINT32 sensorIndex, MBOOL enable)                          = 0;
    /**
    * @brief enable/disable PDCout
    */
    virtual MVOID       setPDCoutEnable(MINT32 sensorIndex, MBOOL enable)                       = 0;
    /**
     * @brief enable/disable RMG
     */
    virtual MVOID       setRMGEnable(MINT32 sensorIndex, MBOOL enable)                          = 0;
    /**
     * @brief enable/disable RMM
     */
    virtual MVOID       setRMMEnable(MINT32 sensorIndex, MBOOL enable)                          = 0;
    /**
     * @brief enable/disable RMG debug
     */
    virtual MVOID       setRMGDebug(MINT32 sensorIndex, MINT32 debugDump)                       = 0;
    /**
     * @brief enable/disable RMM debug
     */
    virtual MVOID       setRMMDebug(MINT32 sensorIndex, MINT32 debugDump)                       = 0;
    /**
     * @brief enable/disable CPN debug
     */
    virtual MVOID       setCPNDebug(MINT32 sensorIndex, MINT32 debugDump) { (void)sensorIndex; (void)debugDump; }
    /**
     * @brief enable/disable DCPN debug
     */
    virtual MVOID       setDCPNDebug(MINT32 sensorIndex, MINT32 debugDump) { (void)sensorIndex; (void)debugDump; }

    /**
     * @brief Since RRZ ratio may cause some ISP module registers overflow,
     *        caller has to check the maximum RRZ downscale ratio.
     * @return denominator of 1 (aka 1/N)
     */
    virtual MUINT32     queryRRZ_MaxRatio() const
    {
        return 4;
    }

    /**
     * @brief config RMG,RMG2 initialize Parameter
     */
    virtual MVOID       configRMG_RMG2(MINT32 sensorIndex, RMG_Config_Param& param)             = 0;
    /**
     * @brief config RMM.RMM2 initialize Parameter
     */
    virtual MVOID       configRMM_RMM2(MINT32 sensorIndex, RMM_Config_Param& param)             = 0;
    /**
     * @brief config CPN.CPN2 initialize Parameter
     */
    virtual MVOID       configCPN_CPN2(MINT32 sensorIndex, MBOOL zEnable) { (void)sensorIndex; (void)zEnable; }
    /**
     * @brief config DCPN.DCPN2 initialize Parameter
     */
    virtual MVOID       configDCPN_DCPN2(MINT32 sensorIndex, MBOOL zEnable) { (void)sensorIndex; (void)zEnable; }
    /**
     * @brief query LCSO parameters, such as size, format, stride...
     */
    virtual MVOID       queryLCSOParams(LCSO_Param& param)                                      = 0;
    virtual MUINT32     queryDualSyncInfoSize() { return 0;}

#if 0
    virtual MBOOL       queryLCSTopControl(MUINT i4SensorOpenIndex)                             = 0;
#endif

     /**
     * @brief set NR3D parameter and modify data in pTuning buffer
     */
    virtual MVOID       postProcessNR3D(MINT32 sensorIndex, NR3D_Config_Param& param,
                                                void* pTuning)                                  = 0;

    /**
     * @brief get iso-corresponding ABF tuning data from NVRAM
     */
    virtual MVOID*      getAbfTuningData(MINT32 const sensorIndex, int iso)                     = 0;
};

}; // namespace NS3Av3


/**
 * @brief The definition of the maker of IIspMgr instance.
 */
typedef NS3Av3::IIspMgr* (*IspMgr_FACTORY_T)();
#define MAKE_IspMgr(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_AAA_ISP_MGR, IspMgr_FACTORY_T, __VA_ARGS__)


#endif
