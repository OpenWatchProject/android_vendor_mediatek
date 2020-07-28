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
#include <string.h>
#include <fcntl.h>
#include <error.h>
#include <cutils/properties.h>

/* use nvram */
#include "CFG_BT_File.h"
#include "CFG_BT_Default.h"
#include "CFG_file_lid.h"
#include "libnvram.h"

#include "bt_kal.h"
#include "cust_bt.h"
#include "bt_drv.h"

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/
/* Audio interface & Codec information Mapping */
struct audio_t audio_conf_map[] = {
#if defined(__MTK_MERGE_INTERFACE_SUPPORT__)
    { 0x6628,    { MERGE_INTERFACE,  SYNC_8K,  SHORT_FRAME,  0 } },
#else
    { 0x6628,    { PCM,              SYNC_8K,  SHORT_FRAME,  0 } },
#endif
#if defined(__MTK_MERGE_INTERFACE_SUPPORT__)
    { 0x6630,    { MERGE_INTERFACE,  SYNC_8K,  SHORT_FRAME,  0 } },
#elif defined(__MTK_BT_I2S_SUPPORT__)
    { 0x6630,    { I2S,              SYNC_8K,  SHORT_FRAME,  0 } },
#else
    { 0x6630,    { PCM,              SYNC_8K,  SHORT_FRAME,  0 } },
#endif
#if defined(__MTK_MERGE_INTERFACE_SUPPORT__)
    { 0x6632,    { MERGE_INTERFACE,  SYNC_8K,  SHORT_FRAME,  0 } },
#elif defined(__MTK_BT_I2S_SUPPORT__)
    { 0x6632,    { I2S,              SYNC_8K,  SHORT_FRAME,  0 } },
#else
    { 0x6632,    { PCM,              SYNC_8K,  SHORT_FRAME,  0 } },
#endif
    { 0x8163,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x8127,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x8167,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6582,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6592,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6752,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x0321,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x0335,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x0337,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6580,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6570,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6735,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6755,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6797,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6757,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6759,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6763,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6758,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6739,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6771,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6775,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6765,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x3967,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6761,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6768,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6779,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6785,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0x6885,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },
    { 0,         { 0,                0,        0,            0 } }
};

static int io_flags;

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

extern BOOL BT_InitDevice(INT32 comPort, UINT32 chipId, PUCHAR pucNvRamData);

extern BOOL BT_DeinitDevice(INT32 comPort);

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

/* Initialize UART driver */
static int init_uart(char *dev)
{
    int fd;

    LOG_TRC();

    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        LOG_ERR("Can't open %s (%s), errno[%d]\n", dev, strerror(errno), errno);
        return -1;
    }

    io_flags = 0;
    return fd;
}

int bt_get_combo_id(unsigned int *pChipId)
{
    int  chipId_ready_retry = 0;
    char chipId_val[PROPERTY_VALUE_MAX];

    do {
        if (property_get("persist.vendor.connsys.chipid", chipId_val, NULL) &&
            0 != strcmp(chipId_val, "-1")){
            *pChipId = (unsigned int)strtoul(chipId_val, NULL, 16);
            break;
        }
        else {
            chipId_ready_retry ++;
            usleep(500000);
        }
    } while(chipId_ready_retry < 10);

    LOG_DBG("Get combo chip id retry %d\n", chipId_ready_retry);
    if (chipId_ready_retry >= 10) {
        LOG_DBG("Invalid combo chip id!\n");
        return -1;
    }
    else {
        LOG_DBG("Combo chip id %x\n", *pChipId);
        return 0;
    }
}

static int bt_read_nvram(unsigned char *pucNvRamData)
{
    F_ID bt_nvram_fd;
    int rec_size = 0;
    int rec_num = 0;
    ap_nvram_btradio_struct bt_nvram;

    int nvram_ready_retry = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];

    LOG_TRC();

    /* Sync with Nvram daemon ready */
    do {
        if (property_get("vendor.service.nvram_init", nvram_init_val, NULL) &&
            0 == strcmp(nvram_init_val, "Ready"))
            break;
        else {
            nvram_ready_retry ++;
            usleep(500000);
        }
    } while(nvram_ready_retry < 10);

    LOG_DBG("Get NVRAM ready retry %d\n", nvram_ready_retry);
    if (nvram_ready_retry >= 10) {
        LOG_ERR("Get NVRAM restore ready fails!\n");
        return -1;
    }

    bt_nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISREAD);
    if (bt_nvram_fd.iFileDesc < 0) {
        LOG_WAN("Open BT NVRAM fails errno %d\n", errno);
        return -1;
    }

    if (rec_num != 1) {
        LOG_ERR("Unexpected record num %d", rec_num);
        NVM_CloseFileDesc(bt_nvram_fd);
        return -1;
    }

    if (rec_size != sizeof(ap_nvram_btradio_struct)) {
        LOG_ERR("Unexpected record size %d ap_nvram_btradio_struct %zu",
                rec_size, sizeof(ap_nvram_btradio_struct));
        NVM_CloseFileDesc(bt_nvram_fd);
        return -1;
    }

    if (read(bt_nvram_fd.iFileDesc, &bt_nvram, rec_num*rec_size) < 0) {
        LOG_ERR("Read NVRAM fails errno %d\n", errno);
        NVM_CloseFileDesc(bt_nvram_fd);
        return -1;
    }

    NVM_CloseFileDesc(bt_nvram_fd);
    memcpy(pucNvRamData, &bt_nvram, sizeof(ap_nvram_btradio_struct));

    return 0;
}


/* MTK specific chip initialize process */
int bt_init(void)
{
    int fd = -1;
    unsigned int chipId = 0;
    ap_nvram_btradio_struct nvData, zero;
    memset(&nvData, 0, sizeof(nvData));
    memset(&zero, 0, sizeof(zero));

    LOG_TRC();

    fd = init_uart(CUST_BT_SERIAL_PORT);
    if (fd < 0){
        LOG_ERR("Can't initialize" CUST_BT_SERIAL_PORT "\n");
        return -1;
    }

    /* Get combo chip id */
    if (bt_get_combo_id(&chipId) < 0) {
        LOG_ERR("Get combo chip id fails\n");
        goto error;
    }

    /* Read NVRAM data */
    if ((bt_read_nvram((unsigned char *)&nvData) < 0) ||
          0 == memcmp(&nvData, &zero, sizeof(ap_nvram_btradio_struct))) {
        LOG_ERR("Read NVRAM data fails or NVRAM data all zero!!\n");
        LOG_WAN("Use %04x default value\n", chipId);
        switch (chipId) {
          case 0x6628:
            /* Use MT6628 default value */
            memcpy(&nvData, &stBtDefault_6628, sizeof(ap_nvram_btradio_struct));
            break;
          case 0x6630:
            /* Use MT6630 default value */
            memcpy(&nvData, &stBtDefault_6630, sizeof(ap_nvram_btradio_struct));
            break;
          case 0x6632:
            /* Use MT6632 default value */
            memcpy(&nvData, &stBtDefault_6632, sizeof(ap_nvram_btradio_struct));
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
            /* Use A-D die default value */
            memcpy(&nvData, &stBtDefault_consys, sizeof(ap_nvram_btradio_struct));
            break;
          case 0x6765:
          case 0x3967:
          case 0x6761:
          case 0x6768:
          case 0x6785:
            /* Use CONNAC1.0 SOC1_0 default value */
            memcpy(&nvData, &stBtDefault_connac_soc_1_0, sizeof(ap_nvram_btradio_struct));
            break;
          case 0x6779:
            /* Use CONNAC1.0 SOC2_0 default value */
            #ifdef MTK_CONSYS_ADIE_6631
              memcpy(&nvData, &stBtDefault_connac_soc_2_0_6631, sizeof(ap_nvram_btradio_struct));
            #else
              memcpy(&nvData, &stBtDefault_connac_soc_2_0, sizeof(ap_nvram_btradio_struct));
            #endif
            break;
          case 0x6885:
            memcpy(&nvData, &stBtDefault_connac_soc_3_0, sizeof(ap_nvram_btradio_struct));
            break;
          default:
            LOG_WAN("Unknown combo chip id: %04x\n", chipId);
            goto error;
        }
    }

    LOG_WAN("[BDAddr %02x-%02x-%02x-%02x-%02x-%02x][Voice %02x %02x][Codec %02x %02x %02x %02x] \
             [Radio %02x %02x %02x %02x %02x %02x][Sleep %02x %02x %02x %02x %02x %02x %02x][BtFTR %02x %02x] \
             [TxPWOffset %02x %02x %02x][CoexAdjust %02x %02x %02x %02x %02x %02x][PIP %02x %02x] \
             [RadioExt %02x %02x][TxPWOffsetExt1 %02x %02x %02x]\
             [TxPWOffsetExt2 %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]\n",
             nvData.addr[0], nvData.addr[1], nvData.addr[2], nvData.addr[3], nvData.addr[4], nvData.addr[5],
             nvData.Voice[0], nvData.Voice[1],
             nvData.Codec[0], nvData.Codec[1], nvData.Codec[2], nvData.Codec[3],
             nvData.Radio[0], nvData.Radio[1], nvData.Radio[2], nvData.Radio[3], nvData.Radio[4], nvData.Radio[5],
             nvData.Sleep[0], nvData.Sleep[1], nvData.Sleep[2], nvData.Sleep[3], nvData.Sleep[4], nvData.Sleep[5], nvData.Sleep[6],
             nvData.BtFTR[0], nvData.BtFTR[1],
             nvData.TxPWOffset[0], nvData.TxPWOffset[1], nvData.TxPWOffset[2],
             nvData.CoexAdjust[0], nvData.CoexAdjust[1], nvData.CoexAdjust[2], nvData.CoexAdjust[3], nvData.CoexAdjust[4], nvData.CoexAdjust[5],
             nvData.PIP[0], nvData.PIP[1],
             nvData.RadioExt[0], nvData.RadioExt[1],
             nvData.TxPWOffsetExt1[0], nvData.TxPWOffsetExt1[1], nvData.TxPWOffsetExt1[2],
             nvData.TxPWOffsetExt2[0], nvData.TxPWOffsetExt2[1], nvData.TxPWOffsetExt2[2], nvData.TxPWOffsetExt2[3], nvData.TxPWOffsetExt2[4],
             nvData.TxPWOffsetExt2[5], nvData.TxPWOffsetExt2[6], nvData.TxPWOffsetExt2[7], nvData.TxPWOffsetExt2[8], nvData.TxPWOffsetExt2[9]);

    if (BT_InitDevice(fd, chipId, (unsigned char *)&nvData) == FALSE) {
        LOG_ERR("Initialize BT device fails\n");
        goto error;
    }

    LOG_WAN("bt_init success\n");
    return fd;

error:
    if (fd >= 0)
        close(fd);
    return -1;
}

/* MTK specific deinitialize process */
int bt_restore(int fd)
{
    LOG_TRC();
    BT_DeinitDevice(fd);
    close(fd);
    return 0;
}

int bt_send_data(int fd, unsigned char *buf, unsigned int len)
{
    int bytesWritten = 0;
    unsigned int bytesToWrite = len;

    if (fd < 0)
        return -EINVAL;

    /* Try to send len bytes data in buffer */
    while (bytesToWrite > 0) {
        bytesWritten = write(fd, buf, bytesToWrite);
        if (bytesWritten < 0) {
            if (((errno == EINTR) && !(io_flags & WR_INTERRUPTIBLE)) || (errno == EAGAIN))
                continue;
            else
                return -errno;
        }
        bytesToWrite -= bytesWritten;
        buf += bytesWritten;
    }

    return 0;
}

int bt_receive_data(int fd, unsigned char *buf, unsigned int len)
{
    int bytesRead = 0;
    unsigned int bytesToRead = len;

    int ret = 0;
    struct timeval tv;
    fd_set readfd;

    tv.tv_sec = 5; /* SECOND */
    tv.tv_usec = 0; /* USECOND */
    FD_ZERO(&readfd);

    if (fd < 0)
        return -EINVAL;

    /* Try to receive len bytes */
    while (bytesToRead > 0) {

        FD_SET(fd, &readfd);
        ret = select(fd + 1, &readfd, NULL, NULL, &tv);

        if (ret > 0) {
            bytesRead = read(fd, buf, bytesToRead);
            if (bytesRead < 0) {
                if (((errno == EINTR) && !(io_flags & RD_INTERRUPTIBLE)) || (errno == EAGAIN))
                    continue;
                else
                    return -errno;
            }
            else {
                bytesToRead -= bytesRead;
                buf += bytesRead;
            }
        }
        else if (ret == 0) {
            LOG_ERR("Read com port timeout 5s!\n");
            return -ETIMEDOUT;
        }
        else if ((ret == -1) && (errno == EINTR) && !(io_flags & RD_INTERRUPTIBLE)) {
            LOG_ERR("select error ignore EINTR\n");
        }
        else {
            LOG_ERR("select error %s(%d)!\n", strerror(errno), errno);
            return -errno;
        }
    }

    return 0;
}

void bt_set_io_attr(int flags)
{
    io_flags = flags;
    LOG_DBG("current io_flags %x\n", io_flags);
}

int bt_get_io_attr(void)
{
    return io_flags;
}

static int bt_get_audio_configuration(BT_INFO *pBTInfo)
{
    unsigned int chipId = 0;
    int i;

    LOG_DBG("BT_MTK_OP_AUDIO_GET_CONFIG\n");

    /* Get combo chip id */
    if (bt_get_combo_id(&chipId) < 0) {
        LOG_ERR("Get combo chip id fails\n");
        return -2;
    }

    /* Return the specific audio config on current chip */
    for (i = 0; audio_conf_map[i].chip_id; i++) {
        if (audio_conf_map[i].chip_id == chipId) {
            LOG_DBG("Find chip %x\n", chipId);
            memcpy(&(pBTInfo->audio_conf), &(audio_conf_map[i].audio_conf), sizeof(AUDIO_CONFIG));
            return 0;
        }
    }

    LOG_ERR("Current chip is not included in audio_conf_map\n");
    return -3;
}

int mtk_bt_op(bt_mtk_opcode_t opcode, void *param)
{
    int ret = -1;

    switch (opcode) {
      case BT_MTK_OP_AUDIO_GET_CONFIG: {
        BT_INFO *pBTInfo = (BT_INFO*)param;
        if (pBTInfo != NULL) {
            ret = bt_get_audio_configuration(pBTInfo);
        }
        else {
            LOG_ERR("BT_MTK_OP_AUDIO_GET_CONFIG have NULL as parameter\n");
        }
        break;
      }
      default:
        LOG_ERR("Unknown operation %d\n", opcode);
        break;
    }
    return ret;
}
