
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <mtkcam/def/common.h>

#include "FakeDrvSensor.h"
#include "seninf_reg.h"
#include "kd_seninf.h"

#define SENINF_DEV_NAME         "/dev/seninf"
#define SENINF_RANGE             (0x1000)

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

#define TG1_TM_CTL          (0x608)

MUINT32 TS_FakeDrvSensor::readReg(E_SENINF_ID hw_module, MUINT32 RegOffset)
{
    return *((volatile MUINT32 *)(mpSeninfDrv[hw_module] + RegOffset));
}

MVOID TS_FakeDrvSensor::writeReg(E_SENINF_ID hw_module, MUINT32 RegOffset, MUINT32 Value)
{
    *(volatile MUINT32 *)(mpSeninfDrv[hw_module] + RegOffset) = Value;
}

MVOID TS_FakeDrvSensor::setPixelMode(MUINT32 pixMode)
{
    pixMode;
}

MVOID TS_FakeDrvSensor::adjustPatternSize(MUINT32 h_min)
{
    #define H_BAR   0xC

    MUINT32 size_diff_threadhold = 0x100;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfDrv[eSeninf_1];
    TS_LOGD("mpSeninfDrv[eSeninf_1] = %p\n", mpSeninfDrv[eSeninf_1]);
    if (mpSeninfDrv[eSeninf_1]){
        MUINT32 pattern_type=0;
        MUINT32 reg = 0;
        MUINT32 h=0;

        reg = readReg(eSeninf_1, TG1_TM_CTL);
        pattern_type = ((reg >> 4) & 0xf);
        TS_LOGD("readReg(eSeninf_1, TG1_TM_CTL) = 0x%x", reg);
        TS_LOGD("pattern_type = 0x%x", pattern_type);
        if (pattern_type == H_BAR){
            reg = readReg(eSeninf_1, 0x60C);
            h = (reg & 0xffff)>>16;

            if (h < h_min) {
                h = h_min;
            }
            if ((h - h_min) < size_diff_threadhold) {
                h += size_diff_threadhold;
                reg = ((reg & 0xffff) | (h << 16));
                writeReg(eSeninf_1, 0x60c, reg);
                TS_LOGD("reg = 0x%x", reg);
            }
        }
        else{
            TS_LOGD("error:this can only be applied on H-color-bar.Otherwise, bit-ture will be failed\n");
        }
    }
    else{
        TS_LOGD("error:NULL drv ptr, need to poweron 1st\n");
    }

}

MBOOL TS_FakeDrvSensor::init(char const* szCallerName)
{
    MBOOL ret = MTRUE;
    KD_SENINF_REG reg;

    TS_LOGD("[%s]createInstance +\n", szCallerName);
    mInitCount++;
    if (szCallerName == NULL) {
       TS_LOGE("user is null\n");
       return MFALSE;
    }
    if (mpSeninfHwRegBA != NULL) {
       TS_LOGE("already created (szCallerName:%s BA:%p)\n", szCallerName, mpSeninfHwRegBA);
       return MTRUE;
    }



    /*Open seninf driver*/
    mfd = open(SENINF_DEV_NAME, O_RDWR);

    if (mfd < 0) {
        TS_LOGE("error open kernel driver, %d, %s\n", errno, strerror(errno));
        return MFALSE;
    }
    if (ioctl(mfd, KDSENINFIOC_X_GET_REG_ADDR, &reg) < 0) {
       TS_LOGE("ERROR:KDSENINFIOC_X_GET_REG_ADDR\n");
       return MFALSE;
    }

    /*mmap seninf reg*/
    mpSeninfHwRegBA = (unsigned char *) mmap(0, reg.seninf.map_length, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, reg.seninf.map_addr);

    if (mpSeninfHwRegBA == MAP_FAILED) {
        TS_LOGE("mmap err(1), %d, %s \n", errno, strerror(errno));
        return MFALSE;
    }

    /*set base address*/
    for (MUINT32 i = 0; i < eSeninf_max; i++){
        mpSeninfDrv[i] = mpSeninfHwRegBA + (i * SENINF_RANGE);
        TS_LOGD("mpSeninfDrv[%d] = %p -\n",i, mpSeninfDrv[i]);
    }

    TS_LOGE("[%s]createInstance[%d] -\n", szCallerName, mInitCount);

    return ret;
}

MBOOL TS_FakeDrvSensor::uninit(char const* szCallerName)
{
    MBOOL ret = MTRUE;
    KD_SENINF_REG reg;

    TS_LOGD("[%s]destroyInstance +\n", szCallerName);
    mInitCount--;
    if((0 == strcmp(szCallerName, mCaller))){
        if (ioctl(mfd, KDSENINFIOC_X_GET_REG_ADDR, &reg) < 0) {
           TS_LOGE("ERROR:KDSENINFIOC_X_GET_REG_ADDR\n");
           return MFALSE;
        }
        if ( 0 != mpSeninfHwRegBA ) {
            if(munmap(mpSeninfHwRegBA, reg.seninf.map_length) != 0) {
                TS_LOGE("mpSeninfHwRegAddr munmap err, %d, %s \n", errno, strerror(errno));
                return MFALSE;
            }
        }
        if (mfd > 0) {
            close(mfd);
        }
        else{
            TS_LOGE("ERROR: mfd need > 0");
            return MFALSE;
        }

        mfd = 0;

        mpSeninfHwRegBA = NULL;

        for (MUINT32 i = 0; i < eSeninf_max; i++) {
            mpSeninfDrv[i] = NULL;
        }
        TS_LOGE("last user have closed.(%s/%s)\n", szCallerName, mCaller);
        strncpy((char*)mCaller, (char const*)"\0", sizeof("\0"));
    }
    else{
        TS_LOGE("wait for last user to close(%s/%s)\n", szCallerName, mCaller);

        if(mInitCount > 0){
            return MTRUE;
        }else{
            return MFALSE;
        }
    }

    TS_LOGE("[%s]destroyInstance[%d] -\n", szCallerName, mInitCount);

    return ret;
}

MBOOL TS_FakeDrvSensor::powerOn(
        char const* szCallerName,
        MUINT32 const uCountOfIndex,
        MUINT32 const*pArrayOfIndex,
        MUINT32 const*pSetingTbl)
{
    MINT32 nNum = 0, i = 0, ret = 0;
   (void)uCountOfIndex; (void)pArrayOfIndex;

    TS_LOGD("powerOn +\n");

    strncpy((char*)mCaller, (char const*)szCallerName, sizeof(mCaller));
    TS_LOGD("szCallerName = %s,mCaller = %s\n",szCallerName, mCaller);

    if (MFALSE == init(szCallerName)) {
        TS_LOGD("Error: init fail");
        return MFALSE;
    }

    if (pSetingTbl != NULL) {
        MUINT32 num = 0, i = 0;
        MUINT32 reg = 0;

        for(i = 0; pSetingTbl[i] != 0xdeadbeef; i++){
            num++;
        }

        TS_LOGD("#################################################\n");
        TS_LOGD("load test setting ...............................\n");
        TS_LOGD("#################################################\n");

        TS_LOGD("total %d entry\n", num);
        for (i = 0; i < num; i += 2) {
            writeReg((E_SENINF_ID)(*pArrayOfIndex), pSetingTbl[i], pSetingTbl[i+1]);
            TS_LOGD(" [0x%08x]=0x%08x, 0x%08x", pSetingTbl[i], pSetingTbl[i+1], readReg((E_SENINF_ID)(*pArrayOfIndex), pSetingTbl[i]));
        }

    }

    TS_LOGD("powerOn -\n");

    return MTRUE;
}

MBOOL TS_FakeDrvSensor::powerOff(
        char const* szCallerName,
        MUINT32 const uCountOfIndex,
        MUINT32 const*pArrayOfIndex)
{
    (void)uCountOfIndex; (void)pArrayOfIndex;

    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfDrv[*pArrayOfIndex];
    TS_LOGD("powerOff +\n");

    if (pSeninf) {
        TS_LOGD("Set TM disable");
        writeReg((E_SENINF_ID)(*pArrayOfIndex), TG1_TM_CTL, (readReg((E_SENINF_ID)(*pArrayOfIndex), TG1_TM_CTL) & ~0x01));
    }

    if (MFALSE == uninit(szCallerName)) {
        TS_LOGD("Error: init fail");
        return MFALSE;
    }

    TS_LOGD("powerOff -\n");

    return MTRUE;
}

