/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <cutils/properties.h>

/* use nvram */
#include "CFG_BT_File.h"
#include "CFG_BT_Default.h"
#include "CFG_file_lid.h"
#include "libnvram.h"

#include "bt_hci_bdroid.h"
#include "bt_vendor_lib.h"
#include "bt_mtk.h"


/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

BT_INIT_VAR_T btinit[1];
BT_INIT_CB_T  btinit_ctrl;

static uint16_t wOpCode;
static HCI_CMD_STATUS_T cmd_status;
extern bt_vendor_callbacks_t *bt_vnd_cbacks;

static BOOL fgGetEFUSE = FALSE;
static UCHAR ucDefaultAddr[6];
static UCHAR ucZeroAddr[6];

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

static BOOL GORMcmd_HCC_Get_Local_BD_Addr(HC_BT_HDR *);
static VOID GORMevt_HCE_Get_Local_BD_Addr(VOID *);
static BOOL GORMcmd_HCC_Set_Local_BD_Addr(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_LinkKeyType(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_UnitKey(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_Encryption(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_PinCodeType(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_Voice(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_PCM(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_Radio(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_TX_Power_Offset(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_Sleep_Timeout(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_PIP(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Coex_Performance_Adjust(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_BT_FTR(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_OSC_Info(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_LPO_Info(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_PTA(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_BLEPTA(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_Internal_PTA_1(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_Internal_PTA_2(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_RF_Reg_100(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Coex_Bt_Ctrl(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Coex_Opp_Time_Ratio(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Coex_Lescan_Time_Ratio(HC_BT_HDR *);
static BOOL GORMcmd_HCC_RESET(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_SSP_Debug_Mode(HC_BT_HDR *);

static VOID GORMevt_HCE_Common_Complete(VOID *);
static VOID GetRandomValue(UCHAR *);
static BOOL WriteBDAddrToNvram(UCHAR *);

//===================================================================
// Combo chip
HCI_SEQ_T bt_init_preload_script_6628[] =
{
    {  GORMcmd_HCC_Get_Local_BD_Addr       }, /*0x1009*/
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_LinkKeyType         }, /*0xFC1B*/
    {  GORMcmd_HCC_Set_UnitKey             }, /*0xFC75*/
    {  GORMcmd_HCC_Set_Encryption          }, /*0xFC76*/
    {  GORMcmd_HCC_Set_PinCodeType         }, /*0x0C0A*/
    {  GORMcmd_HCC_Set_Voice               }, /*0x0C26*/
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_Set_BT_FTR              }, /*0xFC7D*/
    {  GORMcmd_HCC_Set_OSC_Info            }, /*0xFC7B*/
    {  GORMcmd_HCC_Set_LPO_Info            }, /*0xFC7C*/
    {  GORMcmd_HCC_Set_PTA                 }, /*0xFC74*/
    {  GORMcmd_HCC_Set_BLEPTA              }, /*0xFCFC*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  GORMcmd_HCC_Set_Internal_PTA_1      }, /*0xFCFB*/
    {  GORMcmd_HCC_Set_Internal_PTA_2      }, /*0xFCFB*/
    {  GORMcmd_HCC_Set_RF_Reg_100          }, /*0xFCB0*/
    {  0  },
};

HCI_SEQ_T bt_init_preload_script_6630[] =
{
    {  GORMcmd_HCC_Get_Local_BD_Addr       }, /*0x1009*/
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_Coex_Performance_Adjust }, /*0xFC22*/
    {  GORMcmd_HCC_Set_SSP_Debug_Mode      }, /*0x1804*/
    {  0  },
};

HCI_SEQ_T bt_init_preload_script_6632[] =
{
    {  GORMcmd_HCC_Get_Local_BD_Addr       }, /*0x1009*/
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  GORMcmd_HCC_Set_SSP_Debug_Mode      }, /*0x1804*/
    {  0  },
};

HCI_SEQ_T bt_init_preload_script_consys[] =
{
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  GORMcmd_HCC_Set_SSP_Debug_Mode      }, /*0x1804*/
    {  0  },
};

HCI_SEQ_T bt_init_preload_script_connac[] =
{
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  GORMcmd_HCC_Set_PIP                 }, /*0xFCC5*/
    {  GORMcmd_HCC_Set_SSP_Debug_Mode      }, /*0x1804*/
    {  0  },
};

HCI_SEQ_T bt_init_preload_script_connac20[] =
{
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  GORMcmd_HCC_Set_PIP                 }, /*0xFCC5*/
    {  GORMcmd_HCC_Set_SSP_Debug_Mode      }, /*0x1804*/
    /* co-exist cmd are set by init script after connac20 */
    {  GORMcmd_HCC_Coex_Bt_Ctrl            }, /*0xFCFB*/
    {  GORMcmd_HCC_Coex_Opp_Time_Ratio     }, /*0xFCFB*/
    {  GORMcmd_HCC_Coex_Lescan_Time_Ratio  }, /*0xFCFB*/
    {  0  },
};

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

static BOOL GORMcmd_HCC_Get_Local_BD_Addr(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x1009;

    p_cmd->len = 3;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 0;

    LOG_DBG("GORMcmd_HCC_Get_Local_BD_Addr\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Get_Local_BD_Addr);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static VOID GORMevt_HCE_Get_Local_BD_Addr(VOID *p_evt)
{
    HC_BT_HDR *p_buf = (HC_BT_HDR *)p_evt;
    uint8_t *p;
    uint8_t event, status;
    uint16_t opcode;
    BOOL success;

    UCHAR ucBDAddr[6];

    LOG_DBG("GORMevt_HCE_Get_Local_BD_Addr\n");

    p = (uint8_t *)(p_buf + 1);
    event = *p;
    p += 3;
    STREAM_TO_UINT16(opcode, p)
    status = *p++;

    if ((event == 0x0E) && /* Command Complete Event */
        (opcode == 0x1009) && /* OpCode correct */
        (status == 0)) /* Success */
    {
        success = TRUE;
    }
    else {
        success = FALSE;
    }

    ucBDAddr[5] = *p++;
    ucBDAddr[4] = *p++;
    ucBDAddr[3] = *p++;
    ucBDAddr[2] = *p++;
    ucBDAddr[1] = *p++;
    ucBDAddr[0] = *p++;

    LOG_WAN("Retrieve eFUSE address: %02x-%02x-%02x-%02x-%02x-%02x\n",
            ucBDAddr[0], ucBDAddr[1], ucBDAddr[2], ucBDAddr[3], ucBDAddr[4], ucBDAddr[5]);

    memcpy(btinit->bt_nvram.fields.addr, ucBDAddr, 6);

    if (bt_vnd_cbacks) {
        bt_vnd_cbacks->dealloc(p_buf);
    }

    pthread_mutex_lock(&btinit_ctrl.mutex);
    cmd_status = success ? CMD_SUCCESS : CMD_FAIL;
    /* Wake up command tx thread */
    pthread_cond_signal(&btinit_ctrl.cond);
    pthread_mutex_unlock(&btinit_ctrl.mutex);

    return;
}

static BOOL GORMcmd_HCC_Set_Local_BD_Addr(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC1A;

    if ((0 == memcmp(btinit->bt_nvram.fields.addr, ucDefaultAddr, 6)) ||
        (0 == memcmp(btinit->bt_nvram.fields.addr, ucZeroAddr, 6))) {
        if (fgGetEFUSE) {
            LOG_WAN("eFUSE address default value\n");
        }
        else {
            LOG_WAN("NVRAM BD address default value\n");
        }

        #ifdef BD_ADDR_AUTOGEN
        GetRandomValue(btinit->bt_nvram.fields.addr);
        #endif

        /* Save BD address to NVRAM */
        WriteBDAddrToNvram(btinit->bt_nvram.fields.addr);
    }
    else {
        if (fgGetEFUSE) {
            LOG_WAN("eFUSE address has valid value\n");
            /* Save BD address to NVRAM */
            WriteBDAddrToNvram(btinit->bt_nvram.fields.addr);
         }
         else {
            LOG_WAN("NVRAM BD address has valid value\n");
         }
    }

    fgGetEFUSE = FALSE; /* Clear flag */

    p_cmd->len = 9;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 6;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.addr[5];
    *p++ = btinit->bt_nvram.fields.addr[4];
    *p++ = btinit->bt_nvram.fields.addr[3];
    *p++ = btinit->bt_nvram.fields.addr[2];
    *p++ = btinit->bt_nvram.fields.addr[1];
    *p++ = btinit->bt_nvram.fields.addr[0];

    LOG_DBG("GORMcmd_HCC_Set_Local_BD_Addr\n");

    LOG_WAN("Write BD address: %02x-%02x-%02x-%02x-%02x-%02x\n",
            btinit->bt_nvram.fields.addr[0], btinit->bt_nvram.fields.addr[1],
            btinit->bt_nvram.fields.addr[2], btinit->bt_nvram.fields.addr[3],
            btinit->bt_nvram.fields.addr[4], btinit->bt_nvram.fields.addr[5]);


    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}


static BOOL GORMcmd_HCC_Set_LinkKeyType(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC1B;

    p_cmd->len = 4;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 1;

    /* HCI cmd params */
    *p++ = 0x01; /* 00: Unit key; 01: Combination key */

    LOG_DBG("GORMcmd_HCC_Set_LinkKeyType\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_UnitKey(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC75;

    p_cmd->len = 19;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 16;

    /* HCI cmd params */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    LOG_DBG("GORMcmd_HCC_Set_UnitKey\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Encryption(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC76;

    p_cmd->len = 6;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 3;

    /* HCI cmd params */
    *p++ = 0x00;
    *p++ = 0x02;
    *p++ = 0x10;

    LOG_DBG("GORMcmd_HCC_Set_Encryption\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_PinCodeType(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x0C0A;

    p_cmd->len = 4;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 1;

    /* HCI cmd params */
    *p++ = 0x00; /* 00: Variable PIN; 01: Fixed PIN */

    LOG_DBG("GORMcmd_HCC_Set_PinCodeType\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Voice(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x0C26;

    p_cmd->len = 5;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 2;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.Voice[0];
    *p++ = btinit->bt_nvram.fields.Voice[1];

    LOG_DBG("GORMcmd_HCC_Set_Voice\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_PCM(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC72;

    p_cmd->len = 7;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 4;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.Codec[0];
    *p++ = btinit->bt_nvram.fields.Codec[1];
    *p++ = btinit->bt_nvram.fields.Codec[2];
    *p++ = btinit->bt_nvram.fields.Codec[3];

    LOG_DBG("GORMcmd_HCC_Set_PCM\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Radio(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC79;

    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);

    if (btinit->chip_id == 0x6632 || btinit->chip_type == BT_CONNAC) {
        p_cmd->len = 11;
        *p++ = 8;

        /* HCI cmd params */
        *p++ = btinit->bt_nvram.fields.Radio[0];
        *p++ = btinit->bt_nvram.fields.Radio[1];
        *p++ = btinit->bt_nvram.fields.Radio[2];
        *p++ = btinit->bt_nvram.fields.Radio[3];
        *p++ = btinit->bt_nvram.fields.Radio[4];
        *p++ = btinit->bt_nvram.fields.Radio[5];
        *p++ = btinit->bt_nvram.fields.RadioExt[0];
        *p++ = btinit->bt_nvram.fields.RadioExt[1];
    } else {
        p_cmd->len = 9;
        *p++ = 6;

        /* HCI cmd params */
        *p++ = btinit->bt_nvram.fields.Radio[0];
        *p++ = btinit->bt_nvram.fields.Radio[1];
        *p++ = btinit->bt_nvram.fields.Radio[2];
        *p++ = btinit->bt_nvram.fields.Radio[3];
        *p++ = btinit->bt_nvram.fields.Radio[4];
        *p++ = btinit->bt_nvram.fields.Radio[5];
    }

    LOG_DBG("GORMcmd_HCC_Set_Radio\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_TX_Power_Offset(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC93;

    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);

    if (btinit->chip_id == 0x6632) {
        p_cmd->len = 9;
        *p++ = 6;

        /* HCI cmd params */
        *p++ = btinit->bt_nvram.fields.TxPWOffset[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[2];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt1[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt1[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt1[2];
    } else if (btinit->chip_type == BT_CONNAC) {
        p_cmd->len = 19;
        *p++ = 16;

        /* HCI cmd params*/
        *p++ = btinit->bt_nvram.fields.TxPWOffset[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[2];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt1[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt1[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt1[2];

        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[2];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[3];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[4];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[5];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[6];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[7];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[8];
        *p++ = btinit->bt_nvram.fields.TxPWOffsetExt2[9];
    }
    else{
        p_cmd->len = 6;
        *p++ = 3;

        /* HCI cmd params */
        *p++ = btinit->bt_nvram.fields.TxPWOffset[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[2];
    }

    LOG_DBG("GORMcmd_HCC_Set_TX_Power_Offset, cmd len %d\n", p_cmd->len);

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Sleep_Timeout(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC7A;

    p_cmd->len = 10;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 7;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.Sleep[0];
    *p++ = btinit->bt_nvram.fields.Sleep[1];
    *p++ = btinit->bt_nvram.fields.Sleep[2];
    *p++ = btinit->bt_nvram.fields.Sleep[3];
    *p++ = btinit->bt_nvram.fields.Sleep[4];
    *p++ = btinit->bt_nvram.fields.Sleep[5];
    *p++ = btinit->bt_nvram.fields.Sleep[6];

    LOG_DBG("GORMcmd_HCC_Set_Sleep_Timeout\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_PIP(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCC5;

    p_cmd->len = 5;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 2;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.PIP[0];
    *p++ = btinit->bt_nvram.fields.PIP[1];

    LOG_DBG("GORMcmd_HCC_Set_PIP\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }
    return ret;
}

static BOOL GORMcmd_HCC_Coex_Performance_Adjust(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC22;

    p_cmd->len = 9;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 6;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.CoexAdjust[0];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[1];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[2];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[3];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[4];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[5];

    LOG_DBG("GORMcmd_HCC_Coex_Performance_Adjust\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}


static BOOL GORMcmd_HCC_Set_BT_FTR(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC7D;

    p_cmd->len = 5;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 2;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.BtFTR[0];
    *p++ = btinit->bt_nvram.fields.BtFTR[1];

    LOG_DBG("GORMcmd_HCC_Set_BT_FTR\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_OSC_Info(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC7B;

    p_cmd->len = 8;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 5;

    /* HCI cmd params */
    *p++ = 0x01;
    *p++ = 0x01;
    *p++ = 0x14; /* clock drift */
    *p++ = 0x0A; /* clock jitter */
    *p++ = 0x08; /* OSC stable time */

    LOG_DBG("GORMcmd_HCC_Set_OSC_Info\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_LPO_Info(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC7C;

    p_cmd->len = 13;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 10;

    /* HCI cmd params */
    *p++ = 0x01; /* LPO source = external */
    *p++ = 0xFA; /* LPO clock drift = 250ppm */
    *p++ = 0x0A; /* LPO clock jitter = 10us */
    *p++ = 0x02; /* LPO calibration mode = manual mode */
    *p++ = 0x00; /* LPO calibration interval = 10 mins */
    *p++ = 0xA6;
    *p++ = 0x0E;
    *p++ = 0x00;
    *p++ = 0x40; /* LPO calibration cycles = 64 */
    *p++ = 0x00;

    LOG_DBG("GORMcmd_HCC_Set_LPO_Info\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_PTA(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC74;

    p_cmd->len = 13;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 10;

    /* HCI cmd params */
    *p++ = 0xC9; /* PTA mode register */
    *p++ = 0x8B;
    *p++ = 0xBF;
    *p++ = 0x00;
    *p++ = 0x00; /* PTA time register */
    *p++ = 0x52;
    *p++ = 0x0E;
    *p++ = 0x0E;
    *p++ = 0x1F; /* PTA priority setting */
    *p++ = 0x1B;

    LOG_DBG("GORMcmd_HCC_Set_PTA\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_BLEPTA(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCFC;

    p_cmd->len = 8;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 5;

    /* HCI cmd params */
    *p++ = 0x16; /* Select BLE PTA command */
    *p++ = 0x0E; /* BLE PTA time setting */
    *p++ = 0x0E;
    *p++ = 0x00; /* BLE PTA priority setting */
    *p++ = 0x07;

    LOG_DBG("GORMcmd_HCC_Set_BLEPTA\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Internal_PTA_1(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCFB;

    p_cmd->len = 18;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 15;

    /* HCI cmd params */
    *p++ = 0x00;
    *p++ = 0x01; /* PTA high level Tx */
    *p++ = 0x0F; /* PTA mid level Tx */
    *p++ = 0x0F; /* PTA low level Tx */
    *p++ = 0x01; /* PTA high level Rx */
    *p++ = 0x0F; /* PTA mid level Rx */
    *p++ = 0x0F; /* PTA low level Rx */
    *p++ = 0x01; /* BLE PTA high level Tx */
    *p++ = 0x0F; /* BLE PTA mid level Tx */
    *p++ = 0x0F; /* BLE PTA low level Tx */
    *p++ = 0x01; /* BLE PTA high level Rx */
    *p++ = 0x0F; /* BLE PTA mid level Rx */
    *p++ = 0x0F; /* BLE PTA low level Rx */
    *p++ = 0x02; /* time_r2g */
    *p++ = 0x01;

    LOG_DBG("GORMcmd_HCC_Set_Internal_PTA_1\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Internal_PTA_2(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCFB;

    p_cmd->len = 10;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 7;

    /* HCI cmd params */
    *p++ = 0x01;
    *p++ = 0x19; /* wifi20_hb */
    *p++ = 0x19; /* wifi40_hb */
    *p++ = 0x07; /* next RSSI update BT slots */
    *p++ = 0xD0;
    *p++ = 0x00; /* stream identify by host */
    *p++ = 0x01; /* enable auto AFH */

    LOG_DBG("GORMcmd_HCC_Set_Internal_PTA_2\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_RF_Reg_100(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCB0;

    p_cmd->len = 9;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 6;

    /* HCI cmd params */
    *p++ = 0x64;
    *p++ = 0x01;
    *p++ = 0x02;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    LOG_DBG("GORMcmd_HCC_Set_RF_Reg_100\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Coex_Bt_Ctrl(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCFB;

    p_cmd->len = 7;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 4;

    /* HCI cmd params */
    *p++ = 0x1E;
    *p++ = btinit->fw_conf_para.coex_bt_ctrl;
    *p++ = btinit->fw_conf_para.coex_bt_ctrl_mode;
    *p++ = btinit->fw_conf_para.coex_bt_ctrl_rw;

    LOG_DBG("GORMcmd_HCC_Coex_Bt_Ctrl\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Coex_Opp_Time_Ratio(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCFB;

    p_cmd->len = 7;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 4;

    /* HCI cmd params */
    *p++ = 0x1F;
    *p++ = btinit->fw_conf_para.coex_opp_time_ratio;
    *p++ = btinit->fw_conf_para.coex_opp_time_ratio_bt_slot;
    *p++ = btinit->fw_conf_para.coex_opp_time_ratio_wifi_slot;

    LOG_DBG("GORMcmd_HCC_Coex_Opp_Time_Ratio\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Coex_Lescan_Time_Ratio(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCFB;

    p_cmd->len = 7;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 4;

    /* HCI cmd params */
    *p++ = 0x20;
    *p++ = btinit->fw_conf_para.coex_lescan_time_ratio;
    *p++ = btinit->fw_conf_para.coex_lescan_time_ratio_bt_slot;
    *p++ = btinit->fw_conf_para.coex_lescan_time_ratio_wifi_slot;

    LOG_DBG("GORMcmd_HCC_Coex_Lescan_Time_Ratio\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_RESET(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x0C03;

    p_cmd->len = 3;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 0;

    LOG_DBG("GORMcmd_HCC_RESET\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

/*
 * Simple Pairing Debug Mode:
 *
 * \brief
 *  Send HCI_Write_Simple_Pairing_Debug_Mode with 'enabled' parameter if Bluetooth SSP Debug Mode
 *  is set enabled in Engineer Mode UI for debugging purpose.
 *  This command configures the BR/EDR Controller to use a predefined Diffie Hellman private key
 *  for Simple Pairing, in order for debug equipment to be able to determine the link key
 *  and therefore, be able to monitor the encrypted connection.
 *  
 *  Command parameters:
 *  - 0x00: Simple Pairing debug mode disabled (default)
 *  - 0x01: Simple Pairing debug mode enabled
 *
 *  Event generated:
 *  - When HCI_Write_Simple_Pairing_Debug_Mode command has completed, a Command Complete event
 *    shall be generated.
 */
static BOOL GORMcmd_HCC_Set_SSP_Debug_Mode(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x1804;

    p_cmd->len = 4;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 1;

    /* HCI cmd params */
    *p++ = 0x01;
    LOG_DBG("GORMcmd_HCC_Set_SSP_Debug_Mode enable\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    } else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Fw_Cfg_Cmd(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret, i;
    wOpCode = (btinit->fw_conf_cmd[1] << 8) + btinit->fw_conf_cmd[0];

    p_cmd->len = 3 + btinit->fw_conf_cmd[2];
    p = (uint8_t *)(p_cmd + 1);
    *p++ = btinit->fw_conf_cmd[0]; // opcode
    *p++ = btinit->fw_conf_cmd[1]; // opcode
    *p++ = btinit->fw_conf_cmd[2]; // parameter length
    for(i = 3; i < (btinit->fw_conf_cmd[2] + 3); i++) {
        *p++ = btinit->fw_conf_cmd[i]; // parameter
    }

    LOG_DBG("GORMcmd_HCC_Fw_Cfg_Cmd\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static VOID GORMevt_HCE_Common_Complete(VOID *p_evt)
{
    HC_BT_HDR *p_buf = (HC_BT_HDR *)p_evt;
    uint8_t *p;
    uint8_t event, status;
    uint16_t opcode;
    BOOL success;

    LOG_DBG("GORMevt_HCE_Common_Complete\n");

    p = (uint8_t *)(p_buf + 1);
    event = *p;
    p += 3;
    STREAM_TO_UINT16(opcode, p);
    status = *p;

    if ((event == 0x0E) && /* Command Complete Event */
        (opcode == wOpCode) && /* OpCode correct */
        (status == 0)) /* Success */
    {
        success = TRUE;
    }
    else {
        success = FALSE;
    }

    if (bt_vnd_cbacks) {
        bt_vnd_cbacks->dealloc(p_buf);
    }

    pthread_mutex_lock(&btinit_ctrl.mutex);
    cmd_status = success ? CMD_SUCCESS : CMD_FAIL;
    /* Wake up command tx thread */
    pthread_cond_signal(&btinit_ctrl.cond);
    pthread_mutex_unlock(&btinit_ctrl.mutex);

    return;
}


VOID notify_thread_exit(VOID)
{
    pthread_mutex_lock(&btinit_ctrl.mutex);
    cmd_status = CMD_TERMINATE;
    /* Wake up command tx thread */
    pthread_cond_signal(&btinit_ctrl.cond);
    pthread_mutex_unlock(&btinit_ctrl.mutex);
}

BOOL _parse_cmd(const char* line, char *cmd_buf) {
    char *key = NULL;
    char *tmp = NULL;
	int i = 0;

    if(line[0] == '#') {
        return FALSE;
    }
    key = strsep(&line, ":");
    if (key == NULL || line == NULL) {
        return FALSE;
    }
    if(strcmp(key, "VENDOR_CMD") != 0) {
        return FALSE;
    }

    LOG_DBG("%s: %s", __func__, line);
    tmp = strsep(&line, " ");  // first whitespace
    while(tmp != NULL && line != NULL && line[0] != ' ') {
        tmp = strsep(&line, " ");
        cmd_buf[i] = (UCHAR)strtol(tmp, NULL, 16);
        i++;
    }
    return TRUE;
}

void _parse_para(const char* line) {
    char* key = NULL;
    UCHAR val = 0;
    if(line[0] == '#') {
        return;
    }
    key = strsep(&line, ":");
    if (key != NULL && line != NULL) {
        val = (UCHAR)strtol(line, NULL, 16);
        if(strcmp(key, "coex_bt_ctrl") == 0) {
            btinit->fw_conf_para.coex_bt_ctrl = val;
        } else if(strcmp(key, "coex_bt_ctrl_mode") == 0) {
            btinit->fw_conf_para.coex_bt_ctrl_mode = val;
        } else if(strcmp(key, "coex_bt_ctrl_rw") == 0) {
            btinit->fw_conf_para.coex_bt_ctrl_rw = val;
        } else if(strcmp(key, "coex_opp_time_ratio") == 0) {
            btinit->fw_conf_para.coex_opp_time_ratio = val;
        } else if(strcmp(key, "coex_opp_time_ratio_bt_slot") == 0) {
            btinit->fw_conf_para.coex_opp_time_ratio_bt_slot = val;
        } else if(strcmp(key, "coex_opp_time_ratio_wifi_slot") == 0) {
            btinit->fw_conf_para.coex_opp_time_ratio_wifi_slot = val;
        } else if(strcmp(key, "coex_lescan_time_ratio") == 0) {
            btinit->fw_conf_para.coex_lescan_time_ratio = val;
        } else if(strcmp(key, "coex_lescan_time_ratio_bt_slot") == 0) {
            btinit->fw_conf_para.coex_lescan_time_ratio_bt_slot = val;
        } else if(strcmp(key, "coex_lescan_time_ratio_wifi_slot") == 0) {
            btinit->fw_conf_para.coex_lescan_time_ratio_wifi_slot = val;
        }
    }
}

int parse_fw_cfg_para(void) {
    FILE *fp = fopen(BT_FW_CFG_FILE, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    if(fp == NULL) {
        LOG_ERR("Unable to open %s!", BT_FW_CFG_FILE);
        return -1;
    }

    while((read = getline(&line, &len, fp)) != -1) {
        //LOG_DBG("read[%zd], len[%zd], strlen[%zd] %s\n", read, len, strlen(line), line);
        _parse_para(line);
    }
    fclose(fp);
    free(line);

    LOG_DBG("btinit.fw_conf_para.coex_bt_ctrl = 0x%02x\n", btinit->fw_conf_para.coex_bt_ctrl);
    LOG_DBG("btinit.fw_conf_para.coex_bt_ctrl_mode = 0x%02x\n", btinit->fw_conf_para.coex_bt_ctrl_mode);
    LOG_DBG("btinit.fw_conf_para.coex_bt_ctrl_rw = 0x%02x\n", btinit->fw_conf_para.coex_bt_ctrl_rw);
    LOG_DBG("btinit.fw_conf_para.coex_opp_time_ratio = 0x%02x\n", btinit->fw_conf_para.coex_opp_time_ratio);
    LOG_DBG("btinit.fw_conf_para.coex_opp_time_ratio_bt_slot = 0x%02x\n", btinit->fw_conf_para.coex_opp_time_ratio_bt_slot);
    LOG_DBG("btinit.fw_conf_para.coex_opp_time_ratio_wifi_slot = 0x%02x\n", btinit->fw_conf_para.coex_opp_time_ratio_wifi_slot);
    LOG_DBG("btinit.fw_conf_para.coex_lescan_time_ratio = 0x%02x\n", btinit->fw_conf_para.coex_lescan_time_ratio);
    LOG_DBG("btinit.fw_conf_para.coex_lescan_time_ratio_bt_slot = 0x%02x\n", btinit->fw_conf_para.coex_lescan_time_ratio_bt_slot);
    LOG_DBG("btinit.fw_conf_para.coex_lescan_time_ratio_wifi_slot = 0x%02x\n", btinit->fw_conf_para.coex_lescan_time_ratio_wifi_slot);
    return 0;
}

BOOL process_cmd_rsp(HCI_CMD_FUNC_T func)
{
    HC_BT_HDR  *p_buf = NULL;
    int rc = 0;
    struct timespec ts;

    pthread_mutex_lock(&btinit_ctrl.mutex);
    if (cmd_status != CMD_TERMINATE) {
        cmd_status = CMD_PENDING;
        pthread_mutex_unlock(&btinit_ctrl.mutex);
    } else {
        pthread_mutex_unlock(&btinit_ctrl.mutex);
        return FALSE;
    }

    if (bt_vnd_cbacks) {
        p_buf = (HC_BT_HDR *)bt_vnd_cbacks->alloc(BT_HC_HDR_SIZE + HCI_CMD_MAX_SIZE);
    } else {
        LOG_ERR("No libbt-hci callbacks!\n");
        return FALSE;
    }

    if (p_buf) {
        p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
        p_buf->offset = 0;
        p_buf->layer_specific = 0;

        if (func(p_buf) == FALSE) {
            LOG_ERR("Send command fails\n");
            if (bt_vnd_cbacks) {
                bt_vnd_cbacks->dealloc(p_buf);
            }
            return FALSE;
        }
    } else {
        LOG_ERR("Alloc command buffer fails\n");
        return FALSE;
    }
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += WAIT_TIME_SECONDS;

    /* Wait for event returned */
    pthread_mutex_lock(&btinit_ctrl.mutex);
    while (cmd_status == CMD_PENDING) {
        rc = pthread_cond_timedwait(&btinit_ctrl.cond, &btinit_ctrl.mutex, &ts);
        if (rc == ETIMEDOUT) {
            LOG_DBG("command timedout, trigger assert\n");
            mtk_set_fw_assert(((UINT32)wOpCode << 16) | 31);
            cmd_status = CMD_FAIL;
            return FALSE;
        }
    }

    if (cmd_status == CMD_SUCCESS) {
        LOG_DBG("Receive success event\n");
        pthread_mutex_unlock(&btinit_ctrl.mutex);
        return TRUE;
    } else if (cmd_status == CMD_FAIL) {
        LOG_ERR("Receive error event\n");
        pthread_mutex_unlock(&btinit_ctrl.mutex);
        return FALSE;
    } else {
        LOG_WAN("FW init thread is forced to exit\n");
        //cmd_status = CMD_SUCCESS; /* restore the global variable for next time */
        pthread_mutex_unlock(&btinit_ctrl.mutex);
        return FALSE;
    }
}

VOID *GORM_FW_Init_Thread(UNUSED_ATTR VOID *ptr)
{
    INT32 i = 0;
    bt_vendor_op_result_t ret = BT_VND_OP_RESULT_FAIL;
    char bt_ssp_debug_val[PROPERTY_VALUE_MAX];
    char bt_pip_val[PROPERTY_VALUE_MAX];

    LOG_DBG("FW init thread starts\n");
    parse_fw_cfg_para();

    /* preload init script */
    switch (btinit->chip_id) {
      case 0x6628:
        btinit->cur_script = bt_init_preload_script_6628;
        btinit->chip_type = BT_EXT_COMBO;
        memcpy(ucDefaultAddr, stBtDefault_6628.addr, 6);
        break;
      case 0x6630:
        btinit->cur_script = bt_init_preload_script_6630;
        btinit->chip_type = BT_EXT_COMBO;
        memcpy(ucDefaultAddr, stBtDefault_6630.addr, 6);
        break;
      case 0x6632:
        btinit->cur_script = bt_init_preload_script_6632;
        btinit->chip_type = BT_EXT_COMBO;
        memcpy(ucDefaultAddr, stBtDefault_6632.addr, 6);
        break;
      case 0x8163:
      case 0x8127:
      case 0x8167:
      case 0x6582:
      case 0x6592:
      case 0x6752:
      case 0x0321:
      case 0x0335:
      case 0x0337:
      case 0x6580:
      case 0x6570:
      case 0x6735:
      case 0x6755:
      case 0x6797:
      case 0x6757:
      case 0x6759:
      case 0x6763:
      case 0x6758:
      case 0x6739:
      case 0x6771:
      case 0x6775:
        LOG_WAN("A-D die chip id: %04x\n", btinit->chip_id);
        btinit->cur_script = bt_init_preload_script_consys;
        btinit->chip_type = BT_CONSYS;
        memcpy(ucDefaultAddr, stBtDefault_consys.addr, 6);
        break;
      case 0x6765:
      case 0x3967:
      case 0x6761:
      case 0x6768:
      case 0x6785:
        LOG_WAN("CONNAC chip id: %04x\n", btinit->chip_id);
        btinit->cur_script = bt_init_preload_script_connac;
        btinit->chip_type = BT_CONNAC;
        memcpy(ucDefaultAddr, stBtDefault_connac_soc_1_0.addr, 6);
        break;
      case 0x6779:
        LOG_WAN("CONNAC chip id: %04x\n", btinit->chip_id);
        btinit->cur_script = bt_init_preload_script_connac;
        btinit->chip_type = BT_CONNAC;
        #ifdef MTK_CONSYS_ADIE_6631
          LOG_WAN("Use connsys 6631 setting!");
          memcpy(ucDefaultAddr, stBtDefault_connac_soc_2_0_6631.addr, 6);
        #else
          LOG_WAN("Use connsys 6635 setting!");
          memcpy(ucDefaultAddr, stBtDefault_connac_soc_2_0.addr, 6);
        #endif
        break;
      case 0x6885:
        LOG_WAN("CONNAC chip id: %04x\n", btinit->chip_id);
        btinit->cur_script = bt_init_preload_script_connac20;
        btinit->chip_type = BT_CONNAC;
        memcpy(ucDefaultAddr, stBtDefault_connac_soc_3_0.addr, 6);
        break;
      default:
        LOG_ERR("Unknown combo chip id: %04x\n", btinit->chip_id);
        break;
    }

    /* Can not find matching script, simply skip */
    if ((btinit->cur_script) == NULL) {
        LOG_ERR("No matching init script\n");
        goto exit;
    }

    /* BT chip_type was not initialized, simply skip */
    if (btinit->chip_type == BT_UNKNOWN) {
        LOG_ERR("chip_type was not initialized!\n");
        goto exit;
    }

    if (btinit->chip_type == BT_EXT_COMBO) {
        if ((0 == memcmp(btinit->bt_nvram.fields.addr, ucDefaultAddr, 6)) ||
            (0 == memcmp(btinit->bt_nvram.fields.addr, ucZeroAddr, 6))) {
            /* NVRAM BD address default value */
            /* Want to retrieve module eFUSE address on external combo chip */
            fgGetEFUSE = TRUE;
        }

        if (fgGetEFUSE) /* perform GORMcmd_HCC_Get_Local_BD_Addr */
            i = 0;
        else /* skip GORMcmd_HCC_Get_Local_BD_Addr */
            i = 1;
    }

    /* process init script */
    while (btinit->cur_script[i].command_func) {
        /* Some debug commands are executed by request */
        if (btinit->cur_script[i].command_func == GORMcmd_HCC_Set_SSP_Debug_Mode) {
            if (!property_get("persist.vendor.bt.sspdebug.enable", bt_ssp_debug_val, NULL) ||
                0 != strcmp(bt_ssp_debug_val, "1")) {
                i++;
                continue;
            }
        } else if (btinit->cur_script[i].command_func == GORMcmd_HCC_Set_PIP) {
            if (!property_get("persist.vendor.bt.pip.enable", bt_pip_val, NULL) ||
                0 != strcmp(bt_pip_val, "1")) {
                i++;
                continue;
            }
        }
        if(process_cmd_rsp(btinit->cur_script[i].command_func) != TRUE) {
            goto exit;
        }
        i++;
    }
    if (btinit->cur_script[i].command_func == NULL)
        ret = BT_VND_OP_RESULT_SUCCESS;

    /* process vendor cmd assigned in cfg file */
    FILE *fp = fopen(BT_FW_CFG_FILE, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    if(fp != NULL) {
        while((read = getline(&line, &len, fp)) != -1) {
            if(_parse_cmd(line, btinit->fw_conf_cmd) == TRUE) {
                if(process_cmd_rsp(GORMcmd_HCC_Fw_Cfg_Cmd) != TRUE) {
                    fclose(fp);
                    free(line);
                    goto exit;
                }
            }
        }
        fclose(fp);
        free(line);
    }

exit:
    pthread_mutex_lock(&btinit_ctrl.mutex);
    if (bt_vnd_cbacks) {
        bt_vnd_cbacks->fwcfg_cb(ret);
    }
    pthread_mutex_unlock(&btinit_ctrl.mutex);

    btinit_ctrl.worker_thread_running = FALSE;
    return NULL;
}

UCHAR GetRandomByte(UCHAR exclude)
{
    struct timeval tv;
    UCHAR ret = exclude;
    while(ret == exclude) {
        gettimeofday(&tv, NULL);
        srand(tv.tv_usec);
        ret = ((rand()>>8) & 0xFF);
    }
    return ret;
}

static VOID GetRandomValue(UCHAR string[6])
{
    INT32 iRandom = 0;
    LOG_WAN("Enable random generation\n");

    /* Initialize random seed */
    srand(time(NULL));
    iRandom = rand();
    string[0] = (((iRandom>>24|iRandom>>16) & (0xFE)) | (0x02)); /* Must use private bit(1) and no BCMC bit(0) */

    string[1] = GetRandomByte(0x00);
    /* Generate random LAP, exclude 0x9E8B00 ~ 0x9E8BFF*/
    string[3] = GetRandomByte(0x9E);
    string[4] = GetRandomByte(0x8B);
    string[5] = GetRandomByte(0x00);
    LOG_WAN("Generate bd_addr:  %02x-%02x-%02x-%02x-%02x-%02x\n",
        string[0], string[1], string[2], string[3], string[4], string[5]);
    return;
}

static BOOL WriteBDAddrToNvram(UCHAR *pucBDAddr)
{
    F_ID bt_nvram_fd;
    INT32 rec_size = 0;
    INT32 rec_num = 0;

    bt_nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISWRITE);
    if (bt_nvram_fd.iFileDesc < 0) {
        LOG_WAN("Open BT NVRAM fails errno %d\n", errno);
        return FALSE;
    }

    if (rec_num != 1) {
        LOG_ERR("Unexpected record num %d\n", rec_num);
        NVM_CloseFileDesc(bt_nvram_fd);
        return FALSE;
    }

    if (rec_size != sizeof(ap_nvram_btradio_struct)) {
        LOG_ERR("Unexpected record size %d ap_nvram_btradio_struct %zu\n",
                rec_size, sizeof(ap_nvram_btradio_struct));
        NVM_CloseFileDesc(bt_nvram_fd);
        return FALSE;
    }

    lseek(bt_nvram_fd.iFileDesc, 0, 0);

    /* Update BD address */
    if (write(bt_nvram_fd.iFileDesc, pucBDAddr, 6) < 0) {
        LOG_ERR("Write BT NVRAM fails errno %d\n", errno);
        NVM_CloseFileDesc(bt_nvram_fd);
        return FALSE;
    }

    NVM_CloseFileDesc(bt_nvram_fd);
    return TRUE;
}
