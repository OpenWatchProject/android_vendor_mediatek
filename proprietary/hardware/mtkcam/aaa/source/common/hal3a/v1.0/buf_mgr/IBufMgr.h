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

#ifndef __IBUFMGR_H__
#define __IBUFMGR_H__

#include <private/aaa_hal_private.h>
#if (CAM3_3A_ISP_40_EN)
#include <hal3a/v1.0/buf_mgr/StatisticBuf.h>
#else
#include "StatisticBuf.h"
#endif

#define MAX_STATISTIC_BUFFER_CNT (2)

namespace NS3Av3 {

#if (!CAM3_3A_ISP_30_EN)
    typedef enum {
        BUF_AAO   = 0,
        BUF_AFO   = 1,
        BUF_FLKO  = 2,
        BUF_PDO   = 3,
        BUF_MVHDR = 4,
        BUF_AEO   = 5,
        BUF_PSO   = 6,
#if (!CAM3_3A_ISP_40_EN)
        BUF_MVHDR3EXPO = 7,
#endif
        BUF_NUM
    } BUF_PORTS;
#else
    typedef enum {
        BUF_AAO   = 0,
        BUF_AFO   = 1,
        BUF_FLKO  = 2,
        BUF_PDO   = 3,
        BUF_NUM
    } BUF_PORTS;
#endif
    class IBufMgr
    {
        public:
            static IBufMgr* createInstance(BUF_PORTS rPort,
                    MINT32 const i4SensorDev, MINT32 const i4SensorIdx,
                    const ConfigInfo_T& rConfigInfo, STT_CFG_INFO_T const sttInfo = STT_CFG_INFO_T());
            virtual MBOOL destroyInstance() = 0;
            virtual ~IBufMgr(){}

            virtual MINT32 dequeueHwBuf() = 0;
            virtual MBOOL enqueueHwBuf() = 0;
            virtual StatisticBufInfo* dequeueSwBuf() = 0;
            virtual MVOID abortDequeue() = 0;
            virtual MINT32 waitDequeue() = 0;
            virtual MVOID notifyPreStop() = 0;
            virtual MVOID setRequestNum(MUINT32 = 0) {};
            virtual MVOID reset() {};
    };

    class AAOBufMgr : public IBufMgr
    {
        public:
            static AAOBufMgr* createInstance(
                    MINT32 const i4SensorDev, MINT32 const i4SensorIdx,
                    const ConfigInfo_T& rConfigInfo, STT_CFG_INFO_T const sttInfo = STT_CFG_INFO_T());
            virtual MBOOL destroyInstance() = 0;
            virtual ~AAOBufMgr(){}

            virtual MINT32 dequeueHwBuf() = 0;
            virtual MBOOL enqueueHwBuf() = 0;
            virtual StatisticBufInfo* dequeueSwBuf() = 0;
            virtual MVOID abortDequeue() = 0;
            virtual MINT32 waitDequeue() = 0;
            virtual MVOID notifyPreStop() = 0;
            virtual MVOID setRequestNum(MUINT32 = 0) {};
            virtual MVOID reset() = 0;
    };

    class AEOBufMgr : public IBufMgr
    {
        public:
            static AEOBufMgr* createInstance(
                    MINT32 const i4SensorDev, MINT32 const i4SensorIdx,
                    MINT32 const i4Tglineno, STT_CFG_INFO_T const sttInfo = STT_CFG_INFO_T());
            virtual MBOOL destroyInstance() = 0;
            virtual ~AEOBufMgr(){}

            virtual MINT32 dequeueHwBuf() = 0;
            virtual MBOOL enqueueHwBuf() = 0;
            virtual StatisticBufInfo* dequeueSwBuf() = 0;
            virtual MVOID abortDequeue() = 0;
            virtual MINT32 waitDequeue() = 0;
            virtual MVOID notifyPreStop() = 0;
    };

    class AFOBufMgr : public IBufMgr
    {
        public:
            static AFOBufMgr* createInstance(
                    MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo = STT_CFG_INFO_T());
            virtual MBOOL destroyInstance() = 0;
            virtual ~AFOBufMgr(){}

            virtual MINT32 dequeueHwBuf() = 0;
            virtual MBOOL enqueueHwBuf() = 0;
            virtual StatisticBufInfo* dequeueSwBuf() = 0;
            virtual MVOID abortDequeue() = 0;
            virtual MINT32 waitDequeue() = 0;
            virtual MVOID notifyPreStop() = 0;
            virtual MVOID reset() = 0;
    };

    class PDOBufMgr : public IBufMgr
    {
        public:
            static PDOBufMgr* createInstance(
                    MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo = STT_CFG_INFO_T());
            virtual MBOOL destroyInstance() = 0;
            virtual ~PDOBufMgr(){}

            virtual MINT32 dequeueHwBuf() = 0;
            virtual MBOOL enqueueHwBuf() = 0;
            virtual StatisticBufInfo* dequeueSwBuf() = 0;
            virtual MVOID abortDequeue() = 0;
            virtual MINT32 waitDequeue() = 0;
            virtual MVOID notifyPreStop() = 0;
            virtual MVOID reset() = 0;
    };

    class MVHDRBufMgr : public IBufMgr
    {
        public:
            static MVHDRBufMgr* createInstance(
                    MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo = STT_CFG_INFO_T());
            virtual MBOOL destroyInstance() = 0;
            virtual ~MVHDRBufMgr(){}

            virtual MINT32 dequeueHwBuf() = 0;
            virtual MBOOL enqueueHwBuf() = 0;
            virtual StatisticBufInfo* dequeueSwBuf() = 0;
            virtual MVOID abortDequeue() = 0;
            virtual MINT32 waitDequeue() = 0;
            virtual MVOID notifyPreStop() = 0;
    };

    class PSOBufMgr : public IBufMgr
    {
        public:
            static PSOBufMgr* createInstance(
                    MINT32 const i4SensorDev, MINT32 const i4SensorIdx,
                    const ConfigInfo_T& rConfigInfo, STT_CFG_INFO_T const sttInfo = STT_CFG_INFO_T());
            virtual MBOOL destroyInstance() = 0;
            virtual ~PSOBufMgr(){}

            virtual MINT32 dequeueHwBuf() = 0;
            virtual MBOOL enqueueHwBuf() = 0;
            virtual StatisticBufInfo* dequeueSwBuf() = 0;
            virtual MVOID abortDequeue() = 0;
            virtual MVOID notifyPreStop() = 0;
            virtual MVOID reset() = 0;
    };

    class MVHDR3ExpoBufMgr : public IBufMgr
    {
        public:
            static MVHDR3ExpoBufMgr* createInstance(
                    MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo = STT_CFG_INFO_T());
            virtual MBOOL destroyInstance() = 0;
            virtual ~MVHDR3ExpoBufMgr(){}

            virtual MINT32 dequeueHwBuf() = 0;
            virtual MBOOL enqueueHwBuf() = 0;
            virtual StatisticBufInfo* dequeueSwBuf() = 0;
            virtual MVOID abortDequeue() = 0;
            virtual MINT32 waitDequeue() = 0;
            virtual MVOID notifyPreStop() = 0;
    };
};
#endif /* __IBUFMGR_H__ */

