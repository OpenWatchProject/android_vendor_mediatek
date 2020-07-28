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
 *   MtkOmxAudioDecBase.cpp
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


#define MTK_LOG_ENABLE 1
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <cutils/log.h>
#include "osal_utils.h"
#include "MtkOmxAudUtil.h"
#include <cutils/properties.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxAudUtil"

#define ENABLE_XLOG_MTK_OMX_UTIL
#ifdef ENABLE_XLOG_MTK_OMX_UTIL
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#define LOGE SLOGE
#define LOGW SLOGW
#define LOGI SLOGI
#define LOGD SLOGD
#define LOGV SLOGV
#endif

#define OMX_DOWNMIX_ARM_OPT

// compile otpion enabled by MULTI_CH_PLAYBACK_SUPPORT
#ifdef DOWNMIX_SUPPORT


__inline OMX_S32 omx_smulwb(OMX_S32 x, OMX_S32 y)
{
    OMX_S32 ret;
    asm("smulwb %[ret], %[x], %[y] \n"
        : [ret]"=r"(ret)
        : [x]"%r"(x), [y]"r"(y));
    return ret;
}

__inline OMX_S32 omx_smlawb(OMX_S32 x, OMX_S32 y, OMX_S32 a)
{
    OMX_S32 ret;
    asm("smlawb %[ret], %[x], %[y], %[a] \n"
        : [ret]"=r"(ret)
        : [x]"%r"(x), [y]"r"(y), [a]"r"(a));
    return ret;
}

/*
 * Saturating 32-bit integer addition, with the second operand
 * multiplied by two. (i.e. return x + 2 * y)
 */
__inline OMX_S32 omx_qdadd(OMX_S32 x, OMX_S32 y)
{
    OMX_S32 ret;
    asm("qdadd %[ret], %[x], %[y] \n"
        : [ret]"=r"(ret)
        : [x]"%r"(x), [y]"r"(y));
    return ret;
}



OMX_ERRORTYPE MtkOmxAudUtil::Downmix(OMX_AUDIO_DOWNMIX_PARM *pDownmixParm)
{
    OMX_ERRORTYPE err;
    OMX_U32 i, u4Step;
    OMX_U32 u4FrameLen;
    OMX_S16 *pInL, *pInR, *pInC, *pInLs, *pInRs, *pInLfe;
    OMX_S16 *pOutL, *pOutR;
    OMX_AUDIO_CH_LAYOUT *ptInCfg;
    OMX_AUDIO_CH_LAYOUT *ptOutCfg;
    OMX_S32 LFE;
    OMX_S32 a, b, c;

    LOGD("-- Downmix V1.3 -- ");

    if ((NULL == pDownmixParm)             ||
            (NULL == pDownmixParm->ptInChCfg)  ||
            (NULL == pDownmixParm->ptOutChCfg) ||
            (NULL == pDownmixParm->pInPcmBuf)  ||
            (NULL == pDownmixParm->pOutPcmBuf))
    {
        err = OMX_ErrorBadParameter;
        return err;
    }

    u4FrameLen = pDownmixParm->u4FrameLen;
    ptInCfg  = pDownmixParm->ptInChCfg;
    ptOutCfg = pDownmixParm->ptOutChCfg;

    switch (pDownmixParm->ePcmBufType)
    {
        case OMX_AUDIO_PCMBUF_COUPLE_INTERLEAVE:
            pInL   = pDownmixParm->pInPcmBuf + (ptInCfg->L / 2)   * u4FrameLen * 2;
            pInR   = pDownmixParm->pInPcmBuf + (ptInCfg->R / 2)   * u4FrameLen * 2;
            pInC   = pDownmixParm->pInPcmBuf + (ptInCfg->C / 2)   * u4FrameLen * 2;
            pInLs  = pDownmixParm->pInPcmBuf + (ptInCfg->Ls / 2)  * u4FrameLen * 2;
            pInRs  = pDownmixParm->pInPcmBuf + (ptInCfg->Rs / 2)  * u4FrameLen * 2;
            pInLfe = pDownmixParm->pInPcmBuf + (ptInCfg->Lfe / 2) * u4FrameLen * 2;
            u4Step = 2;
            break;

        case OMX_AUDIO_PCMBUF_MULTI_INTERLEAVE:
            pInL   = pDownmixParm->pInPcmBuf + ptInCfg->L;
            pInR   = pDownmixParm->pInPcmBuf + ptInCfg->R;
            pInC   = pDownmixParm->pInPcmBuf + ptInCfg->C;
            pInLs  = pDownmixParm->pInPcmBuf + ptInCfg->Ls;
            pInRs  = pDownmixParm->pInPcmBuf + ptInCfg->Rs;
            pInLfe = pDownmixParm->pInPcmBuf + ptInCfg->Lfe;
            u4Step = pDownmixParm->u4InChNum;
            break;

        case OMX_AUDIO_PCMBUF_CH_BLOCK:
            pInL   = pDownmixParm->pInPcmBuf + (ptInCfg->L)   * u4FrameLen;
            pInR   = pDownmixParm->pInPcmBuf + (ptInCfg->R)   * u4FrameLen;
            pInC   = pDownmixParm->pInPcmBuf + (ptInCfg->C)   * u4FrameLen;
            pInLs  = pDownmixParm->pInPcmBuf + (ptInCfg->Ls)  * u4FrameLen;
            pInRs  = pDownmixParm->pInPcmBuf + (ptInCfg->Rs)  * u4FrameLen;
            pInLfe = pDownmixParm->pInPcmBuf + (ptInCfg->Lfe) * u4FrameLen;
            u4Step = 1;
            break;

        default:
            err = OMX_ErrorBadParameter;
            return err;

    }

    // Because we always mix LFE regardless LFE exists or not, to prevent reading invalid memory adress when LFE doesn't exist
    // set pointer to pcmbuf here
    if (ptInCfg->Lfe == 0xFF)
    {
        pInLfe = pDownmixParm->pInPcmBuf;
    }

    LFE = (ptInCfg->Lfe != 0xFF) ? 0x4C3FE5C9 : 0;   // -4.5dB mix gain

    pOutL = pDownmixParm->pOutPcmBuf + ptOutCfg->L;
    pOutR = pDownmixParm->pOutPcmBuf + ptOutCfg->R;

    // 5.1/5.0->2
    if ((pDownmixParm->u4InChNum == 6) || (pDownmixParm->u4InChNum == 5))
    {
        OMX_S32 L_L   = 0x29033200;
        OMX_S32 C_L   = 0x1CFFE600;
        OMX_S32 LS_L  = 0xE3001A00;
        OMX_S32 RS_L  = 0xE3001A00;
        OMX_S32 R_R   = 0x29033200;
        OMX_S32 C_R   = 0x1CFFE600;
        OMX_S32 LS_R  = 0x1CFFE600;
        OMX_S32 RS_R  = 0x1CFFE600;

        for (i = 0; i < pDownmixParm->u4FrameLen; i++)
        {
#ifdef OMX_DOWNMIX_ARM_OPT
            a      = omx_smulwb(C_L, (*pInC));        // C_L = C_R
            c      = omx_smulwb(LFE, (*pInLfe)) * 2;

            b      = omx_smlawb(L_L, (*pInL), a);
            b      = omx_smlawb(LS_L, (*pInLs), b);
            b      = omx_smlawb(RS_L, (*pInRs), b);
            *pOutL = omx_qdadd(c, b) >> 16;

            b      = omx_smlawb(R_R, (*pInR), a);
            b      = omx_smlawb(LS_R, (*pInLs), b);
            b      = omx_smlawb(RS_R, (*pInRs), b);
            *pOutR = omx_qdadd(c, b) >> 16;

#else
            *pOutL = ((OMX_S64)LFE  * (*pInLfe) +
                      (OMX_S64)L_L  * (*pInL)  +
                      (OMX_S64)C_L  * (*pInC)  +
                      (OMX_S64)LS_L * (*pInLs) +
                      (OMX_S64)RS_L * (*pInRs)) >> 31;

            *pOutR = ((OMX_S64)LFE  * (*pInLfe) +
                      (OMX_S64)R_R  * (*pInR)  +
                      (OMX_S64)C_R  * (*pInC)  +
                      (OMX_S64)LS_R * (*pInLs) +
                      (OMX_S64)RS_R * (*pInRs)) >> 31;
#endif

            pInL += u4Step;
            pInR += u4Step;
            pInC += u4Step;
            pInLs += u4Step;
            pInRs += u4Step;
            pInLfe += u4Step;
            pOutL += 2;
            pOutR += 2;
        }

    }
    // 3/1 -> 2
    else if (pDownmixParm->u4InChNum == 4)
    {
        // use center channel existence to distinguish 3/1 or 2/2
        if (ptInCfg->C != 0xFF)
        {
            OMX_S32 L_L   = 0x35062600;
            OMX_S32 C_L   = 0x257E2D00;
            OMX_S32 LS_L  = 0xDA81D300;
            OMX_S32 R_R   = 0x35062600;
            OMX_S32 C_R   = 0x257E2D00;
            OMX_S32 LS_R  = 0x257E2D00;

            // 3/1 (L,R,C,S) -> 2
            for (i = 0; i < pDownmixParm->u4FrameLen; i++)
            {
#ifdef OMX_DOWNMIX_ARM_OPT
                a      = omx_smulwb(C_L, (*pInC));        // C_L = C_R
                c      = omx_smulwb(LFE, (*pInLfe)) * 2;

                b      = omx_smlawb(L_L, (*pInL), a);
                b      = omx_smlawb(LS_L, (*pInLs), b);
                *pOutL = omx_qdadd(c, b) >> 16;

                b      = omx_smlawb(R_R, (*pInR), a);
                b      = omx_smlawb(LS_R, (*pInLs), b);
                *pOutR = omx_qdadd(c, b) >> 16;

#else
                *pOutL = ((OMX_S64)LFE  * (*pInLfe) +
                          (OMX_S64)L_L  * (*pInL)  +
                          (OMX_S64)C_L  * (*pInC)  +
                          (OMX_S64)LS_L * (*pInLs)) >> 31;

                *pOutR = ((OMX_S64)LFE  * (*pInLfe) +
                          (OMX_S64)R_R  * (*pInR)  +
                          (OMX_S64)C_R  * (*pInC)  +
                          (OMX_S64)LS_R * (*pInLs)) >> 31;
#endif

                pInL += u4Step;
                pInR += u4Step;
                pInC += u4Step;
                pInLs += u4Step;
                pInLfe += u4Step;
                pOutL += 2;
                pOutR += 2;
            }
        }
        else
        {
            OMX_S32 L_L   = 0x35062600;
            OMX_S32 LS_L  = 0xDA81D300;
            OMX_S32 RS_L  = 0xDA81D300;
            OMX_S32 R_R   = 0x35062600;
            OMX_S32 LS_R  = 0x257E2D00;
            OMX_S32 RS_R  = 0x257E2D00;

            // 2/2 (L,R,Ls,Rs) -> 2
            for (i = 0; i < pDownmixParm->u4FrameLen; i++)
            {
#ifdef OMX_DOWNMIX_ARM_OPT
                c      = omx_smulwb(LFE, (*pInLfe)) * 2;

                a      = omx_smulwb(L_L, (*pInL));
                b      = omx_smlawb(LS_L, (*pInLs), a);
                b      = omx_smlawb(RS_L, (*pInRs), b);
                *pOutL = omx_qdadd(c, b) >> 16;


                a      = omx_smulwb(R_R, (*pInR));
                b      = omx_smlawb(LS_R, (*pInLs), a);
                b      = omx_smlawb(RS_R, (*pInRs), b);
                *pOutR = omx_qdadd(c, b) >> 16;

#else

                *pOutL = ((OMX_S64)LFE  * (*pInLfe) +
                          (OMX_S64)L_L  * (*pInL)  +
                          (OMX_S64)LS_L * (*pInLs)  +
                          (OMX_S64)RS_L * (*pInRs)) >> 31;

                *pOutR = ((OMX_S64)LFE  * (*pInLfe) +
                          (OMX_S64)R_R  * (*pInR)  +
                          (OMX_S64)LS_R * (*pInLs)  +
                          (OMX_S64)RS_R * (*pInRs)) >> 31;
#endif

                pInL += u4Step;
                pInR += u4Step;
                pInLs += u4Step;
                pInRs += u4Step;
                pInLfe += u4Step;
                pOutL += 2;
                pOutR += 2;
            }
        }
    }
    else if (pDownmixParm->u4InChNum == 3)
    {
        // use center channel existence to distinguish 3/0 or 2/1
        if (ptInCfg->C != 0xFF)
        {
            OMX_S32 L_L  = 0x4AFC3E00;
            OMX_S32 C_L  = 0x35058400;
            OMX_S32 R_R  = 0x4AFC3E00;
            OMX_S32 C_R  = 0x35058400;

            // 3/0 (L,R,C) -> 2
            for (i = 0; i < pDownmixParm->u4FrameLen; i++)
            {
#ifdef OMX_DOWNMIX_ARM_OPT
                c      = omx_smulwb(LFE, (*pInLfe)) * 2;
                a      = omx_smulwb(C_L, (*pInC));  // C_L = C_R

                b      = omx_smlawb(L_L, (*pInL), a);
                *pOutL = omx_qdadd(c, b) >> 16;

                b      = omx_smlawb(R_R, (*pInR), a);
                *pOutR = omx_qdadd(c, b) >> 16;
#else
                *pOutL = ((OMX_S64)LFE  * (*pInLfe) +
                          (OMX_S64)L_L * (*pInL)  +
                          (OMX_S64)C_L * (*pInC)) >> 31;

                *pOutR = ((OMX_S64)LFE  * (*pInLfe) +
                          (OMX_S64)R_R * (*pInR)  +
                          (OMX_S64)C_R * (*pInC)) >> 31;
#endif

                pInL += u4Step;
                pInR += u4Step;
                pInC += u4Step;
                pInLfe += u4Step;
                pOutL += 2;
                pOutR += 2;
            }
        }
        else
        {
            OMX_S32 L_L   = 0x4AFC3E00;
            OMX_S32 LS_L  = 0xCAFA7C00;
            OMX_S32 R_R   = 0x4AFC3E00;
            OMX_S32 LS_R  = 0x35058400;

            // 2/1 (L,R,Ls) -> 2
            for (i = 0; i < pDownmixParm->u4FrameLen; i++)
            {
#ifdef OMX_DOWNMIX_ARM_OPT
                c      = omx_smulwb(LFE, (*pInLfe)) * 2;

                a      = omx_smulwb(L_L, (*pInL));
                b      = omx_smlawb(LS_L, (*pInLs), a);
                *pOutL = omx_qdadd(c, b) >> 16;

                a      = omx_smulwb(R_R, (*pInR));
                b      = omx_smlawb(LS_R, (*pInLs), a);
                *pOutR = omx_qdadd(c, b) >> 16;
#else

                *pOutL = ((OMX_S64)LFE  * (*pInLfe) +
                          (OMX_S64)L_L  * (*pInL)  +
                          (OMX_S64)LS_L * (*pInLs)) >> 31;

                *pOutR = ((OMX_S64)LFE  * (*pInLfe) +
                          (OMX_S64)R_R  * (*pInR)  +
                          (OMX_S64)LS_R * (*pInLs)) >> 31;
#endif
                pInL += u4Step;
                pInR += u4Step;
                pInC += u4Step;
                pInLfe += u4Step;
                pOutL += 2;
                pOutR += 2;
            }
        }
    }

    return OMX_ErrorNone;

}

#define OMX_AUDIO_MAX_MULTI_CH_NS     (8)


// out channel config is derived from /alps/system/core/include/system/Audio.h
// audio_channel_out_mask_from_count
/*
    case 3:
        return (AUDIO_CHANNEL_OUT_STEREO | AUDIO_CHANNEL_OUT_FRONT_CENTER);
    case 4: // 4.0
        return AUDIO_CHANNEL_OUT_QUAD;
    case 5: // 5.0
        return (AUDIO_CHANNEL_OUT_QUAD | AUDIO_CHANNEL_OUT_FRONT_CENTER);
    case 6: // 5.1
        return AUDIO_CHANNEL_OUT_5POINT1;
    case 7: // 6.1
        return (AUDIO_CHANNEL_OUT_5POINT1 | AUDIO_CHANNEL_OUT_BACK_CENTER);
    case 8:
        return AUDIO_CHANNEL_OUT_7POINT1;
*/
/*
    also see dwnmix_foldFromxxxx in EffectDownmix.c
*/

OMX_AUDIO_CH_LAYOUT MultiChOutCfgTbl[6] =
{
   //L , R , C , Ls , Rs, Lsr , Rsr , LFE
    {0 , 1 , 2   , 0xFF , 0xFF , 0xFF, 0xFF, 0xFF},  // 3.0 channel order : L R C 3/0
    {0 , 1 ,0xFF ,  2   ,  3   , 0xFF, 0xFF, 0xFF},  // 4.0 channel order : L R Ls Rs   2/2
    {0 , 1 , 2   ,  3   ,  4   , 0xFF, 0xFF, 0xFF},  // 5.0 channel order : L R C Ls Rs   3/2
    {0 , 1 , 2   ,  4   ,  5   , 0xFF, 0xFF,  3 },   // 5.1 channel order : L R C LFE Ls Rs
    {0 , 1 , 2   ,  3   ,  4   ,   5,   6,   0xFF},  // 7.0 channel order : L R C Ls Rs Lsr Rsr   
    {0 , 1 , 2   ,  4   ,  5   ,   6,   7,    3 },   // 7.1 channel order : L R C LFE Ls Rs Lsr Rsr
};

OMX_AUDIO_CH_LAYOUT Ch4COutCfg = 
{
    0 , 1 , 2   ,  3   , 0xFF , 0xFF, 0xFF, 0xFF       // 4.0 channel order : L R C S   3/1
};  

OMX_ERRORTYPE MtkOmxAudUtil::Reorder(OMX_AUDIO_REORDER_PARM *pReorderParm )
{
    OMX_U32 i, j, u4Step, hasC;
    OMX_U32 u4FrameLen;
    OMX_S16 *pIn;
    OMX_S16 *pOut;
    OMX_U8 *ptInCfg, *ptOutCfg;
    OMX_U8 in_idx, out_idx;
    

    if ((NULL == pReorderParm)             ||
        (NULL == pReorderParm->ptInChCfg)  ||
        (NULL == pReorderParm->pInPcmBuf)  ||
        (NULL == pReorderParm->pOutPcmBuf))
    {
        return OMX_ErrorBadParameter;
    }

    if (pReorderParm->u4ChNum < 3)
    {
        return OMX_ErrorBadParameter;
    }

    u4FrameLen = pReorderParm->u4FrameLen;
    ptInCfg    = (OMX_U8 *)(&pReorderParm->ptInChCfg->L);

    hasC = (pReorderParm->ptInChCfg->C != 0xFF) ? 1 : 0;

    if ((pReorderParm->u4ChNum == 4) && (hasC))
    {
        // L,R,C,S
        ptOutCfg =  (OMX_U8 *)(&Ch4COutCfg.L);
    }
    else
    {
        ptOutCfg   = (OMX_U8 *)(&MultiChOutCfgTbl[pReorderParm->u4ChNum-3].L);
    }        

    u4Step = pReorderParm->u4ChNum;

    // input pcm buffer, channel data is interleaved
    for (i = 0; i < OMX_AUDIO_MAX_MULTI_CH_NS; i++)
    {
        in_idx = *ptInCfg++;
        out_idx = *ptOutCfg++;
        
        if ((in_idx != 0xFF) && (out_idx != 0xFF))
        {
            pIn  = pReorderParm->pInPcmBuf  + in_idx;
            pOut = pReorderParm->pOutPcmBuf + out_idx;

            for (j = 0; j < u4FrameLen; j++)
            {
                memcpy(pOut, pIn, 2);
                pIn += u4Step;
                pOut += u4Step;
            }
        }            
    }

    return OMX_ErrorNone;

}

#endif


