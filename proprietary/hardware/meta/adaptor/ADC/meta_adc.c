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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <log/log.h>
#include <unistd.h>
#include "meta_adc.h"
#include "errno.h"

#ifdef MTK_GM_30
#define SUPPORT_RW_NVRAM
#endif

/* read NVRAM */
#ifdef SUPPORT_RW_NVRAM
#include "../../../../external/nvram/libnvram/libnvram.h"
#include "CFG_FG_File.h"
#include "CFG_FG_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"
#endif

#define TEST_PMIC_PRINT 	_IO('k', 0)
#define PMIC_READ 				_IOW('k', 1, int)
#define PMIC_WRITE 				_IOW('k', 2, int)
#define SET_PMIC_LCDBK 		_IOW('k', 3, int)
#define ADC_CHANNEL_READ 	_IOW('k', 4, int)
#define GET_CHANNEL_NUM    	_IOW('k', 5, int)
#define ADC_EFUSE_ENABLE    _IOW('k', 6, int)
//add for meta tool-----------------------------------------
#define Get_META_BAT_VOL _IOW('k', 10, int) 
#define Get_META_BAT_SOC _IOW('k', 11, int) 
#define Get_META_BAT_CAR_TUNE_VALUE _IOW('k', 12, int)
#define Set_META_BAT_CAR_TUNE_VALUE _IOW('k', 13, int)
//add for meta tool-----------------------------------------

#ifdef SUPPORT_RW_NVRAM
static  ap_nvram_fg_config_struct stFGReadback, stFGWriteback;
#endif
int meta_adc_fd=0;

/*Input : ChannelNUm, Counts*/ 
/*Output : Sum, Result (0:success, 1:failed)*/
int adc_in_data[2] = {1,1}; 

int adc_out_data[2] = {1,1}; 

#ifdef SUPPORT_RW_NVRAM
static int read_NVRAM()
{
	// int fg_nvram_fd = 0;
	F_ID fg_nvram_fd;
	int rec_size;
	int rec_num;
	bool IsRead = true;

	memset(&stFGReadback, 0, sizeof(stFGReadback));

	fg_nvram_fd = NVM_GetFileDesc(AP_CFG_CUSTOM_FILE_FUEL_GAUGE_LID, &rec_size, &rec_num, IsRead);
	printf("read NVRAM, rec_size %d, rec_num %d\n", rec_size, rec_num);
	if (fg_nvram_fd.iFileDesc > 0) {   /* >0 means ok */
		if (read(fg_nvram_fd.iFileDesc, &stFGReadback , rec_size*rec_num) < 0)
			printf("read NVRAM error, %s\n", strerror(errno));
		NVM_CloseFileDesc(fg_nvram_fd);

		if (strlen(stFGReadback.dsp_dev) != 0) {
			printf("[NVRAM Area] FG NVRam size:%d, number:%d, [Read]\n", rec_size, rec_num);
			printf("[NVRAM Item] dsp_dev : %s\n", stFGReadback.dsp_dev);
			printf("[NVRAM Item] nvram_car_tune_value : %d\n", stFGReadback.nvram_car_tune_value);
		} else {
			printf("FG NVRam mnl_config.dev_dsp == NULL \n");
			return -1;
		}
	} else {
		printf("FG read NVRam fg_nvram_fd == %d \n", fg_nvram_fd.iFileDesc);
		return -1;
	}
	return 0;
}

static int write_NVRAM()
{
	// int fg_nvram_fd = 0;
	F_ID fg_nvram_fd;
	int rec_size;
	int rec_num;
	bool IsRead = false;

	fg_nvram_fd = NVM_GetFileDesc(AP_CFG_CUSTOM_FILE_FUEL_GAUGE_LID, &rec_size, &rec_num, IsRead);
	printf("write NVRAM, rec_size %d, rec_num %d\n", rec_size, rec_num);
	if (fg_nvram_fd.iFileDesc > 0) {   /* >0 means ok */
		if (write(fg_nvram_fd.iFileDesc, &stFGWriteback , rec_size*rec_num) < 0)
			printf("write NVRAM error, %s\n", strerror(errno));
		NVM_CloseFileDesc(fg_nvram_fd);

		if (strlen(stFGWriteback.dsp_dev) != 0) {
			printf("[NVRAM Area] FG NVRam size:%d, number:%d, [Write]\n", rec_size, rec_num);
			printf("[NVRAM Item] dsp_dev : %s\n", stFGWriteback.dsp_dev);
			printf("[NVRAM Item] nvram_car_tune_value : %d\n", stFGWriteback.nvram_car_tune_value);
		} else {
			printf("FG NVRam mnl_config.dev_dsp == NULL \n");
			return -1;
		}
	} else {
		printf("FG write NVRam fg_nvram_fd == %d \n", fg_nvram_fd.iFileDesc);
		return -1;
	}
	return 0;
}
#endif

void Meta_ADC_OP(ADC_REQ *req, char *peer_buff, unsigned short peer_len)
{
	ADC_CNF ADCMetaReturn;
	int ret;
	unsigned short fix_warning;
	char *fix_warning2;
#ifdef SUPPORT_RW_NVRAM
	int ret3;
#endif
	fix_warning2 = peer_buff;
	fix_warning = peer_len;
	memset(&ADCMetaReturn, 0, sizeof(ADCMetaReturn));

	ADCMetaReturn.header.id=req->header.id+1;
	ADCMetaReturn.header.token=req->header.token;
	ADCMetaReturn.status=META_SUCCESS;
	ADCMetaReturn.type=req->type;

	//add for meta tool-----------------------------------------
	meta_adc_fd = open("/dev/MT_pmic_adc_cali",O_RDWR, 0);
	if (meta_adc_fd == -1) {
		printf("Open /dev/MT_pmic_adc_cali : ERROR \n");
		ALOGD("Open /dev/MT_pmic_adc_cali : ERROR \n");
		ADCMetaReturn.status = META_FAILED;
		goto ADC_Finish;
	}
	
	adc_in_data[0] = req->type;
	if(adc_in_data[0] == ADC_OP_BAT_VOL) {
		ret = ioctl(meta_adc_fd, Get_META_BAT_VOL, adc_out_data);
		if (ret == -1)
		{
			ADCMetaReturn.status = META_FAILED;
		}
		else
		{
			ADCMetaReturn.cnf.m_bat_vol.vol = (int)adc_out_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}
	}
	else if(adc_in_data[0] == ADC_OP_BAT_CAPACITY){
		ret = ioctl(meta_adc_fd, Get_META_BAT_SOC, adc_out_data);
		if (ret == -1)
		{
			ADCMetaReturn.status = META_FAILED;
		}
		else
		{
			ADCMetaReturn.cnf.m_bat_capacity.capacity = (int)adc_out_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}

	}
	else if(adc_in_data[0] == ADC_OP_GET_CAR_TUNE_VALUE){
#ifdef SUPPORT_RW_NVRAM
		memset(&stFGReadback, 0, sizeof(stFGReadback));
		ret = read_NVRAM();
		adc_out_data[0] = stFGReadback.nvram_car_tune_value;
		printf("Read [%d] from nvram_car_tune_value\n", (int)adc_out_data[0]);
#else
		ret = ioctl(meta_adc_fd, Get_META_BAT_CAR_TUNE_VALUE, adc_out_data);
#endif
		if (ret == -1)
		{
			ADCMetaReturn.status = META_FAILED;
		}
		else
		{
			ADCMetaReturn.cnf.m_car_tune_value.car_tune_value = (int)adc_out_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}

	}
	else if(adc_in_data[0] == ADC_OP_SET_CAR_TUNE_VALUE){
		adc_out_data[0] = 0;
		adc_out_data[1] = req->req.dummy;
		ret = ioctl(meta_adc_fd, Set_META_BAT_CAR_TUNE_VALUE, adc_out_data);
		if (ret == -1)
		{
			ADCMetaReturn.status = META_FAILED;
		}
		else
		{
			ADCMetaReturn.cnf.m_car_tune_value.car_tune_value = (int)adc_out_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}

#ifdef SUPPORT_RW_NVRAM
			printf("Write [%d] to nvram_car_tune_value\n", (int)adc_out_data[0]);
			memset(&stFGWriteback, 0, sizeof(stFGWriteback));
			strncpy(stFGWriteback.dsp_dev, "/dev/stpfgd", 12);
			stFGWriteback.nvram_car_tune_value = (int)adc_out_data[0];
			ret3 = write_NVRAM();

			if (ret3 != 0) {
				printf(" nvram ioctl check 444 \n");
				ADCMetaReturn.status = META_FAILED;
			} else {
				printf(" nvram ioctl check 555 \n");
				ADCMetaReturn.status = META_SUCCESS;
			}
#endif
	}

	printf("Meta_ADC_OP : ADC_OP = %d, %d, %d\n", req->type, adc_out_data[0], adc_out_data[1]);	
	close(meta_adc_fd);

ADC_Finish:

	if (false == WriteDataToPC(&ADCMetaReturn,sizeof(ADCMetaReturn),NULL,0)) {
        printf("Meta_ADC_OP : WriteDataToPC() fail 2\n");
		ALOGD("Meta_ADC_OP : WriteDataToPC() fail 2\n");
    }

	printf("Meta_ADC_OP : Finish !\n");
	ALOGD("Meta_ADC_OP : Finish !\n");
}

int Meta_AUXADC_Init(void)
{
	return 1;
}

int Meta_AUXADC_Deinit(void)
{
	return 1;
}

