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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_INORMALSTREAM_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_INORMALSTREAM_H_
//
#include "IHalPostProcPipe.h"
#include <map>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {


enum EJpgCmd
{
    EJpgCmd_SetWorkBufSize = 0,
    EJpgCmd_SetQualityParam = 1
};

enum ENormalStreamTag
{
    ENormalStreamTag_Normal=0,
    ENormalStreamTag_Prv = ENormalStreamTag_Normal, //fadeout, would be remove later
    ENormalStreamTag_Cap = ENormalStreamTag_Normal, //fadeout, would be remove later
    ENormalStreamTag_Vss,
    ENormalStreamTag_vFB_FB, //fb path in vfb
    ENormalStreamTag_MFB_Bld,
    ENormalStreamTag_MFB_Mix,
    ENormalStreamTag_Bokeh,
    ENormalStreamTag_FE,
    ENormalStreamTag_FM,
    ENormalStreamTag_COLOR_EFT,
    ENormalStreamTag_DeNoise,
    ENormalStreamTag_WUV,
    ENormalStreamTag_Y16Dump,
    ENormalStreamTag_IP_Tpipe,
    ENormalStreamTag_LPCNR_Pass1,
    ENormalStreamTag_LPCNR_Pass2,
    ENormalStreamTag_total
};

/******************************************************************************
 *
 * @enum ESDCmd
 * @brief enum for sendcommand function (specific usage)
 * @details
 *
 ******************************************************************************/
enum ESDCmd
{
    ESDCmd_UNKNOWN = 0,
    ESDCmd_CONFIG_VENC_DIRLK,     //config for isp-mdp-venc direct link
    ESDCmd_RELEASE_VENC_DIRLK, //release
};

/******************************************************************************
 *
 * @class INormalStream
 * @brief Post-Processing Pipe Interface for Normal Stream.
 * @details
 * The data path will be Mem --> ISP--XDP --> Mem.
 *
 ******************************************************************************/
class INormalStream : public virtual IHalPostProcPipe
{
public:     ////                    Instantiation.

    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static  INormalStream*          createInstance(MUINT32 openedSensorIndex, DipUserParam UserParam = {EDipUserEnum_DEFAUT, 0, 0} );
    static  INormalStream*          createInstance(char const* szCallerName,ENormalStreamTag streamTag,MUINT32 openedSensorIndex, MBOOL isV3=false);    //fadeout

    /**
     * @query DIP register size
     *
     * @details
     *
     * @note
     *
     *
     * @return
     * - MUINT32 size of dip
     */
    static  MUINT32                   getRegTableSize(void);


    /**
     * @query DIP information, include DIP version...etc
     *
     * @details
     *
     * @note
     *
     *
     * @return
     * - MTRUE  indicates success;
     * - MFALSE indicates failure;
     */
    static  MBOOL                   queryDIPInfo(std::map<EDIPInfoEnum, MUINT32> &mapDipInfo);

     /**
     * @query DIP secure buffer
     *
     * @details
     *
     * @note
     *
     *
     * @return
     * - MTRUE  need secure buffer;
     * - MFALSE no need secure buffer;
     */
    static  enum EDIPSecDMAType     needSecureBuffer(MUINT32 portIdx);

public:     ////                    Attributes.
    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    static  char const*             pipeName() { return "NormalStream"; }

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const { return pipeName(); }
    /**
     * @brief sendCommand to update jpeg related param.
     *
     * @param[in] jpgCmd: command to update jpeg related information
     * @param[in] arg1: argument1
     * @param[in] agr2: argument2
     *
     * @details
     * setJpegParam(EJpgCmd_SetWorkBufSize,0,0)
     * setJpegParam(EJpgCmd_SetQualityParam,soi_en,fQuality)
     *
     * @note
     *
     * @return
     *      - [true]
     */
    virtual MBOOL                    setJpegParam(EJpgCmd jpgCmd,int arg1,int arg2)=0;
    /**
     * @brief set frame rate
     *
     * @details
     *
     * @note
     *
     *
     * @return
     * - MTRUE  indicates success;
     * - MFALSE indicates failure;
     */
    virtual MBOOL                   setFps(MINT32 fps)=0;

    /**
             * @brief send isp extra command
             *
             * @details
             *
             * @note
             *
             * @param[in] cmd: command
             * @param[in] arg1: arg1
             * @param[in] arg2: arg2
             * @param[in] arg3: arg3
             *
             * @return
             * - MTRUE indicates success;
             * - MFALSE indicates failure, and an error code can be retrived by sendCommand().
             */
        virtual MBOOL sendCommand(ESDCmd cmd, MINTPTR arg1=0, MINTPTR arg2=0, MINTPTR arg3=0)=0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPostProc
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_INORMALSTREAM_H_

