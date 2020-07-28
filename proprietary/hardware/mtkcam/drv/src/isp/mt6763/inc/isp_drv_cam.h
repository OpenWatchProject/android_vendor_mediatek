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

#ifndef _ISP_DRV_CAM_H_
#define _ISP_DRV_CAM_H_
/**
    this module contains CAM_A, CAM_B in abstract method + UNI_A cq
    note:
        UNI_A phy is not included
*/
#include <isp_drv.h>
#include <imem_drv.h>
#include "uni_drv.h"

/**
    patch for hw design prob:
    under twin mode, dmx_id/dmx_sel have no DB, need to use master cam's timing to ctrl slave cam's CQ.
    (setting is only valid at  v-blanking )
*/
#define TWIN_CQ_SW_WORKAROUND   1

/**
    descriptor range & dummy reg for descriptor
*/
#define CQ_DES_RANGE    0xFFFF
#define CQ_DUMMY_REG    0x4ffc
///////////////////////////////////////////////////////////
/**
    CAM CQ descriptor
    its a abstract descriptor , cam_a & cam_b use the same enum
*/
typedef enum {
    //put slave cam's ctrl 1st, for pipe master cam & slave cam.
    TWIN_CQ_BASEADDRESS = 0,         // 1    5168   //must be allocated before salve cam's descriptor which is for salve cam's cq-trig
#if TWIN_CQ_SW_WORKAROUND
        CAM_CQ0_EN_,                 // 1    4164/5164   //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
        CAM_CQ0_TRIG_,               // 1    4000/5000
#endif
    CAM_CTL_EN_,             // 2    4004~4008
    CAM_CTL_FMT_,                // 1    400c
    CAM_CTL_SEL_,                // 1    4010
    CAM_CTL_INT_EN_,             // 1    4020
    CAM_CTL_INT2_EN_,            // 1    4030
    CAM_CTL_DONE_SEL_,           // 3    4044~404c
    CAM_TWIN_INFO,               // 2    4050
    CAM_CQ_COUNTER,              // 1    4058
    CAM_CTL_SW_DONE_SEL_,        // 1    405C
    CAM_FBC_IMGO_,               // 1    4110
    CAM_FBC_RRZO_,               // 1    4118
    CAM_FBC_UFEO_,               // 1    4120
    CAM_FBC_LCSO_,               // 1    4128
    CAM_FBC_AAO_,               // 1    4138
    CAM_FBC_PDO_,                   // 1    4140
    CAM_FBC_PSO_,                   // 1    4148
    CAM_CTL_DMA_V_FLIP_,         // 1    420C
    //DMA CON1 & CON2 & CON3 can't be wriitten by CQ
    CAM_DMA_IMGO_BA,             // 1    4220
    CAM_DMA_IMGO_BA_OFST,        // 1    4228
    CAM_DMA_IMGO_,               // 3    4230~4238
    CAM_DMA_IMGO_CROP_,          // 1    4248
    CAM_DMA_RRZO_BA,             // 1    4250
    CAM_DMA_RRZO_BA_OFST,        // 1    4258
    CAM_DMA_RRZO_,               // 3    4260~4268
    CAM_DMA_RRZO_CROP_,          // 1    4278
    CAM_DMA_AAO_BA,              // 1    4280
    CAM_DMA_AAO_,                // 3    4290~4298
    CAM_DMA_AFO_BA,              // 1    42b0
    CAM_DMA_AFO_BA_OFST,         // 1    42b8
    CAM_DMA_AFO_,                // 3    42C0~42C8
    CAM_DMA_LCSO_BA,             // 1    42E0
    CAM_DMA_LCSO_,               // 3    42F0~42F8
    CAM_DMA_UFEO_BA,             // 1    4310~4314
    CAM_DMA_UFEO_BA_OFST,        // 1    4318
    CAM_DMA_UFEO_,               // 3    4320~4328
    CAM_DMA_PDO_BA,              // 1    4340
    CAM_DMA_PDO_,                // 3    4350~4358
    CAM_DMA_PSO_BA,              // 2    4D80
    CAM_DMA_PSO_,                // 3    4D88~4D98
    CAM_DMA_BPCI_,               // 7    4370~4388
    CAM_DMA_CACI_,               // 7    43A0~43B8
    CAM_DMA_LSCI_,               // 7    43D0~43E8
    CAM_DMA_FH_EN_,              // 1    4C00
    CAM_DMA_FH_IMGO_,            // 1    4C04
    CAM_DMA_FH_IMGO_SPARE_,      // 15   4C30~4C68
    CAM_DMA_FH_RRZO_,            // 1    4C08
    CAM_DMA_FH_RRZO_SPARE_,      // 15   4C70~4CA8
    CAM_DMA_FH_AAO_,             // 1    4C0C
    CAM_DMA_FH_AAO_SPARE_MAGIC_, // 1    4cb0
    CAM_DMA_FH_AAO_SPARE_,       // 14   4CB4~4CE8
    CAM_DMA_FH_AFO_,             // 1    4C10
    CAM_DMA_FH_AFO_SPARE_MAGIC_, // 1    4cf0
    CAM_DMA_FH_AFO_SPARE_,       // 14   4CF4~4D28
    CAM_DMA_FH_LCSO_,            // 1    4C14
    CAM_DMA_FH_LCSO_SPARE_,      // 15   4D30~4D68
    CAM_DMA_FH_UFEO_,            // 1    4C18
    CAM_DMA_FH_UFEO_SPARE_,      // 15   4D70~4DA8
    CAM_DMA_FH_PDO_,             // 1    4C1C
    CAM_DMA_FH_PDO_SPARE_MAGIC_, // 1    4db0
    CAM_DMA_FH_PDO_SPARE_,       // 14   4DB4~4DE8
    CAM_DMA_FH_PSO_,             // 1    4E20
    CAM_DMA_FH_PSO_SPARE_MAGIC_, // 1    4DB0
    CAM_DMA_FH_PSO_SPARE_,       // 14   4DB4~4DE8
    CAM_CQ_THRE0_ADDR_,          // 1    4168
    CAM_CQ_THRE0_SIZE_,          // 1    416C
    CAM_CQ_THRE1_ADDR_,          // 1    4174
    CAM_CQ_THRE1_SIZE_,          // 1    4178
    CAM_CQ_THRE2_ADDR_,          // 1    4180
    CAM_CQ_THRE2_SIZE_,          // 1    4184
    CAM_CQ_THRE3_ADDR_,          // 1    418C
    CAM_CQ_THRE3_SIZE_,          // 1    4190
    CAM_CQ_THRE4_ADDR_,          // 1    4198
    CAM_CQ_THRE4_SIZE_,          // 1    419C
    CAM_CQ_THRE5_ADDR_,          // 1    41A4
    CAM_CQ_THRE5_SIZE_,          // 1    41A8
    CAM_CQ_THRE6_ADDR_,          // 1    41B0
    CAM_CQ_THRE6_SIZE_,          // 1    4184
    CAM_CQ_THRE7_ADDR_,          // 1    41BC
    CAM_CQ_THRE7_SIZE_,          // 1    41C0
    CAM_CQ_THRE8_ADDR_,          // 1    41C8
    CAM_CQ_THRE8_SIZE_,          // 1    41CC
    CAM_CQ_THRE9_ADDR_,          // 1    41D4
    CAM_CQ_THRE9_SIZE_,          // 1    41D8
    CAM_CQ_THRE10_ADDR_,         // 1    41E0
    CAM_CQ_THRE10_SIZE_,         // 1    41E4
    CAM_CQ_THRE11_ADDR_,         // 1    41EC
    CAM_CQ_THRE11_SIZE_,         // 1    41F0
    CAM_CQ_THRE12_ADDR_,         // 1    41F8
    CAM_CQ_THRE12_SIZE_,         // 1    41FC
    CAM_ISP_DMX_,                // 3    4580~4588
    CAM_ISP_PBN_,                // 3    4BB0~4BB8
    CAM_ISP_DBN_,                // 2    4BA0~4BA4
    CAM_ISP_BIN_,                // 3    4B80~4B88
    CAM_ISP_DBS_,                // 6    4B40~4B54
    CAM_ISP_OBC_,                // 8    45F0~460C
    CAM_ISP_RMG_,                // 2    45A0~45A4
    CAM_ISP_BNR_,                // 32   4620~469C
    CAM_ISP_STM_,                // 2    46A0~46A4
    CAM_ISP_SCM_,                // 2    46B0~46B4
    CAM_ISP_RMM_,                // 9    45C0~45E0
    CAM_ISP_CAC_,                // 2    4AF0~4AF4
    CAM_ISP_BMX_,                // 3    4780~4788
    CAM_ISP_LSC_,                // 8    46A0~46BC
    CAM_ISP_RCP_,                // 2    48F0~48F4
    CAM_ISP_RPG_,                // 6    46C0~46D4
    CAM_ISP_RRZ_,                // 11   46E0~4708
    CAM_ISP_RMX_,                // 3    4740~4748
    CAM_ISP_SGG5_,               // 3    4760~4768
    CAM_ISP_UFE_,                // 1    47C0
    CAM_ISP_PMX_,                // 3    4B00~4B08
    CAM_ISP_RCP3_,               // 2    4BC0~4BC4
    CAM_ISP_SGM_,                // 12   4BD0~4BFC
    CAM_ISP_CPG_,                // 6    4AD0~4AE4
    CAM_ISP_QBN1_,               // 1    4AC0
    CAM_ISP_QBN4_,               // 1    4D00
    CAM_ISP_PS_AWB_,             // 8    4D10~4D2C
    CAM_ISP_PS_AE_,              // 2    4D30~4D34
    CAM_ISP_SL2F_,
    CAM_ISP_PCP_CROP_,           // 2    4CC0~4CC4
    CAM_ISP_SGG2_,               // 3    4CD0~4CD8
    CAM_ISP_SL2J_,               //12   4C80~4CAC
    CAM_ISP_HLR_,                //12   4850~487C
    CAM_ISP_LCS_,                // 6    47E0~47F4
    CAM_ISP_AE_,                 // 27   49E0~4A48
    CAM_ISP_VBN_,
    CAM_ISP_AMX_,
    CAM_ISP_AWB_,                // 37   4920~49B0
    CAM_ISP_SGG1_,               // 3    4900~4908
    CAM_ISP_AF_,                 // 16   4800~483C

    //UNIT
    UNI_CTL_EN_,                 // 2    3010~3014
    UNI_CTL_SEL_,                // 1    3018
    UNI_CTL_FMT_,                // 1    301C
    UNI_FBC_FLKO_,               // 1    3080
    UNI_FBC_EISO_,               // 1    3088
    UNI_FBC_RSSO_,               // 1    3090
    UNI_DMA_V_FLIP_,             // 1    3204
    UNI_DMA_EISO_BA,             // 2    3220~3224
    UNI_DMA_EISO_,               // 5    3228~3238
    UNI_DMA_FLKO_BA,             // 2    3250~3254
    UNI_DMA_FLKO_,               // 5    3258~3268
    UNI_DMA_RSSO_A_BA,           // 2    3280~3284
    UNI_DMA_RSSO_A_,             // 5    3288~3298
    UNI_DMA_RSSO_B_BA,           // 2    32b0~32b4
    UNI_DMA_RSSO_B_,             // 5    32B8~32C8
    UNI_DMA_RAWI_,               // 7    3340~3358
    UNI_DMA_FH_EN_,              // 1    33C0
    UNI_DMA_FH_EISO_,            // 1    33C8
    UNI_DMA_FH_EISO_SPARE_,      // 15   33E0~3418
    UNI_DMA_FH_FLKO_,            // 1    33C8
    UNI_DMA_FH_FLKO_SPARE_MAGIC_,// 1    3420
    UNI_DMA_FH_FLKO_SPARE_,      // 14   3424~3458
    UNI_DMA_FH_RSSO_A_,          // 1    33CC
    UNI_DMA_FH_RSSO_A_SPARE_,    // 15   3460~3498
    UNI_DMA_FH_RSSO_B_,          // 1    33D0
    UNI_DMA_FH_RSSO_B_SPARE_,    // 15   34A0~34D8
    UNI_ISP_UNP2_,               // 1    3500
    UNI_ISP_SGG3_,               // 3    3520~3528
    UNI_ISP_FLK_,                // 4    3530~353C
    UNI_ISP_HDS_A_,              // 1    35A0
    UNI_ISP_EIS_,                // 9    3550~3570
    UNI_ISP_RSS_A_,              // 9    35C0~35E0
    UNI_ISP_HDS_B_,              // 1    3610
    UNI_ISP_RSS_B_,              // 9    3630~3650
    CAM_DUMMY_,                  // 1    4ffc
    CAM_CTL_FBC_RCNT_INC_,       // 1    406C   //must be allocated after all FBC_en descirptor
    CAM_CQ_EN_,                  // 1    4160
    CAM_NEXT_Thread_,            // 1    must at the end of descriptor  , or apb will be crashed
    CAM_CAM_END_,                                    //must be kept at the end of descriptor
    CAM_CAM_MODULE_MAX
}E_CAM_MODULE;

#define RingBuf_MODULE_MAX   (4+1)  // +1 for cq inner baseaddr when high-speed mode

//special deifne
//write 2 cq inner address directly!!!!!
#define CAM_CQ_THRE0_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C168           // for CQ access only
#define CAM_CQ_THRE1_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C174           // for CQ access only
#define CAM_CQ_THRE2_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C180           // for CQ access only
#define CAM_CQ_THRE3_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C18C           // for CQ access only
#define CAM_CQ_THRE4_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C198           // for CQ access only
#define CAM_CQ_THRE5_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C1A4           // for CQ access only
#define CAM_CQ_THRE6_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C1B0           // for CQ access only
#define CAM_CQ_THRE7_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C1BC           // for CQ access only
#define CAM_CQ_THRE8_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C1C8           // for CQ access only
#define CAM_CQ_THRE9_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    C1D4           // for CQ access only
#define CAM_CQ_THRE10_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    C1E0           // for CQ access only
#define CAM_CQ_THRE11_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    C1EC           // for CQ access only
#define CAM_CQ_THRE12_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    C1F8           // for CQ access only
///////////////////////////////////////////////////////////////////////////


/**
    RW marco, for coding convenience

    note1: cam + uni module only (uni is included because of cq programming range.

            if __offset over 0x1000, its a special design for CQ  baseaddr.
            target reg address is at current offset - 0x1000 + 0x8000. (0x1000 is for roll back, 0x8000 is mapping to set&clr domain.
    note2: caller here don't care, CAM_A or CAM_B r all acceptable!
*/


//user no need to write reg address, drv will fill the address automatically
#define CAM_BURST_READ_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    ret=IspDrvPtr->readRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAM_A);\
    ret;\
})

//user no need to write reg address, drv will fill the address automatically
#define CAM_BURST_WRITE_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL __ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    __ret=IspDrvPtr->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAM_A);\
    __ret;\
})

#define CAM_READ_REG(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset,CAM_A);\
})

#define CAM_READ_BITS(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,CAM_A);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define CAM_WRITE_REG(IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->writeReg(__offset, Value,CAM_A);\
}while(0);

#define CAM_WRITE_BITS(IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,CAM_A);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg(__offset, ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw,CAM_A);\
}while(0);

#define CAM_REG_ADDR(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    __offset;\
})

///////////////////////////////////////////////////////////
/**
    CAM hw CQ
*/
typedef enum
{
    ISP_DRV_CQ_THRE0 = 0,
    ISP_DRV_CQ_THRE1,
    ISP_DRV_CQ_THRE2,
    ISP_DRV_CQ_THRE3,
    ISP_DRV_CQ_THRE4,
    ISP_DRV_CQ_THRE5,
    ISP_DRV_CQ_THRE6,
    ISP_DRV_CQ_THRE7,
    ISP_DRV_CQ_THRE8,
    ISP_DRV_CQ_THRE9,
    ISP_DRV_CQ_THRE10,
    ISP_DRV_CQ_THRE11,
    ISP_DRV_CQ_THRE12,
    ISP_DRV_CAM_BASIC_CQ_NUM,    //baisc set, pass1 cqs
    ISP_DRV_CQ_NONE = ISP_DRV_CAM_BASIC_CQ_NUM
}E_ISP_CAM_CQ;

/**
    CQ trig mode
*/
typedef enum
{
    CQ_SINGLE_IMMEDIATE_TRIGGER = 0,
    CQ_SINGLE_EVENT_TRIGGER,
    CQ_CONTINUOUS_EVENT_TRIGGER,
    CQ_TRIGGER_MODE_NUM
}E_ISP_CAM_CQ_TRIGGER_MODE;


/**
    CQ trig source
*/
typedef enum
{
    CQ_TRIG_BY_START = 0,
    CQ_TRIG_BY_PASS1_DONE,
    CQ_TRIG_BY_IMGO_DONE,
    CQ_TRIG_BY_RRZO_DONE,
    CQ_TRIG_SRC_NUM,
    CQ_TRIG_BY_NONE = CQ_TRIG_SRC_NUM
}E_ISP_CAM_CQ_TRIGGER_SOURCE;

///////////////////////////////////////////////////////////
/**
    CAM bufctrl CmdQ special descriptor, and ob special descriptor
*/
typedef enum{
    BUF_CTRL_ENQUE  = 0,
    BUF_CTRL_ENQUE_HIGH_SPEED = 1,
    BUF_CTRL_CLEAR,
    SET_FH_SPARE,
    GET_RING_DEPTH,
}E_BUF_CTRL;

typedef struct{
    E_BUF_CTRL      ctrl;
    MUINT32         dma_PA;
    MUINT32         dma_FH_PA;
    MUINT32*        pDma_fh_spare;
}CQ_RingBuf_ST;

typedef struct _cq_cmd_st_CAM_
{
    unsigned int descriptor;
    unsigned int data;
}CQ_CMD_ST_CAM;

typedef struct _cq_info_rtbc_st_CAM_
{
    CQ_CMD_ST_CAM   dma;
    CQ_CMD_ST_CAM   dma_fh;
    CQ_CMD_ST_CAM   fh_spare;
    CQ_CMD_ST_CAM   next_Thread_addr;
    CQ_CMD_ST_CAM   next_Thread_addr_inner;
    CQ_CMD_ST_CAM   end;
    MUINT32         dma_base_pa;
    MUINT32         dma_fh_base_pa;
    MUINT32         dma_fh_spare[CAM_IMAGE_HEADER];
}CQ_INFO_RTBC_ST_CAM;

typedef struct _cq_ring_cmd_st_CAM_
{
    CQ_INFO_RTBC_ST_CAM cq_rtbc;
    unsigned long next_pa;
    struct _cq_ring_cmd_st_CAM_ *pNext;
}CQ_RING_CMD_ST_CAM;

typedef struct _cq_rtbc_ring_st_CAM_
{
    CQ_RING_CMD_ST_CAM rtbc_ring[MAX_RING_SIZE];   //ring depth: sw maximum: 16 [hw maximus is depended on hw bits]
    unsigned int   dma_ring_size;
}CQ_RTBC_RING_ST_CAM;

//CQ0B for AE smoothing, set obc_gain0~3
typedef struct _cq0b_info_rtbc_st_CAM_
{
    CQ_CMD_ST_CAM ob;
    CQ_CMD_ST_CAM end;
}CQ0B_INFO_RTBC_ST_CAM;

typedef struct _cq0b_ring_cmd_st_CAM_
{
    CQ0B_INFO_RTBC_ST_CAM cq0b_rtbc;
    unsigned long next_pa;
    struct _cq0b_ring_cmd_st_CAM_ *pNext;
}CQ0B_RING_CMD_ST_CAM;

typedef struct _cq0b_rtbc_ring_st_CAM_
{
    CQ0B_RING_CMD_ST_CAM rtbc_ring;
}CQ0B_RTBC_RING_ST_CAM;

//////////////////////////////////////////////////////////////////
/**
    vir cq definition
*/
#define CmdQ_Cache  0   //CmdQ using by Cache is 1, Non-cache is 0
#define Max_PageNum 64  //e.g. nBurst * nDup <= Max_PageNum
/**
    define the order of vir_reg.
    if 1
        cam 1st , uni 2nd
    else if 0
        uni 1st . cam 2nd
*/
#define Vir_Buf_Order   1
/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
//control range: CAM + UNI_VIR only
class ISP_DRV_CAM : public IspDrvVir, public IspDrv
{
    public:
        ~ISP_DRV_CAM(){};
        ISP_DRV_CAM(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx);
    public:

        static  IspDrvVir*  createInstance(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx,const char* userName);

        virtual void    destroyInstance(void);


        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   start(void);
        virtual MBOOL   stop(void);

        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq);

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);

        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);

        virtual MBOOL   cqAddModule(MUINT32 moduleId);
        virtual MBOOL   cqAddModule(MUINT32 moduleId, MUINT32 extModuleId);
        virtual MBOOL   cqDelModule(MUINT32 moduleId);
        virtual MBOOL   cqNopModule(MUINT32 moduleId);
        virtual MBOOL   cqApbModule(MUINT32 moduleId);
        virtual MBOOL   dumpCQTable(void);


        virtual MUINT32*    getCQDescBufPhyAddr(void);
        virtual MUINT32*    getCQDescBufVirAddr(void);
        virtual MUINT32*    getIspVirRegPhyAddr(void);
        virtual MUINT32*    getIspVirRegVirAddr(void);

        virtual MBOOL       getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize);
        virtual MBOOL       flushCmdQ(void);

        IspDrv*             getPhyObj(void);

        MBOOL               getCurObjInfo(ISP_HW_MODULE* p_module,E_ISP_CAM_CQ* p_cq,MUINT32* p_page);
        MBOOL               DumpReg(MBOOL bPhy);

        //for CQ those descriptor is static, not programable
        MUINT32             cqRingBuf(CQ_RingBuf_ST *pBuf_ctrl);

        //set cq in fixed format in which each reg have it's own descriptor with data length:1 only.  e.g.:CQ11...
        MBOOL               CQ_SetContent(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count);

        //read cq registers of each CqModule
        MBOOL               readCQModuleInfo(MUINT32 moduleId, void **ptr, MUINT32 size);

    private:
                MBOOL   FSM_CHK(MUINT32 op,const char* caller);
                //
                MBOOL   CQ_Allocate_method1(MUINT32 step);
                MBOOL   CQ_Allocate_method2(MUINT32 step);
                //
                MBOOL   updateEnqCqRingBuf(CQ_RingBuf_ST *pBuf_ctrl);
                MUINT32*    update_FH_Spare(CQ_RingBuf_ST *pBuf_ctrl);

                MUINT32 VirReg_OFFSET(MUINT32 hwModule);
                MBOOL   VirReg_ValidRW(void);
                MBOOL   twinPath_CQ_Ctrl(MUINT32 moduleId, MUINT32 *offset); //this is for master cam to control salve cam under twin mode
    public:

    private:
        volatile MINT32         m_UserCnt;
        mutable android::Mutex           IspRegMutex;
        //
        MUINT32*        m_pIspVirRegAddr_va;
        MUINT32*        m_pIspVirRegAddr_pa;

        //
        ISP_DRV_CQ_CMD_DESC_STRUCT*         m_pIspDescript_vir;
        MUINT32*                            m_pIspDescript_phy;

        //
        MUINT32         m_FSM;
        //imem
        IMemDrv*        m_pMemDrv;
        IMEM_BUF_INFO   m_ispVirRegBufInfo;
        IMEM_BUF_INFO   m_ispCQDescBufInfo;

        E_ISP_CAM_CQ    m_CQ;
        ISP_HW_MODULE   m_HWModule;
        ISP_HW_MODULE   m_Module;           //phy + virtual hw module
        MUINT32         m_pageIdx;
        char            m_useName[32];

        static MBOOL    m_bInit[CAM_MAX];   //flag to avoid re-init phy CAM obj
        IspDrv*         m_pIspDrvImp;       //ptr for access CAM_A/CAM_B ...etc. phy
};





#endif //_ISP_DRV_CAM_H_
