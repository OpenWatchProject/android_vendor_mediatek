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

#include "camera_custom_types.h"
#include "aaa_scheduling_custom.h"

static MUINT32 cycleCtr_Sub = -1;

WorkPerCycle getWorkPerCycle_Sub_M1() //M = fps/30 = 1, -> 30 fps
{
    #define M1_CYCLE_NUM    3
    static WorkPerCycle WPC_M1[M1_CYCLE_NUM] = 
    {
        //1st cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   1,   1,   E_AE_AE_CALC|E_AE_FLARE|E_AE_AE_APPLY},
                
                    //the following part is "don't care"
    /*frameidx 2*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            1 //mValidFrameIdx, idx start from 1
        },
        
        //2nd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   1,   1,   E_AE_FLARE|E_AE_AE_APPLY},
                
                    //the following part is "don't care"
    /*frameidx 2*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            1 //mValidFrameIdx, idx start from 1
        },

        //3rd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   1,   1,   E_AE_FLARE|E_AE_AE_APPLY},
                
                    //the following part is "don't care"
    /*frameidx 2*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            1 //mValidFrameIdx, idx start from 1
        }
    };
    
    cycleCtr_Sub = (cycleCtr_Sub+1)%M1_CYCLE_NUM;
    return WPC_M1[cycleCtr_Sub];    
}

WorkPerCycle getWorkPerCycle_Sub_M2() //M = fps/30 = 2, -> 60 fps
{
    #define M2_CYCLE_NUM    3
    static WorkPerCycle WPC_M2[M2_CYCLE_NUM] = 
    {
        //1st cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   0,   1,   1,   0,   E_AE_AE_CALC|E_AE_FLARE|E_AE_AE_APPLY},
    /*frameidx 2*/    {    0,   1,   0,   1,   1,   E_AE_AE_APPLY},
                    
                    //the following part is "don't care"
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            2 //mValidFrameIdx, idx start from 1
        },
        
        //2nd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   0,   1,   1,   0,   E_AE_FLARE},
    /*frameidx 2*/    {    0,   1,   0,   1,   1,   E_AE_IDLE},
                    
                    //the following part is "don't care"
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            2 //mValidFrameIdx, idx start from 1
        },

        //3rd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   0,   1,   1,   0,   E_AE_FLARE},
    /*frameidx 2*/    {    0,   1,   0,   1,   1,   E_AE_IDLE},
                    
                    //the following part is "don't care"
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            2 //mValidFrameIdx, idx start from 1
        }
    };
    
    cycleCtr_Sub = (cycleCtr_Sub+1)%M2_CYCLE_NUM;
    return WPC_M2[cycleCtr_Sub];    
}

WorkPerCycle getWorkPerCycle_Sub_M3() //M = fps/30 = 3, -> 90 fps
{
    #define M3_CYCLE_NUM    3
    static WorkPerCycle WPC_M3[M3_CYCLE_NUM] = 
    {
        //1st cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_AE_CALC|E_AE_FLARE},
    /*frameidx 2*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},
    /*frameidx 3*/    {    0,   0,   0,   0,   1,   E_AE_AE_APPLY},
                    
                    //the following part is "don't care"
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            3 //mValidFrameIdx, idx start from 1
        },
        
        //2nd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_IDLE},
    /*frameidx 2*/    {    0,   0,   0,   0,   1,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
                    
                    //the following part is "don't care"
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            2 //mValidFrameIdx, idx start from 1
        },

        //3rd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_IDLE},
    /*frameidx 2*/    {    0,   0,   0,   0,   1,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
                    
                    //the following part is "don't care"
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE}
            },
            2 //mValidFrameIdx, idx start from 1
        }
    };
    
    cycleCtr_Sub = (cycleCtr_Sub+1)%M3_CYCLE_NUM;
    return WPC_M3[cycleCtr_Sub];    
}

WorkPerCycle getWorkPerCycle_Sub_M4() //M = fps/30 = 4, -> 120 fps
{
    #define M4_CYCLE_NUM    3
    static WorkPerCycle WPC_M4[M4_CYCLE_NUM] = 
    {
        //1st cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_AE_CALC|E_AE_FLARE},
    /*frameidx 2*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},
    /*frameidx 3*/    {    0,   0,   0,   0,   1,   E_AE_AE_APPLY},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},
                
                    //the following part is "don't care"
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
            },
            3 //mValidFrameIdx, idx start from 1
        },
        
        //2nd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_IDLE},
    /*frameidx 2*/    {    0,   0,   0,   0,   1,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
                
                    //the following part is "don't care"
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
            },
            2 //mValidFrameIdx, idx start from 1
        },

        //3rd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_IDLE},
    /*frameidx 2*/    {    0,   0,   0,   0,   1,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
                
                    //the following part is "don't care"
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
            },
            2 //mValidFrameIdx, idx start from 1
        }
    };
    
    cycleCtr_Sub = (cycleCtr_Sub+1)%M4_CYCLE_NUM;
    return WPC_M4[cycleCtr_Sub];    
}

WorkPerCycle getWorkPerCycle_Sub_M5() //M = fps/30 = 5, -> 150 fps
{
    #define M5_CYCLE_NUM    3
    static WorkPerCycle WPC_M5[M5_CYCLE_NUM] = 
    {
        //1st cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_AE_CALC|E_AE_FLARE},
    /*frameidx 2*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},
    /*frameidx 3*/    {    0,   0,   0,   0,   1,   E_AE_AE_APPLY},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},                
                    
                    //the following part is "don't care"
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
            },
            3 //mValidFrameIdx, idx start from 1
        },
        
        //2nd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_IDLE},
    /*frameidx 2*/    {    0,   0,   0,   0,   1,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
                    
                    //the following part is "don't care"
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
            },
            2 //mValidFrameIdx, idx start from 1
        },

        //3rd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_IDLE},
    /*frameidx 2*/    {    0,   0,   0,   0,   1,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
                    
                    //the following part is "don't care"
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
            },
            2 //mValidFrameIdx, idx start from 1
        }
    };
    
    cycleCtr_Sub = (cycleCtr_Sub+1)%M5_CYCLE_NUM;
    return WPC_M5[cycleCtr_Sub];    
}

WorkPerCycle getWorkPerCycle_Sub_M6() //M = fps/30 = 6, -> 180 fps
{
    #define M6_CYCLE_NUM    3
    static WorkPerCycle WPC_M6[M6_CYCLE_NUM] = 
    {
        //1st cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_AE_CALC|E_AE_FLARE},
    /*frameidx 2*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},
    /*frameidx 3*/    {    0,   0,   0,   0,   1,   E_AE_AE_APPLY},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},                
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_AE_APPLY},
                    //the following part is "don't care"
            },
            3 //mValidFrameIdx, idx start from 1
        },
        
        //2nd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_IDLE},
    /*frameidx 2*/    {    0,   0,   0,   0,   1,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
                    //the following part is "don't care"
            },
            2 //mValidFrameIdx, idx start from 1
        },

        //3rd cycle
        {
            {
                    //format:
    /*frameidx  */  //{  AAO, AWB,  AF, Flk, Lsc,   AE&Flare 
    /*frameidx 1*/    {    1,   1,   1,   0,   0,   E_AE_IDLE},
    /*frameidx 2*/    {    0,   0,   0,   0,   1,   E_AE_IDLE},
    /*frameidx 3*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 4*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 5*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
    /*frameidx 6*/    {    0,   0,   0,   0,   0,   E_AE_IDLE},
                    //the following part is "don't care"
            },
            2 //mValidFrameIdx, idx start from 1
        }
    };
    
    cycleCtr_Sub = (cycleCtr_Sub+1)%M6_CYCLE_NUM;
    return WPC_M6[cycleCtr_Sub];    
}






WorkPerCycle getWorkPerCycle_Sub(int normalizeM) // M = fps/30
{
    typedef WorkPerCycle(*FUNC_PTR)(); 
    static FUNC_PTR arr[MAX_FRAME_PER_CYCLE] = 
    {
        getWorkPerCycle_Sub_M1, //for 30 fps
        getWorkPerCycle_Sub_M2, //for 60 fps
        getWorkPerCycle_Sub_M3, //for 90 fps
        getWorkPerCycle_Sub_M4, //for 120 fps
        getWorkPerCycle_Sub_M5, //for 150 fps
        getWorkPerCycle_Sub_M6  //for 180 fps
    };
    return (*arr[normalizeM-1])();
}

MVOID resetCycleCtr_Sub()
{
    cycleCtr_Sub = -1;
}




