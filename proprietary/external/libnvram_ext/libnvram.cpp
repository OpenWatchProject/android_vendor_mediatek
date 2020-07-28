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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   Nvram_lib.c
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *   Stream interface implementation for NVM Driver.
 *
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183)
 *
 ****************************************************************************/

#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
//#include "CFG_file_lid.h"
#include <dirent.h>
#include<sys/mount.h>
//#include <fs_mgr.h>
#include <fstab.h>
#include <cutils/properties.h>
#include "libnvram_log.h"
#include "libnvram.h"
#include "libfile_op.h"
//#include "../../../../kernel/include/mtd/mtd-abi.h"
#include <mtd/mtd-abi.h>

#include "android_filesystem_config.h"
#include <pthread.h>
//add for log
#include <time.h>
//add for log
#define PREFIX_IMEI  "MP0B_"   
#define PREFIX_BARCODE  "MP09_"  
#define MAXLINE   1024
#define MaxFileNum   936
#define INVALID_HANDLE_VALUE    -1
#include "libnvram_sec.h"

extern const char* __progname;

typedef struct {
	unsigned int ulCheckSum;
	unsigned int iCommonFileNum;
	unsigned int iCustomFileNum;
} BackupFileInfo;


extern int iCustomBeginLID;
extern int iFileVerInfoLID;
static const char *g_pcNVM            = "/mnt/vendor/nvdata";
static const char *g_pcNVM_BT         = "/mnt/vendor/nvdata/APCFG/APRDEB/BT_Addr";
static const char *g_pcNVM_AllFile    = "/mnt/vendor/nvdata/AllFile";
static const char *g_pcNVM_AllMap     = "/mnt/vendor/nvdata/AllMap";
static const char *g_pcNVM_Flag       =
    "/mnt/vendor/nvdata/RestoreFlag";//The File Will be Created after restore

#ifdef MTK_NVRAM_AUTO_BACKUP
static const char *g_pcNVM_BackFlag   =
    "/mnt/vendor/nvdata/BackupFlag";//The file will be updated after modifying important data
#endif

#if 0
static const char *g_pcNVM_APCalFile  = "/mnt/vendor/nvdata/APCFG/APRDCL";
static const char *g_pcNVM_APRdebFile = "/mnt/vendor/nvdata/APCFG/APRDEB";
#endif
static const char *g_pcNVM_MD         = "/mnt/vendor/nvdata/md/NVRAM";
static const char *g_pcNVM_MDCalFile  = "/mnt/vendor/nvdata/md/NVRAM/CALIBRAT";
static const char *g_pcNVM_MDRdebFile = "/mnt/vendor/nvdata/md/NVRAM/NVD_IMEI";
#if 0
static const char *g_pcNVM_MDImptFile = "/mnt/vendor/nvdata/md/NVRAM/IMPORTNT";
static const char *g_pcNVM_MDDataFile = "/mnt/vendor/nvdata/md/NVRAM/NVD_DATA";
static const char *g_pcNVM_MDCoreFile = "/mnt/vendor/nvdata/md/NVRAM/NVD_CORE";
//Add for second modem for MT658*
static const char *g_pcNVM_MD2CalFile  = "/mnt/vendor/nvdata/md2/NVRAM/CALIBRAT";
static const char *g_pcNVM_MD2RdebFile = "/mnt/vendor/nvdata/md2/NVRAM/NVD_IMEI";
static const char *g_pcNVM_MD2ImptFile = "/mnt/vendor/nvdata/md2/NVRAM/IMPORTNT";
//End of Comment

/*Add for C2K modem*/
static const char *g_pcNVM_MD3CalFile  = "/mnt/vendor/nvdata/md3/NVRAM/CALIBRAT";
static const char *g_pcNVM_MD3RdebFile = "/mnt/vendor/nvdata/md3/NVRAM/NVD_IMEI";
static const char *g_pcNVM_MD3ImptFile = "/mnt/vendor/nvdata/md3/NVRAM/IMPORTNT";


//LTE support
static const char *g_pcNVM_MD5CalFile  = "/mnt/vendor/nvdata/md5/NVRAM/CALIBRAT";
static const char *g_pcNVM_MD5RdebFile = "/mnt/vendor/nvdata/md5/NVRAM/NVD_IMEI";
static const char *g_pcNVM_MD5ImptFile = "/mnt/vendor/nvdata/md5/NVRAM/IMPORTNT";
static const char *g_pcNVM_Via = "/mnt/vendor/nvdata/md_via";


static const char *g_pcBKP_APCalFile  = "/backup/APCFG/APRDCL";
static const char *g_pcBKP_APRdebFile = "/backup/APCFG/APRDEB";
static const char *g_pcBKP_MDCalFile  = "/backup/md/NVRAM/CALIBRAT";
static const char *g_pcBKP_MDRdebFile = "/backup/md/NVRAM/NVD_IMEI";
static const char *g_pcBKP_MDImptFile = "/backup/md/NVRAM/IMPORTNT";
static const char *g_pcBKP_MDDataFile = "/backup/md/NVRAM/NVD_DATA";
static const char *g_pcBKP_MDCoreFile = "/backup/md/NVRAM/NVD_CORE";
static const char *g_pcNVM_ModomBackNum =
    "/mnt/vendor/nvdata/md/NVRAM/IMPORTNT/MP0D_000";////The file will be updated after modifying MD important data
#endif

// partition device path
char g_LOG_DEVICE[128] = {0};
char g_MISC_DEVICE[128] = {0};
char g_NVRAM_BACKUP_DEVICE[128] = {0};
char g_NVRAM_PROINFO_DEVICE[128] = {0};
char g_NVRAM_NVDATA_DEVICE[128] = {0};

unsigned int gFileStartAddr = 0;
pthread_mutex_t gFileStartAddrlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t generateFileVerlock = PTHREAD_MUTEX_INITIALIZER;
#define BitmapFlag 0xAABBCCDD
#define MAX_FILENUM 32767
typedef struct {
	char	cFileName[128];
	unsigned int iLID;
} FileName;

extern const TCFG_FILE g_akCFG_File[];
extern const int g_i4CFG_File_Count;
extern const TCFG_FILE g_akCFG_File_Custom[];
extern const int g_i4CFG_File_Custom_Count;
extern int iCustomBeginLID;
extern int iFileVerInfoLID;
extern int iNvRamFileMaxLID;
extern pfConvertFunc aNvRamConvertFuncTable[];
volatile int g_i4MaxNvRamLid = 0;
extern FileName aBackupToBinRegion[];
extern const unsigned int g_Backup_File_Count;
//#define forDMBlockNum (2)
const char *strVerInfo = "NVRAM_VER_INFO";
pthread_mutex_t dirlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t backuplock = PTHREAD_MUTEX_INITIALIZER;
int  NvRamBlockNum = 0;

int nvram_platform_log_block = 8;
int nvram_platform_resv_block = 8;
int nvram_platform_DM_block = 2;
int nvram_platform_layout_version = 0;
int nvram_platform_header_offset = 0;
int nvram_init_flag = 0;
int nvram_gpt_flag = 0;
int BinRegionBlockTotalNum = 0;

extern const char *nvram_new_partition_name;
extern const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[];
extern const unsigned int g_new_nvram_lid_count;
extern int nvram_misc_log_block_offset;

using android::fs_mgr::Fstab;
using android::fs_mgr::GetEntryForMountPoint;
using android::fs_mgr::ReadDefaultFstab;

static int get_partition_path(void) {
	Fstab fstab;
	if (nvram_emmc_support() || nvram_ufs_support()) {
		if (!ReadDefaultFstab(&fstab)) {
			NVRAM_LOG("failed to read fstab \n");
			return -1;
		}

		auto rec = GetEntryForMountPoint(&fstab, NVRAM_MNT_POINT);
		if (rec == nullptr) {
			NVRAM_LOG("failed to get nvram path %s \n", NVRAM_MNT_POINT);
		} else {
			memset(g_LOG_DEVICE, 0, sizeof(g_LOG_DEVICE));
			memset(g_NVRAM_BACKUP_DEVICE, 0, sizeof(g_NVRAM_BACKUP_DEVICE));
			snprintf(g_LOG_DEVICE, sizeof(g_LOG_DEVICE),"%s", rec->blk_device.c_str());
			snprintf(g_NVRAM_BACKUP_DEVICE, sizeof(g_NVRAM_BACKUP_DEVICE),"%s", rec->blk_device.c_str());
		}

		rec = GetEntryForMountPoint(&fstab, NVRAM_MNT_POINT);
		if (rec == nullptr) {
			NVRAM_LOG("failed to get misc path %s \n", MISC_MNT_POINT);
		} else {
			memset(g_MISC_DEVICE, 0, sizeof g_MISC_DEVICE);
		    snprintf(g_MISC_DEVICE, sizeof(g_MISC_DEVICE),"%s", rec->blk_device.c_str());
		}

		rec = GetEntryForMountPoint(&fstab, NVDATA_MNT_POINT);
		if (rec == nullptr) {
			NVRAM_LOG("failed to get nvdata path %s \n", NVDATA_MNT_POINT);
		} else {
			memset(g_NVRAM_NVDATA_DEVICE, 0, sizeof g_NVRAM_NVDATA_DEVICE);
		    snprintf(g_NVRAM_NVDATA_DEVICE, sizeof(g_NVRAM_NVDATA_DEVICE),"%s", rec->blk_device.c_str());
		}

		rec = GetEntryForMountPoint(&fstab, PROINFO_MNT_POINT);
		if (rec == nullptr) {
			NVRAM_LOG("failed to get proinfo path %s \n", PROINFO_MNT_POINT);
		} else {
			memset(g_NVRAM_PROINFO_DEVICE, 0, sizeof g_NVRAM_PROINFO_DEVICE);
		    snprintf(g_NVRAM_PROINFO_DEVICE, sizeof(g_NVRAM_PROINFO_DEVICE),"%s", rec->blk_device.c_str());
		}
	} else {
		memset(g_LOG_DEVICE, 0, sizeof(g_LOG_DEVICE));
		memset(g_NVRAM_BACKUP_DEVICE, 0, sizeof(g_NVRAM_BACKUP_DEVICE));
		sprintf(g_LOG_DEVICE, "/dev/nvram");
		sprintf(g_NVRAM_BACKUP_DEVICE, "/dev/nvram");
		memset(g_MISC_DEVICE, 0, sizeof g_MISC_DEVICE);
		sprintf(g_MISC_DEVICE, "/dev/misc");
		memset(g_NVRAM_PROINFO_DEVICE, 0, sizeof g_NVRAM_PROINFO_DEVICE);
		sprintf(g_NVRAM_PROINFO_DEVICE, "/dev/pro_info");
	}

	return 0;
}


int open_file_with_dirs(const char *fn, int flag, mode_t mode) {
	char tmp[PATH_MAX];
	int i = 0;
	int filedesc = 0;
	struct stat info;
	int val = 0;
	umask(000);
	pthread_mutex_lock(&dirlock);
	while (*fn) {
		tmp[i] = *fn;

		if (*fn == '/' && i) {
			tmp[i] = '\0';
			if (access(tmp, F_OK) != 0) {
				if (mkdir(tmp, 0771) == -1) {
					NVRAM_LOG("mkdir error! %s\n", (char*)strerror(errno));
					if (errno != EEXIST) {
						pthread_mutex_unlock(&dirlock);
						return -1;
					}
				}
#if 1
				val = stat(tmp, &info);
				if (val == 0) {
					//maybe meta tool operate these files
					if (strstr(tmp, "/media")) {
						if (-1 == chown(tmp, -1, AID_AUDIO))
							NVRAM_LOG("change dir group owner failed(media):%s\n", (char*)strerror(errno));
					} else if (strstr(tmp, "BT_Addr")) {
						if (-1 == chown(tmp, -1, AID_BLUETOOTH))
							NVRAM_LOG("change dir group owner failed(bt):%s\n", (char*)strerror(errno));
					} else {
						if (-1 == chown(tmp, -1, AID_SYSTEM))
							NVRAM_LOG("change dir group owner failed:%s\n", (char*)strerror(errno));
					}
				}
#endif
			}
			tmp[i] = '/';
		}
		i++;
		fn++;
	}
	tmp[i] = '\0';
	filedesc = open(tmp, flag, mode);
	if (-1 != filedesc) {
		val = stat(tmp, &info);
#if 1
		if (val == 0) {
			if (strstr(tmp, "/media")) {
				if (-1 == chown(tmp, -1, AID_AUDIO))
					NVRAM_LOG("change file group owner failed(media):%s\n", (char*)strerror(errno));
			} else if (strstr(tmp, "BT_Addr")) {
				if (-1 == chown(tmp, -1, AID_BLUETOOTH))
					NVRAM_LOG("change file group owner failed(bt):%s\n", (char*)strerror(errno));
			} else {
				if (-1 == chown(tmp, -1, AID_SYSTEM))
					NVRAM_LOG("change file group owner failed:%s\n", (char*)strerror(errno));
			}
			if (!S_ISDIR(info.st_mode)) {
				if (-1 == chmod(tmp, mode))
					NVRAM_LOG("chmod file failed: %s\n", (char*)strerror(errno));
			}
		}
#endif
	}
	pthread_mutex_unlock(&dirlock);
	return filedesc;
}

bool NVM_GetDeviceInfo(const char *path, struct mtd_info_user *device_info) {
	int fd;
	int iRet;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("NVM_GetDeviceInfo : open  %s fail!!! %s\n", path,
		          (char*)strerror(errno));
		return false;
	}
	if (nvram_gpt_flag == 1) {
		device_info->type = MTD_NANDFLASH;
		device_info->flags = MTD_WRITEABLE;
		device_info->size = lseek(fd, 0, SEEK_END);
		device_info->erasesize = 128 * 1024;
		device_info->writesize = 512;
		device_info->oobsize = 0;
	} else {
		iRet = ioctl(fd, MEMGETINFO, device_info);
		if (iRet < 0) {
			NVRAM_LOG("NVM_GetDeviceInfo : dumchar ioctl fail %s\n",
			          (char*)strerror(errno));
			close(fd);
			return false;
		}
	}
	close(fd);

	return true;
}

bool NVM_EraseDeviceBlock(const char *path, struct erase_info_user erase_info) {
	int fd;
	int iRet;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("NVM_EraseDeviceBlock: dumchar open fail!!! %s\n",
		          (char*)strerror(errno));
		return false;
	} else {
		iRet = ioctl(fd, MEMERASE, &erase_info);
		if (iRet < 0) {
			NVRAM_LOG("NVM_EraseDeviceBlock: dumchar erase fail! %s\n",
			          (char*)strerror(errno));
			close(fd);
			return false;
		}
	}
	close(fd);

	return true;
}


//iModifiedFileNum:the num of modified file
//iSrc:0,from modem;1:from AP
bool NVM_CheckBackFlag(int iFileDesc) {
	unsigned int index = 0;
	char cChecksum = 0;
	char cBuf = 0;
	bool flag = true;
	int iResult = 0;
	char savedchecksum = 0;
	struct stat buf;
	memset(&buf, 0, sizeof(struct stat));
	if (-1 == iFileDesc) {
		NVRAM_LOG("NVM_CheckBackFlag:file desc is wrong(ignore)\n");
		return false;
	}
	if (-1 == fstat(iFileDesc, &buf)) {
		NVRAM_LOG("NVM_CheckBackFlag:Err fstat(checksum)(ignore)\n");
		return false;
	}
	if (buf.st_size != sizeof(unsigned int) + sizeof(char)) {
		NVRAM_LOG("NVM_CheckBackFlag:file size is wrong\n");
		return false;
	}
	for (index = 0; index < sizeof(unsigned int); ++index) {
		if (sizeof(char) == read(iFileDesc, &cBuf, sizeof(char))) {
			if (flag) {
				cChecksum ^= cBuf;
				flag = false;
			} else {
				cChecksum += cBuf;
				flag = true;
			}
		} else {
			NVRAM_LOG("NVM_CheckBackFlag:Err read Fail(checksum)(ignore):%d\n", iResult);
			return false;
		}
	}
	iResult = read(iFileDesc, &savedchecksum, sizeof(char));
	if (iResult != sizeof(char)) {
		NVRAM_LOG("[NVRAM]:can not read BackupFlag check sum:%d\n", iResult);
		return false;
	}
	if (cChecksum == savedchecksum)
		return true;
	else
		return false;
}
bool NVM_CheckMDBackFlag(int iFileDesc ) {
	unsigned int index = 0;
	unsigned short cChecksum = 0;
	unsigned char *byte_checksum = (unsigned char *)&cChecksum;
	char cBuf = 0;
	int iResult = 0;
	unsigned short savedchecksum = 0;
	struct stat buf;
	memset(&buf, 0, sizeof(struct stat));

	if (-1 == iFileDesc) {
		NVRAM_LOG("NVM_CheckMDBackFlag:file desc is wrong(ignore)\n");
		return false;
	}
	if (-1 == fstat(iFileDesc, &buf)) {
		NVRAM_LOG("NVM_CheckMDBackFlag:Err fstat(checksum)(ignore)\n");
		return false;
	}
	if (buf.st_size != 4 * sizeof(char)) {
		NVRAM_LOG("NVM_CheckMDBackFlag:file size is wrong\n");
		return false;
	}
	for (index = 0; index < 2 * sizeof(char); ++index) {
		if (sizeof(char) == read(iFileDesc, &cBuf, sizeof(char))) {
			if (index & 0x1)
				*(byte_checksum + 1) += cBuf;
			else
				*(byte_checksum) += cBuf;
		} else {
			NVRAM_LOG("NVM_CheckMDBackFlag:Err read Fail(checksum)(ignore):%d\n", iResult);
			return false;
		}
	}
	iResult = read(iFileDesc, &savedchecksum, sizeof(unsigned short));
	if (iResult != sizeof(unsigned short)) {
		NVBAK_LOG("[NVRAM]:can not read MDBackupFile check sum:%d\n", iResult);
		return false;
	}
	if (cChecksum == savedchecksum)
		return true;
	else if (savedchecksum == 0xCDEF && cChecksum == 0x0)
		return true;
	else
		return false;
}

bool NVM_ComputeBackflagCheckSum(int iFileDesc) {
	char buf = 0;
	char cChecksum = 0;
	unsigned int index = 0;
	bool flag = true;
	int iResult = 0;
	if (iFileDesc == -1) {
		NVRAM_LOG("NVM_ComputeBackflagCheckSum:File desc is wrong\n");
		return false;
	}
	if (-1 == lseek(iFileDesc, 0, SEEK_SET)) {
		NVRAM_LOG("NVM_ComputeBackflagCheckSum:(check sum)lseek error\n");
		return false;
	}
	for (index = 0; index < sizeof(unsigned int); ++index) {
		if (sizeof(char) == read(iFileDesc, &buf, sizeof(char))) {
			if (flag) {
				cChecksum ^= buf;
				flag = false;
			} else {
				cChecksum += buf;
				flag = true;
			}
		} else {
			NVRAM_LOG("NVM_ComputeBackflagCheckSum:Err read Fail(checksum)(ignore):%d\n",
			          iResult);
			return false;
		}
	}
	iResult = write(iFileDesc, &cChecksum, sizeof(char));
	if (iResult != sizeof(char)) {
		NVRAM_LOG("write BackupFlag File check sum Fail:%d\n", iResult);
		return false;
	}
	return true;

}
bool NVM_GetBackupFileNum(unsigned int * iAPBackupFileNum,
                          unsigned short *iMDBackupFileNum)

{
	int fd, iResult;
	char cMtdDevName[128] = {0};
	char *tempBuffer = NULL;
	char *tempBitmap1 = NULL;
	char *tempBitmap2 = NULL;
	int i, pos = 0, iBlockSize, iBlockNum, flag = 0;
	bool bSuccessFound = false;
	struct mtd_info_user info;
	unsigned int iMapFileSize;
	bool bRet;
//   int iBackup_Partition=get_partition_numb("nvram");


	NVRAM_LOG("Get BackupFileNum:\n");
	if (iAPBackupFileNum == NULL || iMDBackupFileNum == NULL) {
		NVRAM_LOG("[NVRAM]:the pointer of iAPBackupFileNum or  iMDBackupFileNum is error\r\n");
		return false;
	}
	memset(cMtdDevName, 0, sizeof cMtdDevName);
	//strcpy(cMtdDevName,g_NVRAM_BACKUP_DEVICE);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);
	NVRAM_LOG("[NVRAM]:cMtdDevName:%s\n", cMtdDevName);


	bRet = NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet) {
		NVRAM_LOG("[NVRAM]:BackupFileNum: get device info fail!!!\n");
		return false;
	}

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("[NVRAM]:mtd open error %s\r\n", (char*)strerror(errno));
		return false;
	}


	iBlockSize = info.erasesize;
	//iPartitionSize=info.size;
	if ( nvram_platform_layout_version == 0)
		BinRegionBlockTotalNum = info.size / iBlockSize - nvram_platform_log_block -
		                         nvram_platform_resv_block;
	else if ( nvram_platform_layout_version == 1)
		BinRegionBlockTotalNum = info.size / iBlockSize ;
	else {
		NVRAM_LOG("[NVRAM]: invalid nvram layout version %d\r\n",
		          nvram_platform_layout_version);
		close(fd);
		return false;
	}
	NvRamBlockNum = BinRegionBlockTotalNum - nvram_platform_DM_block;

	tempBuffer = (char*)malloc(iBlockSize);
	if (tempBuffer == NULL) {
		NVRAM_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		return false;
	}
	iBlockNum = NvRamBlockNum;
	NVRAM_LOG("[NVRAM]:iBlockNum:%d\n", iBlockNum);

	tempBitmap1 = (char *)malloc(NvRamBlockNum);
	if (tempBitmap1 == NULL) {
		NVRAM_LOG("[NVRAM]:malloc tempBitmap1 Fail!!\r\n");
		free(tempBuffer);
		close(fd);
		return false;
	}
	tempBitmap2 = (char *)malloc(NvRamBlockNum);
	if (tempBitmap2 == NULL) {
		NVRAM_LOG("[NVRAM]:malloc tempBitmap2 Fail!!\r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		return false;
	}

	while (iBlockNum > 0) {
		flag = 0;
		iBlockNum--;
		NVRAM_LOG("[NVRAM]:iBlockNum:%d\n", iBlockNum);
		lseek(fd, iBlockNum * iBlockSize, SEEK_SET);
		iResult = read(fd, tempBuffer, iBlockSize);
		NVRAM_LOG("[NVRAM]:read:%d\n", iResult);
		if (iResult <= 0) {
			NVRAM_LOG("[NVRAM]:read size error\r\n");
			close(fd);
			free(tempBuffer);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}
		memcpy(tempBitmap1, tempBuffer, NvRamBlockNum * sizeof(char));
		memcpy(tempBitmap2, tempBuffer + NvRamBlockNum * sizeof(char),
		       NvRamBlockNum * sizeof(char));
		for (i = 0; i < NvRamBlockNum; i++) {
			if (tempBitmap1[i] != tempBitmap2[i]) {
				NVRAM_LOG("[NVRAM]:1i:%d,1:%d,2:%d\n", i, tempBitmap1[i], tempBitmap2[i]);
				flag = 1;
				break;
			}
		}
		if (flag)
			continue;
		memcpy(tempBitmap2, tempBuffer + 2 * NvRamBlockNum * sizeof(char),
		       NvRamBlockNum * sizeof(char));
		for (i = 0; i < NvRamBlockNum; i++) {
			if (tempBitmap1[i] != tempBitmap2[i]) {
				NVRAM_LOG("[NVRAM]:2i:%d,1:%d,2:%d\n", i, tempBitmap1[i], tempBitmap2[i]);
				flag = 1;
				break;
			}
		}
		if (flag)
			continue;
		bSuccessFound = true;
		break;
	}
	if (!bSuccessFound) {
		NVRAM_LOG("[NVRAM]:can not find bad block bit map\r\n");
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	#if 0
	for (i = 0; i < iBlockNum; i++) {
		NVRAM_LOG("[NVRAM_Bitmap]:Block%d,%d\n", i, tempBitmap1[i]);
	}
	#endif
	for (i = 0; i < iBlockNum; i++) {
		if (tempBitmap1[i] == 0) {
			pos = i;
			break;
		}
	}
	if (i == iBlockNum) {
		NVRAM_LOG("[NVRAM]:can not find map file\r\n");
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	iResult = lseek(fd, pos * iBlockSize, SEEK_SET);
	if (iResult != (pos * iBlockSize)) {
		NVRAM_LOG("[NVRAM]:mtd lseek error\r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	iResult = read(fd, &iMapFileSize, sizeof(unsigned int));
	if (iResult != sizeof(unsigned int)) {
		NVRAM_LOG("[NVRAM]:mtd read error\r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	if (iMapFileSize == 0) {
		NVRAM_LOG("[NVRAM]:MapFile size is zero, BinRegion is empty\r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	iResult = lseek(fd, pos * iBlockSize + 3 * sizeof(unsigned int) + iMapFileSize,
	                SEEK_SET);

	iResult = read(fd, iAPBackupFileNum, sizeof(unsigned int));
	if (iResult != sizeof(unsigned int)) {
		NVRAM_LOG("[NVRAM]:mtd read error(AP)\r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}

	iResult = read(fd, iMDBackupFileNum, sizeof(unsigned short));
	if (iResult != sizeof(unsigned short)) {
		NVRAM_LOG("[NVRAM]:mtd read error(MD)r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	NVRAM_LOG("[NVRAM]:Saved AP Backup File Num:%d,Saved MD Backup File Num:%d\n",
	          *iAPBackupFileNum, *iMDBackupFileNum);
	NVRAM_LOG("[NVRAM]:mtd Get Backup File Num success\r\n");
	close(fd);
	free(tempBuffer);
	free(tempBitmap1);
	free(tempBitmap2);
	return true;

}

bool NVM_AddBackupNum(unsigned int iModifiedFileNum, int iSrc) {
	(void)(iModifiedFileNum);
	(void)(iSrc);
#ifdef MTK_NVRAM_AUTO_BACKUP
	int iFileDesc, iResult;
	unsigned int iSavedFileNum = 0;
	unsigned short iMDBackupFileNum = 0;

	if (iModifiedFileNum <= 0) {
		NVRAM_LOG("Err iModifiedFileNum:%d\n", iModifiedFileNum);
		return false;
	}
	if (iSrc == 0)
		NVRAM_LOG("iModifiedFileNum:%d,from modem\n", iModifiedFileNum);
	else
		NVRAM_LOG("iModifiedFileNum:%d,from AP\n", iModifiedFileNum);

	pthread_mutex_lock(&backuplock);
	iFileDesc = open(g_pcNVM_BackFlag, O_RDWR);
	if (-1 == iFileDesc) {
		iFileDesc = open_file_with_dirs(g_pcNVM_BackFlag, O_CREAT | O_RDWR,
		                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if (-1 == iFileDesc) {
			NVRAM_LOG("Err Create BackupFlag File Fail\n");
			pthread_mutex_unlock(&backuplock);
			return false;
		}
	} else {
		if (NVM_CheckBackFlag(iFileDesc)) {
			if (-1 == lseek(iFileDesc, 0, SEEK_SET)) {
				NVRAM_LOG("NVM_AddBackupNum:read lseek error\n");
				goto read_from_bin;
			}
			iResult = read(iFileDesc, &iSavedFileNum, sizeof(unsigned int));
			if (iResult != sizeof(unsigned int)) {
				NVRAM_LOG("Err read BackupFlag File Fail(ignore):%d\n", iResult);
				goto read_from_bin;
			}
			goto read_done;
		}
read_from_bin:
		close(iFileDesc);
		iFileDesc = open(g_pcNVM_BackFlag, O_TRUNC | O_RDWR);
		if (!NVM_GetBackupFileNum(&iSavedFileNum, &iMDBackupFileNum)
		        || iSavedFileNum == 0xFFFFFFFF) {
			NVRAM_LOG("Err read Backup Number(BIN) Fail(ignore)\n");
			iSavedFileNum = 0;
		}
	}
read_done:
	NVRAM_LOG("AddBackupNum Saved:%d\n", iSavedFileNum);
	iSavedFileNum += iModifiedFileNum;
	NVRAM_LOG("AddBackupNum Update:%d\n", iSavedFileNum);
	if (-1 == lseek(iFileDesc, 0, SEEK_SET)) {
		NVRAM_LOG("NVM_AddBackupNum:write lseek error\n");
		unlink(g_pcNVM_BackFlag);
		close(iFileDesc);
		pthread_mutex_unlock(&backuplock);
		return false;
	}

	iResult = write(iFileDesc, &iSavedFileNum, sizeof(unsigned int));
	if (iResult != sizeof(unsigned int)) {
		NVRAM_LOG("Err write BackupFlag File Fail:%d\n", iResult);
		unlink(g_pcNVM_BackFlag);
		close(iFileDesc);
		pthread_mutex_unlock(&backuplock);
		return false;
	}
	if (!NVM_ComputeBackflagCheckSum(iFileDesc)) {
		unlink(g_pcNVM_BackFlag);
		close(iFileDesc);
		pthread_mutex_unlock(&backuplock);
		return false;
	}
	pthread_mutex_unlock(&backuplock);
	close(iFileDesc);
#endif

	return true;
}

bool NVM_AddBackupFileNum(unsigned int iLid) {
	int i;

	for (i = 0; i < (int)g_Backup_File_Count; i++) {
		if (iLid == aBackupToBinRegion[i].iLID) {
			if (NVM_AddBackupNum(1, 1)) {
				NVRAM_LOG("write BackupFlag File success:%d\n", iLid);
			} else {
				NVRAM_LOG("write BackupFlag File fail:%d\n", iLid);
			}
		}
	}
	return true;
}

/********************************************************************************
//FUNCTION:
//		NVM_Init
//DESCRIPTION:
//		this function is called to call nvram lib and get max lid.
//
//PARAMETERS:
//		None
//
//RETURN VALUE:
//		the max Lid number.
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
    extern int nvram_platform_callback(NVRAM_PLATFORM_T* pPlatform );
#ifdef __cplusplus
}
#endif

#if 0
extern int init_nvram_platform_callback();
extern int nvram_platform_callback();

typedef int(*pfCallbackForPlatform)(NVRAM_PLATFORM_T*);
extern pfCallbackForPlatform callback_for_nvram_platform;
#endif



FileName *gFileNameArray = NULL;


int NVM_Init(void) {
#if 0
	int ret_callback = 0;
#endif
	NVRAM_PLATFORM_T nvram_platform = {0,0,0,0,0};
	int ret;

    memset(&nvram_platform, 0, sizeof(NVRAM_PLATFORM_T));

	/* If NVRAM Init Completed, do not run twice*/
	if (nvram_init_flag) {
		return (g_i4MaxNvRamLid);
	}

	/* Each proecss need get nvram device path once*/
	ret = get_partition_path();
	if (ret < 0) {
		NVRAM_LOG("failed to get partition path\n");
		return -1;
	}
#if 0
	ret_callback = init_nvram_platform_callback();
	NVRAM_LOG("init_nvram_platform_callback: %d\n", ret_callback);
	if (callback_for_nvram_platform != NULL) {
		ret_callback = callback_for_nvram_platform(&nvram_platform);
		nvram_platform_log_block = nvram_platform.log_block;
		nvram_platform_resv_block = nvram_platform.resv_block;
		nvram_platform_DM_block = nvram_platform.DM_block;
	}
#endif
	/* NVRAM platform layout initialzation*/
	nvram_platform_callback(&nvram_platform);
	nvram_platform_log_block = nvram_platform.log_block;
	nvram_platform_resv_block = nvram_platform.resv_block;
	nvram_platform_DM_block = nvram_platform.DM_block;
	nvram_platform_layout_version = nvram_platform.layout_version;
	nvram_platform_header_offset = nvram_platform.header_offset;

	if (nvram_multi_storage_support(&nvram_platform)) {
		NVRAM_LOG("Enter nvram_multi_storage_support.\n");
		nvram_platform_log_block = nvram_platform.log_block;
		nvram_platform_resv_block = nvram_platform.resv_block;
		nvram_platform_DM_block = nvram_platform.DM_block;
		nvram_platform_layout_version = nvram_platform.layout_version;
		nvram_platform_header_offset = nvram_platform.header_offset;
	}

	NVRAM_LOG("libfile nvram_platform_log_block =%d,nvram_platform_resv_block=%d,nvram_platform_DM_block=%d "
		      "nvram_platform_layout_version##=%d,nvram_platform_header_offset=%d,g_i4CFG_File_Count=%d\n",
	          nvram_platform_log_block, nvram_platform_resv_block, nvram_platform_DM_block,
	          nvram_platform_layout_version, nvram_platform_header_offset, g_i4CFG_File_Count);

	/* Calculate the Max Lid, contain the common lid and custom lid*/
	g_i4MaxNvRamLid = g_i4CFG_File_Count + g_i4CFG_File_Custom_Count;
	NVRAM_LOG("NVM_Init Max Lid: %d\n", g_i4MaxNvRamLid);

	/*
	 *  If g_NVRAM_BACKUP_DEVICE get the path through the get_partition_path operations,
	 *  need confirm this path is for GPT solution or PMT solution.
	 */
	if (!strstr(g_NVRAM_BACKUP_DEVICE, "block"))
		nvram_gpt_flag = 0;
	else
		nvram_gpt_flag = 1;
	NVRAM_LOG("[NVRAM]: info nvram_gpt_flag =%d\r\n", nvram_gpt_flag);

	/* NVRAM_Init Completed and set one flag in each process */
	nvram_init_flag = 1;
	return (g_i4MaxNvRamLid);
}
int NVM_GetLIDByName(char* filename) {

	int i = 0;
	int Lid = - 1;

	NVRAM_LOG("NVM_GetLIDByName %s \n", filename);
	if (NVM_Init() < 0) {
		NVRAM_LOG("NVM_GetLIDByName--NVM_Init fail !!!\n");
		return -1;
	}

	if (gFileNameArray == NULL) {

		gFileNameArray = (FileName*)malloc(g_i4MaxNvRamLid * sizeof(FileName));
		//NVRAM_LOG("gFileNameArray =%x\n", gFileNameArray);

		if (gFileNameArray == NULL) {
			NVRAM_LOG("Filename array malloc fail \n");
			free(gFileNameArray);
			return -1;
		}


		for ( i = 0; i < g_i4CFG_File_Count; i++) {

			//strcpy(gFileNameArray[i].cFileName, g_akCFG_File[i].cFileName);
			snprintf(gFileNameArray[i].cFileName, sizeof(g_akCFG_File[i].cFileName),"%s", g_akCFG_File[i].cFileName);
			gFileNameArray[i].iLID = i;

			NVRAM_LOG("deal with cfgfile =%s,%d\n", gFileNameArray[i].cFileName,
			          gFileNameArray[i].iLID);
		}
		for ( i = g_i4CFG_File_Count ;
		        i < g_i4CFG_File_Count + g_i4CFG_File_Custom_Count; i++) {

			//strcpy(gFileNameArray[i].cFileName, g_akCFG_File_Custom[i-iCustomBeginLID].cFileName);
			snprintf(gFileNameArray[i].cFileName, sizeof(gFileNameArray[i].cFileName),"%s", g_akCFG_File_Custom[i-iCustomBeginLID].cFileName);
			gFileNameArray[i].iLID = i;

			NVRAM_LOG("deal with cfgcustfile =%s,%d\n", gFileNameArray[i].cFileName,
			          gFileNameArray[i].iLID);
		}

	}



	if (!filename || (strstr(filename, "Reserved") != NULL)
	        || (strlen(filename) > FILENAMELENGTH)  || (strlen(filename) == 0)) {
		NVRAM_LOG("NVRAM: Invalide argument for find LID name array! \n");
		return -1;
	}

	for (i = 0 ; i < g_i4MaxNvRamLid; i++) {

		if (strstr(gFileNameArray[i].cFileName, filename)) {


			Lid = gFileNameArray[i].iLID;

			break;
		}
	}
	NVRAM_LOG("NVRAM: NVM_GetLIDByName Lid =%d \n", Lid);
	return Lid;

}

/********************************************************************************
//FUNCTION:
//		NVM_GetCfgFileTable
//DESCRIPTION:
//		this function is called to the array table of nvram file information .
//
//PARAMETERS:
//		file_lid: [IN] the lid of nvram file
//
//RETURN VALUE:
//		refers to the definition of "TCFG_FILE"
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
const TCFG_FILE* NVM_GetCfgFileTable(int file_lid) {
	//NVRAM_LOG("NVM_GetCfgFileTable : file_lid = %d\n", file_lid);
	// NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);
	// NVRAM_LOG("g_i4CFG_File_Count:%d,g_i4CFG_File_Custom_Count:%d\n",g_i4CFG_File_Count,g_i4CFG_File_Custom_Count);
	if (file_lid >= (g_i4CFG_File_Count + g_i4CFG_File_Custom_Count)) {
		NVRAM_LOG("NVM_GetCfgFileTable file_lid is over than maximum %d\n",
		          (g_i4CFG_File_Count + g_i4CFG_File_Custom_Count));
		return NULL;
	}
	//if (file_lid < AP_CFG_CUSTOM_BEGIN_LID)
	if (file_lid < iCustomBeginLID) {
		//NVRAM_LOG("NVM_GetCfgFileTable MTK %d\n", file_lid);
		return g_akCFG_File;
	}

	//NVRAM_LOG("NVM_GetCfgFileTable Customer %d\n", file_lid);
	return g_akCFG_File_Custom;
}

/*******************************************************************************
//FUNCTION:
//		NVM_GenerateFileVer
//DESCRIPTION:
//		this function is called to generate the version file in backup or nvram partition.
//
//PARAMETERS:
//		CPY_File_To_NVM: [IN] true is generate version file in NVM partition
//
//RETURN VALUE:
//		true is success, otherwise is fail
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
******************************************************************************/
bool NVM_GenerateFileVer(bool CPY_File_To_NVM) {
	int iFileDesc;
	int iFilNO = 0;
	char tempstr[FILENAMELENGTH];
	int iStrLen = 0;
	memset(tempstr, 0, FILENAMELENGTH);
	iStrLen = strlen(strVerInfo);

	NVRAM_LOG("NVM_GenerateFileVer: CPY_File_To_NVM=%d ++\n", CPY_File_To_NVM);
	NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);

	// we now use global varialbe to desribe the version instead of files,
	// so it just return when check default version file
	if (!CPY_File_To_NVM) {
		return true;
	}

	pthread_mutex_lock(&generateFileVerlock);
	// create the version file in FAT2 partitioin.
	iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_CREAT | O_RDWR,
	                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	NVRAM_LOG("Create/Open the file of %s \n",
	          g_akCFG_File[iFileVerInfoLID].cFileName);
	if (iFileDesc == -1) {
		NVRAM_LOG("Create the dir path of %s\n",
		          g_akCFG_File[iFileVerInfoLID].cFileName);
		iFileDesc = open_file_with_dirs(g_akCFG_File[iFileVerInfoLID].cFileName,
		                                O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	}

	if (iFileDesc == -1) {
		NVRAM_LOG("Fail to open %s\n", g_akCFG_File[iFileVerInfoLID].cFileName);
		pthread_mutex_unlock(&generateFileVerlock);
		return false;
	}
	memcpy(tempstr, strVerInfo, iStrLen + 1);
	write(iFileDesc, tempstr, FILENAMELENGTH);
	write(iFileDesc, g_akCFG_File[iFileVerInfoLID].cFileVer, 4);
	memset(tempstr, 0, FILENAMELENGTH);
	for (iFilNO = iFileVerInfoLID; iFilNO < iCustomBeginLID; iFilNO++) {
		int i;
		iStrLen = strlen(g_akCFG_File[iFilNO].cFileName);
		for (i = iStrLen; i >= 0; i--) {
			if (g_akCFG_File[iFilNO].cFileName[i] == '/') {
                            //strcpy(tempstr,g_akCFG_File[iFilNO].cFileName+i+1);
                            snprintf(tempstr, sizeof(tempstr),"%s", g_akCFG_File[iFilNO].cFileName+i+1);
				break;
			}
		}
		write(iFileDesc, tempstr, FILENAMELENGTH);
		write(iFileDesc, g_akCFG_File[iFilNO].cFileVer, 4);
		memset(tempstr, 0, FILENAMELENGTH);
	}

	for (iFilNO = iCustomBeginLID;
	        iFilNO < (g_i4CFG_File_Count + g_i4CFG_File_Custom_Count); iFilNO++) {
		int i;
		iStrLen = strlen(g_akCFG_File_Custom[iFilNO - iCustomBeginLID].cFileName);
		for (i = iStrLen; i >= 0; i--) {
			if (g_akCFG_File_Custom[iFilNO - iCustomBeginLID].cFileName[i] == '/') {
                            strncpy(tempstr, g_akCFG_File_Custom[iFilNO-iCustomBeginLID].cFileName+i+1, sizeof(tempstr)-1);
                            tempstr[sizeof(tempstr)-1] = '\0';
				break;
			}
		}
		write(iFileDesc, tempstr, FILENAMELENGTH);
		write(iFileDesc, g_akCFG_File_Custom[iFilNO - iCustomBeginLID].cFileVer, 4);
		memset(tempstr, 0, FILENAMELENGTH);
	}
	if (-1 == chown(g_akCFG_File[iFileVerInfoLID].cFileName, -1, AID_SYSTEM))
		NVRAM_LOG("change dir group owner failed:%s\n", (char*)strerror(errno));
	if (-1 == chmod(g_akCFG_File[iFileVerInfoLID].cFileName, 0664))
		NVRAM_LOG("chmod file failed: %s\n", (char*)strerror(errno));

	close(iFileDesc);
	pthread_mutex_unlock(&generateFileVerlock);

	NVRAM_LOG("NVM_GenerateFileVer: CPY_File_To_NVM =%d --\n", CPY_File_To_NVM);

	return true;
}



/********************************************************************************
//FUNCTION:
//		NVM_CheckVerFile
//DESCRIPTION:
//		this function is called to check the exist of versiono file information
//      in NVM partition or default version.
//
//PARAMETERS:
//		Is_NVM: [IN] true is to check NVM partition, otherwise check default version
//
//RETURN VALUE:
//		true is exist, otherwise version is not exist.
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_CheckVerFile(bool In_NVM) {
	struct stat statbuf;
	//NVRAM_LOG("NVM_CheckVerFile: %d\n", In_NVM);

	// when check default version file, just return
	// becuase we now use global variable (g_akCFG_File.cFileVer)
	if (In_NVM) {
		if (stat(g_akCFG_File[iFileVerInfoLID].cFileName, &statbuf) == -1
		        || !S_ISREG(statbuf.st_mode)) {
			return false;
		}
	}
	return true;
}

/********************************************************************************
//FUNCTION:
//		NVM_UpdateFileVerNo
//DESCRIPTION:
//		this function is called to reset a NvRam to default value.
//
//PARAMETERS:
//		file_lid: [IN] the lid of nvram file
//
//RETURN VALUE:
//		true is success, otherwise is fail
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_UpdateFileVerNo(int file_lid, VerInfoUpdateFlag UpdateFlag) {
	int iFileDesc;
	int iFileSize;
	int iRealFileLid = 0;
	struct stat st;
	char * buffer1 = NULL;
	char * buffer2 = NULL;
	int iResult, iStrLen;
	bool bIsNewVerInfo;
	char tempstr[FILENAMELENGTH];
	char tempstr1[FILENAMELENGTH];
	const TCFG_FILE *pCfgFileTable = NULL;

	struct stat statbuf;
	int ilooptime, iIndex;
	off_t offset = 0;

	memset(tempstr, 0, FILENAMELENGTH);
	memset(tempstr1,0,FILENAMELENGTH);

	NVRAM_LOG("NVM_UpdateFileVerNo: %d ++\n", file_lid);
	NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);

	if (UpdateFlag == VerUpdate) {
		pCfgFileTable = NVM_GetCfgFileTable(file_lid);
		//NVRAM_LOG("NVM_GetCfgFileTable done\n");
		if ( file_lid < iCustomBeginLID ) {
			NVRAM_LOG("This File LID is belong to common lid!\n");
			iRealFileLid = file_lid;
		} else {
			NVRAM_LOG("This File LID is belong to custom lid\n");
			iRealFileLid = file_lid - iCustomBeginLID;
		}

		if (pCfgFileTable == NULL) {
			NVRAM_LOG("NVM_GetCfgFileTable Fail!!! \n");
			return false;
		}

		memset(tempstr1, 0, FILENAMELENGTH);
		iStrLen = strlen(pCfgFileTable[iRealFileLid].cFileName);
		for (iIndex = iStrLen; iIndex >= 0; iIndex--) {
			if ( pCfgFileTable[iRealFileLid].cFileName[iIndex] == '/') {
	         		//strcpy( tempstr1, pCfgFileTable[iRealFileLid].cFileName + iIndex + 1 );//get the target name
	         		snprintf(tempstr1, sizeof(tempstr1),"%s", pCfgFileTable[iRealFileLid].cFileName + iIndex + 1);  		
				break;
			}
		}

		NVRAM_LOG("DEBUG searching LID = %d,%s\n", file_lid, tempstr1);

		iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_RDONLY);
     	if (iFileDesc == -1) {
        	NVRAM_LOG("Error NVM_UpdateFileVerNo Open1 version file fail \n");
		pthread_mutex_unlock(&generateFileVerlock);
        	return false;
     	}
		fstat(iFileDesc, &statbuf);

		iFileSize = (int)statbuf.st_size;
		ilooptime = iFileSize / (FILEVERLENGTH + FILENAMELENGTH);

		for (iIndex = 0; iIndex < ilooptime; iIndex++) {
#if 1
			offset = iIndex * (FILEVERLENGTH + FILENAMELENGTH);
			iResult =pread(iFileDesc, tempstr, FILENAMELENGTH-1, offset);
			if (iResult == -1) { NVRAM_LOG("pread failed with errno=%d\n", errno); iResult = 0; }
			tempstr[iResult]='\0';
			if (0 == strcmp(tempstr, tempstr1)) {
				break;
			}
#endif
		}
		if (iIndex == ilooptime) {
			NVRAM_LOG("NVM_UpdateFileVerNo find target file fail: %d\n", file_lid);
			UpdateFlag = VerAdd;

		}
		close(iFileDesc);
		memset(tempstr, 0, FILENAMELENGTH);
	}
	/* open file */
	pthread_mutex_lock(&generateFileVerlock);
	iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_RDWR);

	if (iFileDesc == -1) {
		NVRAM_LOG("Error NVM_UpdateFileVerNo Open1 version file fail \n");
		pthread_mutex_unlock(&generateFileVerlock);
		return false;
	}
    	iResult = pread(iFileDesc,tempstr,FILENAMELENGTH-1, 0);
		if (iResult == -1) { NVRAM_LOG("pread failed with errno=%d\n", errno); iResult = 0; }
		tempstr[iResult]='\0';
	if (0 == strcmp(tempstr, strVerInfo))
		bIsNewVerInfo = true;
	else
		bIsNewVerInfo = false;
	memset(tempstr, 0, FILENAMELENGTH);

	if (UpdateFlag == VerUpdate) {
		/* Ver Info has existed , or added at the end of Version Info File
		 * move to the address in the version file.
		 * becuase we just use cFileVer to save the version,
		 * so the offset address is file_lid* sizeof(cFileVer)
		 */
		if (!bIsNewVerInfo) {
			offset = file_lid * FILEVERLENGTH;
		}

		if (file_lid < iCustomBeginLID) {
			if (bIsNewVerInfo) {
				int i;
				iStrLen = strlen(g_akCFG_File[file_lid].cFileName);
				for (i = iStrLen; i >= 0; i--) {
					if (g_akCFG_File[file_lid].cFileName[i] == '/') {
             					//strcpy(tempstr,g_akCFG_File[file_lid].cFileName+i+1);
             					snprintf(tempstr, sizeof(tempstr),"%s", g_akCFG_File[file_lid].cFileName+i+1);  
						break;
					}
				}
				pwrite(iFileDesc, tempstr, FILENAMELENGTH, offset);
				memset(tempstr, 0, FILENAMELENGTH);
			}
			pwrite(iFileDesc, g_akCFG_File[file_lid].cFileVer, FILEVERLENGTH,
			       offset + FILENAMELENGTH);
		} else {
			if (bIsNewVerInfo) {
				int i;
				iStrLen = strlen(g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileName);
				for (i = iStrLen; i >= 0; i--) {
					if (g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileName[i] == '/') {
 			            		//strcpy(tempstr,g_akCFG_File_Custom[file_lid-iCustomBeginLID].cFileName+i+1);
 			            		snprintf(tempstr, sizeof(tempstr),"%s", g_akCFG_File_Custom[file_lid-iCustomBeginLID].cFileName+i+1);  	
						break;
					}
				}
				pwrite(iFileDesc, tempstr, FILENAMELENGTH, offset);
				memset(tempstr, 0, FILENAMELENGTH);
			}
			pwrite(iFileDesc, g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileVer,
			       FILEVERLENGTH, offset + FILENAMELENGTH);
		}
		//close(iFileDesc);
	} else if (UpdateFlag == VerDel) {
		if (stat(g_akCFG_File[iFileVerInfoLID].cFileName, &st) < 0) {
			NVRAM_LOG("Error NVM_UpdateFileVerNo stat \n");
			close(iFileDesc);
			pthread_mutex_unlock(&generateFileVerlock);
			return false;
		}
		offset = 0;
		iFileSize = st.st_size;

		buffer1 = (char*)malloc(iFileSize);
		if (bIsNewVerInfo)
			buffer2 = (char*)malloc(iFileSize - FILEVERLENGTH - FILENAMELENGTH);
		else
			buffer2 = (char*)malloc(iFileSize - FILEVERLENGTH);
		if (buffer1 == NULL || buffer2 == NULL) {
			NVRAM_LOG("Error NVM_UpdateFileVerNo stat \n");
			if (buffer1 != NULL)
				free(buffer1);
			if (buffer2 != NULL)
				free(buffer2);
			close(iFileDesc);
			pthread_mutex_unlock(&generateFileVerlock);
			return false;
		}
		iResult = pread(iFileDesc, buffer1, iFileSize, offset);
		if (iResult != st.st_size) {
			NVRAM_LOG("Error NVM_UpdateFileVerNo read del\n");
			NVRAM_LOG("iResult = %d, st.st_size = %d.\n", iResult, (int)st.st_size);
			free(buffer1);
			free(buffer2);
			close(iFileDesc);
			pthread_mutex_unlock(&generateFileVerlock);
			return false;
		}
		close(iFileDesc);
		pthread_mutex_unlock(&generateFileVerlock);
		if (!bIsNewVerInfo) {
			memcpy(buffer2, buffer1, file_lid * FILEVERLENGTH);
			memcpy(buffer2 + file_lid * FILEVERLENGTH,
			       buffer1 + (file_lid + 1)*FILEVERLENGTH,
			       iFileSize - (file_lid + 1)*FILEVERLENGTH);
		} else {
			memcpy(buffer2, buffer1, (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH));
			memcpy(buffer2 + (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH),
			       buffer1 + (file_lid + 1 + 1) * (FILEVERLENGTH + FILENAMELENGTH),
			       iFileSize - (file_lid + 1 + 1) * (FILEVERLENGTH + FILENAMELENGTH));
		}
		pthread_mutex_lock(&generateFileVerlock);
		iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName,
		                 O_TRUNC | O_RDWR);
		if (iFileDesc == -1) {
			NVRAM_LOG("Error NVM_UpdateFileVerNo Open2 version file fail \n");
			free(buffer1);
			free(buffer2);
			pthread_mutex_unlock(&generateFileVerlock);
			return false;
		}
		if (!bIsNewVerInfo) {
			iResult = pwrite(iFileDesc, buffer2, iFileSize - FILEVERLENGTH, 0);
			if (iResult != (iFileSize - FILEVERLENGTH)) {
				NVRAM_LOG("Error NVM_UpdateFileVerNo write version file fail \n");
				close(iFileDesc);
				free(buffer1);
				free(buffer2);
				pthread_mutex_unlock(&generateFileVerlock);
				return false;
			}
		} else {
			iResult = pwrite(iFileDesc, buffer2, iFileSize - FILEVERLENGTH - FILENAMELENGTH,
			                 0);
			if (iResult != (iFileSize - FILEVERLENGTH - FILENAMELENGTH)) {
				NVRAM_LOG("Error NVM_UpdateFileVerNo write version file fail \n");
				close(iFileDesc);
				free(buffer1);
				free(buffer2);
				pthread_mutex_unlock(&generateFileVerlock);
				return false;
			}
		}
		free(buffer1);
		free(buffer2);
	} else if (UpdateFlag == VerAdd) {
		if (stat(g_akCFG_File[iFileVerInfoLID].cFileName, &st) < 0) {
			NVRAM_LOG("Error NVM_UpdateFileVerNo stat \n");
			close(iFileDesc);
			pthread_mutex_unlock(&generateFileVerlock);
			return false;
		}
		iFileSize = st.st_size;
		bool bAddedAtend;
		if (bIsNewVerInfo) {
			if ((file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH) >= iFileSize)
				bAddedAtend = true;
			else
				bAddedAtend = false;
		} else {
			if (file_lid * FILEVERLENGTH >= iFileSize)
				bAddedAtend = true;
			else
				bAddedAtend = false;
		}
		if (bAddedAtend) {
			NVRAM_LOG("Added at the end of the file \n");
			if (bIsNewVerInfo)
				offset = (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH);
			else
				offset = file_lid * FILEVERLENGTH;

			if (file_lid < iCustomBeginLID) {
				if (bIsNewVerInfo) {
					int i;
					iStrLen = strlen(g_akCFG_File[file_lid].cFileName);
					for (i = iStrLen; i >= 0; i--) {
						if (g_akCFG_File[file_lid].cFileName[i] == '/') {
               						//strcpy(tempstr,g_akCFG_File[file_lid].cFileName+i+1);
               						snprintf(tempstr, sizeof(tempstr),"%s", g_akCFG_File[file_lid].cFileName+i+1); 
							break;
						}
					}
					pwrite(iFileDesc, tempstr, FILENAMELENGTH, offset);
					memset(tempstr, 0, FILENAMELENGTH);
				}
				pwrite(iFileDesc, g_akCFG_File[file_lid].cFileVer, FILEVERLENGTH,
				       offset + FILENAMELENGTH);
			} else {
				if (bIsNewVerInfo) {
					int i;
					iStrLen = strlen(g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileName);
					for (i = iStrLen; i >= 0; i--) {
						if (g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileName[i] == '/') {
	               					//strcpy(tempstr,g_akCFG_File_Custom[file_lid-iCustomBeginLID].cFileName+i+1);
	               					snprintf(tempstr, sizeof(tempstr),"%s", g_akCFG_File_Custom[file_lid-iCustomBeginLID].cFileName+i+1);
							break;
						}
					}
					pwrite(iFileDesc, tempstr, FILENAMELENGTH, offset);
					memset(tempstr, 0, FILENAMELENGTH);
				}
				pwrite(iFileDesc, g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileVer,
				       FILEVERLENGTH, offset + FILENAMELENGTH);
			}
		} else {
			NVRAM_LOG("Added at the middle part of the file \n");
			buffer1 = (char*)malloc(iFileSize);
			if (bIsNewVerInfo) {
				buffer2 = (char*)malloc(iFileSize + FILEVERLENGTH + FILENAMELENGTH);
			} else {
				buffer2 = (char*)malloc(iFileSize + FILEVERLENGTH);
			}
			if (buffer1 == NULL || buffer2 == NULL) {
				NVRAM_LOG("Error NVM_UpdateFileVerNo stat \n");
				if (buffer1 != NULL)
					free(buffer1);
				if (buffer2 != NULL)
					free(buffer2);
				close(iFileDesc);
				pthread_mutex_unlock(&generateFileVerlock);
				return false;
			}
			close(iFileDesc);
			pthread_mutex_unlock(&generateFileVerlock);
			pthread_mutex_lock(&generateFileVerlock);
			iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_RDWR);
			if (iFileDesc == -1) {
				NVRAM_LOG("Error NVM_UpdateFileVerNo Open1 version file fail \n");
				pthread_mutex_unlock(&generateFileVerlock);
				free(buffer1);
				free(buffer2);
				return false;
			}
			iResult = pread(iFileDesc, buffer1, iFileSize, 0);
			if (iResult != iFileSize) {
				NVRAM_LOG("Error NVM_UpdateFileVerNo read add \n");
				free(buffer1);
				free(buffer2);
				close(iFileDesc);
				pthread_mutex_unlock(&generateFileVerlock);
				return false;
			}
			close(iFileDesc);
			pthread_mutex_unlock(&generateFileVerlock);
			NVRAM_LOG("cpy1\n");
			if (bIsNewVerInfo) {
				memcpy(buffer2, buffer1, (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH));
			} else {
				memcpy(buffer2, buffer1, file_lid * FILEVERLENGTH);
			}
			if (file_lid < iCustomBeginLID) {
				if (bIsNewVerInfo) {
					int i;
					iStrLen = strlen(g_akCFG_File[file_lid].cFileName);
					for (i = iStrLen; i >= 0; i--) {
						if (g_akCFG_File[file_lid].cFileName[i] == '/') {
                 					//strcpy(tempstr,g_akCFG_File[file_lid].cFileName+i+1);
                 					snprintf(tempstr, sizeof(tempstr),"%s", g_akCFG_File[file_lid].cFileName+i+1);
							break;
						}
					}
					memcpy(buffer2 + (file_lid + 1) * (FILENAMELENGTH + FILEVERLENGTH), tempstr,
					       FILENAMELENGTH);
					memset(tempstr, 0, FILENAMELENGTH);
					memcpy(buffer2 + (file_lid + 1) * (FILENAMELENGTH + FILEVERLENGTH) +
					       FILENAMELENGTH,
					       g_akCFG_File[file_lid].cFileVer, FILEVERLENGTH);
				} else {
					memcpy(buffer2 + file_lid * FILEVERLENGTH, g_akCFG_File[file_lid].cFileVer,
					       FILEVERLENGTH);
				}
			} else {
				if (bIsNewVerInfo) {
					int i;
					iStrLen = strlen(g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileName);
					for (i = iStrLen; i >= 0; i--) {
						if (g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileName[i] == '/') {
                  					//strcpy(tempstr,g_akCFG_File_Custom[file_lid-iCustomBeginLID].cFileName+i+1);
                  					snprintf(tempstr, sizeof(tempstr),"%s", g_akCFG_File_Custom[file_lid-iCustomBeginLID].cFileName+i+1);  
							break;
						}
					}
					memcpy(buffer2 + (file_lid + 1) * (FILENAMELENGTH + FILEVERLENGTH), tempstr,
					       FILENAMELENGTH);
					memset(tempstr, 0, FILENAMELENGTH);
					memcpy(buffer2 + (file_lid + 1) * (FILENAMELENGTH + FILEVERLENGTH) +
					       FILENAMELENGTH,
					       g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileVer, FILEVERLENGTH);

				} else {
					memcpy(buffer2 + file_lid * FILEVERLENGTH,
					       g_akCFG_File_Custom[file_lid - iCustomBeginLID].cFileVer, FILEVERLENGTH);
				}
			}
			NVRAM_LOG("cpy2\n");
			if (bIsNewVerInfo) {
				memcpy(buffer2 + (file_lid + 1 + 1) * (FILEVERLENGTH + FILENAMELENGTH),
				       buffer1 + (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH),
				       iFileSize - (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH));
			} else {
				memcpy(buffer2 + (file_lid + 1)*FILEVERLENGTH,
				       buffer1 + file_lid * FILEVERLENGTH,
				       iFileSize - file_lid * FILEVERLENGTH);
			}
			NVRAM_LOG("cpy3\n");
			pthread_mutex_lock(&generateFileVerlock);
			iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_TRUNC | O_RDWR);
			if (iFileDesc == -1) {
				NVRAM_LOG("Error NVM_UpdateFileVerNo Open2 version file fail \n");
				free(buffer1);
				free(buffer2);
				pthread_mutex_unlock(&generateFileVerlock);
				return false;
			}
			if (bIsNewVerInfo) {
				iResult = pwrite(iFileDesc, buffer2, iFileSize + FILEVERLENGTH + FILENAMELENGTH,
				                 0);
				if (iResult != (iFileSize + FILEVERLENGTH + FILENAMELENGTH)) {
					NVRAM_LOG("Error NVM_UpdateFileVerNo write version file fail \n");
					close(iFileDesc);
					free(buffer1);
					free(buffer2);
					pthread_mutex_unlock(&generateFileVerlock);
					return false;
				}
			} else {
				iResult = pwrite(iFileDesc, buffer2, iFileSize + FILEVERLENGTH, 0);
				if (iResult != (iFileSize + FILEVERLENGTH)) {
					NVRAM_LOG("Error NVM_UpdateFileVerNo write version file fail \n");
					close(iFileDesc);
					free(buffer1);
					free(buffer2);
					pthread_mutex_unlock(&generateFileVerlock);
					return false;
				}
			}
			free(buffer1);
			free(buffer2);
		}
	} else {
		pthread_mutex_unlock(&generateFileVerlock);
		return false;
	}
	close(iFileDesc);
	pthread_mutex_unlock(&generateFileVerlock);
	NVRAM_LOG("UpdateFileVerNo: %d --\n", file_lid);
	return true;
}
#define Loop_num    4096

static unsigned int NVM_ComputeCheckSum(void)
{
     time_t start = time(NULL);
     NVBAK_LOG("Starting NVM_ComputeCheckSum (pid %d) on %s",getpid(),ctime(&start));////add for log
     int iFileDesc_file;
     unsigned int iFileSize;
     unsigned int ulCheckSum=0;
     int looptime;
     struct stat st;
     int i,flag,j;//add j
     int iResult;
     int iLength=sizeof(unsigned int);
     unsigned int tempNum;
     unsigned int  tempBuf[4096] ;//add for read
     int jloop_num=0;
	 int  tmplooptime;//add for read
     if(stat(g_pcNVM_AllFile,&st)<0)
     {
        NVBAK_LOG("Error NVM_ComputeCheckSum stat \n");
        return 0;
    }
   iFileSize=st.st_size;
   looptime=iFileSize/(sizeof(unsigned int));
   tmplooptime = looptime/(Loop_num) + 1;
   iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
      if(iFileDesc_file<0)
      {
            NVBAK_LOG("NVM_ComputeCheckSum cannot open data file\n");
            return 0;
      }
   flag=1;
   //NVBAK_LOG("looptime is %d \n", looptime);////add for log
   //NVBAK_LOG("tmplooptime is %d \n", tmplooptime);////add for log
   //time_t start_while = time(NULL);////add for log
   //NVBAK_LOG("Start_while NVM_ComputeCheckSum (pid %d) on %s",getpid(),ctime(&start_while));//add
   for(i=0;i<tmplooptime;i++)
    {
        if(i != tmplooptime-1 )
        {
            jloop_num=Loop_num;}
        else
        {
            jloop_num=looptime%(Loop_num);}
            //NVBAK_LOG("Start_while: i is %d, NVM_ComputeCheckSum (pid %d) on %s",i,getpid(),ctime(&start_while));// add for log
            iResult=read(iFileDesc_file, tempBuf, jloop_num*iLength);
            //NVBAK_LOG("Start_while: iResult is %d, NVM_ComputeCheckSum (pid %d) on %s",iResult,getpid(),ctime(&start_while));//add for log
        if(iResult!= jloop_num*iLength)
        {
            NVBAK_LOG("NVM_ComputeCheckSum cannot read checksum data\n");
            close(iFileDesc_file);
            return 0;
        }
        //NVBAK_LOG("Start_while:before_while NVM_ComputeCheckSum (pid %d) on %s",getpid(),ctime(&start_while));//add for log
        for(j=0;j<jloop_num;j++)
        {
            if(flag)
            {
                ulCheckSum^=tempBuf[j];
                flag=0;
            }
            else
            {
                ulCheckSum+=tempBuf[j];
                flag=1;
            }
      }
      }
      tempNum=0;
      iLength=iFileSize%(sizeof(unsigned int));
      iResult=read(iFileDesc_file, &tempNum, iLength);
      NVBAK_LOG("tempNum = %x", tempNum);//add for log
      if(iResult!= iLength)
           {
              NVBAK_LOG("NVM_ComputeCheckSum cannot read last checksum data\n");
              close(iFileDesc_file);
              return 0;
           }
        ulCheckSum+=tempNum;
        //ulCheckSum^=gFileStartAddr;
        close(iFileDesc_file);
        time_t end = time(NULL);
        NVBAK_LOG("Ending NVM_ComputeCheckSum (pid %d) on %s",getpid(),ctime(&end));//add for log
        return ulCheckSum;
}
#if 0
static unsigned int NVM_ComputeCheckSum(void) {
	int iFileDesc_file;
	char cReadData;
	unsigned int iFileSize;
	unsigned int ulCheckSum = 0;
	int looptime;
	struct stat st;
	int i, flag;
	int iResult;
	int iLength = sizeof(unsigned int);
	unsigned int tempNum;

	if (stat(g_pcNVM_AllFile, &st) < 0) {
		NVBAK_LOG("Error NVM_ComputeCheckSum stat \n");
		return 0;
	}

	iFileSize = st.st_size;

	looptime = iFileSize / (sizeof(unsigned int));

	iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
	if (iFileDesc_file < 0) {
		NVBAK_LOG("NVM_ComputeCheckSum cannot open data file\n");
		return 0;
	}
	flag = 1;
	for (i = 0; i < looptime; i++) {
		iResult = read(iFileDesc_file, &tempNum, iLength);
		if (iResult != iLength) {
			NVBAK_LOG("NVM_GetCheckSum cannot read checksum data\n");
			close(iFileDesc_file);
			return 0;
		}
		if (flag) {
			ulCheckSum ^= tempNum;
			flag = 0;
		} else {
			ulCheckSum += tempNum;
			flag = 1;
		}
	}
	tempNum = 0;
	iLength = iFileSize % (sizeof(unsigned int));
	iResult = read(iFileDesc_file, &tempNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("NVM_GetCheckSum cannot read last checksum data\n");
		close(iFileDesc_file);
		return 0;
	}
	ulCheckSum += tempNum;
	//ulCheckSum^=gFileStartAddr;
	close(iFileDesc_file);
	return ulCheckSum;
}
#endif


static BackupFileInfo stBackupFileInfo;
static bool NVM_GetCheckSum(void) {
	int iFileDesc_map;
	int iResult;
	int iLength = sizeof(unsigned int);

	iFileDesc_map = open(g_pcNVM_AllMap, O_RDONLY);
	if (iFileDesc_map < 0) {
		NVBAK_LOG("NVM_GetCheckSum cannot open/create map data\n");
		return false;
	}
	iResult = read(iFileDesc_map, &stBackupFileInfo.ulCheckSum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("NVM_GetCheckSum cannot read checksum data\n");
		close(iFileDesc_map);
		return false;
	}
	iLength = sizeof(unsigned int);
	iResult = read(iFileDesc_map, &stBackupFileInfo.iCommonFileNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("NVM_GetCheckSum cannot read checksum data\n");
		close(iFileDesc_map);
		return false;
	}
	iResult = read(iFileDesc_map, &stBackupFileInfo.iCustomFileNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("NVM_GetCheckSum cannot read checksum data\n");
		close(iFileDesc_map);
		return false;
	}
	close(iFileDesc_map);

	return true;
}

bool NVM_SetCheckSum(unsigned int ulCheckSum) {
	int iFileDesc_map = 0;
	int iResult;
	int iLength = sizeof(unsigned int);
	unsigned int iOldCommonFileNum = g_i4CFG_File_Count;
	unsigned int iOldCustomFileNum = g_i4CFG_File_Custom_Count;

	pthread_mutex_lock(&gFileStartAddrlock);
	ulCheckSum ^= gFileStartAddr;
	gFileStartAddr = 0;
	pthread_mutex_unlock(&gFileStartAddrlock);
	iFileDesc_map = open(g_pcNVM_AllMap, O_WRONLY);
	if (iFileDesc_map < 0) {
		NVBAK_LOG("NVM_SetCheckSum cannot open/create map data\n");
		return false;
	}
	iResult = write(iFileDesc_map, &ulCheckSum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("NVM_SetCheckSum cannot write checksum data\n");
		close(iFileDesc_map);
		return false;
	}
	NVBAK_LOG("common file num:%d,custom file num:%d\n", iOldCommonFileNum,
	          iOldCustomFileNum);
	iLength = sizeof(unsigned int);
	iResult = write(iFileDesc_map, &iOldCommonFileNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("NVM_SetCheckSum cannot write common file num data\n");
		close(iFileDesc_map);
		return false;
	}
	iResult = write(iFileDesc_map, &iOldCustomFileNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("NVM_SetCheckSum cannot write custom file num data\n");
		close(iFileDesc_map);
		return false;
	}
	close(iFileDesc_map);
	return true;
}

static bool NVM_CompareCheckSum(unsigned int ulCheckSum1,
                                unsigned int ulCheckSum2) {
	if (ulCheckSum1 != ulCheckSum2)
		return false;
	return true;
}
bool NVM_RestoreFromFiles_OneFile(int eBackupType, int file_lid,
                                  const char* filename, bool* find_flag) {
	int iFileDesc_file, iFileDesc_map, iFileDesc, iSize;
	int iFileTitleOffset = 0;
	short int iFileNum = 0;
	char *buf;  /* content  */
	File_Title *FileInfo = NULL;
	bool bRet = false;
	off_t iRet;
	File_Title_Header1 FileTitleInfo1;
	File_Title_Header2 FileTitleInfo2;
	File_Title_Header3 FileTitleInfo3;
	File_Title_Header4 FileTitleInfo4;
	File_Title_Header5 FileTitleInfo5;

	memset(&FileTitleInfo3, 0x00, sizeof(File_Title_Header3));
	int fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
	              File_Title_Header1);

	if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
		NVRAM_LOG("NVM_RestoreFromFiles_OneFile: File_Title_Header1 set\n");
	} else if (nvram_md5_support() && !nvram_evdo_support()
	           && !nvram_ecci_c2k_support()) {
		NVRAM_LOG("NVM_RestoreFromFiles_OneFile: File_Title_Header2 set\n");
		fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
		          File_Title_Header2) - 2 * sizeof(short
		                  int); //In order to align, minus 2*sizeof(short int)
	} else if (!nvram_md5_support() && nvram_evdo_support()
	           && !nvram_ecci_c2k_support()) {
		NVRAM_LOG("NVM_RestoreFromFiles_OneFile: File_Title_Header3 set\n");
		fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
		          File_Title_Header3) - 4 * sizeof(short
		                  int); //In order to align, minus 4*sizeof(short int)
	} else if (!nvram_md5_support() && !nvram_evdo_support()
	           && !nvram_ecci_c2k_support()) {
		NVRAM_LOG("NVM_RestoreFromFiles_OneFile: File_Title_Header4 set\n");
		fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
		          File_Title_Header4) - 4 * sizeof(short int);
	} else {
		NVRAM_LOG("NVM_RestoreFromFiles_OneFile: File_Title_Header5 set\n");
		fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
		          File_Title_Header5);
	}
	int fis = sizeof(File_Title);

	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;
	int filesizeintable = 0;
	if ((filename == NULL) && (file_lid >= 0)) {
		NVRAM_LOG("NVM_RestoreFromFiles_OneFile : %d ++\n", file_lid);
		NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);

		if (!NVM_CheckVerFile(true)) {
			NVM_GenerateFileVer(true);
		}

		//get the file informatin table.
		pCfgFielTable = NVM_GetCfgFileTable(file_lid);
		if (pCfgFielTable == NULL) {
			NVRAM_LOG("NVM_RestoreFromFiles_OneFile: NVM_GetCfgFileTable Fail!!!\n");
			return 0;
		}


		if (file_lid == iFileVerInfoLID) {
			if (!NVM_GenerateFileVer(true)) {
				return 0;
			}
			NVRAM_LOG("NVM_RestoreFromFiles_OneFile:Wrong file_lid Fail!!!\n");
			return 0;
		}

		if (file_lid >= iCustomBeginLID) {
			iRealFileLid = file_lid - iCustomBeginLID;
		} else {
			iRealFileLid = file_lid;
		}

	} else {
		NVRAM_LOG("NVM_RestoreFromFiles_OneFile(MD): %s ++\n", filename);
	}
	/* malloc the buffer of title buf */
	FileInfo = (File_Title *)malloc(sizeof(File_Title));
	memset(FileInfo, 0, sizeof(File_Title));

	iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
	iFileDesc_map = open(g_pcNVM_AllMap, O_RDWR);

	if ( iFileDesc_map < 0) {
		NVBAK_LOG("NVM_RestoreFromFiles_OneFile cannot open allmap file\n");
	}

	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		/* Error handling */
		NVBAK_LOG("NVM_RestoreFromFiles_OneFile cannot open file data\n");
		free(FileInfo);
		if ( iFileDesc_map > 0)
		close(iFileDesc_map);
		return false;
	}

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		/* Error handling */
		NVBAK_LOG("NVM_RestoreFromFiles_OneFile cannot open map data\n");
		close(iFileDesc_file);
		free(FileInfo);
		return false;
	}

	lseek(iFileDesc_map, sizeof(unsigned int) + 2 * sizeof(unsigned int), SEEK_SET);
	if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support())
		iSize = (int)read(iFileDesc_map, &FileTitleInfo1, sizeof(File_Title_Header1));
	else if (nvram_md5_support() && !nvram_evdo_support()
	         && !nvram_ecci_c2k_support())
		iSize = (int)read(iFileDesc_map, &FileTitleInfo2,
		                  sizeof(File_Title_Header2) - 2 * sizeof(short
		                          int)); //In order to align, minus 2*sizeof(short int)

	else if (!nvram_md5_support() && nvram_evdo_support()
	         && !nvram_ecci_c2k_support())
		iSize = (int)read(iFileDesc_map, &FileTitleInfo3,
		                  sizeof(File_Title_Header3) - 4 * sizeof(short
		                          int)); //In order to align, minus 4*sizeof(short int)

	else if (!nvram_md5_support() && !nvram_evdo_support()
	         && !nvram_ecci_c2k_support())
		iSize = (int)read(iFileDesc_map, &FileTitleInfo4,
		                  sizeof(File_Title_Header4) - 4 * sizeof(short int));
	else
		iSize = (int)read(iFileDesc_map, &FileTitleInfo5, sizeof(File_Title_Header5));
#if 0

	NVBAK_LOG("File_Title_Header iApBootNum(%d)", FileTitleInfo1.iApBootNum);
	NVBAK_LOG("File_Title_Header iApCleanNum(%d)", FileTitleInfo1.iApCleanNum);
	NVBAK_LOG("File_Title_Header iMdBootNum(%d)", FileTitleInfo1.iMdBootNum);
	NVBAK_LOG("File_Title_Header iMdCleanNum(%d)", FileTitleInfo1.iMdCleanNum);
	NVBAK_LOG("File_Title_Header iMdImpntNum(%d)", FileTitleInfo1.iMdImpntNum);
	NVBAK_LOG("File_Title_Header iMdCoreNum(%d)", FileTitleInfo1.iMdCoreNum);
	NVBAK_LOG("File_Title_Header iMdDataNum(%d)", FileTitleInfo1.iMdDataNum);
	if (nvram_platform_header_offset == 0) {
		//Add for second modem for MT658*
		NVBAK_LOG("File_Title_Header iMd2BootNum(%d)", FileTitleInfo1.iMd2BootNum);
		NVBAK_LOG("File_Title_Header iMd2CleanNum(%d)", FileTitleInfo1.iMd2CleanNum);
		NVBAK_LOG("File_Title_Header iMd2ImpntNum(%d)", FileTitleInfo1.iMd2ImpntNum);
		//End of Comment
		if (nvram_md5_support()) {
			NVBAK_LOG("File_Title_Header iMd5BootNum(%d)", FileTitleInfo1.iMd5BootNum);
			NVBAK_LOG("File_Title_Header iMd5CleanNum(%d)", FileTitleInfo1.iMd5CleanNum);
			NVBAK_LOG("File_Title_Header iMd5ImpntNum(%d)", FileTitleInfo1.iMd5ImpntNum);
		}
	}
	if (nvram_evdo_support())
		NVBAK_LOG("File_Title_Header iViaNum(%d)", FileTitleInfo1.iViaNum);

	if (nvram_platform_header_offset == 0) {

		NVBAK_LOG("File_Title_Header iFileBufLen(%d)", FileTitleInfo1.iFileBufLen);
		NVBAK_LOG("File_Title_Header BackupFlag(%d)", FileTitleInfo1.BackupFlag);
	}

	if (nvram_platform_header_offset != 0) {
		NVBAK_LOG("File_Title_Header iFileBufLen(%d)",
		          *((short int *) & (FileTitleInfo1.iFileBufLen) - nvram_platform_header_offset));
		NVBAK_LOG("File_Title_Header BackupFlag(%d)",
		          *((short int *) & (FileTitleInfo1.BackupFlag) - nvram_platform_header_offset));
	}

#endif
	if (nvram_platform_header_offset != 0) {
		fhs = fhs - nvram_platform_header_offset * 2;
	}
	switch (eBackupType) {
	case APBOOT:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APBOOT start !");
			iFileNum = FileTitleInfo1.iApBootNum;
			iFileTitleOffset = fhs;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APBOOT start !");
			iFileNum = FileTitleInfo2.iApBootNum;
			iFileTitleOffset = fhs;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APBOOT start !");
			iFileNum = FileTitleInfo3.iApBootNum;
			iFileTitleOffset = fhs;
		} else if (!nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APBOOT start !");
			iFileNum = FileTitleInfo4.iApBootNum;
			iFileTitleOffset = fhs;
		} else {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APBOOT start !");
			iFileNum = FileTitleInfo5.iApBootNum;
			iFileTitleOffset = fhs;
		}
		break;

	case APCLN:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APCLN start !");
			iFileNum = FileTitleInfo1.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APCLN start !");
			iFileNum = FileTitleInfo2.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APCLN start !");
			iFileNum = FileTitleInfo3.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum) * fis;
		} else if (!nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APCLN start !");
			iFileNum = FileTitleInfo4.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum) * fis;
		} else {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile APCLN start !");
			iFileNum = FileTitleInfo5.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum) * fis;
		}
		break;

	case MDBOOT:
		if (nvram_md5_support() && nvram_evdo_support()  && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDBOOT start !");
			iFileNum = FileTitleInfo1.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum +
			                          FileTitleInfo1.iApCleanNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDBOOT start !");
			iFileNum = FileTitleInfo2.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum +
			                          FileTitleInfo2.iApCleanNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDBOOT start !");
			iFileNum = FileTitleInfo3.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum +
			                          FileTitleInfo3.iApCleanNum) * fis;
		} else if (!nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDBOOT start !");
			iFileNum = FileTitleInfo4.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum +
			                          FileTitleInfo4.iApCleanNum) * fis;
		} else {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDBOOT start !");
			iFileNum = FileTitleInfo5.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum +
			                          FileTitleInfo5.iApCleanNum) * fis;
		}

		break;

	case MDCLN:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCLN start !");
			iFileNum = FileTitleInfo1.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCLN start !");
			iFileNum = FileTitleInfo2.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
			                          + FileTitleInfo2.iMdBootNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCLN start !");
			iFileNum = FileTitleInfo3.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum) * fis;
		} else if (!nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCLN start !");
			iFileNum = FileTitleInfo4.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
			                          + FileTitleInfo4.iMdBootNum) * fis;
		} else {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCLN start !");
			iFileNum = FileTitleInfo5.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum) * fis;
		}
		break;

	case MDIMP:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDIMP start !");
			iFileNum = FileTitleInfo1.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDIMP start !");
			iFileNum = FileTitleInfo2.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
			                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDIMP start !");
			iFileNum = FileTitleInfo3.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum) * fis;
		}

		else if (!nvram_md5_support() && !nvram_evdo_support()
		         && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDIMP start !");
			iFileNum = FileTitleInfo4.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
			                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum) * fis;
		} else {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDIMP start !");
			iFileNum = FileTitleInfo5.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum) * fis;
		}
		break;

	case MDCOR:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCOR start !");
			iFileNum = FileTitleInfo1.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
			                          FileTitleInfo1.iMdImpntNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCOR start !");
			iFileNum = FileTitleInfo2.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
			                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
			                          FileTitleInfo2.iMdImpntNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCOR start !");
			iFileNum = FileTitleInfo3.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
			                          FileTitleInfo3.iMdImpntNum) * fis;
		}

		else if (!nvram_md5_support() && !nvram_evdo_support()
		         && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCOR start !");
			iFileNum = FileTitleInfo4.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
			                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum +
			                          FileTitleInfo4.iMdImpntNum) * fis;
		} else {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDCOR start !");
			iFileNum = FileTitleInfo5.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum) * fis;
		}
		break;

	case MDDATA:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDDATA start !");
			iFileNum = FileTitleInfo1.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
			                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDDATA start !");
			iFileNum = FileTitleInfo2.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
			                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
			                          FileTitleInfo2.iMdImpntNum + FileTitleInfo2.iMdCoreNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDDATA start !");
			iFileNum = FileTitleInfo3.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
			                          FileTitleInfo3.iMdImpntNum + FileTitleInfo3.iMdCoreNum) * fis;
		}

		else if (!nvram_md5_support() && !nvram_evdo_support()
		         && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDDATA start !");
			iFileNum = FileTitleInfo4.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
			                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum +
			                          FileTitleInfo4.iMdImpntNum + FileTitleInfo4.iMdCoreNum) * fis;
		} else {
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile MDDATA start !");
			iFileNum = FileTitleInfo5.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum) * fis;
		}
		break;

	//Add for second modem for MT658*
	case MD2BOOT:

		if (nvram_platform_header_offset == 0) {
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2BOOT start !");
				iFileNum = FileTitleInfo1.iMd2BootNum;
				iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
				                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
				                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum +
				                          FileTitleInfo1.iMdDataNum) * fis;
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2BOOT start !");
				iFileNum = FileTitleInfo2.iMd2BootNum;
				iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
				                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
				                          FileTitleInfo2.iMdImpntNum + FileTitleInfo2.iMdCoreNum +
				                          FileTitleInfo2.iMdDataNum) * fis;
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2BOOT start !");
				iFileNum = FileTitleInfo3.iMd2BootNum;
				iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
				                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
				                          FileTitleInfo3.iMdImpntNum + FileTitleInfo3.iMdCoreNum +
				                          FileTitleInfo3.iMdDataNum) * fis;
			}

			else if (!nvram_md5_support() && !nvram_evdo_support()
			         && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2BOOT start !");
				iFileNum = FileTitleInfo4.iMd2BootNum;
				iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
				                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum +
				                          FileTitleInfo4.iMdImpntNum + FileTitleInfo4.iMdCoreNum +
				                          FileTitleInfo4.iMdDataNum) * fis;
			} else {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2BOOT start !");
				iFileNum = FileTitleInfo5.iMd2BootNum;
				iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
				                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
				                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
				                          FileTitleInfo5.iMdDataNum) * fis;
			}
		}
		break;

	case MD2CLN:
		if (nvram_platform_header_offset == 0) {

			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2CLN start !");
				iFileNum = FileTitleInfo1.iMd2CleanNum;
				iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
				                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
				                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum +
				                          FileTitleInfo1.iMdDataNum + FileTitleInfo1.iMd2BootNum) * fis;
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2CLN start !");
				iFileNum = FileTitleInfo2.iMd2CleanNum;
				iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
				                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
				                          FileTitleInfo2.iMdImpntNum + FileTitleInfo2.iMdCoreNum +
				                          FileTitleInfo2.iMdDataNum + FileTitleInfo2.iMd2BootNum) * fis;
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2CLN start !");
				iFileNum = FileTitleInfo3.iMd2CleanNum;
				iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
				                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
				                          FileTitleInfo3.iMdImpntNum + FileTitleInfo3.iMdCoreNum +
				                          FileTitleInfo3.iMdDataNum + FileTitleInfo3.iMd2BootNum) * fis;
			}

			else if (!nvram_md5_support() && !nvram_evdo_support()
			         && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2CLN start !");
				iFileNum = FileTitleInfo4.iMd2CleanNum;
				iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
				                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum +
				                          FileTitleInfo4.iMdImpntNum + FileTitleInfo4.iMdCoreNum +
				                          FileTitleInfo4.iMdDataNum + FileTitleInfo4.iMd2BootNum) * fis;
			} else {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2CLN start !");
				iFileNum = FileTitleInfo5.iMd2CleanNum;
				iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
				                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
				                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
				                          FileTitleInfo5.iMdDataNum + FileTitleInfo5.iMd2BootNum) * fis;
			}
		}
		break;

	case MD2IMP:
		if (nvram_platform_header_offset == 0) {
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2IMP start !");
				iFileNum = FileTitleInfo1.iMd2ImpntNum;
				iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
				                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
				                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum +
				                          FileTitleInfo1.iMdDataNum + FileTitleInfo1.iMd2BootNum +
				                          FileTitleInfo1.iMd2CleanNum) * fis;
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2IMP start !");
				iFileNum = FileTitleInfo2.iMd2ImpntNum;
				iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
				                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
				                          FileTitleInfo2.iMdImpntNum + FileTitleInfo2.iMdCoreNum +
				                          FileTitleInfo2.iMdDataNum + FileTitleInfo2.iMd2BootNum +
				                          FileTitleInfo2.iMd2CleanNum) * fis;
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2IMP start !");
				iFileNum = FileTitleInfo3.iMd2ImpntNum;
				iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
				                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
				                          FileTitleInfo3.iMdImpntNum + FileTitleInfo3.iMdCoreNum +
				                          FileTitleInfo3.iMdDataNum + FileTitleInfo3.iMd2BootNum +
				                          FileTitleInfo3.iMd2CleanNum) * fis;
			}

			else if (!nvram_md5_support() && !nvram_evdo_support()
			         && !nvram_ecci_c2k_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2IMP start !");
				iFileNum = FileTitleInfo4.iMd2ImpntNum;
				iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
				                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum +
				                          FileTitleInfo4.iMdImpntNum + FileTitleInfo4.iMdCoreNum +
				                          FileTitleInfo4.iMdDataNum + FileTitleInfo4.iMd2BootNum +
				                          FileTitleInfo4.iMd2CleanNum) * fis;
			} else {
				NVBAK_LOG("FileOp_RestoreFromFiles MD2IMP start !");
				iFileNum = FileTitleInfo5.iMd2ImpntNum;
				iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
				                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
				                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
				                          FileTitleInfo5.iMdDataNum + FileTitleInfo5.iMd2BootNum +
				                          FileTitleInfo5.iMd2CleanNum) * fis;
			}
		}
		break;
	//End of Comment

	case MD5BOOT:
		if (nvram_platform_header_offset == 0 && nvram_md5_support()
		        && !nvram_ecci_c2k_support()) {
			if (nvram_evdo_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD5BOOT start !");
				iFileNum = FileTitleInfo1.iMd5BootNum;
				iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
				                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
				                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum +
				                          FileTitleInfo1.iMdDataNum + FileTitleInfo1.iMd2BootNum +
				                          FileTitleInfo1.iMd2CleanNum + FileTitleInfo1.iMd2ImpntNum) * fis;
			} else {
				NVBAK_LOG("FileOp_RestoreFromFiles MD5BOOT start !");
				iFileNum = FileTitleInfo2.iMd5BootNum;
				iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
				                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
				                          FileTitleInfo2.iMdImpntNum + FileTitleInfo2.iMdCoreNum +
				                          FileTitleInfo2.iMdDataNum + FileTitleInfo2.iMd2BootNum +
				                          FileTitleInfo2.iMd2CleanNum + FileTitleInfo2.iMd2ImpntNum) * fis;
			}
		} else if (nvram_platform_header_offset == 0 && nvram_md5_support()
		           && nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MD5BOOT start !");
			iFileNum = FileTitleInfo5.iMd5BootNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
			                          FileTitleInfo5.iMdDataNum + FileTitleInfo5.iMd2BootNum +
			                          FileTitleInfo5.iMd2CleanNum + FileTitleInfo5.iMd2ImpntNum) * fis;
		}
		break;

	case MD5CLN:
		if (nvram_platform_header_offset == 0 && nvram_md5_support()
		        && !nvram_ecci_c2k_support()) {
			if (nvram_evdo_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD5CLN start !");
				iFileNum = FileTitleInfo1.iMd5CleanNum;
				iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
				                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
				                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum +
				                          FileTitleInfo1.iMdDataNum + FileTitleInfo1.iMd2BootNum +
				                          FileTitleInfo1.iMd2CleanNum + FileTitleInfo1.iMd2ImpntNum +
				                          FileTitleInfo1.iMd5BootNum) * fis;
			} else {
				NVBAK_LOG("FileOp_RestoreFromFiles MD5CLN start !");
				iFileNum = FileTitleInfo2.iMd5CleanNum;
				iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
				                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
				                          FileTitleInfo2.iMdImpntNum + FileTitleInfo2.iMdCoreNum +
				                          FileTitleInfo2.iMdDataNum + FileTitleInfo2.iMd2BootNum +
				                          FileTitleInfo2.iMd2CleanNum + FileTitleInfo2.iMd2ImpntNum +
				                          FileTitleInfo2.iMd5BootNum) * fis;
			}
		} else if (nvram_platform_header_offset == 0 && nvram_md5_support()
		           && nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MD5CLN start !");
			iFileNum = FileTitleInfo5.iMd5CleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
			                          FileTitleInfo5.iMdDataNum + FileTitleInfo5.iMd2BootNum +
			                          FileTitleInfo5.iMd2CleanNum + FileTitleInfo5.iMd2ImpntNum +
			                          FileTitleInfo5.iMd5BootNum) * fis;
		}
		break;

	case MD5IMP:
		if (nvram_platform_header_offset == 0 && nvram_md5_support()
		        && !nvram_ecci_c2k_support()) {
			if (nvram_evdo_support()) {
				NVBAK_LOG("FileOp_RestoreFromFiles MD5IMP start !");
				iFileNum = FileTitleInfo1.iMd5ImpntNum;
				iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
				                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
				                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum +
				                          FileTitleInfo1.iMdDataNum + FileTitleInfo1.iMd2BootNum +
				                          FileTitleInfo1.iMd2CleanNum + FileTitleInfo1.iMd2ImpntNum +
				                          FileTitleInfo1.iMd5BootNum + FileTitleInfo1.iMd5CleanNum) * fis;
			} else {
				NVBAK_LOG("FileOp_RestoreFromFiles MD5IMP start !");
				iFileNum = FileTitleInfo2.iMd5ImpntNum;
				iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
				                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
				                          FileTitleInfo2.iMdImpntNum + FileTitleInfo2.iMdCoreNum +
				                          FileTitleInfo2.iMdDataNum + FileTitleInfo2.iMd2BootNum +
				                          FileTitleInfo2.iMd2CleanNum + FileTitleInfo2.iMd2ImpntNum +
				                          FileTitleInfo2.iMd5BootNum + FileTitleInfo2.iMd5CleanNum) * fis;
			}
		} else if (nvram_platform_header_offset == 0 && nvram_md5_support()
		           && nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MD5IMP start !");
			iFileNum = FileTitleInfo5.iMd5ImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
			                          FileTitleInfo5.iMdDataNum + FileTitleInfo5.iMd2BootNum +
			                          FileTitleInfo5.iMd2CleanNum + FileTitleInfo5.iMd2ImpntNum +
			                          FileTitleInfo5.iMd5BootNum + FileTitleInfo5.iMd5CleanNum) * fis;
		}

		break;
	case VIA:
		if (nvram_platform_header_offset == 0 && nvram_md5_support()
		        && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles VIA start !");
			iFileNum = FileTitleInfo1.iViaNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
			                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum +
			                          FileTitleInfo1.iMdDataNum + FileTitleInfo1.iMd2BootNum +
			                          FileTitleInfo1.iMd2CleanNum + FileTitleInfo1.iMd2ImpntNum +
			                          FileTitleInfo1.iMd5BootNum + FileTitleInfo1.iMd5CleanNum +
			                          FileTitleInfo1.iMd5ImpntNum) * fis;
		} else if (nvram_platform_header_offset == 0 && !nvram_md5_support()
		           && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles VIA start !");
			iFileNum = FileTitleInfo3.iViaNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
			                          FileTitleInfo3.iMdImpntNum + FileTitleInfo3.iMdCoreNum +
			                          FileTitleInfo3.iMdDataNum + FileTitleInfo3.iMd2BootNum +
			                          FileTitleInfo3.iMd2CleanNum + FileTitleInfo3.iMd2ImpntNum) * fis;
		} else if (nvram_platform_header_offset != 0 && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles VIA start !");
			iFileNum = FileTitleInfo3.iViaNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
			                          FileTitleInfo3.iMdImpntNum + FileTitleInfo3.iMdCoreNum +
			                          FileTitleInfo3.iMdDataNum) * fis;
		} else
			break;
		break;

	case MD3BOOT:
		if (nvram_platform_header_offset == 0 && nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MD3BOOT start !");
			iFileNum = FileTitleInfo5.iMd3BootNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
			                          FileTitleInfo5.iMdDataNum + FileTitleInfo5.iMd2BootNum +
			                          FileTitleInfo5.iMd2CleanNum + FileTitleInfo5.iMd2ImpntNum +
			                          FileTitleInfo5.iMd5BootNum + FileTitleInfo5.iMd5CleanNum +
			                          FileTitleInfo5.iMd5ImpntNum) * fis;
		}
		break;

	case MD3CLN:
		if (nvram_platform_header_offset == 0 && nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MD3CLN start !");
			iFileNum = FileTitleInfo5.iMd3CleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
			                          FileTitleInfo5.iMdDataNum + FileTitleInfo5.iMd2BootNum +
			                          FileTitleInfo5.iMd2CleanNum + FileTitleInfo5.iMd2ImpntNum +
			                          FileTitleInfo5.iMd5BootNum + FileTitleInfo5.iMd5CleanNum +
			                          FileTitleInfo5.iMd5ImpntNum + FileTitleInfo5.iMd3BootNum) * fis;
		}
		break;


	case MD3IMP:
		if (nvram_platform_header_offset == 0 && nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MD3IMP start !");
			iFileNum = FileTitleInfo5.iMd3ImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum + FileTitleInfo5.iMdCoreNum +
			                          FileTitleInfo5.iMdDataNum + FileTitleInfo5.iMd2BootNum +
			                          FileTitleInfo5.iMd2CleanNum + FileTitleInfo5.iMd2ImpntNum +
			                          FileTitleInfo5.iMd5BootNum + FileTitleInfo5.iMd5CleanNum +
			                          FileTitleInfo5.iMd5ImpntNum + FileTitleInfo5.iMd3BootNum +
			                          FileTitleInfo5.iMd3CleanNum) * fis;
		}
		break;

	case ALL:
		break;
	default:
		close(iFileDesc_map);
		close(iFileDesc_file);
		free(FileInfo);
		return false;
		//break;
	}

	iRet = lseek(iFileDesc_map, iFileTitleOffset, SEEK_SET);

	//restore all files one by one according to the fout.txt and fTitleInfo.txt
	//do
	//avoid the file num of some folder is zero
	while (iFileNum > 0  && iFileNum < MAX_FILENUM) {
		iSize = (int)read(iFileDesc_map, FileInfo, sizeof(File_Title));
		FileInfo->cFileName[MAX_NAMESIZE-1]='\0';
		if ((filename == NULL) && (file_lid >= 0) && (pCfgFielTable != NULL)) {
			if (strcmp(FileInfo->cFileName, pCfgFielTable[iRealFileLid].cFileName) != 0) {
				--iFileNum;
				continue;
			}
		} else if ((filename != NULL) && (strncmp("/nvdata", filename, 7) == 0)) {
			NVBAK_LOG("filename+7 %s\n", filename + 7);
			NVBAK_LOG("FileInfo->cFileName + 11 %s\n", FileInfo->cFileName + 11);

			if (strcmp(FileInfo->cFileName + 11, filename + 7) != 0) { //compare string after /mnt/vendor/nvdata and /nvdata
				--iFileNum;
				continue;
			}
		} else if (filename != NULL){
			if (strcmp(FileInfo->cFileName, filename) != 0) {
				--iFileNum;
				continue;
			}
		} else {
		    NVBAK_LOG("cFileName or filename == NULL\n");
		}
		*find_flag = 1;
		iFileNum = 0;
		NVBAK_LOG("FileInfo: %s\n", FileInfo->cFileName);
		if ((filename == NULL) && (file_lid >= 0) && (pCfgFielTable != NULL)) {
			filesizeintable = pCfgFielTable[iRealFileLid].i4RecNum *
			                  pCfgFielTable[iRealFileLid].i4RecSize;
			NVBAK_LOG("FileInfo: startaddr(0x%x) size(0x%x) size in table(0x%x).\n",
			          FileInfo->FielStartAddr, FileInfo->Filesize, filesizeintable);
			if (FileInfo->Filesize != filesizeintable + 2) {
				NVBAK_LOG("Restored file size error !");
				close(iFileDesc_map);
				close(iFileDesc_file);
				free(FileInfo);
				return false;
			}
		}
        NVBAK_LOG("FileInfo->Filesize=%d,INT_MAX=%d\n",FileInfo->Filesize,INT_MAX);
        if(FileInfo->Filesize <=0 || FileInfo->Filesize >=INT_MAX) {
            NVBAK_LOG("FileInfo->Filesize overflow\n");
            close(iFileDesc_map);
            close(iFileDesc_file);
            free(FileInfo);
            return false;
        }
		buf = (char *)malloc(FileInfo->Filesize);

		//read the data and write to the file
		iRet = lseek(iFileDesc_file, FileInfo->FielStartAddr, SEEK_SET);
		if (iRet == -1) {
			NVBAK_LOG("lseek fail !");
			free(buf);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(FileInfo);
			return false;
		}
		iSize = (int)read(iFileDesc_file, buf, FileInfo->Filesize);
		if (iSize != FileInfo->Filesize) {
			NVBAK_LOG("read fail !iSize=%d,FileInfo->Filesize=%d\n", iSize,
			          FileInfo->Filesize);
			free(buf);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(FileInfo);
			return false;
		}

		iFileDesc = open_file_with_dirs(FileInfo->cFileName, O_CREAT | O_TRUNC | O_RDWR,
		                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );

		if (INVALID_HANDLE_VALUE == iFileDesc) {
			/* Error handling */
			NVBAK_LOG("NVM_RestoreFromFiles_OneFile cannot create %s\n",
			          FileInfo->cFileName);

			//added
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(buf);
			free(FileInfo);
			return false;
		}
		iSize = (int)write(iFileDesc, buf, FileInfo->Filesize);
		if (iSize != FileInfo->Filesize) {
			NVBAK_LOG("write fail !iSize=%d,FileInfo->Filesize=%d\n", iSize,
			          FileInfo->Filesize);
			close(iFileDesc);
			free(buf);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(FileInfo);
			return false;
		}
		close(iFileDesc);
		free(buf);
		if ((filename == NULL) && (file_lid >= 0)) {

#ifndef MTK_BASIC_PACKAGE
			if (!NVM_Enc_Dnc_File(file_lid, DECODE)) {
				NVRAM_LOG("Dnc fail!!!\n");
				close(iFileDesc_map);
				close(iFileDesc_file);
				free(FileInfo);
				return false;
			}
#endif

#if 0
			if (NVM_ProtectDataFile(file_lid, true) == 1) {
				NVRAM_LOG("NVM_RestoreFromFiles_OneFile ProtectDataFile Success!!\n");
			} else {
				NVRAM_LOG("NVM_RestoreFromFiles_OneFile ProtectDataFile SET Fail!!\n");
				return false;
			}
#endif
			if (NVM_CheckFile(FileInfo->cFileName)
			        && (NVM_CheckFileSize(iRealFileLid, pCfgFielTable) != -1)) {
				NVRAM_LOG("NVM_RestoreFromFiles_OneFile ProtectDataFile Success!!\n");
			} else {
				NVRAM_LOG("NVM_RestoreFromFiles_OneFile ProtectDataFile Fail!!\n");
				close(iFileDesc_map);
				close(iFileDesc_file);
				free(FileInfo);
				return false;
			}
		}
		bRet = true;
		break;
	}

	close(iFileDesc_map);
	close(iFileDesc_file);
	free(FileInfo);
	return bRet;
}

bool NVM_CheckData_OneFile(int file_lid, const char * filename) {
	bool bRet = false;
	int eBackupType;
	unsigned int ulSavedCheckSum;
	unsigned int ulCheckSum;
	struct stat st;
	unsigned int iFileSize;
	bool find_flag = 0;
	int iFileMask[ALL];
	memset(iFileMask, 0, (ALL * (sizeof(int))));
	if (!nvram_md5_support() && !nvram_evdo_support()) {
		iFileMask[5] = 1;
		iFileMask[6] = 1;
		iFileMask[10] = 1;
		iFileMask[11] = 1;
		iFileMask[12] = 1;
		iFileMask[13] = 1;
	} else if (!nvram_md5_support() && nvram_evdo_support()) {
		iFileMask[5] = 1;
		iFileMask[6] = 1;
		iFileMask[10] = 1;
		iFileMask[11] = 1;
		iFileMask[12] = 1;
	} else if (nvram_md5_support() && !nvram_evdo_support()) {
		iFileMask[5] = 1;
		iFileMask[6] = 1;
		iFileMask[13] = 1;
	} else {
		iFileMask[5] = 1;
		iFileMask[6] = 1;
	}
	if (!nvram_ecci_c2k_support()) {
		iFileMask[14] = 1;
		iFileMask[15] = 1;
		iFileMask[16] = 1;
	}

	NVBAK_LOG("[NVM_CheckData_OneFile] start !");

	if (!NVM_GetCheckSum()) {
		NVBAK_LOG("[NVM_CheckData_OneFile] GetCheckSum Fail !");
		return false;
	}
	ulSavedCheckSum = stBackupFileInfo.ulCheckSum;
	ulCheckSum = NVM_ComputeCheckSum();
	NVBAK_LOG("ulCheckSun:%u\n", ulCheckSum);
	if (stat(g_pcNVM_AllFile, &st) < 0) {
		NVBAK_LOG("Error NVM_CheckData_OneFile stat \n");
		return false;
	}
	iFileSize = st.st_size;
	NVBAK_LOG("iFileSize:%d\n", iFileSize);
	ulCheckSum ^= iFileSize;

	NVBAK_LOG("NVM_CheckData_OneFile:%x,%x", ulSavedCheckSum, ulCheckSum);
	if (!NVM_CompareCheckSum(ulSavedCheckSum, ulCheckSum)) {
		NVBAK_LOG("check sum not match!");
		return false;
	}

	for (eBackupType = APBOOT; eBackupType < ALL; eBackupType++) {
		if (iFileMask[eBackupType] == 1)
			continue;
		if (find_flag == 0)
			bRet = NVM_RestoreFromFiles_OneFile(eBackupType, file_lid, filename,
			                                    &find_flag);
		else
			break;
	}

	NVBAK_LOG("[NVM_CheckData_OneFile] end !");
	return bRet;
}

bool NVM_RestoreFromBinRegion_OneFile(int file_lid, const char * filename) {
	int iFileDesc_file, iFileDesc_map, fd, iResult, iBlockNum, iFileDesc_backupflag;
	unsigned int iMapFileSize, iDatFileSize, iBlockSize, iPartitionSize,
	         iAPBackupFileNum,iWriteSize;
//   int BinRegionBlockTotalNum,NvRamBlockNum,forDMBlockNum=2;
	unsigned short iMDBackupFileNum;
	bool bRet = true;
	char cMtdDevName[128] = {0};
	char *tempBuffer = NULL;
	char *tempBitmap1 = NULL;
	char *tempBitmap2 = NULL;
	int i, j, pos = 0, flag = 0;
	bool bSuccessFound = false;
	struct mtd_info_user info;
	NVBAK_LOG("[NVRAM]:NVM_RestoreFromBinRegion_OneFile\n");
	//int iBackup_Partition=get_partition_numb("nvram");

	(void)(iFileDesc_backupflag);

	//fix nvram layout issue , reassgined block numerbers by different platform
	if (NVM_Init() < 0) {
		NVBAK_LOG("[NVRAM]:NVM_RestoreFromBinRegion_OneFile--NVM_Init fail !!!\n");
		return false;
	}

	if ((file_lid < 0) && (filename == NULL)) {
		NVBAK_LOG("[NVRAM]:NVM_RestoreFromBinRegion_OneFile bad arg\n");
		return false;
	}
	memset(cMtdDevName, 0, sizeof cMtdDevName);
        //strcpy(cMtdDevName, g_NVRAM_BACKUP_DEVICE);
        snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);
	NVBAK_LOG("[NVRAM]:cMtdDevName:%s\n", cMtdDevName);

	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:NVM_RestoreFromBinRegion_OneFile get device info fail!!!\n");
	}

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error %s\r\n", (char*)strerror(errno));
		return false;
	}
	iBlockSize = info.erasesize;
	iPartitionSize = info.size;
   iWriteSize = info.writesize;

	if (nvram_platform_layout_version == 0)
		BinRegionBlockTotalNum = info.size / iBlockSize - nvram_platform_log_block -
		                         nvram_platform_resv_block;
	else if (nvram_platform_layout_version == 1)
		BinRegionBlockTotalNum = info.size / iBlockSize;
	else {
		NVBAK_LOG("[NVRAM]: invalid nvram layout version %d\r\n",
		          nvram_platform_layout_version);
		close(fd);
		return false;
	}

	NvRamBlockNum = BinRegionBlockTotalNum - nvram_platform_DM_block;

	tempBuffer = (char*)malloc(iBlockSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		return false;
	}
lseek(fd, pos * iBlockSize, SEEK_SET);
	iResult = read(fd, tempBuffer, iBlockSize);

	iMapFileSize = *((unsigned int*)tempBuffer); // get the mapfile
	iDatFileSize = *((unsigned int*)(tempBuffer + 4));
	NVBAK_LOG("[NVRAM]: map file:%d,dat file:%d\n", iMapFileSize, iDatFileSize);
	if(iMapFileSize == 0 || iMapFileSize == 0xFFFFFFFF) {
		NVBAK_LOG("[NVRAM]:map file size error\r\n");
		free(tempBuffer);
		close(fd);
		return false;
	}
	if(iDatFileSize == 0 || iDatFileSize == 0xFFFFFFFF) {
		NVBAK_LOG("[NVRAM]:data file size error\r\n");
		free(tempBuffer);
		close(fd);
		return false;
	}
	unsigned int iMemSize;
	int iReservePos = 0;
	int iReserveBlockCount = 0;
	if ((iMapFileSize + 3 * sizeof(unsigned int)) % iWriteSize != 0)
		iMemSize = (((iMapFileSize + 3 * sizeof(unsigned int)) / iWriteSize) + 1) *
		           iWriteSize;
	else
		iMemSize = iMapFileSize + 3 * sizeof(unsigned
		                                     int); //mapfile size, datfile size, cleanboot fla
	NVBAK_LOG("[NVRAM Restore - heli]:map file size:%d\n", iMemSize);

	if ( iMemSize > iBlockSize ) { // > 180KB
		iReservePos = 1;
		iReserveBlockCount = 2;
	} else { // < 128KB
		iReservePos = 0;
		iReserveBlockCount = 1;
	}

	free(tempBuffer);
	tempBuffer = NULL;
	tempBuffer = (char*)malloc(iBlockSize *
	                           iReserveBlockCount); // for 128KB and 256KB
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		return false;
	}
	iBlockNum = NvRamBlockNum;
	NVBAK_LOG("[NVRAM]:iBlockNum:%d\n", iBlockNum);

	tempBitmap1 = (char *)malloc(NvRamBlockNum);
	if (tempBitmap1 == NULL) {
		NVBAK_LOG("[NVRAM]:malloc tempBitmap1 Fail!!\r\n");
		free(tempBuffer);
		close(fd);
		return false;
	}
	tempBitmap2 = (char *)malloc(NvRamBlockNum);
	if (tempBitmap2 == NULL) {
		NVBAK_LOG("[NVRAM]:malloc tempBitmap2 Fail!!\r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		return false;
	}

	int iBitmapFlag = 0;
	while (iBlockNum > 0) {
		flag = 0;
		iBlockNum--;
		NVBAK_LOG("[NVRAM]:iBlockNum:%d\n", iBlockNum);
      lseek(fd, (iBlockNum - iReservePos)*iBlockSize, SEEK_SET);
		iResult = read(fd, tempBuffer, iBlockSize);
		NVBAK_LOG("[NVRAM]:read:%d\n", iResult);
		if (iResult <= 0) {
			NVBAK_LOG("[NVRAM]:read size error\r\n");
			close(fd);
			free(tempBuffer);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}
		memcpy(tempBitmap1, tempBuffer, NvRamBlockNum * sizeof(char));
		memcpy(tempBitmap2, tempBuffer + NvRamBlockNum * sizeof(char),
		       NvRamBlockNum * sizeof(char));
		for (i = 0; i < NvRamBlockNum; i++) {
			if (tempBitmap1[i] != tempBitmap2[i]) {
				NVBAK_LOG("[NVRAM]:1i:%d,1:%d,2:%d\n", i, tempBitmap1[i], tempBitmap2[i]);
				flag = 1;
				break;
			}
		}
		if (flag)
			continue;
		memcpy(tempBitmap2, tempBuffer + 2 * NvRamBlockNum * sizeof(char),
		       NvRamBlockNum * sizeof(char));
		for (i = 0; i < NvRamBlockNum; i++) {
			if (tempBitmap1[i] != tempBitmap2[i]) {
				NVBAK_LOG("[NVRAM]:2i:%d,1:%d,2:%d\n", i, tempBitmap1[i], tempBitmap2[i]);
				flag = 1;
				break;
			}
		}
		if (flag)
			continue;
		memcpy(&iBitmapFlag, tempBuffer + 3 * NvRamBlockNum * sizeof(char),
		       sizeof(unsigned int));
		if (iBitmapFlag != (int)BitmapFlag) {
			NVBAK_LOG("[NVRAM]:iBitMapFlag:%d,BitMapFlag:%d\n", iBitmapFlag, BitmapFlag);
			continue;
		}
		bSuccessFound = true;
		break;
	}
	if (!bSuccessFound) {
		NVBAK_LOG("[NVRAM]:can not find bad block bit map\r\n");
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	#if 0
	for (i = 0; i < NvRamBlockNum; i++) {
		NVBAK_LOG("[NVRAM_Bitmap]:Block%d,%d\n", i, tempBitmap1[i]);
	}
	#endif
	for (i = 0; i < iBlockNum; i++) {
		if (tempBitmap1[i] == 0) {
			pos = i;
			break;
		}
	}
	if (i == iBlockNum) {
		NVBAK_LOG("[NVRAM]:can not find map file\r\n");
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	lseek(fd, pos * iBlockSize, SEEK_SET);
   iResult = read(fd, tempBuffer,
				  iBlockSize * iReserveBlockCount); // if mapfile > 128KB read 2*128KB
	NVBAK_LOG("[NVRAM]:map file read size:%d\n", iResult);
	if (iResult <= 0) {
		NVBAK_LOG("[NVRAM]:read size error\r\n");
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	iMapFileSize = *((unsigned int*)tempBuffer);
	iDatFileSize = *((unsigned int*)(tempBuffer + 4));
	iAPBackupFileNum = *((unsigned int*)(tempBuffer + 3 * sizeof(
	        unsigned int) + iMapFileSize));
	iMDBackupFileNum = *((unsigned short*)(tempBuffer + 3 * sizeof(
	        unsigned int) + iMapFileSize + sizeof(unsigned int)));
	NVBAK_LOG("[NVRAM]: map file:%d,dat file:%d\n", iMapFileSize, iDatFileSize);
	/* NVBAK_LOG("[NVRAM]: AP backup file num:%d,MD backup file num:%d\n",iAPBackupFileNum,iMDBackupFileNum);
	if(iAPBackupFileNum != 0 ){
		iFileDesc_backupflag = open(g_pcNVM_BackFlag , O_TRUNC|O_CREAT|O_RDWR, S_IRUSR|S_IWUSR| S_IRGRP |S_IWGRP);
		if(INVALID_HANDLE_VALUE == iFileDesc_backupflag)
			{
	    		NVBAK_LOG("[NVRAM]: cannot open backup file num data\n");
				close(iFileDesc_backupflag);
	    		close(fd);
				free(tempBuffer);
				free(tempBitmap1);
				free(tempBitmap2);
	    		return false;
			}
		result=write(iFileDesc_backupflag,&iAPBackupFileNum,sizeof(unsigned int));
		if(result!=(int)sizeof(unsigned int))
		{
	  		NVBAK_LOG("[NVRAM]:map file write error\r\n");
	  		close(fd);
	  		free(tempBuffer);
	  		close(iFileDesc_backupflag);
			free(tempBitmap1);
			free(tempBitmap2);
	  		return false;
			}
		}*/
	iFileDesc_file = open(g_pcNVM_AllFile , O_TRUNC | O_CREAT | O_RDWR,
	                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	iFileDesc_map = open(g_pcNVM_AllMap, O_TRUNC | O_CREAT | O_RDWR,
	                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		NVBAK_LOG("[NVRAM]: cannot open file data\n");
		if (iFileDesc_map != INVALID_HANDLE_VALUE)
			close(iFileDesc_map);
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		NVBAK_LOG("[NVRAM]: cannot open map data\n");
		close(iFileDesc_file);
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}

	iResult = write(iFileDesc_map, tempBuffer + 3 * sizeof(unsigned int),
	                iMapFileSize);
	if (iResult != (int)iMapFileSize) {
		NVBAK_LOG("[NVRAM]:map file write error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}

	free(tempBuffer);
	tempBuffer = NULL;
	if (iDatFileSize % iBlockSize != 0)
		tempBuffer = (char*)malloc((iDatFileSize / iBlockSize + 1) * iBlockSize);
	else
		tempBuffer = (char*)malloc(iDatFileSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	int iFreeBlockNum = 0;
	for (i = pos + 1 + iReservePos; i < iBlockNum;
	        i++) { // heli if > 126KB need pos + 2
   	  if(tempBitmap1[i]==0)
	  	{
			pos = i;
			break;
		}
	}
	if (i == iBlockNum) {
		NVBAK_LOG("[NVRAM]:there are not enough good blocks for read nvram data file\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	} else {
		for (; i < iBlockNum; i++) {
			if (tempBitmap1[i] == 0)
				iFreeBlockNum++;
		}
		if ((iFreeBlockNum * iBlockSize) < (iDatFileSize)) {
			NVBAK_LOG("[NVRAM]:there are not enough good blocks for read  nvram data file\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}
	}
	int iReadTime = 0;
	if (iDatFileSize % iBlockSize != 0)
		iReadTime = iDatFileSize / iBlockSize + 1;
	else
		iReadTime = iDatFileSize / iBlockSize;
	int iAlreadyRead = 0;
	NVBAK_LOG("[NVRAM]:dat file read begin:%d\n", pos);
	for (i = 0; i < iReadTime; i++) {
		iResult = lseek(fd, pos * iBlockSize, SEEK_SET);
		if (iResult != pos * (int)iBlockSize) {
			NVBAK_LOG("[NVRAM]:binregion lseek error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}

		iResult = read(fd, tempBuffer + i * iBlockSize, iBlockSize);
		NVBAK_LOG("[NVRAM]:dat file read size:%d\n", iResult);
		if (iResult != (int)iBlockSize) {
			NVBAK_LOG("[NVRAM]:bin region read error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}
		iAlreadyRead++;
		if (iAlreadyRead == iReadTime)
			break;
		for (j = pos + 1; j < iBlockNum; j++) {
			if (tempBitmap1[j] == 0) {
				pos = j;
				break;
			}
		}
		if (j >= iBlockNum) {
			NVBAK_LOG("[NVRAM]:there are not enough good blocks to read nvram data file\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}
	}
	NVBAK_LOG("[NVRAM]:dat file read end:%d\n", pos);
	iResult = write(iFileDesc_file, tempBuffer, iDatFileSize);
	if (iResult != (int)iDatFileSize) {
		NVBAK_LOG("[NVRAM]:dat file write error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	close(fd);
	free(tempBuffer);
	close(iFileDesc_file);
	close(iFileDesc_map);
	free(tempBitmap1);
	free(tempBitmap2);


	bRet = NVM_CheckData_OneFile(file_lid, filename);
	return bRet;

}

/*******************************************************************************
//FUNCTION:
//NVM_ComputeCheckNo
//DESCRIPTION:
//		this function is called to compute CheckNo of file in NvRam
//PARAMETERS:
//		filepath:[IN] the path name of the file
//		ContentSize:[IN] the content size of the file
//		CheckNo_flag:[IN] the flag of CheckNo in the file
//RETURN VALUE:
//		CheckNo
//DEPENDENCY:
//		NVM_CheckFileSize must be called
//GLOBALS AFFECTED:
//		None
********************************************************************************/
char NVM_ComputeCheckNo(const char * filepath, char *pCheckNo_flag,
                        bool IS_OLD_FILE) {
	int iFileDesc = 0;
	unsigned int i = 0, ilooptimes = 0;
	bool flag = 0;
	char buf, cCheckNo;
	struct stat st;
    if (filepath == NULL)
        {
            NVRAM_LOG("NVM_ComputeCheckNo: filepath == NULL!!\n");
            return 0;
        }
	if (stat(filepath, &st) < 0) {
		NVRAM_LOG("Error NVM_ComputeCheckNo stat!\n");
		*pCheckNo_flag = 0xFF;
		return 0;
	}
	iFileDesc = open(filepath, O_RDONLY);
	if (iFileDesc == -1) { //if file doesn't exist
		NVRAM_LOG("NVM_ComputeCheckNo:Open file failed!");
		*pCheckNo_flag = 0xFF;
		return 0;
	}
	memset(&buf, 0, sizeof(char));
	memset(&cCheckNo, 0, sizeof(char));
	if (IS_OLD_FILE)
		ilooptimes = (st.st_size - 2 * sizeof(char)) / sizeof(char);
	else
		ilooptimes = st.st_size / sizeof(char);
	for (i = 0; i < ilooptimes; ++i) {
		if (sizeof(char) == read(iFileDesc, &buf, sizeof(char))) {
			if (flag) {
				cCheckNo ^= buf;
				flag = false;
			} else {
				cCheckNo += buf;
				flag = true;
			}
		} else {
			NVRAM_LOG("NVM_ComputeCheckNo:Read file failed!");
			*pCheckNo_flag = 0xFF;
			close(iFileDesc);
			return 0;
		}

	}
	close(iFileDesc);
	return cCheckNo;
}
/********************************************************************************
//FUNCTION:
//		NVM_CheckFileSize
//DESCRIPTION:
//		this function is called to Check the size of the file in NvRam contain the check bytes.
//
//PARAMETERS:
//		file_lid:[IN] the lid of the file
//
//RETURN VALUE:
//		true is exist, otherwise is fail
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
int NVM_CheckFileSize(int iRealFileLid, const TCFG_FILE *pCfgFielTable) {
	struct stat st;
	if (stat(pCfgFielTable[iRealFileLid].cFileName, &st) < 0) {
		NVRAM_LOG("NVM_CheckFileSize:stat Fail!!!\n");
		return -1;
	}

	//NVRAM_LOG("NVM_CheckFileSize:stat_size:%lld,size in table:%d\n", st.st_size,
	//          pCfgFielTable[iRealFileLid].i4RecNum * pCfgFielTable[iRealFileLid].i4RecSize);
	switch (pCfgFielTable[iRealFileLid].stDefualType) {
	case DEFAULT_ZERO:
	case DEFAULT_FF:		// when the type of default value is DEFAULT_ZERO/DEFAULT_FF
	case SIGNLE_DEFUALT_REC:
		if (((long long)pCfgFielTable[iRealFileLid].i4RecNum) * pCfgFielTable[iRealFileLid].i4RecSize
		        == st.st_size)
			return 0;
		else if (((long long)pCfgFielTable[iRealFileLid].i4RecNum) *
		         pCfgFielTable[iRealFileLid].i4RecSize == st.st_size - 2 * sizeof(char))
			return 1;
		else {
			NVRAM_LOG("NVM_CheckFileSize:File size not match!!!\n");
			return -1;
		}
		break;

	case MULTIPLE_DEFUALT_REC:	// when the type of default value is MULTIPLE_DEFUALT_REC, we use the defined default value of one record to generate file
		if (((long long)pCfgFielTable[iRealFileLid].i4RecNum) * pCfgFielTable[iRealFileLid].i4RecNum
		        * pCfgFielTable[iRealFileLid].i4RecSize == st.st_size)
			return 0;
		else if (((long long)pCfgFielTable[iRealFileLid].i4RecNum) *
		         pCfgFielTable[iRealFileLid].i4RecNum * pCfgFielTable[iRealFileLid].i4RecSize ==
		         st.st_size - 2 * sizeof(char))
			return 1;
		else {
			NVRAM_LOG("NVM_CheckFileSize:File size not match!!!\n");
			return -1;
		}
		break;
	default:
		break;

	}
	return -1;
}
/********************************************************************************
//FUNCTION:
//		NVM_SetCheckNo
//DESCRIPTION:
//		this function is called to set the CheckNo of the file in NvRam.
//
//PARAMETERS:
//		filepath:[IN] the path name of the file
//
//RETURN VALUE:
//		true is success, otherwise is fail
//
//DEPENDENCY:
//		NVM_CheckFileSize must be called
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_SetCheckNo(const char *filepath, bool isOldFile) {
	char cCheckNo = 0;
	char cCheckFlag = 0;
	int iFileDesc;
	char cFlagValue = 0xAA;

	cCheckNo = NVM_ComputeCheckNo(filepath, &cCheckFlag, isOldFile);
	NVRAM_LOG("NVM_SetCheckNo: CheckNo:%x\n", cCheckNo);
	NVRAM_LOG("NVM_SetCheckNo: CheckFlag:%x\n", cCheckFlag);
	if (cCheckFlag == 0xFF) {
		NVRAM_LOG("NVM_SetCheckNo: ComputeCheckNoFail!!\n");
		return 0;
	}
	iFileDesc = open(filepath, O_RDWR | O_SYNC);
	if (iFileDesc == -1) { //if file doesn't exist
		NVRAM_LOG("NVM_SetCheckNo:Open file failed!");
		return 0;
	}
	if (isOldFile) {
		if (lseek(iFileDesc, -2 * (int)sizeof(char), SEEK_END) < 0) {
			NVRAM_LOG("NVM_SetCheckNo: (OldFile) lseek Fail!!\n");
			close(iFileDesc);
			return 0;
		}
	} else {
		if (lseek(iFileDesc, 0, SEEK_END) < 0) {
			NVRAM_LOG("NVM_SetCheckNo: (Newfile) lseek Fail!!\n");
			close(iFileDesc);
			return 0;
		}
	}
	if (sizeof(char) != write(iFileDesc, &cFlagValue, sizeof(char))) {
		NVRAM_LOG("NVM_SetCheckNo: write Check flag Fail!!\n");
		close(iFileDesc);
		return 0;
	}
	if (sizeof(char) == write(iFileDesc, &cCheckNo, sizeof(char))) {
		//NVRAM_LOG("NVM_SetCheckNo: CheckNo generate successfully!\n");
		close(iFileDesc);
		return 1;
	} else {
		NVRAM_LOG("NVM_SetCheckNo: CheckNo generate Fail!\n");
		close(iFileDesc);
		return 0;
	}
}


/********************************************************************************
//FUNCTION:
//		NVM_CheckFile
//DESCRIPTION:
//		this function is called to Check file in NvRam correct.
//
//PARAMETERS:
//		filepath:[IN] the path name of the file
//
//RETURN VALUE:
//		true is correct (1), otherwise is fail(0)
//
//DEPENDENCY:
//		NVM_CheckFileSize must be called
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_CheckFile(const char * filepath) {
	char cCheckNo = 0;
	char cCheckNo_file[2];
	char cCheckFlag = 0;
	int iFileDesc;
	char cFlagValue = 0xAA;
	bool ISOLDFILE = true;
    if (filepath == NULL)
        {
            NVRAM_LOG("NVM_CheckFile: filepath == NULL!!\n");
            return 0;
        }
	memset(cCheckNo_file, 0, 2 * sizeof(char));

	cCheckNo = NVM_ComputeCheckNo(filepath, &cCheckFlag, ISOLDFILE);
	if (cCheckFlag == 0xFF) {
		NVRAM_LOG("NVM_CheckFile: ComputeCheckNo Fail!!\n");
		return 0;
	}
	iFileDesc = open(filepath, O_RDONLY);
	if (iFileDesc == -1) { //if file doesn't exist
		NVRAM_LOG("NVM_CheckFile:Open file failed!");
		return 0;
	}
	if (lseek(iFileDesc, -2 * sizeof(char), SEEK_END) < 0) {
		NVRAM_LOG("NVM_CheckFile: lseek Fail!!\n");
		close(iFileDesc);
		return 0;
	} else {
		if (2 * sizeof(char) == read(iFileDesc, cCheckNo_file, 2 * sizeof(char)))
			if (cCheckNo_file[0] == cFlagValue) {
				if (cCheckNo_file[1] == cCheckNo) {
					//NVRAM_LOG("NVM_CheckFile: File is correct!\n");
					close(iFileDesc);
					return 1;

				} else {
					NVRAM_LOG("NVM_CheckFile: File has been modified!!\n");
					close(iFileDesc);
					return 0;
				}
			} else {
				NVRAM_LOG("NVM_CheckFile: Check flag in File has been destroyed!!\n");
				close(iFileDesc);
				return 0;
			}
		else {
			NVRAM_LOG("NVM_CheckFile: Read file Fail!!\n");
			close(iFileDesc);
			return 0;
		}

	}

}
/********************************************************************************
//FUNCTION:
//		NVM_ProtectUserData
//DESCRIPTION:
//		this function is called to Protect User's Data.
//
//PARAMETERS:
//		filepath:[IN] the path name of the file
//
//RETURN VALUE:
//		true is correct (1), otherwise is fail(0)
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
//int loop_Reset_Protect = 0;
pthread_mutex_t recoverlock = PTHREAD_MUTEX_INITIALIZER;

int NVM_ProtectDataFile(int file_lid, bool Setflag) {
	int CheckFileSize_flag = 0;
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;
	bool restore_flag = false;
	int i = 0, j = 0;
	char filename[MAX_NAMESIZE];
	memset(filename, 0, MAX_NAMESIZE);

	NVRAM_LOG("NVM_ProtectDataFile : %d ++\n", file_lid);
	//NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);

	if (!NVM_CheckVerFile(true)) {
		NVM_GenerateFileVer(true);
	}

	//get the file informatin table.
	pCfgFielTable = NVM_GetCfgFileTable(file_lid);
	if (pCfgFielTable == NULL) {
		NVRAM_LOG("NVM_ProtectDataFile: NVM_GetCfgFileTable Fail!!!\n");
		return -1;
	}


	if (file_lid == iFileVerInfoLID) {
		if (!NVM_GenerateFileVer(true)) {
			return -1;
		}
		NVRAM_LOG("NVM_ProtectDataFile:Wrong file_lid Fail!!!\n");
		return -1;
	}

	if (file_lid >= iCustomBeginLID) {
		iRealFileLid = file_lid - iCustomBeginLID;
	} else {
		iRealFileLid = file_lid;
	}

	CheckFileSize_flag = NVM_CheckFileSize(iRealFileLid, pCfgFielTable);
	//NVRAM_LOG("NVM_ProtectUserData\n");
	if (CheckFileSize_flag == -1) {
		NVRAM_LOG("NVM_ProtectUserData:File Size Error!!!\n");
		goto restore;
	}
	if (Setflag || CheckFileSize_flag == 0) {
		//NVRAM_LOG("NVM_ProtectUserData:Set Check Num\n");
		if (!NVM_SetCheckNo(pCfgFielTable[iRealFileLid].cFileName,
		                    CheckFileSize_flag)) {
			NVRAM_LOG("NVM_ProtectUserData:Set Check Num Fail!!!\n");
			return 0;
		} else {
			NVRAM_LOG("NVM_ProtectUserData:Set Check Num Success\n");
			return 1;
		}

	}
	if (!Setflag) {
		//NVRAM_LOG("NVM_ProtectUserData:Check File\n");
		if (!NVM_CheckFile(pCfgFielTable[iRealFileLid].cFileName)) {
			NVRAM_LOG("NVM_ProtectUserData:Check Failed!!!\n");
			goto restore;
		} else {
			NVRAM_LOG("NVM_ProtectUserData:Check Success\n");
			return 1;
		}
	}

restore:
	for (j = strlen(pCfgFielTable[iRealFileLid].cFileName) - 1; j >= 0; j--)
		if (pCfgFielTable[iRealFileLid].cFileName[j] == '/') {
					//strcpy(filename,pCfgFielTable[iRealFileLid].cFileName+j+1);
					snprintf(filename, sizeof(filename),"%s", (char *)(pCfgFielTable[iRealFileLid].cFileName+j+1));
			NVRAM_LOG("filename:%s\n", filename);
		break;
		}
	for (i = 0; i < (int)g_Backup_File_Count; i++) {
		if (0 == strcmp(filename, aBackupToBinRegion[i].cFileName)
		        && (unsigned int)file_lid == aBackupToBinRegion[i].iLID) {
			restore_flag = true;
			break;
		}
	}
	NVRAM_LOG("NVM_ProtectUserData: Restore or Reset!\n");
	pthread_mutex_lock(&recoverlock);
	if (!restore_flag || (restore_flag
	                      && !NVM_RestoreFromBinRegion_OneFile(file_lid, NULL))) {
		pthread_mutex_unlock(&recoverlock);
		if (restore_flag)
			NVRAM_LOG("NVM_ProtectUserData Restore Fail! Reset!!\n");
		else
			NVRAM_LOG("NVM_ProtectUserData Reset\n");
		if (!NVM_ResetFileToDefault(file_lid)) {
			NVRAM_LOG("NVM_ProtectUserData Reset Fail!!\n");
			return 0;
		} else {
#ifndef MTK_BASIC_PACKAGE
			if (!NVM_Enc_Dnc_File(file_lid, DECODE)) {
				NVRAM_LOG("Dec fail!!!\n");
				return false;
			}
#endif
			NVRAM_LOG("NVM_ProtectUserData Reset Success\n");
			return 1;
		}
	} else {
		pthread_mutex_unlock(&recoverlock);
		NVRAM_LOG("NVM_ProtectUserData Restore Success\n");
		return 1;
	}
}
/********************************************************************************
//FUNCTION:
//		NVM_ResetFileToDefault
//DESCRIPTION:
//		this function is called to reset a NvRam to default value.
//
//PARAMETERS:
//		file_lid: [IN] the lid of nvram file
//
//RETURN VALUE:
//		true is success, otherwise is fail
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
extern FileName aPerformance[];
extern const unsigned int g_Performance_File_Count;
bool NVM_ResetFileToDefault(int file_lid) {
	int iFileDesc;
	char *databuf = NULL;
	int i = 0;
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;
	int i4RecNum, i4RecSize;

	NVRAM_LOG("NVM_ResetFileToDefault : %d ++\n", file_lid);
	NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);

	if (!NVM_CheckVerFile(true)) {
		if (false == NVM_GenerateFileVer(true)) {
			NVRAM_LOG("GenerateFileVer Fail!\n");
			return false;
		}
	}

	//get the file informatin table.
	pCfgFielTable = NVM_GetCfgFileTable(file_lid);
	if (pCfgFielTable == NULL) {
		NVRAM_LOG("NVM_GetCfgFileTable Fail!!!\n");
		return false;
	}

	// if the file lid is version file,
	// just generate it directly by call NVM_GenerateFileVer(...)
	//if (file_lid == AP_CFG_FILE_VER_INFO_LID)
	if (file_lid == iFileVerInfoLID) {
		if (!NVM_GenerateFileVer(true)) {
			return false;
		}
		return true;
	}

	// calculate the offset lid in the file inforamtion table.
	// because of custom's lid is from AP_CFG_CUSTOM_BEGIN_LID,
	// so the offset(iRealFileLid) lid is file_lid - AP_CFG_CUSTOM_BEGIN_LID*/
	//if (file_lid >= AP_CFG_CUSTOM_BEGIN_LID) {
	if (file_lid >= iCustomBeginLID) {
		iRealFileLid = file_lid - iCustomBeginLID;
	} else {
		iRealFileLid = file_lid;
	}

	switch (pCfgFielTable[iRealFileLid].stDefualType) {
	case DEFAULT_ZERO:
	case DEFAULT_FF:		// when the type of default value is DEFAULT_ZERO/DEFAULT_FF, we don't care the default vlaue
		iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName,
		                 O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

		if (iFileDesc == -1) {
			iFileDesc = open_file_with_dirs(pCfgFielTable[iRealFileLid].cFileName,
			                                O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
			NVRAM_LOG("Create the dir path of %s\n", pCfgFielTable[iRealFileLid].cFileName);
		}

		if (iFileDesc == -1) {
			NVRAM_LOG("Error NVM_ResetFileToDefault can't open file %s\n",
			          pCfgFielTable[iRealFileLid].cFileName);
			return false;
		}

		i4RecNum = pCfgFielTable[iRealFileLid].i4RecNum;
		i4RecSize = pCfgFielTable[iRealFileLid].i4RecSize;

		databuf = (char*)malloc(i4RecSize * i4RecNum);
		if (databuf == NULL) {
			NVRAM_LOG("malloc databuf failed!\n");
			close(iFileDesc);
			return false;
		}

		if (pCfgFielTable[iRealFileLid].stDefualType == DEFAULT_ZERO) {
			memset(databuf, 0, i4RecSize * i4RecNum);
		} else {
			memset(databuf, 0xff, i4RecSize * i4RecNum);
		}
		write(iFileDesc, databuf, i4RecSize * i4RecNum);

		if (databuf != NULL) {
			free(databuf);
		}

		close(iFileDesc);
		break;

	case SIGNLE_DEFUALT_REC:	// when the type of default value is SIGNLE_DEFUALT_REC, we use the defined default vlaue to generate file
		iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName,
		                 O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

		if (iFileDesc == -1) {
			iFileDesc = open_file_with_dirs(pCfgFielTable[iRealFileLid].cFileName,
			                                O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
			NVRAM_LOG("Create the dir path of %s\n", pCfgFielTable[iRealFileLid].cFileName);
		}

		if (iFileDesc == -1) {
			NVRAM_LOG("Error NVM_ResetFileToDefault can't open file %s\n",
			          pCfgFielTable[iRealFileLid].cFileName);
			return false;
		}

		i4RecNum = pCfgFielTable[iRealFileLid].i4RecNum;
		i4RecSize = pCfgFielTable[iRealFileLid].i4RecSize;

		write(iFileDesc, pCfgFielTable[iRealFileLid].pDefualtVaule,
		      i4RecSize * i4RecNum);
		close(iFileDesc);
		break;

	case MULTIPLE_DEFUALT_REC:	// when the type of default value is MULTIPLE_DEFUALT_REC, we use the defined default value of one record to generate file
		iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName,
		                 O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

		if (iFileDesc == -1) {
			iFileDesc = open_file_with_dirs(pCfgFielTable[iRealFileLid].cFileName,
			                                O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
			NVRAM_LOG("Create the dir path of %s\n", pCfgFielTable[iRealFileLid].cFileName);
		}

		if (iFileDesc == -1) {
			NVRAM_LOG("Error NVM_ResetFileToDefault can't open file %s\n",
			          pCfgFielTable[iRealFileLid].cFileName);
			return false;
		}

		if ((pCfgFielTable[iRealFileLid].pDefualtVaule == NULL)
		        || (pCfgFielTable[iRealFileLid].i4RecNum < 2)) {

			NVRAM_LOG("NVM_ResetFileToDefault Mulitple para is error \n");
			close(iFileDesc);
			return false;
		}

		i4RecNum = pCfgFielTable[iRealFileLid].i4RecNum;
		i4RecSize = pCfgFielTable[iRealFileLid].i4RecSize;

		//use one record to generate all record value
		for (i = 0; i < i4RecNum; i++) {
			write(iFileDesc, pCfgFielTable[iRealFileLid].pDefualtVaule,
			      i4RecSize * i4RecNum);
		}

		close(iFileDesc);
		break;

	default:
		break;

	}

	NVM_UpdateFileVerNo(file_lid, VerUpdate);
	unsigned int index = 0;
	bool maskflag = 0;
	for (index = 0; index < g_Performance_File_Count; index++) {
		if ((unsigned int)file_lid == aPerformance[index].iLID) {
			maskflag = 1;
			break;
		}
	}
	if (maskflag == 0) {
#if 0
		if (NVM_ProtectDataFile(file_lid, Setflag) == 1) {
			NVRAM_LOG("NVM_ResetFileToDefault ProtectDataFile Success!!\n");
		} else {
			NVRAM_LOG("NVM_ResetFileToDefault ProtectDataFile SET Fail!!\n");
			return false;
		}

#endif
		NVRAM_LOG("NVM_ResetFileToDefault :Set Check Num\n");
		if (!NVM_SetCheckNo(pCfgFielTable[iRealFileLid].cFileName, false)) {
			NVRAM_LOG("NVM_ResetFileToDefault :Set Check Num Fail!!!\n");
			return false;
		} else {
			NVRAM_LOG("NVM_ResetFileToDefault :Set Check Num Success\n");
		}
		NVRAM_LOG("NVM_ResetFileToDefault Success!!\n");
	}
#ifndef MTK_BASIC_PACKAGE
	if (!NVM_Enc_Dnc_File(file_lid, ENCODE)) {
		NVRAM_LOG("Enc fail!!!\n");
		return false;
	}
#endif
	NVRAM_LOG("NVM_ResetFileToDefault : %d --\n", file_lid);
	return true;
}


/********************************************************************************
//FUNCTION:
//		NVM_CmpFileVerNo
//DESCRIPTION:
//		this function is called to compare file version between FAT2 and default version.
//
//PARAMETERS:
//		file_lid: [IN] the lid of nvram file
//
//RETURN VALUE:
//		true is same, otherwise is not same
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_CmpFileVerNo(int file_lid) {
	int iFileDesc;
	char cFbyte[4] = {0, 0, 0, 0};
	int i = 0, iResult;
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;
    char tempstr[FILENAMELENGTH]={0};
    char tempstr1[FILENAMELENGTH]={0};
	bool bIsNewVerInfo;

	NVRAM_LOG("NVM_CmpFileVerNo %d  \n" , file_lid);
	//get the file informatin table. if the file lid is version file, just generate it directly by call GenerateFileVer
	pCfgFielTable = NVM_GetCfgFileTable(file_lid);
	if (pCfgFielTable == NULL) {
		NVRAM_LOG("NVM_GetCfgFileTable Fail!!!\n");
		return false;
	}

	//if (file_lid >= AP_CFG_CUSTOM_BEGIN_LID) {
	if (file_lid >= iCustomBeginLID) {
		//iRealFileLid = file_lid - AP_CFG_CUSTOM_BEGIN_LID;
		iRealFileLid = file_lid - iCustomBeginLID;
	} else {
		iRealFileLid = file_lid;
	}
	int iStrLen;
	iStrLen = strlen(pCfgFielTable[iRealFileLid].cFileName);
	for (i = iStrLen; i >= 0; i--) {
		if (pCfgFielTable[iRealFileLid].cFileName[i] == '/') {
                    //strcpy(tempstr1,pCfgFielTable[iRealFileLid].cFileName+i+1);//get the target name
                    snprintf(tempstr1, sizeof(tempstr1),"%s", pCfgFielTable[iRealFileLid].cFileName+i+1);
			break;
		}
	}
	if (i < 0) {
		NVRAM_LOG("Path parse Fail!!!\n");
		return false;
	}

#if 1
	//NVRAM_LOG("Check if FILE_VER exists before openning it!\n");
	if (!NVM_CheckVerFile(true)) {
		if (false == NVM_GenerateFileVer(true)) {
			NVRAM_LOG("GenerateFileVer Fail!\n");
			return false;
		}
	}
#endif

	//compare the file version
	iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_RDONLY);
	if (iFileDesc == -1) {
		NVRAM_LOG("Error NVM_CmpFileVerNo Open2 version file fail: %s\n",
		          g_akCFG_File[iFileVerInfoLID].cFileName);
		return false;
	}
    iResult = read(iFileDesc,tempstr,FILENAMELENGTH-1);
	if (iResult == -1) { NVRAM_LOG("read failed with errno=%d\n", errno); iResult = 0; }
	tempstr[iResult]='\0';
	if (0 == strcmp(tempstr, strVerInfo)) {
		//NVRAM_LOG("New version info file\n");
		bIsNewVerInfo = true;
	} else {
		//NVRAM_LOG("Old version info file\n");
		bIsNewVerInfo = false;
	}
	if (bIsNewVerInfo) {
		memset(tempstr, 0, FILENAMELENGTH);
		if (-1 == lseek(iFileDesc, (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH),
		                SEEK_SET)) {
			NVRAM_LOG("Error NVM_CmpFileVerNo lseek version file fail: %s\n",
			          g_akCFG_File[iFileVerInfoLID].cFileName);
			close(iFileDesc);
			return false;
		}
    		iResult = read(iFileDesc, tempstr, FILENAMELENGTH-1);
			if (iResult == -1) { NVRAM_LOG("read failed with errno=%d\n", errno); iResult = 0; }
			tempstr[iResult]='\0';
		if (0 == strcmp(tempstr, tempstr1)) {
			if (-1 == lseek(iFileDesc,
			                (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH) + FILENAMELENGTH, SEEK_SET)) {
				NVRAM_LOG("Error NVM_CmpFileVerNo lseek version file fail: %s\n",
				          g_akCFG_File[iFileVerInfoLID].cFileName);
				close(iFileDesc);
				return false;
			}
			read(iFileDesc, cFbyte, FILEVERLENGTH);
		} else {
			//search the version info from the file
			struct stat statbuf;
			int iFileSize, ilooptime;
			fstat(iFileDesc, &statbuf);
			iFileSize = (int)statbuf.st_size;
			ilooptime = iFileSize / (FILEVERLENGTH + FILENAMELENGTH);
			for (i = 0; i < ilooptime; i++) {
				if (-1 == lseek(iFileDesc, i * (FILEVERLENGTH + FILENAMELENGTH), SEEK_SET)) {
					NVRAM_LOG("Error NVM_CmpFileVerNo lseek version file fail: %s\n",
					          g_akCFG_File[iFileVerInfoLID].cFileName);
					close(iFileDesc);
					return false;
				}
    		    iResult = read(iFileDesc, tempstr, FILENAMELENGTH-1);
				if (iResult == -1) { NVRAM_LOG("read failed with errno=%d\n", errno); iResult = 0; }
				tempstr[iResult]='\0';
				if (0 == strcmp(tempstr, tempstr1)) {
					memset(tempstr, 0, FILENAMELENGTH);
                                if (-1 == lseek(iFileDesc, i * (FILEVERLENGTH + FILENAMELENGTH)+FILENAMELENGTH, SEEK_SET)) {
                                        NVRAM_LOG("Error NVM_CmpFileVerNo lseek version file fail: %s\n",
                                            g_akCFG_File[iFileVerInfoLID].cFileName);
                                        close(iFileDesc);
                                        return false;
                                }
					read(iFileDesc, cFbyte, FILEVERLENGTH);
                                        NVRAM_LOG("FILEVER size change, Load File Version: %s, NvRam File Version: %s\n",
                                            pCfgFielTable[iRealFileLid].cFileVer, cFbyte);
					break;
				} else {
					memset(tempstr, 0, FILENAMELENGTH);
					continue;
				}
			}
			if (i == ilooptime) {
				NVRAM_LOG("Error NVM_CmpFileVerNo find target file fail: %d\n", file_lid);
				close(iFileDesc);
				return false;
			}
		}
	} else {
		if (-1 == lseek(iFileDesc, file_lid * FILEVERLENGTH, SEEK_SET)) {
			NVRAM_LOG("Error NVM_CmpFileVerNo lseek version file fail: %s\n",
			          g_akCFG_File[iFileVerInfoLID].cFileName);
			close(iFileDesc);
			return false;
		}
		read(iFileDesc, cFbyte, FILEVERLENGTH);
		NVRAM_LOG("!!old version FILEVER, Load File Version: %s, NvRam File Version: %s\n",
		    pCfgFielTable[iRealFileLid].cFileVer, cFbyte);
	}

	close(iFileDesc);
	NVRAM_LOG("Load File Version: %s, NvRam File Version: %s\n",
	          pCfgFielTable[iRealFileLid].cFileVer, cFbyte);
	// compare the version one by one char
	for (i = 0; i < 4; i++) {
		if (pCfgFielTable[iRealFileLid].cFileVer[i] != cFbyte[i]) {
			NVRAM_LOG("Error NVM_CmpFileVerNo is not same %d, %d ,%d\n",
			          iRealFileLid, pCfgFielTable[iRealFileLid].cFileVer[i], cFbyte[i]);
			return false;
		}
	}

	return true;
}

/********************************************************************************
//FUNCTION:
//		NVM_DataVerConvert
//DESCRIPTION:
//		this function is called to convert data acccording to the version info .
//
//PARAMETERS:
//		file_lid: [IN] the lid of nvram file
//RETURN VALUE:
//		convert successfully?
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
int NVM_DataVerConvert(int file_lid) {
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;
	int iFileDesc;
	int iCurrentFileVerNO;
	int iNewFileVerNO;
	int iResult = 1;
     char pFileVerInfo[4]={0};
	int rec_size = 0;
	int rec_num = 0;
	char* pSrcMem = NULL;
	char* pDstMem = NULL;
	struct stat st;
	int iFileSize;
     char tempstr[FILENAMELENGTH]={0};
	bool bIsNewVerInfo;
	bool setflag = true;
	//return 1;
	NVRAM_LOG("NVM_DataVerConvert %d  \n" , file_lid);
	//NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);
	iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_RDONLY);
	if (iFileDesc == -1) {
		NVRAM_LOG("Error NVM_DataVerConvert Open2 version file fail: %s\n",
		          g_akCFG_File[iFileVerInfoLID].cFileName);
		return 0;
	}

	iResult = read(iFileDesc,tempstr,FILENAMELENGTH-1);
	if (iResult == -1) { NVRAM_LOG("read failed with errno=%d\n", errno); iResult = 0; }
	tempstr[iResult]='\0';
	if (0 == strcmp(tempstr, strVerInfo)) {
		NVRAM_LOG("New version info file\n");
		bIsNewVerInfo = true;
	} else {
		NVRAM_LOG("Old version info file\n");
		bIsNewVerInfo = false;
	}

	if (stat(g_akCFG_File[iFileVerInfoLID].cFileName, &st) < 0) {
		NVRAM_LOG("Error NVM_DataVerConvert stat \n");
		close(iFileDesc);
		return 0;
	}
	//if the file lid is greater than the max file lid
	if (bIsNewVerInfo) {
		if (((file_lid + 1 + 1) * (FILEVERLENGTH + FILENAMELENGTH)) > st.st_size) {
			NVRAM_LOG("NVM_DataVerConvert:the file lid is greater than the max file lid, reset it\n");
			if (!NVM_ResetFileToDefault(file_lid)) {
				NVRAM_LOG("reset the file fail\n");
				close(iFileDesc);
				return 0;
			}
			close(iFileDesc);
			return 1;
		}
	} else {
		if (((file_lid + 1) * FILEVERLENGTH) > st.st_size) {
			NVRAM_LOG("NVM_DataVerConvert:the file lid is greater than the max file lid, reset it\n");
			if (!NVM_ResetFileToDefault(file_lid)) {
				NVRAM_LOG("reset the file fail\n");
				close(iFileDesc);
				return 0;
			}
			close(iFileDesc);
			return 1;
		}

	}

	pCfgFielTable = NVM_GetCfgFileTable(file_lid);
	if (pCfgFielTable == NULL) {
		NVRAM_LOG("NVM_GetCfgFileTable Fail!!!\n");
		close(iFileDesc);
		return 0;
	}

	if (file_lid >= iCustomBeginLID) {
		iRealFileLid = file_lid - iCustomBeginLID;
	} else {
		iRealFileLid = file_lid;
	}

	if (bIsNewVerInfo) {
		if (-1 == lseek(iFileDesc,
		                (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH) + FILENAMELENGTH, SEEK_SET)) {
			NVRAM_LOG("Error NVM_DataVerConvert lseek version file fail: %s\n",
			          g_akCFG_File[iFileVerInfoLID].cFileName);
			close(iFileDesc);
			return 0;
		}
	} else {
		if (-1 == lseek(iFileDesc, file_lid * FILEVERLENGTH, SEEK_SET)) {
			NVRAM_LOG("Error NVM_DataVerConvert lseek version file fail: %s\n",
			          g_akCFG_File[iFileVerInfoLID].cFileName);
			close(iFileDesc);
			return 0;
		}
	}

     iResult=read(iFileDesc, pFileVerInfo, FILEVERLENGTH-1);
	 if (iResult == -1) { NVRAM_LOG("read failed with errno=%d\n", errno); iResult = 0; }
	 pFileVerInfo[iResult]='\0';
	close(iFileDesc);

	iNewFileVerNO = atoi(pCfgFielTable[iRealFileLid].cFileVer);
	iCurrentFileVerNO = atoi(pFileVerInfo);

	if (stat(pCfgFielTable[iRealFileLid].cFileName, &st) < 0) {
		NVRAM_LOG("Error NVM_DataVerConvert stat \n");
		return 0;
	}

	iFileSize = st.st_size;
	//iFileDesc=NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
	iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_RDWR);
	if (iFileDesc == -1) {
		NVRAM_LOG("Error NVM_DataVerConvert Open2  file fail: %s\n",
		          pCfgFielTable[iRealFileLid].cFileName);
		return 0;
	}
	pSrcMem = (char*)malloc(iFileSize);
	if (pSrcMem == NULL) {
		NVRAM_LOG("Error NVM_DataVerConvert malloc \n");
		close(iFileDesc);
		return 0;
	}
	memset(pSrcMem, 0, iFileSize);
	if (read(iFileDesc, pSrcMem , iFileSize) < 0) {
		NVRAM_LOG("Read NVRAM fails %d\n", errno);
		close(iFileDesc);
		free(pSrcMem);
		return 0;
	}

	rec_size = pCfgFielTable[iRealFileLid].i4RecSize;
	rec_num = pCfgFielTable[iRealFileLid].i4RecNum;
	pDstMem = (char*)malloc(rec_size * rec_num);
	if (pDstMem == NULL) {
		NVRAM_LOG("Error NVM_DataVerConvert malloc2 \n");
		free(pSrcMem);
		close(iFileDesc);
		return 0;
	}
	memset(pDstMem, 0, rec_size * rec_num);

	//Call Nvram Data Convert Function
	/*
	if(aNvRamConvertFuncTable[file_lid]!=NULL)
	{
	  iResult=aNvRamConvertFuncTable[file_lid](iCurrentFileVerNO,iNewFileVerNO,pSrcMem,pDstMem);
	  if(iResult!=1)
		  {
			 NVRAM_LOG("Error NVM_DataVerConvert fail \n");
			 free(pSrcMem);
			 free(pDstMem);
			 close(iFileDesc);
			 return 0;
		  }
	}
	*/
	if (pCfgFielTable[iRealFileLid].NVM_DataConvertFunc != NULL) {
		iResult = pCfgFielTable[iRealFileLid].NVM_DataConvertFunc(iCurrentFileVerNO,
		          iNewFileVerNO, pSrcMem, pDstMem);
		if (iResult != 1) {
			NVRAM_LOG("Error NVM_DataVerConvert fail \n");
			free(pSrcMem);
			free(pDstMem);
			close(iFileDesc);
			return 0;
		}
	} else {
		NVRAM_LOG("Error NVM_DataVerConvert doesn't exist \n");
		free(pSrcMem);
		free(pDstMem);
		close(iFileDesc);
		return 0;
	}
	close(iFileDesc);
	//Clear the old content of the file
	iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_TRUNC | O_RDWR);
	if (iFileDesc == -1) {
		NVRAM_LOG("Error NVM_DataVerConvert Open2  file fail: %s\n",
		          pCfgFielTable[iRealFileLid].cFileName);
		free(pSrcMem);
		free(pDstMem);
		return 0;
	}
	//NVRAM_LOG("rec_num:%d,%d\n",rec_num,rec_size);
	if (write(iFileDesc, pDstMem , rec_num * rec_size) < 0) {
		printf("WriteFile bt nvram failed:%d\r\n", errno);
		free(pSrcMem);
		free(pDstMem);
		close(iFileDesc);
		return 0;
	}
	free(pSrcMem);
	free(pDstMem);
	close(iFileDesc);
	if (iResult == 1) {
		//if Data convert successfully, the data version info in ver file should also be changed.
		NVM_UpdateFileVerNo(file_lid, VerUpdate);
#if 0
		iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_RDWR);
		if (iFileDesc == -1) {
			NVRAM_LOG("Error NVM_DataVerConvert Open2 version file fail: %s\n",
			          g_akCFG_File[iFileVerInfoLID].cFileName);
			return 0;
		}
		sprintf(pFileVerInfo, "%03d", iNewFileVerNO);
		if (bIsNewVerInfo) {
			if (-1 == lseek(iFileDesc,
			                (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH) + FILENAMELENGTH, SEEK_SET)) {
				NVRAM_LOG("Error NVM_DataVerConvert lseek version file fail: %s\n",
				          g_akCFG_File[iFileVerInfoLID].cFileName);
				close(iFileDesc);
				return 0;
			}
		} else {
			if (-1 == lseek(iFileDesc, file_lid * FILEVERLENGTH, SEEK_SET)) {
				NVRAM_LOG("Error NVM_DataVerConvert lseek version file fail: %s\n",
				          g_akCFG_File[iFileVerInfoLID].cFileName);
				close(iFileDesc);
				return 0;
			}
		}
		write(iFileDesc, pFileVerInfo, FILEVERLENGTH);
		close(iFileDesc);
#endif
	}
	bool maskflag = 0;
	unsigned int index = 0;
	for (index = 0; index < g_Performance_File_Count; index++) {
		if ((unsigned int)file_lid == aPerformance[index].iLID) {
			maskflag = 1;
			break;
		}
	}
	if (maskflag == 0) {
		if (NVM_ProtectDataFile(file_lid, setflag) == 1) {
			NVRAM_LOG("NVM_DataVerConvert ProtectDataFile Success!!\n");
		} else {
			NVRAM_LOG("NVM_DataVerConvert ProtectDataFile SET Fail!!\n");
			return false;
		}
	}
	return iResult;

}
static bool NVM_CheckFileNum(unsigned int iOldCommonFileNum,
                             unsigned int iOldCustomFileNum) {
	unsigned int iLid = 0;
	int iNewCustomFileNum = iNvRamFileMaxLID - iCustomBeginLID;
	if ((iOldCommonFileNum == 0) || (iOldCustomFileNum == 0)) {
		NVRAM_LOG("File Num is zero, maybe this nvram data is from 32mp,48mpv0 or SD update\n");//support for 32mp and 48mpv0
		return true;
	}
	if ((iOldCommonFileNum == (unsigned int)iCustomBeginLID)
	        && (iOldCustomFileNum == (unsigned int)iNewCustomFileNum)) {
		NVRAM_LOG("File Num matches\n");
		return true;
	} else {
		if (iOldCommonFileNum < (unsigned int)iCustomBeginLID
		        && iOldCommonFileNum > 0) {
			for (iLid = iOldCommonFileNum; iLid < (unsigned int)iCustomBeginLID; iLid++) {
				if (!NVM_UpdateFileVerNo(iLid, VerAdd)) {
					NVRAM_LOG("NVM_CheckFileNum: Update File Fail:%d\n", iLid);
					return false;
				}
			}
		}

		if (iOldCustomFileNum < (unsigned int)iNewCustomFileNum
		        && iOldCustomFileNum > 0) {
			for (iLid = (iCustomBeginLID + iOldCustomFileNum);
			        iLid < (unsigned int)(iCustomBeginLID + iNewCustomFileNum); iLid++) {
				if (!NVM_UpdateFileVerNo(iLid, VerAdd)) {
					NVRAM_LOG("NVM_CheckFileNum: Update File Fail:%d\n", iLid);
					return false;
				}
			}
		}

		if (iOldCommonFileNum > (unsigned int)iCustomBeginLID) {
			for (iLid = (unsigned int)iCustomBeginLID; iLid < iOldCommonFileNum; iLid++) {
				if (!NVM_UpdateFileVerNo(iLid, VerDel)) {
					NVRAM_LOG("NVM_CheckFileNum: Update File Version Fail:%d\n", iLid);
					return false;
				}
			}
		}

		if (iOldCustomFileNum > (unsigned int)iNewCustomFileNum) {
			for (iLid = (iOldCommonFileNum + iNewCustomFileNum);
			        iLid < (iOldCommonFileNum + iOldCustomFileNum); iLid++) {
				if (!NVM_UpdateFileVerNo(iLid, VerDel)) {
					NVRAM_LOG("NVM_CheckFileNum: Update File Version Fail:%d\n", iLid);
					return false;
				}
			}
		}

	}

	return true;
}
/********************************************************************************
//FUNCTION:
//		NVM_DataVerConvertAll
//DESCRIPTION:
//		this function is called to convert all data acccording to the version info .
//
//PARAMETERS:
//		file_lid: [IN] the lid of nvram file
//RETURN VALUE:
//		convert successfully?
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_DataVerConvertAll(unsigned int iOldCommonFileNum,
                           unsigned int iOldCustomFileNum) {
	int iLID = 1;
	int iFileDesc = 0;
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid;
	bool bWrongFileVer = false;
	NVRAM_LOG("Enter DataConvert All\n");
	if (iOldCommonFileNum == 0 || iOldCustomFileNum == 0) {
		NVRAM_LOG("File Num is zero, maybe this nvram data is from 32mp,48mpv0 or SD update\n");//support for 32mp and 48mpv0
		return true;
	}
	if (!NVM_CheckVerFile(true)) {
		NVRAM_LOG("fatal error: cna't find version info file\n");
		return false;
	}
	if (!NVM_CheckFileNum(iOldCommonFileNum, iOldCustomFileNum)) {
		NVRAM_LOG("NVM_CheckFileNum fail\n");
		return false;
	}
	for (iLID = 1; iLID < iNvRamFileMaxLID; iLID++) {
		bool bFileExist = true;
		struct stat st;
		memset(&st, 0, sizeof(st));
		pCfgFielTable = NVM_GetCfgFileTable(iLID);
		if (pCfgFielTable == NULL) {
			NVRAM_LOG("NVM_GetCfgFileTable Fail!!!\n");
			return false;
		}

		if (iLID >= iCustomBeginLID) {
			iRealFileLid = iLID - iCustomBeginLID;
		} else {
			iRealFileLid = iLID;
		}
		if (stat(pCfgFielTable[iRealFileLid].cFileName, &st) < 0) {
			NVRAM_LOG("this file doesn't exist, maybe this file is a new file or camera file\n");
			bFileExist = false;
		}

		if (bFileExist) { //Camera NvRam Data should not be checked.
			if (!NVM_CmpFileVerNo(iLID)) {
				NVRAM_LOG("DataConvert Begin\n");
				if (0 == NVM_DataVerConvert(iLID)) {
					//Clear the old content of the file
					iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_TRUNC | O_RDWR);
					if (iFileDesc == -1) {
						NVRAM_LOG("NVM_Clear File Content Fail\n");
						return false;
					}
					close(iFileDesc);

					NVRAM_LOG("Data Convert Fail,reset the file\n");
					if (!NVM_ResetFileToDefault(iLID)) {
						NVRAM_LOG("reset the file fail\n");
						return false;
					}

				}
				NVRAM_LOG("DataConvert End\n");
			} else {
				//if version info is the same ,then output the size info
				NVRAM_LOG("Compare the size of same vesion file\n");

				int iLoadFileSize = (pCfgFielTable[iRealFileLid].i4RecSize) *
				                    (pCfgFielTable[iRealFileLid].i4RecNum);
				NVRAM_LOG("Compare the size,the load file size:%d, the nvram file size:%lld\n",
				          iLoadFileSize, st.st_size);
				if (iLoadFileSize != st.st_size
				        && iLoadFileSize + 2 * (int)sizeof(char) != st.st_size) {
					NVRAM_LOG("NvRam data size can't match between version \n");
					bWrongFileVer = true;
				}

			}

		}
	}
	NVRAM_LOG("Leave DataConvert All\n");
	if (bWrongFileVer == true)
		return false;
	return true;
}
/********************************************************************************
//FUNCTION:
//		NVM_CheckFileID
//DESCRIPTION:
//		this function is called to check the file lid and record id of nvram file.
//
//PARAMETERS:
//		file_lid: 	[IN]		the lid of nvram file
//		rec_id:		[IN]		the record id
//
//RETURN VALUE:
//		true is success, otherwise is fail
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_CheckFileID(int file_lid, int rec_id) {
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;

	NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);
	//get the file informatin table. if the file lid is version file, just generate it directly by call GenerateFileVer
	pCfgFielTable = NVM_GetCfgFileTable(file_lid);

	if (pCfgFielTable == NULL) {
		NVRAM_LOG("NVM_GetCfgFileTable Fail!!!\n");
		return false;
	}

	//if (file_lid >= AP_CFG_CUSTOM_BEGIN_LID) {
	if (file_lid >= iCustomBeginLID) {
		//iRealFileLid = file_lid - AP_CFG_CUSTOM_BEGIN_LID;
		iRealFileLid = file_lid - iCustomBeginLID;
	} else {
		iRealFileLid = file_lid;
	}

	// check the lid and record number
	if ((rec_id > pCfgFielTable[iRealFileLid].i4RecNum)
	        || (file_lid >= g_i4MaxNvRamLid)) {
		return false;
	}

	return true;
}

/********************************************************************************
//FUNCTION:
//		NVM_ReadFileVerInfo
//DESCRIPTION:
//		this function is called to read the version information of nvram file.
//		if the file version is not right, it wll be reset to default value
//
//PARAMETERS:
//		file_lid:	[IN] the lid of nvram file
//
//RETURN VALUE:
//		refers to the definition of "F_INFO"
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
F_INFO NVM_ReadFileVerInfo(int file_lid) {
	int iFileDesc;
	struct stat statbuf;
	char byte[4] = {0, 0, 0, 0};
	F_INFO file_info;
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;
	int iResult = 0;

	NVRAM_LOG("NVM_ReadFileVerInfo ++\n");
	NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);
	memset(&file_info, 0, sizeof(file_info));

	pCfgFielTable = NVM_GetCfgFileTable(file_lid);
	if (pCfgFielTable == NULL) {
		NVRAM_LOG("NVM_GetCfgFileTable Fail!!! \n");
		return file_info;
	}

	// get the file informatin table. if the file lid is version file,
	// just generate it directly by call GenerateFileVer
	//if (file_lid >= AP_CFG_CUSTOM_BEGIN_LID)
	if (file_lid >= iCustomBeginLID) {
		//iRealFileLid = file_lid - AP_CFG_CUSTOM_BEGIN_LID;
		iRealFileLid = file_lid - iCustomBeginLID;
		NVRAM_LOG("Custom File Lid is %d !!! \n", iRealFileLid);
	} else {
		iRealFileLid = file_lid;
		NVRAM_LOG("Default File Lid is %d !!! \n", iRealFileLid);
	}

	//check the version file is exist or not. if not, generate it.
	if (NVM_CheckVerFile(true)) {
		NVRAM_LOG("Version file is exist on nvram \n");
		if (!NVM_CmpFileVerNo(file_lid)) {
			// if file version is not same, reset it.
			if (pCfgFielTable[iRealFileLid].bDataProcessingType == DataReset) { //Reset Data
				iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_CREAT | O_TRUNC,
				                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
				if (iFileDesc == -1) {
					NVRAM_LOG("NVM_Clear File Content Fail_Check Version \n");
					return file_info;
				}
				close(iFileDesc);
				if (!NVM_ResetFileToDefault(file_lid))
					return file_info;
			} else {
				NVRAM_LOG("NVM_ReadFileVerInfo NVM_CheckFileInfo Fail: Convert!!! \n");
				//only for test
				iResult = NVM_DataVerConvert(file_lid);
				if (iResult == 0) {

					iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_CREAT | O_TRUNC,
					                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
					if (iFileDesc == -1) {
						NVRAM_LOG("NVM_Clear File Content Fail_Check Version \n");
						return file_info;
					}
					close(iFileDesc);
					if (!NVM_ResetFileToDefault(file_lid))
						return file_info;
				}
			}
		} else {
			NVRAM_LOG("NvRam Read %s File \n", pCfgFielTable[iRealFileLid].cFileName);
			if (stat(pCfgFielTable[iRealFileLid].cFileName, &statbuf) == -1
			        || !S_ISREG(statbuf.st_mode)) {
				NVRAM_LOG("%s File is not exist: Reset!!! \n",
				          pCfgFielTable[iRealFileLid].cFileName);
				//only for test
				if (!NVM_ResetFileToDefault(file_lid))
					return file_info;
			}
		}
	} else {
		NVRAM_LOG("Version file is not exist on nvram \n");
		if (!NVM_GenerateFileVer(true)) {
			return file_info;
		} else {
			NVRAM_LOG("NVM_ReadFileVerInfo Version File is not exist: Reset!!! \n");
			if (false == NVM_ResetFileToDefault(file_lid)) {
				NVRAM_LOG("reset fail!\n");
				return file_info;
			}
		}
	}

	//read the file version
	//iFileDesc = open(g_akCFG_File[AP_CFG_FILE_VER_INFO_LID].cFileName, O_RDONLY, S_IRUSR);
	iFileDesc = open(g_akCFG_File[iFileVerInfoLID].cFileName, O_RDONLY);
	if (iFileDesc == -1) {
		NVRAM_LOG("Error NVM_ReadFileVerInfo Open version file fail\n");
		return file_info;
	}
    char tempstr[FILENAMELENGTH]={0};
	bool bIsNewVerInfo;
    iResult =read(iFileDesc,tempstr,FILENAMELENGTH-1);
	if (iResult == -1) { NVRAM_LOG("read failed with errno=%d\n", errno); iResult = 0; }
	tempstr[iResult]='\0';
	if (0 == strcmp(tempstr, strVerInfo)) {
		NVRAM_LOG("New version info file\n");
		bIsNewVerInfo = true;
	} else {
		NVRAM_LOG("Old version info file\n");
		bIsNewVerInfo = false;
	}
	if (bIsNewVerInfo) {
		if (-1 == lseek(iFileDesc,
		                (file_lid + 1) * (FILEVERLENGTH + FILENAMELENGTH) + FILENAMELENGTH, SEEK_SET)) {
			NVRAM_LOG("Error NVM_ReadFileVerInfo lseek version file fail\n");
			close(iFileDesc);
			return file_info;
		}
	} else {
		if (-1 == lseek(iFileDesc, file_lid * FILEVERLENGTH, SEEK_SET)) {
			NVRAM_LOG("Error NVM_ReadFileVerInfo lseek version file fail\n");
			close(iFileDesc);
			return file_info;
		}
	}
	read(iFileDesc, byte, FILEVERLENGTH);
	close(iFileDesc);

	memcpy(file_info.cFileVer, byte, FILEVERLENGTH);
	memcpy(file_info.cFileName, pCfgFielTable[iRealFileLid].cFileName,
	       FILENAMELENGTH);
	file_info.i4RecSize = pCfgFielTable[iRealFileLid].i4RecSize;
	file_info.i4RecNum = pCfgFielTable[iRealFileLid].i4RecNum;
	file_info.i4MaxFileLid = g_i4CFG_File_Count + g_i4CFG_File_Custom_Count;

	NVRAM_LOG("ReadFileVerInfo --\n");
	return file_info;
}


/********************************************************************************
//FUNCTION:
//		NVM_CheckFileInfo
//DESCRIPTION:
//		this function is called to check file information, including the file lid
// 		and record id.if the file version is not right, it wll be reset to default
//		value.
//
//PARAMETERS:
//		file_lid: 	[IN]		the lid of nvram file
//		rec_id:	[IN]		the record id
//
//RETURN VALUE:
//		true is success, otherwise is fail
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_CheckFileInfo(int file_lid, int rec_id) {
	struct stat statbuf;
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;

	NVRAM_LOG("NVM_CheckFileInfo ++\n");
	//NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);

	// get the file informatin table. if the file lid is version file,
	// just generate it directly by call GenerateFileVer
	pCfgFielTable = NVM_GetCfgFileTable(file_lid);
	if (pCfgFielTable == NULL) {
		NVRAM_LOG("NVM_CheckFileInfo NVM_GetCfgFileTable Fail!!! \n");
		return false;
	}

	//if (file_lid < AP_CFG_CUSTOM_BEGIN_LID) {
	if (file_lid < iCustomBeginLID) {
		iRealFileLid = file_lid;
	} else {
		//iRealFileLid = file_lid - AP_CFG_CUSTOM_BEGIN_LID;
		iRealFileLid = file_lid - iCustomBeginLID;
	}

	//check the lid and record id are right or not
	if (!NVM_CheckFileID(file_lid, rec_id)) {
		return false;
	}

	//check the file versio and check the version file is exist or not. if not, generate it.
	if (NVM_CheckVerFile(true)) {
		if (!NVM_CmpFileVerNo(file_lid)) {
			NVRAM_LOG("NVM_CheckFileInfo NVM_CheckFileInfo Fail: Reset!!! \n");
			if (!NVM_ResetFileToDefault(file_lid)) {
				return false;
			}
		} else {
			if (stat(pCfgFielTable[iRealFileLid].cFileName, &statbuf) == -1 ||
			        !S_ISREG(statbuf.st_mode)) {
				NVRAM_LOG("NVM_CheckFileInfo File is not exist: Reset!!! \n");
				if (!NVM_ResetFileToDefault(file_lid)) {
					return false;
				}
			}
		}
	} else {
		if (!NVM_GenerateFileVer(true)) {
			return false;
		} else {
			NVRAM_LOG("NVM_CheckFileInfo NVM_GenerateFileVer to Reset!!! \n");
			if (!NVM_ResetFileToDefault(file_lid)) {
				return false;
			}
		}
	}

	NVRAM_LOG("NVM_CheckFileInfo --\n");
	return true;
}

/********************************************************************************
//FUNCTION:
//		NVM_GetFileDesc
//DESCRIPTION:
//		this function is called to the desc of nvram file and the information
//      of record size and number.
//
//PARAMETERS:
//		file_lid: 	[IN]	the lid of nvram file
//		pRecSize: 	[OUT] 	the record size
//		pRecNum: 	[OUT] 	the rocord number
//		IsRead: 	[IN]	true is read, otherwise is write
//
//RETURN VALUE:
//		the file file desc
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

F_ID NVM_GetFileDesc(int file_lid, int *pRecSize, int *pRecNum, bool IsRead) {
	int iFileDesc;
	int iResult;
	const TCFG_FILE *pCfgFielTable = NULL;
	int iRealFileLid = 0;
	bool Checkflag = 0;
	F_ID FileID;
	FileID.iFileDesc = -1;
	FileID.ifile_lid = file_lid;
	FileID.bIsRead = IsRead;

	unsigned int index = 0;
	bool maskflag = 0;
	bool bRet = false;
	int index_in_list = -1;
	struct mtd_info_user info;
	int i = 0;
	char cMtdDevName[128] = {0};
	struct erase_info_user erase_info;
	for (index = 0; index < g_Performance_File_Count; index++) {
		if ((unsigned int)file_lid == aPerformance[index].iLID) {
			maskflag = 1;
			break;
		}
	}
	//NVRAM_LOG("NVM_GetFileDesc ++\n");
	if (NVM_Init() < 0) {
		NVRAM_LOG("NVM_GetFileDesc--NVM_Init fail !!!\n");
		return FileID;
	}
	//NVRAM_LOG("iCustomBeginLID = %d\n", iCustomBeginLID);
	//get the file informatin table. if the file lid is version file, just generate it directly by call GenerateFileVer
#ifndef MTK_BASIC_PACKAGE
	if (!NVM_CheckWritePermission(IsRead, file_lid)) {
		return FileID;
	}
#endif

	pCfgFielTable = NVM_GetCfgFileTable(file_lid);
	//NVRAM_LOG("NVM_GetCfgFileTable done\n");
	//if (file_lid < AP_CFG_CUSTOM_BEGIN_LID) {
	if (file_lid < iCustomBeginLID) {
		iRealFileLid = file_lid;
	} else {
		//iRealFileLid = file_lid - AP_CFG_CUSTOM_BEGIN_LID;
		iRealFileLid = file_lid - iCustomBeginLID;
	}

	if (pCfgFielTable == NULL) {
		NVRAM_LOG("NVM_GetCfgFileTable Fail!!! \n");
		return FileID;
	}

	if (nvram_new_partition_support()) {
		if (NVM_InSpecialLidList(file_lid, &index_in_list)) {
			if (pRecSize != NULL && pRecNum != NULL) {
				*pRecSize = pCfgFielTable[iRealFileLid].i4RecSize;
				*pRecNum  = pCfgFielTable[iRealFileLid].i4RecNum;
			}
			goto Solve_Special_Lid;
		}
	}

	//get file hanle.

	NVRAM_LOG("%s: Open %s,LID:%d\n", __func__,
	          pCfgFielTable[iRealFileLid].cFileName, file_lid);
	//Check if the lid is reserved.
	if (strcmp(pCfgFielTable[iRealFileLid].cFileName, "Reserved") == 0) {
		NVRAM_LOG("The LID is Reserved, please check lid you used!!!");
		return FileID;
	}
	if (IsRead) {
		iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_RDONLY);
	} else {
		iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_RDWR);
	}
	//get file RecSize and RecNum information
	if (pRecSize != NULL && pRecNum != NULL) {
		*pRecSize = pCfgFielTable[iRealFileLid].i4RecSize;
		*pRecNum  = pCfgFielTable[iRealFileLid].i4RecNum;
	}
	if (iFileDesc == -1) { //if file doesn't exist
		if (access(g_pcNVM_Flag, F_OK) == 0) {
			NVRAM_LOG("File is not exist, try to restore from binregion!!!");
			for (i = 0; i < (int)g_Backup_File_Count; i++) {
				if ((unsigned int)file_lid == aBackupToBinRegion[i].iLID) {
					if (NVM_RestoreFromBinRegion_OneFile(file_lid, NULL)) {
						NVRAM_LOG("successfully restore.\n");
						goto ProtectData;
					}
					break;
				}
			}
		}
		NVRAM_LOG("Create the dir path of %s\n", pCfgFielTable[iRealFileLid].cFileName);
		iFileDesc = open_file_with_dirs(pCfgFielTable[iRealFileLid].cFileName,
		                                O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); //create the file
		if (iFileDesc == -1) {
			NVRAM_LOG("Error Num %s\n", (char*)strerror(errno));
			NVRAM_LOG("Error NVM_GetFileDesc open_file_with_dirs file fail: %s\n",
			          pCfgFielTable[iRealFileLid].cFileName);
			} else {
		close(iFileDesc);
			}
		if (!NVM_ResetFileToDefault(file_lid)) {
			NVRAM_LOG("ResetFileToDefault Failed\n");
			return FileID;
		}
	} else {
		close(iFileDesc);//avoid the bug of re-open file
		if (!NVM_CmpFileVerNo(file_lid)) {
			// if file version is not same, convert it.
			if (pCfgFielTable[iRealFileLid].bDataProcessingType == DataReset) { //Reset Data
				NVRAM_LOG("NVM_ReadFileVerInfo NVM_CheckFileInfo Fail: Reset!!! \n");
				iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_TRUNC);
				if (iFileDesc == -1) {
					NVRAM_LOG("NVM_Clear File Content Fail_Get File Desc \n");
					return FileID;
				}
				close(iFileDesc);
				if (!NVM_ResetFileToDefault(file_lid))
					return FileID;
			} else {
				NVRAM_LOG("NVM_ReadFileVerInfo NVM_CheckFileInfo Fail: Convert!!! \n");
				//only for test
				iResult = NVM_DataVerConvert(file_lid);
				if (iResult == 0) {
					iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_TRUNC);
					if (iFileDesc == -1) {
						NVRAM_LOG("NVM_Clear File Content Fail_Get File Desc \n");
						return FileID;
					}
					close(iFileDesc);
					if (!NVM_ResetFileToDefault(file_lid))
						return FileID;
				}
			}
		}

	}
ProtectData:
#ifndef MTK_BASIC_PACKAGE
	if (!NVM_Enc_Dnc_File(file_lid, DECODE)) {
		NVRAM_LOG("Dec fail!!!\n");
		return FileID;
	}
#endif
	if (maskflag == 0) {
		if (NVM_ProtectDataFile(file_lid, Checkflag) == 1) {
			//NVRAM_LOG("NVM_GetFileDesc ProtectDataFile Check Success!!\n");
		} else {
			NVRAM_LOG("NVM_GetFileDesc ProtectDataFile Fail!!\n");
			return FileID;
		}
	}
	if (IsRead) {
		iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_RDONLY);
	} else {
		iFileDesc = open(pCfgFielTable[iRealFileLid].cFileName, O_RDWR | O_SYNC);
	}
	//NVRAM_LOG("NVM_GetFileDesc --\n");
	FileID.iFileDesc = iFileDesc;
	return FileID;


Solve_Special_Lid:
	memset(cMtdDevName, 0, sizeof cMtdDevName);
        //strcpy(cMtdDevName, g_NVRAM_PROINFO_DEVICE);
        snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_PROINFO_DEVICE);
	NVRAM_LOG("New NVRAM partition name is %s.\n", cMtdDevName);
	bRet = NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet) {
		NVRAM_LOG("NVM_GetSequenceNum: get device info fail!!!\n");
		return FileID;
	}

	if (g_new_nvram_lid[index_in_list].start_address % info.erasesize != 0
	        || g_new_nvram_lid[index_in_list].start_address +
	        g_new_nvram_lid[index_in_list].size > (long)info.size) {
		NVRAM_LOG("Lid info in special info is error!!!\n");
		return FileID;
	}

	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {
		if ((pRecSize != NULL) && ((unsigned int)*pRecSize < info.writesize)) {
			NVRAM_LOG("*pRecSize %d < info.writesize %d\n", *pRecSize, info.writesize);
			*pRecSize = info.writesize;
		}
	}

	if (*pRecSize % info.writesize != 0) {
		NVRAM_LOG("Please make sure size for special lid in new nvram partition should alignment %d\n",
		          info.writesize);
		return FileID;
	}

	if (IsRead) {
		iFileDesc = open(cMtdDevName, O_RDONLY);
	} else {
		if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {
			erase_info.start = g_new_nvram_lid[index_in_list].start_address;
			erase_info.length = g_new_nvram_lid[index_in_list].size;
			if (g_new_nvram_lid[index_in_list].size < (long)info.erasesize)
				erase_info.length = info.erasesize;
			bRet = NVM_EraseDeviceBlock(cMtdDevName, erase_info);
			if ( false == bRet) {
				NVRAM_LOG("NVM_IncSequenceNum: erase device failed");
				return FileID;
			}
		}

		iFileDesc = open(cMtdDevName, O_RDWR | O_SYNC);
		if (iFileDesc < 0) {
			NVRAM_LOG("Open new nvram partition fail!!!\n");
			return FileID;
		}
	}

	if (lseek(iFileDesc, g_new_nvram_lid[index_in_list].start_address,
	          SEEK_SET) < 0) {
		NVRAM_LOG("seek for lid %d fail!!!\n", file_lid);
		close(iFileDesc);
		return FileID;
	}
	FileID.iFileDesc = iFileDesc;
	return FileID;

}


/********************************************************************************
//FUNCTION:
//		NVM_CloseFileDesc
//DESCRIPTION:
//		this function is called to close the file desc which is open by NVM_GetFileDesc.
//
//PARAMETERS:
//		hFile: 	[IN] the file desc
//
//RETURN VALUE:
//		true is success, otherwise is fail
//
//DEPENDENCY:
//		GetFileDesc must have been called
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool NVM_CloseFileDesc(F_ID FileID) {

	int index_in_list = -1;
	bool Setflag = 1;
	if (nvram_new_partition_support()) {
		if (NVM_InSpecialLidList(FileID.ifile_lid, &index_in_list)) {
			//the handle of pro info shoud be closed
			if (FileID.iFileDesc == -1) {
				return false;
			}

#if 0
			//if (!FileID.bIsRead)
			{
				if (0 != fsync(FileID.iFileDesc)) {
					NVRAM_LOG("NVM_CloseFileDesc fsync file error!!\n");
					return false;
				}
			}
#endif
			if (0 != close(FileID.iFileDesc)) {
				NVRAM_LOG("NVM_CloseFileDesc Close file error!!\n");
				return false;
			}
			return true;
		}
	}

	if (FileID.iFileDesc == -1) {
		return false;
	}

	//if (!FileID.bIsRead)
	{
		if (0 != fsync(FileID.iFileDesc)) {
			NVRAM_LOG("NVM_CloseFileDesc fsync file error!!\n");
			return false;
		}
	}

	if (0 != close(FileID.iFileDesc)) {
		NVRAM_LOG("NVM_CloseFileDesc Close file error!!\n");
		return false;
	}
	bool maskflag = 0;
	unsigned int index = 0;
	for (index = 0; index < g_Performance_File_Count; index++) {
		if ((unsigned int)FileID.ifile_lid == aPerformance[index].iLID) {
			maskflag = 1;
			break;
		}
	}
	if (maskflag || FileID.bIsRead) {
#ifndef MTK_BASIC_PACKAGE
		if (!NVM_Enc_Dnc_File(FileID.ifile_lid, ENCODE)) {
			NVRAM_LOG("Enc fail!!!\n");
			return false;
		}
#endif
	}
	if (maskflag)
		return true;
	if (true == FileID.bIsRead) {
		NVRAM_LOG("NVM_CloseFileDesc: Open by Readonly, no need to check when close\n");
		return true;
	}
	if (NVM_ProtectDataFile(FileID.ifile_lid, Setflag) == 1) {
		NVRAM_LOG("NVM_CloseFileDesc ProtectDataFile Success!!\n");
#ifndef MTK_BASIC_PACKAGE
		if (!NVM_Enc_Dnc_File(FileID.ifile_lid, ENCODE)) {
			NVRAM_LOG("Enc fail!!!\n");
			return false;
		}
#endif
		return true;
	} else {
		NVRAM_LOG("NVM_CloseFileDesc ProtectDataFile SET Fail!!\n");
		return false;
	}

}

unsigned int NVM_GetSequenceNum(void) {
	nvram_journal_handle_t *journal;
	struct mtd_info_user info;
	int fd;
	int iResult;
	unsigned int sequence = 0;
	bool bRet;
	loff_t offset = 0;
	char *buffer = NULL;
	char cMtdDevName[128] = {0};


	memset(cMtdDevName, 0, sizeof cMtdDevName);
	if (nvram_platform_layout_version == 0)
            snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_LOG_DEVICE);
	else if (nvram_platform_layout_version == 1) {
		NVRAM_LOG("NVM_GetSequenceNum: could't support in nvram layout version %d\n",
		          nvram_platform_layout_version);
		return true;
		//	ret = NVM_GetDeviceInfo(MISC_DEVICE, &info);
	} else {
		NVRAM_LOG("NVM_GetSequenceNum: invalid nvram layout version %d\n",
		          nvram_platform_layout_version);
		return false;
	}
	bRet = NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet) {
		NVRAM_LOG("NVM_GetSequenceNum: get device info fail!!!\n");
		return -1;
	}
	if (info.writesize < sizeof(nvram_journal_handle_t) || info.size == 0) {
		NVRAM_LOG("NVM_GetSequenceNum: journal size is larger than device write size!!!\n");
		return -1;
	}

	journal = (nvram_journal_handle_t *)malloc(sizeof(nvram_journal_handle_t));
	if (NULL == journal) {
		NVRAM_LOG("NVM_GetSequenceNum : allocate for journal fail!!!\n");
		return false;
	}
	memset(journal, 0, sizeof(nvram_journal_handle_t));

	buffer = (char *) malloc(info.writesize);
	if (NULL == buffer) {
		NVRAM_LOG("NVM_GetSequenceNum: allocate for temp buffer fail!!!\n");
		free(journal);
		return -1;
	}
	memset(buffer, 0, info.writesize);

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("NVM_GetSequenceNum: dumchar open fail!!!\n");
		free(journal);
		free(buffer);
		return -1;
	}

	offset = info.size - info.erasesize;
	iResult = lseek(fd, offset, SEEK_SET);
	if (iResult  < 0) {
		NVRAM_LOG("NVM_GetSequenceNum: seek log device fail!!!\n");
		free(journal);
		free(buffer);
		close(fd);
		return -1;
	}
	iResult = read(fd, buffer, info.writesize);
	if ((unsigned int)iResult != info.writesize) {
		NVRAM_LOG("NVM_GetSequenceNum: read nvram super page fail, iResult = %d !!!\n",
		          iResult);
		free(journal);
		free(buffer);
		close(fd);
		return -1;
	}
	close(fd);
	memcpy(journal, buffer, sizeof(nvram_journal_handle_t));
	if (journal->h_magic == NVRAM_JOURNAL_MAGIC) {
		sequence = journal->h_sequence;
	} else {
		sequence = 0;
	}
	free(journal);
	free(buffer);
	return sequence;
}

bool NVM_IncSequenceNum(unsigned int sequence) {
	nvram_journal_handle_t *journal;
	struct mtd_info_user info;

	struct erase_info_user erase_info;

	int fd;
	int iResult;
	bool bRet;
	loff_t offset;
	time_t tm;
	char *buffer = NULL;
	char cMtdDevName[128] = {0};

	memset(cMtdDevName, 0, sizeof cMtdDevName);
	if (nvram_platform_layout_version == 0)
		snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_LOG_DEVICE);
	else if (nvram_platform_layout_version == 1) {
		NVRAM_LOG("NVM_IncSequenceNum: could't support in nvram layout version %d\n",
		          nvram_platform_layout_version);
		return true;
		//	ret = NVM_GetDeviceInfo(MISC_DEVICE, &info);
	} else {
		NVRAM_LOG("NVM_IncSequenceNum: invalid nvram layout version %d\n",
		          nvram_platform_layout_version);
		return false;
	}
	bRet = NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet) {
		NVRAM_LOG("NVM_IncSequenceNum: get device info fail!!!\n");
		return false;
	}
	if (info.writesize < sizeof(nvram_journal_handle_t) || info.size == 0) {
		NVRAM_LOG("NVM_IncSequenceNum: journal size is larger than device write size!!!\n");
		return false;
	}

	journal = (nvram_journal_handle_t *)malloc(sizeof(nvram_journal_handle_t));
	if (NULL == journal) {
		NVRAM_LOG("NVM_IncSequenceNum: allocate for journal fail!!!\n");
		return false;
	}
	memset(journal, 0, sizeof(nvram_journal_handle_t));

	buffer = (char *) malloc(info.writesize);
	if (NULL == buffer) {
		NVRAM_LOG("NVM_IncSequenceNum: allocate for temp buffer fail!!!\n");
		free(journal);
		return false;
	}
	memset(buffer, 0, info.writesize);

	tm = time(&tm);
	journal->h_magic = NVRAM_JOURNAL_MAGIC;
	journal->h_type = NVRAM_JOURNAL_SUPER_PAGE;
	journal->h_sequence = ++sequence;
	journal->h_utc_time = tm;

	offset = info.size - info.erasesize;
	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {
		erase_info.start = offset;
		erase_info.length = info.erasesize;


		bRet = NVM_EraseDeviceBlock(cMtdDevName, erase_info);
		if ( false == bRet) {
			NVRAM_LOG("NVM_IncSequenceNum: erase device failed\n");
			free(journal);
			free(buffer);
			return false;
		}
	}

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("NVM_IncSequenceNum: dumchar open fail!!!\n");
		free(journal);
		free(buffer);
		return false;
	}
	iResult = lseek(fd, offset, SEEK_SET);
	if (iResult  < 0) {
		NVRAM_LOG("NVM_IncSequenceNum: seek log device fail!!!\n");
		free(journal);
		free(buffer);
		close(fd);
		return false;
	}
	memcpy(buffer, journal, sizeof(nvram_journal_handle_t));
	iResult = write(fd, buffer, info.writesize);
	if ((unsigned int)iResult != info.writesize) {
		NVRAM_LOG("NVM_IncSequenceNum: read nvram super page fail, iResult = %d !!!\n",
		          iResult);
		free(journal);
		free(buffer);
		close(fd);
		return false;
	}
	free(journal);
	free(buffer);
	close(fd);
	return true;
}

//add by min
bool NVM_HistoryLog_add_file(const char *fix_dir, const char *pre_fix_file,nvram_journal_handle_t *journal)
{
     DIR *dir;
     struct dirent *entry;
     struct stat  st;
     int size=0;
     int read_size=0;
     char acSrcPathName[MAX_NAMESIZE];
     int fd;
     unsigned char buffer[MAX_BARCODE_LENGTH];//3 barcode is 66*3 < MAX_BARCODE_LENGTH
     NVRAM_LOG("NVRAM_HistoryLog: dir is  %s\n",fix_dir);
     dir = opendir(fix_dir);
     int num=0;// how much barcode file
    if(dir == NULL)
    {
        NVRAM_LOG("NVRAM_HistoryLog: open dir %s error %s\n",fix_dir, (char*)strerror(errno));
        return false;
     } 
    while(1)
    {
         entry = readdir(dir);
         if(entry == NULL)
         {
            break;
         }

         if (!strcmp(entry->d_name, ".")|| !strcmp(entry->d_name, ".."))
         {
    		continue;
          }
          if(strncmp(entry->d_name,pre_fix_file,strlen(pre_fix_file)) == 0)
          {
              if(num >= 3)
                  break;// total 3 barcode file
              sprintf(acSrcPathName, "%s/%s", fix_dir, entry->d_name);
              NVRAM_LOG("NVRAM_HistoryLog: file is  %s",acSrcPathName);
              if(stat(acSrcPathName,&st)<0)
              {
                    NVRAM_LOG("NVRAM_HistoryLog: Error %s stat ",acSrcPathName);
     	            closedir(dir);
                    return false;
              }
             size=st.st_size;
             fd= open(acSrcPathName,O_RDONLY);
             if (fd < 0)
             {
				NVRAM_LOG("NVRAM_HistoryLog: open %s error %s\n",acSrcPathName, (char*)strerror(errno) );
                closedir(dir);
				return false;
    	     }
    	     NVRAM_LOG("NVRAM_HistoryLog: sizeofbuffer is %d\n",sizeof(buffer));
             read_size= (int)read(fd, buffer, size);
              if (read_size != size)
             {
		         NVRAM_LOG("NVRAM_HistoryLog: read_size is %d",read_size);
                 close(fd);
                 closedir(dir);
		         return false;
             }
             NVRAM_LOG("NVRAM_HistoryLog: readsize is %d\n",read_size);
             if(120 == read_size)
             {
                 memcpy(journal->imei, entry->d_name, MAX_IMEI_LENGTH);//add by min, copy IMEI_FLAG to joural
                 memcpy(journal->imei+strlen(entry->d_name), buffer, size);//add by min, copy imei to joural
             }
             else if(66 == read_size)
             {
				memcpy(journal->barcode+num*(size+strlen(entry->d_name)), entry->d_name, strlen(entry->d_name));//add by min, copy BARCODE_FLAG to joural
				memcpy(journal->barcode+num*(size+strlen(entry->d_name))+strlen(entry->d_name), buffer , size);//add by min, copy barcode to joural
			 }
			 else
             {
                NVRAM_LOG("NVRAM_HistoryLog: read_size is wrong %d",read_size);
             }
             num++;
             close(fd);
         }
    }
    closedir(dir);
    return true;
//  add
}
//add by min
//add by min
bool NVM_HistoryLog_Time(unsigned int level, const char *func, unsigned int line,const char *log,unsigned char *time)
{//add
	NVRAM_LOG("NVM_HistoryLog: time is  %s\n",time);
    bool bRet = false;
    int iResult = 0;
    int fd;
    //time_t tm;//don't use tm
    loff_t start_address;
    loff_t offset;
    unsigned int writepages_per_block = 0;
    unsigned int sequence = 0;
    nvram_journal_handle_t *journal;
    struct mtd_info_user info;
   
    struct erase_info_user erase_info;
    
    char *buffer = NULL;
	char cMtdDevName[128] = {0};
    //just when log level larger than default log level, the log will print
    if (level < DEFAULT_LOG_LEVEL)
        return true;

	memset(cMtdDevName, 0, sizeof cMtdDevName);
    if (nvram_platform_layout_version == 0)
        snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_LOG_DEVICE);
	else if (nvram_platform_layout_version == 1)
	{
		NVRAM_LOG("NVM_HistoryLog: could't support in nvram layout version %d\n",nvram_platform_layout_version);
		 return true;
		//	ret = NVM_GetDeviceInfo(MISC_DEVICE, &info);
	}
	else
	{
	 	NVRAM_LOG("NVM_HistoryLog: invalid nvram layout version %d\n",nvram_platform_layout_version);
	 	return false;
	}
   	bRet =  NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet)
	{
        NVRAM_LOG("NVM_HistoryLog : get device info failed!!!\n");
        return false;
    }
    if(info.writesize < sizeof(nvram_journal_handle_t) || info.size == 0)
	{
        NVRAM_LOG("NVM_HistoryLog : journal size is larger than device write size!!!\n");
        return false;
    }

    //calculate the location for write log
    writepages_per_block = info.erasesize/info.writesize;
    sequence = NVM_GetSequenceNum();
    if(sequence < 0)
	{
        NVRAM_LOG("NVM_HistoryLog : get sequence number fail!!!\n");
        return false;
    }

    start_address = (loff_t)((loff_t)info.size - ((loff_t)nvram_platform_log_block * (loff_t)info.erasesize));
    offset = (loff_t)((loff_t)info.writesize * ((loff_t)sequence % ((loff_t)writepages_per_block * ((loff_t)nvram_platform_log_block - 1))));

    if((!nvram_emmc_support()) && (!nvram_ufs_support()))
	{
	    if ((sequence % writepages_per_block ) == 0)
		{
			erase_info.start = start_address + offset;
			erase_info.length = info.erasesize;

		   bRet = NVM_EraseDeviceBlock(cMtdDevName, erase_info);
		   if (false == bRet)
		   {
	           NVRAM_LOG("NVM_HistoryLog : erase super block fail!!!\n");
	           return false;
	       }
	    }
    }
    journal = (nvram_journal_handle_t *)malloc(sizeof(nvram_journal_handle_t));
    if (NULL == journal) {
        NVRAM_LOG("NVM_HistoryLog : allocate for journal fail!!!\n");
        return false;
    }
    memset(journal, 0, sizeof(nvram_journal_handle_t));

    buffer = (char*)malloc(info.writesize);
    if (NULL == buffer) {
        NVRAM_LOG("NVM_HistoryLog : allocate for temp buffer fail!!!\n");
        free(journal);
        return false;
    }
    memset(buffer, 0, info.writesize);

    //tm = time(&tm);//don't use tm
    journal->h_magic = NVRAM_JOURNAL_MAGIC;
    journal->h_type = NVRAM_JOURNAL_LOG_PAGE;
    journal->h_sequence = sequence;
    //journal->h_utc_time = time;//don't use tm
    journal->h_func_line = line;
    //by min,IMEI
    if(!NVM_HistoryLog_add_file(g_pcNVM_MDRdebFile, PREFIX_IMEI,journal))
     {
        NVRAM_LOG("NVRAM_HistoryLog: get IMEI error");
		free(journal);
		free(buffer);
        return false;
     }
    //by min,BARCODE
    if(!NVM_HistoryLog_add_file(g_pcNVM_MDCalFile, PREFIX_BARCODE,journal))
    {
       NVRAM_LOG("NVRAM_HistoryLog: get barcode error");
	   free(journal);
	   free(buffer);
       return false;
    }
    snprintf(journal->h_func_name, sizeof(journal->h_func_name), "%s", func);
    snprintf(journal->h_log, MAX_LOG_LENGTH, "%s:%s", time, log);
    journal->h_log_lengeh = strlen(journal->h_log);

	fd = open(cMtdDevName, O_RDWR);
    if (fd < 0)
	{
        NVRAM_LOG("NVM_HistoryLog : open log device fail!!!\n");
        free(journal);
        free(buffer);
        return false;
    }
    iResult = lseek(fd, start_address + offset, SEEK_SET);
    if (iResult  < 0)
	{
        NVRAM_LOG("NVM_HistoryLog : seek log device fail!!!\n");
        free(journal);
        free(buffer);
        close(fd);
        return false;
    }
    memcpy(buffer, journal, sizeof(nvram_journal_handle_t));
    iResult = write(fd, buffer, info.writesize);
    if((unsigned int)iResult != info.writesize)
	{
        NVRAM_LOG("NVM_HistoryLog : write log device fail, iResult = %d !!!\n", iResult);
        free(journal);
        free(buffer);
        close(fd);
        return false;
    }
    free(journal);
    free(buffer);
    close(fd);
    //sync();
    bRet = NVM_IncSequenceNum(sequence);
    if( false == bRet)
	{
        NVRAM_LOG("NVM_HistoryLog : update sequence number fail!!!\n");
        return false;
    }

    return true;
	
//add	
}

bool NVM_HistoryLog(unsigned int level, const char *func, unsigned int line,
                    const char *log) {
	bool bRet = false;
	int iResult = 0;
	int fd;
	time_t tm;
	loff_t start_address;
	loff_t offset;
	unsigned int writepages_per_block = 0;
	unsigned int sequence = 0;
	nvram_journal_handle_t *journal;
	struct mtd_info_user info;

	struct erase_info_user erase_info;

	char *buffer = NULL;
	char cMtdDevName[128] = {0};
	//just when log level larger than default log level, the log will print
	if (level < DEFAULT_LOG_LEVEL)
		return true;

	memset(cMtdDevName, 0, sizeof cMtdDevName);
	if (nvram_platform_layout_version == 0)
		//strcpy(cMtdDevName, g_LOG_DEVICE);
                snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_LOG_DEVICE);
	else if (nvram_platform_layout_version == 1) {
		NVRAM_LOG("NVM_HistoryLog: could't support in nvram layout version %d\n",
		          nvram_platform_layout_version);
		return true;
		//	ret = NVM_GetDeviceInfo(MISC_DEVICE, &info);
	} else {
		NVRAM_LOG("NVM_HistoryLog: invalid nvram layout version %d\n",
		          nvram_platform_layout_version);
		return false;
	}
	bRet =  NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet) {
		NVRAM_LOG("NVM_HistoryLog : get device info failed!!!\n");
		return false;
	}
	if (info.writesize < sizeof(nvram_journal_handle_t) || info.size == 0) {
		NVRAM_LOG("NVM_HistoryLog : journal size is larger than device write size!!!\n");
		return false;
	}

	//calculate the location for write log
	writepages_per_block = info.erasesize / info.writesize;
	sequence = NVM_GetSequenceNum();
	if (sequence < 0) {
		NVRAM_LOG("NVM_HistoryLog : get sequence number fail!!!\n");
		return false;
	}

	start_address = info.size - ((loff_t)nvram_platform_log_block *
	                             (loff_t)info.erasesize);
	offset = (loff_t)((loff_t)info.writesize * ((loff_t)sequence % ((loff_t)writepages_per_block *
	                                    (loff_t)(nvram_platform_log_block - 1))));

	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {
		if ((sequence % writepages_per_block ) == 0) {
			erase_info.start = start_address + offset;
			erase_info.length = info.erasesize;

			bRet = NVM_EraseDeviceBlock(cMtdDevName, erase_info);
			if (false == bRet) {
				NVRAM_LOG("NVM_HistoryLog : erase super block fail!!!\n");
				return false;
			}
		}
	}
	journal = (nvram_journal_handle_t *)malloc(sizeof(nvram_journal_handle_t));
	if (NULL == journal) {
		NVRAM_LOG("NVM_HistoryLog : allocate for journal fail!!!\n");
		return false;
	}
	memset(journal, 0, sizeof(nvram_journal_handle_t));

	buffer = (char*)malloc(info.writesize);
	if (NULL == buffer) {
		NVRAM_LOG("NVM_HistoryLog : allocate for temp buffer fail!!!\n");
		free(journal);
		return false;
	}
	memset(buffer, 0, info.writesize);

	tm = time(&tm);
	journal->h_magic = NVRAM_JOURNAL_MAGIC;
	journal->h_type = NVRAM_JOURNAL_LOG_PAGE;
	journal->h_sequence = sequence;
	journal->h_utc_time = tm;
	journal->h_func_line = line;

	snprintf(journal->h_func_name, sizeof(journal->h_func_name), "%s", func);
	snprintf(journal->h_log, MAX_LOG_LENGTH, "%s:%s", ctime(&tm), log);
	journal->h_log_lengeh = strlen(journal->h_log);

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("NVM_HistoryLog : open log device fail!!!\n");
		free(journal);
		free(buffer);
		return false;
	}
	iResult = lseek(fd, start_address + offset, SEEK_SET);
	if (iResult  < 0) {
		NVRAM_LOG("NVM_HistoryLog : seek log device fail!!!\n");
		free(journal);
		free(buffer);
		close(fd);
		return false;
	}
	memcpy(buffer, journal, sizeof(nvram_journal_handle_t));
	iResult = write(fd, buffer, info.writesize);
	if ((unsigned int)iResult != info.writesize) {
		NVRAM_LOG("NVM_HistoryLog : write log device fail, iResult = %d !!!\n",
		          iResult);
		free(journal);
		free(buffer);
		close(fd);
		return false;
	}
	free(journal);
	free(buffer);
	close(fd);
	//sync();
	bRet = NVM_IncSequenceNum(sequence);
	if ( false == bRet) {
		NVRAM_LOG("NVM_HistoryLog : update sequence number fail!!!\n");
		return false;
	}

	return true;
}

unsigned int NVM_MiscGetSeqNum(void) {
	nvram_journal_handle_t *journal;
	struct mtd_info_user info;
	int fd;
	int iResult;
	unsigned int sequence = 0;
	bool bRet;
	loff_t offset = 0;
	char *buffer = NULL;
	char cMtdDevName[128] = {0};

	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {
		NVRAM_LOG("NVM_MiscGetSeqNum: not support NAND\n");
		return false;
	}
	if (nvram_platform_layout_version == 1) {
		NVRAM_LOG("NVM_MiscGetSeqNum: could't support in nvram layout version %d\n",
		          nvram_platform_layout_version);
		return -1;
	}
	memset(cMtdDevName, 0, sizeof cMtdDevName);
	//strcpy(cMtdDevName, g_MISC_DEVICE);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_MISC_DEVICE);
	bRet = NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet) {
		NVRAM_LOG("NVM_MiscGetSeqNum: get device info fail!!!\n");
		return -1;
	}
	if (info.writesize < sizeof(nvram_journal_handle_t) || info.size == 0) {
		NVRAM_LOG("NVM_MiscGetSeqNum: journal size is larger than device write size!!!\n");
		return -1;
	}

	journal = (nvram_journal_handle_t *)malloc(sizeof(nvram_journal_handle_t));
	if (NULL == journal) {
		NVRAM_LOG("NVM_MiscGetSeqNum : allocate for journal fail!!!\n");
		return false;
	}
	memset(journal, 0, sizeof(nvram_journal_handle_t));

	buffer = (char *) malloc(info.writesize);
	if (NULL == buffer) {
		NVRAM_LOG("NVM_MiscGetSeqNum: allocate for temp buffer fail!!!\n");
		free(journal);
		return -1;
	}
	memset(buffer, 0, info.writesize);
	if (nvram_platform_layout_version == 1) {
		NVRAM_LOG("NVM_MiscGetSeqNum: could't support in nvram layout version %d\n",
		          nvram_platform_layout_version);
		return -1;
	}
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("NVM_MiscGetSeqNum: dumchar open fail!!!\n");
		free(journal);
		free(buffer);
		return -1;
	}
	offset = (loff_t)((loff_t)info.size - (loff_t)info.erasesize * (loff_t)nvram_misc_log_block_offset);

	iResult = lseek(fd, offset, SEEK_SET);
	if (iResult  < 0) {
		NVRAM_LOG("NVM_MiscGetSeqNum: seek log device fail!!!\n");
		free(journal);
		free(buffer);
		close(fd);
		return -1;
	}
	iResult = read(fd, buffer, info.writesize);
	if ((unsigned int)iResult != info.writesize) {
		NVRAM_LOG("NVM_MiscGetSeqNum: read nvram super page fail, iResult = %d !!!\n",
		          iResult);
		free(journal);
		free(buffer);
		close(fd);
		return -1;
	}
	close(fd);
	memcpy(journal, buffer, sizeof(nvram_journal_handle_t));
	if (journal->h_magic == NVRAM_MISC_JOURNAL_MAGIC) {
		sequence = journal->h_sequence;
	} else {
		sequence = 0;
	}
	free(journal);
	free(buffer);
	return sequence;
}

bool NVM_MiscIncSeqNum(unsigned int sequence) {
	nvram_journal_handle_t *journal;
	struct mtd_info_user info;
	char cMtdDevName[128] = {0};

	int fd;
	int iResult;
	bool bRet;
	loff_t offset;
	time_t tm;
	char *buffer = NULL;

	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {
		NVRAM_LOG("NVM_MiscIncSeqNum: not support NAND\n");
		return false;
	}
	if (nvram_platform_layout_version == 1) {
		NVRAM_LOG("NVM_MiscGetSeqNum: could't support in nvram layout version %d\n",
		          nvram_platform_layout_version);
		return -1;
	}
	memset(cMtdDevName, 0, sizeof cMtdDevName);
	//strcpy(cMtdDevName, g_MISC_DEVICE);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_MISC_DEVICE);
	bRet = NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet) {
		NVRAM_LOG("NVM_MiscIncSeqNum: get device info fail!!!\n");
		return false;
	}
	if (info.writesize < sizeof(nvram_journal_handle_t) || info.size == 0) {
		NVRAM_LOG("NVM_MiscIncSeqNum: journal size is larger than device write size!!!\n");
		return false;
	}

	journal = (nvram_journal_handle_t *)malloc(sizeof(nvram_journal_handle_t));
	if (NULL == journal) {
		NVRAM_LOG("NVM_MiscIncSeqNum: allocate for journal fail!!!\n");
		return false;
	}
	memset(journal, 0, sizeof(nvram_journal_handle_t));

	buffer = (char *) malloc(info.writesize);
	if (NULL == buffer) {
		NVRAM_LOG("NVM_MiscIncSeqNum: allocate for temp buffer fail!!!\n");
		free(journal);
		return false;
	}
	memset(buffer, 0, info.writesize);

	tm = time(&tm);
	journal->h_magic = NVRAM_MISC_JOURNAL_MAGIC;
	journal->h_type = NVRAM_JOURNAL_SUPER_PAGE;
	journal->h_sequence = ++sequence;
	journal->h_utc_time = tm;


	offset = (loff_t)((loff_t)info.size - (loff_t)info.erasesize * (loff_t)nvram_misc_log_block_offset);
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("NVM_MiscIncSeqNum: dumchar open fail!!!\n");
		free(journal);
		free(buffer);
		return false;
	}
	iResult = lseek(fd, offset, SEEK_SET);
	if (iResult  < 0) {
		NVRAM_LOG("NVM_MiscIncSeqNum: seek log device fail!!!\n");
		free(journal);
		free(buffer);
		close(fd);
		return false;
	}
	memcpy(buffer, journal, sizeof(nvram_journal_handle_t));
	iResult = write(fd, buffer, info.writesize);
	if ((unsigned int)iResult != info.writesize) {
		NVRAM_LOG("NVM_MiscIncSeqNum: read nvram super page fail, iResult = %d !!!\n",
		          iResult);
		free(journal);
		free(buffer);
		close(fd);
		return false;
	}
	free(journal);
	free(buffer);
	close(fd);

	return true;
}


bool NVM_MiscLog(unsigned int level, const char *func, unsigned int line,
                 const char *log) {
	bool bRet = false;
	int iResult = 0;
	int fd;
	time_t tm;
	loff_t start_address;
	loff_t offset;
	unsigned int writepages_per_block = 0;
	unsigned int sequence = 0;
	nvram_journal_handle_t *journal;
	struct mtd_info_user info;

	char *buffer = NULL;
	char cMtdDevName[128] = {0};

	(void)(level);
	if (nvram_platform_layout_version == 1) {
		NVRAM_LOG("NVM_MiscLog: not support in layout version %d\n",
		          nvram_platform_layout_version);
		return false;
	}

	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {
		NVRAM_LOG("NVM_MiscLog: not support NAND\n");
		return false;
	}

	//Get the info about log_device(/dev/nvram), we use the last 16 block(2M) for nvram journal log
	if (nvram_platform_layout_version == 1) {
		NVRAM_LOG("NVM_MiscLog : couldn't support in this nvram layout %d\n",
		          nvram_platform_layout_version);
		return false;
	}
	memset(cMtdDevName, 0, sizeof cMtdDevName);
	//strcpy(cMtdDevName, g_MISC_DEVICE);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_MISC_DEVICE);
	bRet =  NVM_GetDeviceInfo(cMtdDevName, &info);
	if (false == bRet) {
		NVRAM_LOG("NVM_MiscLog : get device info failed!!!\n");
		return false;
	}
	if (info.writesize < sizeof(nvram_journal_handle_t) || info.size == 0) {
		NVRAM_LOG("NVM_MiscLog : journal size is larger than device write size!!!\n");
		return false;
	}

	//calculate the location for write log
	writepages_per_block = info.erasesize / info.writesize;
	sequence = NVM_MiscGetSeqNum();
	if (sequence < 0) {
		NVRAM_LOG("NVM_MiscLog : get sequence number fail!!!\n");
		return false;
	}


	//save NVRAM misc log at 3nd block of misc region
	start_address = info.size - (loff_t)info.erasesize * (loff_t)
	                nvram_misc_log_block_offset ;
	//sequence number always in first write unit
	offset = (loff_t)((loff_t)info.writesize * ((loff_t)sequence % (loff_t)writepages_per_block + 1));


	journal = (nvram_journal_handle_t *)malloc(sizeof(nvram_journal_handle_t));
	if (NULL == journal) {
		NVRAM_LOG("NVM_MiscLog : allocate for journal fail!!!\n");
		return false;
	}
	memset(journal, 0, sizeof(nvram_journal_handle_t));

	buffer = (char*)malloc(info.writesize);
	if (NULL == buffer) {
		NVRAM_LOG("NVM_MiscLog : allocate for temp buffer fail!!!\n");
		free(journal);
		return false;
	}
	memset(buffer, 0, info.writesize);

	tm = time(&tm);
	journal->h_magic = NVRAM_MISC_JOURNAL_MAGIC;
	journal->h_type = NVRAM_JOURNAL_LOG_PAGE;
	journal->h_sequence = sequence;
	journal->h_utc_time = tm;
	journal->h_func_line = line;
	snprintf(journal->h_func_name, sizeof(journal->h_func_name), "%s", func);
	snprintf(journal->h_log, MAX_LOG_LENGTH, "%s:%s", ctime(&tm), log);
	journal->h_log_lengeh = strlen(journal->h_log);

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVRAM_LOG("NVM_MiscLog : open log device fail!!!\n");
		free(journal);
		free(buffer);
		return false;
	}
	iResult = lseek(fd, start_address + offset, SEEK_SET);
	if (iResult  < 0) {
		NVRAM_LOG("NVM_MiscLog : seek log device fail start_address=%llx,offset=%llx!!!\n",
		          start_address, offset);
		free(journal);
		free(buffer);
		close(fd);
		return false;
	}
	memcpy(buffer, journal, sizeof(nvram_journal_handle_t));
	iResult = write(fd, buffer, info.writesize);
	if ((unsigned int)iResult != info.writesize) {
		NVRAM_LOG("NVM_MiscLog : write log device fail, iResult = %d !!!\n", iResult);
		free(journal);
		free(buffer);
		close(fd);
		return false;
	}
	free(journal);
	free(buffer);
	close(fd);
	//sync();
	bRet = NVM_MiscIncSeqNum(sequence);
	if ( false == bRet) {
		NVRAM_LOG("NVM_MiscLog : update sequence number fail!!!\n");
		return false;
	}

	return true;
}
bool NVM_InSpecialLidList(int file_lid, int *index) {
	int i;
	if (g_new_nvram_lid_count == 0) {
		NVRAM_LOG("The spcial lid is empty!!!\n");
		return false;
	}

	for (i = 0; i < (int)g_new_nvram_lid_count; i++) {
		if (file_lid == g_new_nvram_lid[i].lid) {
			*index = i;
			break;
		}
	}

	if (i == (int)g_new_nvram_lid_count)
		return false;
	else {
		NVRAM_LOG("%d is in new nvram partition!!!\n", file_lid);
		return true;
	}
}

bool Check_FileVerinFirstBoot(void) {
	int ret;
	int max_lid_num = 0;
	struct stat statbuf;

	NVRAM_LOG("Check FILE_VER in first boot\n");
	max_lid_num = NVM_Init();
	if (max_lid_num <= 0) {
		NVRAM_LOG("Check_FileVerinFirstBoot--NVM_Init fail !!!\n");
		return false;
	}
	ret = stat(g_pcNVM_Flag, &statbuf);
	if (-1 == ret) {
		NVRAM_LOG("No RestoreFlag\n");
		if (stat(g_akCFG_File[iFileVerInfoLID].cFileName, &statbuf) == -1
		        || !S_ISREG(statbuf.st_mode)
		        || statbuf.st_size != ((FILENAMELENGTH + FILEVERLENGTH) * (max_lid_num + 1))) {
			NVRAM_LOG("FILE_VER is invalid, generate it manually\n");
			return NVM_GenerateFileVer(true);
		}
	}
	return true;
}
bool Check_UpdateStatus(void) {
	struct stat info;
	struct stat statbuf;
	struct stat stb;
	int val = 0;
	int ret = 0;
	int vre = 0;

	NVRAM_LOG("Check if upgrade status\n");
	val = stat(g_pcNVM, &info);
	if (-1 == val) {
		NVRAM_LOG("No data/nvram folder and no need to change permission\n");
		return true;
	} else {
		ret = stat(g_pcNVM_MD, &statbuf);
		//------KK->L/KK.AOSP->L
		if ((info.st_gid != AID_SYSTEM) || (ret >= 0 && statbuf.st_uid != AID_RADIO
		                                    && statbuf.st_uid != AID_SYSTEM && statbuf.st_uid != AID_ROOT) || (ret >= 0
		                                            && statbuf.st_gid != AID_RADIO && statbuf.st_gid != AID_SYSTEM)) {
			NVRAM_LOG("Upgrade status, need to change all files' permission!!\n");
			if (chown("/mnt/vendor/nvdata", AID_ROOT, AID_SYSTEM)) {
				NVRAM_LOG("Fail to change all files' permission when upgrade\n");
				return false;
			}
		}
		vre = stat(g_pcNVM_BT, &stb);
		//------KK->cobranch/KK.AOSP->cobranch/L->cobranch
		if (vre >= 0 && stb.st_gid != AID_BLUETOOTH) {
			NVRAM_LOG("Upgrade status, need to change BT file permission!!\n");
			if (chown("/mnt/vendor/nvdata/APCFG/APRDEB/BT_Addr", AID_ROOT, AID_BLUETOOTH)) {
				NVRAM_LOG("Fail to change BT group permission when upgrade\n");
				return false;
			}
			if (chmod("/mnt/vendor/nvdata/APCFG", 0771) ||
				chmod("/mnt/vendor/nvdata/APCFG/APRDEB", 0771) ||
				chmod("/mnt/vendor/nvdata/APCFG/APRDCL", 0771) ||
				chmod("/mnt/vendor/nvdata/APCFG/APRDCL/FILE_VER", 0664)) {
				NVRAM_LOG("Fail to change BT folder permission when upgrade for /mnt/vendor/nvdata\n");
				return false;
			}
		}

		return true;
	}
}



