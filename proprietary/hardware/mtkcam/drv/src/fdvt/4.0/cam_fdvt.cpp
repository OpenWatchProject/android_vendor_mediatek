/***********************************************************************************************
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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "FDVT_Drv"
//#include <utils/Errors.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "utils/Mutex.h"    // android mutex
//#include <sys/mman.h>

#include "cam_fdvt.h"
#include "fdvt_learning_data_model1.h"
#include "fdvt_learning_data_model1_extra.h"
#include "fdvt_learning_data_model2.h"
#include "fdvt_learning_data_model2_extra.h"

#include <imem_drv.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#undef MY_LOGI
#undef MY_LOGE
#undef MY_LOGD
#define MY_LOGI(fmt, arg...)  ALOGI(fmt, ##arg)
#define MY_LOGE(fmt, arg...)  ALOGE(fmt, ##arg)
#define MY_LOGD(fmt, arg...)  do { if (g_isLogEnable) { ALOGI(fmt, ##arg); } } while(0)

/*******************************************************************************
*
********************************************************************************/
//-------------------------------------------//
//  Global face detection related parameter  //
//-------------------------------------------//
#define FDVT_DEV_NAME    "/dev/camera-fdvt"
static MINT32 fdFDVT = -1;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define FDVT_RAW_DUMP      0
#define FDVT_REG_DUMP      0
#define FDVT_DRAM_DUMP     0

FdDrv_input_struct *g_FdDrv_input;
#define FDVT_SCALE_SIZE_LIMITATION     26/*MT6797>24,MT6757>25*/
/*******************************************************************************
*
********************************************************************************/
MUINT32 FdReg_Addr[FDVT_PARA_NUM];
MUINT32 FdReg_Value[FDVT_PARA_NUM];

static android::Mutex   m_FDInitMutex;
MINT32 g_UserCount = 0;
MUINT8 g_GFD_PatternType = 1;

FdDrv_Para *g_FdDrv_Para = (FdDrv_Para *)malloc(sizeof(FdDrv_Para));

MBOOL g_EnquedStatus = MFALSE;

MBOOL g_isLogEnable = MFALSE;

/*******************************************************************************
*
********************************************************************************/
StdIMemDrv *g_ImemDrv = NULL;
IMEM_BUF_INFO Imem_Learning_Data[LEARNDATA_NUM];
IMEM_BUF_INFO Imem_Learning_Data_Extra[EXTRA_LEARNDATA_NUM];
IMEM_BUF_INFO Imem_RS_ConfigData;
IMEM_BUF_INFO Imem_FD_ConfigData;
IMEM_BUF_INFO Imem_RS_BufferData;
IMEM_BUF_INFO Imem_FD_ResultData;
IMEM_BUF_INFO Imem_Buf_Info;
MUINT8    *Imem_pLogVir=NULL;
MINT32     Imem_MemID;
MUINT32  Imem_Size;
MUINT32  Imem_AllocNum = 0;

/*******************************************************************************
*
********************************************************************************/
MINT32 FDVT_Imem_flush()
{
    MINT32 ret = 0;
    MINT32 i;
    //MY_LOGD("Do Flush (IMEM) \n");

    //Flush Learning Data
    for(i=0;i<LEARNDATA_NUM;i++)
    {
        Imem_Buf_Info.size = Imem_Learning_Data[i].size;
        Imem_Buf_Info.memID = Imem_Learning_Data[i].memID;
        Imem_Buf_Info.virtAddr = (MUINTPTR)Imem_Learning_Data[i].virtAddr;
        Imem_Buf_Info.phyAddr = (MUINTPTR)Imem_Learning_Data[i].phyAddr;
        if(g_ImemDrv)
        {
            g_ImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &Imem_Buf_Info);
        }
    }

    //Flush Config Data
    Imem_Buf_Info.size = Imem_RS_ConfigData.size;
    Imem_Buf_Info.memID = Imem_RS_ConfigData.memID;
    Imem_Buf_Info.virtAddr = (MUINTPTR)Imem_RS_ConfigData.virtAddr;
    Imem_Buf_Info.phyAddr = (MUINTPTR)Imem_RS_ConfigData.phyAddr;
    if(g_ImemDrv)
    {
        g_ImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &Imem_Buf_Info);
    }

    Imem_Buf_Info.size = Imem_FD_ConfigData.size;
    Imem_Buf_Info.memID = Imem_FD_ConfigData.memID;
    Imem_Buf_Info.virtAddr = (MUINTPTR)Imem_FD_ConfigData.virtAddr;
    Imem_Buf_Info.phyAddr = (MUINTPTR)Imem_FD_ConfigData.phyAddr;
    if(g_ImemDrv)
    {
        g_ImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &Imem_Buf_Info);
    }

    //g_ImemDrv->cacheFlushAll();

    return ret;
}

MINT32 FDVT_Imem_invalid()
{
    MINT32 ret = 0;
    //MY_LOGD("Do Ivalid (IMEM) \n");
#if FDVT_RAW_DUMP
    //Invalidate Rs buffer Data
    Imem_Buf_Info.size = Imem_RS_BufferData.size;
    Imem_Buf_Info.memID = Imem_RS_BufferData.memID;
    Imem_Buf_Info.virtAddr = (MUINTPTR)Imem_RS_BufferData.virtAddr;
    Imem_Buf_Info.phyAddr = (MUINTPTR)Imem_RS_BufferData.phyAddr;
    if(g_ImemDrv)
    {
        g_ImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &Imem_Buf_Info);
    }
#endif
    //Invalidate FD Result Data
    Imem_Buf_Info.size = Imem_FD_ResultData.size;
    Imem_Buf_Info.memID = Imem_FD_ResultData.memID;
    Imem_Buf_Info.virtAddr = (MUINTPTR)Imem_FD_ResultData.virtAddr;
    Imem_Buf_Info.phyAddr = (MUINTPTR)Imem_FD_ResultData.phyAddr;
    if(g_ImemDrv)
    {
        g_ImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &Imem_Buf_Info);
    }

     return ret;
}

MINT32 FDVT_Imem_alloc(MUINT32 size,MINT32 *memId,MUINT8 **vAddr,MUINTPTR *pAddr)
{
    if ( NULL == g_ImemDrv ) {
        g_ImemDrv = StdIMemDrv::createInstance();
        g_ImemDrv->init();
    }
    //
    Imem_Buf_Info.size = size;
    //Imem_Buf_Info.useNoncache = 1;
    if (g_ImemDrv->allocVirtBuf(&Imem_Buf_Info) != 0)
        MY_LOGE("Imem Allocate Virtual Buffer Fail!\n");

    *memId = Imem_Buf_Info.memID;
    *vAddr = (MUINT8 *)Imem_Buf_Info.virtAddr;
    //
    if (g_ImemDrv->mapPhyAddr(&Imem_Buf_Info) != 0)
        MY_LOGE("Imem Map Physical Address Fail\n");

    *pAddr = (MUINTPTR)Imem_Buf_Info.phyAddr;
    Imem_AllocNum ++;

    /*MY_LOGD("Imem_AllocNum(%d)\n",Imem_AllocNum);*/
    /*MY_LOGD("vAddr(0x%p) pAddr(0x%p) Imem_AllocNum(%d)\n",*vAddr,*pAddr,Imem_AllocNum);*/

    return 0;
}


MINT32 FDVT_Imem_free(MUINT8 *vAddr, MUINTPTR phyAddr,MUINT32 size,MINT32 memId)
{
    Imem_Buf_Info.size = size;
    Imem_Buf_Info.memID = memId;
    Imem_Buf_Info.virtAddr = (MUINTPTR)vAddr;
    Imem_Buf_Info.phyAddr = (MUINTPTR)phyAddr;
    //

    if(g_ImemDrv)
    {
        if(g_ImemDrv->unmapPhyAddr(&Imem_Buf_Info) != 0)
            MY_LOGE("Imem Unmap Physical Address Fail!\n");

        if(g_ImemDrv->freeVirtBuf(&Imem_Buf_Info) != 0)
            MY_LOGE("Imem Free Virtual Buffer Fail!\n");

        if(Imem_AllocNum)
        {
            Imem_AllocNum--;
            if(Imem_AllocNum==0)
            {
                g_ImemDrv->uninit();
                g_ImemDrv = NULL;
            }
        }
    }
    else
    {
        MY_LOGE("No ImemDrv\n");
    }

    /*MY_LOGD("Imem_AllocNum(%d)\n",Imem_AllocNum);*/

    return 0;
}

MINT32 FDVT_IOCTL_OpenDriver()
{
    MINT32 err = S_Detection_OK;

    //open fd driver
    if (fdFDVT == -1) {
        fdFDVT = open(FDVT_DEV_NAME, O_RDWR|O_CLOEXEC);
        if (fdFDVT < 0) {
            MY_LOGE("error opening %s: %s\n", FDVT_DEV_NAME, strerror(errno));
            return E_Detection_Driver_Fail;
        }
    }

    //FDVT_PARA_SET(&mFDVTReg);
    //ioctl(fdFDVT,MT6573FDVT_INIT_SETPARA_CMD,&mFDVTReg);
    ioctl(fdFDVT,FDVT_IOC_INIT_SETPARA_CMD);
    return err;
}

MINT32 FDVT_IOCTL_ParaSetting(MUINT32* Adr,MUINT32* value,MINT32 num,FDVT_OPERATION_MODE fd_state)
{
    MINT32 ret = S_Detection_OK;
    FDVTRegIO mFDVTReg;
    mFDVTReg.pAddr=Adr;
    mFDVTReg.pData=value;
    mFDVTReg.u4Count=num;
    if (fd_state == FDVT_MODE_GFD)
       ret=ioctl(fdFDVT,FDVT_IOC_T_SET_FDCONF_CMD,&mFDVTReg);
    else if (fd_state == FDVT_MODE_SD)
       ret=ioctl(fdFDVT,FDVT_IOC_T_SET_SDCONF_CMD,&mFDVTReg);
    else
       ret=ioctl(fdFDVT,FDVT_IOC_T_SET_FDCONF_CMD,&mFDVTReg);
    return ret;
}

MINT32 FDVT_IOCTL_StartHW()
{
    MINT32 ret = 0;
#if FDVT_DRAM_DUMP
    MUINT8 rsLoop = 0;
#endif
    MUINT64  *rs_cfg;
    MUINT64  *fd_cfg;

    //Do IMEM flush before start HW to sync cache data to dram
    FDVT_Imem_flush();
    //MY_LOGD("Set Start HW in \n");
    rs_cfg  = g_FdDrv_Para->RS_Config_VA;
    fd_cfg  = g_FdDrv_Para->FD_Config_VA;

#if FDVT_DRAM_DUMP
    /* Dump RS Config in DRAM */
    MY_LOGI("Dump RS Config in DRAM\n");
    for (rsLoop = 0; rsLoop < g_FdDrv_Para->RS_Num; rsLoop++)
    {
        MY_LOGI("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", *((MUINT32 *)rs_cfg+8*rsLoop), *((MUINT32 *)rs_cfg+8*rsLoop+1), *((MUINT32 *)rs_cfg+8*rsLoop+2), *((MUINT32 *)rs_cfg+8*rsLoop+3), *((MUINT32 *)rs_cfg+8*rsLoop+4), *((MUINT32 *)rs_cfg+8*rsLoop+5), *((MUINT32 *)rs_cfg+8*rsLoop+6), *((MUINT32 *)rs_cfg+8*rsLoop+7));
    }

    MY_LOGI("Dump FD Config in DRAM\n");
    for (rsLoop = 0; rsLoop < g_FdDrv_Para->RS_Num; rsLoop++)
    {
        MY_LOGI("0x%08x 0x%08x 0x%08x 0x%08x\n", *((MUINT32 *)fd_cfg+4*rsLoop), *((MUINT32 *)fd_cfg+4*rsLoop+1), *((MUINT32 *)fd_cfg+4*rsLoop+2), *((MUINT32 *)fd_cfg+4*rsLoop+3));
    }
#endif
    ret=ioctl(fdFDVT,FDVT_IOC_STARTFD_CMD);
    //MY_LOGD("Set Start HW out \n");
    return ret;
}

MINT32 FDVT_IOCTL_WaitIRQ()
{
    MUINT32 irqSts;
    MINT32 ret = 0;

    do{
        ret = ioctl(fdFDVT,FDVT_IOC_G_WAITIRQ,&irqSts);

        if(ret == (-SIG_ERESTARTSYS))
        {
            MY_LOGE("Receive restart system call signal, wait IRQ again\n");
        }
        else
            break;
    } while(1);

    return ret;
}

MINT32 FDVT_IOCTL_GetHWResult(MUINT32* Adr,MUINT32* value,MINT32 num,MUINT32 &result)
{
    MINT32 ret = S_Detection_OK;
    //MHAL_ASSERT(bufSize >= sizeof(fdvt_result_struct) * 16, "bufSize is too small");
    FDVTRegIO mFDVTReg;
    mFDVTReg.pAddr=Adr;
    mFDVTReg.pData=value;
    mFDVTReg.u4Count=num;
    ret=ioctl(fdFDVT,FDVT_IOC_G_READ_FDREG_CMD,&mFDVTReg);
    result=mFDVTReg.pData[0];
    return ret;
}

MINT32 FDVT_IOCTL_CloseDriver()
{
    //MHAL_LOG("FDVT_Uninit in ");
    if (fdFDVT > 0) {
        close(fdFDVT);
        fdFDVT = -1;
    }
    return S_Detection_OK;
}

MINT32 FDVT_IOCTL_DumpReg()
{
    ioctl(fdFDVT,FDVT_IOC_T_DUMPREG);
    return 0;
}

/*******************************************************************************
*
********************************************************************************/

void FDVT_AllocMem_LearningData()
{
    if(g_FdDrv_Para->Learning_Type == 1)
    {
        for(MINT32 i=0;i<LEARNDATA_NUM;i++)
        {
            if(i==0)          {Imem_Size=sizeof(learning_data_model2_0);}
            else if(i==1)     {Imem_Size=sizeof(learning_data_model2_1);}
            else if(i==2)     {Imem_Size=sizeof(learning_data_model2_2);}
            else if(i==3)     {Imem_Size=sizeof(learning_data_model2_3);}
            else if(i==4)     {Imem_Size=sizeof(learning_data_model2_4);}
            else if(i==5)     {Imem_Size=sizeof(learning_data_model2_5);}
            else if(i==6)     {Imem_Size=sizeof(learning_data_model2_6);}
            else if(i==7)     {Imem_Size=sizeof(learning_data_model2_7);}
            else if(i==8)     {Imem_Size=sizeof(learning_data_model2_8);}
            else if(i==9)     {Imem_Size=sizeof(learning_data_model2_9);}
            else if(i==10)    {Imem_Size=sizeof(learning_data_model2_10);}
            else if(i==11)    {Imem_Size=sizeof(learning_data_model2_11);}
            else if(i==12)    {Imem_Size=sizeof(learning_data_model2_12);}
            else if(i==13)    {Imem_Size=sizeof(learning_data_model2_13);}
            else if(i==14)    {Imem_Size=sizeof(learning_data_model2_14);}
            else if(i==15)    {Imem_Size=sizeof(learning_data_model2_15);}
            else if(i==16)    {Imem_Size=sizeof(learning_data_model2_16);}
            else if(i==17)    {Imem_Size=sizeof(learning_data_model2_17);}
            FDVT_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &Imem_Learning_Data[i].phyAddr);
            Imem_Learning_Data[i].virtAddr=(MUINTPTR)Imem_pLogVir;
            Imem_Learning_Data[i].memID=Imem_MemID;
            Imem_Learning_Data[i].size=Imem_Size;
            if(i==0)          {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_0), Imem_Size);}
            else if(i==1)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_1), Imem_Size);}
            else if(i==2)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_2), Imem_Size);}
            else if(i==3)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_3), Imem_Size);}
            else if(i==4)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_4), Imem_Size);}
            else if(i==5)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_5), Imem_Size);}
            else if(i==6)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_6), Imem_Size);}
            else if(i==7)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_7), Imem_Size);}
            else if(i==8)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_8), Imem_Size);}
            else if(i==9)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_9), Imem_Size);}
            else if(i==10)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_10), Imem_Size);}
            else if(i==11)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_11), Imem_Size);}
            else if(i==12)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_12), Imem_Size);}
            else if(i==13)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_13), Imem_Size);}
            else if(i==14)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_14), Imem_Size);}
            else if(i==15)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_15), Imem_Size);}
            else if(i==16)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_16), Imem_Size);}
            else if(i==17)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model2_17), Imem_Size);}
            MY_LOGD("Learning data (model2) (%d) PA/VA (0x%lx)/(0x%lx) \n",i,(unsigned long)Imem_Learning_Data[i].phyAddr,(unsigned long)Imem_Learning_Data[i].virtAddr);
        }
    }
    else
    {
        for(MINT32 i=0;i<LEARNDATA_NUM;i++)
        {
            if(i==0)          {Imem_Size=sizeof(learning_data_model1_0);}
            else if(i==1)     {Imem_Size=sizeof(learning_data_model1_1);}
            else if(i==2)     {Imem_Size=sizeof(learning_data_model1_2);}
            else if(i==3)     {Imem_Size=sizeof(learning_data_model1_3);}
            else if(i==4)     {Imem_Size=sizeof(learning_data_model1_4);}
            else if(i==5)     {Imem_Size=sizeof(learning_data_model1_5);}
            else if(i==6)     {Imem_Size=sizeof(learning_data_model1_6);}
            else if(i==7)     {Imem_Size=sizeof(learning_data_model1_7);}
            else if(i==8)     {Imem_Size=sizeof(learning_data_model1_8);}
            else if(i==9)     {Imem_Size=sizeof(learning_data_model1_9);}
            else if(i==10)    {Imem_Size=sizeof(learning_data_model1_10);}
            else if(i==11)    {Imem_Size=sizeof(learning_data_model1_11);}
            else if(i==12)    {Imem_Size=sizeof(learning_data_model1_12);}
            else if(i==13)    {Imem_Size=sizeof(learning_data_model1_13);}
            else if(i==14)    {Imem_Size=sizeof(learning_data_model1_14);}
            else if(i==15)    {Imem_Size=sizeof(learning_data_model1_15);}
            else if(i==16)    {Imem_Size=sizeof(learning_data_model1_16);}
            else if(i==17)    {Imem_Size=sizeof(learning_data_model1_17);}
            FDVT_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &Imem_Learning_Data[i].phyAddr);
            Imem_Learning_Data[i].virtAddr=(MUINTPTR)Imem_pLogVir;
            Imem_Learning_Data[i].memID=Imem_MemID;
            Imem_Learning_Data[i].size=Imem_Size;
            if(i==0)          {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_0), Imem_Size);}
            else if(i==1)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_1), Imem_Size);}
            else if(i==2)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_2), Imem_Size);}
            else if(i==3)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_3), Imem_Size);}
            else if(i==4)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_4), Imem_Size);}
            else if(i==5)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_5), Imem_Size);}
            else if(i==6)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_6), Imem_Size);}
            else if(i==7)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_7), Imem_Size);}
            else if(i==8)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_8), Imem_Size);}
            else if(i==9)     {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_9), Imem_Size);}
            else if(i==10)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_10), Imem_Size);}
            else if(i==11)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_11), Imem_Size);}
            else if(i==12)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_12), Imem_Size);}
            else if(i==13)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_13), Imem_Size);}
            else if(i==14)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_14), Imem_Size);}
            else if(i==15)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_15), Imem_Size);}
            else if(i==16)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_16), Imem_Size);}
            else if(i==17)    {memcpy( (MUINT8*)(Imem_Learning_Data[i].virtAddr), (MUINT8*)(learning_data_model1_17), Imem_Size);}
            MY_LOGD("Learning data (model1) (%d) PA/VA (0x%lx)/(0x%lx) \n",i,(unsigned long)Imem_Learning_Data[i].phyAddr,(unsigned long)Imem_Learning_Data[i].virtAddr);
        }
    }

    g_FdDrv_Para->Learning_Data0_PA = (MUINT32 *)(Imem_Learning_Data[0].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data1_PA = (MUINT32 *)(Imem_Learning_Data[1].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data2_PA = (MUINT32 *)(Imem_Learning_Data[2].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data3_PA = (MUINT32 *)(Imem_Learning_Data[3].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data4_PA = (MUINT32 *)(Imem_Learning_Data[4].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data5_PA = (MUINT32 *)(Imem_Learning_Data[5].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data6_PA = (MUINT32 *)(Imem_Learning_Data[6].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data7_PA = (MUINT32 *)(Imem_Learning_Data[7].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data8_PA = (MUINT32 *)(Imem_Learning_Data[8].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data9_PA = (MUINT32 *)(Imem_Learning_Data[9].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data10_PA = (MUINT32 *)(Imem_Learning_Data[10].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data11_PA = (MUINT32 *)(Imem_Learning_Data[11].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data12_PA = (MUINT32 *)(Imem_Learning_Data[12].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data13_PA = (MUINT32 *)(Imem_Learning_Data[13].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data14_PA = (MUINT32 *)(Imem_Learning_Data[14].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data15_PA = (MUINT32 *)(Imem_Learning_Data[15].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data16_PA = (MUINT32 *)(Imem_Learning_Data[16].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Learning_Data17_PA = (MUINT32 *)(Imem_Learning_Data[17].phyAddr & 0x00000000FFFFFFFF);

    g_FdDrv_Para->Learning_Data0_VA = (MUINT64*)Imem_Learning_Data[0].virtAddr;
    g_FdDrv_Para->Learning_Data1_VA = (MUINT64*)Imem_Learning_Data[1].virtAddr;
    g_FdDrv_Para->Learning_Data2_VA = (MUINT64*)Imem_Learning_Data[2].virtAddr;
    g_FdDrv_Para->Learning_Data3_VA = (MUINT64*)Imem_Learning_Data[3].virtAddr;
    g_FdDrv_Para->Learning_Data4_VA = (MUINT64*)Imem_Learning_Data[4].virtAddr;
    g_FdDrv_Para->Learning_Data5_VA = (MUINT64*)Imem_Learning_Data[5].virtAddr;
    g_FdDrv_Para->Learning_Data6_VA = (MUINT64*)Imem_Learning_Data[6].virtAddr;
    g_FdDrv_Para->Learning_Data7_VA = (MUINT64*)Imem_Learning_Data[7].virtAddr;
    g_FdDrv_Para->Learning_Data8_VA = (MUINT64*)Imem_Learning_Data[8].virtAddr;
    g_FdDrv_Para->Learning_Data9_VA = (MUINT64*)Imem_Learning_Data[9].virtAddr;
    g_FdDrv_Para->Learning_Data10_VA = (MUINT64*)Imem_Learning_Data[10].virtAddr;
    g_FdDrv_Para->Learning_Data11_VA = (MUINT64*)Imem_Learning_Data[11].virtAddr;
    g_FdDrv_Para->Learning_Data12_VA = (MUINT64*)Imem_Learning_Data[12].virtAddr;
    g_FdDrv_Para->Learning_Data13_VA = (MUINT64*)Imem_Learning_Data[13].virtAddr;
    g_FdDrv_Para->Learning_Data14_VA = (MUINT64*)Imem_Learning_Data[14].virtAddr;
    g_FdDrv_Para->Learning_Data15_VA = (MUINT64*)Imem_Learning_Data[15].virtAddr;
    g_FdDrv_Para->Learning_Data16_VA = (MUINT64*)Imem_Learning_Data[16].virtAddr;
    g_FdDrv_Para->Learning_Data17_VA = (MUINT64*)Imem_Learning_Data[17].virtAddr;
}

void FDVT_AllocMem_LearningData_Extra()
{
    if(g_FdDrv_Para->Learning_Type == 1)
    {
        for(MINT32 i=0;i<EXTRA_LEARNDATA_NUM;i++)
        {
            if(i==0)          {Imem_Size=sizeof(learning_data_model2_extra_0);}
            else if(i==1)     {Imem_Size=sizeof(learning_data_model2_extra_1);}
            else if(i==2)     {Imem_Size=sizeof(learning_data_model2_extra_2);}
            else if(i==3)     {Imem_Size=sizeof(learning_data_model2_extra_3);}
            else if(i==4)     {Imem_Size=sizeof(learning_data_model2_extra_4);}
            else if(i==5)     {Imem_Size=sizeof(learning_data_model2_extra_5);}
            else if(i==6)     {Imem_Size=sizeof(learning_data_model2_extra_6);}
            else if(i==7)     {Imem_Size=sizeof(learning_data_model2_extra_7);}
            else if(i==8)     {Imem_Size=sizeof(learning_data_model2_extra_8);}
            else if(i==9)     {Imem_Size=sizeof(learning_data_model2_extra_9);}
            else if(i==10)    {Imem_Size=sizeof(learning_data_model2_extra_10);}
            else if(i==11)    {Imem_Size=sizeof(learning_data_model2_extra_11);}
            else if(i==12)    {Imem_Size=sizeof(learning_data_model2_extra_12);}
            else if(i==13)    {Imem_Size=sizeof(learning_data_model2_extra_13);}
            else if(i==14)    {Imem_Size=sizeof(learning_data_model2_extra_14);}
            else if(i==15)    {Imem_Size=sizeof(learning_data_model2_extra_15);}
            else if(i==16)    {Imem_Size=sizeof(learning_data_model2_extra_16);}
            else if(i==17)    {Imem_Size=sizeof(learning_data_model2_extra_17);}
            FDVT_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &Imem_Learning_Data_Extra[i].phyAddr);
            Imem_Learning_Data_Extra[i].virtAddr=(MUINTPTR)Imem_pLogVir;
            Imem_Learning_Data_Extra[i].memID=Imem_MemID;
            Imem_Learning_Data_Extra[i].size=Imem_Size;
            if(i==0)          {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_0), Imem_Size);}
            else if(i==1)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_1), Imem_Size);}
            else if(i==2)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_2), Imem_Size);}
            else if(i==3)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_3), Imem_Size);}
            else if(i==4)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_4), Imem_Size);}
            else if(i==5)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_5), Imem_Size);}
            else if(i==6)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_6), Imem_Size);}
            else if(i==7)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_7), Imem_Size);}
            else if(i==8)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_8), Imem_Size);}
            else if(i==9)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_9), Imem_Size);}
            else if(i==10)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_10), Imem_Size);}
            else if(i==11)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_11), Imem_Size);}
            else if(i==12)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_12), Imem_Size);}
            else if(i==13)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_13), Imem_Size);}
            else if(i==14)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_14), Imem_Size);}
            else if(i==15)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_15), Imem_Size);}
            else if(i==16)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_16), Imem_Size);}
            else if(i==17)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model2_extra_17), Imem_Size);}
            MY_LOGD("Learning data (model2 extra) (%d) PA/VA (0x%lx)/(0x%lx) \n",i,(unsigned long)Imem_Learning_Data_Extra[i].phyAddr,(unsigned long)Imem_Learning_Data_Extra[i].virtAddr);
        }
    }
    else
    {
        for(MINT32 i=0;i<EXTRA_LEARNDATA_NUM;i++)
        {
            if(i==0)          {Imem_Size=sizeof(learning_data_model1_extra_0);}
            else if(i==1)     {Imem_Size=sizeof(learning_data_model1_extra_1);}
            else if(i==2)     {Imem_Size=sizeof(learning_data_model1_extra_2);}
            else if(i==3)     {Imem_Size=sizeof(learning_data_model1_extra_3);}
            else if(i==4)     {Imem_Size=sizeof(learning_data_model1_extra_4);}
            else if(i==5)     {Imem_Size=sizeof(learning_data_model1_extra_5);}
            else if(i==6)     {Imem_Size=sizeof(learning_data_model1_extra_6);}
            else if(i==7)     {Imem_Size=sizeof(learning_data_model1_extra_7);}
            else if(i==8)     {Imem_Size=sizeof(learning_data_model1_extra_8);}
            else if(i==9)     {Imem_Size=sizeof(learning_data_model1_extra_9);}
            else if(i==10)    {Imem_Size=sizeof(learning_data_model1_extra_10);}
            else if(i==11)    {Imem_Size=sizeof(learning_data_model1_extra_11);}
            else if(i==12)    {Imem_Size=sizeof(learning_data_model1_extra_12);}
            else if(i==13)    {Imem_Size=sizeof(learning_data_model1_extra_13);}
            else if(i==14)    {Imem_Size=sizeof(learning_data_model1_extra_14);}
            else if(i==15)    {Imem_Size=sizeof(learning_data_model1_extra_15);}
            else if(i==16)    {Imem_Size=sizeof(learning_data_model1_extra_16);}
            else if(i==17)    {Imem_Size=sizeof(learning_data_model1_extra_17);}
            FDVT_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &Imem_Learning_Data_Extra[i].phyAddr);
            Imem_Learning_Data_Extra[i].virtAddr=(MUINTPTR)Imem_pLogVir;
            Imem_Learning_Data_Extra[i].memID=Imem_MemID;
            Imem_Learning_Data_Extra[i].size=Imem_Size;
            if(i==0)          {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_0), Imem_Size);}
            else if(i==1)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_1), Imem_Size);}
            else if(i==2)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_2), Imem_Size);}
            else if(i==3)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_3), Imem_Size);}
            else if(i==4)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_4), Imem_Size);}
            else if(i==5)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_5), Imem_Size);}
            else if(i==6)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_6), Imem_Size);}
            else if(i==7)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_7), Imem_Size);}
            else if(i==8)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_8), Imem_Size);}
            else if(i==9)     {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_9), Imem_Size);}
            else if(i==10)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_10), Imem_Size);}
            else if(i==11)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_11), Imem_Size);}
            else if(i==12)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_12), Imem_Size);}
            else if(i==13)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_13), Imem_Size);}
            else if(i==14)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_14), Imem_Size);}
            else if(i==15)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_15), Imem_Size);}
            else if(i==16)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_16), Imem_Size);}
            else if(i==17)    {memcpy( (MUINT8*)(Imem_Learning_Data_Extra[i].virtAddr), (MUINT8*)(learning_data_model1_extra_17), Imem_Size);}
            MY_LOGD("Learning data (model1 extra) (%d) PA/VA (0x%lx)/(0x%lx) \n",i,(unsigned long)Imem_Learning_Data_Extra[i].phyAddr,(unsigned long)Imem_Learning_Data_Extra[i].virtAddr);
        }
    }

    g_FdDrv_Para->Extra_Learning_Data0_PA = (MUINT32 *)(Imem_Learning_Data_Extra[0].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data1_PA = (MUINT32 *)(Imem_Learning_Data_Extra[1].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data2_PA = (MUINT32 *)(Imem_Learning_Data_Extra[2].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data3_PA = (MUINT32 *)(Imem_Learning_Data_Extra[3].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data4_PA = (MUINT32 *)(Imem_Learning_Data_Extra[4].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data5_PA = (MUINT32 *)(Imem_Learning_Data_Extra[5].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data6_PA = (MUINT32 *)(Imem_Learning_Data_Extra[6].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data7_PA = (MUINT32 *)(Imem_Learning_Data_Extra[7].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data8_PA = (MUINT32 *)(Imem_Learning_Data_Extra[8].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data9_PA = (MUINT32 *)(Imem_Learning_Data_Extra[9].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data10_PA = (MUINT32 *)(Imem_Learning_Data_Extra[10].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data11_PA = (MUINT32 *)(Imem_Learning_Data_Extra[11].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data12_PA = (MUINT32 *)(Imem_Learning_Data_Extra[12].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data13_PA = (MUINT32 *)(Imem_Learning_Data_Extra[13].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data14_PA = (MUINT32 *)(Imem_Learning_Data_Extra[14].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data15_PA = (MUINT32 *)(Imem_Learning_Data_Extra[15].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data16_PA = (MUINT32 *)(Imem_Learning_Data_Extra[16].phyAddr & 0x00000000FFFFFFFF);
    g_FdDrv_Para->Extra_Learning_Data17_PA = (MUINT32 *)(Imem_Learning_Data_Extra[17].phyAddr & 0x00000000FFFFFFFF);

    g_FdDrv_Para->Extra_Learning_Data0_VA = (MUINT64*)Imem_Learning_Data_Extra[0].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data1_VA = (MUINT64*)Imem_Learning_Data_Extra[1].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data2_VA = (MUINT64*)Imem_Learning_Data_Extra[2].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data3_VA = (MUINT64*)Imem_Learning_Data_Extra[3].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data4_VA = (MUINT64*)Imem_Learning_Data_Extra[4].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data5_VA = (MUINT64*)Imem_Learning_Data_Extra[5].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data6_VA = (MUINT64*)Imem_Learning_Data_Extra[6].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data7_VA = (MUINT64*)Imem_Learning_Data_Extra[7].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data8_VA = (MUINT64*)Imem_Learning_Data_Extra[8].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data9_VA = (MUINT64*)Imem_Learning_Data_Extra[9].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data10_VA = (MUINT64*)Imem_Learning_Data_Extra[10].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data11_VA = (MUINT64*)Imem_Learning_Data_Extra[11].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data12_VA = (MUINT64*)Imem_Learning_Data_Extra[12].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data13_VA = (MUINT64*)Imem_Learning_Data_Extra[13].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data14_VA = (MUINT64*)Imem_Learning_Data_Extra[14].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data15_VA = (MUINT64*)Imem_Learning_Data_Extra[15].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data16_VA = (MUINT64*)Imem_Learning_Data_Extra[16].virtAddr;
    g_FdDrv_Para->Extra_Learning_Data17_VA = (MUINT64*)Imem_Learning_Data_Extra[17].virtAddr;
}

void FDVT_AllocMem_RSFDConfigData()
{
    //Imem_Size=sizeof(rs_config_data);
    Imem_Size=sizeof(rs_config_data_640x480);
    FDVT_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &Imem_RS_ConfigData.phyAddr);
    Imem_RS_ConfigData.virtAddr=(MUINTPTR)Imem_pLogVir;
    Imem_RS_ConfigData.memID=Imem_MemID;
    Imem_RS_ConfigData.size=Imem_Size;
    memcpy( (MUINT8*)(Imem_RS_ConfigData.virtAddr), (MUINT8*)(rs_config_data_640x480), Imem_Size);
    MY_LOGD("rs config data PA/VA (0x%lx)/(0x%lx) \n",(unsigned long)Imem_RS_ConfigData.phyAddr,(unsigned long)Imem_RS_ConfigData.virtAddr);

    g_FdDrv_Para->RS_Config_PA = (MUINT32*)Imem_RS_ConfigData.phyAddr;
    g_FdDrv_Para->RS_Config_VA = (MUINT64*)Imem_RS_ConfigData.virtAddr;

    //Imem_Size=sizeof(fd_config_data);
    Imem_Size=sizeof(fd_config_data_640x480);
    FDVT_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &Imem_FD_ConfigData.phyAddr);
    Imem_FD_ConfigData.virtAddr=(MUINTPTR)Imem_pLogVir;
    Imem_FD_ConfigData.memID=Imem_MemID;
    Imem_FD_ConfigData.size=Imem_Size;

    memcpy( (MUINT8*)(Imem_FD_ConfigData.virtAddr), (MUINT8*)(fd_config_data_640x480), Imem_Size);
    MY_LOGD("fd config data PA/VA (0x%lx)/(0x%lx) \n",(unsigned long)Imem_FD_ConfigData.phyAddr,(unsigned long)Imem_FD_ConfigData.virtAddr);

    g_FdDrv_Para->FD_Config_PA = (MUINT32*)Imem_FD_ConfigData.phyAddr;
    g_FdDrv_Para->FD_Config_VA = (MUINT64*)Imem_FD_ConfigData.virtAddr;
}

void FDVT_AllocMem_RSBuffer()
{
    Imem_Size = RS_BUFFER_MAX_SIZE;
    FDVT_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &Imem_RS_BufferData.phyAddr);
    Imem_RS_BufferData.virtAddr=(MUINTPTR)Imem_pLogVir;
    Imem_RS_BufferData.memID=Imem_MemID;
    Imem_RS_BufferData.size=Imem_Size;

    MY_LOGD("rs Buffer Addr PA/VA (0x%lx)/(0x%lx) \n",(unsigned long)Imem_RS_BufferData.phyAddr,(unsigned long)Imem_RS_BufferData.virtAddr);

    g_FdDrv_Para->RS_Result_PA = (MUINT32*)Imem_RS_BufferData.phyAddr;
    g_FdDrv_Para->RS_Result_VA = (MUINT64*)Imem_RS_BufferData.virtAddr;
}

void FDVT_AllocMem_ResultData()
{
    Imem_Size = FD_RESULT_MAX_SIZE;
    FDVT_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &Imem_FD_ResultData.phyAddr);
    Imem_FD_ResultData.virtAddr=(MUINTPTR)Imem_pLogVir;
    Imem_FD_ResultData.memID=Imem_MemID;
    Imem_FD_ResultData.size=Imem_Size;

    MY_LOGD("fd HW result PA/VA (0x%lx)/(0x%lx) \n",(unsigned long)Imem_FD_ResultData.phyAddr,(unsigned long)Imem_FD_ResultData.virtAddr);

    g_FdDrv_Para->FD_Result_PA = (MUINT32*)Imem_FD_ResultData.phyAddr;
    g_FdDrv_Para->FD_Result_VA = (MUINT64*)Imem_FD_ResultData.virtAddr;
}

void FDVT_FreeMem_LearningData(MUINT8 learnDataNum)
{
    for(MINT32 i=0;i<learnDataNum;i++)
    {
       FDVT_Imem_free((MUINT8 *)Imem_Learning_Data[i].virtAddr, Imem_Learning_Data[i].phyAddr, Imem_Learning_Data[i].size, Imem_Learning_Data[i].memID);
    }
}

void FDVT_FreeMem_LearningData_Extra(MUINT8 learnDataNum)
{
    for(MINT32 i=0;i<learnDataNum;i++)
    {
       FDVT_Imem_free((MUINT8 *)Imem_Learning_Data_Extra[i].virtAddr, Imem_Learning_Data_Extra[i].phyAddr, Imem_Learning_Data_Extra[i].size, Imem_Learning_Data_Extra[i].memID);
    }
}

void FDVT_FreeMem_ConfigData()
{
    FDVT_Imem_free((MUINT8 *)Imem_RS_ConfigData.virtAddr, Imem_RS_ConfigData.phyAddr, Imem_RS_ConfigData.size, Imem_RS_ConfigData.memID);
    FDVT_Imem_free((MUINT8 *)Imem_FD_ConfigData.virtAddr, Imem_FD_ConfigData.phyAddr, Imem_FD_ConfigData.size, Imem_FD_ConfigData.memID);
}

void FDVT_FreeMem_RsBufferData()
{
    FDVT_Imem_free((MUINT8 *)Imem_RS_BufferData.virtAddr, Imem_RS_BufferData.phyAddr, Imem_RS_BufferData.size, Imem_RS_BufferData.memID);
}

void FDVT_FreeMem_FdResultData()
{
    FDVT_Imem_free((MUINT8 *)Imem_FD_ResultData.virtAddr, Imem_FD_ResultData.phyAddr, Imem_FD_ResultData.size, Imem_FD_ResultData.memID);
}

void FDVT_InitialDRAM(MUINT32 learning_type)
{
    g_FdDrv_Para->Learning_Type = learning_type;
    FDVT_AllocMem_LearningData();
    FDVT_AllocMem_LearningData_Extra();
    FDVT_AllocMem_RSFDConfigData();
    FDVT_AllocMem_RSBuffer();
    FDVT_AllocMem_ResultData();

    g_FdDrv_Para->GFD_Info.x0 = 0;
    g_FdDrv_Para->GFD_Info.y0 = 0;
    g_FdDrv_Para->GFD_Info.x1 = g_FdDrv_Para->HW_Used_Width;
    g_FdDrv_Para->GFD_Info.y1 = g_FdDrv_Para->HW_Used_Height;
    g_FdDrv_Para->GFD_Info.pose = 0x207;
}

void FDVT_UninitDRAM()
{
    FDVT_FreeMem_LearningData(LEARNDATA_NUM);
    FDVT_FreeMem_LearningData_Extra(EXTRA_LEARNDATA_NUM);
    FDVT_FreeMem_ConfigData();
    FDVT_FreeMem_RsBufferData();
    FDVT_FreeMem_FdResultData();

    g_FdDrv_Para->FD_Result_VA = NULL;
    g_FdDrv_Para->RS_Config_VA = NULL;
    g_FdDrv_Para->FD_Config_VA = NULL;
    g_FdDrv_Para->RS_Result_VA = NULL;

    g_FdDrv_Para->FD_Result_PA = NULL;
    g_FdDrv_Para->RS_Config_PA = NULL;
    g_FdDrv_Para->FD_Config_PA = NULL;
    g_FdDrv_Para->RS_Result_PA = NULL;

    g_FdDrv_Para->Learning_Data0_PA=NULL;
    g_FdDrv_Para->Learning_Data1_PA=NULL;
    g_FdDrv_Para->Learning_Data2_PA=NULL;
    g_FdDrv_Para->Learning_Data3_PA=NULL;
    g_FdDrv_Para->Learning_Data4_PA=NULL;
    g_FdDrv_Para->Learning_Data5_PA=NULL;
    g_FdDrv_Para->Learning_Data6_PA=NULL;
    g_FdDrv_Para->Learning_Data7_PA=NULL;
    g_FdDrv_Para->Learning_Data8_PA=NULL;
    g_FdDrv_Para->Learning_Data9_PA=NULL;
    g_FdDrv_Para->Learning_Data10_PA=NULL;
    g_FdDrv_Para->Learning_Data11_PA=NULL;
    g_FdDrv_Para->Learning_Data12_PA=NULL;
    g_FdDrv_Para->Learning_Data13_PA=NULL;
    g_FdDrv_Para->Learning_Data14_PA=NULL;
    g_FdDrv_Para->Learning_Data15_PA=NULL;
    g_FdDrv_Para->Learning_Data16_PA=NULL;
    g_FdDrv_Para->Learning_Data17_PA=NULL;

    g_FdDrv_Para->Learning_Data0_VA=NULL;
    g_FdDrv_Para->Learning_Data1_VA=NULL;
    g_FdDrv_Para->Learning_Data2_VA=NULL;
    g_FdDrv_Para->Learning_Data3_VA=NULL;
    g_FdDrv_Para->Learning_Data4_VA=NULL;
    g_FdDrv_Para->Learning_Data5_VA=NULL;
    g_FdDrv_Para->Learning_Data6_VA=NULL;
    g_FdDrv_Para->Learning_Data7_VA=NULL;
    g_FdDrv_Para->Learning_Data8_VA=NULL;
    g_FdDrv_Para->Learning_Data9_VA=NULL;
    g_FdDrv_Para->Learning_Data10_VA=NULL;
    g_FdDrv_Para->Learning_Data11_VA=NULL;
    g_FdDrv_Para->Learning_Data12_VA=NULL;
    g_FdDrv_Para->Learning_Data13_VA=NULL;
    g_FdDrv_Para->Learning_Data14_VA=NULL;
    g_FdDrv_Para->Learning_Data15_VA=NULL;
    g_FdDrv_Para->Learning_Data16_VA=NULL;
    g_FdDrv_Para->Learning_Data17_VA=NULL;
}

void FDVT_ResetGFDInfo()
{
    //MY_LOGD("Set GFD Info In\n");
    for (MINT32 m = 0 ; m < FDVT_BUFF_NUM ; m ++)
    {
        g_FdDrv_Para->FD_Result[m].face_index = 14;
        g_FdDrv_Para->FD_Result[m].type = 3 ;
        g_FdDrv_Para->FD_Result[m].x0 = 0 ;
        g_FdDrv_Para->FD_Result[m].y0 = 0 ;
        g_FdDrv_Para->FD_Result[m].x1 = 0 ;
        g_FdDrv_Para->FD_Result[m].y1 = 0 ;
        g_FdDrv_Para->FD_Result[m].fcv = 0 ;
        g_FdDrv_Para->FD_Result[m].rip_dir = 0 ;
        g_FdDrv_Para->FD_Result[m].rop_dir = 0 ;
        g_FdDrv_Para->FD_Result[m].size_index = 0 ;
    }
}

void FDVT_SetHWRIPPose(FdDrv_input_struct *FdDrv_input)
{
    MUINT8 current_feature = FdDrv_input->RIP_feature;
    MY_LOGD("SW RIP Input: %d,\n", current_feature);
    if(current_feature==1)
        g_FdDrv_Para->GFD_Info.pose = 0x3FF;
    else if(current_feature==2)
        g_FdDrv_Para->GFD_Info.pose = 0x3FF;
    else if(current_feature==3)
        g_FdDrv_Para->GFD_Info.pose = 0x3FF;
    else if(current_feature==4)
        g_FdDrv_Para->GFD_Info.pose = 0x5FF;
    else if(current_feature==5)
        g_FdDrv_Para->GFD_Info.pose = 0x9FF;
    else if(current_feature==6)
        g_FdDrv_Para->GFD_Info.pose = 0x5FF;
    else if(current_feature==7)
        g_FdDrv_Para->GFD_Info.pose = 0x9FF;
    else if(current_feature==8)
        g_FdDrv_Para->GFD_Info.pose = 0x5FF;
    else if(current_feature==9)
        g_FdDrv_Para->GFD_Info.pose = 0x9FF;
    else if(current_feature==10)
        g_FdDrv_Para->GFD_Info.pose = 0x11FF;
    else if(current_feature==11)
        g_FdDrv_Para->GFD_Info.pose = 0x11FF;
    else if(current_feature==12)
        g_FdDrv_Para->GFD_Info.pose = 0x11FF;
    else
    {
        g_FdDrv_Para->GFD_Info.pose = 0x3FF;
        MY_LOGE("Error SW RIP Input!!! SW RIP:%d,\n", current_feature);
    }
}

void FDVT_RIPindexFromHWtoFW(FD_Result_Struct *FD_Result)
{
    MUINT32 m = 0;

    for (m = 0 ; m < g_FdDrv_Para->GFD_Total_Num ; m ++)
    {
        switch (FD_Result[m].rip_dir)
        {
            case  0 :
                FD_Result[m].rip_dir = 1 ;
                break ;
            case  1 :
                FD_Result[m].rip_dir = 2 ;
                break ;
            case  2 :
                FD_Result[m].rip_dir = 4 ;
                break ;
            case  3 :
                FD_Result[m].rip_dir = 6 ;
                break ;
            case  4 :
                FD_Result[m].rip_dir = 8 ;
                break ;
            case  5 :
                FD_Result[m].rip_dir = 10 ;
                break ;
            case  6 :
                FD_Result[m].rip_dir = 12 ;
                break ;
            case  7 :
                FD_Result[m].rip_dir = 11 ;
                break ;
            case  8 :
                FD_Result[m].rip_dir = 9 ;
                break ;
            case  9 :
                FD_Result[m].rip_dir = 7 ;
                break ;
            case  10 :
                FD_Result[m].rip_dir = 5 ;
                break ;
            case  11 :
                FD_Result[m].rip_dir = 3 ;
                break ;
            default :
                FD_Result[m].rip_dir = 1 ;
                MY_LOGE(" Error !! Feature pose out of range in main.cpp !!\n") ;
                break ;
        }
    }
}

/*******************************************************************************
* Local FD
********************************************************************************/
void FDVT_RIPindexFromFWtoHW(FdDrv_output_struct *FdDrv_output)
{
    MINT32 m;

    for (m = 0 ; m < 15 ; m ++)
    {
        switch (FdDrv_output->face_feature_set_index[m])
        {
            case  1 :
                g_FdDrv_Para->HW_RIP[m] = 0 ;
                break ;
            case  2 :
                g_FdDrv_Para->HW_RIP[m] = 1 ;
                break ;
            case  3 :
                g_FdDrv_Para->HW_RIP[m] = 11 ;
                break ;
            case  4 :
                g_FdDrv_Para->HW_RIP[m] = 2 ;
                break ;
            case  5 :
                g_FdDrv_Para->HW_RIP[m] = 10 ;
                break ;
            case  6 :
                g_FdDrv_Para->HW_RIP[m] = 3 ;
                break ;
            case  7 :
                g_FdDrv_Para->HW_RIP[m] = 9 ;
                break ;
            case  8 :
                g_FdDrv_Para->HW_RIP[m] = 4 ;
                break ;
            case  9 :
                g_FdDrv_Para->HW_RIP[m] = 8 ;
                break ;
            case  10 :
                g_FdDrv_Para->HW_RIP[m] = 5 ;
                break ;
            case  11 :
                g_FdDrv_Para->HW_RIP[m] = 7 ;
                break ;
            case  12 :
                g_FdDrv_Para->HW_RIP[m] = 6 ;
                break ;
            default :
                g_FdDrv_Para->HW_RIP[m] = 0 ;
                MY_LOGE(" Error !! POSE_RIP_FW2HW out of range !!\n") ;
                break ;
        }
    }
}

void FDVT_DramConfig(FdDrv_input_struct *FdDrv_input)
{
    //MY_LOGD("RS Register Config Start\n");
    //MY_LOGD("Size of MINTPTR: %d\n", sizeof(MINTPTR));
    MUINT64  *image_buffer_address = FdDrv_input->source_img_address;
    MUINT64  *image_buffer_address_UV = FdDrv_input->source_img_address_UV;
    g_FdDrv_Para->SW_Input_Width[0] = FdDrv_input->source_img_width[0];
    g_FdDrv_Para->SW_Input_Height[0] = FdDrv_input->source_img_height[0];
    MUINT64  *rs_cfg;
    MUINT64  *fd_cfg;
    MUINT32  rs_dst_addr = 0x0;
    MUINT64  rs_dst_addr_64 = 0x0;
    MUINT32  srcbuf = 0x0, srcbuf_UV = 0x0;
    MUINT64  srcbuf_64 = 0x0, srcbuf_UV_64 = 0x0;
    MUINT16  src_w = 0x0, src_h = 0x0, des_w = 0x0, des_h = 0x0;
    MUINT16  rFactor = 0x0, sFactor = 0x0, iFactor = 0x0;
    //MUINT16  src_h, src_w;
    MUINT8    rsLoop;
    //MUINT16   XMAG_YMAG; //rfactor //unused
    //MUINT8    SRCFORMAT; //unused
    MUINT32   SCALEFORMAT_1stSCALE = 0;
    MUINT32   SCALEFORMAT_OtherSCALE = 0;
    //**************************************//

    if(FdDrv_input->scale_manual_mode == 1)
    {
        memcpy( (MUINT8*)(Imem_RS_ConfigData.virtAddr), (MUINT8*)(rs_config_data_640x480), sizeof(rs_config_data_640x480));
        memcpy( (MUINT8*)(Imem_FD_ConfigData.virtAddr), (MUINT8*)(fd_config_data_640x480), sizeof(fd_config_data_640x480));
        g_FdDrv_Para->HW_Used_Width = FdDrv_input->source_img_width[0];//640;
        g_FdDrv_Para->HW_Used_Height = FdDrv_input->source_img_height[0];//480;
        g_FdDrv_Para->RS_Num = FdDrv_input->scale_num_from_user;
        if (g_FdDrv_Para->RS_Num > FACE_SIZE_NUM_MAX)
        {
            g_FdDrv_Para->RS_Num = FACE_SIZE_NUM_MAX;
            MY_LOGE("RS_Num Bigger than MAX, set to default value: %d\n", g_FdDrv_Para->RS_Num);
        }
    }
    else
    {
        if(FdDrv_input->fd_mode==0)  //0:FD
        {
            memcpy( (MUINT8*)(Imem_RS_ConfigData.virtAddr), (MUINT8*)(rs_config_data_640x480), sizeof(rs_config_data_640x480));
            memcpy( (MUINT8*)(Imem_FD_ConfigData.virtAddr), (MUINT8*)(fd_config_data_640x480), sizeof(fd_config_data_640x480));
            g_FdDrv_Para->HW_Used_Width = 640;
            g_FdDrv_Para->HW_Used_Height = 480;
            g_FdDrv_Para->RS_Num = 14;
        }
        else if(FdDrv_input->fd_mode==1) //1:SD, 2:vFB  3:cFB
        {
            memcpy( (MUINT8*)(Imem_RS_ConfigData.virtAddr), (MUINT8*)(rs_config_data_400x300), sizeof(rs_config_data_400x300));
            memcpy( (MUINT8*)(Imem_FD_ConfigData.virtAddr), (MUINT8*)(fd_config_data_400x300), sizeof(fd_config_data_400x300));
            g_FdDrv_Para->HW_Used_Width = 640;
            g_FdDrv_Para->HW_Used_Height = 480;
            g_FdDrv_Para->RS_Num = 12;
        }
        else if(FdDrv_input->fd_mode==2)
        {
            memcpy( (MUINT8*)(Imem_RS_ConfigData.virtAddr), (MUINT8*)(rs_config_data_320x240), sizeof(rs_config_data_320x240));
            memcpy( (MUINT8*)(Imem_FD_ConfigData.virtAddr), (MUINT8*)(fd_config_data_320x240), sizeof(fd_config_data_320x240));
            g_FdDrv_Para->HW_Used_Width  = 640;
            g_FdDrv_Para->HW_Used_Height = 480;
            g_FdDrv_Para->RS_Num = 11;
        }
    }
    g_FdDrv_Para->GFD_Info.x1 = g_FdDrv_Para->HW_Used_Width;
    g_FdDrv_Para->GFD_Info.y1 = g_FdDrv_Para->HW_Used_Height;
    g_FdDrv_Para->RS_Config_PA = (MUINT32*)Imem_RS_ConfigData.phyAddr;
    g_FdDrv_Para->RS_Config_VA = (MUINT64*)Imem_RS_ConfigData.virtAddr;
    g_FdDrv_Para->FD_Config_PA = (MUINT32*)Imem_FD_ConfigData.phyAddr;
    g_FdDrv_Para->FD_Config_VA = (MUINT64*)Imem_FD_ConfigData.virtAddr;

    //MY_LOGD("RS config data PA/VA (0x%p)/(0x%p) \n",g_FdDrv_Para->RS_Config_PA,g_FdDrv_Para->RS_Config_VA);
    //MY_LOGD("FD config data PA/VA (0x%p)/(0x%p) \n",g_FdDrv_Para->FD_Config_PA,g_FdDrv_Para->FD_Config_VA);
    //**************************************//
    rs_cfg  = g_FdDrv_Para->RS_Config_VA;
    fd_cfg  = g_FdDrv_Para->FD_Config_VA;

    rs_dst_addr_64 = (MUINT64)g_FdDrv_Para->RS_Result_PA;
    rs_dst_addr = rs_dst_addr_64 & 0x00000000ffffffff;
    srcbuf_64=(MUINT64)image_buffer_address; //phyical address
    srcbuf = srcbuf_64 & 0x00000000ffffffff;
    srcbuf_UV_64=(MUINT64)image_buffer_address_UV;
    srcbuf_UV = srcbuf_UV_64 & 0x00000000ffffffff;

    MY_LOGD("FD Config: Manual(%d) Src_W(%d) Src_H(%d) Format(%d) fd_mode(%d) RIP(%d) rs_num(%d)\n", FdDrv_input->scale_manual_mode, FdDrv_input->source_img_width[0], FdDrv_input->source_img_height[0], FdDrv_input->source_img_fmt, FdDrv_input->fd_mode, FdDrv_input->RIP_feature, g_FdDrv_Para->RS_Num);

    if (FdDrv_input->source_img_fmt == FMT_YUYV || FdDrv_input->source_img_fmt == FMT_UYVY || FdDrv_input->source_img_fmt == FMT_YUV_2P)
    {
        SCALEFORMAT_1stSCALE = 0x00030000;
        SCALEFORMAT_OtherSCALE = 0x00330000;
    }
    else if (FdDrv_input->source_img_fmt == FMT_YVYU || FdDrv_input->source_img_fmt == FMT_VYUY || FdDrv_input->source_img_fmt == FMT_YVU_2P)
    {
        SCALEFORMAT_1stSCALE = 0x00020000;
        SCALEFORMAT_OtherSCALE = 0x00220000;
    }
    else
        MY_LOGE("FdDrv_input->source_img_fmt Error!\n");


    /* RS_CONFIG Reg is 256 bit = 8 * 32bits */
    for (rsLoop = 0; rsLoop < g_FdDrv_Para->RS_Num; rsLoop++)
    {
        /* Set source addr for all scale of RS buffer */
        if (rsLoop==0)
        {
            // 1st rs src Y addr = input Y addr
            *((MUINT32 *)rs_cfg) = srcbuf;
            // 1st rs src UV addr = input UV addr
            *((MUINT32 *)rs_cfg+6) = srcbuf_UV;
            // fill up format field of input source image
            *((MUINT32 *)rs_cfg+5) = (*((MUINT32 *)rs_cfg+5) & 0xFF88FFFF) | (((FdDrv_input->source_img_fmt << 20) | SCALEFORMAT_1stSCALE) & 0x00770000);
        }
        else
        {
            // Nth rs src addr = (N -1)th rs dst addr
            *((MUINT32 *)rs_cfg+8*rsLoop) = *((MUINT32 *)rs_cfg+8*(rsLoop-1)+1);
            *((MUINT32 *)rs_cfg+8*rsLoop+5) = (*((MUINT32 *)rs_cfg+8*rsLoop+5) & 0xFF88FFFF) | SCALEFORMAT_OtherSCALE;
        }

        /* Set destination addr for all scale of RS buffer */
        *((MUINT32 *)rs_cfg+8*rsLoop+1) = rs_dst_addr;

        /* fd src addr (inverse order before FD 3.5), the order should be in forward direction in FD 4.0 (normal order) */
        *((MUINT32 *)fd_cfg+4*rsLoop) = rs_dst_addr;

        if (FdDrv_input->scale_manual_mode != 1)
        {
            /* Need to calculate destination IMG's width & height of each RS buffer */
            des_w = ((*((MUINT32 *)rs_cfg+8*rsLoop+3) & 0x7)<<8)|((*((MUINT32 *)rs_cfg+8*rsLoop+2) >> 24) & 0xFF);
            des_h = ((*((MUINT32 *)rs_cfg+8*rsLoop+3) >> 4) & 0x7FF);
        }
        else
        {
            src_w = FdDrv_input->source_img_width[rsLoop];
            src_h = FdDrv_input->source_img_height[rsLoop];
            des_w = FdDrv_input->source_img_width[rsLoop+1];
            des_h = FdDrv_input->source_img_height[rsLoop+1];

            rFactor = src_w * 512 / des_w;

            des_h = src_h * 512 / rFactor;
            FdDrv_input->source_img_height[rsLoop+1] = des_h;

            if (des_h < FDVT_SCALE_SIZE_LIMITATION ||
				des_w < FDVT_SCALE_SIZE_LIMITATION)
            {
                g_FdDrv_Para->RS_Num = rsLoop;
                break;
            }

            sFactor = FdDrv_input->source_img_width[0] * 512 / des_w;
            iFactor = des_w * 512 / FdDrv_input->source_img_width[0];
            *((MUINT32 *)rs_cfg+8*rsLoop+2) = src_w | (src_h << 12) | ((des_w & 0x0FF) << 24);
            *((MUINT32 *)rs_cfg+8*rsLoop+3) = ((des_w & 0xF00) >> 8) | (des_h << 4) | (rFactor << 16);
            *((MUINT32 *)rs_cfg+8*rsLoop+5) = (*((MUINT32 *)rs_cfg+8*rsLoop+5) & 0xFFFFF000) | src_w; //stride = src_w

            *((MUINT32 *)fd_cfg+4*rsLoop+1) = des_w | (des_h << 10) | (0x3 << 20);
            *((MUINT32 *)fd_cfg+4*rsLoop+2) = sFactor | (iFactor << 15);
            *((MUINT32 *)fd_cfg+4*rsLoop+3) = des_w << 16;
        }

        MY_LOGD("Loop(%2d) srcW(%3d) srcH(%3d) desW(%3d) desH(%3d) rFactor(%3d) sFactor(%4d) iFactor(%3d)\n", rsLoop, src_w, src_h, des_w, des_h, rFactor, sFactor, iFactor);
        //MY_LOGD("RS_Config, rsLoop = %d, src_addr = 0x%p, dst_addr = 0x%p,\n", rsLoop, *((MUINT32 *)rs_cfg+8*rsLoop), *((MUINT32 *)rs_cfg+8*rsLoop+1));

        /* Calculate next destination address */
        rs_dst_addr += des_w * des_h * 2; // 2-plane to 1-plane YUV, size multiply 2

        // round to 16 bytes align
        if ((rs_dst_addr & 0xf) != 0)
        {
            rs_dst_addr = (rs_dst_addr & ~0xf) + 0x10;
        }
    }
}

void FDVT_FDRegisterConfig(FdDrv_input_struct *FdDrv_input)
{
    MUINT8 GFD_skip_H = FdDrv_input->GFD_skip;
    MUINT8 GFD_skip_V = FdDrv_input->GFD_skip_V;

    g_FdDrv_Para->GFD_Skip_H = GFD_skip_H;
    g_FdDrv_Para->GFD_Skip_V = GFD_skip_V;
    //if(g_GFD_PatternType>g_FdDrv_Para->GFD_Skip_Num)
    //{
    //    g_GFD_PatternType = 0;
    //}

    MY_LOGD("FDRegisterConfig In, Skip_H:%d, Skip_V:%d\n", g_FdDrv_Para->GFD_Skip_H, g_FdDrv_Para->GFD_Skip_V);

    MINT32 i = 0, idx;
    MINT32 LFDCount=0;

    /* Clear LFD Ctrl Registers (0~14) */
    for(i=0;i<15;i++){
         idx = 0x00a4+i*12+0;
         FdReg_Addr[i*3]=idx;
         FdReg_Value[i*3]=0;
         FdReg_Addr[(i*3)+1]=idx+4;
         FdReg_Value[(i*3)+1]=0;
         FdReg_Addr[(i*3)+2]=idx+8;
         FdReg_Value[(i*3)+2]=0;
         LFDCount++;
         //LOGD("LFD_INFO x0:%x y0:%x x1:%x y1:%x \n",LFD_INFO[i].x0,LFD_INFO[i].y0,LFD_INFO[i].x1,LFD_INFO[i].y1);
         //LOGD("LFD_INFO Adr:0x%x Val1:0x%x Val2:0x%x Val3:0x%x\n",FdReg_Addr[i*3],FdReg_Value[i*3],FdReg_Value[(i*3)+1],FdReg_Value[(i*3)+2]);
    }
    i*=3;

    /* Always set (0, 0, src_width, src_height) as original setting */
    FdReg_Addr[i]=0x008c;
    FdReg_Value[i]=(MINT32)((g_FdDrv_Para->GFD_Info.y0   &0x03ff)<< 16   )+
                     (MINT32)((g_FdDrv_Para->GFD_Info.x0   &0x03ff)<<  0   );
    FdReg_Addr[i+1]=0x0090;
    FdReg_Value[i+1]=(MINT32)((g_FdDrv_Para->GFD_Info.y1   &0x03ff)<< 16   )+
                     (MINT32)((g_FdDrv_Para->GFD_Info.x1   &0x03ff)<<  0   );

    /* Set ROP/RIP bit in 64-bit register (FD3.5) */
    #if 0
    FdReg_Addr[i+2]=0x0094;
    FdReg_Value[i+2]=(MINT32)((g_FdDrv_Para->GFD_Info.pose   &0xffffffff)<<  0   );
    FdReg_Addr[i+3]=0x0098;
    FdReg_Value[i+3]=(MINT32)(( ((MUINT64)g_FdDrv_Para->GFD_Info.pose>>32) &0x0fffffff)<<  0   );
    #endif
    /* Set ROP/RIP bit in 64-bit register (FD4.0)*/
    FdReg_Addr[i+2]=0x0094;
    FdReg_Value[i+2]=(MINT32)((g_FdDrv_Para->GFD_Info.pose   &0x1fff)<<  0   );
    //FdReg_Addr[i+3]=0x0098;
    //FdReg_Value[i+3]=(MINT32)(( ((MUINT64)g_FdDrv_Para->GFD_Info.pose>>32) &0x0fffffff)<<  0   );


    /* Set source image's width & height */
    FdReg_Addr[i+3]=0x0160;
    FdReg_Value[i+3]=(MINT32)((g_FdDrv_Para->HW_Used_Width&0x03ff)<< 16   )+
                       (MINT32)((g_FdDrv_Para->HW_Used_Height&0x03ff)<<  0   );

    /* Indicate the address of RS config data in DRAM */
    FdReg_Addr[i+4]=0x000C;
    FdReg_Value[i+4]=(MUINT32)((MUINTPTR)g_FdDrv_Para->RS_Config_PA & 0x00000000ffffffff);

    /* Indicate the address of FD config data in DRAM */
    FdReg_Addr[i+5]=0x0080;
    FdReg_Value[i+5]=(MUINT32)((MUINTPTR)g_FdDrv_Para->FD_Config_PA & 0x00000000ffffffff);

    /* Indicate the address of where to store the FD result*/
    FdReg_Addr[i+6]=0x009C;
    FdReg_Value[i+6]=(MUINT32)((MUINTPTR)g_FdDrv_Para->FD_Result_PA & 0x00000000ffffffff);

    /* Should ask DE whether this config need modify */
    //FdReg_Addr[i+8]=0x0078;
    //FdReg_Value[i+8]=REG_RMAP;

    /* Set RS_ENABLE = FD_ENABLE = FF_ENABLE = 1 */
    FdReg_Addr[i+7]=0x0004;
    FdReg_Value[i+7]=0x111;

    /* Indicate the address of learning data */
    FdReg_Addr[i+8]=0x0054;
    FdReg_Value[i+8]=(FdDrv_input->dynamic_change_model[0] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data0_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data0_PA);
    FdReg_Addr[i+9]=0x0058;
    FdReg_Value[i+9]=(FdDrv_input->dynamic_change_model[1] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data1_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data1_PA);
    FdReg_Addr[i+10]=0x005C;
    FdReg_Value[i+10]=(FdDrv_input->dynamic_change_model[2] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data2_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data2_PA);
    FdReg_Addr[i+11]=0x0060;
    FdReg_Value[i+11]=(FdDrv_input->dynamic_change_model[3] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data3_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data3_PA);
    FdReg_Addr[i+12]=0x0064;
    FdReg_Value[i+12]=(FdDrv_input->dynamic_change_model[4] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data4_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data4_PA);
    FdReg_Addr[i+13]=0x0068;
    FdReg_Value[i+13]=(FdDrv_input->dynamic_change_model[5] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data5_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data5_PA);
    FdReg_Addr[i+14]=0x006C;
    FdReg_Value[i+14]=(FdDrv_input->dynamic_change_model[6] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data6_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data6_PA);
    FdReg_Addr[i+15]=0x0070;
    FdReg_Value[i+15]=(FdDrv_input->dynamic_change_model[7] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data7_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data7_PA);
    FdReg_Addr[i+16]=0x01A4;
    FdReg_Value[i+16]=(FdDrv_input->dynamic_change_model[8] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data8_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data8_PA);
    FdReg_Addr[i+17]=0x01A8;
    FdReg_Value[i+17]=(FdDrv_input->dynamic_change_model[9] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data9_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data9_PA);
    FdReg_Addr[i+18]=0x01AC;
    FdReg_Value[i+18]=(FdDrv_input->dynamic_change_model[10] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data10_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data10_PA);
    FdReg_Addr[i+19]=0x01B0;
    FdReg_Value[i+19]=(FdDrv_input->dynamic_change_model[11] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data11_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data11_PA);
    FdReg_Addr[i+20]=0x01B4;
    FdReg_Value[i+20]=(FdDrv_input->dynamic_change_model[12] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data12_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data12_PA);
    FdReg_Addr[i+21]=0x01B8;
    FdReg_Value[i+21]=(FdDrv_input->dynamic_change_model[13] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data13_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data13_PA);
    FdReg_Addr[i+22]=0x01BC;
    FdReg_Value[i+22]=(FdDrv_input->dynamic_change_model[14] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data14_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data14_PA);
    FdReg_Addr[i+23]=0x01C0;
    FdReg_Value[i+23]=(FdDrv_input->dynamic_change_model[15] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data15_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data15_PA);
    FdReg_Addr[i+24]=0x01D8;
    FdReg_Value[i+24]=(FdDrv_input->dynamic_change_model[16] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data16_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data16_PA);
    FdReg_Addr[i+25]=0x01DC;
    FdReg_Value[i+25]=(FdDrv_input->dynamic_change_model[17] == MTRUE)?(MUINT32)((MUINTPTR)g_FdDrv_Para->Extra_Learning_Data17_PA):(MUINT32)((MUINTPTR)g_FdDrv_Para->Learning_Data17_PA);

    //FD4.0 don't use this register
    //MUINT8 feature_TH = FdDrv_input->feature_threshold;
    //FdReg_Addr[i+29]=0x0074;
    //FdReg_Value[i+29]= (0x3F000000 | ((feature_TH<<16)&0x00FF0000) | ((feature_TH<<8)&0x0000FF00) | (feature_TH&0x000000FF));

    /* Set the GFD Skip Mode & GFD Pattern Type */
    FdReg_Addr[i+26]=0x0084;
    FdReg_Value[i+26]=((g_FdDrv_Para->GFD_Skip_V<<16) & 0x001F0000)|((g_FdDrv_Para->GFD_Skip_H<<3) & 0x00000008);
    //FdReg_Value[i+26]=((g_GFD_PatternType<<4)&0x70)|(g_FdDrv_Para->GFD_Skip_Num & 0x07);
    /* GFD_PATTERN_TYPE should be equal or less than g_FdDrv_Para->GFD_Skip_Num */
    //g_GFD_PatternType++;
    //if(g_GFD_PatternType>g_FdDrv_Para->GFD_Skip_Num)
    //{
    //    g_GFD_PatternType = 0;
    //}
    //LOGD("SVM=%d, Skip Mode Type = [%d %d %d %d %x]",SVM_OPITION,GFD_SKIP_MODE,g_GFD_PatternType,LFD_SKIP_MODE,LFD_PATTERN_TYPE,FdReg_Value[i+23]);

    /* Set FD Loop Num */
    FdReg_Addr[i+27]=0x007C;
    FdReg_Value[i+27]=0x04000000 | g_FdDrv_Para->RS_Num;

    /* Set RS Loop Num */
    FdReg_Addr[i+28]=0x0008;
    FdReg_Value[i+28]=0x00000400 | g_FdDrv_Para->RS_Num;

    /* Control the Learning Data Number included */
    //FdReg_Addr[i+33]=0x0024;
    //if(g_FdDrv_Para->Learning_Type == 1)      //1:Black human
    //{
    //    FdReg_Value[i+33]=0x02310231;
    //}
    //else                          //0:Non-Black human
    //{
    //    FdReg_Value[i+33]=0x01D10203;
    //}

    /* Set register value to kernel space */
    FDVT_IOCTL_ParaSetting(FdReg_Addr,FdReg_Value,i+29,FDVT_MODE_GFD);

    //MY_LOGD("FDRegisterConfig Out\n");
}

MINT32 FDVT_FDGetHWResult()
{
    //MY_LOGD("FDGetHWResult In\n");
    MUINT32 i;
    MUINT32 reg_fdvt_result;
    FD_Result_Struct *FD_Result;
    MUINT32 reg_val;
    MUINT32 GFD_Num;
    MINT32  ret = 0;
    ret = FDVT_IOCTL_WaitIRQ();
    if (ret != 0x0)
    {
        MY_LOGE("FD IRQ ERROR! RET: %d\n", ret);
        return -1;
    }
    //Do IMEM invalidate before Get HW Result
    FDVT_Imem_invalid();

    FD_Result = g_FdDrv_Para->FD_Result;

    /* Use IOCTL to get the GFD_NUM value calculated by HW */
    FdReg_Addr[0] = FDVT_RESULT;
    FDVT_IOCTL_GetHWResult(FdReg_Addr,FdReg_Value,1,reg_fdvt_result);
    GFD_Num = (reg_fdvt_result & 0x000007FF);
    #if 0
    LFD_NUM = (reg_fdvt_result & 0x01FF0000);
    #endif
    //MY_LOGD("HW_Result: GFD_NUM: %d L_TYPE:%d, \n",GFD_Num, g_FdDrv_Para->Learning_Type);

    #if 0
    FdReg_Addr[0]=FDVT_RESULT;
    FDVT_IOCTL_GetHWResult(FdReg_Addr,FdReg_Value,1,reg_fdvt_result);
    for(i=0;i<15;i++)
    {
        LFD_INFO[i].valid = ((reg_fdvt_result >> (16+i)) & 0x1);
    }
    #endif

    g_FdDrv_Para->GFD_Total_Num = MIN((GFD_Num/* + LFD_NUM*/), 1024);

    for(i = 0; i < g_FdDrv_Para->GFD_Total_Num; i++)
    {
        //reg_val = g_FdDrv_Para->FD_Result_VA[4*i];
        reg_val = g_FdDrv_Para->FD_Result_VA[2*i] & 0x00000000ffffffff;
        //MY_LOGD("reg_val_32 = %x\n", reg_val_32);
        FD_Result[i].y0 = ((reg_val>>23)&0x1ff);  //9 bits, remain 1 bit in next word
        FD_Result[i].x0 = ((reg_val>>13)&0x3ff);  //10 bits
        FD_Result[i].type = ((reg_val>>12)&0x001);
        FD_Result[i].face_index = ((reg_val>>0)&0xfff);

        //reg_val = g_FdDrv_Para->FD_Result_VA[4*i+1];
        reg_val = (g_FdDrv_Para->FD_Result_VA[2*i] & 0xffffffff00000000) >> 32;
        //MY_LOGD("reg_val_32 = %x\n", reg_val_32);
        FD_Result[i].fcv = ((reg_val>>21)&0x7ff); //11 bits, remain 7 bits in next word
        FD_Result[i].y1 = ((reg_val>>11)&0x3ff);
        FD_Result[i].x1 = ((reg_val>>1)&0x3ff);
        FD_Result[i].y0 |= (((reg_val>>0)&0x001)<<9);

        //reg_val = g_FdDrv_Para->FD_Result_VA[4*i+2];
        reg_val = (g_FdDrv_Para->FD_Result_VA[2*i+1] & 0x00000000ffffffff);
        //MY_LOGD("reg_val_32 = %x\n", reg_val_32);
        FD_Result[i].size_index = ((reg_val>>14)&0x01f);
        FD_Result[i].rop_dir = ((reg_val>>11)&0x007);
        FD_Result[i].rip_dir = ((reg_val>>7)&0x00f);
        FD_Result[i].fcv |= (((reg_val>>0)&0x07f)<<11);

        //MY_LOGD("FD_Result[%d]: x0 %d  y0 %d  x1 %d  y1 %d\n",i,FD_Result[i].x0,FD_Result[i].y0,FD_Result[i].x1,FD_Result[i].y1);
        //MY_LOGD("FD_Result[%d]: rip: %d  top: %d %d\n",i,FD_Result[i].rip_dir,FD_Result[i].rop_dir);
        //MY_LOGD("GetHWResult [%d] fcv: %d,",i, FD_Result[i].fcv);
    }

/*
    for(i = 0; i < g_FdDrv_Para->GFD_Total_Num; i++)
    {
        reg_val = g_FdDrv_Para->FD_Result_VA[4*i];
        FD_Result[i].y0 = ((reg_val>>23)&0x1ff);  //9 bits, remain 1 bit in next word
        FD_Result[i].x0 = ((reg_val>>13)&0x3ff);  //10 bits
        FD_Result[i].type = ((reg_val>>12)&0x001);
        FD_Result[i].face_index = ((reg_val>>0)&0xfff);

        reg_val = g_FdDrv_Para->FD_Result_VA[4*i+1];
        FD_Result[i].fcv = ((reg_val>>21)&0x7ff); //11 bits, remain 7 bits in next word
        FD_Result[i].y1 = ((reg_val>>11)&0x3ff);
        FD_Result[i].x1 = ((reg_val>>1)&0x3ff);
        FD_Result[i].y0 |= (((reg_val>>0)&0x001)<<9);

        reg_val = g_FdDrv_Para->FD_Result_VA[4*i+2];
        FD_Result[i].size_index = ((reg_val>>14)&0x01f);
        FD_Result[i].rop_dir = ((reg_val>>11)&0x007);
        FD_Result[i].rip_dir = ((reg_val>>7)&0x00f);
        FD_Result[i].fcv |= (((reg_val>>0)&0x07f)<<11);

        MY_LOGD("FD_Result[%d]: x0 %d  y0 %d  x1 %d  y1 %d\n",i,FD_Result[i].x0,FD_Result[i].y0,FD_Result[i].x1,FD_Result[i].y1);
        MY_LOGD("FD_Result[%d]: rip: %d  top: %d %d\n",i,FD_Result[i].rip_dir,FD_Result[i].rop_dir);
        //MY_LOGD("GetHWResult [%d] fcv: %d,",i, FD_Result[i].fcv);
    }
*/
#if FDVT_RAW_DUMP
    static MINT32 Count=0;

    MY_LOGI("Set: g_FdDrv_Para->RS_Result_VA (0x%p) \n",g_FdDrv_Para->RS_Result_VA);
    Count++;
    if((Count%4) ==0)
    {
       char szFileName[100];
       MY_LOGI(szFileName, "/data/fd_data/320x240_Y_%d.raw", Count);

       FILE * pRawFp = fopen(szFileName, "wb");
       if (NULL == pRawFp )
       {
           MY_LOGE("Can't open file to save RAW Image\n");
           while(1);
       }
       MINT32 i4WriteCnt = fwrite((void *)g_FdDrv_Para->RS_Result_VA,2, (320*240),pRawFp);
       fflush(pRawFp);
       fclose(pRawFp);

    }
#endif

    //FDVT_IOCTL_DumpReg();
    //MY_LOGD("FDGetHWResult Out\n");
    return 0;
}

void FDVT_SetFDResultToSW(FdDrv_output_struct *FdDrv_output)
{
    FDVT_RIPindexFromHWtoFW(g_FdDrv_Para->FD_Result);
    MUINT32 i;
    MINT32 skip_face = 0;
    //MUINT32 limit_Height = g_FdDrv_Para->HW_Used_Width * g_FdDrv_Para->SW_Input_Height[0] / g_FdDrv_Para->SW_Input_Width[0];
    MUINT32 limit_Height = g_FdDrv_Para->HW_Used_Height;
    //MY_LOGD("Limit Height = %d\n", limit_Height);
    //FdDrv_output->new_face_number = g_total_fd_num;
    FdDrv_output->new_face_number = 0;

    for(i = 0; i < g_FdDrv_Para->GFD_Total_Num; i++)
    {
        if( g_FdDrv_Para->FD_Result[i].y1 < limit_Height )
        {
            FdDrv_output->face_candi_pos_x0[i] = g_FdDrv_Para->FD_Result[i].x0;
            FdDrv_output->face_candi_pos_y0[i] = g_FdDrv_Para->FD_Result[i].y0;
            FdDrv_output->face_candi_pos_x1[i] = g_FdDrv_Para->FD_Result[i].x1;
            FdDrv_output->face_candi_pos_y1[i] = g_FdDrv_Para->FD_Result[i].y1;

            //MY_LOGD("Check FD result:i=%d, x0:%d, y0:%d, x1:%d, y1:%d, \n",i, g_FdDrv_Para->FD_Result[i].x0, g_FdDrv_Para->FD_Result[i].y0, g_FdDrv_Para->FD_Result[i].x1, g_FdDrv_Para->FD_Result[i].y1);
            //MY_LOGD("RIP:%d, ROP:%d, size_index:%d, ", g_FdDrv_Para->FD_Result[i].rip_dir, g_FdDrv_Para->FD_Result[i].rop_dir, g_FdDrv_Para->FD_Result[i].size_index);
            //MY_LOGD("After SW:x0:%d, y0:%d, x1:%d, y1:%d, \n",FdDrv_output->face_candi_pos_x0[i], FdDrv_output->face_candi_pos_y0[i], FdDrv_output->face_candi_pos_x1[i], FdDrv_output->face_candi_pos_y1[i]);

            FdDrv_output->face_feature_set_index[i] =  g_FdDrv_Para->FD_Result[i].rip_dir;
            FdDrv_output->rip_dir[i] =  g_FdDrv_Para->FD_Result[i].rip_dir;
            FdDrv_output->rop_dir[i] =  g_FdDrv_Para->FD_Result[i].rop_dir;
            FdDrv_output->face_reliabiliy_value[i]  =  g_FdDrv_Para->FD_Result[i].fcv;
            FdDrv_output->display_flag[i] = MTRUE;
            FdDrv_output->detected_face_size_label[i] = g_FdDrv_Para->FD_Result[i].size_index;
            FdDrv_output->new_face_number++;

            //MY_LOGD("ResultToSW [%d] fcv: %d,",FdDrv_output->new_face_number, FdDrv_output->face_reliabiliy_value[i]);
        }
        else
        {
            MY_LOGD("Over Region :i=%d, x0:%d, y0:%d, x1:%d, y1:%d, \n",i, g_FdDrv_Para->FD_Result[i].x0, g_FdDrv_Para->FD_Result[i].y0, g_FdDrv_Para->FD_Result[i].x1, g_FdDrv_Para->FD_Result[i].y1);
            skip_face++;
        }
    }
    MY_LOGI("Final: HW_NUM:%d, Filter_NUM:%d, SKIP_NUM:%d\n", g_FdDrv_Para->GFD_Total_Num, FdDrv_output->new_face_number, skip_face);

    //MY_LOGD("Check FD result1: x0:%d, y0:%d, x1:%d, y1:%d, ", g_FdDrv_Para->FD_Result[0].x0, g_FdDrv_Para->FD_Result[0].y0, g_FdDrv_Para->FD_Result[0].x1, g_FdDrv_Para->FD_Result[0].y1);
    //MY_LOGD("RIP:%d, ROP:%d, size_index:%d, ", g_FdDrv_Para->FD_Result[0].rip_dir, g_FdDrv_Para->FD_Result[0].rop_dir, g_FdDrv_Para->FD_Result[0].size_index);

    //MY_LOGD("Check FD result2: x0:%d, y0:%d, x1:%d, y1:%d, ", g_FdDrv_Para->FD_Result[1].x0, g_FdDrv_Para->FD_Result[1].y0, g_FdDrv_Para->FD_Result[1].x1, g_FdDrv_Para->FD_Result[1].y1);
    //MY_LOGD("RIP:%d, ROP:%d, size_index:%d, ", g_FdDrv_Para->FD_Result[1].rip_dir, g_FdDrv_Para->FD_Result[1].rop_dir, g_FdDrv_Para->FD_Result[1].size_index);

}

MINT32 FDVT_OpenDriverWithUserCount(MUINT32 learning_type)
{
    android::Mutex::Autolock lock(m_FDInitMutex);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.fdvt.debug.enable", value, "0");
    g_isLogEnable = atoi(value);

    if (g_UserCount == 0)
    {
        MY_LOGI("FDVT_Init, HW FD Open CLK\n");
        FDVT_IOCTL_OpenDriver();
        FDVT_InitialDRAM(learning_type);
    }
    else if (g_UserCount > 0)
    {
        MY_LOGI("FDVT_Init Many Times, UserCount = %d\n", g_UserCount);
    }
    else if (g_UserCount < 0)
    {
        MY_LOGI("FDVT UserCount(%d) < 0, Reset UserCount = 0\n", g_UserCount);
        g_UserCount = 0;
    }
    g_UserCount ++;

    return g_UserCount;
}

MINT32 FDVT_CloseDriverWithUserCount()
{
    android::Mutex::Autolock lock(m_FDInitMutex);
    g_UserCount --;
    if (g_UserCount == 0)
    {
        MY_LOGI("FDVT_Uninit, HW FD Close CLK\n");
        FDVT_UninitDRAM();
        FDVT_IOCTL_CloseDriver();
    }
    else if (g_UserCount > 0)
    {
        MY_LOGI("FDVT_Uninit Many Times, UserCount = %d\n", g_UserCount);
    }
    else if (g_UserCount < 0)
    {
        MY_LOGI("FDVT UserCount(%d) < 0, Reset UserCount = 0\n", g_UserCount);
        g_UserCount = 0;
    }

    return g_UserCount;
}

MINT32 FDVT_GetModelVersion()
{
    if (g_UserCount > 0)
    {
        if (g_FdDrv_Para->Learning_Type == 0)
        {
            return (unsigned int)model1_version;
        }
        else if (g_FdDrv_Para->Learning_Type == 1)
        {
            return (unsigned int)model2_version;
        }
        else
        {
            MY_LOGE("Illegal learning type");
            return -1;
        }
    }
    else
    {
        MY_LOGE("Please initial FD driver first");
        return -1;
    }
}

void FDVT_Enque(FdDrv_input_struct *FdDrv_input)
{
    MY_LOGD("FDVT Enque Start\n");
    g_EnquedStatus = MTRUE;
    g_FdDrv_input = FdDrv_input;
    FDVT_DramConfig(FdDrv_input);

    //MY_LOGD("getchar_RS_DONE()");
    //getchar();

    FDVT_SetHWRIPPose(FdDrv_input);
    FDVT_FDRegisterConfig(FdDrv_input);

    //MY_LOGD("getchar_FD_DONE()\n");
    //getchar();

    //MY_LOGD("IMEM=>RS: PA/VA (0x%lx)/(0x%lx) id (%d)\n", (unsigned long)Imem_RS_ConfigData.phyAddr, (unsigned long)Imem_RS_ConfigData.virtAddr, Imem_RS_ConfigData.memID);
    //MY_LOGD("RS config data PA/VA (0x%lx)/(0x%lx) \n",(unsigned long)g_FdDrv_Para->RS_Config_PA,(unsigned long)g_FdDrv_Para->RS_Config_VA);

#if FDVT_REG_DUMP
    MUINT32 reg_fdvt[FDVT_PARA_NUM];
    for (MUINT32 u4Index = 0; u4Index <= 0x1DC; u4Index += 0x4)
    {
        FdReg_Addr[u4Index/0x4] = u4Index;
    }
    MUINT32 result = 0;
    FDVT_IOCTL_GetHWResult(FdReg_Addr,FdReg_Value,(0x1DC/0x4)+1,result);
    for (MUINT32 loop = 0; loop <= (0x1DC/0x4); loop ++)
    {
        MY_LOGI("Dump Reg[%3d] => Addr: 0x%03x, Value: 0x%08x\n", loop, FdReg_Addr[loop], FdReg_Value[loop]);
    }
#endif
    FDVT_IOCTL_StartHW();
    //MY_LOGD("getchar_START_HW_DONE()");
    //getchar();
    MY_LOGD("Enque End\n");
}

void FDVT_Deque(FdDrv_output_struct *FdDrv_output)
{
    MY_LOGD("Deque Start\n");
    if (!g_EnquedStatus)
    {
        MY_LOGE("Should not call FD Deque before calling FD Enque!\n");
    }
    else
    {
        g_EnquedStatus = MFALSE;
        MINT32 ret = 0;
        ret = FDVT_FDGetHWResult();

        if (ret != 0)
        {
            MY_LOGE("HW FD Fail, Dump FD Regs:\n");
            //MUINT32 reg_fdvt[FDVT_PARA_NUM]; //unused
            for (MUINT32 u4Index = 0; u4Index <= 0x1DC; u4Index += 0x4)
            {
                FdReg_Addr[u4Index/0x4] = u4Index;
            }
            MUINT32 result = 0;
            FDVT_IOCTL_GetHWResult(FdReg_Addr,FdReg_Value,(0x1DC/0x4)+1,result);
            for (MUINT32 loop = 0; loop <= (0x1DC/0x4); loop ++)
            {
                MY_LOGE("Dump Reg[%3d] => Addr: 0x%03x, Value: 0x%08x\n", loop, FdReg_Addr[loop], FdReg_Value[loop]);
            }
        }

        FDVT_SetFDResultToSW(FdDrv_output);
        MY_LOGD("Deque End\n");
    }
}
