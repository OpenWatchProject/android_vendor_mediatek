
#include <stdio.h>
#include <stdlib.h>

#include "FakeDrvSensor.h"

#define SENSOR_PREVIEW_MODE     (0)
#define SENSOR_CAPTURE_MODE     (1)
#define SENSOR_MODE             (1)

//4176; 2088; 1600;
//3088; 1544; 1200;
#if (SENSOR_MODE == SENSOR_PREVIEW_MODE)
#define SENSOR_WIDTH        (1600)
#define SENSOR_HEIGHT       (1200)
#define SENSOR_WIDTH_SUB    (1600)
#define SENSOR_HEIGHT_SUB   (1200)
#elif (SENSOR_MODE == SENSOR_CAPTURE_MODE)
#define SENSOR_WIDTH        (1600)//3200 4176
#define SENSOR_HEIGHT       (1200)//2400 3088
#define SENSOR_WIDTH_SUB    (1600)
#define SENSOR_HEIGHT_SUB   (1200)
#else
#define SENSOR_WIDTH        (1600)
#define SENSOR_HEIGHT       (1200)
#define SENSOR_WIDTH_SUB    (1600)
#define SENSOR_HEIGHT_SUB   (1200)
#endif

IspDrvImp   *TS_FakeDrvSensor::m_pIspDrv = NULL;
#define MAKE_SENINF_REG(_a, _r)     (_a - 0x14040000 + _r)


void TS_FakeDrvSensor::setPixelMode(MUINT32 pixMode)
{
    mSensorPixMode = pixMode;
}

void TS_FakeDrvSensor::adjustPatternSize(MUINT32 h_min)
{
    MUINT32 size_diff_threadhold = 0x100;
    #define H_BAR   0xC
    if (m_pIspDrv){
        MUINT32 pattern_type=0;
        MUINT32 reg = 0;
        MUINT32 h=0;

        m_pIspDrv->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only
        pattern_type = m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF_TG1_TM_CTL, 0));
        pattern_type = ((pattern_type >> 4)&0xf);
        if(pattern_type == H_BAR){
            reg = m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF_TG1_TM_SIZE, 0));
            h = (reg & 0xffff)>>16;

            if(h < h_min)
                h = h_min;

            if((h - h_min) < size_diff_threadhold){
                h += size_diff_threadhold;
                reg = ((reg & 0xffff)|(h<<16));
                m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF_TG1_TM_SIZE, 0), reg);
            }
        }
        else{
            TS_LOGD("error:this can only be applied on H-color-bar.Otherwise, bit-ture will be failed\n");
        }



        //
        m_pIspDrv->setRWMode(ISP_DRV_RW_MMAP);
    }
    else{
        TS_LOGD("error:NULL drv ptr, need to poweron 1st\n");
    }

}

static MUINT32 mPowerOnTM[] = {
        //0x200 <-> 0x100,0x500
        SENINF1_CTRL, 0x00001001, //SENINF1_CTRL
        //0xd00 <-> 0x120,0x520
        SENINF1_MUX_CTRL, 0x96DF1080, //SENINF1_MUX_CTRL
        SENINF1_MUX_INTEN, 0x8000007F, //SENINF1_MUX_INTEN(RST)
        SENINF1_MUX_SIZE, 0x00000000, //SENINF1_MUX_SIZE(RST)
        SENINF1_MUX_SPARE, 0x000E2000, //SENINF1_MUX_SPARE(RST)
        SENINF1_MUX_CROP, 0x00000000, //SENINF1_MUX_CROP(RST)

        //0x600 <-> 0x200
        SENINF_TG1_TM_CTL, 0x002804C1, // TG1_TM_CTL
                                // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
        SENINF_TG1_TM_SIZE, 0x0FA01F00, // TG1_TM_SIZE
        SENINF_TG1_TM_CLK, 0x00000008, // TG1_TM_CLK
        SENINF_TG1_TM_STP, 0x1        // always setting timestamp dividor to 1 for test only
};


MBOOL TS_FakeDrvSensor::powerOn(
        char const* szCallerName,
        MUINT32 const uCountOfIndex,
        MUINT32 const*pArrayOfIndex,
        MUINT32 const*pSetingTbl)
{
    MINT32 nNum = 0, i = 0, ret = 0;

    IspDrvImp *_pIspDrv = NULL;

    (void)szCallerName; (void)uCountOfIndex; (void)pArrayOfIndex;


    TS_LOGD("Read CAM_%s to apply pixel mode", (pArrayOfIndex[0] == 0) ? "A" : "B");
    _pIspDrv = (IspDrvImp*)IspDrvImp::createInstance((pArrayOfIndex[0] == 0) ? CAM_A : CAM_B);
    ret = _pIspDrv->init("FakeSensor");
    if (ret < 0) {
        TS_LOGD("Error: IspDrv init fail 0");
        goto _DESTROY_EXIT;
    }
    switch ((CAM_READ_BITS(_pIspDrv, CAM_TG_SEN_MODE, DBL_DATA_BUS1) << 1) | CAM_READ_BITS(_pIspDrv, CAM_TG_SEN_MODE, DBL_DATA_BUS)) {
    case 0x0:
        mSensorPixMode = 1;
        break;
    case 0x1:
        mSensorPixMode = 2;
        break;
    case 0x3:
        mSensorPixMode = 4;
        break;
    default:
        TS_LOGD("Error: wrong pixel mode %d_%d",
            ((CAM_READ_BITS(_pIspDrv, CAM_TG_SEN_MODE, DBL_DATA_BUS1) << 1)| CAM_READ_BITS(_pIspDrv, CAM_TG_SEN_MODE, DBL_DATA_BUS)));
        break;
    }
    _pIspDrv->uninit("FakeSensor");
    _pIspDrv->destroyInstance();

    TS_LOGD("PowerOn: %d, pixMode: %d", pArrayOfIndex[0], mSensorPixMode);

    /* alaska only seninf0 has TM */
#ifdef TODO_DE
    if (m_pIspDrv) {
#else
    if (0) {
#endif
        MUINT32 mux_ctl = 0;

        m_pIspDrv->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only

        mux_ctl = m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF_TOP_CAM_MUX_CTRL, 0));
        mux_ctl = mux_ctl & ~(0xF << (pArrayOfIndex[0] * 4));

        TS_LOGD("Update mux_ctl: 0x%x => 0x%x", m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF_TOP_CAM_MUX_CTRL, 0)), mux_ctl);

        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF_TOP_CAM_MUX_CTRL, 0), mux_ctl); //SENINF_TOP_CAM_MUX_CTRL

        m_pIspDrv->setRWMode(ISP_DRV_RW_MMAP);

        return MTRUE;
    }

    m_pIspDrv = (IspDrvImp*)IspDrvImp::createInstance((pArrayOfIndex[0] == 0) ? CAM_A : CAM_B);


    if (NULL == m_pIspDrv) {
        TS_LOGD("Error: IspDrv CreateInstace fail");
        goto _DESTROY_EXIT;
    }
    ret = m_pIspDrv->init("FakeSensor");
    if (ret < 0) {
        TS_LOGD("Error: IspDrv init fail");
        goto _DESTROY_EXIT;
    }
    m_pIspDrv->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only
    if (pSetingTbl != NULL) {
        MUINT32 num = 0, i = 0;
        MUINT32 reg = 0;

        for(i=0;pSetingTbl[i] != 0xdeadbeef;i++){
            num++;
        }

        TS_LOGD("#################################################\n");
        TS_LOGD("load test setting ...............................\n");
        TS_LOGD("#################################################\n");
#if 0
        for (i = 0; i < num; i+=2) {
            TS_LOGD(" [0x%08x]=0x%08x\n", pSetingTbl[i],
                m_pIspDrv->readReg(MAKE_SENINF_REG(pSetingTbl[i], 0)));
        }
        TS_LOGD("#################################################\n");
#endif
        TS_LOGD("total %d entry\n", num);
        for (i = 0; i < num; i+=2) {
            m_pIspDrv->writeReg(pSetingTbl[i], pSetingTbl[i+1]);
#if 1
            TS_LOGD(" [0x%08x]=0x%08x, 0x%08x", pSetingTbl[i], pSetingTbl[i+1],
                m_pIspDrv->readReg(pSetingTbl[i]));
#endif
        }

    }
    else{
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF_TOP_CTRL, 0), 0x00000C00); //SENINF_TOP_CTRL
        #if 1
        TS_LOGD("Set TM enable");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_CTRL_EXT, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_CTRL_EXT, 0)) | 0x02);//SENINF1_CTRL_EXT
        #endif
        TS_LOGD("#################################################\n");
        TS_LOGD("load default setting ............................\n");
        TS_LOGD("#################################################\n");

        nNum = (sizeof(mPowerOnTM) / sizeof(mPowerOnTM[0])) / 2;
        TS_LOGD("Total %d registers", nNum);

        for (i = 0; i < nNum; i++) {
            #if 0 //change test model pattern
            if (0x1A040608 == mPowerOnTM[i<<1]) {
                TS_LOGD("Seninf Test Mode : %d", (atoi(szCallerName) & 0xF));
                mPowerOnTM[(i<<1)+1] = (0x00040401 | ((atoi(szCallerName) & 0xF) << 4));
            }
            #endif
            m_pIspDrv->writeReg(MAKE_SENINF_REG(mPowerOnTM[i<<1], 0), mPowerOnTM[(i<<1)+1]);
        }
    }
#ifdef TODO_DE
#if 1
{
    MUINT32 reg = 0;
    reg = m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_CTRL, 0));
    m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_CTRL, 0), (reg|0x1));
    reg = m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_CTRL_EXT, 0));
    m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_CTRL_EXT, 0), (reg|0x40));
    reg = m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_CTRL_EXT, 0));
    m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_CTRL_EXT, 0), (reg|0x2));
    reg = m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF_TG1_TM_CTL, 0));
    m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF_TG1_TM_CTL, 0), (reg|0x1));
    //*SENINF1_CTRL      = *SENINF1_CTRL      | 0x1; // seninf_en
    //*SENINF1_CTRL_EXT  = *SENINF1_CTRL_EXT  | 0x40; // csi2_en
    //*SENINF1_CTRL_EXT  = *SENINF1_CTRL_EXT  | 0x2; // testmdl_en
    //*SENINF_TG1_TM_CTL = *SENINF_TG1_TM_CTL | 0x1;   // Prevent early start , TM_EN
}
#endif
    if(mSensorPixMode == 2) //two pixel mode
    {
        TS_LOGD("Set two pixel mode");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL, 0)) | 0x100);//SENINF1_MUX_CTRL
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL_EXT, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL_EXT, 0)) & ~0x10);//SENINF1_MUX_CTRL_EXT
    }
    else if (mSensorPixMode == 4)//four pixel mode
    {
        TS_LOGD("Set four pixel mode");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL, 0)) & ~0x100);
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL_EXT, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL_EXT, 0)) | 0x10);
    }
    else
    {
        TS_LOGD("Set one pixel mode");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL, 0)) & ~0x100);
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL_EXT, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF1_MUX_CTRL_EXT, 0)) & ~0x10);
    }
#endif
    if (0) {//(1 == bYUV)
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF_TG1_TM_CTL, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF_TG1_TM_CTL, 0)) | 0x4); //SENINF_TG1_TM_CTL
    }

    for (i = 0; i < nNum; i++) {
        TS_LOGD(" Reg[x%08x] = x%08x/x%08x", mPowerOnTM[i<<1],
            m_pIspDrv->readReg(MAKE_SENINF_REG(mPowerOnTM[i<<1], 0)), mPowerOnTM[(i<<1)+1]);
    }

    m_pIspDrv->setRWMode(ISP_DRV_RW_MMAP);

    return MTRUE;

_DESTROY_EXIT:
    if (m_pIspDrv) {
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }

    return MFALSE;
}

MBOOL TS_FakeDrvSensor::powerOff(
        char const* szCallerName,
        MUINT32 const uCountOfIndex,
        MUINT32 const*pArrayOfIndex)
{
#if 0    //this is for seninf hw version : rome style version
    MUINT32 regOffset = (pArrayOfIndex[0] == 0)? 0x0: 0x400;
#else
    MUINT32 regOffset = 0;
#endif
    (void)szCallerName; (void)uCountOfIndex; (void)pArrayOfIndex;

    if (m_pIspDrv) {
        TS_LOGD("Set TM disable");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(SENINF_TG1_TM_CTL, regOffset), (m_pIspDrv->readReg(MAKE_SENINF_REG(SENINF_TG1_TM_CTL, regOffset)) & ~0x01));
        m_pIspDrv->uninit("FakeSensor");
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }

    return MTRUE;
}

