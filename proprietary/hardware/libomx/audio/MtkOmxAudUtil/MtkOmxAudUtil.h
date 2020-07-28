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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxAudUtil.h
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX audio utility function
 *
 * Author:
 * -------
 *   PC Chen (mtk00634)
 *
 ****************************************************************************/

#ifndef MTK_OMX_AUDIO_UTIL_H
#define MTK_OMX_AUDIO_UTIL_H

#include "OMX_Core.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    typedef enum OMX_AUDIO_PCMBUF_TYPE
    {
        OMX_AUDIO_PCMBUF_MULTI_INTERLEAVE,      /**< [12345][12345]...    */
        OMX_AUDIO_PCMBUF_COUPLE_INTERLEAVE,     /**< [1212][3434[5656]... */
        OMX_AUDIO_PCMBUF_CH_BLOCK               /**< [1111][2222]3333]... */

    }
    OMX_AUDIO_PCMBUF_TYPE;

    typedef enum OMX_AUDIO_DOWNMIX_COEF_TYPE
    {
        OMX_AUDIO_DOWNMIX_DEFAULT,              /**< default coefficient  */
        OMX_AUDIO_DOWNMIX_USER                  /**< user defined coefficient */

    } OMX_AUDIO_DOWNMIX_COEF_TYPE;

    // The channel position in pcm buffer, start from 0
    // If channel not exists, assign 0xFF
    typedef struct OMX_AUDIO_CH_LAYOUT
    {
        OMX_U8    L;
        OMX_U8    R;
        OMX_U8    C;
        OMX_U8    Ls;
        OMX_U8    Rs;
        OMX_U8    Lsr;
        OMX_U8    Rsr;
        OMX_U8    Lfe;

    } OMX_AUDIO_CH_LAYOUT;

    // L,R,C,Ls,Rs,Lfe,Lsr,Rsr
#define OMX_AUDIO_COEF_L                             (0)
#define OMX_AUDIO_COEF_R                             (1)
#define OMX_AUDIO_COEF_C                             (2)
#define OMX_AUDIO_COEF_LS                            (3)
#define OMX_AUDIO_COEF_RS                            (4)
#define OMX_AUDIO_COEF_LFE                           (5)
#define OMX_AUDIO_COEF_LSR                           (6)
#define OMX_AUDIO_COEF_RSR                           (7)

#define OMX_AUDIO_COEF_NUM                           (8)

    typedef struct OMX_AUDIO_DOWNMIX_PARM
    {
        OMX_IN  OMX_U32 u4FrameLen;                       /**< frame length (samples) */
        OMX_IN  OMX_U32 u4InChNum;                        /**< input channel number */
        OMX_IN  OMX_U32 u4OutChNum;                       /**< output channel number (only support 2 now) */
        OMX_IN  OMX_AUDIO_PCMBUF_TYPE ePcmBufType;        /**< pcm buffer type */
        OMX_IN  OMX_AUDIO_DOWNMIX_COEF_TYPE eCoefType;    /**< coefficient type */
        OMX_IN  OMX_AUDIO_CH_LAYOUT *ptInChCfg;           /**< input channel layout config */
        OMX_IN  OMX_AUDIO_CH_LAYOUT *ptOutChCfg;          /**< output channel layout config */
        OMX_IN  OMX_S32 *psa4Coef[OMX_AUDIO_COEF_NUM];    /**< user defiend coefficient arary */
        OMX_IN  OMX_S16 *pInPcmBuf;                       /**< input pcm buffer */
        OMX_OUT OMX_S16 *pOutPcmBuf;                      /**< output pcm buffer */

    } OMX_AUDIO_DOWNMIX_PARM;

typedef struct OMX_AUDIO_REORDER_PARM
{
    OMX_IN  OMX_U32 u4FrameLen;                     /**< frame length (samples) */
    OMX_IN  OMX_U32 u4ChNum;                        /**< input channel number */
    OMX_IN  OMX_AUDIO_CH_LAYOUT *ptInChCfg;      /**< input channel layout config */
    OMX_IN  OMX_S16 *pInPcmBuf;                    /**< input pcm buffer (one frame)*/
    OMX_OUT OMX_S16 *pOutPcmBuf;                   /**< output pcm buffer (one frame)*/

} OMX_AUDIO_REORDER_PARM;



#ifdef __cplusplus
}
#endif /* __cplusplus */


class MtkOmxAudUtil
{
public:
    static OMX_ERRORTYPE  Downmix(OMX_AUDIO_DOWNMIX_PARM *pDownmixParm);
    static OMX_ERRORTYPE  Reorder(OMX_AUDIO_REORDER_PARM *pReorderParm );

};

#endif

