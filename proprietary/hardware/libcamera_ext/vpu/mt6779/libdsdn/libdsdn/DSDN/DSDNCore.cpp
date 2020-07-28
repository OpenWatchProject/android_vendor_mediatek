#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DSDNCore.h"
#include "TimeProf.h"

#define LOG_TAG "AppDSDN"

#define DSDN_VPU_ALGO_NAME "vpu_lib_d2d_dsdnblnv12"

#ifdef NEON_OPT
#ifdef _MSC_VER
#include "neon_template.hpp"
#else
#include "arm_neon.h"
#endif
#endif



#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#if 0
static int property_set(const char *key, const char *value)
{
    return __system_property_set(key, value);
}
static int property_get(const char *key, char *value, const char *default_value)
{
    int len;
    len = __system_property_get(key, value);
    if (len > 0) {
        return len;
    }

    if (default_value) {
        len = strlen(default_value);
        memcpy(value, default_value, len + 1);
    }
    return len;
}
#endif 

DSDNCore::DSDNCore()
{
    vpu = NULL;
    algo = NULL;
    request = NULL;
}

DSDNCore::~DSDNCore()
{
}

MRESULT DSDNCore::initialize(DSDNInitInfo* pInitInfo)
{
    MRESULT ret = S_DSDN_OK;

    LOGD("[%s] initialize \n", LOG_TAG);

    dsdnMode = pInitInfo->dsdnMode;
    maxWidth = pInitInfo->maxWidth;
    maxHeight = pInitInfo->maxHeight;

    bufferSizeNeeded = calBufferSizeNeeded(pInitInfo);

    initializeTuningParam(pInitInfo);

    memset(m_dumpPath, 0, sizeof(m_dumpPath));
    #ifndef SIM_MAIN
    sprintf(m_dumpPath, "%s", PHONE_DUMP_PATH);
    #else
    sprintf(m_dumpPath, "%s", PC_DUMP_PATH);
    #endif

    //init VpuStream & algo
    ret = initializeVPU();
    return ret;
}

MRESULT DSDNCore::initializeVPU(void)
{
    MRESULT ret = S_DSDN_OK;
    
    //create vpu stream
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        LOGD("fail to get vpu handle");
        ret = E_DSDN_VPU_INIT_ERROR;
    }

    //get dsdn vpu algo
    algo = vpu->getAlgo(DSDN_VPU_ALGO_NAME);
    if (algo == NULL)
    {
        LOGD("fail to get vpu algo handle");
        ret = E_DSDN_VPU_GET_ALGO_ERROR;
    }
    
    return ret;
}

void DSDNCore::initializeTuningParam(DSDNInitInfo* pInitInfo)
{
    debugLevel = DSDN_DEBUG_LEVEL;
    mLogLevel = DSDN_LOG_LEVEL;
    forcedVpuOppStep = -1;

    initializeParseTuningParam(pInitInfo->pTuningInfo);

    char dft[32];
    char value[128] = {'\0'};

    sprintf(dft, "%d", debugLevel);
    property_get("vendor.DSDN.debugLevel", value, dft);
    debugLevel = atoi(value);

    sprintf(dft, "%d", mLogLevel);
    property_get("vendor.DSDN.logLevel", value, dft);
    mLogLevel = atoi(value);

    sprintf(dft, "%d", forcedVpuOppStep);
    property_get("vendor.DSDN.forcedVpuOppStep", value, dft);
    forcedVpuOppStep = atoi(value);

    if(forcedVpuOppStep <0 || forcedVpuOppStep > 15)
    {
        LOGD("forcedVpuOppStep invalid range, set to -1\n");
        forcedVpuOppStep = -1;
    }

    LOGD("[%s] debugLevel(%d)\n", __FUNCTION__, debugLevel);
    LOGD("[%s] logLevel(%d)\n", __FUNCTION__, mLogLevel);
    LOGD("[%s] forcedVpuOppStep(%d)\n", __FUNCTION__, forcedVpuOppStep);
}

void DSDNCore::initializeParseTuningParam(DSDNTuningInfo* pTuningInfo)
{
    MUINT32 numOfParam = pTuningInfo->NumOfParam;
    char curKey[128];

    LOGD("%s: Init tuning. numOfParam(%d)\n", __FUNCTION__, numOfParam);

    for(int i=0;i<numOfParam;i++)
    {
        sprintf(curKey,"%s", pTuningInfo->params[i].key);
        LOGD("%d=> key:%s, value %d\n", i,curKey, pTuningInfo->params[i].value);

        if(strcmp(curKey, "vendor.DSDN.debugLevel")==0){
            debugLevel = pTuningInfo->params[i].value;
        }
        else if(strcmp(curKey, "vendor.DSDN.logLevel")==0){
            mLogLevel = pTuningInfo->params[i].value;
        }
        else if(strcmp(curKey, "vendor.DSDN.forcedVpuOppStep")==0){
            forcedVpuOppStep = pTuningInfo->params[i].value;
        }
    }
}


MRESULT DSDNCore::reset(void)
{
    MRESULT ret = S_DSDN_OK;

    if (vpu)
    {
        LOGD("delete vpu object\n");
        delete vpu;
    }
    return ret;
}

int DSDNCore::calBufferSizeNeeded(DSDNInitInfo* pInitInfo)
{
    int ret = 0;
    //TBD
    return ret;
}

int DSDNCore::getBufferSizeNeeded(void)
{
    LOGD("[%s] buffersize Needed: %d\n", __FUNCTION__, bufferSizeNeeded);
    return bufferSizeNeeded;
}

MRESULT DSDNCore::setupBuffer(DSDNInitInfo* initInfo)
{
    workingBufSize = initInfo->workingBuffSize;
    workingBufAddr = initInfo->workingBuffAddr;

    LOGD("[%s] buf size(%d) buf addr(0x%x)\n", __FUNCTION__, workingBufSize, workingBufAddr);

    if(workingBufSize< bufferSizeNeeded)
    {
        LOGD("[%s] buf size too small!! Needed(%d) Given(%d)\n", __FUNCTION__, bufferSizeNeeded, workingBufSize);
        return E_DSDN_INSUFF_WORK_BUF;
    }

    //TBD Check working buffer size

    //MUINT8* workingBufEnd = NULL;

    //if( (workingBufEnd - workingBufAddr) > workingBufSize)
    //{
    //    return E_DSDN_INSUFF_WORK_BUF;
    //}

    memset(workingBufAddr,0,workingBufSize);

    return S_DSDN_OK;
}

MRESULT DSDNCore::setProcInfo(DSDNProcInfo* procInfoParam)
{
    MRESULT ret = S_DSDN_OK;

    if(procInfoParam !=NULL)
    {
        memcpy(&procInfo, procInfoParam, sizeof(DSDNProcInfo));
    }

    int bufferNumber = procInfo.numOfBuffer;

    if(mLogLevel)
    {
        LOGD("[%s] numOfBuffer(%d)\n", __FUNCTION__, procInfo.numOfBuffer);
    }

    DSDNBufferInfo* bufferInfo;

    for(int i=0; i<bufferNumber; i++)
    {
        bufferInfo = &(procInfo.bufferInfo[i]);
        if(mLogLevel)
        {
            printBufferInfo(bufferInfo);
        }

        switch(bufferInfo->type)
        {
            case DSDN_BUFFER_TYPE_IN1:
                bufInfoIn1 = bufferInfo;
                dstWidth = bufferInfo->width;
                dstHeight = bufferInfo->height;
            break;

            case DSDN_BUFFER_TYPE_IN2:
                bufInfoIn2 = bufferInfo;
                srcWidth = bufferInfo->width;
                srcHeight = bufferInfo->height;
            break;

            case DSDN_BUFFER_TYPE_IN3:
                bufInfoIn3 = bufferInfo;
            break;

            case DSDN_BUFFER_TYPE_OUT1:
                bufInfoOut1 = bufferInfo;
            break;

            default:
                LOGD("unknown buffer type\n");
            break;
        };
    }

    currentFPS = procInfo.fps;

    if(mLogLevel >=2)
    {
        LOGD("FPS(%d) dstWidth(%d) dstHeight(%d)\n", currentFPS, dstWidth, dstHeight);
    }

    return ret;
}
/* =========================================================================================== */

MRESULT DSDNCore::doDSDNMain(void)
{
    MRESULT ret = S_DSDN_OK;
    bool res=false;
    VpuRequest *deque_req = NULL;
    DSDN_VPU_PROPERTY dsdnProperty;

    if ((request = vpu->acquire(algo)) == NULL)
    {
        LOGD("fail to get vpu request !!!");
        ret = E_DSDN_VPU_GET_REQ_ERROR;
        return ret;
    }

    MUINT32 i;
    VpuBuffer bufIn1, bufIn2, bufIn3, bufOut1;

    setupVPUBuffer(0, &bufIn2, bufInfoIn2);
    setupVPUBuffer(1, &bufIn3, bufInfoIn3);
    setupVPUBuffer(2, &bufIn1, bufInfoIn1);
    setupVPUBuffer(3, &bufOut1, bufInfoOut1);

    request->addBuffer(bufIn2);
    request->addBuffer(bufIn3);
    request->addBuffer(bufIn1);
    request->addBuffer(bufOut1);

    //for x,y scale and shift
    setupVPUProperty(&dsdnProperty);
    res = request->setProperty((void*)&dsdnProperty,sizeof(DSDN_VPU_PROPERTY));
    if(res == false)
    {
        LOGD("Error: set property fail!");
        ret = E_DSDN_VPU_SET_PROP_FAIL;
        return ret;
    }

    VpuExtraParam extraParam;
    finalVpuOppStep = getVpuOppStep();

    if(mLogLevel >=2)
    {
        LOGD("Set VPU opp step to %d\n", finalVpuOppStep);
    }

    if(finalVpuOppStep>=0 && finalVpuOppStep < 16)
    {
        extraParam.opp_step = finalVpuOppStep;
    }
    else
    {
        extraParam.opp_step = DSDN_VPU_LOW_OPPSTEP;
    }
    
    request->setExtraParam(extraParam);

    if(mLogLevel)
    {
        LOGD("Set Opp Step to %d\n", extraParam.opp_step);
    }

#if 0
        res = vpu->getVpuStatus(vpu_status);
        if(res == false)
        {
            MY_LOGD("Error: vpu_status!");
            goto EXIT;
        }
    
        // 4. enqueue & dequeue
        extra_param.opp_step = 0;
        //extra_param.bw = 0;
        request-> setExtraParam(extra_param);
        MY_LOGD("vpu->enque(0x%lx): number %d", (unsigned long)request, 0);
#endif


    //res = vpu->enque(request, eIndex_A);
    res = vpu->enque(request);
    if(res == false)
    {
        LOGD("Error: enque fail!");
        ret = E_DSDN_VPU_ENQ_FAIL;
        return ret;
    }

    deque_req = vpu->deque();
    if(mLogLevel)
    {
        LOGD("vpu deque finished !!!, (req=%p, deque_req=%p)",request, deque_req);
    }

    // release resource & exit
    if (request)
    {
        vpu->release(request);
    }

    return ret;
}

MRESULT DSDNCore::printBufferInfo(DSDNBufferInfo* bufferInfo)
{
    MUINT32 count;
    if(mLogLevel)
    {
        LOGD("type(%2d) format(%d) planeCount(%d) width(%4d) height(%4d)\n", bufferInfo->type, bufferInfo->format, bufferInfo->planeCount, bufferInfo->width, bufferInfo->height);
        for(count = 0; count<bufferInfo->planeCount;count++)
        {
            LOGD("fd(%2d) offset(%8d) stride(%4d) length(%8d) va(0x%X)\n", 
                bufferInfo->planes[count].fd, 
                bufferInfo->planes[count].offset, 
                bufferInfo->planes[count].stride, 
                bufferInfo->planes[count].length, 
                bufferInfo->planes[count].va);
        }
    }
    return S_DSDN_OK;
}

void DSDNCore::setupVPUBuffer(unsigned int portId, VpuBuffer* buf, DSDNBufferInfo* bufInfo)
{
    int i;
    buf->port_id = portId;
    buf->format      = (VpuBufferFormat)bufInfo->format;
    buf->width       = bufInfo->width;
    buf->height      = bufInfo->height;
    buf->planeCount  = bufInfo->planeCount;

    for(i=0;i< buf->planeCount;i++)
    {
        buf->planes[i].fd      = (unsigned int) bufInfo->planes[i].fd;
        buf->planes[i].offset  = bufInfo->planes[i].offset;
        buf->planes[i].stride  = bufInfo->planes[i].stride;
        buf->planes[i].length  = bufInfo->planes[i].length;
    }

    #if 1
    //workaround plane0 buffer length
    MUINT32 totalBufferLength = 0;
    for(i=0;i< buf->planeCount;i++)
    {
        totalBufferLength += buf->planes[i].length;
    }

    buf->planes[0].length = totalBufferLength;

    if(mLogLevel)
    {
        LOGD("Set buf->planes[0].length to %d\n", buf->planes[0].length);
    }
    #endif
}

MUINT32 DSDNCore::getVpuOppStep(void)
{
    MUINT32 ret = 0;
    if(forcedVpuOppStep !=-1)
    {
        ret = forcedVpuOppStep;
    }
    else
    {
        //resolution > 6M, used high opp step
        if((dstWidth * dstHeight) > DSDN_VPU_OPPSTEP_THRESHOLD && currentFPS >= DSDN_VPU_FPS_THRESHOLD)
        {
            ret = DSDN_VPU_HIGH_OPPSTEP;
        }
        else
        {
            ret = DSDN_VPU_LOW_OPPSTEP;
        }
    }

    return ret;
}


void DSDNCore::setupVPUProperty(DSDN_VPU_PROPERTY* property)
{
    //for Y
    property->xScale = (int)((float)(1 << 18) * (srcWidth - 1) / (dstWidth - 1));
    property->yScale = (int)((float)(1 << 18) * (srcHeight - 1) / (dstHeight - 1));
    property->xShift = (1 << 17) - (property->xScale >> 1);
    property->yShift = (1 << 17) - (property->yScale >> 1);
    //for UV
    property->xScale_uv = (int)((float)(1 << 18) * ((srcWidth>>1) - 1) / ((dstWidth>>1) - 1));
    property->yScale_uv = (int)((float)(1 << 18) * ((srcHeight>>1) - 1) / ((dstHeight>>1) - 1));
    property->xShift_uv = (1 << 17) - (property->xScale_uv >> 1);
    property->yShift_uv = (1 << 17) - (property->yScale_uv >> 1);

    property->Bformat = (bufInfoIn1->format == DSDNFormatImageNV12)? 0 : 1;     //big image
    property->format  = (bufInfoIn3->format == DSDNFormatImageNV12)? 0 : 1;     //small image

    if(mLogLevel)
    {
        LOGD("dstWidth(%d) dstHeight(%d) srcWidth(%d) srcHeight(%d)\n",dstWidth, dstHeight, srcWidth, srcHeight);
        LOGD("Y  xScale(%d) yScale(%d) xShift(%d) yShift(%d)\n", property->xScale, property->yScale, property->xShift, property->yShift);
        LOGD("UV xScale(%d) yScale(%d) xShift(%d) yShift(%d)\n", property->xScale_uv, property->yScale_uv, property->xShift_uv, property->yShift_uv);
        LOGD("Bformat(%d) format(%d) 0:NV12 1:NV21\n",property->Bformat, property->format);
    }
}

