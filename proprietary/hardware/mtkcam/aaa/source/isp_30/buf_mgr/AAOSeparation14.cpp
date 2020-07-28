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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "AAOSeparation14"

#include "AAOSeparation.h"
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <string.h>
#include <arm_neon.h>

MBOOL aaoSeparation14(const MVOID* AAOSrc , const AAO_BUF_CONFIG_T& rAAOBufConfig, AAO_BUF_T& rAAOBuf, const MINT32 AAOSepType)
{
    MBOOL bVersion = ::property_get_int32("debug.aao.version", 0);

    // separate buffer data from input buffer.
    const MUINT8* pSrc   = reinterpret_cast<const MUINT8*>(AAOSrc);
    MUINT8* pawb = (MUINT8*)rAAOBuf.m_pAAOSepAWBBuf;
    MUINT16* pae = (MUINT16*)rAAOBuf.m_pAAOSepAEBuf;
    MUINT16* poverexpcnt = (MUINT16* )rAAOBuf.m_pAAOSepOverCntBuf;
    MUINT16* plsc = (MUINT16*)rAAOBuf.m_pAAOSepLSCBuf;
    MUINT32 u4aaoNoAWBAESz = rAAOBufConfig.m_i4AwbLineSize + rAAOBufConfig.m_i4AeSize + rAAOBufConfig.m_i4HdrSize;

    if(AAOSepType == AAONeonType) //NEON-version
    {
        CAM_LOGD_IF(bVersion, "[%s] NEON-version", __FUNCTION__);

        // AE
        uint16x8_t carry4AE = vdupq_n_u16(0x20);
        uint16x8_t mask = vdupq_n_u16(0x3F);

        // LSC
        uint16_t maskPattern[4] = {0x1F, 0x7E0, 0xF800, 0};// find R, G, B, L value
        int16_t rightShiftPattern[4] = {0, -5, -11, 0};
        uint16_t carryPattern[8] = {0x10, 0x20, 0x10, 0xFF,0x10, 0x20, 0x10, 0xFF};//0xFF => because let data 0 is not bigger than carry
        int16_t leftShiftPattern[8] = {5, 6, 5, 6,5, 6, 5, 6};
        int16_t shift14BitPattern[8] = {3, 2, 3, 2, 3, 2, 3, 2};
        uint16x4_t mask4TSF = vld1_u16(maskPattern);
        int16x4_t rightShift4TSF = vld1_s16(rightShiftPattern);
        uint16x8_t carry4TSF = vld1q_u16(carryPattern);
        int16x8_t leftShift4TSF = vld1q_s16(leftShiftPattern);
        int16x8_t shift14Bit4TSF = vld1q_s16(shift14BitPattern);

        MUINT32 u4FistBlkNumX = (rAAOBufConfig.m_i4BlkNumX >> 4);// 120/16
        MUINT32 u4SecondBlkNumXOver = (rAAOBufConfig.m_i4BlkNumX & 0xF);// 120%16
        MUINT32 u4SecondBlkNumX = ((u4SecondBlkNumXOver) >> 1);

        for(int j=0; j<rAAOBufConfig.m_i4BlkNumY; j++)
        {
            if(rAAOBuf.m_pAAOSepAWBBuf!=0)//awb
            {
                memcpy(pawb, pSrc, rAAOBufConfig.m_i4BlkNumX*4);
                pawb+=rAAOBufConfig.m_i4BlkNumX*4;
            }//awb end

            if(rAAOBuf.m_pAAOSepAEBuf!=0)//ae
            {
                if(rAAOBufConfig.m_bEnableHDRYConfig)
                {
                    const MUINT8* pAeLSB = ((const MUINT8*)pSrc + rAAOBufConfig.m_i4AwbLineSize+rAAOBufConfig.m_i4BlkNumX);
                    const MUINT8* pAeMSB = ((const MUINT8*)pSrc + rAAOBufConfig.m_i4AwbLineSize);
                    for(int k=0;k < (MINT32)u4FistBlkNumX;k++)//(120/16=7)
                    {
                        //LSB
                        uint8x16_t Vc6InputD0 = vld1q_u8(pAeLSB);
                        //Splitting vectors
                        uint8x8_t Vc6Split0 = vget_low_u8(Vc6InputD0);
                        uint8x8_t Vc6Split1 = vget_high_u8(Vc6InputD0);
                        //vector long move
                        uint16x8_t Vc6Res0 = vmovl_u8(Vc6Split0);
                        uint16x8_t Vc6Res1 = vmovl_u8(Vc6Split1);

                        Vc6Res0 = vandq_u16(Vc6Res0, mask);
                        Vc6Res1 = vandq_u16(Vc6Res1, mask);

                        //MSB
                        uint8x16_t Vc8InputD0 = vld1q_u8(pAeMSB);

                        //Splitting vectors
                        uint8x8_t Vc8Split0 = vget_low_u8(Vc8InputD0);
                        uint8x8_t Vc8Split1 = vget_high_u8(Vc8InputD0);

                        //vector long move
                        uint16x8_t Vc8Res0 = vmovl_u8(Vc8Split0);
                        uint16x8_t Vc8Res1 = vmovl_u8(Vc8Split1);

                        //==========Data Mergging==========//*pDst++ = ((c6 & 0x20) ? (((c8-1)<<6)|c6) : ((c8<<6)|c6));
                        //data0
                        uint16x8_t Vcarry0 = vcgeq_u16(Vc6Res0, carry4AE);//(c6 & 0x20)
                        uint16x8_t Vc8Merge0 = vaddq_u16(Vc8Res0, Vcarry0);//(c8+(-1)) and (c8+0)
                        Vc8Merge0 = vshlq_n_u16(Vc8Merge0,6);//left shift 6 bit
                        uint16x8_t Vdst0 = vorrq_u16(Vc8Merge0,Vc6Res0);

                        //data1
                        uint16x8_t Vcarry1 = vcgeq_u16(Vc6Res1, carry4AE);//(c6 & 0x20)
                        uint16x8_t Vc8Merge1 = vaddq_u16(Vc8Res1, Vcarry1);//(c8+(-1)) and (c8+0)
                        Vc8Merge1 = vshlq_n_u16(Vc8Merge1,6);//left shift 6 bit
                        uint16x8_t Vdst1 = vorrq_u16(Vc8Merge1,Vc6Res1);

                        //Store a single vector into memory
                        vst1q_u16 (pae, Vdst0);
                        pae+=8;
                        vst1q_u16 (pae, Vdst1);
                        pae+=8;
                        pAeMSB+=16;
                        pAeLSB+=16;
                    }

                    for(int k=0;k<(MINT32)u4SecondBlkNumXOver;k++)//(120%16)
                    {
                        MUINT16 LSBbit50 = (*(pAeLSB+k))& 0x3F;//0~5 bit
                        MUINT16 MSBfor50 = (*(pAeMSB+k));//0~7 bit
                        *(pae) = ((MSBfor50 - (LSBbit50 >= 32)) << 6) | LSBbit50;//((c6 & 0x20) ? (((c8-1)<<6)|c6) : ((c8<<6)|c6));
                        pae++;
                    }
                }
                else
                {
                    for(int k=0;k<(rAAOBufConfig.m_i4BlkNumX/2);k++)
                    {
                        MUINT16 u2FirstByte = (*(pSrc+rAAOBufConfig.m_i4AwbLineSize+2*k));
                        MUINT16 u2SecondByte = (*(pSrc+rAAOBufConfig.m_i4AwbLineSize+2*k+1));
                        *(pae+2*k)  = u2FirstByte;
                        *(pae+2*k+1)= u2SecondByte;
                    }
                    pae+=rAAOBufConfig.m_i4BlkNumX;
                }
            }//ae end

            if(rAAOBuf.m_pAAOSepOverCntBuf!=0)//overexpcnt
            {
                if(rAAOBufConfig.m_bEnableOverCntConfig)
                {
                    const MUINT8* pover = ((const MUINT8*)pSrc + u4aaoNoAWBAESz);
                    for(int k=0;k<(MINT32)u4FistBlkNumX;k++)
                    {
                        uint8x16_t VoverData = vld1q_u8(pover);
                        //Splitting vectors
                        uint8x8_t VoverDataL = vget_low_u8(VoverData);
                        uint8x8_t VoverDataH = vget_high_u8(VoverData);
                        //vector long move
                        uint16x8_t VoverRes0 = vmovl_u8(VoverDataL);
                        uint16x8_t VoverRes1 = vmovl_u8(VoverDataH);
                        if(rAAOBufConfig.m_u4AEOverExpCntShift)
                        {
                            VoverRes0 = vmulq_n_u16(VoverRes0,2);
                            VoverRes1 = vmulq_n_u16(VoverRes1,2);
                        }
                        //Store a single vector into memory
                        vst1q_u16 (poverexpcnt, VoverRes0);
                        poverexpcnt+=8;
                        vst1q_u16 (poverexpcnt, VoverRes1);
                        poverexpcnt+=8;
                        pover+=16;
                    }
                    //last data
                    for(int k=0;k<(MINT32)u4SecondBlkNumXOver;k+=8)
                    {
                        uint8x8_t VoverData = vld1_u8(pover);
                        uint16x8_t VoverRes = vmovl_u8(VoverData);
                        if(rAAOBufConfig.m_u4AEOverExpCntShift)
                            VoverRes = vmulq_n_u16(VoverRes,2);
                        vst1q_u16 (poverexpcnt, VoverRes);
                        poverexpcnt+=8;
                    }
                }else{
                    for (int k=0; k<rAAOBufConfig.m_i4BlkNumX; k++){
                         *(poverexpcnt+k)= 0;
                    }
                    poverexpcnt+=rAAOBufConfig.m_i4BlkNumX;
                }
            }//overexpcnt end

            if(rAAOBuf.m_pAAOSepLSCBuf!=0)//lsc
            {
                if (rAAOBufConfig.m_bEnableTSFConfig)    // c4 is available
                {
                    const MUINT8* pTmpSrc = pSrc;
                    const MUINT16* pTsfSrc = (const MUINT16*)((const MUINT8*)pSrc + rAAOBufConfig.m_i4SkipSize);
                    MINT32 x = rAAOBufConfig.m_i4BlkNumX * 4;

                    //Neon start
                    while (x >= 16)
                    {
                        //==========Data Preprocessing==========//
                        //c56
                        uint16x4_t Vc56InputD = vld1_u16(pTsfSrc);// load 4 data to 4 vector, and 1 vector is 16 bits

                        uint16x4_t Vc56InputD0 = vdup_lane_u16(Vc56InputD, 0);// 4 vector of Vc56InputD0 is the same data. Data is Vc56InputD[0] value.
                        Vc56InputD0 = vand_u16(Vc56InputD0, mask4TSF); // AND
                        Vc56InputD0 = vshl_u16(Vc56InputD0, rightShift4TSF);// right shift find R0 G0 B0 L0


                        uint16x4_t Vc56InputD1 = vdup_lane_u16(Vc56InputD, 1);// 4 vector of Vc56InputD0 is the same data. Data is Vc56InputD[1] value.
                        Vc56InputD1 = vand_u16(Vc56InputD1, mask4TSF); // AND
                        Vc56InputD1 = vshl_u16(Vc56InputD1, rightShift4TSF);// right shift find R1 G1 B1 L1


                        uint16x4_t Vc56InputD2 = vdup_lane_u16(Vc56InputD, 2);// 4 vector of Vc56InputD0 is the same data. Data is Vc56InputD[2] value.
                        Vc56InputD2 = vand_u16(Vc56InputD2, mask4TSF); // AND
                        Vc56InputD2 = vshl_u16(Vc56InputD2, rightShift4TSF);// right shift find R2 G2 B2 L2

                        uint16x4_t Vc56InputD3 = vdup_lane_u16(Vc56InputD, 3);// 4 vector of Vc56InputD0 is the same data. Data is Vc56InputD[3] value.
                        Vc56InputD3 = vand_u16(Vc56InputD3, mask4TSF); // AND
                        Vc56InputD3 = vshl_u16(Vc56InputD3, rightShift4TSF);// right shift find R3 G3 B3 L2

                        uint16x8_t Vc56CombineRes0 = vcombine_u16(Vc56InputD0, Vc56InputD1);
                        uint16x8_t Vc56CombineRes1 = vcombine_u16(Vc56InputD2, Vc56InputD3);

                        //c8
                        uint8x16_t Vc8Input = vld1q_u8(pTmpSrc);

                        //Splitting vectors
                        uint8x8_t Vc8Split0 = vget_low_u8(Vc8Input);
                        uint8x8_t Vc8Split1 = vget_high_u8(Vc8Input);

                        //vector long move
                        uint16x8_t Vc8Res0 = vmovl_u8(Vc8Split0);
                        uint16x8_t Vc8Res1 = vmovl_u8(Vc8Split1);

                        //==========Data Mergging==========//*pDst++ = ((c5 & 0x10) ? (((c8-1)<<5)|c5) : ((c8<<5)|c5)), *pDst++ = ((c6 & 0x20) ? (((c8-1)<<6)|c6) : ((c8<<6)|c6));
                        //data0
                        uint16x8_t Vcarry0 = vcgeq_u16(Vc56CombineRes0, carry4TSF);//(c5 & 0x10), (c6 & 0x20)
                        uint16x8_t Vc8Merge0 = vaddq_u16(Vc8Res0, Vcarry0);//(c8+(-1)) and (c8+0)
                        Vc8Merge0 = vshlq_u16(Vc8Merge0, leftShift4TSF);//left shift
                        uint16x8_t Vdst0 = vorrq_u16(Vc8Merge0, Vc56CombineRes0);
                        Vdst0 = vshlq_u16(Vdst0, shift14Bit4TSF);//left shift 16 bit
                        //data1
                        uint16x8_t Vcarry1 = vcgeq_u16(Vc56CombineRes1, carry4TSF);//(c5 & 0x10), (c6 & 0x20)
                        uint16x8_t Vc8Merge1 = vaddq_u16(Vc8Res1, Vcarry1);//(c8+(-1)) and (c8+0)
                        Vc8Merge1 = vshlq_u16(Vc8Merge1, leftShift4TSF);//left shift
                        uint16x8_t Vdst1 = vorrq_u16(Vc8Merge1, Vc56CombineRes1);
                        Vdst1 = vshlq_u16(Vdst1, shift14Bit4TSF);//left shift 16 bit

                        //Store a single vector into memory
                        vst1q_u16 (plsc, Vdst0);
                        plsc+=8;
                        vst1q_u16 (plsc, Vdst1);
                        plsc+=8;
                        pTsfSrc+=4;
                        pTmpSrc+=16;
                        x -= 16;
                    }
                    //Neon end

                    MUINT32 last_res = *((const MUINT32*)pTsfSrc);
                    while (x)
                    {
                        MUINT16 c8;
                        MUINT16 c56;
                        #define NeonEXTRACT56(bit,offset)\
                            c8 = *pTmpSrc++;\
                            c56 = (last_res & ((1<<(bit))-1)); last_res >>= (bit);\
                            *plsc++ = (((c56 & (1<<((bit)-1))) ? (((c8-1)<<(bit))|c56) : ((c8<<(bit))|c56)) << (offset));// shift 16 bits
                        #define NeonEXTRACT\
                            c8 = *pTmpSrc++;\
                            *plsc++ = (c8<<8);

                        NeonEXTRACT56(5,3);
                        NeonEXTRACT56(6,2);
                        NeonEXTRACT56(5,3);
                        NeonEXTRACT
                        x -= 4;
                    }
                }
                else    // c4 is not available
                {
                    const MUINT8*  pTmpSrc = pSrc;
                    MINT32 x = rAAOBufConfig.m_i4BlkNumX * 4;
                    while (x >= 8)
                    {
                        *plsc++ = ((*pTmpSrc++) << 8);// shift 16 bits
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);

                        x -= 8;
                    }
                    while (x)
                    {
                        *plsc++ = ((*pTmpSrc++) << 8);
                        x--;
                    }
                }

            }//lsc end

            pSrc += rAAOBufConfig.m_i4LineSize;
        }//h end

        if(rAAOBuf.m_pAAOSepHistBuf!=0)//hist
        {
            int* pHist = (int*)(rAAOBuf.m_pAAOSepHistBuf);
            pSrc = (MUINT8*)AAOSrc;
            int aaoNoHistSz = rAAOBufConfig.m_i4LineSize*rAAOBufConfig.m_i4BlkNumY;
            pSrc+=aaoNoHistSz;
            if (!rAAOBufConfig.m_bDisablePixelHistConfig){
               for(int j=0;j<AAO_STT_HIST_NUM;j++)
               {
                 for(int i=0;i<AAO_STT_HIST_BIN_NUM;i++)
                 {
                   MUINT32 v;
                   MUINT32 b1;
                   MUINT32 b2;
                   MUINT32 b3;
                   b1 = *pSrc;
                   b2 = *(pSrc+1);
                   b3 = *(pSrc+2);
                   //v = b1+(b2<<8)+(b3<<16);
                   v = b1|(b2<<8)|(b3<<16);
                   pSrc+=3;
                   *pHist=v;
                   pHist++;
                 }
               }
            }else{
               for(int j=0;j<AAO_STT_HIST_NUM;j++)//4
               {
                 for(int i=0;i<AAO_STT_HIST_BIN_NUM;i++)
                 {
                   int v;
                   int b1;
                   int b2;
                   b1 = *pSrc;
                   b2 = *(pSrc+1);
                   //v = b1+(b2<<8);
                   v = b1|(b2<<8);
                   pSrc+=2;
                   *pHist=v;
                   pHist++;
                 }
               }
            }
        }//hist end
    }
    else //C-version
    {
        CAM_LOGD_IF(bVersion, "[%s] C-version", __FUNCTION__);

        for(int j=0; j<rAAOBufConfig.m_i4BlkNumY; j++)
        {
            if(rAAOBuf.m_pAAOSepAWBBuf!=0)//awb
            {
                memcpy(pawb, pSrc, rAAOBufConfig.m_i4BlkNumX*4);
                pawb+=rAAOBufConfig.m_i4BlkNumX*4;
            }//awb end


            if(rAAOBuf.m_pAAOSepAEBuf!=0)//ae
            {
                if(rAAOBufConfig.m_bEnableHDRYConfig)
                {
                    const MUINT8* pAeLSB = ((const MUINT8*)pSrc + rAAOBufConfig.m_i4AwbLineSize+rAAOBufConfig.m_i4BlkNumX);
                    const MUINT8* pAeMSB = ((const MUINT8*)pSrc + rAAOBufConfig.m_i4AwbLineSize);

                    for(int k=0;k<(rAAOBufConfig.m_i4BlkNumX);k++)
                    {
                        MUINT16 LSBbit50 = (*(pAeLSB+k))& 0x3F;//0~5 bit
                        //MUINT16 LSBbit76 = (*(pAeLSB+k))>>2;//6~7 bit
                        MUINT16 MSBfor50 = (*(pAeMSB+k));//0~7 bit

                        *(pae+k) = ((MSBfor50 - (LSBbit50 >= 32)) << 6) | LSBbit50;//((c4 & 0x8) ? (((c8-1)<<4)|c4) : ((c8<<4)|c4))
                    }
                    pae+=rAAOBufConfig.m_i4BlkNumX;
                }
                else
                {
                    for(int k=0;k<(rAAOBufConfig.m_i4BlkNumX/2);k++)
                    {
                        MUINT16 u2FirstByte = (*(pSrc+rAAOBufConfig.m_i4AwbLineSize+2*k));
                        MUINT16 u2SecondByte = (*(pSrc+rAAOBufConfig.m_i4AwbLineSize+2*k+1));
                        *(pae+2*k)  = u2FirstByte;
                        *(pae+2*k+1)= u2SecondByte;
                    }
                    pae+=rAAOBufConfig.m_i4BlkNumX;
                }
            }//ae end
            if(rAAOBuf.m_pAAOSepOverCntBuf!=0)//overexpcnt
            {
                if(rAAOBufConfig.m_bEnableOverCntConfig)
                {
                    const MUINT8* pover = ((const MUINT8*)pSrc + u4aaoNoAWBAESz);

                    for (int k=0; k<rAAOBufConfig.m_i4BlkNumX; k++){
                         *(poverexpcnt+k)=*(pover+k);
                         if (rAAOBufConfig.m_u4AEOverExpCntShift){
                            *(poverexpcnt+k)= (*(poverexpcnt+k))*2;
                         }
                    }
                    poverexpcnt+=rAAOBufConfig.m_i4BlkNumX;
                }else{
                    for (int k=0; k<rAAOBufConfig.m_i4BlkNumX; k++){
                         *(poverexpcnt+k)= 0;
                    }
                    poverexpcnt+=rAAOBufConfig.m_i4BlkNumX;
                }
            }//overexpcnt end

            if(rAAOBuf.m_pAAOSepLSCBuf!=0)//lsc
            {
                if (rAAOBufConfig.m_bEnableTSFConfig)    // c4 is available
                {
                    const MUINT8*  pTmpSrc = pSrc;
                    const MUINT32* pTsfSrc = (const MUINT32*)((const MUINT8*)pSrc + rAAOBufConfig.m_i4SkipSize);
                    MINT32 x = rAAOBufConfig.m_i4BlkNumX * 4;

                    while (x >= 8)
                    {
                        MUINT16 c8;
                        MUINT16 c56;
                        MUINT32 res = *pTsfSrc++;
                        #define CEXTRACT56(bit,offset)\
                            c8 = *pTmpSrc++;\
                            c56 = (res & (((1<<(bit))-1))); res >>= (bit);\
                            *plsc++ = (((c56 & ((1<<((bit)-1)))) ? (((c8-1)<<(bit))|c56) : ((c8<<(bit))|c56)) << (offset)); //shift 16 bits
                        #define CEXTRACT\
                            c8 = *pTmpSrc++;\
                            *plsc++ = (c8 << 8);// shift 16 bits

                        CEXTRACT56(5,3);
                        CEXTRACT56(6,2);
                        CEXTRACT56(5,3);
                        CEXTRACT
                        CEXTRACT56(5,3);
                        CEXTRACT56(6,2);
                        CEXTRACT56(5,3);
                        CEXTRACT
                        x -= 8;
                    }

                    MUINT32 last_res = *pTsfSrc;
                    while (x) // multiple of 4
                    {
                        MUINT16 c8;
                        MUINT16 c56;
                        #define EXTRACT56(bit,offset)\
                            c8 = *pTmpSrc++;\
                            c56 = (last_res & (((1<<(bit))-1))); last_res >>= (bit);\
                            *plsc++ = (((c56 & ((1<<((bit)-1)))) ? (((c8-1)<<(bit))|c56) : ((c8<<(bit))|c56)) << (offset)); //shift 16 bits
                        #define EXTRACT\
                            c8 = *pTmpSrc++;\
                            *plsc++ = (c8 << 8);// shift 14 bits

                        EXTRACT56(5,3);
                        EXTRACT56(6,2);
                        EXTRACT56(5,3);
                        EXTRACT
                        x -= 4;
                    }
                }
                else    // c4 is not available
                {
                    const MUINT8*  pTmpSrc = pSrc;
                    MINT32 x = rAAOBufConfig.m_i4BlkNumX * 4;
                    while (x >= 8)
                    {
                        *plsc++ = ((*pTmpSrc++) << 8);// shift 16 bits
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);
                        *plsc++ = ((*pTmpSrc++) << 8);

                        x -= 8;
                    }
                    while (x)
                    {
                        *plsc++ = ((*pTmpSrc++) << 8);
                        x--;
                    }
                }

            }//lsc end

            pSrc += rAAOBufConfig.m_i4LineSize;
        }//h end

        if(rAAOBuf.m_pAAOSepHistBuf!=0)//hist
        {
            int* pHist = (int*)(rAAOBuf.m_pAAOSepHistBuf);
            pSrc = (MUINT8*)AAOSrc;
            int aaoNoHistSz = rAAOBufConfig.m_i4LineSize*rAAOBufConfig.m_i4BlkNumY;
            pSrc+=aaoNoHistSz;
            if (!rAAOBufConfig.m_bDisablePixelHistConfig){
               for(int j=0;j<AAO_STT_HIST_NUM;j++)
               {
                 for(int i=0;i<AAO_STT_HIST_BIN_NUM;i++)
                 {
                   MUINT32 v;
                   MUINT32 b1;
                   MUINT32 b2;
                   MUINT32 b3;
                   b1 = *pSrc;
                   b2 = *(pSrc+1);
                   b3 = *(pSrc+2);
                   //v = b1+(b2<<8)+(b3<<16);
                   v = b1|(b2<<8)|(b3<<16);
                   pSrc+=3;
                   *pHist=v;
                   pHist++;
                 }
               }
            }else{
               for(int j=0;j<AAO_STT_HIST_NUM;j++)//4
               {
                 for(int i=0;i<AAO_STT_HIST_BIN_NUM;i++)
                 {
                   int v;
                   int b1;
                   int b2;
                   b1 = *pSrc;
                   b2 = *(pSrc+1);
                   //v = b1+(b2<<8);
                   v = b1|(b2<<8);
                   pSrc+=2;
                   *pHist=v;
                   pHist++;
                 }
               }
            }
        }//hist end
    }

    return MTRUE;
}

