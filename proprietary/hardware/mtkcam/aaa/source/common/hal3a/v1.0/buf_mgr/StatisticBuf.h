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

#ifndef _STATISTIC_BUF_H_
#define _STATISTIC_BUF_H_

#include <utils/Condition.h>
#include <aaa_types.h>

namespace NS3Av3 {

    /* AAOSeparation */
    enum AAOlist {
        AAOSepAWBBuf = 0,
        AAOSepAEBuf,
        AAOSepHistBuf,
        AAOSepOverCntBuf,
        AAOSepLSCBuf,
        AAOSepBuf
    };

    /* PSOSeparation */
    enum PSOlist {
        PSOSepAWBBuf = 0,
        PSOSepAEBuf,
        PSOSepBuf
    };

    struct StatisticBufInfo {
        MUINT32    mMagicNumber;
        MUINT32    mSize;
        MUINTPTR   mVa;
        MUINT32    mStride;
        MUINTPTR   mPa_offset;
        MVOID     *mPrivateData;
        MUINT32    mPrivateDataSize;
        MUINT32    mFrameCount;
        MINT64     mTimeStamp;
        MUINT32    mRawType; // PSO before OB or after OB
        MUINT32    mConfigNum;
        MUINT32    mConfigLatency;
        MUINT32    mFlashState;
        MUINT32    mMVHDRRatio_x100;
        MUINT32    mMVHDRStatWidth;
        MUINT32    mMVHDRStatHeight;
        MUINT32    mImgoSizeW;
        MUINT32    mImgoSizeH;
        MUINT32    mImgoBitsPerPixel;
        MUINT32    mImgoFmt;
        MINT32     mImgoBuf_handle;
        MUINT32    mImgHeapID;
        MUINT32    mImgoBuf_share_fd;
        MUINT32    mLensPosition;
        MUINT8     mu1AeMode;
        MBOOL dump(const char* filename) const;
        virtual MVOID* getPart(MUINT32 /*u4ParId*/) const {return NULL;}
        virtual MUINT32 getPartSize(MUINT32 /*u4ParId*/) const {return 0;}
        virtual ~StatisticBufInfo(){}
    };

    class StatisticBuf
    {
        protected:
            mutable android::Mutex  m_Lock;
            MBOOL                   m_fgLog;
            MBOOL                   m_fgCmd;
            MBOOL                   m_fgFlush;
            android::Condition      m_Cond;
            StatisticBufInfo        m_rBufInfo;
            MUINT32                 m_i4TotalBufSize;

        private:
            /* Copy constructor is NOT allowed */
            StatisticBuf(StatisticBuf const&);
            /* Copy-assignment operator is NOT allowed */
            StatisticBuf& operator=(StatisticBuf const&);

        public:
            StatisticBuf();
            StatisticBuf(MUINT32 const i4BufSize);
            virtual ~StatisticBuf();

            virtual MBOOL allocateBuf(MUINT32 const i4BufSize);
            virtual MBOOL freeBuf();
            virtual MBOOL write(StatisticBufInfo rBufInfo);
            virtual StatisticBufInfo* read();
            virtual MBOOL flush();
    };

};
#endif /* _STATISTIC_BUF_H_ */

