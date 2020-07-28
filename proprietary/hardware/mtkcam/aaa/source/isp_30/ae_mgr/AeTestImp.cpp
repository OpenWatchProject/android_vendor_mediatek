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
#define LOG_TAG "ae_test"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "IAeTest.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
#include <aaa_sensor_mgr.h>
#include <isp_tuning_mgr.h>








#define AE_TEST_LOG(fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)
// AE Auto Test
#define AE_AT_MAX_CNT_NUM   10
        
        
using namespace NS3Av3;
using namespace NSIspTuningv3;



static MUINT32  AEAutoTestThreshTable[]= {
  0,
  15, // for testing ISO change
  8, // for testing scene change
  5, // for testing EV comp change
  8, // for testing preview->video
  8 // for testing preview->capture
};


class AETestImp : public IAeTest
{
public:
     static IAeTest*                           getInstance(ESensorDev_T sensor);

     virtual MVOID                             AeTestInit();
     virtual MVOID                             monitorAvgY(MINT32 avgy);
     virtual MVOID                             recordAvgY(E_AE_TEST_T testtype ,MINT32 avgy);
     virtual MVOID                             resetAvgY(E_AE_TEST_T testtype ,MINT32 avgy);
     virtual MBOOL                             IsPerframeCtrl();
     virtual MVOID                             PerframeCtrl(MINT32 avgy, MBOOL istypeAsensor, ESensorMode_T eSensorMode,
                                                            AE_PLINETABLE_T* pAEPlineTable, strFinerEvPline* pPreviewTableF, MUINT32 u4IndexFMax);

protected:
                                    AETestImp(ESensorDev_T sensor);
    virtual                         ~AETestImp();

    ESensorDev_T m_eSensorDev;

    // AE Auto Test
    MBOOL m_bEnAutoTest;
    MUINT32 m_u4ATCnt;
    MUINT32 m_u4ATYval;
    E_AE_TEST_T m_eATType;
    MUINT32 m_u4AdbAEPerFrameCtrl;
    MUINT32 m_u4IndexTestCnt;


};


IAeTest*
IAeTest::
getInstance(ESensorDev_T sensor)
{
    return AETestImp::getInstance(sensor);
}

IAeTest*
AETestImp::
getInstance(ESensorDev_T sensor)
{
    AE_TEST_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    switch (sensor)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AETestImp singleton_main(ESensorDev_Main);
        AE_TEST_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AETestImp singleton_main2(ESensorDev_MainSecond);
        AE_TEST_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_MainThird:  //  Main Second Sensor
        static AETestImp singleton_main3(ESensorDev_MainThird);
        AE_TEST_LOG("ESensorDev_MainThird(%p)", &singleton_main3);
        return &singleton_main3;
    case ESensorDev_Sub:        //  Sub Sensor
        static AETestImp singleton_sub(ESensorDev_Sub);
        AE_TEST_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:  //  Sub2 Sensor
        static AETestImp singleton_sub2(ESensorDev_SubSecond);
        AE_TEST_LOG("ESensorDev_SubSecond(%p)", &singleton_sub2);
        return &singleton_sub2;
    }
}

AETestImp::
AETestImp(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_bEnAutoTest(0)
    , m_u4ATCnt(0)
    , m_u4ATYval(0)
    , m_eATType(AE_AT_TYPE_NON)
    , m_u4AdbAEPerFrameCtrl(0)
    , m_u4IndexTestCnt(0)
{
    AE_TEST_LOG("Enter AETestImp - DeviceId:%d",(MUINT32)m_eSensorDev);
}

AETestImp::
~AETestImp()
{
    AE_TEST_LOG("Delete ~AETestImp - DeviceId:%d",(MUINT32)m_eSensorDev);
}

MVOID
AETestImp::
AeTestInit()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    // AE Auto Test
    property_get("vendor.debug.ae.autoTest.enable", value, "0");
    m_bEnAutoTest = atoi(value);
    // AE Perframe control test
    m_u4IndexTestCnt = 0;
    property_get("vendor.debug.ae.perframeCtrl.enable", value, "0");
    m_u4AdbAEPerFrameCtrl = atoi(value);

}


MVOID
AETestImp::
monitorAvgY(MINT32 avgy)
{
    if(m_bEnAutoTest) {
        if(m_eATType != AE_AT_TYPE_NON && m_eATType != AE_AT_TYPE_Pre2Cap) {
            MUINT32 u4AvgYcur = (MUINT32)avgy;
            switch(m_eATType) {
                case AE_AT_TYPE_ISO:
                    if ((u4AvgYcur<(m_u4ATYval-m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100))||(u4AvgYcur>(m_u4ATYval+m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100)))
                        AE_TEST_LOG( "AE Auto Test - ISO failed\n");
                    else
                        AE_TEST_LOG( "[%s()] u4AvgYcur(%d) type(%d) count(%d) - ISO Pass!\n",__FUNCTION__ , u4AvgYcur, m_eATType, m_u4ATCnt);
                    break;
                case AE_AT_TYPE_SCENE:
                    if ((u4AvgYcur<(m_u4ATYval-m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100))||(u4AvgYcur>(m_u4ATYval+m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100)))
                        AE_TEST_LOG( "AE Auto Test - Scene mode failed\n");
                    else
                        AE_TEST_LOG( "[%s()] u4AvgYcur(%d) type(%d) count(%d) - Scene mode Pass!\n",__FUNCTION__ , u4AvgYcur, m_eATType, m_u4ATCnt);
                    break;
                case AE_AT_TYPE_Pre2Video:
                    if ((u4AvgYcur<(m_u4ATYval-m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100))||(u4AvgYcur>(m_u4ATYval+m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100)))
                        AE_TEST_LOG( "AE Auto Test - Pre/Video failed\n");
                    else
                        AE_TEST_LOG( "[%s()] u4AvgYcur(%d) type(%d) count(%d) - Pre/Video Pass!\n",__FUNCTION__ , u4AvgYcur, m_eATType, m_u4ATCnt);
                    break;
                case AE_AT_TYPE_EVCOMP:
                    if(m_u4ATCnt < (AE_AT_MAX_CNT_NUM-3)) {
                        if ((u4AvgYcur>(m_u4ATYval-m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100))&&(u4AvgYcur<(m_u4ATYval+m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100)))
                            AE_TEST_LOG( "AE Auto Test - EV comp failed\n");
                        else
                            AE_TEST_LOG( "[%s()] u4AvgYcur(%d) type(%d) count(%d) - EV comp Pass!\n",__FUNCTION__ , u4AvgYcur, m_eATType, m_u4ATCnt);
                    }
                    break;
                default:
                    break;
            }
            m_u4ATCnt --;
            if(m_u4ATCnt <= 0) {
                m_u4ATCnt = 0;
                m_u4ATYval = 0;
                m_eATType = AE_AT_TYPE_NON;
                AE_TEST_LOG( "[%s()] - Reset type\n",__FUNCTION__);
            }
        }
    }

}

MVOID
AETestImp::
recordAvgY(E_AE_TEST_T testtype ,MINT32 avgy)
{
    if(m_bEnAutoTest) {
        m_u4ATCnt = AE_AT_MAX_CNT_NUM;
        m_u4ATYval = (MUINT32)avgy;
        m_eATType = testtype;
        AE_TEST_LOG( "[%s()] recordY(%d) thres(%d) type(%d)\n",__FUNCTION__, m_u4ATYval, AEAutoTestThreshTable[m_eATType], m_eATType);
    }
}


MVOID
AETestImp::
resetAvgY(E_AE_TEST_T testtype ,MINT32 avgy)
{
    if(m_bEnAutoTest) {
        if(m_eATType == testtype) {
            MUINT32 u4AvgYcur = (MUINT32)avgy;
            //AE_TEST_LOG( "[%s()] u4AvgYcur(%d) type(%d)\n",__FUNCTION__, u4AvgYcur, m_eATType);
            if ((u4AvgYcur<(m_u4ATYval-m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100))||(u4AvgYcur>(m_u4ATYval+m_u4ATYval*AEAutoTestThreshTable[m_eATType]/100)))
                AE_TEST_LOG( "AE Auto Test - Pre/Cap failed\n");
            else
                AE_TEST_LOG( "[%s()] u4AvgYcur(%d) type(%d) count(%d) - Pre/Cap Pass!\n",__FUNCTION__ , u4AvgYcur, m_eATType, m_u4ATCnt);
            m_u4ATCnt = 0;
            m_u4ATYval = 0;
            m_eATType = AE_AT_TYPE_NON;
            AE_TEST_LOG( "[%s()] Reset type\n",__FUNCTION__);
        }
    }
}

MBOOL
AETestImp::
IsPerframeCtrl()
{
    return (m_u4AdbAEPerFrameCtrl != 0);

}

MVOID
AETestImp::
PerframeCtrl(MINT32 avgy, MBOOL istypeAsensor, ESensorMode_T eSensorMode,
             AE_PLINETABLE_T* pAEPlineTable, strFinerEvPline* pPreviewTableF, MUINT32 u4IndexFMax)
{
    AE_OUTPUT_T m_rAEOutput;
    memset(&m_rAEOutput, 0, sizeof(AE_OUTPUT_T));

    MUINT32 u4TestStep = 10;
    MUINT32 u4Testidx = m_u4AdbAEPerFrameCtrl;
    MUINT32 u4TestCnt = 300;
    if(u4Testidx == 31) u4TestCnt = pAEPlineTable->AEGainList.u2TotalNum * u4TestStep + u4TestStep;
    else if(u4Testidx == 32) u4TestCnt = u4IndexFMax * u4TestStep + u4TestStep;

    if(m_u4IndexTestCnt < u4TestCnt) {

        switch(u4Testidx) {
                case 1: // Shutter delay frames [Exp:10ms <-> 20ms, 1x ]
                    if( m_u4IndexTestCnt%20 < u4TestStep ) {
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
                    } else {
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000*2;
                        m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
                    }
                    break;
                case 2: // Gain delay frames [Gain:2x <-> 4x , 10ms ]
                    if( m_u4IndexTestCnt%20 < u4TestStep ) {
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2;
                    } else {
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        m_rAEOutput.rPreviewMode.u4AfeGain = 1024*4;
                    }
                    break;
                case 3: // Delay frames IT [20ms/2x <-> 10ms/4x ]
                    if( m_u4IndexTestCnt%20 < u4TestStep ) { // 0-9 20ms/1x
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 20000;
                        if(istypeAsensor) { //0-9 1x
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2;
                        } else if(m_u4IndexTestCnt%20 == 0){ // 0 2x
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*4;
                        } else{                              // 1-9 1x
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2;
                        }
                    } else {                        //10-19 10ms/2x
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        if(istypeAsensor) { //10-19 2x
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*4;
                        } else if(m_u4IndexTestCnt%20 == u4TestStep){ // 10 1x
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2;
                        } else{                              // 11-19 2x
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*4;
                        }
                    }
                    break;
                case 4: // Perframe shutter command [10ms<->20ms,1x]
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000*(m_u4IndexTestCnt%2 + 1);
                        m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
                    break;
                case 5: // Perframe gain command [2x<->4x,10ms]
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2*(m_u4IndexTestCnt%2 + 1);
                    break;
                case 6: // Perframe command IT [10ms/6x<->30ms/2x]
                    if(m_u4IndexTestCnt%2 == 0) {
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        if(istypeAsensor) {
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*6;
                        } else {
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2;
                        }
                    } else if(m_u4IndexTestCnt%2 == 1) {
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000*3;
                        if(istypeAsensor) {
                             m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2;
                        } else {
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*6;
                        }
                    }
                    break;
                case 7: // Perframe framerate command [10ms/5x <-> 50ms/1x]
                    if(m_u4IndexTestCnt%2 == 0) {
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        if(istypeAsensor) {
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*5;
                        } else {
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
                        }
                    } else if(m_u4IndexTestCnt%2 == 1) {
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000*5;
                        if(istypeAsensor) {
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
                        } else {
                            m_rAEOutput.rPreviewMode.u4AfeGain = 1024*5;
                        }
                    }
                    break;
                case 8: // Shutter Linearity
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 5000*((m_u4IndexTestCnt%160)/u4TestStep + 1);
                        m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
                    break;
                case 9: // Gain Linearity
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        m_rAEOutput.rPreviewMode.u4AfeGain = 1024*((m_u4IndexTestCnt%160)/u4TestStep + 1);
                    break;
                case 10: // Frame rate control
                    m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                    m_rAEOutput.rPreviewMode.u4AfeGain = 2048;
                    if(m_u4IndexTestCnt%2 == 0) {
                        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, 300, eSensorMode);
                    } else if(m_u4IndexTestCnt%2 == 1) {
                        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, 150, eSensorMode);
                    }
                    break;
                case 31: // Gain Table Linearity
                        m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
                        m_rAEOutput.rPreviewMode.u4AfeGain = pAEPlineTable->AEGainList.u2SensorGainStep[(m_u4IndexTestCnt/u4TestStep) % pAEPlineTable->AEGainList.u2TotalNum][0]; //1024*((m_u4IndexTestCnt%160)/10 + 1);
                    break;
                case 32: // Finer-EV Pline Linearity
                        m_rAEOutput.rPreviewMode.u4Eposuretime = pPreviewTableF->sPlineTable[(m_u4IndexTestCnt/u4TestStep) % (u4IndexFMax+1)].u4Eposuretime;
                        m_rAEOutput.rPreviewMode.u4AfeGain = pPreviewTableF->sPlineTable[(m_u4IndexTestCnt/u4TestStep) % (u4IndexFMax+1)].u4AfeGain;
                        m_rAEOutput.rPreviewMode.u4IspGain = pPreviewTableF->sPlineTable[(m_u4IndexTestCnt/u4TestStep) % (u4IndexFMax+1)].u4IspGain;
                    break;
                default:
                    AE_TEST_LOG( "[startSensorPerFrameControl] Test item:%d isn't supported \n", u4Testidx);
                    break;
            }

        if(u4Testidx != 32) m_rAEOutput.rPreviewMode.u4IspGain = 1024;
        m_u4IndexTestCnt++;
    } else {
        m_rAEOutput.rPreviewMode.u4Eposuretime = 1000;
        m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
        m_rAEOutput.rPreviewMode.u4IspGain = 1024;
    }
    AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, m_rAEOutput.rPreviewMode.u4Eposuretime);
    AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, m_rAEOutput.rPreviewMode.u4AfeGain);
    IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, m_rAEOutput.rPreviewMode.u4IspGain>>1);
    AE_TEST_LOG( "[startSensorPerFrameControl] testid:%d u4Index:%03d avgY:%03d Shutter:%d Gain:%d %d\n", u4Testidx , m_u4IndexTestCnt-1, avgy, m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain);

}


