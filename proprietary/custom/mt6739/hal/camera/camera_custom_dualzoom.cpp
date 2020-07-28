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

#include "camera_custom_dualzoom.h"
#include "camera_custom_dualzoom_func.h"



int RatTBL10cm[21][2]=
{  //wide macro OTP pattern distance =10cm
{500,  0   },
{300,  13  },
{200,  30  },
{150,  46  },
{120,  62  },
{100,  79  },
{90 ,  90  },
{80 ,  104 },
{70 ,  121 },
{60 ,  145 },
{50 ,  178 },
{40 ,  228 },
{35 ,  264 },
{30 ,  312 },
{25 ,  379 },
{20 ,  481 },
{18 ,  538 },
{16 ,  609 },
{14 ,  701 },
{12 ,  825 },
{10 ,  1000}};

int RatTBL9cm[22][2]=
{  //wide macro OTP pattern distance =9cm
{500,     0   },
{300,     12  },
{200,     26  },
{150,     41  },
{120,     56  },
{100,     70  },
{90    ,  80  },
{80    ,  92  },
{70    ,  108 },
{60    ,  129 },
{50    ,  159 },
{40    ,  204 },
{35    ,  236 },
{30    ,  278 },
{25    ,  338 },
{20    ,  429 },
{18    ,  480 },
{16    ,  544 },
{14    ,  626 },
{12    ,  737 },
{10    ,  894 },
{9  ,  1000}};

int RatTBL8cm[23][2]=
{ //wide macro OTP pattern distance =8cm
{500,  0   },
{300,  10  },
{200,  23  },
{150,  36  },
{120,  49  },
{100,  62  },
{90 ,  71  },
{80 ,  82  },
{70 ,  96  },
{60 ,  114 },
{50 ,  140 },
{40 ,  180 },
{35 ,  208 },
{30 ,  245 },
{25 ,  298 },
{20 ,  379 },
{18 ,  423 },
{16 ,  480 },
{14 ,  552 },
{12 ,  650 },
{10 ,  789 },
{9  ,  882 },
{8  ,  1000}};


void get_Dualzoom_Switch_ratio(int TMacDist, int WMacDist, int &highThRat, int &lowThRat)
{
    int x = 1;
    switch (WMacDist)
    {
    case 10: //wide macro pattern distance 10cm
        for (x = 1; x < sizeof(RatTBL10cm) / 2 / sizeof(int); x++)
        {
            if (TMacDist > RatTBL10cm[x][0])
            {
                lowThRat  = RatTBL10cm[x-1][1];
                highThRat = RatTBL10cm[x  ][1];
                break;
            }
        }
        break;
    case 9://wide macro pattern distance 9cm
        for (x = 1; x < sizeof(RatTBL9cm) / 2 / sizeof(int); x++)
        {
            if (TMacDist > RatTBL9cm[x][0])
            {
                lowThRat  = RatTBL9cm[x-1][1];
                highThRat = RatTBL9cm[x  ][1];
                break;
            }
        }
        break;
    case 8://wide macro pattern distance 8cm
        for (x = 1; x < sizeof(RatTBL8cm) / 2 / sizeof(int); x++)
        {
            if (TMacDist > RatTBL8cm[x][0])
            {
                lowThRat  = RatTBL8cm[x-1][1];
                highThRat = RatTBL8cm[x  ][1];
                break;
            }
        }
        break;
    }
}

void get_Dualzoom_AF_Thresholds(int infinity, int macro, int &highTh, int &lowTh, int WMacDist, int TMacDist)
{
    int lowThRat=0, highThRat=0;

    get_Dualzoom_Switch_ratio(TMacDist, WMacDist, lowThRat, highThRat);

    if (infinity != 0 && macro != 0)
    {
        // calculate about 40cm DAC
        highTh = infinity + ((macro - infinity) * lowThRat / 1000);
        // calculate about 60cm DAC
        lowTh  = infinity + ((macro - infinity) * highThRat / 1000);
    }
    else
    {
        highTh = DUALZOOM_AF_DAC_HIGH_THRESHOLD;
        lowTh  = DUALZOOM_AF_DAC_LOW_THRESHOLD;
    }
}

void get_Dualzoom_AF_TeleDACTh(int &DacTh)
{
    DacTh = 400;
    return;
}
