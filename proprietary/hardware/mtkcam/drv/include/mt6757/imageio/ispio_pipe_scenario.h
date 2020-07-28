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
#ifndef _ISPIO_PIPE_SCENARIO_H_
#define _ISPIO_PIPE_SCENARIO_H_


/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Driver Scenario
********************************************************************************/
enum EDrvScenario
{
    //
    eDrvScenario_P2A = 0,      //  normal, P2A path
    //
    eDrvScenario_VFB_FB = 1,       //  FB, P2B path
    //
    eDrvScenario_MFB_Blending = 2 ,          //  blending, P2A
    //
    eDrvScenario_MFB_Mixing = 3,      //  mixing
    //
    eDrvScenario_VSS = 4,                //  vss, P2A
    //
    eDrvScenario_P2B_Bokeh = 5,     //  Boken, P2B
    //
    eDrvScenario_FE = 6,            // including FE module
    //
    eDrvScenario_FM = 7,            // including FM module
     //
    eDrvScenario_Color_Effect = 8,            // specific color effect path
    //
    eDrvScenario_DeNoise = 9,            // specific denoise path
    //
    eScenarioID_NUM
};


/*******************************************************************************
* Scenario Format
********************************************************************************/
enum EScenarioFmt
{
    //
    eScenarioFmt_RAW,       //
    //
    eScenarioFmt_YUV,       //
    //
    eScenarioFmt_RGB,       //
    //
    eScenarioFmt_FG,        //
    //
    eScenarioFmt_UFO,       //
    //
    eScenarioFmt_JPG,       //
    //
    eScenarioFmt_MFB,       //
    //
    eScenarioFmt_RGB_LOAD,  //
    //
    eScenarioFmt_UNKNOWN    = 0xFFFFFFFF,   //
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_SCENARIO_H_

