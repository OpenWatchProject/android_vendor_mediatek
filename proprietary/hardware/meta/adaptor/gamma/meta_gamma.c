/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_gamma.cpp
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *   Implement GAMMA calibration interface for META mode.
 *
 * Author:
 * -------
 *
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#include "meta_gamma_para.h"

/* for read NVRAM */
#include "libnvram.h"
#include "CFG_PQ_File.h"
#include "CFG_PQ_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"

/*****************************************************************************
 *
 * definition for write gamma to  hardware register
 *
 ****************************************************************************/
#define DISP_IOCTL_SET_GAMMALUT    _IOW    (DISP_IOCTL_MAGIC, 23 , DISP_GAMMA_LUT_T)
typedef enum {
    DISP_GAMMA0 = 0,
    DISP_GAMMA1,
    DISP_GAMMA_TOTAL
} disp_gamma_id_t;

typedef unsigned int gamma_entry;
#define GAMMA_ENTRY(r10, g10, b10) (((r10) << 20) | ((g10) << 10) | (b10))

#define DISP_GAMMA_LUT_SIZE 512
#define TOOL_GAMMA_LUT_SIZE 512

typedef struct {
    disp_gamma_id_t hw_id;
    gamma_entry lut[DISP_GAMMA_LUT_SIZE];
} DISP_GAMMA_LUT_T;

typedef struct {
    GAMMA_CMD_TYPE cmd_type;
    unsigned int param0;
    unsigned int param1;
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int lutNodeN;
    unsigned int lutIndex;
    unsigned int gainIndex;
    unsigned int checksum;
    bool aal_ink_en;
    bool ccorr_en;
    bool aal_en;
    bool color_en;
    bool gamma_en;
	unsigned int rgbindex;
	unsigned short lut[1024];

} GAMMA_CAL_CMD_PARAMETER;

int drvID = -1;

static unsigned int g_gamma_lut_template[257][3];
static unsigned int g_gamma_lut_nvram[257][3];
static unsigned int g_gamma_lut[1024][3];
static unsigned int g_gamma_lut_hw[DISP_GAMMA_LUT_SIZE][3];
static unsigned int g_gamma_gain[14][3];
static unsigned int g_gamma_lut_node_number=0;
static unsigned int g_gamma_lut_index = 0;
static unsigned int g_gamma_lut_pos = 0;
static unsigned int g_gamma_lut_pos_pre = 0;
static unsigned int g_gamma_lut_checksum = 0;

static bool __DEBUG__ = true;
#define GAMMA_LOG(fmt, args...)                           \
    do {                                                \
        if (__DEBUG__)                                  \
            ALOGD("AUTO_GAMMA: %s: "fmt, __func__,##args); \
    } while (0)

#define GAMMA_ERR(fmt, args...)                           \
    do {                                                \
        ALOGE("AUTO_GAMMA: %s: "fmt, __func__,##args); \
    } while (0)

/*****************************************************************************
 *
 * auto gamma command parser
 *
 ****************************************************************************/
static void parseGammaCmd(GAMMA_REQ *req, GAMMA_CAL_CMD_PARAMETER *cmd_parameter,
	char *peer_buff, unsigned short peer_len)
{
    int cmd_type = 0;
	int i = 0;

    cmd_parameter->cmd_type = req->type;

    /* dispatch parameter */
    switch(cmd_parameter->cmd_type) {
        case GAMMA_PQ_ONOFF:
            cmd_parameter->aal_ink_en = (req->param.pq_onoff_ctl.aalink == 0)? false : true;
            cmd_parameter->color_en = (req->param.pq_onoff_ctl.color == 0)? false : true;
            cmd_parameter->ccorr_en = (req->param.pq_onoff_ctl.ccorr == 0)? false : true;
            cmd_parameter->aal_en = (req->param.pq_onoff_ctl.aal == 0)? false : true;
            cmd_parameter->gamma_en = (req->param.pq_onoff_ctl.gamma == 0)? false : true;
            break;

        case GAMMA_SET_DISP_PATTERN:
            cmd_parameter->R = req->param.ink_rgb.r;
            cmd_parameter->G = req->param.ink_rgb.g;
            cmd_parameter->B = req->param.ink_rgb.b;
            break;

        case GAMMA_SET_LUT_SIZE:
            cmd_parameter->lutNodeN = req->param.lut_size.size;
            break;

        case GAMMA_WRITE_LUT:
            cmd_parameter->rgbindex = req->param.lut_rgb.rgbindex;
			memcpy(cmd_parameter->lut, peer_buff, peer_len);
            break;

        case GAMMA_WRITE_LUT_FINISH:
            break;

        case GAMMA_READ_CHECKSUM:
            break;

        case GAMMA_WRITE_GAIN:
            cmd_parameter->gainIndex = req->param.gain_rgb.gain_index;
            cmd_parameter->R = req->param.gain_rgb.r;
            cmd_parameter->G = req->param.gain_rgb.g;
            cmd_parameter->B = req->param.gain_rgb.b;
            break;

        case GAMMA_DUMP_LUT:
            break;

        default:
            break;
    }

    /* fetch parameter from command */

    GAMMA_LOG("cmd type=%d param0=0x%08x param1=0x%08x",
        cmd_parameter->cmd_type, cmd_parameter->param0, cmd_parameter->param1);
    GAMMA_LOG("cmd type=%d color(%d) ccorr(%d) aal(%d) gamma(%d)",
        cmd_parameter->cmd_type, cmd_parameter->color_en, cmd_parameter->ccorr_en,
        cmd_parameter->aal_en, cmd_parameter->gamma_en);
    GAMMA_LOG("cmd type=%d R=0x%08x G=0x%08x B=0x%08x",
        cmd_parameter->cmd_type, cmd_parameter->R, cmd_parameter->G, cmd_parameter->B);
    GAMMA_LOG("cmd type=%d lutNodeN=0x%08x lutIndex=0x%08x gainIndex=0x%08x",
        cmd_parameter->cmd_type, cmd_parameter->lutNodeN, cmd_parameter->lutIndex, cmd_parameter->gainIndex);
}

/*****************************************************************************
 *
 * PQ module on/off function
 *
 ****************************************************************************/
static void setPQOnOff(GAMMA_CAL_CMD_PARAMETER *gamma_cal_cmd)
{
    DISP_READ_REG read_reg_params;
    DISP_WRITE_REG write_reg_params;
    int colorRegBase, aalRegBase, gammaRegBase, ccorrRegbase;
    unsigned int value;
    int ret = -1;
	int mutex_value = 1;

	//ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &mutex_value);

    /* COLOR */
    value = 0;
    read_reg_params.reg = 0xFFFF0000;
    read_reg_params.mask = 0xFFFFFFFF;
    ret = ioctl(drvID, DISP_IOCTL_READ_SW_REG, &read_reg_params);
    colorRegBase = read_reg_params.val ;
    /* bypass color */
    value = (gamma_cal_cmd->color_en == true)? 0x0:0x80;
    write_reg_params.reg  = colorRegBase + 0x400;
    write_reg_params.val  = value;
    write_reg_params.mask = 0x80;
    ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);
    GAMMA_LOG("color en=%d ret=%d(%x)", gamma_cal_cmd->color_en, ret, colorRegBase);

    /* AAL */
    value = 0;
    read_reg_params.reg = 0xFFFF0003;
    read_reg_params.mask = 0xFFFFFFFF;
    ioctl(drvID, DISP_IOCTL_READ_SW_REG, &read_reg_params);
    aalRegBase = read_reg_params.val;
    /* enable engine */
    value = 0x2;
    write_reg_params.reg  = aalRegBase + 0x20;
    write_reg_params.val  = value;
    write_reg_params.mask = 0x3;
    ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);
    /* force cabc gain=256 */
    value = 256;
    write_reg_params.reg  = aalRegBase + 0x214;
    write_reg_params.val  = value;
    write_reg_params.mask = 0x3ff;
    ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);
    /* bypass dre map */
    value = (gamma_cal_cmd->aal_en == true)? 0x0:0x10;
    write_reg_params.reg  = aalRegBase + 0x3b0;
    write_reg_params.val  = value;
    write_reg_params.mask = 0x10;
    ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);
    /* aal ink on/off */
    write_reg_params.reg = aalRegBase + 0x21c;
    //write_reg_params.val = (gamma_cal_cmd->aal_ink_en == true)? 0x80000000:0x00000000;
    write_reg_params.val = 0x80000000;
    write_reg_params.mask = 0x80000000;
    ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);
    GAMMA_LOG("aal en=%d ret=%d(%x)", gamma_cal_cmd->aal_en, ret, aalRegBase);

    /* GAMMA */
    value = 0x0;
    read_reg_params.reg = 0xFFFF0001;
    read_reg_params.mask = 0xFFFFFFFF;
    ioctl(drvID, DISP_IOCTL_READ_SW_REG, &read_reg_params);
    gammaRegBase = read_reg_params.val;

    /* on/off GAMMA_LUT_EN */
    value = (gamma_cal_cmd->gamma_en == true)? 0x0:0x1;
    write_reg_params.reg  = gammaRegBase + 0x20;
    write_reg_params.val  = value;
    write_reg_params.mask = 0x1;
    ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);
    GAMMA_LOG("gamma en=%d ret=%d(%x)", gamma_cal_cmd->gamma_en, ret, gammaRegBase);

    /* CCORR */
    value = 0x0;
    read_reg_params.reg = 0xFFFF0007;
    read_reg_params.mask = 0xFFFFFFFF;
    ioctl(drvID, DISP_IOCTL_READ_SW_REG, &read_reg_params);
    ccorrRegbase = read_reg_params.val;

    value = (gamma_cal_cmd->ccorr_en== true)? 0x0:0x1;
    write_reg_params.reg  = ccorrRegbase + 0x20;
    write_reg_params.val  = value;
    write_reg_params.mask = 0x1;
    ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);
    GAMMA_LOG("ccorr en=%d ret=%d(%x)", gamma_cal_cmd->gamma_en, ret, ccorrRegbase);
END:
	mutex_value = 2;
	ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &mutex_value);
}

/*****************************************************************************
 *
 * AAL ink pattern control function
 *
 ****************************************************************************/
static int inkIdx=0;
static void setAalInk(GAMMA_CAL_CMD_PARAMETER *gamma_cal_cmd)
{
    DISP_READ_REG read_reg_params;
    DISP_WRITE_REG write_reg_params;
    int RegBase;
    int ret = -1;
    int rgbConf, r, g, b;
    unsigned int value;
	int mutex_value = 1;

	//ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &mutex_value);
	// convert 8 bit (0~255) pattern to 9 bit (0~511)
	r = (gamma_cal_cmd->R << 1) & 0x1ff;
    g = (gamma_cal_cmd->G << 1) & 0x1ff;
    b = (gamma_cal_cmd->B << 1) & 0x1ff;
	// level (8 bit 255) to level (9 bit 511)
	r = (r == 510) ? 511 : r;
	g = (g == 510) ? 511 : g;
	b = (b == 510) ? 511 : b;

    rgbConf = (b<<18) | (g<<9) | r;

    /* AAL */
    value = 0x0;
    read_reg_params.reg = 0xFFFF0003;
    read_reg_params.mask = 0xFFFFFFFF;
    ioctl(drvID, DISP_IOCTL_READ_SW_REG, &read_reg_params);
    RegBase = read_reg_params.val;
	/* enable CABC */
	value = (1 << 31);
    write_reg_params.reg = RegBase + 0x20c;
    write_reg_params.val = value;
    write_reg_params.mask = (1 << 31);
	ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);

    value = rgbConf | 0x80000000;
    write_reg_params.reg = RegBase + 0x21c;
    write_reg_params.val = value;
    write_reg_params.mask = 0xFFFFFFFF;
    ret = ioctl(drvID, DISP_IOCTL_WRITE_REG, &write_reg_params);
    GAMMA_LOG("set aal RGBConf=%x ret=%d(%x)", rgbConf, ret, RegBase);

	mutex_value = 2;
	ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &mutex_value);
}

/*****************************************************************************
 *
 * config gamma table size
 *
 ****************************************************************************/
static void setGammaLutSize(GAMMA_CAL_CMD_PARAMETER *gamma_cal_cmd)
{
	int i;

    g_gamma_lut_index = 0;
    g_gamma_lut_pos = 0;
    g_gamma_lut_pos_pre = 0;
    g_gamma_lut_checksum = 0;
    g_gamma_lut_node_number = gamma_cal_cmd->lutNodeN;

	for (i = 0; i < 1024; i += 1)
		gamma_cal_cmd->lut[i] = 0;

    GAMMA_LOG("set gamma lut node num=%d",
        g_gamma_lut_node_number);
}

/*****************************************************************************
 *
 * config gamma table size
 *
 ****************************************************************************/
static int GetLevelBySizeInterp(int Size, int CurrentIndex, int MaxLevel, int MinLevel)
{
    double Ratio = ((double)CurrentIndex) / (Size - 1);
    return (int)(Ratio * (MaxLevel - MinLevel) + MinLevel + 0.5);
}

static int GetLevelBySizeIndex(int Size, int CurrentIndex)
{
    return GetLevelBySizeInterp(Size, CurrentIndex, 255, 0);
}

/*****************************************************************************
 *
 * 1. remapping node to range 0~255 ex: mapping 1 in range 0~16 to 16 inrange 0~255
 * 2. interpolation for fill 256 lut
 *
 ****************************************************************************/
static void writeGammaLut256(GAMMA_CAL_CMD_PARAMETER *gamma_cal_cmd)
{
    unsigned int node0, node1, nodei, interval, intepor;
    int i, j;

    /* remapping node from range 0~g_gamma_lut_node_number to 0~256 */
    if(g_gamma_lut_node_number == 256) {
        g_gamma_lut_index = (g_gamma_lut_index > 256) ? 256 : g_gamma_lut_index;
    } else {
        if(g_gamma_lut_index < g_gamma_lut_node_number)
            g_gamma_lut_pos = GetLevelBySizeIndex(g_gamma_lut_node_number, g_gamma_lut_index);
        else
            g_gamma_lut_pos = 256;
    }

    GAMMA_LOG("debug gamma lut256 pos=%4d previous pos=%4d",
        g_gamma_lut_pos, g_gamma_lut_pos_pre);

    g_gamma_lut_template[g_gamma_lut_pos][0] = gamma_cal_cmd->R;
    g_gamma_lut_template[g_gamma_lut_pos][1] = gamma_cal_cmd->G;
    g_gamma_lut_template[g_gamma_lut_pos][2] = gamma_cal_cmd->B;

    g_gamma_lut_checksum += gamma_cal_cmd->R;
    g_gamma_lut_checksum += gamma_cal_cmd->G;
    g_gamma_lut_checksum += gamma_cal_cmd->B;
    g_gamma_lut_checksum &= 0xffff;

    /* level 1 inteporation to 256 */
    if(g_gamma_lut_pos > g_gamma_lut_pos_pre) {

        interval = g_gamma_lut_pos - g_gamma_lut_pos_pre;

        for(i = 0; i < 3; i++) {
            node0 = g_gamma_lut_template[g_gamma_lut_pos_pre][i];
            node1 = g_gamma_lut_template[g_gamma_lut_pos][i];

            for(intepor = 1; intepor < interval; intepor++) {
                nodei = (node0*(interval-intepor) +  node1*intepor) / interval;
                g_gamma_lut_template[g_gamma_lut_pos_pre+intepor][i] = nodei;
                GAMMA_LOG("debug gamma lut256(%3d) intepor (%4d)-(%4d)-(%4d)",
                    g_gamma_lut_pos_pre+intepor, node0, nodei, node1);
            }
        }
    }

    g_gamma_lut_pos_pre = g_gamma_lut_pos;
    g_gamma_lut_index += 1;

}

static int writeGammaLut(GAMMA_CAL_CMD_PARAMETER *gamma_cal_cmd,
	unsigned int gamma_lut[1024][3])
{
	int i;
	int index = gamma_cal_cmd->rgbindex;

	if (index < 0 || index > 2)
		return -1;
#if 0
	for (i = 0; i < 512; i += 1) {
		gamma_cal_cmd->lut[i] = 4096 - i*8;
	}
#endif
	for (i = 0; i < 1024; i += 1) {
		gamma_lut[i][index] = (unsigned int)gamma_cal_cmd->lut[i];
		g_gamma_lut_checksum += gamma_lut[i][index];
		g_gamma_lut_checksum = g_gamma_lut_checksum & 0xffff;

        if ((i & 0xf) == 0)
            GAMMA_LOG("%4d lut:%4d src:%4d", i, gamma_lut[i][index], gamma_cal_cmd->lut[i]);
	}

	GAMMA_LOG("writeGammaLut1024 idx=%d checksum=%x", index, g_gamma_lut_checksum);
	return 0;
}


/*****************************************************************************
 *
 * write gamma gain to NvRAM
 *
 ****************************************************************************/
static void writeGammaGain(GAMMA_CAL_CMD_PARAMETER *gamma_cal_cmd)
{
    int index = gamma_cal_cmd->gainIndex;
    index = (index > 9) ? 9 : index;

    g_gamma_gain[index][0] = gamma_cal_cmd->R;
    g_gamma_gain[index][1] = gamma_cal_cmd->G;
    g_gamma_gain[index][2] = gamma_cal_cmd->B;

    GAMMA_LOG("write gain lut(%4d) r=%04x g=%04x b=%04x",
        index, g_gamma_gain[index][0], g_gamma_gain[index][1], g_gamma_gain[index][2]);
}

/*****************************************************************************
 *
 * debug function for dump gamma lut from NvRAM
 *
 ****************************************************************************/
static int dumpGammaLut()
{
    F_ID gamma_nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;
    int i = 0;
    FILE* fCfg = NULL;
    PQ_CUSTOM_LUT mPQ_CUSTOM_LUT;

    int ret = -1;

    gamma_nvram_fd = NVM_GetFileDesc(AP_CFG_RDCL_FILE_PQ_LID, &rec_size, &rec_num, ISWRITE);
    GAMMA_LOG("FD %d rec_size %d rec_num %d\n", gamma_nvram_fd.iFileDesc, rec_size, rec_num);

    if(rec_num != 1){
        GAMMA_LOG("Unexpected record num %d", rec_num);
        NVM_CloseFileDesc(gamma_nvram_fd);
        return -1;
    }

    if(rec_size != sizeof(PQ_CUSTOM_LUT)){
        GAMMA_LOG("Unexpected record size %d ap_nvram_btradio_struct %ud",
                  rec_size, sizeof(PQ_CUSTOM_LUT));
        NVM_CloseFileDesc(gamma_nvram_fd);
        return -1;
    }

    if(read(gamma_nvram_fd.iFileDesc, &mPQ_CUSTOM_LUT, rec_num*rec_size) < 0){
        GAMMA_LOG("Read NVRAM fails %d\n", errno);
        NVM_CloseFileDesc(gamma_nvram_fd);
        return -1;
    } else {
        ALOGD("id=%d", mPQ_CUSTOM_LUT.gamma_id);
        for (i=0; i<257; i++) {
            GAMMA_LOG("lutidx=%3d value=%d\n", i, mPQ_CUSTOM_LUT.gamma_lut[i]);
        }
        GAMMA_LOG("checksum=%d", mPQ_CUSTOM_LUT.gamma_checksum);
    }

    NVM_CloseFileDesc(gamma_nvram_fd);
    return 0;
}

/*****************************************************************************
 *
 * interpolation function for expend 0~255 to 0~512
 *
 ****************************************************************************/
static void inteporGammaLut(unsigned int gamma_lut_hw[DISP_GAMMA_LUT_SIZE][3],
unsigned int gamma_lut_src[1024][3])
{
    int i, j, intepor, interval, last;

    for(i = 0; i < 3; i++) {
        for(j = 0; j < 256; j++) {
            gamma_lut_hw[j*2][i] = gamma_lut_src[j][i];
        }
        for(j = 0; j < 255; j++) {
            gamma_lut_hw[j*2+1][i] = (gamma_lut_hw[j*2][i]+gamma_lut_hw[j*2+2][i]) / 2;
        }
        gamma_lut_hw[DISP_GAMMA_LUT_SIZE-1][i] = gamma_lut_hw[DISP_GAMMA_LUT_SIZE-2][i];
    }

    for(j = 0; j < DISP_GAMMA_LUT_SIZE; j += 1) {
#if 1
        GAMMA_LOG("lut hw(%3d) intepor (%4d) (%4d) (%4d)",
            j, gamma_lut_hw[j][0], gamma_lut_hw[j][1], gamma_lut_hw[j][2]);
#endif
    }
}


/*****************************************************************************
 *
 * down sample lut 512 to 257
 *
 ****************************************************************************/
static void convertToNvramGammaLut(unsigned int gamma_lut_nvram[257][3],
    unsigned int gamma_lut_src[1024][3])
{
    int i, colorIdx;

    for (colorIdx = 0; colorIdx < 3; colorIdx += 1) {
        for (i = 0; i < 256; i += 1) {
            gamma_lut_nvram[i][colorIdx] = gamma_lut_src[i*2][colorIdx];
        }
        gamma_lut_nvram[256][colorIdx] = gamma_lut_src[511][colorIdx];
    }
}

/*****************************************************************************
 *
 * write gamma lut to gamma register
 *
 ****************************************************************************/
static int writeGammaReg(unsigned int gamma_lut[1024][3])
{
    int i;
    int ret = -1;
	int mutex_value = 2;
    DISP_GAMMA_LUT_T *driverGamma = (DISP_GAMMA_LUT_T *)malloc(sizeof(DISP_GAMMA_LUT_T));

	ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &mutex_value);

    driverGamma->hw_id = DISP_GAMMA0;
    for (i = 0; i < DISP_GAMMA_LUT_SIZE; i++) {
        driverGamma->lut[i] = GAMMA_ENTRY(gamma_lut[i][0]>>2, gamma_lut[i][1]>>2, gamma_lut[i][2]>>2);
        //GAMMA_LOG("wr reg %4d RGB:%4d %4d %4d %08x", i,
        //    gamma_lut[i][0], gamma_lut[i][1], gamma_lut[i][2], driverGamma->lut[i]);
    }

    ret = ioctl(drvID, DISP_IOCTL_SET_GAMMALUT, driverGamma);

	mutex_value = 2;
	ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &mutex_value);

	GAMMA_LOG("write gamma register ret=%d", ret);

    free(driverGamma);

	ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &mutex_value);

    return ret;
}

/*****************************************************************************
 *
 * write gamma lut to NvRAM
 *
 ****************************************************************************/
static int writeGammaNVRAM(unsigned int gamma_lut[257][3])
{
    F_ID gamma_nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;
    int i = 0;
    FILE* fCfg = NULL;
    PQ_CUSTOM_LUT mPQ_CUSTOM_LUT;
    unsigned int rgbConf;
    unsigned int r, g, b, checksum;

    /* access NvRam */
    gamma_nvram_fd = NVM_GetFileDesc(AP_CFG_RDCL_FILE_PQ_LID, &rec_size, &rec_num, ISWRITE);
    GAMMA_LOG("FD %d rec_size %d rec_num %d\n", gamma_nvram_fd.iFileDesc, rec_size, rec_num);

    if(rec_num != 1){
        GAMMA_LOG("Unexpected record num %d", rec_num);
        NVM_CloseFileDesc(gamma_nvram_fd);
        return -1;
    }

    if(rec_size != sizeof(PQ_CUSTOM_LUT)){
        GAMMA_LOG("Unexpected record size %d ap_nvram_btradio_struct %ud",
                  rec_size, sizeof(PQ_CUSTOM_LUT));
        NVM_CloseFileDesc(gamma_nvram_fd);
        return -1;
    }

    if(read(gamma_nvram_fd.iFileDesc, &mPQ_CUSTOM_LUT, rec_num*rec_size) < 0){
        GAMMA_LOG("Read NVRAM fails %d\n", errno);
        NVM_CloseFileDesc(gamma_nvram_fd);
        return -1;
    } else {
        GAMMA_LOG("id=%d", mPQ_CUSTOM_LUT.gamma_id);
    }

    /* reset nvram lut */
    for (i = 0; i < 257; i++)
        mPQ_CUSTOM_LUT.gamma_lut[i] = 0x0;
    checksum = 0;

    /* set RGB value to lut */
    for (i = 0; i < 257; i++) {

        /* rounding to 10 bit */
        r = ((gamma_lut[i][0] & 0x2) != 0)? (gamma_lut[i][0]>>2)+1 : gamma_lut[i][0]>>2;
        g = ((gamma_lut[i][1] & 0x2) != 0)? (gamma_lut[i][1]>>2)+1 : gamma_lut[i][1]>>2;
        b = ((gamma_lut[i][2] & 0x2) != 0)? (gamma_lut[i][2]>>2)+1 : gamma_lut[i][2]>>2;

        /* avoid over flow */
        r = (r > 1023)? 1023 : r;
        g = (g > 1023)? 1023 : g;
        b = (b > 1023)? 1023 : b;

        /* checksum generation */
        checksum += r;
        checksum &= 0xffff;
        checksum += g;
        checksum &= 0xffff;
        checksum += b;
        checksum &= 0xffff;

        rgbConf = GAMMA_ENTRY(r, g, b);
        mPQ_CUSTOM_LUT.gamma_lut[i] = rgbConf;

		if ((i & 0xf) == 0 || (i == 511))
			GAMMA_LOG("wr nvram%4d RGB:%4d(%4d) %4d(%4d) %4d(%4d) %08x", i,
				gamma_lut[i][0], r,
				gamma_lut[i][1], g,
				gamma_lut[i][2], b,
				mPQ_CUSTOM_LUT.gamma_lut[i]);
    }

    mPQ_CUSTOM_LUT.gamma_checksum = checksum;
    GAMMA_LOG("checksum 10 = %d/%d", checksum);

    lseek(gamma_nvram_fd.iFileDesc, 0, 0);

    if(write(gamma_nvram_fd.iFileDesc, &mPQ_CUSTOM_LUT, rec_num*rec_size) < 0){
        GAMMA_LOG("write NVRAM fail\n");
    }
    NVM_CloseFileDesc(gamma_nvram_fd);

    return 0;

}

/*****************************************************************************
 *
 * read checksum api function
 *
 ****************************************************************************/
static unsigned int readCheckSum()
{
    GAMMA_LOG("%d(%x)", g_gamma_lut_checksum, g_gamma_lut_checksum);
    return g_gamma_lut_checksum;
}

static int execGammaCmd(GAMMA_CAL_CMD_PARAMETER *cmd_parameter)
{
    int ret = 0;

	if(drvID == -1)
		drvID = open("/dev/mtk_disp_mgr", O_RDONLY, 0);
	if(drvID == -1)
		drvID = open("/proc/mtk_mdp_cmdq", O_RDONLY, 0);
	if(drvID == -1)
		drvID = open("/proc/mtk_disp", O_RDONLY, 0);
	if(drvID == -1)
		drvID = open("/dev/mtk_disp", O_RDONLY, 0);

	if(drvID == -1)
		GAMMA_LOG("drvID = -1!!!");

    switch(cmd_parameter->cmd_type) {
        case GAMMA_PQ_ONOFF: /* on/off pq module */
            setPQOnOff(cmd_parameter);
            break;

        case GAMMA_SET_DISP_PATTERN:
            setAalInk(cmd_parameter);
            break;

        case GAMMA_SET_LUT_SIZE:
            setGammaLutSize(cmd_parameter);
            break;

        case GAMMA_WRITE_LUT:
            writeGammaLut(cmd_parameter, g_gamma_lut);
            break;

        case GAMMA_WRITE_LUT_FINISH:
            convertToNvramGammaLut(g_gamma_lut_nvram, g_gamma_lut);
            writeGammaNVRAM(g_gamma_lut_nvram);
            writeGammaReg(g_gamma_lut);
            break;

        case GAMMA_READ_CHECKSUM:
            break;

        case GAMMA_WRITE_GAIN:
            writeGammaGain(cmd_parameter);
            break;

        case GAMMA_DUMP_LUT:
            dumpGammaLut();
            break;

        default:
            break;
    }
    return ret;
}

static void acktoPc(GAMMA_REQ *req, GAMMA_CAL_CMD_PARAMETER *cmd_parameter)
{
	GAMMA_CNF cnf;
	memset(&cnf, 0, sizeof(GAMMA_CNF));
    cnf.header.token = req->header.token;
    cnf.header.id    = FT_GAMMA_CNF_ID;
    cnf.type         = req->type;
    cnf.status       = META_SUCCESS;
#if 1
    cnf.feedback.status_return_checksum.checksum = readCheckSum();
#else
    if (cmd_parameter->cmd_type == GAMMA_READ_CHECKSUM) {
        cnf.feedback.status_return_checksum.checksum = readCheckSum();
    } else {
        cnf.feedback.status_return_checksum.checksum = 0;
    }
#endif
	GAMMA_LOG("checksum=%x", cnf.feedback.status_return_checksum.checksum);
    WriteDataToPC(&cnf, sizeof(GAMMA_CNF), NULL, 0);
}

void META_GAMMA_OP(GAMMA_REQ *req, char *peer_buff, unsigned short peer_len)
{
    GAMMA_CAL_CMD_PARAMETER cmd_param;

    parseGammaCmd(req, &cmd_param, peer_buff, peer_len);

    execGammaCmd(&cmd_param);

    acktoPc(req, &cmd_param);
}

