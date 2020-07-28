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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   libfile_op.c
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *    driver main function
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183)
 *
 *******************************************************************************/
#include <sys/types.h>
//add for log
#include <unistd.h>
#include <time.h>
//add for log
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/mount.h>
#include "libfile_op.h"
#include "libnvram_log.h"
#include "libnvram.h"
#include "CFG_file_public.h"
//#include "../../../../kernel/include/mtd/mtd-abi.h"
#include <mtd/mtd-abi.h>
#include "android_filesystem_config.h"
#include <pthread.h>
#include <cutils/properties.h>

// NVRAM Data Size About 1MB
//#define BinTitleSize   128*1*1024
//#define BinContentSize 128*7*1024
#define Loop_num    4096
#define MaxFileNum   936
#define INVALID_HANDLE_VALUE    -1

typedef struct {
	unsigned int ulCheckSum;
	unsigned int iCommonFileNum;
	unsigned int iCustomFileNum;
} BackupFileInfo;
typedef struct {
	char	cFileName[128];
	unsigned int iLID;
} FileName;

typedef struct {
	char cCheckFileName[128];
	int  iCheckFileIndex;
} CheckFileName;

typedef struct {
	char cRecordCallerName[128];
	int  iRecordCallerIndex;
} RecordCallerName;


//#define FILETITLE_START_BLOCK  IMAGE_MTK_META_START_BLOCK
//#define FILETILE_BLOCK_SIZE    1
//#define CONTENT_START_BLOCK    (FILETITLE_START_BLOCK + FILETILE_BLOCK_SIZE)
//#define CONTENT_BLOCK_SIZE     7
static const char *g_nvram_path_prefix = "/mnt/vendor/nvdata/";
static const char *g_pcNVM_AllFile    = "/mnt/vendor/nvdata/AllFile";
static const char *g_pcNVM_AllMap     = "/mnt/vendor/nvdata/AllMap";
static const char *g_pcNVM_AllFile_Check    = "/mnt/vendor/nvdata/AllFileCheck";
static const char *g_pcNVM_AllMap_Check     = "/mnt/vendor/nvdata/AllMapCheck";
static const char *g_pcNVM_Flag       =
    "/mnt/vendor/nvdata/RestoreFlag";//The File Will be Created after restore
static const char *g_pcNVM_BackFlag   =
    "/mnt/vendor/nvdata/BackupFlag";//The file will be updated after modifying important data

static const char *g_pcNVM_APCalFile  = "/mnt/vendor/nvdata/APCFG/APRDCL";
static const char *g_pcNVM_APRdebFile = "/mnt/vendor/nvdata/APCFG/APRDEB";
static const char *g_pcNVM_MDCalFile  = "/mnt/vendor/nvdata/md/NVRAM/CALIBRAT";
static const char *g_pcNVM_MDRdebFile = "/mnt/vendor/nvdata/md/NVRAM/NVD_IMEI";
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

unsigned int gFileStartAddr = 0;
pthread_mutex_t gFileStartAddrlock = PTHREAD_MUTEX_INITIALIZER;

extern int nvram_platform_log_block;
extern int nvram_platform_resv_block;
extern int nvram_platform_DM_block;
extern int nvram_platform_header_offset;
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define CHECK_FILE_EXIST_NUMBER 10
#define CHECK_FILE_EXIST_MARK 0x54535845
unsigned int gFileExistMark[CHECK_FILE_EXIST_NUMBER] = {0};
extern FileName aPerformance[];
extern FileName aBackupToBinRegion[];
extern CheckFileName aCheckExistList[];
extern RecordCallerName aRecordCallerList[];
extern unsigned int g_i4CFG_File_Count;
extern const unsigned int g_Backup_File_Count;
extern const unsigned int g_Check_File_Count;
extern const unsigned int g_Performance_File_Count;
extern unsigned int g_i4CFG_File_Custom_Count;
extern const unsigned int g_RecordCaller_Count;

extern int nvram_misc_log_feature;
extern int nvram_check_exist_feature;
extern int nvram_read_back_feature;
extern int nvram_check_exist_block_offset;
extern int nvram_platform_layout_version;
pthread_mutex_t dirlock = PTHREAD_MUTEX_INITIALIZER;

extern char g_NVRAM_BACKUP_DEVICE[128];

static void fix_legacy_nvram_path_in_binregion(char *path, int path_size) {
      static const char *legacy_nvram_path_prefix[] = {"/vendor/nvdata/", "/data/nvram/"};
      static bool is_check_legacy_path = false;
      static int legacy_path_index = -1;

      char path_suffix[MAX_NAMESIZE];
      int i;

      if(!is_check_legacy_path) {
        is_check_legacy_path = true;
        for (i=0; i < sizeof(legacy_nvram_path_prefix)/sizeof(char *); i++) {
            if(!strncmp(path, legacy_nvram_path_prefix[i], strlen(legacy_nvram_path_prefix[i]))) {
                legacy_path_index = i;
                NVRAM_LOG("Warnning, the nvram path in bin region is legacy path, %s\n", path);
                break;
            }
        }
      }

      if(legacy_path_index != -1) {
         snprintf(path_suffix, sizeof(path_suffix),"%s", path+strlen(legacy_nvram_path_prefix[legacy_path_index]));
         snprintf(path, path_size, "%s%s", g_nvram_path_prefix, path_suffix);
      }
}

static int open_file_with_dirs(const char *fn, mode_t mode) {
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
				if ((val == 0)) {
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
	filedesc = open(tmp, O_TRUNC | O_CREAT | O_WRONLY, mode);
	if (-1 != filedesc) {
		val = stat(tmp, &info);
#if 1
		if ((val == 0) ) {
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

#if 0
//-----------------------------------------------------------------------------
static bool FileOp_CopyFile(const char* src, const char* dst) {
	unsigned char acBuffer[1024];
	int iSrc, iDst, iFileSize, iWriteSize, iSize;
	bool bRet = false;
	struct stat statbuf;

	iSrc = open(src, O_RDONLY, S_IRUSR);
	iDst = open(dst, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (-1 == iSrc || -1 == iDst) {
		goto EXIT;
	}

	fstat(iSrc, &statbuf);
	iFileSize = (int)statbuf.st_size;

	while (iFileSize > 0) {
		iWriteSize = min(iFileSize, (int)sizeof(acBuffer));
		iSize = (int)read(iSrc, acBuffer, iWriteSize);
		if (iSize != iWriteSize) {
			goto EXIT;
		}
		iSize = (int)write(iDst, acBuffer, iWriteSize);
		if (iSize != iWriteSize) {
			goto EXIT;
		}
		iFileSize -= iWriteSize;
	}

	bRet = true;
EXIT:
	if (iDst != -1) {
		close(iDst);
	}
	if (iSrc != -1) {
		close(iSrc);
	}
	return bRet;
}
//-----------------------------------------------------------------------------
bool FileOp_CreateNVMFolder(void) {
	if (0 != mkdir("/nvram/APCFG", S_IRUSR | S_IWUSR)) {
		NVBAK_LOG("Err Create Directory APCFG Fail\n");
		return false;
	}

	if (0 != mkdir(g_pcNVM_APCalFile, S_IRUSR | S_IWUSR)) {
		NVBAK_LOG("Err CreateDirectory APCFG Fail\n");
		return false;
	}

	if (0 != mkdir(g_pcNVM_APRdebFile, S_IRUSR | S_IWUSR)) {
		NVBAK_LOG("Err CreateDirectory APCFG Fail\n");
		return false;
	}

	NVBAK_LOG("CreateData Folder Success!\n");
	return true;

}
//-----------------------------------------------------------------------------
bool FileOp_RestoreData(MetaData eRestoreType ) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	const char* lpSrcDirName = NULL;
	const char* lpDstDirName = NULL;
	char acSrcPathName[MAX_NAMESIZE];
	char acDstPathName[MAX_NAMESIZE];
	bool bRet = true;

	switch (eRestoreType) {
	case APBOOT:
		lpSrcDirName = g_pcBKP_APRdebFile;
		lpDstDirName = g_pcNVM_APRdebFile;
		break;

	case MDBOOT:
		lpSrcDirName = g_pcBKP_MDRdebFile;
		lpDstDirName = g_pcNVM_MDRdebFile;
		break;

	case APCLN:
		lpSrcDirName = g_pcBKP_APCalFile;
		lpDstDirName = g_pcNVM_APCalFile;
		break;

	case MDCLN:
		lpSrcDirName = g_pcBKP_MDCalFile;
		lpDstDirName = g_pcNVM_MDCalFile;
		break;
	case ALL:
	default:
		return false;
		break;
	}

	dir = opendir(lpSrcDirName);
	if (dir == NULL) {
		printf("ERROR FileOp_RestoreData open dir: %s", lpSrcDirName);
		return false;
	}

	while (1) {
		entry = readdir(dir);
		if (entry == NULL) {
			break;
		}
		lstat(entry->d_name, &statbuf);
		/*
		if (!strcmp(entry->d_name, ".")|| !strcmp(entry->d_name, "..")) {
			continue;
		}
		*/
		if (!S_ISREG(statbuf.st_mode)) {
			continue;
		}
		sprintf(acSrcPathName, "%s/%s", lpSrcDirName, entry->d_name);
		sprintf(acDstPathName, "%s/%s", lpDstDirName, entry->d_name);

		if (!FileOp_CopyFile(acSrcPathName, acDstPathName)) {
			printf("ERROR FileOp_RestoreData %s\n", lpSrcDirName);
			bRet = false;
			continue;
		}
	}
	closedir(dir);
	return bRet;
}
//-----------------------------------------------------------------------------
bool FileOp_BackupData(MetaData eBackupType ) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	const char* lpSrcDirName = NULL;
	const char* lpDstDirName = NULL;
	char acSrcPathName[MAX_NAMESIZE];
	char acDstPathName[MAX_NAMESIZE];
	bool bRet = true;

	switch (eBackupType) {
	case APBOOT:
		lpSrcDirName = g_pcNVM_APRdebFile;
		lpDstDirName = g_pcBKP_APRdebFile;
		break;

	case MDBOOT:
		lpSrcDirName = g_pcNVM_MDRdebFile;
		lpDstDirName = g_pcBKP_MDRdebFile;
		break;

	case APCLN:
		lpSrcDirName = g_pcNVM_APCalFile;
		lpDstDirName = g_pcBKP_APCalFile;
		break;

	case MDCLN:
		lpSrcDirName = g_pcNVM_MDCalFile;
		lpDstDirName = g_pcBKP_MDCalFile;
		break;

	case ALL:
	default:
		return false;
		break;
	}

	dir = opendir(lpSrcDirName);
	if (dir == NULL) {
		printf("ERROR FileOp_BackupData open dir: %s", lpSrcDirName);
		return false;
	}

	while (1) {
		entry = readdir(dir);
		if (entry == NULL) {
			break;
		}
		lstat(entry->d_name, &statbuf);
		/*
		if (!strcmp(entry->d_name, ".")|| !strcmp(entry->d_name, "..")) {
			continue;
		}
		*/
		if (!S_ISREG(statbuf.st_mode)) {
			continue;
		}
		sprintf(acSrcPathName, "%s/%s", lpSrcDirName, entry->d_name);
		sprintf(acDstPathName, "%s/%s", lpDstDirName, entry->d_name);

		if (!FileOp_CopyFile(acSrcPathName, acDstPathName)) {
			printf("ERROR FileOp_BackupData %s\n", lpSrcDirName);
			bRet = false;
			continue;
		}
	}
	closedir(dir);
	return bRet;
}
//-----------------------------------------------------------------------------
bool FileOp_DeleteData(const char* pcDirName) {
	DIR *dir = NULL;
	struct dirent *entry;
	struct stat statbuf;
	char acLongPathName[MAX_NAMESIZE];

	NVBAK_LOG("Delete Data\n");

	//find the first file
	dir = opendir(pcDirName);
	if (dir == NULL) {
		NVBAK_LOG("Cannot open FindFirstFile file.\n");
		return false;
	}

	//get the next nvram file in the folder
	while (1) {
		entry = readdir(dir);
		if (entry == NULL) {
			break;
		}

		lstat(entry->d_name, &statbuf);
		/*
		if (!strcmp(entry->d_name, ".")|| !strcmp(entry->d_name, "..")) {
			NVBAK_LOG("open .........input file.\n");
			continue;
		}
		*/
		if (!S_ISREG(statbuf.st_mode)) {
			continue;
		}

		NVBAK_LOG("the file is: %c%c%c%c%c%c \n",
		          entry->d_name[0], entry->d_name[1], entry->d_name[2],
		          entry->d_name[3], entry->d_name[4], entry->d_name[5]);

		sprintf(acLongPathName, "%s/%s", pcDirName, entry->d_name);

		remove(acLongPathName);
	}

	closedir(dir);

	return true;
}
//-----------------------------------------------------------------------------
void FileOp_BackupAll(void ) {
	FileOp_BackupData(APBOOT);
	FileOp_BackupData(MDBOOT);
	FileOp_BackupData(APCLN);
	FileOp_BackupData(MDCLN);
}
//-----------------------------------------------------------------------------
void FileOp_RestoreAll(void ) {
	//restore the all files to fat2 partition one by one type
	FileOp_RestoreData(APBOOT);
	FileOp_RestoreData(MDBOOT);
	FileOp_RestoreData(APCLN);
	FileOp_RestoreData(MDCLN);
}
//-----------------------------------------------------------------------------
void FileOp_DeleteAll(void ) {
	FileOp_CreateNVMFolder();
	FileOp_DeleteData(g_pcNVM_APRdebFile);
	FileOp_DeleteData(g_pcNVM_APCalFile);
}
//-----------------------------------------------------------------------------
void FileOp_DeleteRdebData(void ) {
	FileOp_CreateNVMFolder();
	FileOp_DeleteData(g_pcNVM_APRdebFile);
}
//-----------------------------------------------------------------------------
void FileOp_RestoreBootData(void ) {
	FileOp_RestoreData(APBOOT);
	FileOp_RestoreData(MDBOOT);
}
#endif

//#define Backup_PARTITION 1
#define CleanBootFlag 0x12345678
#define BitmapFlag 0xAABBCCDD
//#define forDMBlockNum 2
int  NvRamBlockNum = 0;
int BinRegionBlockTotalNum = 0;
static char *bBadBlockBitMap = NULL;
bool FileOp_GetBadBlockBitMap(char* cMtdDevName) {
	int fd, iResult;
	char *tempBuffer = NULL;
	char *tempBitmap1 = NULL;
	char *tempBitmap2 = NULL;
	int i, j, pos = 0, iBlockSize, iBlockNum, flag = 0;
	bool bSuccessFound = false;
	struct mtd_info_user info;
	NVBAK_LOG("[NVRAM]:GetBadBlockBitMap\r\n");


	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_GetBadBlockBitMap get device info error\r\n");

		return false;
	}
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		return false;
	}


	iBlockSize = info.erasesize;
	//iPartitionSize=info.size;
	if (nvram_platform_layout_version == 0)
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
		NVBAK_LOG("[NVRAM]:memory malloc error,errono:%d,error:%s\r\n", errno,
		          strerror(errno));
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
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		return false;
	}

	while (iBlockNum > 0) {
		flag = 0;
		iBlockNum--;
		NVBAK_LOG("[NVRAM]:iBlockNum:%d\n", iBlockNum);
		lseek(fd, iBlockNum * iBlockSize, SEEK_SET);
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
	} else {
		NVBAK_LOG("[NVRAM]:Find bad block bit map!\r\n");
		bBadBlockBitMap = NULL;
		bBadBlockBitMap = (char*)malloc(NvRamBlockNum * sizeof(char));
		if (bBadBlockBitMap == NULL) {
			NVBAK_LOG("[NVRAM]:malloc bBadBlockBitMap Failed!\r\n");
			close(fd);
			free(tempBuffer);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}
		memcpy(bBadBlockBitMap, tempBitmap1, NvRamBlockNum * sizeof(char));
		NVBAK_LOG("[NVRAM]:find bBadBlockBitMap!\n");
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return true;
	}

}
bool FileOp_SetCleanBootFlag(bool bSetFlag) {
	int fd, iResult;
	char cMtdDevName[128] = {0};
	char *tempBuffer = NULL;
	struct mtd_info_user info;
	struct erase_info_user erase_info;
	int iWriteSize, iBlockSize;
	unsigned int iCleanBootFlag;
	int i, pos = 0;
//   int iBackup_Partition=get_partition_numb("nvram");

	if (NVM_Init()) {
		NVBAK_LOG("[NVRAM]:FileOp_SetCleanBootFlag--NVM_Init Fail !!!\n");
		return false;
	}
	memset(cMtdDevName, 0, sizeof cMtdDevName);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);

	NVBAK_LOG("[NVRAM]:FileOp_SetCleanBootFlag\n");
	NVBAK_LOG("[NVRAM]:cMtdDevName:%s\n", cMtdDevName);
	if (!FileOp_GetBadBlockBitMap(cMtdDevName)) {
		NVBAK_LOG("[NVRAM]:GetBadBlockBitMap Failed\r\n");
		return false;
	}
	NVBAK_LOG("[NVRAM]:FileOp_SetCleanBootFlag:find BadBlockBitMap\n");

	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_SetCleanBootFlag get device info error\r\n");
		free(bBadBlockBitMap);
		return false;
	}
	fd = open(cMtdDevName, O_RDWR);
	if ( fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error %s\r\n", (char*)strerror(errno));
		free(bBadBlockBitMap);
		return false;
	}




	iWriteSize = info.writesize;
	iBlockSize = info.erasesize;

	NVBAK_LOG("NvRamBlockNum:%d\r\n", NvRamBlockNum);
	tempBuffer = (char *)malloc(iBlockSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		free(bBadBlockBitMap);
		return false;
	}
	memset(tempBuffer, 0xFF, iBlockSize);
	for (i = 0; i < NvRamBlockNum; i++) {
		if (bBadBlockBitMap[i] == 0) {
			pos = i;
			break;
		}
	}
	if (i == NvRamBlockNum) {
		NVBAK_LOG("[NVRAM]:there are not enough good blocks for set cleanboot flag\r\n");
		close(fd);
		free(tempBuffer);
		free(bBadBlockBitMap);
		return false;
	}
	free(bBadBlockBitMap);
	lseek(fd, pos * iBlockSize, SEEK_SET);
	iResult = read(fd, tempBuffer, iBlockSize);
	if (iResult < 0) {
		NVBAK_LOG("[NVRAM]:mtd read error\r\n");
		close(fd);
		free(tempBuffer);
		return false;
	}
	//set clean boot flag
	if (bSetFlag) {
		*((unsigned int*)(tempBuffer + 2 * sizeof(unsigned int))) = CleanBootFlag;
	} else {
		*((unsigned int*)(tempBuffer + 2 * sizeof(unsigned int))) = 0xFFFFFFFF;
	}
	lseek(fd, pos * iBlockSize, SEEK_SET);
	erase_info.start = pos * iBlockSize;
	erase_info.length = iBlockSize;




	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {

		if (!NVM_EraseDeviceBlock(cMtdDevName, erase_info)) {
			NVBAK_LOG("[NVRAM]:mtd erase error\r\n");
			close(fd);
			free(tempBuffer);
			return false;
		}
	}

	lseek(fd, pos * iBlockSize, SEEK_SET);

	iResult = write(fd, tempBuffer, iBlockSize);
	if (iResult != iBlockSize) {
		NVBAK_LOG("[NVRAM]:mtd write error\r\n");
		close(fd);
		free(tempBuffer);
		return false;
	}

	lseek(fd, pos * iBlockSize, SEEK_SET);

	iResult = read(fd, tempBuffer, iBlockSize);
	if (iResult < 0 ) {
		NVBAK_LOG("[NVRAM]:mtd re-read error\r\n");
		close(fd);
		free(tempBuffer);
		return false;
	}

	iCleanBootFlag = *((unsigned int*)(tempBuffer + 2 * sizeof(unsigned int)));
	NVBAK_LOG("[NVRAM]:set/clear clean boot:%x,pos:%d\n", iCleanBootFlag, pos);
	if (bSetFlag) {
		if (iCleanBootFlag != CleanBootFlag) {
			NVBAK_LOG("[NVRAM]:mtd set clean boot flag error\r\n");
			close(fd);
			free(tempBuffer);
			return false;
		}
	} else {
		if (iCleanBootFlag != 0xFFFFFFFF) {
			NVBAK_LOG("[NVRAM]:mtd set clean boot flag error\r\n");
			close(fd);
			free(tempBuffer);
			return false;
		}
	}

	NVBAK_LOG("[NVRAM]:mtd set clean boot flag success\r\n");
	close(fd);
	free(tempBuffer);
	return true;
}

bool Fileop_SetBackupFileNum(bool bSetFlag) {
	//int iFileDesc,iResult;
	int fd, iResult, iFileDesc;
	char cMtdDevName[128] = { 0 };
	char *tempBuffer = NULL;
	struct mtd_info_user info;
	struct erase_info_user erase_info;
	int iWriteSize, iBlockSize;
	unsigned int iBackupFileNum = 0, iBackupFileNum1, iMapFileSize, open_error = 0;
	unsigned short iMDBackupNum = 0, iMDBackupNum1;
	int i, pos = 0;
//  int iBackup_Partition=get_partition_numb("nvram");
	NVBAK_LOG("[NVRAM]:Fileop_SetBackupFileNum:find BadBlockBitMap\n");
	memset(cMtdDevName, 0, sizeof cMtdDevName);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);
	NVBAK_LOG("[NVRAM]:cMtdDevName:%s\n", cMtdDevName);
	NVBAK_LOG("[NVRAM]:Fileop_SetBackupFileNum\n");
	iFileDesc = open(g_pcNVM_BackFlag, O_RDWR);
	if (iFileDesc == -1) {
		NVBAK_LOG("[NVRAM]:can not open BackupFileNum Flag\n");
		++open_error;
	} else {
		if (!NVM_CheckBackFlag(iFileDesc)) {
			NVBAK_LOG("[NVRAM]:Fileop_SetBackupFileNum Backflag is wrong\n");
			unlink(g_pcNVM_BackFlag);
			close(iFileDesc);
			goto mdflag;
		}
		if (-1 == lseek(iFileDesc, 0, SEEK_SET)) {
			NVRAM_LOG("[NVRAM]:Fileop_SetBackupFileNum lseek error\n");
			iBackupFileNum = 0;
			close(iFileDesc);
			goto mdflag;
		}
		iResult = read(iFileDesc, &iBackupFileNum, sizeof(unsigned int));
		if (iResult != sizeof(unsigned int)) {
			NVBAK_LOG("[NVRAM]:can not read BackupFileNum Flag(ignore):%d\n", iResult);
			iBackupFileNum = 0;
			unlink(g_pcNVM_BackFlag);
		}
		close(iFileDesc);
	}
mdflag:
	iResult = 0;
	iFileDesc = -1;
	iFileDesc = open(g_pcNVM_ModomBackNum, O_RDWR);
	if (iFileDesc == -1) {
		NVBAK_LOG("[NVRAM]:can not open ModomBackNum Flag\n");
		++open_error;
	} else {
		if (!NVM_CheckMDBackFlag(iFileDesc)) {
			NVBAK_LOG("[NVRAM]:MDBackflag is wrong!\n");
			unlink(g_pcNVM_ModomBackNum);
			close(iFileDesc);
			goto readbackupdone;
		}
		if (-1 == lseek(iFileDesc, 0, SEEK_SET)) {
			NVRAM_LOG("Fileop_SetBackupFileNum:lseek MDBackflag error\n");
			close(iFileDesc);
			iMDBackupNum = 0;
			goto readbackupdone;
		}
		iResult = read(iFileDesc, &iMDBackupNum, sizeof(unsigned short));
		if (iResult != sizeof(unsigned short)) {
			NVBAK_LOG("[NVRAM]:can not read ModomBackNum Flag(ignore):%d\n", iResult);
			iMDBackupNum = 0;
			unlink(g_pcNVM_ModomBackNum);
		}
		close(iFileDesc);
	}
	if (open_error >= 2) {
		NVBAK_LOG("[NVRAM]:BackupFileNum Flag and ModemBackupNum not exist!\n");
	}
	NVBAK_LOG("[NVRAM]:AP modified file Num:%d\n", iBackupFileNum);

	NVBAK_LOG("[NVRAM]:MD modified file Num:%d\n", iMDBackupNum);
	//search the first avaiable block to write
readbackupdone:
	if (!FileOp_GetBadBlockBitMap(cMtdDevName)) {
		NVBAK_LOG("[NVRAM]:GetBadBlockBitMap Failed\r\n");
		return false;
	}

	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:Fileop_SetBackupFileNum get device info error\r\n");
		free(bBadBlockBitMap);
		return false;
	}
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		free(bBadBlockBitMap);
		return false;
	}



	iWriteSize = info.writesize;
	iBlockSize = info.erasesize;


	tempBuffer = malloc(iBlockSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		free(bBadBlockBitMap);
		return false;
	}
	memset(tempBuffer, 0xFF, iBlockSize);
	for (i = 0; i < NvRamBlockNum; i++) {
		if (bBadBlockBitMap[i] == 0) {
			pos = i;
			break;
		}
	}
	if (i == NvRamBlockNum) {
		NVBAK_LOG("[NVRAM]:there are not enough good blocks for set cleanboot flag\r\n");
		close(fd);
		free(tempBuffer);
		free(bBadBlockBitMap);
		return false;
	}
	free(bBadBlockBitMap);
	lseek(fd, pos * iBlockSize, SEEK_SET);
	iResult = read(fd, tempBuffer, iBlockSize);
	if (iResult < 0) {
		NVBAK_LOG("[NVRAM]:mtd read error\r\n");
		close(fd);
		free(tempBuffer);
		return false;
	}
	iMapFileSize = *((unsigned int*)(tempBuffer));
	if ((iMapFileSize + 3 * sizeof(unsigned int)) > (iBlockSize - sizeof(
	            unsigned int) - sizeof(unsigned short))) {
		NVBAK_LOG("[NVRAM]:there is no space for storing backup file num\n");
		close(fd);
		free(tempBuffer);
		return false;
	}
	if (bSetFlag) {
		*((unsigned int*)(tempBuffer + 3 * sizeof(unsigned int) + iMapFileSize)) =
		    iBackupFileNum;
		*((unsigned short*)(tempBuffer + 3 * sizeof(unsigned int) + iMapFileSize +
		                    sizeof(unsigned int))) = iMDBackupNum;
	} else {
		*((unsigned int*)(tempBuffer + 3 * sizeof(unsigned int) + iMapFileSize)) =
		    0xFFFFFFFF;
		*((unsigned short*)(tempBuffer + 3 * sizeof(unsigned int) + iMapFileSize +
		                    sizeof(unsigned int))) = 0xFFFF;
	}

	lseek(fd, pos * iBlockSize, SEEK_SET);
	erase_info.start = pos * iBlockSize;
	erase_info.length = iBlockSize;


	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {

		if (!NVM_EraseDeviceBlock(cMtdDevName, erase_info)) {
			NVBAK_LOG("[NVRAM]:mtd erase error\r\n");
			close(fd);
			free(tempBuffer);
			return false;
		}
	}

	lseek(fd, pos * iBlockSize, SEEK_SET);

	iResult = write(fd, tempBuffer, iBlockSize);
	if (iResult != iBlockSize) {
		NVBAK_LOG("[NVRAM]:mtd write error\r\n");
		close(fd);
		free(tempBuffer);
		return false;
	}

	lseek(fd, pos * iBlockSize, SEEK_SET);

	iResult = read(fd, tempBuffer, iBlockSize);
	if (iResult < 0 ) {
		NVBAK_LOG("[NVRAM]:mtd re-read error\r\n");
		close(fd);
		free(tempBuffer);
		return false;
	}

	iBackupFileNum1 = *((unsigned int*)(tempBuffer + 3 * sizeof(
	                                        unsigned int) + iMapFileSize));
	iMDBackupNum1 = *((unsigned short*)(tempBuffer + 3 * sizeof(
	                                        unsigned int) + iMapFileSize + sizeof(unsigned int)));
	NVBAK_LOG("[NVRAM]:set AP backup file num:%d,MD backup file num:%d,pos:%d\n",
	          iBackupFileNum1, iMDBackupNum1, pos);
	if (bSetFlag) {
		if (iBackupFileNum1 != iBackupFileNum || iMDBackupNum1 != iMDBackupNum) {
			NVBAK_LOG("[NVRAM]:mtd set File Backup Num error\r\n");
			close(fd);
			free(tempBuffer);
			return false;
		}
	} else {
		if (iBackupFileNum1 != 0xFFFFFFFF || iMDBackupNum1 != 0xFFFF) {
			NVBAK_LOG("[NVRAM]:set File Backup Num error\r\n");
			close(fd);
			free(tempBuffer);
			return false;
		}
	}

	NVBAK_LOG("[NVRAM]:Fileop_SetBackupFileNum success\r\n");
	close(fd);
	free(tempBuffer);
	return true;
}
bool FileOp_GetCleanBootFlag(unsigned int * iCleanBootFlag) {
	int fd, iResult;
	char cMtdDevName[128] = { 0 };
	char *tempBuffer = NULL;
	char *tempBitmap1 = NULL;
	char *tempBitmap2 = NULL;
	int i, j, pos = 0, iBlockSize, iBlockNum, flag = 0;
	bool bSuccessFound = false;
	struct mtd_info_user info;
	//int iBackup_Partition=get_partition_numb("nvram");


	if (iCleanBootFlag == NULL) {
		NVBAK_LOG("[NVRAM]:the pointer of iCleanBootFlag is error\r\n");
		return false;
	}

	memset(cMtdDevName, 0, sizeof cMtdDevName);

	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);


	NVBAK_LOG("[NVRAM]:cMtdDevName:%s\n", cMtdDevName);




	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_GetCleanBootFlag get device info error\r\n");
		return false;
	}

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		return false;
	}

	iBlockSize = info.erasesize;
	//iPartitionSize=info.size;
	BinRegionBlockTotalNum = info.size / iBlockSize;
	NvRamBlockNum = BinRegionBlockTotalNum - nvram_platform_DM_block;
	tempBuffer = (char*)malloc(iBlockSize);
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
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		return false;
	}

	while (iBlockNum > 0) {
		flag = 0;
		iBlockNum--;
		NVBAK_LOG("[NVRAM]:iBlockNum:%d\n", iBlockNum);
		lseek(fd, iBlockNum * iBlockSize, SEEK_SET);
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
	for (i = 0; i < iBlockNum; i++) {
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

	iResult = lseek(fd, pos * iBlockSize + 2 * sizeof(unsigned int), SEEK_SET);
	if (iResult != (int)(pos * iBlockSize + 2 * sizeof(unsigned int))) {
		NVBAK_LOG("[NVRAM]:mtd lseek error\r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}

	iResult = read(fd, iCleanBootFlag, sizeof(unsigned int));
	if (iResult != sizeof(unsigned int)) {
		NVBAK_LOG("[NVRAM]:mtd read error\r\n");
		free(tempBuffer);
		close(fd);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	NVBAK_LOG("[NVRAM]:Clean Boot Flag:%x\n", *iCleanBootFlag);
	NVBAK_LOG("[NVRAM]:mtd get CleanBoot Flag success\r\n");
	close(fd);
	free(tempBuffer);
	free(tempBitmap1);
	free(tempBitmap2);
	return true;

}

bool FileOp_CmpBackupFileNum() {
	unsigned int iSavedAPBackupFileNum = 0, iAPBackupFileNum = 0, open_error = 0;
	unsigned short iSavedMDBackupFileNum = 0, iMDBackupFileNum = 0;
	int iFileDesc, iResult, iFileDesc_Flag;
	struct stat statbuf;
	unsigned int iCompleteFlag;

	NVBAK_LOG("[NVRAM]:FileOp_CmpBackupFileNum\n");
	iFileDesc = open(g_pcNVM_BackFlag, O_RDWR);
	if (iFileDesc == -1) {
		NVBAK_LOG("[NVRAM]:can not open BackupFileNum Flag1,maybe this file doesn't exist\n");
		//there is no important data to modify
		++open_error;
	} else {
		if (!NVM_CheckBackFlag(iFileDesc)) {
			NVBAK_LOG("[NVRAM]:Backflag is wrong\n");
			goto recover;
		}
		if (-1 == lseek(iFileDesc, 0, SEEK_SET)) {
			NVRAM_LOG("FileOp_CmpBackupFileNum:lseek BackupFileNum error\n");
			close(iFileDesc);
			return true;
		}
		iResult = read(iFileDesc, &iAPBackupFileNum, sizeof(unsigned int));
		if (iResult != sizeof(unsigned int)) {
			NVBAK_LOG("[NVRAM]:can not read BackupFileNum Flag:%d\n", iResult);
			close(iFileDesc);
			return true;
		}
		close(iFileDesc);
	}

	iFileDesc = open(g_pcNVM_ModomBackNum, O_RDWR);
	if (iFileDesc == -1) {
		NVBAK_LOG("[NVRAM]:can not open ModomBackNum Flag\n");
		++open_error;
	} else {
		if (!NVM_CheckMDBackFlag(iFileDesc)) {
			NVBAK_LOG("[NVRAM]:MDBackflag is wrong!\n");
			unlink(g_pcNVM_ModomBackNum);
			close(iFileDesc);
			return true;
		}
		if (-1 == lseek(iFileDesc, 0, SEEK_SET)) {
			NVRAM_LOG("FileOp_CmpBackupFileNum:lseek MDBackflag error\n");
			close(iFileDesc);
			return true;
		}
		iResult = read(iFileDesc, &iMDBackupFileNum, sizeof(unsigned short));
		if (iResult != sizeof(unsigned short)) {
			NVBAK_LOG("[NVRAM]:can not read ModomBackNum Flag:%d\n", iResult);
			close(iFileDesc);
			return true;
		}
		if (iMDBackupFileNum == 0)
			++open_error;
		close(iFileDesc);
	}
	if (open_error >= 2) {
		NVBAK_LOG("[NVRAM]:BackupFileNum Flag and ModemBackupNum not exist!\n");
		return true;
	}

	if (NVM_GetBackupFileNum(&iSavedAPBackupFileNum, &iSavedMDBackupFileNum)) {
		if (iSavedAPBackupFileNum != iAPBackupFileNum
		        || iSavedMDBackupFileNum != iMDBackupFileNum) {
			iResult = stat(g_pcNVM_Flag, &statbuf);
			if (-1 == iResult) {
				NVBAK_LOG("[NVRAM]:Last Restore is failed, can not trigger auto backup!!!");
				return true;
			}
			iFileDesc_Flag = open(g_pcNVM_Flag, O_RDONLY);
			if (-1 == iFileDesc_Flag) {
				NVBAK_LOG("[NVRAM]:Open Restore Flag file failed!!!");
				return true;
			}
			iResult = read(iFileDesc_Flag, &iCompleteFlag, sizeof(unsigned int));
			if (iResult != sizeof(unsigned int)) {
				NVBAK_LOG("[NVRAM]:Read retore Flag file failed!!!");
				close(iFileDesc_Flag);
				return true;
			}
			if (0x12345678 != iCompleteFlag) {
				NVBAK_LOG("[NVRAM]:Restore Flag is error!!!");
				close(iFileDesc_Flag);
				return true;
			}
			close(iFileDesc_Flag);

			NVBAK_LOG("[NVRAM]:SavedAPBackupFileNum:%d,APBackupFileNum:%d\n",
			          iSavedAPBackupFileNum, iAPBackupFileNum);
			NVBAK_LOG("[NVRAM]:SavedMDBackupFileNum:%d,MDBackupFileNum:%d\n",
			          iSavedMDBackupFileNum, iMDBackupFileNum);
			NVBAK_LOG("[NVRAM]:Saved and Current NOT match!Back up!\n");
			//close(iFileDesc);
			return false;
		}
	} else {
		NVBAK_LOG("[NVRAM]:Get SavedBackupFileNum Fail\n");
		//close(iFileDesc);
		return false;
	}
	NVBAK_LOG("[NVRAM]:SavedAPBackupFileNum:%d,APBackupFileNum:%d SUCCESS\n",
	          iSavedAPBackupFileNum, iAPBackupFileNum);
	NVBAK_LOG("[NVRAM]:SavedMDBackupFileNum:%d,MDBackupFileNum:%d SUCCESS\n",
	          iSavedMDBackupFileNum, iMDBackupFileNum);
	NVBAK_LOG("[NVRAM]:Saved and Current match!\n");
	return true;
recover:
	if (NVM_GetBackupFileNum(&iSavedAPBackupFileNum, &iSavedMDBackupFileNum)) {
		close(iFileDesc);
		iFileDesc = open(g_pcNVM_BackFlag, O_TRUNC | O_RDWR);
            if(INVALID_HANDLE_VALUE == iFileDesc)
            {
                NVBAK_LOG("[NVRAM]: cannot open backup flag\n");
                return false;
            }
		if (iSavedAPBackupFileNum == 0xFFFFFFFF ) {
			NVBAK_LOG("[NVRAM]:FileOp_CmpBackupFileNum:SavedAPBackupFileNum:%d\n",
			          iSavedAPBackupFileNum);
			unlink(g_pcNVM_BackFlag);
			close(iFileDesc);
			return true;
		}
		if (-1 == lseek(iFileDesc, 0, SEEK_SET)) {
			NVRAM_LOG("FileOp_CmpBackupFileNum:lseek error\n");
			unlink(g_pcNVM_BackFlag);
			close(iFileDesc);
			return true;
		}
		iResult = write(iFileDesc, &iSavedAPBackupFileNum, sizeof(unsigned int));
		if (iResult != sizeof(unsigned int)) {
			NVBAK_LOG("[NVRAM]:FileOp_CmpBackupFileNum:can not write BackupFileNum Flag:%d\n",
			          iResult);
			unlink(g_pcNVM_BackFlag);
			close(iFileDesc);
			return true;
		}
		if (!NVM_ComputeBackflagCheckSum(iFileDesc)) {
			NVBAK_LOG("[NVRAM]:FileOp_CmpBackupFileNum:Backflag compute check sum failed(ignore)\r\n");
			unlink(g_pcNVM_BackFlag);
		}
		close(iFileDesc);
		return true;
	} else {
		NVBAK_LOG("[NVRAM]:FileOp_CmpBackupFileNum:Get SavedBackupFileNum Fail\n");
		unlink(g_pcNVM_BackFlag);
		close(iFileDesc);
		return true;
	}
}

bool FileOp_CreateBinRegionBadBlockBitMap() {
	int fd, iResult, i;
	char cMtdDevName[128] = {0};
	int iWriteSize, iBlockSize, iPartitionSize, iBlockNum;
	struct mtd_info_user info;
	struct erase_info_user erase_info;


	memset(cMtdDevName, 0, sizeof cMtdDevName);
	//int iBackup_Partition=get_partition_numb("nvram");
	/*
	if(TotalBlockNum==NULL)
		{
	   NVBAK_LOG("[NVRAM]:input pointer of blocknum is NULL\r\n");
	   return false;
	}
	*/


	NVBAK_LOG("[NVRAM]:FileOp_CreateBinRegionBadBlockBitMap:%s\n", cMtdDevName);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);
	NVBAK_LOG("[NVRAM]:FileOp_CreateBinRegionBadBlockBitMap:%s\n", cMtdDevName);


	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_CreateBinRegionBadBlockBitMap get device info error\r\n");
		return false;
	}

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		return false;
	}

	iWriteSize = info.writesize;
	iBlockSize = info.erasesize;
	iPartitionSize = info.size;

	BinRegionBlockTotalNum = iPartitionSize / iBlockSize - nvram_platform_log_block
	                         - nvram_platform_resv_block;
	if (nvram_platform_layout_version == 0)
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
	bBadBlockBitMap = NULL;
	bBadBlockBitMap = (char*)malloc(NvRamBlockNum);
	if (bBadBlockBitMap == NULL) {
		NVBAK_LOG("[NVRAM]:malloc bBadBlockBitMap error!!\r\n");
		close(fd);
		return false;
	}
	memset(bBadBlockBitMap, 0, (NvRamBlockNum * sizeof(char)));
	iBlockNum = NvRamBlockNum;
	NVBAK_LOG("[NVRAM]:mtd get info:iBlockSize:%d,iPartitionSize:%d,iBlockNum:%d\n",
	          iBlockSize, iPartitionSize, iBlockNum);

	for (i = 0; i < iBlockNum; i++) {
		erase_info.start = i * iBlockSize;
		erase_info.length = iBlockSize;
		if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {

			if (!NVM_EraseDeviceBlock(cMtdDevName, erase_info)) {
				NVBAK_LOG("[NVRAM]:mtd erase error,block id:%d\r\n", i);
				bBadBlockBitMap[i] = 1;
			} else {
				bBadBlockBitMap[i] = 0;
			}

		}
	}
	close(fd);
	#if 0
	for (i = 0; i < iBlockNum; i++) {
		NVBAK_LOG("[NVRAM_Bitmap]:Block%d,%d\n", i, bBadBlockBitMap[i]);
	}
	#endif
	return true;

}

bool FileOp_CheckFileExist() {
	int i, fd, iResult, pos;
	unsigned int iWriteSize, iBlockSize, iPartitionSize;
	struct stat st;
	struct mtd_info_user info;
	struct erase_info_user erase_info;
	loff_t start_address;
	char cMtdDevName[128] = {0};



	if (nvram_platform_layout_version == 1) {
		NVBAK_LOG("[NVRAM]:FileOp_CheckFileExist not support in this layout.\r\n");
		return false;
	}


	memset(cMtdDevName, 0, sizeof cMtdDevName);
	memset(gFileExistMark, 0, sizeof(gFileExistMark));
	gFileExistMark[0] = CHECK_FILE_EXIST_MARK;
	if (g_Check_File_Count == 0)
		return true;
	for (i = 0; i < g_Check_File_Count; i++) {
		if (stat(aCheckExistList[i].cCheckFileName, &st) == 0) {
			gFileExistMark[aCheckExistList[i].iCheckFileIndex / 32 + 1] |= (0x1U <<
			        (aCheckExistList[i].iCheckFileIndex - (aCheckExistList[i].iCheckFileIndex / 32)
			         * 32));
		} else {
			NVBAK_LOG("FileOp_CheckFileExist stat %s fail: %s",
			          aCheckExistList[i].cCheckFileName, (char*)strerror(errno));
			gFileExistMark[aCheckExistList[i].iCheckFileIndex / 32 + 1] &= ~(0x1U <<
			        (aCheckExistList[i].iCheckFileIndex - (aCheckExistList[i].iCheckFileIndex / 32)
			         * 32));
		}
	}

	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:FileOp_CheckFileExist open /dev/nvram error\r\n");
		return false;
	}




	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_CheckFileExist get device info error\r\n");
		close(fd);
		return false;
	}
	start_address = info.size - ((loff_t)nvram_platform_log_block +
	                             (loff_t)nvram_check_exist_block_offset ) * (loff_t)info.erasesize;
	if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {
		erase_info.start = start_address ;
		erase_info.length = info.erasesize;
		if (!NVM_EraseDeviceBlock(cMtdDevName, erase_info)) {
			NVRAM_LOG("[NVRAM]:FileOp_CheckFileExist erase  block fail!!!\n");
			close(fd);
			return false;
		}
	}
	iResult = lseek(fd, start_address , SEEK_SET);
	if (iResult  < 0) {
		NVRAM_LOG("NVRAM]:FileOp_CheckFileExist seek log device fail!!!\n");
		close(fd);
		return false;
	}
	iResult = write(fd, gFileExistMark, CHECK_FILE_EXIST_NUMBER * 4);
	if (iResult != (int)(CHECK_FILE_EXIST_NUMBER * 4)) {
		NVBAK_LOG("[NVRAM]:FileOp_CheckFileExist write mark error\r\n");
		close(fd);
		return false;
	}
	close(fd);
	NVBAK_LOG("FileOp_CheckFileExist exist .\n");
	return true;
}

bool FileOp_CreatMdFileList(const char* SrcDirName)
{
	DIR *dir = NULL;
    struct dirent *entry;
	char filelist[MAX_NAMESIZE];
	char filelistPathName[MAX_NAMESIZE];
	int filelistfd;
	int pos =0;
	int iResult =0;
    NVBAK_LOG("open dir %s !", SrcDirName);
    if (SrcDirName) {
		dir = opendir(SrcDirName);
	}

        if (dir == NULL)
        {
    		NVBAK_LOG("FileOp_CreatMdFileList %s ,this dir not exist!", SrcDirName);
			return false;
        }
    sprintf(filelistPathName, "%s/FILELIST", SrcDirName);
		filelistfd = open(filelistPathName, O_TRUNC|O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
        while(1)
	{
                entry = readdir(dir);
                if (entry == NULL)
                {
                     break;
                }
                if (!strcmp(entry->d_name, ".")|| !strcmp(entry->d_name, ".."))
                {
                     continue;
                }
                snprintf(filelist, sizeof(filelist), "%s/%s", SrcDirName, entry->d_name);
                NVBAK_LOG("FileOp_CreatMdFileList: (%s)\n", filelist);
                iResult = write(filelistfd,entry->d_name,strlen(entry->d_name)+1);
		if (iResult != (strlen(entry->d_name)+1)) {
      			NVBAK_LOG("[FileOp_CreatMdFileList write error\r\n");
      			close(filelistfd);
				closedir(dir);
      			return false;
    		}
		pos+=strlen(entry->d_name)+1;
		iResult = lseek(filelistfd, pos , SEEK_SET);
        if(iResult != (int)(pos))
   	    {
   	     NVBAK_LOG("FileOp_CreatMdFileList lseek error\r\n");
         close(filelistfd);
		 closedir(dir);
	     return false;
   	    }
        }
    if(-1 == chmod(filelistPathName, 0644))
    NVRAM_LOG("chmod file failed: %s\n", (char*)strerror(errno));
	close(filelistfd);
	closedir(dir);
	return true;
}

#define MAX_RETRY_COUNT 30
bool checkNvramReady(void) {
    int read_nvram_ready_retry = 0;
    int ret = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];
    while (read_nvram_ready_retry < MAX_RETRY_COUNT) {
        read_nvram_ready_retry++;
        property_get("vendor.service.nvram_init", nvram_init_val, NULL);
        if (strcmp(nvram_init_val, "Ready") == 0 ||
            strcmp(nvram_init_val, "Pre_Ready") == 0) {
            ret = true;
            break;
        } else {
            NVRAM_LOG("%s(), property_get(\"vendor.service.nvram_init\") = %s, read_nvram_ready_retry = %d",
                  __FUNCTION__, nvram_init_val, read_nvram_ready_retry);
            usleep(500 * 1000);
        }
    }
    if (read_nvram_ready_retry >= MAX_RETRY_COUNT) {
        NVRAM_LOG("Get nvram ready faild !!!\n");
        ret = false;
    }
    return ret;
}

bool FileOp_BackupToBinRegion_All( ) {
	bool bRet = true;
	struct stat st;
	char *tempBuffer = NULL;
	char *BitMapBuffer = NULL;
	struct mtd_info_user info;
	struct erase_info_user erase_info;
	unsigned int iMapFileSize, iDatFileSize, iWriteSize, iBlockSize, iPartitionSize;
	unsigned int iMemSize;
	unsigned int iBitmapFlag = BitmapFlag;
	int iFileDesc_file, iFileDesc_map, fd, iResult, i, j, m, iWriteTime, pos = 0,
	                                                                     iFreeBlockNum = 0;
	int pos_old = 0;
	int iReserveBlockCount = 0;
	int iReservePos = 0;
	//   int iBackup_Partition=get_partition_numb("nvram");

	// wait nvram ready to prevent nvram_daemon from doing restore operation at the same time
	checkNvramReady();

	char cMtdDevName[128] = {0};
	NVBAK_LOG("Enter BackupToBinRegion_all\n");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/CALIBRAT");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/BACKUP");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/INFO_FILE");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_CORE");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_DATA");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_IMEI");
	//fix nvram layout issue , reassgined block numerbers by different platform
	if (NVM_Init() < 0) {
		NVBAK_LOG("FileOp_BackupToBinRegion_All--NVM_Init fail !!!");
		return false;
	}
	/* GenerateFileVer again to avoid wrong FILE_VER backup to BIN region */
	if (false == NVM_GenerateFileVer(true)) {
		NVRAM_LOG("GenerateFileVer Fail!\n");
		return false;
	}

	memset(cMtdDevName, 0, sizeof cMtdDevName);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);


	//Add NVRAM history log
	bRet = NVRAM_HISTORY_LOG(1, "Enter BackupToBinRegion_all");
	if (bRet == false) {
		NVBAK_LOG("NVRAM_HISTORY_LOG write error!!!");
		return false;
	}

	if (nvram_misc_log_feature)
		NVRAM_MISC_LOG(1, "Enter BackupToBinRegion_all");


	bool bWorkForBinRegion = true;
	int iFileMask[ALL];
	//need init ifilemark value ,otherwize it cause backup fail on 6592+kk1.mp3+non lte project
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
	if (!(nvram_ecci_c2k_support())) {
		iFileMask[14] = 1;
		iFileMask[15] = 1;
		iFileMask[16] = 1;
	}

//   ret=FileOp_BackupData_All();
	bRet = FileOp_BackupDataToFiles(iFileMask, bWorkForBinRegion);
	if (bRet == false) {
		NVBAK_LOG("[NVRAM]:Crete the map file and the data file fail\n");
		return false;
	}
	if (stat(g_pcNVM_AllMap, &st) < 0) {
		NVBAK_LOG("[NVRAM]:Error MapFile stat \n");
		return false;
	}
	iMapFileSize = st.st_size;
	if (stat(g_pcNVM_AllFile, &st) < 0) {
		NVBAK_LOG("[NVRAM]:Error DatFile stat \n");
		return false;
	}
	iDatFileSize = st.st_size;
	NVBAK_LOG("[NVRAM]:info:iMapFileSize:%d,iDatFileSize:%d\n", iMapFileSize,
	          iDatFileSize);

	if (!FileOp_CreateBinRegionBadBlockBitMap()) {
		NVBAK_LOG("[NVRAM]:Error create Badblock Bitmap \n");
		return false;
	}

	if (nvram_check_exist_feature)
		FileOp_CheckFileExist();

	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_BackupToBinRegion_All get device info error\r\n");
		free(bBadBlockBitMap);
		return false;
	}


	//NVBAK_LOG("[NVRAM]:Backup_Partition:%d\n",iBackup_Partition);
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		free(bBadBlockBitMap);
		return false;
	}




	iWriteSize = info.writesize;
	iBlockSize = info.erasesize;
	iPartitionSize = info.size;
	NVBAK_LOG("[NVRAM]:mtd get info:iBlockSize:%d,iPartitionSize:%d\n", iBlockSize,
	          iPartitionSize);
	//already erase nand in CreateBitmap
	/*
	erase_info.start=0;
	erase_info.length=iPartitionSize;

	result=ioctl(fd, MEMERASE, &erase_info);
	if(result<0)
	{
	    NVBAK_LOG("[NVRAM]:mtd erase error\r\n");
	    close(fd);
	   return false;
	}
	 */

	iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
	iFileDesc_map = open(g_pcNVM_AllMap, O_RDWR);

	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		NVBAK_LOG("[NVRAM]: cannot open file data\n");
		if (iFileDesc_map != INVALID_HANDLE_VALUE)
			close(iFileDesc_map);
		close(fd);
		free(bBadBlockBitMap);
		return false;
	}

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		NVBAK_LOG("[NVRAM]: cannot open map data\n");
		close(iFileDesc_file);
		close(fd);
		free(bBadBlockBitMap);
		return false;
	}

	if ((iMapFileSize + 3 * sizeof(unsigned int)) % iWriteSize != 0)
		iMemSize = (((iMapFileSize + 3 * sizeof(unsigned int)) / iWriteSize) + 1) *
		           iWriteSize;//mapfile size, datfile size, cleanboot flag
	else
		iMemSize = iMapFileSize + 3 * sizeof(unsigned
		                                     int); //mapfile size, datfile size, cleanboot flag
	//iMemSize=iBlockSize;
	if (iMemSize > 2 * iBlockSize) {
		NVBAK_LOG("[NVRAM]:MapFile size is biger than a Block Size\r\n");
		close(fd);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	tempBuffer = (char*)malloc(iMemSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	memset(tempBuffer, 0xFF, iMemSize);
	memcpy(tempBuffer, &iMapFileSize, sizeof(unsigned int));
	memcpy(tempBuffer + sizeof(unsigned int), &iDatFileSize, sizeof(unsigned int));

	iResult = read(iFileDesc_map, tempBuffer + 3 * sizeof(unsigned int),
	               iMapFileSize);
	if (iResult != (int)iMapFileSize) {
		NVBAK_LOG("[NVRAM]:map file read error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	for (i = 0; i < NvRamBlockNum; i++) {
		if (bBadBlockBitMap[i] == 0) {
			pos = i;
			break;
		}
	}
	NVBAK_LOG("[NVRAM]:pos for mapfile:%d,i:%d\n", pos, i);
	if (i == NvRamBlockNum) {
		NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram map file\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	if(lseek(fd, pos * iBlockSize, SEEK_SET) < 0) {
		NVBAK_LOG("[NVRAM]:seek to %d error %s\n",pos * iBlockSize, (char*)strerror(errno));
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}

	if (iMemSize > iBlockSize) {
		iResult = write(fd, tempBuffer, iBlockSize);
		NVBAK_LOG("[NVRAM Backup]:map file write one blocksize:%d\n", iResult);
		if (iResult != (int)iBlockSize) {
			NVBAK_LOG("[NVRAM]:map file write error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}

		// check if or not have enough block for write map file
		for (m = pos + 1; m < NvRamBlockNum; m++) {
			if (bBadBlockBitMap[m] == 0) {
				pos = m;
				break;
			}
		}
		NVBAK_LOG("[NVRAM]:pos for mapfile:%d,i:%d\n", pos, m);
		if ( m >= NvRamBlockNum) {
			NVBAK_LOG("[NVRAM]:there are not enough good blocks for write nvram map file\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}

		if(lseek(fd, pos * iBlockSize, SEEK_SET) < 0) {// offset 128KB
			NVBAK_LOG("[NVRAM]:seek to %d error %s\n",pos * iBlockSize, (char*)strerror(errno));
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
		iResult = write(fd, tempBuffer + iBlockSize,
		                iMemSize - iBlockSize); // write the less
		NVBAK_LOG("[NVRAM Backup]:map file write another blocksize:%d\n", iResult);
		if (iResult != (int)(iMemSize - iBlockSize)) {
			NVBAK_LOG("[NVRAM]:map file write error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
		//check the iResult of write
		pos_old = pos - 1;
		if(lseek(fd, pos_old * iBlockSize, SEEK_SET) < 0) {
			NVBAK_LOG("[NVRAM]:seek to %d error %s\n",pos_old * iBlockSize, (char*)strerror(errno));
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
		
		i = 0;
		iResult = read(fd, &i, sizeof(unsigned int));
		if (iResult < 0 || (i != (int)iMapFileSize)) {
			NVBAK_LOG("[NVRAM]:check map file write error:%d,iMapfileSize:%d\n", i,
			          iMapFileSize);
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
	} else {
		iResult = write(fd, tempBuffer, iMemSize);
		NVBAK_LOG("[NVRAM Backup]:map file write :%d\n", iResult);
		if (iResult != (int)iMemSize) {
			NVBAK_LOG("[NVRAM]:map file write error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}

		//check the iResult of write
		if(lseek(fd, pos * iBlockSize, SEEK_SET) < 0) {
			NVBAK_LOG("[NVRAM]:seek to %d error %s\n",pos * iBlockSize, (char*)strerror(errno));
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
		i = 0;
		iResult = read(fd, &i, sizeof(unsigned int));
		if (iResult < 0 || (i != (int)iMapFileSize)) {
			NVBAK_LOG("[NVRAM]:check map file write error:%d,iMapfileSize:%d\n", i,
			          iMapFileSize);
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
	}


	for (j = (NvRamBlockNum - 1); j > pos; j--) {
		if (bBadBlockBitMap[j] == 0) {
			break;
		}
	}
	NVBAK_LOG("[NVRAM]:pos for bitmap:%d\n", j);
	if (j == pos) {
		NVBAK_LOG("[NVRAM]:there is no space for bitmap in nand\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	//write the map file and bitmap file into the last available block of nand
	if (iMemSize > iBlockSize) {
		iReserveBlockCount = 2;
		iReservePos = 1;
	} else {
		iReserveBlockCount = 1;
		iReservePos = 0;
	}

	BitMapBuffer = (char*)malloc(iBlockSize * iReserveBlockCount);
	if (BitMapBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:malloc memory BitMapBuffer error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	memset(BitMapBuffer, 0xFF, iBlockSize * iReserveBlockCount);
	memcpy(BitMapBuffer, bBadBlockBitMap, NvRamBlockNum * sizeof(char));
	memcpy(BitMapBuffer + NvRamBlockNum * sizeof(char), bBadBlockBitMap,
	       NvRamBlockNum * sizeof(char));
	memcpy(BitMapBuffer + 2 * NvRamBlockNum * sizeof(char), bBadBlockBitMap,
	       NvRamBlockNum * sizeof(char));
	memcpy(BitMapBuffer + 3 * NvRamBlockNum * sizeof(char), &iBitmapFlag,
	       sizeof(unsigned int));
    if((3 * NvRamBlockNum * sizeof(char) + sizeof(unsigned int)+ iMemSize)< iBlockSize * iReserveBlockCount) {
	memcpy(BitMapBuffer + 3 * NvRamBlockNum * sizeof(char) + sizeof(unsigned int),
	       tempBuffer, iMemSize);
		} else {
		NVBAK_LOG("[NVRAM]block too small,do not save Mapfile in last block \r\n");	
		}
	if(lseek(fd, (j - iReservePos)*iBlockSize, SEEK_SET) < 0) {
		NVBAK_LOG("[NVRAM]:seek to %d error %s\n",(j - iReservePos)*iBlockSize, (char*)strerror(errno));
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	iResult = write(fd, BitMapBuffer, iBlockSize * iReserveBlockCount);
	if (iResult != (int)iBlockSize * iReserveBlockCount) {
		NVBAK_LOG("[NVRAM]:bimap file write error\r\n");
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	//check the iResult of write the bitmap
	char *tempBitmap1 = NULL;
	char *tempBitmap2 = NULL;
	tempBitmap1 = (char *)malloc(NvRamBlockNum);
	if (tempBitmap1 == NULL) {
		NVBAK_LOG("[NVRAM]:malloc tempBitmap1 Fail!!\r\n");
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	tempBitmap2 = (char *)malloc(NvRamBlockNum);
	if (tempBitmap2 == NULL) {
		NVBAK_LOG("[NVRAM]:malloc tempBitmap2 Fail!!\r\n");
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		free(tempBitmap1);
		return false;
	}
	if(lseek(fd, (j - iReservePos) * iBlockSize, SEEK_SET) < 0) {
		NVBAK_LOG("[NVRAM]:seek to %d error %s\n",(j - iReservePos) * iBlockSize, (char*)strerror(errno));
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	iResult = read(fd, tempBitmap1, NvRamBlockNum * sizeof(char));
	if(iResult != NvRamBlockNum * sizeof(char)) {
		NVBAK_LOG("[NVRAM]:read bimap1 error\r\n");
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	iResult = read(fd, tempBitmap2, NvRamBlockNum * sizeof(char));
	if(iResult != NvRamBlockNum * sizeof(char)) {
		NVBAK_LOG("[NVRAM]:read bimap2 error\r\n");
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	for (i = 0; i < NvRamBlockNum; i++) {
		NVBAK_LOG("[NVRAM]:pos:%d,bitmap[i]:%d\n", j, tempBitmap1[i]);
	}
	for (i = 0; i < NvRamBlockNum; i++) {
		if (tempBitmap1[i] != tempBitmap2[i]) {
			NVBAK_LOG("[NVRAM]:check bimap file write error\r\n");
			close(fd);
			free(tempBuffer);
			free(BitMapBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}
	}
	free(BitMapBuffer);
	free(tempBuffer);
	free(tempBitmap1);
	free(tempBitmap2);
	tempBuffer = NULL;

	if (iDatFileSize % iBlockSize != 0)
		iMemSize = ((iDatFileSize / iBlockSize) + 1) * iBlockSize;
	else
		iMemSize = iDatFileSize;
	tempBuffer = (char*)malloc(iMemSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	memset(tempBuffer, 0xFF, iMemSize);
	iResult = read(iFileDesc_file, tempBuffer, iDatFileSize);
	if (iResult != (int)iDatFileSize) {
		NVBAK_LOG("[NVRAM]:dat file read error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	for (i = pos + 1; i < j; i++) {
		if (bBadBlockBitMap[i] == 0) {
			pos = i;
			break;
		}
	}
	if (i == j) {
		NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram data file\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	} else {
		for (i; i < j; i++) {
			if (bBadBlockBitMap[i] == 0)
				iFreeBlockNum++;
		}
		if ((iFreeBlockNum * iBlockSize) < (iMemSize)) {
			NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram data file\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
	}
	NVBAK_LOG("[NVRAM]:pos for data file:%d\n", pos);
	iWriteTime = iMemSize / iBlockSize;
	int iAlreadyWrite = 0;
	int bitmappos = j;
	NVBAK_LOG("[NVRAM]:iWriteTime:%d\n", iWriteTime);
	for (i = 0; i < iWriteTime; i++) {
		iResult = lseek(fd, pos * iBlockSize, SEEK_SET);
		if (iResult != (int)(pos * iBlockSize)) {
			NVBAK_LOG("[NVRAM]:dat file lseek error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
		iResult = write(fd, tempBuffer + i * iBlockSize, iBlockSize);
		if (iResult != (int)iBlockSize) {
			NVBAK_LOG("[NVRAM]:dat file write error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
		iAlreadyWrite++;
		if (iAlreadyWrite == iWriteTime)
			break;
		for (j = pos + 1; j < bitmappos; j++) {
			if (bBadBlockBitMap[j] == 0) {
				pos = j;
				break;
			}
		}
		if (j >= bitmappos) {
			NVBAK_LOG("[NVRAM]:there are not enough good blocks for write nvram data file\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
	}
	NVBAK_LOG("[NVRAM]:end:%d\n", pos);
	free(bBadBlockBitMap);
	close(fd);
	free(tempBuffer);
	close(iFileDesc_file);
	close(iFileDesc_map);
	NVBAK_LOG("setBackupFileNum\n");

	if (-1 == chown(g_pcNVM_AllFile , -1, AID_SYSTEM)) {
		NVBAK_LOG("chown for AllFile fail: %s", (char*)strerror(errno));
		//  return false;
	}

	if (-1 == chown(g_pcNVM_AllMap, -1, AID_SYSTEM)) {
		NVBAK_LOG("chown for AllMap fail: %s", (char*)strerror(errno));
		//   return false;
	}

#ifdef MTK_NVRAM_AUTO_BACKUP
	if (!Fileop_SetBackupFileNum(true)) {
		NVBAK_LOG("SetBackupFileNum Fail\n");
		return false;
	}
#endif


	NVBAK_LOG("SetBackupFileNum Success\n");
	NVBAK_LOG("Leave BackupToBinRegion_all\n");
	bRet = NVRAM_HISTORY_LOG(1, "Leave BackupToBinRegion_all\n");
	if (bRet == false) {
		NVBAK_LOG("NVRAM_HISTORY_LOG write error!!!");
		return false;
	}

	if (nvram_misc_log_feature)
		NVRAM_MISC_LOG(1, "Leave BackupToBinRegion_all");

	sync();


	if (nvram_read_back_feature) {
		NVBAK_LOG("[NVRAM]Enter Check Backup\n");
		if (!FileOp_RestoreFromBinRegion_ToFile()) {
			NVBAK_LOG("[NVRAM]: FileOp_RestoreFromBinRegion_ToFile fail\n");
			return false;
		}
		if (!FileOp_CheckBackUpResult()) {
			NVBAK_LOG("[NVRAM]: FileOp_CheckBackUpResult fail\n");
			return false;
		}
		NVBAK_LOG("[NVRAM]Leave Check Backup\n");
	}
	return true;
}
//add by min
bool FileOp_BackupToBinRegion_All_Exx(unsigned char *time_value)
{
	time_t before;
	time_t after;
	unsigned char *leave_time={'\0'};//can't get end time of FileOp_BackupToBinRegion_All_Ex,use NULL.
	bool bRet = true;
	struct stat st;
	char *tempBuffer=NULL;
   char *BitMapBuffer=NULL;
   struct mtd_info_user info;
   struct erase_info_user erase_info;
   unsigned int iMapFileSize,iDatFileSize,iWriteSize,iBlockSize,iPartitionSize;
   unsigned int iMemSize;
   unsigned int iBitmapFlag=BitmapFlag;
	int iFileDesc_file, iFileDesc_map, fd, iResult,i,j,m,iWriteTime,pos=0,iFreeBlockNum=0;
	int pos_old = 0;
	int iReserveBlockCount = 0;
	int iReservePos = 0;
	//   int iBackup_Partition=get_partition_numb("nvram");

	char cMtdDevName[128] = {0};
   NVBAK_LOG("Enter BackupToBinRegion_all\n");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/CALIBRAT");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/BACKUP");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/INFO_FILE");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_CORE");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_DATA");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_IMEI");

    //fix nvram layout issue , reassgined block numerbers by different platform
    if (NVM_Init() < 0)
    {
        NVBAK_LOG("FileOp_BackupToBinRegion_All--NVM_Init fail !!!");
        return false;
    }
    /* GenerateFileVer again to avoid wrong FILE_VER backup to BIN region */
    if (false == NVM_GenerateFileVer(true)) {
        NVRAM_LOG("GenerateFileVer Fail!\n");
        return false;
    }

	memset(cMtdDevName, 0, sizeof cMtdDevName);
   	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);


   //Add NVRAM history log
   before=time(&before); //add for time
   NVRAM_LOG("FileOp_BackupToBinRegion_All_Ex: before is  %s\n",ctime(&before));
   NVRAM_LOG("size is  %d\n",sizeof(time_t));
   if(nvram_md1_support())
   bRet = NVRAM_HISTORY_LOG_TIME(1, time_value, "Enter FileOp_BackupToBinRegion_All_Ex");
   else
   bRet = NVRAM_HISTORY_LOG(1, "Enter FileOp_BackupToBinRegion_All_Ex");
   if (bRet == false)
   {
       NVBAK_LOG("NVRAM_HISTORY_LOG write error!!!");
       return false;
   }

   if (nvram_misc_log_feature)
   		NVRAM_MISC_LOG(1, "Enter BackupToBinRegion_all");


   bool bWorkForBinRegion=true;
   int iFileMask[ALL];
   //need init ifilemark value ,otherwize it cause backup fail on 6592+kk1.mp3+non lte project
   memset(iFileMask,0,(ALL*(sizeof(int))));
   if(!nvram_md5_support() && !nvram_evdo_support())
   {
		iFileMask[5]=1;iFileMask[6]=1;iFileMask[10]=1;iFileMask[11]=1;iFileMask[12]=1;iFileMask[13]=1;
   }
   else
   	   if(!nvram_md5_support() && nvram_evdo_support())
   	   	{
   	   		iFileMask[5]=1;iFileMask[6]=1;iFileMask[10]=1;iFileMask[11]=1;iFileMask[12]=1;
   	   	}
   else
   	   if(nvram_md5_support() && !nvram_evdo_support())
   	   	{
   	   		iFileMask[5]=1;iFileMask[6]=1;iFileMask[13]=1;
   	   	}
   else
   		{
   			iFileMask[5]=1;iFileMask[6]=1;
   		}
   if(!(nvram_ecci_c2k_support()))
   {
		iFileMask[14]=1;iFileMask[15]=1;iFileMask[16]=1;
   }

//   ret=FileOp_BackupData_All();
   bRet = FileOp_BackupDataToFiles(iFileMask,bWorkForBinRegion);
   if(bRet == false)
   	{
   	   NVBAK_LOG("[NVRAM]:Crete the map file and the data file fail\n");
	   return false;
   	}
   if(stat(g_pcNVM_AllMap,&st)<0)
    {
       NVBAK_LOG("[NVRAM]:Error MapFile stat \n");
       return false;
    }
   iMapFileSize=st.st_size;
   if(stat(g_pcNVM_AllFile,&st)<0)
    {
       NVBAK_LOG("[NVRAM]:Error DatFile stat \n");
       return false;
    }
   iDatFileSize=st.st_size;
   NVBAK_LOG("[NVRAM]:info:iMapFileSize:%d,iDatFileSize:%d\n",iMapFileSize,iDatFileSize);

   if(!FileOp_CreateBinRegionBadBlockBitMap())
   	{
   	   NVBAK_LOG("[NVRAM]:Error create Badblock Bitmap \n");
       return false;
   	}

   	if (nvram_check_exist_feature)
   			FileOp_CheckFileExist();

	if(!NVM_GetDeviceInfo(cMtdDevName,&info))
	{
       NVBAK_LOG("[NVRAM]:FileOp_BackupToBinRegion_All get device info error\r\n");
	   free(bBadBlockBitMap);
       return false;
	}


   //NVBAK_LOG("[NVRAM]:Backup_Partition:%d\n",iBackup_Partition);
   fd=open(cMtdDevName,O_RDWR);
   if(fd<0)
	{
	   NVBAK_LOG("[NVRAM]:mtd open error\r\n");
	   free(bBadBlockBitMap);
	   return false;
	}




   iWriteSize=info.writesize;
   iBlockSize=info.erasesize;
   iPartitionSize=info.size;
   NVBAK_LOG("[NVRAM]:mtd get info:iBlockSize:%d,iPartitionSize:%d\n",iBlockSize,iPartitionSize);
   //already erase nand in CreateBitmap
   /*
   erase_info.start=0;
   erase_info.length=iPartitionSize;

   result=ioctl(fd, MEMERASE, &erase_info);
   if(result<0)
   {
       NVBAK_LOG("[NVRAM]:mtd erase error\r\n");
       close(fd);
	   return false;
   }
    */

    iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
    iFileDesc_map = open(g_pcNVM_AllMap, O_RDWR);

    if(INVALID_HANDLE_VALUE == iFileDesc_file)
    {
        NVBAK_LOG("[NVRAM]: cannot open file data\n");
			if(iFileDesc_map != INVALID_HANDLE_VALUE)
				close(iFileDesc_map);
		close(fd);
		free(bBadBlockBitMap);
        return false;
    }

	if(INVALID_HANDLE_VALUE == iFileDesc_map)
    {
        NVBAK_LOG("[NVRAM]: cannot open map data\n");
		close(iFileDesc_file);
        close(fd);
        free(bBadBlockBitMap);
        return false;
    }

	if((iMapFileSize+3*sizeof(unsigned int)) % iWriteSize != 0)
	iMemSize=(((iMapFileSize+3*sizeof(unsigned int))/iWriteSize)+1)*iWriteSize;//mapfile size, datfile size, cleanboot flag
	else
		iMemSize=iMapFileSize+3*sizeof(unsigned int);//mapfile size, datfile size, cleanboot flag
	//iMemSize=iBlockSize;
	if(iMemSize > 2 * iBlockSize)
		{
			NVBAK_LOG("[NVRAM]:MapFile size is biger than a Block Size\r\n");
			close(fd);
	  		close(iFileDesc_file);
	  		close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
    tempBuffer=(char*)malloc(iMemSize);
    if(tempBuffer==NULL)
   	{
   	  NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
      close(fd);
	  close(iFileDesc_file);
	  close(iFileDesc_map);
	  free(bBadBlockBitMap);
	  return false;
   	}
    memset(tempBuffer,0xFF,iMemSize);
    memcpy(tempBuffer,&iMapFileSize,sizeof(unsigned int));
    memcpy(tempBuffer+sizeof(unsigned int),&iDatFileSize,sizeof(unsigned int));

    iResult = read(iFileDesc_map,tempBuffer+3*sizeof(unsigned int),iMapFileSize);
    if(iResult != (int)iMapFileSize)
   	{
   	  NVBAK_LOG("[NVRAM]:map file read error\r\n");
      close(fd);
	  free(tempBuffer);
	  close(iFileDesc_file);
	  close(iFileDesc_map);
	  free(bBadBlockBitMap);
	  return false;
   	}
    for(i=0;i<NvRamBlockNum;i++)
   	{
   	  if(bBadBlockBitMap[i]==0)
	  	{
	  	  pos=i;
		  break;
   	  	}
   	}
	NVBAK_LOG("[NVRAM]:pos for mapfile:%d,i:%d\n",pos,i);
    if(i==NvRamBlockNum)
    {
      NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram map file\r\n");
      close(fd);
	  free(tempBuffer);
	  close(iFileDesc_file);
	  close(iFileDesc_map);
	  free(bBadBlockBitMap);
	  return false;
    }
     lseek(fd,pos*iBlockSize,SEEK_SET);
	
     if (iMemSize > iBlockSize) {
  		iResult = write(fd, tempBuffer, iBlockSize);
  		NVBAK_LOG("[NVRAM Backup]:map file write one blocksize:%d\n",iResult);
		if (iResult != (int)iBlockSize) {
      			NVBAK_LOG("[NVRAM]:map file write error\r\n");
      			close(fd);
      			free(tempBuffer);
      			close(iFileDesc_file);
      			close(iFileDesc_map);
      			free(bBadBlockBitMap);
      			return false;
    		}

		// check if or not have enough block for write map file 
		for (m = pos + 1; m < NvRamBlockNum; m++) {
			if (bBadBlockBitMap[m] == 0) {
		    	pos = m;
		    	break;
		  	}
	    	}
		NVBAK_LOG("[NVRAM]:pos for mapfile:%d,i:%d\n",pos,m); 
	    	if ( m >= NvRamBlockNum) {
	      		NVBAK_LOG("[NVRAM]:there are not enough good blocks for write nvram map file\r\n");
	      		close(fd);
	      		free(tempBuffer);
	      		close(iFileDesc_file);
	      		close(iFileDesc_map);
	      		free(bBadBlockBitMap);
	      		return false;
	    	}
		
		lseek(fd, pos * iBlockSize, SEEK_SET); // offset 128KB

		iResult = write(fd, tempBuffer + iBlockSize, iMemSize - iBlockSize); // write the less
		NVBAK_LOG("[NVRAM Backup]:map file write another blocksize:%d\n",iResult);
		if (iResult != (int)(iMemSize - iBlockSize)) {
      			NVBAK_LOG("[NVRAM]:map file write error\r\n");
      			close(fd);
      			free(tempBuffer);
      			close(iFileDesc_file);
      			close(iFileDesc_map);
      			free(bBadBlockBitMap);
      			return false;
    		}
		//check the iResult of write
		pos_old = pos -1;
		lseek(fd, pos_old * iBlockSize, SEEK_SET);
  		i = 0;
  		iResult = read(fd, &i, sizeof(unsigned int));
  		if(iResult < 0||(i != (int)iMapFileSize)) {
      			NVBAK_LOG("[NVRAM]:check map file write error:%d,iMapfileSize:%d\n",i,iMapFileSize);
      			close(fd);
      			free(tempBuffer);
      			close(iFileDesc_file);
      			close(iFileDesc_map);
      			free(bBadBlockBitMap);
      			return false;
    		}
 	} else {
     iResult = write(fd,tempBuffer,iMemSize);
     NVBAK_LOG("[NVRAM Backup]:map file write :%d\n",iResult);
  		if(iResult != (int)iMemSize) {
          NVBAK_LOG("[NVRAM]:map file write error\r\n");
          close(fd);
		  free(tempBuffer);
	      close(iFileDesc_file);
	      close(iFileDesc_map);
		  free(bBadBlockBitMap);
	      return false;
     }

	 //check the iResult of write
	 lseek(fd,pos*iBlockSize,SEEK_SET);
	 i=0;
	 iResult = read(fd,&i,sizeof(unsigned int));
  		if(iResult < 0 || (i != (int)iMapFileSize)) {
	      NVBAK_LOG("[NVRAM]:check map file write error:%d,iMapfileSize:%d\n",i,iMapFileSize);
          close(fd);
		  free(tempBuffer);
	      close(iFileDesc_file);
	      close(iFileDesc_map);
		  free(bBadBlockBitMap);
	      return false;
	 }
  	}


    for(j=(NvRamBlockNum-1);j>pos;j--)
     {
    	 if(bBadBlockBitMap[j]==0)
    	 {
			 break;
    	 }
     }
	NVBAK_LOG("[NVRAM]:pos for bitmap:%d\n",j);
	if(j==pos)
	 {
          NVBAK_LOG("[NVRAM]:there is no space for bitmap in nand\r\n");
          close(fd);
		  free(tempBuffer);
	      close(iFileDesc_file);
	      close(iFileDesc_map);
		  free(bBadBlockBitMap);
	      return false;
     }
	//write the map file and bitmap file into the last available block of nand
	if (iMemSize > iBlockSize) {
		iReserveBlockCount = 2;
		iReservePos = 1;
	} else {
		iReserveBlockCount = 1;
		iReservePos = 0;
	}

	BitMapBuffer=(char*)malloc(iBlockSize * iReserveBlockCount);
	if(BitMapBuffer == NULL)
		{
          NVBAK_LOG("[NVRAM]:malloc memory BitMapBuffer error\r\n");
          close(fd);
		  free(tempBuffer);
	      close(iFileDesc_file);
	      close(iFileDesc_map);
		  free(bBadBlockBitMap);
	      return false;
     }
	memset(BitMapBuffer,0xFF,iBlockSize * iReserveBlockCount);
	memcpy(BitMapBuffer,bBadBlockBitMap,NvRamBlockNum*sizeof(char));
	memcpy(BitMapBuffer+NvRamBlockNum*sizeof(char),bBadBlockBitMap,NvRamBlockNum*sizeof(char));
	memcpy(BitMapBuffer+2*NvRamBlockNum*sizeof(char),bBadBlockBitMap,NvRamBlockNum*sizeof(char));
	memcpy(BitMapBuffer+3*NvRamBlockNum*sizeof(char),&iBitmapFlag,sizeof(unsigned int));
    if((3 * NvRamBlockNum * sizeof(char) + sizeof(unsigned int)+ iMemSize)< iBlockSize * iReserveBlockCount) {	
	memcpy(BitMapBuffer+3*NvRamBlockNum*sizeof(char)+sizeof(unsigned int),tempBuffer,iMemSize);
    } else {
        NVBAK_LOG("[NVRAM]block too small,do not save Mapfile in last block \r\n");
    }
	lseek(fd,(j-iReservePos)*iBlockSize,SEEK_SET);
    iResult = write(fd,BitMapBuffer,iBlockSize * iReserveBlockCount);
	if (iResult != (int)iBlockSize * iReserveBlockCount)
	 {
          NVBAK_LOG("[NVRAM]:bimap file write error\r\n");
          close(fd);
		  free(tempBuffer);
		  free(BitMapBuffer);
	      close(iFileDesc_file);
	      close(iFileDesc_map);
		  free(bBadBlockBitMap);
	      return false;
     }
	//check the iResult of write the bitmap
	char *tempBitmap1 = NULL;
    char *tempBitmap2 = NULL;
	tempBitmap1=(char *)malloc(NvRamBlockNum);
   if(tempBitmap1 == NULL){
   	  NVBAK_LOG("[NVRAM]:malloc tempBitmap1 Fail!!\r\n");
	  close(fd);
	  free(tempBuffer);
	  free(BitMapBuffer);
	  close(iFileDesc_file);
	  close(iFileDesc_map);
	  free(bBadBlockBitMap);
	  return false;
   	}
   	tempBitmap2=(char *)malloc(NvRamBlockNum);
	if(tempBitmap2 == NULL){
   	  NVBAK_LOG("[NVRAM]:malloc tempBitmap2 Fail!!\r\n");
	  close(fd);
	  free(tempBuffer);
	  free(BitMapBuffer);
	  close(iFileDesc_file);
	  close(iFileDesc_map);
	  free(bBadBlockBitMap);
	  free(tempBitmap1);
	  return false;
   	}
	lseek(fd, (j - iReservePos) * iBlockSize, SEEK_SET);
    iResult = read(fd,tempBitmap1,NvRamBlockNum*sizeof(char));
	iResult = read(fd,tempBitmap2,NvRamBlockNum*sizeof(char));
	for(i=0;i<NvRamBlockNum;i++)
	{
	  NVBAK_LOG("[NVRAM]:pos:%d,bitmap[i]:%d\n",j,tempBitmap1[i]);
	}
	for(i=0;i<NvRamBlockNum;i++)
	{
	  if(tempBitmap1[i]!=tempBitmap2[i])
	  	{
	  	  NVBAK_LOG("[NVRAM]:check bimap file write error\r\n");
          close(fd);
		  free(tempBuffer);
		  free(BitMapBuffer);
	      close(iFileDesc_file);
	      close(iFileDesc_map);
		  free(bBadBlockBitMap);
		  free(tempBitmap1);
		  free(tempBitmap2);
	      return false;
	  	}
	}
	free(BitMapBuffer);
    free(tempBuffer);
	free(tempBitmap1);
    free(tempBitmap2);
    tempBuffer=NULL;

	if(iDatFileSize%iBlockSize != 0)
    iMemSize=((iDatFileSize/iBlockSize)+1)*iBlockSize;
	else
		iMemSize=iDatFileSize;
    tempBuffer=(char*)malloc(iMemSize);
    if(tempBuffer==NULL)
   	{
   	  NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
      close(fd);
	  close(iFileDesc_file);
	  close(iFileDesc_map);
	  free(bBadBlockBitMap);
	  return false;
   	}
   memset(tempBuffer,0xFF,iMemSize);
   iResult = read(iFileDesc_file,tempBuffer,iDatFileSize);
   if(iResult != (int)iDatFileSize)
   	{
   	  NVBAK_LOG("[NVRAM]:dat file read error\r\n");
      close(fd);
	  free(tempBuffer);
	  close(iFileDesc_file);
	  close(iFileDesc_map);
	  free(bBadBlockBitMap);
	  return false;
   	}
   for(i=pos+1;i<j;i++)
   	{
   	  if(bBadBlockBitMap[i]==0)
	  	{
	  	 pos=i;
		 break;
   	  	}
   	}
   if(i==j)
   	{
   	  NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram data file\r\n");
      close(fd);
	  free(tempBuffer);
	  close(iFileDesc_file);
	  close(iFileDesc_map);
	  free(bBadBlockBitMap);
	  return false;
   	}
   else
   	{
   	  for(i;i<j;i++)
   	  	{
   	  	  if(bBadBlockBitMap[i]==0)
		  	iFreeBlockNum++;
   	  	}
	  if((iFreeBlockNum*iBlockSize)<(iMemSize))
	  	{
	  	  NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram data file\r\n");
          close(fd);
	      free(tempBuffer);
	      close(iFileDesc_file);
	      close(iFileDesc_map);
		  free(bBadBlockBitMap);
	      return false;
	  	}
   	}
   NVBAK_LOG("[NVRAM]:pos for data file:%d\n",pos);
   iWriteTime=iMemSize/iBlockSize;
   int iAlreadyWrite=0;
   int bitmappos=j;
   NVBAK_LOG("[NVRAM]:iWriteTime:%d\n",iWriteTime);
   for(i=0;i<iWriteTime;i++)
   	{
      iResult = lseek(fd,pos*iBlockSize,SEEK_SET);
      if(iResult != (int)(pos*iBlockSize))
   	    {
   	     NVBAK_LOG("[NVRAM]:dat file lseek error\r\n");
         close(fd);
	     free(tempBuffer);
	     close(iFileDesc_file);
	     close(iFileDesc_map);
		 free(bBadBlockBitMap);
	     return false;
   	    }
       iResult = write(fd,tempBuffer+i*iBlockSize,iBlockSize);
       if(iResult != (int)iBlockSize)
	    {
            NVBAK_LOG("[NVRAM]:dat file write error\r\n");
            close(fd);
		    free(tempBuffer);
	        close(iFileDesc_file);
	        close(iFileDesc_map);
			free(bBadBlockBitMap);
	        return false;
        }
	   iAlreadyWrite++;
	   if(iAlreadyWrite==iWriteTime)
	   	  break;
	   for(j=pos+1;j<bitmappos;j++)
	   	{
	   	   if(bBadBlockBitMap[j]==0)
		   	{
		   	  pos=j;
			  break;
	   	   	}
	   	}
	   if(j>=bitmappos){
	   		NVBAK_LOG("[NVRAM]:there are not enough good blocks for write nvram data file\r\n");
			close(fd);
      		free(tempBuffer);
      		close(iFileDesc_file);
      		close(iFileDesc_map);
			free(bBadBlockBitMap);
      		return false;
	   	}
   	}
   NVBAK_LOG("[NVRAM]:end:%d\n",pos);
   free(bBadBlockBitMap);
   close(fd);
   free(tempBuffer);
   close(iFileDesc_file);
   close(iFileDesc_map);
   NVBAK_LOG("setBackupFileNum\n");

   if(-1 == chown(g_pcNVM_AllFile , -1, AID_SYSTEM)) {
       NVBAK_LOG("chown for AllFile fail: %s", (char*)strerror(errno));
     //  return false;
   }

   if(-1 == chown(g_pcNVM_AllMap, -1, AID_SYSTEM)) {
       NVBAK_LOG("chown for AllMap fail: %s", (char*)strerror(errno));
    //   return false;
   }

#ifdef MTK_NVRAM_AUTO_BACKUP
   if(!Fileop_SetBackupFileNum(true))
   	{
   		NVBAK_LOG("SetBackupFileNum Fail\n");
   		return false;
   	}
#endif


	NVBAK_LOG("SetBackupFileNum Success\n");
	NVBAK_LOG("Leave BackupToBinRegion_all\n");
	after = time(&after); //add for time
	NVRAM_LOG("FileOp_BackupToBinRegion_All_Ex: after is  %s\n",ctime(&after));
	if(nvram_md1_support())
	bRet = NVRAM_HISTORY_LOG_TIME(1, leave_time, "Leave FileOp_BackupToBinRegion_All_Ex");
    else
	bRet = NVRAM_HISTORY_LOG(1, "Leave BackupToBinRegion_all\n");
	if (bRet == false)
	{
		NVBAK_LOG("NVRAM_HISTORY_LOG write error!!!");
		return false;
	}

	if (nvram_misc_log_feature)
	 		NVRAM_MISC_LOG(1, "Leave BackupToBinRegion_all");

  	sync();


  if (nvram_read_back_feature)
  {
	   NVBAK_LOG("[NVRAM]Enter Check Backup\n");
	   if(!FileOp_RestoreFromBinRegion_ToFile())
	   {
	        NVBAK_LOG("[NVRAM]: FileOp_RestoreFromBinRegion_ToFile fail\n");
	        return false;
	   }
	   if(!FileOp_CheckBackUpResult())
	   {
	        NVBAK_LOG("[NVRAM]: FileOp_CheckBackUpResult fail\n");
	        return false;
	   }
	   NVBAK_LOG("[NVRAM]Leave Check Backup\n");
  }
   return true;
}
bool FileOp_BackupToBinRegion_All_Ex(int value) {
	bool bRet = true;
	struct stat st;
	char cMtdDevName[128] = {0};
	char cCallerName[20] = {0};
	char cLog[MAX_LOG_LENGTH] = {0};
	char *tempBuffer = NULL;
	char *BitMapBuffer = NULL;
	bool bMarkCaller = false;

	struct mtd_info_user info;
	struct erase_info_user erase_info;
	unsigned int iMapFileSize, iDatFileSize, iWriteSize, iBlockSize, iPartitionSize;
	unsigned int iMemSize;
	unsigned int iBitmapFlag = BitmapFlag;
	int iFileDesc_file, iFileDesc_map, fd, iResult, i, j, iWriteTime, pos = 0,
	                                                                  iFreeBlockNum = 0;


	memset(cMtdDevName, 0, sizeof cMtdDevName);
	for (i = 0; i < g_RecordCaller_Count; i++) {

		if (value == aRecordCallerList[i].iRecordCallerIndex) {
			memcpy(cCallerName, aRecordCallerList[i].cRecordCallerName, 10);
			bMarkCaller = true;
			break;
		}
	}


//   int iBackup_Partition=get_partition_numb("nvram");



	NVBAK_LOG("Enter BackupToBinRegion_all_ex\n");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/CALIBRAT");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/BACKUP");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/INFO_FILE");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_CORE");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_DATA");
   FileOp_CreatMdFileList("/mnt/vendor/nvdata/md/NVRAM/NVD_IMEI");
	//fix nvram layout issue , reassgined block numerbers by different platform
	if (NVM_Init() < 0) {
		NVBAK_LOG("BackupToBinRegion_all_ex--NVM_Init fail !!!\n");
		return false;
	}

	//Add NVRAM history log
	snprintf(cLog, sizeof(cLog),"%s", "Enter BackupToBinRegion_all");
	if (bMarkCaller) {
		//strcat(cLog, "_");
		//strcat(cLog, cCallerName);
        strncat(cLog,"_",strlen("_"));
		strncat(cLog,cCallerName,strlen(cCallerName));
	}
	NVBAK_LOG("cLog ==%s,bMarkCaller==%d\n", cLog, bMarkCaller);
	bRet = NVRAM_HISTORY_LOG(1, cLog);
	if (bRet == false) {
		NVBAK_LOG("NVRAM_HISTORY_LOG write error!!!");
		return false;
	}

	if (nvram_misc_log_feature)
		NVRAM_MISC_LOG(1, cLog);

	bool bWorkForBinRegion = true;
	int iFileMask[ALL];
	//need init ifilemark value ,otherwize it cause backup fail on 6592+kk1.mp3+non lte project
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

//   ret=FileOp_BackupData_All();
	bRet = FileOp_BackupDataToFiles(iFileMask, bWorkForBinRegion);
	if (bRet == false) {
		NVBAK_LOG("[NVRAM]:Crete the map file and the data file fail\n");
		return false;
	}
	if (stat(g_pcNVM_AllMap, &st) < 0) {
		NVBAK_LOG("[NVRAM]:Error MapFile stat \n");
		return false;
	}
	iMapFileSize = st.st_size;
	if (stat(g_pcNVM_AllFile, &st) < 0) {
		NVBAK_LOG("[NVRAM]:Error DatFile stat \n");
		return false;
	}
	iDatFileSize = st.st_size;
	NVBAK_LOG("[NVRAM]:info:iMapFileSize:%d,iDatFileSize:%d\n", iMapFileSize,
	          iDatFileSize);

	if (!FileOp_CreateBinRegionBadBlockBitMap()) {
		NVBAK_LOG("[NVRAM]:Error create Badblock Bitmap \n");
		return false;
	}
	FileOp_CheckFileExist();

	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);

	//NVBAK_LOG("[NVRAM]:Backup_Partition:%d\n",iBackup_Partition);
	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_BackupToBinRegion_All_Ex get device info error\r\n");
		free(bBadBlockBitMap);
		return false;
	}
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		free(bBadBlockBitMap);
		return false;
	}



	iWriteSize = info.writesize;
	iBlockSize = info.erasesize;
	iPartitionSize = info.size;
	NVBAK_LOG("[NVRAM]:mtd get info:iBlockSize:%d,iPartitionSize:%d\n", iBlockSize,
	          iPartitionSize);
	//already erase nand in CreateBitmap
	/*
	erase_info.start=0;
	erase_info.length=iPartitionSize;

	result=ioctl(fd, MEMERASE, &erase_info);
	if(result<0)
	{
	    NVBAK_LOG("[NVRAM]:mtd erase error\r\n");
	    close(fd);
	   return false;
	}
	 */

	iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
	iFileDesc_map = open(g_pcNVM_AllMap, O_RDWR);

	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		NVBAK_LOG("[NVRAM]: cannot open file data\n");
		if (iFileDesc_map != INVALID_HANDLE_VALUE)
			close(iFileDesc_map);
		close(fd);
		free(bBadBlockBitMap);
		return false;
	}

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		NVBAK_LOG("[NVRAM]: cannot open map data\n");
		close(iFileDesc_file);
		close(fd);
		free(bBadBlockBitMap);
		return false;
	}

	if ((iMapFileSize + 3 * sizeof(unsigned int)) % iWriteSize != 0)
		iMemSize = (((iMapFileSize + 3 * sizeof(unsigned int)) / iWriteSize) + 1) *
		           iWriteSize;//mapfile size, datfile size, cleanboot flag
	else
		iMemSize = iMapFileSize + 3 * sizeof(unsigned
		                                     int); //mapfile size, datfile size, cleanboot flag
	//iMemSize=iBlockSize;
	if (iMemSize > iBlockSize) {
		NVBAK_LOG("[NVRAM]:MapFile size is biger than a Block Size\r\n");
		close(fd);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	tempBuffer = (char*)malloc(iMemSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	memset(tempBuffer, 0xFF, iMemSize);
	memcpy(tempBuffer, &iMapFileSize, sizeof(unsigned int));
	memcpy(tempBuffer + sizeof(unsigned int), &iDatFileSize, sizeof(unsigned int));

	iResult = read(iFileDesc_map, tempBuffer + 3 * sizeof(unsigned int),
	               iMapFileSize);
	if (iResult != (int)iMapFileSize) {
		NVBAK_LOG("[NVRAM]:map file read error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	for (i = 0; i < NvRamBlockNum; i++) {
		if (bBadBlockBitMap[i] == 0) {
			pos = i;
			break;
		}
	}
	NVBAK_LOG("[NVRAM]:pos for mapfile:%d,i:%d\n", pos, i);
	if (i == NvRamBlockNum) {
		NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram map file\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	lseek(fd, pos * iBlockSize, SEEK_SET);
	iResult = write(fd, tempBuffer, iMemSize);
	NVBAK_LOG("[NVRAM Backup]:map file write :%d\n", iResult);
	if (iResult != (int)iMemSize) {
		NVBAK_LOG("[NVRAM]:map file write error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	//check the result of write
	lseek(fd, pos * iBlockSize, SEEK_SET);
	i = 0;
	iResult = read(fd, &i, sizeof(unsigned int));
	if (iResult < 0 || (i != (int)iMapFileSize)) {
		NVBAK_LOG("[NVRAM]:check map file write error:%d,iMapfileSize:%d\n", i,
		          iMapFileSize);
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}

	for (j = (NvRamBlockNum - 1); j > pos; j--) {
		if (bBadBlockBitMap[j] == 0) {
			break;
		}
	}
	NVBAK_LOG("[NVRAM]:pos for bitmap:%d\n", j);
	if (j == pos) {
		NVBAK_LOG("[NVRAM]:there is no space for bitmap in nand\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	//write the map file and bitmap file into the last available block of nand
	BitMapBuffer = (char*)malloc(iBlockSize);
	if (BitMapBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:malloc memory BitMapBuffer error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	memset(BitMapBuffer, 0xFF, iBlockSize);
	memcpy(BitMapBuffer, bBadBlockBitMap, NvRamBlockNum * sizeof(char));
	memcpy(BitMapBuffer + NvRamBlockNum * sizeof(char), bBadBlockBitMap,
	       NvRamBlockNum * sizeof(char));
	memcpy(BitMapBuffer + 2 * NvRamBlockNum * sizeof(char), bBadBlockBitMap,
	       NvRamBlockNum * sizeof(char));
	memcpy(BitMapBuffer + 3 * NvRamBlockNum * sizeof(char), &iBitmapFlag,
	       sizeof(unsigned int));
    if((3 * NvRamBlockNum * sizeof(char) + sizeof(unsigned int)+ iMemSize)< iBlockSize) {	
	memcpy(BitMapBuffer + 3 * NvRamBlockNum * sizeof(char) + sizeof(unsigned int),
	       tempBuffer, iMemSize);
    } else {
        NVBAK_LOG("[NVRAM]block too small,do not save Mapfile in last block \r\n");
    }
	lseek(fd, j * iBlockSize, SEEK_SET);
	iResult = write(fd, BitMapBuffer, iBlockSize);
	if (iResult != (int)iBlockSize) {
		NVBAK_LOG("[NVRAM]:bimap file write error\r\n");
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	//check the result of write the bitmap
	char *tempBitmap1 = NULL;
	char *tempBitmap2 = NULL;
	tempBitmap1 = (char *)malloc(NvRamBlockNum);
	if (tempBitmap1 == NULL) {
		NVBAK_LOG("[NVRAM]:malloc tempBitmap1 Fail!!\r\n");
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	tempBitmap2 = (char *)malloc(NvRamBlockNum);
	if (tempBitmap2 == NULL) {
		NVBAK_LOG("[NVRAM]:malloc tempBitmap2 Fail!!\r\n");
		close(fd);
		free(tempBuffer);
		free(BitMapBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		free(tempBitmap1);
		return false;
	}
	lseek(fd, j * iBlockSize, SEEK_SET);
	iResult = read(fd, tempBitmap1, NvRamBlockNum * sizeof(char));
	iResult = read(fd, tempBitmap2, NvRamBlockNum * sizeof(char));
	for (i = 0; i < NvRamBlockNum; i++) {
		NVBAK_LOG("[NVRAM]:pos:%d,bitmap[i]:%d\n", j, tempBitmap1[i]);
	}
	for (i = 0; i < NvRamBlockNum; i++) {
		if (tempBitmap1[i] != tempBitmap2[i]) {
			NVBAK_LOG("[NVRAM]:check bimap file write error\r\n");
			close(fd);
			free(tempBuffer);
			free(BitMapBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			free(tempBitmap1);
			free(tempBitmap2);
			return false;
		}
	}
	free(BitMapBuffer);
	free(tempBuffer);
	free(tempBitmap1);
	free(tempBitmap2);
	tempBuffer = NULL;

	if (iDatFileSize % iBlockSize != 0)
		iMemSize = ((iDatFileSize / iBlockSize) + 1) * iBlockSize;
	else
		iMemSize = iDatFileSize;
	tempBuffer = (char*)malloc(iMemSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	memset(tempBuffer, 0xFF, iMemSize);
	iResult = read(iFileDesc_file, tempBuffer, iDatFileSize);
	if (iResult != (int)iDatFileSize) {
		NVBAK_LOG("[NVRAM]:dat file read error\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	}
	for (i = pos + 1; i < j; i++) {
		if (bBadBlockBitMap[i] == 0) {
			pos = i;
			break;
		}
	}
	if (i == j) {
		NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram data file\r\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		free(bBadBlockBitMap);
		return false;
	} else {
		for (i; i < j; i++) {
			if (bBadBlockBitMap[i] == 0)
				iFreeBlockNum++;
		}
		if ((iFreeBlockNum * iBlockSize) < (iMemSize)) {
			NVBAK_LOG("[NVRAM]:there are not enough good blocks for backup nvram data file\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
	}
	NVBAK_LOG("[NVRAM]:pos for data file:%d\n", pos);
	iWriteTime = iMemSize / iBlockSize;
	int iAlreadyWrite = 0;
	int bitmappos = j;
	NVBAK_LOG("[NVRAM]:iWriteTime:%d\n", iWriteTime);
	for (i = 0; i < iWriteTime; i++) {
		iResult = lseek(fd, pos * iBlockSize, SEEK_SET);
		if (iResult != (int)(pos * iBlockSize)) {
			NVBAK_LOG("[NVRAM]:dat file lseek error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
		iResult = write(fd, tempBuffer + i * iBlockSize, iBlockSize);
		if (iResult != (int)iBlockSize) {
			NVBAK_LOG("[NVRAM]:dat file write error\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
		iAlreadyWrite++;
		if (iAlreadyWrite == iWriteTime)
			break;
		for (j = pos + 1; j < bitmappos; j++) {
			if (bBadBlockBitMap[j] == 0) {
				pos = j;
				break;
			}
		}
		if (j >= bitmappos) {
			NVBAK_LOG("[NVRAM]:there are not enough good blocks for write nvram data file\r\n");
			close(fd);
			free(tempBuffer);
			close(iFileDesc_file);
			close(iFileDesc_map);
			free(bBadBlockBitMap);
			return false;
		}
	}
	NVBAK_LOG("[NVRAM]:end:%d\n", pos);
	free(bBadBlockBitMap);
	close(fd);
	free(tempBuffer);
	close(iFileDesc_file);
	close(iFileDesc_map);
	NVBAK_LOG("setBackupFileNum\n");

	if (-1 == chown(g_pcNVM_AllFile , -1, AID_SYSTEM)) {
		NVBAK_LOG("chown for AllFile fail: %s", (char*)strerror(errno));
		//  return false;
	}

	if (-1 == chown(g_pcNVM_AllMap, -1, AID_SYSTEM)) {
		NVBAK_LOG("chown for AllMap fail: %s", (char*)strerror(errno));
		//   return false;
	}

#ifdef MTK_NVRAM_AUTO_BACKUP
	if (!Fileop_SetBackupFileNum(true)) {
		NVBAK_LOG("SetBackupFileNum Fail\n");
		return false;
	}
#endif


	NVBAK_LOG("SetBackupFileNum Success\n");
	NVBAK_LOG("Leave BackupToBinRegion_all_ex\n");

	snprintf(cLog, sizeof(cLog),"%s", "Leave BackupToBinRegion_all");
	if (bMarkCaller) {
		strncat (cLog,"_",strlen("_"));
		strncat (cLog,cCallerName,strlen(cCallerName));
	}
	bRet = NVRAM_HISTORY_LOG(1, cLog);
	if (bRet == false) {
		NVBAK_LOG("NVRAM_HISTORY_LOG write error!!!");
		return false;
	}

	if (nvram_misc_log_feature)
		NVRAM_MISC_LOG(1, cLog);

	sync();


	if (nvram_read_back_feature) {
		NVBAK_LOG("[NVRAM]Enter Check Backup\n");
		if (!FileOp_RestoreFromBinRegion_ToFile()) {
			NVBAK_LOG("[NVRAM]: FileOp_RestoreFromBinRegion_ToFile fail\n");
			return false;
		}
		if (!FileOp_CheckBackUpResult()) {
			NVBAK_LOG("[NVRAM]: FileOp_CheckBackUpResult fail\n");
			return false;
		}
		NVBAK_LOG("[NVRAM]Leave Check Backup\n");
	}
	return true;
}


bool FileOp_RestoreFromBinRegion(bool bCleanBoot) {
	int iFileDesc_file, iFileDesc_map, fd, iResult, iBlockNum, iFileDesc_backupflag;
	unsigned int iMapFileSize, iDatFileSize, iBlockSize, iPartitionSize,
	         iAPBackupFileNum, iWriteSize;
	unsigned short iMDBackupFileNum;
	bool bRet = true;
	struct stat st;
	char cMtdDevName[128] = {0};
	char *tempBuffer = NULL;
	char *tempBitmap1 = NULL;
	char *tempBitmap2 = NULL;
	int i, j, pos = 0, flag = 0;
	bool bSuccessFound = false;
	struct mtd_info_user info;
  #if 1
	int linkret =0 ;
  #endif
	//   int iBackup_Partition=get_partition_numb("nvram");

	NVBAK_LOG("[NVRAM]:FileOp_RestoreFromBinRegion\r\n");
	if (NVM_Init() < 0) {
		NVBAK_LOG("[NVRAM]:FileOp_RestoreFromBinRegion--NVM_Init fail !!!\n");
		return false;
	}

	memset(cMtdDevName, 0, sizeof cMtdDevName);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);

	NVBAK_LOG("[NVRAM]:cMtdDevName:%s\n", cMtdDevName);


	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_CreateBinRegionBadBlockBitMap get device info error\r\n");
		return false;
	}
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		return false;
	}

	iBlockSize = info.erasesize;
	iPartitionSize = info.size;
	iWriteSize = info.writesize;
#if 1
	NVBAK_LOG("libfile nvram_platform_log_block =%d,nvram_platform_resv_block=%d,nvram_platform_DM_block=%d nvram_platform_layout_version##=%d\n",
	          nvram_platform_log_block, nvram_platform_resv_block, nvram_platform_DM_block,
	          nvram_platform_layout_version);
	if (nvram_platform_layout_version == 0)
		BinRegionBlockTotalNum = info.size / iBlockSize - nvram_platform_log_block -
		                         nvram_platform_resv_block;
	else if ( nvram_platform_layout_version == 1)
		BinRegionBlockTotalNum = info.size / iBlockSize;
	else {
		NVRAM_LOG("[NVRAM]: invalid nvram layout version %d\r\n",
		          nvram_platform_layout_version);
		close(fd);
		return false;
	}
	NvRamBlockNum = BinRegionBlockTotalNum -  nvram_platform_DM_block;
	NVRAM_LOG("[NVRAM]: BinRegionBlockTotalNum %d , NvRamBlockNum=%d\r\n",
	          BinRegionBlockTotalNum, NvRamBlockNum);

#else
	NVBAK_LOG("libfile nvram_platform_log_block =%d,nvram_platform_resv_block=%d,nvram_platform_DM_block=%d\n",
	          nvram_platform_log_block, nvram_platform_resv_block, nvram_platform_DM_block);
	BinRegionBlockTotalNum = info.size / iBlockSize - nvram_platform_log_block -
	                         nvram_platform_resv_block;
	NvRamBlockNum = BinRegionBlockTotalNum -  nvram_platform_DM_block;
#endif

	tempBuffer = (char*)malloc(iBlockSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		return false;
	}
	if(lseek(fd, pos * iBlockSize, SEEK_SET) < 0) {
		NVBAK_LOG("[NVRAM]:seek to %d error %s\n",pos * iBlockSize, (char*)strerror(errno));
		free(tempBuffer);
		close(fd);
		return false;
	} else {
		//NVBAK_LOG("[NVRAM]:seek to %d Success\n",pos * iBlockSize);
	}
	
	iResult = read(fd, tempBuffer, iBlockSize);
	if(iResult != iBlockSize) {
		NVBAK_LOG("[NVRAM]:read error %s\n",(char*)strerror(errno));
		free(tempBuffer);
		close(fd);
		return false;
	}

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
	NVBAK_LOG("[NVRAM Restore]:map file size:%d\n", iMemSize);

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
		if(lseek(fd, (iBlockNum - iReservePos)*iBlockSize, SEEK_SET) < 0) {
			NVBAK_LOG("[NVRAM]:seek to %d error %s\n",(iBlockNum - iReservePos)*iBlockSize, (char*)strerror(errno));
			free(tempBuffer);
			free(tempBitmap1);
			free(tempBitmap2);
			close(fd);
			return false;
	    }
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
	if(lseek(fd, pos * iBlockSize, SEEK_SET) < 0) {
		NVBAK_LOG("[NVRAM]:seek to %d error %s\n",pos * iBlockSize, (char*)strerror(errno));
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		close(fd);
		return false;
	} else {
		NVBAK_LOG("[NVRAM]:seek to %d Success\n",pos * iBlockSize);
	}
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
	NVBAK_LOG("[NVRAM]: AP:%d,MD:%d\n", iAPBackupFileNum, iMDBackupFileNum);
	if(iMapFileSize == 0 || iMapFileSize == 0xFFFFFFFF) {
		NVBAK_LOG("[NVRAM]:map file size error\r\n");
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
	if(iDatFileSize == 0 || iDatFileSize == 0xFFFFFFFF) {
		NVBAK_LOG("[NVRAM]:data file size error\r\n");
		close(fd);
		free(tempBuffer);
		free(tempBitmap1);
		free(tempBitmap2);
		return false;
	}
#ifdef MTK_NVRAM_AUTO_BACKUP
	NVBAK_LOG("[NVRAM]: AP backup file num:%d,MD backup file num:%d\n",
	          iAPBackupFileNum, iMDBackupFileNum);
	if (iAPBackupFileNum != 0 ) {
		iFileDesc_backupflag = open(g_pcNVM_BackFlag , O_TRUNC | O_CREAT | O_RDWR,
		                            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if (INVALID_HANDLE_VALUE == iFileDesc_backupflag) {
			NVBAK_LOG("[NVRAM]: cannot open backup file num data\n");
			close(iFileDesc_backupflag);
			close(fd);
			free(tempBuffer);
			free(tempBitmap1);
			free(tempBitmap2);
			goto restorefiles;
		}
		iResult = write(iFileDesc_backupflag, &iAPBackupFileNum, sizeof(unsigned int));
		if (iResult != (int)sizeof(unsigned int)) {
			NVBAK_LOG("[NVRAM]:Backflag write error(ignore)\r\n");
			unlink(g_pcNVM_BackFlag);
			close(iFileDesc_backupflag);
		}
		if (!NVM_ComputeBackflagCheckSum(iFileDesc_backupflag)) {
			NVBAK_LOG("[NVRAM]:Backflag compute check sum failed(ignore)\r\n");
			unlink(g_pcNVM_BackFlag);
			close(iFileDesc_backupflag);
		}
	}
#endif
restorefiles:
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
		if (tempBitmap1[i] == 0) {
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
		for (i; i < iBlockNum; i++) {
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
	NVBAK_LOG("[NVRAM]:dat file read begin:%d,then write to ALLFile(data/nvram/ALLFile)\n", pos);
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
		// NVBAK_LOG("[NVRAM]:dat file read size:%d\n", iResult);
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

	if (bCleanBoot) {
		bRet = FileOp_RestoreData_All();
	} else {
		bRet = FileOp_RestoreFromFiles(APBOOT);
		bRet = FileOp_RestoreFromFiles(MDBOOT);
		//Add for second modem in MT658*
		bRet = FileOp_RestoreFromFiles(MD2BOOT);
		//End of Comment
	}

	return bRet;

}

/*
 * Notice
 * For origin design, ccci_fsd may create files and folders whose attribute is "0000, root.root".
 * In order to make cell phone more safe, ccci_fsd change its user from root to ccci.
 * For, MOTA update, data patition will not be ereased; then, ccci_fsd loss the capability to read/write md nvram.
 * EE will occur. So, we modify md nvram files's attribute fisrt here
 */
void change_md_nvram_attr(void) {
#if 0
	struct stat statbuf;
	NVBAK_LOG("[NVRAM]:change_md_nvram_attr++");
	if (stat("/mnt/vendor/nvdata/md/new_ver", &statbuf) == 0) {
		NVBAK_LOG("[NVRAM]:new_ver file exist!!!");
		NVBAK_LOG("[NVRAM]:change_md_nvram_attr--0");
		return;
	}

	NVBAK_LOG("[NVRAM]:new_ver file NOT exist, change attr");
	// Begin to change file mode and group
	//system("chmod 0770 /mnt/vendor/nvdata/md");
	system("chmod 0770 /mnt/vendor/nvdata/md/NVRAM");
	system("chmod 0770 /mnt/vendor/nvdata/md/NVRAM/NVD_IMEI");
	system("chmod 0770 /mnt/vendor/nvdata/md/NVRAM/IMPORTNT");
	system("chmod 0770 /mnt/vendor/nvdata/md/NVRAM/CALIBRAT");
	system("chmod 0770 /mnt/vendor/nvdata/md/NVRAM/NVD_CORE");
	system("chmod 0770 /mnt/vendor/nvdata/md/NVRAM/NVD_DATA");
	system("chmod 0660 /mnt/vendor/nvdata/md/NVRAM/NVD_IMEI/*");
	system("chmod 0660 /mnt/vendor/nvdata/md/NVRAM/IMPORTNT/*");
	system("chmod 0660 /mnt/vendor/nvdata/md/NVRAM/CALIBRAT/*");
	system("chmod 0660 /mnt/vendor/nvdata/md/NVRAM/NVD_CORE/*");
	system("chmod 0660 /mnt/vendor/nvdata/md/NVRAM/NVD_DATA/*");
	// Make sure files has correct owner and group
	system("chown root.nvram /mnt/vendor/nvdata/md");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/NVD_IMEI");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/IMPORTNT");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/CALIBRAT");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/NVD_CORE");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/NVD_DATA");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/NVD_IMEI/*");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/IMPORTNT/*");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/CALIBRAT/*");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/NVD_CORE/*");
	system("chown root.nvram /mnt/vendor/nvdata/md/NVRAM/NVD_DATA/*");
	// Write a flag file
	system("echo flag > /mnt/vendor/nvdata/md/new_ver");
	system("chmod 0660 /mnt/vendor/nvdata/md/new_ver");
	NVBAK_LOG("[NVRAM]:change_md_nvram_attr--1");
#endif
}

bool FileOp_RecoveryData() {
	bool bRet;
	int iResult;
	unsigned int iCleanBootFlag = 0;
	bool bCleanBoot;
	int iFileDesc_Flag;
	struct stat statbuf;
	unsigned int iCompleteFlag;

	//fix nvram layout issue , reassgined block numerbers by different platform
	if (NVM_Init() < 0) {
		NVBAK_LOG("[NVRAM]:FileOp_RecoveryData--NVM_Init fail!!!\n");
		return false;
	}
	change_md_nvram_attr();

	iResult = stat(g_pcNVM_Flag, &statbuf);
	/*if(ret==0)
	{
	   result=FileOp_GetCleanBootFlag(&iCleanBootFlag);
	   if(result!=true)
	   {
		NVBAK_LOG("FileOp_RecoveryData:get clean boot flag fail\n");
		return false;
	   }
	}

	if(ret==-1||iCleanBootFlag==CleanBootFlag)*/

	if (0 == iResult) {

		iFileDesc_Flag = open(g_pcNVM_Flag, O_RDONLY);
		if (-1 == iFileDesc_Flag) {
			NVBAK_LOG("[NVRAM]:Open Restore Flag file failed,go to restore!!!");
			goto Recovery;
		}
		iResult = read(iFileDesc_Flag, &iCompleteFlag, sizeof(unsigned int));
		if (iResult != sizeof(unsigned int)) {
			NVBAK_LOG("[NVRAM]:Read retore Flag file failed,go to restore!!!");
			close(iFileDesc_Flag);
			goto Recovery;
		}
		if (0x12345678 != iCompleteFlag) {
			NVBAK_LOG("[NVRAM]:Restore Flag is error, go to retore!!!");
			close(iFileDesc_Flag);
			goto Recovery;
		}
		close(iFileDesc_Flag);
	}

	if (iResult == -1) {
Recovery:
		bCleanBoot = true;
		NVBAK_LOG("enter clean boot\n");


		if (nvram_misc_log_feature)
			NVRAM_MISC_LOG(1, "Enter clean boot, Restore From BinRegion");

		if (!FileOp_RestoreFromBinRegion(bCleanBoot)) {
			NVBAK_LOG("FileOp_RecoveryData:recovery all data fail\n");
			return false;
		}
		/*if(!FileOp_SetCleanBootFlag(false))
		{
		  NVBAK_LOG("FileOp_RecoveryData:clear claenboot flag fail\n");
		  return false;
		}*/
		iFileDesc_Flag = open(g_pcNVM_Flag, O_TRUNC | O_CREAT | O_RDWR,
		                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if (iFileDesc_Flag == -1) {
			NVBAK_LOG("FileOp_RecoveryData:set claenboot done file fail\n");
			return false;
		}
		unsigned int iCompleteFlag = 0x12345678;
		write(iFileDesc_Flag, &iCompleteFlag, sizeof(unsigned int));
		close(iFileDesc_Flag);

		NVBAK_LOG("Leave clean boot, Restore From BinRegion\n");

		if (nvram_misc_log_feature)
			NVRAM_MISC_LOG(1, "Leave clean boot, Restore From BinRegion");
		iResult = FileOp_RestoreFromBinRegionForDM();
		if (!iResult) {
			NVBAK_LOG("FileOp_RecoveryData:RestoreFromBinRegionForDM fail\n");
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
static bool FileOp_MakeFile(const char* src, unsigned int* iFileNum,
                            int* FileSize) {
	unsigned char acBuffer[1024];
	int iSrc, iDst, iFileSize, iWriteSize, iSize; //FileSize;
	int iFileDesc_file, iFileDesc_map, iFileDesc;
	File_Title *FileInfo = NULL;
	bool bRet = false;
	struct stat statbuf;
	off_t iResult = 0;

	/* Koshi start */

	NVBAK_LOG("FileOp_MakeFile: Handle the file (%s)", src);
	if (iFileNum == NULL) {
		NVBAK_LOG("FileOp_MakeFile:Input iFileNum Null pointer!\n");
		return false;
	}
	if (FileSize == NULL) {
		NVBAK_LOG("FileOp_MakeFile:Input FileSize Null pointer!\n");
		return false;
	}

	iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
	iFileDesc_map = open(g_pcNVM_AllMap, O_RDWR);

	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create file data\n");
		if (iFileDesc_map != INVALID_HANDLE_VALUE)
			close(iFileDesc_map);
		return false;
	}

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create map data\n");
		close(iFileDesc_file);
		return false;
	}

	/* Seek to the file end */
	iResult = lseek(iFileDesc_file, 0, SEEK_END);
	if (iResult == (off_t) - 1) {
		NVBAK_LOG("iFileDesc_file seek error !\n");
		close(iFileDesc_file);
		close(iFileDesc_map);
		return false;
	}

	iResult = lseek(iFileDesc_map, 0, SEEK_END);

	if (iResult == (off_t) - 1) {
		NVBAK_LOG("iFileDesc_map seek error !\n");
		close(iFileDesc_file);
		close(iFileDesc_map);
		return false;
	}

	/* Koshi end */

	iSrc = open(src, O_RDONLY);

	if (-1 == iSrc) {
		goto EXIT;
	}

	fstat(iSrc, &statbuf);
	*FileSize = iFileSize = (int)statbuf.st_size;

	NVBAK_LOG("File size is (%d)", iFileSize);

	if (iFileSize == 0) {
		goto EXIT;
	}

	while (iFileSize > 0) {
		iWriteSize = min(iFileSize, (int)sizeof(acBuffer));
		iSize = (int)read(iSrc, acBuffer, iWriteSize);
		if (iSize != iWriteSize) {
			goto EXIT;
		}
		iSize = (int)write(iFileDesc_file, acBuffer, iWriteSize);
		if (iSize != iWriteSize) {
			goto EXIT;
		}
		iFileSize -= iWriteSize;
	}
	/* malloc the buffer of title buf */
	FileInfo = (File_Title *)malloc(sizeof(File_Title));
	if (FileInfo == NULL) {
		NVBAK_LOG("FileOp_MakeFile malloc memory for FileInfo failed!!\n");
		close(iFileDesc_file);
		close(iFileDesc_map);
		close(iSrc);
		return false;
	}
	memset(FileInfo, 0, sizeof(File_Title));

	/* Koshi: write map file */
	FileInfo->Filesize = *FileSize;
	FileInfo->NameSize = 7;
	*iFileNum += 1;
	pthread_mutex_lock(&gFileStartAddrlock);
	FileInfo->FielStartAddr = gFileStartAddr;
	memcpy(FileInfo->cFileName, src, MAX_NAMESIZE);
	write(iFileDesc_map, FileInfo, sizeof(File_Title));
	NVBAK_LOG("FileInfo: Filenum %u (addr - %d / size - %d) \n", *iFileNum,
	          FileInfo->FielStartAddr, FileInfo->Filesize);
	gFileStartAddr += FileInfo->Filesize;
	pthread_mutex_unlock(&gFileStartAddrlock);

	bRet = true;

EXIT:
	if (iSrc != -1) {
		close(iSrc);
	}
	if (iFileDesc_file != -1) {
		close(iFileDesc_file);
	}
	if (iFileDesc_map != -1) {
		close(iFileDesc_map);
	}
	free(FileInfo);
	return bRet;
}

bool FileOp_RestoreFromFiles(int eBackupType) {
	int iFileDesc_file, iFileDesc_map, iFileDesc, iSize;
	int iFileTitleOffset = 0;
	short int iFileNum = 0;
	char *buf;  /* content  */
	File_Title *FileInfo = NULL;
	bool bRet = true;
	off_t iResult;
	File_Title_Header1 FileTitleInfo1;
	File_Title_Header2 FileTitleInfo2;
	File_Title_Header3 FileTitleInfo3;
	File_Title_Header4 FileTitleInfo4;
	File_Title_Header5 FileTitleInfo5;
	memset(&FileTitleInfo1, 0, sizeof(File_Title_Header1));
	memset(&FileTitleInfo2, 0, sizeof(File_Title_Header2));
	memset(&FileTitleInfo3, 0, sizeof(File_Title_Header3));
	memset(&FileTitleInfo4, 0, sizeof(File_Title_Header4));
	memset(&FileTitleInfo5, 0, sizeof(File_Title_Header5));
	int fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
	              File_Title_Header1);//In order to align, minus sizeof(short int)

	if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
		NVBAK_LOG("FileOp_RestoreFromFiles: File_Title_Header1 set\n");
	} else if (nvram_md5_support() && !nvram_evdo_support()
	           && !nvram_ecci_c2k_support()) {
		NVBAK_LOG("FileOp_RestoreFromFiles: File_Title_Header2 set\n");
		fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
		          File_Title_Header2) - 2 * sizeof(short
		                  int); //In order to align, minus 2*sizeof(short int)
	} else if (!nvram_md5_support() && nvram_evdo_support()
	           && !nvram_ecci_c2k_support()) {
		NVBAK_LOG("FileOp_RestoreFromFiles: File_Title_Header3 set\n");
		fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
		          File_Title_Header3) - 4 * sizeof(short
		                  int); //In order to align, minus 4*sizeof(short int)
	} else if (!nvram_md5_support() && !nvram_evdo_support()
	           && !nvram_ecci_c2k_support()) {
		NVBAK_LOG("FileOp_RestoreFromFiles: File_Title_Header4 set\n");
		fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
		          File_Title_Header4) - 4 * sizeof(short int);
	} else {
		NVBAK_LOG("FileOp_RestoreFromFiles: File_Title_Header5 set\n");
		fhs = sizeof(unsigned int) + 2 * sizeof(unsigned int) + sizeof(
		          File_Title_Header5);
	}

	int fis = sizeof(File_Title);

	/* malloc the buffer of title buf */
	FileInfo = (File_Title *)malloc(sizeof(File_Title));
	if (FileInfo == NULL) {
		NVBAK_LOG("FileOp_MakeFile malloc mermory for FileInfo failed\n");
		return false;
	}
	memset(FileInfo, 0, sizeof(File_Title));

	iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
	iFileDesc_map = open(g_pcNVM_AllMap, O_RDWR);

	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open file data\n");
		if (iFileDesc_map != INVALID_HANDLE_VALUE)
			close(iFileDesc_map);
		free(FileInfo);
		return false;
	}

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open map data\n");
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
		//End of Comemnt
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
		          *( (short int *) & (FileTitleInfo1.iFileBufLen) -
		             nvram_platform_header_offset) );
		NVBAK_LOG("File_Title_Header BackupFlag(%d)",
		          *( (short int *) & (FileTitleInfo1.BackupFlag) -
		             nvram_platform_header_offset) );
	}
#endif
	if (nvram_platform_header_offset != 0) {
		fhs = fhs - nvram_platform_header_offset * 2;
	}
	switch (eBackupType) {
	case APBOOT:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles APBOOT start !");
			iFileNum = FileTitleInfo1.iApBootNum;
			iFileTitleOffset = fhs;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles APBOOT start !");
			iFileNum = FileTitleInfo2.iApBootNum;
			iFileTitleOffset = fhs;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles APBOOT start !");
			iFileNum = FileTitleInfo3.iApBootNum;
			iFileTitleOffset = fhs;
		} else if (!nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles APBOOT start !");
			iFileNum = FileTitleInfo4.iApBootNum;
			iFileTitleOffset = fhs;
		} else { /*nvram_ecci_c2k_support()==ture*/
			NVBAK_LOG("FileOp_RestoreFromFiles APBOOT start !");
			iFileNum = FileTitleInfo5.iApBootNum;
			iFileTitleOffset = fhs;
		}
		break;

	case APCLN:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles APCLN start !");
			iFileNum = FileTitleInfo1.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles APCLN start !");
			iFileNum = FileTitleInfo2.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles APCLN start !");
			iFileNum = FileTitleInfo3.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum) * fis;
		} else if (!nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles APCLN start !");
			iFileNum = FileTitleInfo4.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum) * fis;
		} else {
			NVBAK_LOG("FileOp_RestoreFromFiles APCLN start !");
			iFileNum = FileTitleInfo5.iApCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum) * fis;
		}
		break;

	case MDBOOT:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDBOOT start !");
			iFileNum = FileTitleInfo1.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum +
			                          FileTitleInfo1.iApCleanNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDBOOT start !");
			iFileNum = FileTitleInfo2.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum +
			                          FileTitleInfo2.iApCleanNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDBOOT start !");
			iFileNum = FileTitleInfo3.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum +
			                          FileTitleInfo3.iApCleanNum) * fis;
		} else if (!nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDBOOT start !");
			iFileNum = FileTitleInfo4.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum +
			                          FileTitleInfo4.iApCleanNum) * fis;
		} else {
			NVBAK_LOG("FileOp_RestoreFromFiles MDBOOT start !");
			iFileNum = FileTitleInfo5.iMdBootNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum +
			                          FileTitleInfo5.iApCleanNum) * fis;
		}
		break;

	case MDCLN:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCLN start !");
			iFileNum = FileTitleInfo1.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCLN start !");
			iFileNum = FileTitleInfo2.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
			                          + FileTitleInfo2.iMdBootNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCLN start !");
			iFileNum = FileTitleInfo3.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum) * fis;
		} else if (!nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCLN start !");
			iFileNum = FileTitleInfo4.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
			                          + FileTitleInfo4.iMdBootNum) * fis;
		} else {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCLN start !");
			iFileNum = FileTitleInfo5.iMdCleanNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum) * fis;
		}
		break;

	case MDIMP:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDIMP start !");
			iFileNum = FileTitleInfo1.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDIMP start !");
			iFileNum = FileTitleInfo2.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
			                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDIMP start !");
			iFileNum = FileTitleInfo3.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum) * fis;
		}

		else if (!nvram_md5_support() && !nvram_evdo_support()
		         && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDIMP start !");
			iFileNum = FileTitleInfo4.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
			                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum) * fis;
		} else {
			NVBAK_LOG("FileOp_RestoreFromFiles MDIMP start !");
			iFileNum = FileTitleInfo5.iMdImpntNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum) * fis;
		}
		break;

	case MDCOR:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCOR start !");
			iFileNum = FileTitleInfo1.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
			                          FileTitleInfo1.iMdImpntNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCOR start !");
			iFileNum = FileTitleInfo2.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
			                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
			                          FileTitleInfo2.iMdImpntNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCOR start !");
			iFileNum = FileTitleInfo3.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
			                          FileTitleInfo3.iMdImpntNum) * fis;
		}

		else if (!nvram_md5_support() && !nvram_evdo_support()
		         && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCOR start !");
			iFileNum = FileTitleInfo4.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
			                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum +
			                          FileTitleInfo4.iMdImpntNum) * fis;
		} else {
			NVBAK_LOG("FileOp_RestoreFromFiles MDCOR start !");
			iFileNum = FileTitleInfo5.iMdCoreNum;
			iFileTitleOffset = fhs + (FileTitleInfo5.iApBootNum + FileTitleInfo5.iApCleanNum
			                          + FileTitleInfo5.iMdBootNum + FileTitleInfo5.iMdCleanNum +
			                          FileTitleInfo5.iMdImpntNum) * fis;
		}
		break;

	case MDDATA:
		if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDDATA start !");
			iFileNum = FileTitleInfo1.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo1.iApBootNum + FileTitleInfo1.iApCleanNum
			                          + FileTitleInfo1.iMdBootNum + FileTitleInfo1.iMdCleanNum +
			                          FileTitleInfo1.iMdImpntNum + FileTitleInfo1.iMdCoreNum) * fis;
		} else if (nvram_md5_support() && !nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDDATA start !");
			iFileNum = FileTitleInfo2.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo2.iApBootNum + FileTitleInfo2.iApCleanNum
			                          + FileTitleInfo2.iMdBootNum + FileTitleInfo2.iMdCleanNum +
			                          FileTitleInfo2.iMdImpntNum + FileTitleInfo2.iMdCoreNum) * fis;
		} else if (!nvram_md5_support() && nvram_evdo_support()
		           && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDDATA start !");
			iFileNum = FileTitleInfo3.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo3.iApBootNum + FileTitleInfo3.iApCleanNum
			                          + FileTitleInfo3.iMdBootNum + FileTitleInfo3.iMdCleanNum +
			                          FileTitleInfo3.iMdImpntNum + FileTitleInfo3.iMdCoreNum) * fis;
		}

		else if (!nvram_md5_support() && !nvram_evdo_support()
		         && !nvram_ecci_c2k_support()) {
			NVBAK_LOG("FileOp_RestoreFromFiles MDDATA start !");
			iFileNum = FileTitleInfo4.iMdDataNum;
			iFileTitleOffset = fhs + (FileTitleInfo4.iApBootNum + FileTitleInfo4.iApCleanNum
			                          + FileTitleInfo4.iMdBootNum + FileTitleInfo4.iMdCleanNum +
			                          FileTitleInfo4.iMdImpntNum + FileTitleInfo4.iMdCoreNum) * fis;
		} else {
			NVBAK_LOG("FileOp_RestoreFromFiles MDDATA start !");
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
	/*Add for C2K modem*/
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


	//End of Comment

	case ALL:
		break;
	default:
		close(iFileDesc_map);
		close(iFileDesc_file);
		free(FileInfo);
		return false;
		//break;
	}

	iResult = lseek(iFileDesc_map, iFileTitleOffset, SEEK_SET);

	//restore all files one by one according to the fout.txt and fTitleInfo.txt
	//do
	//avoid the file num of some folder is zero
	while (iFileNum > 0 && iFileNum <=SHRT_MAX) {
		memset(FileInfo, 0, sizeof(File_Title));
		iSize = (int)read(iFileDesc_map, FileInfo, sizeof(File_Title));
		FileInfo->cFileName[MAX_NAMESIZE-1]='\0';

		fix_legacy_nvram_path_in_binregion(FileInfo->cFileName, sizeof(FileInfo->cFileName));
		NVBAK_LOG("FileInfo: %s, startaddr(%x) size(%x) .\n", FileInfo->cFileName, FileInfo->FielStartAddr,
		          FileInfo->Filesize);

		if (strstr(FileInfo->cFileName, "FILE_VER")) {
			iFileDesc = open_file_with_dirs(FileInfo->cFileName, 0664);
		} else {
			iFileDesc = open_file_with_dirs(FileInfo->cFileName, 0660);
		}

		if (INVALID_HANDLE_VALUE == iFileDesc) {
			/* Error handling */
			NVBAK_LOG("FileOp_RestoreFromFiles cannot create %s\n", FileInfo->cFileName);

			//added
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(FileInfo);
			return false;
		}
		if (FileInfo->Filesize <= 0 || FileInfo->Filesize >=INT_MAX) {
			/* Error handling */
			NVBAK_LOG("FileOp_RestoreFromFiles FileInfo->Filesize <= 0\n");

			//added
			close(iFileDesc_map);
			close(iFileDesc_file);
			close(iFileDesc);
			free(FileInfo);
			return false;
		}

		buf = (char *)malloc(FileInfo->Filesize);
		if (buf == NULL) {
			/* Error handling */
			NVBAK_LOG("FileOp_RestoreFromFiles malloc memory for buff faild\n");

			//added
			close(iFileDesc_map);
			close(iFileDesc_file);
			close(iFileDesc);
			free(FileInfo);
			return false;
		}

		//read the data and write to the file
		iResult = lseek(iFileDesc_file, FileInfo->FielStartAddr, SEEK_SET);
		if (iResult == -1) {
			NVBAK_LOG("lseek fail !");
			close(iFileDesc);
			free(buf);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(FileInfo);
			return false;
		}
		iSize = (int)read(iFileDesc_file, buf, FileInfo->Filesize);
		if (iSize != FileInfo->Filesize) {
			NVBAK_LOG("read fail !");
			close(iFileDesc);
			free(buf);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(FileInfo);
			return false;
		}
		iSize = (int)write(iFileDesc, buf, FileInfo->Filesize);
		if (iSize != FileInfo->Filesize) {
			NVBAK_LOG("write fail !");
			close(iFileDesc);
			free(buf);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(FileInfo);
			return false;
		}
		close(iFileDesc);
		free(buf);
		--iFileNum;
	}
	//while (--iFileNum);

	close(iFileDesc_map);
	close(iFileDesc_file);
	free(FileInfo);
	return true;
}
static unsigned int FileOp_ComputeCheckSum(void)
{
     time_t start = time(NULL);
     NVBAK_LOG("Starting FileOp_ComputeCheckSum (pid %d) on %s",getpid(),ctime(&start));////add for log
     int iFileDesc_file;
     char cReadData;
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
        NVBAK_LOG("Error FileOp_ComputeCheckSum stat \n");
        return 0;
    }
   iFileSize=st.st_size;
   looptime=iFileSize/(sizeof(unsigned int));
   tmplooptime = looptime/(Loop_num) + 1;
   iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
      if(iFileDesc_file<0)
      {
            NVBAK_LOG("FileOp_ComputeCheckSum cannot open data file\n");
            return 0;
      }
   flag=1;
   NVBAK_LOG("looptime is %d, tmplooptime is %d \n", looptime, tmplooptime);////add for log
   time_t start_while = time(NULL);////add for log
   // NVBAK_LOG("Start_while FileOp_ComputeCheckSum (pid %d) on %s",getpid(),ctime(&start_while));//add
   for(i=0;i<tmplooptime;i++)
    {
        if(i != tmplooptime-1 )
        {
            jloop_num=Loop_num;}
        else
        {
            jloop_num=looptime%(Loop_num);}
            //NVBAK_LOG("Start_while: i is %d, FileOp_ComputeCheckSum (pid %d) on %s",i,getpid(),ctime(&start_while));// add for log
            iResult=read(iFileDesc_file, tempBuf, jloop_num*iLength);
            //NVBAK_LOG("Start_while: iResult is %d, FileOp_ComputeCheckSum (pid %d) on %s",iResult,getpid(),ctime(&start_while));//add for log
        if(iResult!= jloop_num*iLength)
        {
            NVBAK_LOG("FileOp_GetCheckSum cannot read checksum data\n");
            close(iFileDesc_file);
            return 0;
        }
        //NVBAK_LOG("Start_while:before_while FileOp_ComputeCheckSum (pid %d) on %s",getpid(),ctime(&start_while));//add for log
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
      NVBAK_LOG("tempNum = %lx", tempNum);//add for log
      if(iResult!= iLength)
           {
              NVBAK_LOG("FileOp_GetCheckSum cannot read last checksum data\n");
              close(iFileDesc_file);
              return 0;
           }
        ulCheckSum+=tempNum;
        //ulCheckSum^=gFileStartAddr;
        close(iFileDesc_file);
        time_t end = time(NULL);
        NVBAK_LOG("Ending FileOp_ComputeCheckSum (pid %d) on %s",getpid(),ctime(&end));//add for log
        return ulCheckSum;
}
#if 0
static unsigned int FileOp_ComputeCheckSum(void) {
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
		NVBAK_LOG("Error FileOp_ComputeCheckSum stat \n");
		return 0;
	}

	iFileSize = st.st_size;

	looptime = iFileSize / (sizeof(unsigned int));

	iFileDesc_file = open(g_pcNVM_AllFile , O_RDWR);
	if (iFileDesc_file < 0) {
		NVBAK_LOG("FileOp_ComputeCheckSum cannot open data file\n");
		return 0;
	}
	flag = 1;
	for (i = 0; i < looptime; i++) {
		iResult = read(iFileDesc_file, &tempNum, iLength);
		if (iResult != iLength) {
			NVBAK_LOG("FileOp_GetCheckSum cannot read checksum data\n");
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
		NVBAK_LOG("FileOp_GetCheckSum cannot read last checksum data\n");
		close(iFileDesc_file);
		return 0;
	}
	ulCheckSum += tempNum;
	//ulCheckSum^=gFileStartAddr;
	close(iFileDesc_file);
	return ulCheckSum;
}
#endif

static unsigned int FileOp_ComputeReadBackCheckSum(void) {
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

	if (stat(g_pcNVM_AllFile_Check, &st) < 0) {
		NVBAK_LOG("Error FileOp_ComputeCheckSum stat \n");
		return 0;
	}

	iFileSize = st.st_size;

	looptime = iFileSize / (sizeof(unsigned int));

	iFileDesc_file = open(g_pcNVM_AllFile_Check , O_RDWR);
	if (iFileDesc_file < 0) {
		NVBAK_LOG("FileOp_ComputeCheckSum cannot open data file\n");
		return 0;
	}
	flag = 1;
	for (i = 0; i < looptime; i++) {
		iResult = read(iFileDesc_file, &tempNum, iLength);
		if (iResult != iLength) {
			NVBAK_LOG("FileOp_GetCheckSum cannot read checksum data\n");
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
		NVBAK_LOG("FileOp_GetCheckSum cannot read last checksum data\n");
		close(iFileDesc_file);
		return 0;
	}
	ulCheckSum += tempNum;
	//ulCheckSum^=gFileStartAddr;
	close(iFileDesc_file);
	return ulCheckSum;
}

static BackupFileInfo stBackupFileInfo;
static bool FileOp_GetCheckSum(void) {
	int iFileDesc_map;
	unsigned int ulCheckSum;
	int iResult;
	int iLength = sizeof(unsigned int);

	iFileDesc_map = open(g_pcNVM_AllMap, O_RDONLY);
	if (iFileDesc_map < 0) {
		NVBAK_LOG("FileOp_GetCheckSum cannot open/create map data\n");
		return false;
	}
	iResult = read(iFileDesc_map, &stBackupFileInfo.ulCheckSum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("FileOp_GetCheckSum cannot read checksum data\n");
		close(iFileDesc_map);
		return false;
	}
	iLength = sizeof(unsigned int);
	iResult = read(iFileDesc_map, &stBackupFileInfo.iCommonFileNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("FileOp_GetCheckSum cannot read checksum data\n");
		close(iFileDesc_map);
		return false;
	}
	iResult = read(iFileDesc_map, &stBackupFileInfo.iCustomFileNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("FileOp_GetCheckSum cannot read checksum data\n");
		close(iFileDesc_map);
		return false;
	}
	close(iFileDesc_map);

	return true;
}

static bool FileOp_SetCheckSum(unsigned int ulCheckSum) {
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
		NVBAK_LOG("FileOp_SetCheckSum cannot open/create map data\n");
		return false;
	}
	iResult = write(iFileDesc_map, &ulCheckSum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("FileOp_SetCheckSum cannot write checksum data\n");
		close(iFileDesc_map);
		return false;
	}
	NVBAK_LOG("common file num:%d,custom file num:%d\n", iOldCommonFileNum,
	          iOldCustomFileNum);
	iLength = sizeof(unsigned int);
	iResult = write(iFileDesc_map, &iOldCommonFileNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("FileOp_SetCheckSum cannot write common file num data\n");
		close(iFileDesc_map);
		return false;
	}
	iResult = write(iFileDesc_map, &iOldCustomFileNum, iLength);
	if (iResult != iLength) {
		NVBAK_LOG("FileOp_SetCheckSum cannot write custom file num data\n");
		close(iFileDesc_map);
		return false;
	}
	close(iFileDesc_map);
	return true;
}

static bool FileOp_CompareCheckSum(unsigned int ulCheckSum1,
                                   unsigned int ulCheckSum2) {
	if (ulCheckSum1 != ulCheckSum2)
		return false;
	return true;
}

bool FileOp_IsSyncWithBinRegion( ) {
    bool bRet = true;
    struct stat st;
    unsigned int iMapFileSize, iDatFileSize;
    int fd, iResult;
    char cMtdDevName[128] = {0};
    unsigned int ulCheckSum;

    NVBAK_LOG("Enter %s\n", __FUNCTION__);

    if (NVM_Init() < 0) {
        NVBAK_LOG("%s--NVM_Init fail !!!", __FUNCTION__);
        return false;
    }

    memset(cMtdDevName, 0, sizeof cMtdDevName);
    snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);

    bool bWorkForBinRegion = true;
    int iFileMask[ALL];
    //need init ifilemark value ,otherwize it cause backup fail on 6592+kk1.mp3+non lte project
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
    if (!(nvram_ecci_c2k_support())) {
        iFileMask[14] = 1;
        iFileMask[15] = 1;
        iFileMask[16] = 1;
    }

//   ret=FileOp_BackupData_All();
    bRet = FileOp_BackupDataToFiles(iFileMask, bWorkForBinRegion);
    if (bRet == false) {
        NVBAK_LOG("[NVRAM]:Crete the map file and the data file fail\n");
        return false;
    }
    if (stat(g_pcNVM_AllMap, &st) < 0) {
        NVBAK_LOG("[NVRAM]:Error MapFile stat \n");
        return false;
    }
    iMapFileSize = st.st_size;
    if (stat(g_pcNVM_AllFile, &st) < 0) {
        NVBAK_LOG("[NVRAM]:Error DatFile stat \n");
        return false;
    }
    iDatFileSize = st.st_size;
    NVBAK_LOG("[NVRAM]:info:iMapFileSize:%d,iDatFileSize:%d\n", iMapFileSize,
              iDatFileSize);

    //NVBAK_LOG("[NVRAM]:Backup_Partition:%d\n",iBackup_Partition);
    fd = open(cMtdDevName, O_RDWR);
    if (fd < 0) {
        NVBAK_LOG("[NVRAM]:mtd open error\r\n");
        free(bBadBlockBitMap);
        return false;
    }

    // get checksum from AllMap
    if (!FileOp_GetCheckSum()) {
       NVBAK_LOG("[NVRAM]: FileOp_GetCheckSum() error \n");
       close(fd);
       return false;
    }

    // read checksum from bin region
    if(lseek(fd, 3 * sizeof(unsigned int), SEEK_SET) < 0) {
        NVBAK_LOG("[NVRAM]:seek to %d error %s\n", 3 * sizeof(unsigned int), (char*)strerror(errno));
        close(fd);
        return false;
    }
    iResult = read(fd, &ulCheckSum, sizeof(unsigned int));
    if (iResult != sizeof(unsigned int)) {
        NVBAK_LOG("[NVRAM]: read bin region error\r\n");
        close(fd);
        return false;
    }
    close(fd);

    NVBAK_LOG("[NVRAM]: %s, checksum(in nvdata)=0x%X, checksum(in binregion)=0x%X \n", __FUNCTION__, stBackupFileInfo.ulCheckSum, ulCheckSum);
    return (stBackupFileInfo.ulCheckSum == ulCheckSum);
}

//-----------------------------------------------------------------------------
bool FileOp_BackupDataToFiles(int * iFileMask, bool bWorkForBinRegion) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	const char* lpSrcDirName = NULL;
	char acSrcPathName[MAX_NAMESIZE];
	char acDstPathName[MAX_NAMESIZE];
	unsigned int iFileNum = 0;
	int iFileDesc_map, FileSize = 0, eBackupType, iFileDesc_file;
	bool bRet = true;
	off_t iResult;
	unsigned int i = 0;
	unsigned int ulCheckSum = 0;
	unsigned int iOldCommonFileNum = 0;
	unsigned int iOldCustomFileNum = 0;
	bool bMask = true;
	int iMask[ALL];
	File_Title_Header1 FileTitleInfo1;
	File_Title_Header2 FileTitleInfo2;
	File_Title_Header3 FileTitleInfo3;
	File_Title_Header4 FileTitleInfo4;
	File_Title_Header5 FileTitleInfo5;
	memset(&FileTitleInfo1, 0, sizeof(File_Title_Header1));
	memset(&FileTitleInfo2, 0, sizeof(File_Title_Header2));
	memset(&FileTitleInfo3, 0, sizeof(File_Title_Header3));
	memset(&FileTitleInfo4, 0, sizeof(File_Title_Header4));
	memset(&FileTitleInfo5, 0, sizeof(File_Title_Header5));

	{
	  mode_t pre_umask = umask(000);
	  NVBAK_LOG("%s: previous umask = 0x%x", __FUNCTION__, pre_umask);
	}

	if (iFileMask != NULL)
		memcpy(iMask, iFileMask, (ALL * sizeof(int)));
	else {
		NVBAK_LOG("iFileMask is NULL\n");
		return false;
	}

	/* Create the map file */
	iFileDesc_map = open(g_pcNVM_AllMap, O_TRUNC | O_CREAT | O_WRONLY,
	                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create map data\n");
		return false;
	}

	write(iFileDesc_map, &ulCheckSum, sizeof(unsigned int));
	write(iFileDesc_map, &iOldCommonFileNum, sizeof(unsigned int));
	write(iFileDesc_map, &iOldCustomFileNum, sizeof(unsigned int));
	/* Reserve the FileTitleInfo space */
	if (!nvram_ecci_c2k_support()) {
		if (nvram_platform_header_offset == 0 && nvram_md5_support()
		        && nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo1, sizeof(FileTitleInfo1));
		else if (nvram_platform_header_offset == 0 && nvram_md5_support()
		         && !nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo2,
			      sizeof(FileTitleInfo2) - 2 * sizeof(short
			              int));//In order to align, minus 2*sizeof(short int)
		else if (nvram_platform_header_offset == 0 && !nvram_md5_support()
		         && nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo3,
			      sizeof(FileTitleInfo3) - 4 * sizeof(short
			              int));//In order to align, minus 4*sizeof(short int)
		else if (nvram_platform_header_offset == 0 && !nvram_md5_support()
		         && !nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo4,
			      sizeof(FileTitleInfo4) - 4 * sizeof(short int));
		else if (nvram_platform_header_offset != 0 && nvram_md5_support()
		         && nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo1,
			      sizeof(FileTitleInfo1) - nvram_platform_header_offset * 2);
		else if (nvram_platform_header_offset != 0 && nvram_md5_support()
		         && !nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo2,
			      sizeof(FileTitleInfo2) - 2 * sizeof(short int) - nvram_platform_header_offset *
			      2);//In order to align, minus 2*sizeof(short int)
		else if (nvram_platform_header_offset != 0 && !nvram_md5_support()
		         && nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo3,
			      sizeof(FileTitleInfo3) - 4 * sizeof(short int) - nvram_platform_header_offset *
			      2);//In order to align, minus 4*sizeof(short int)
		else
			write(iFileDesc_map, &FileTitleInfo4,
			      sizeof(FileTitleInfo4) - 4 * sizeof(short int) - nvram_platform_header_offset *
			      2);
	} else
		write(iFileDesc_map, &FileTitleInfo5, sizeof(FileTitleInfo5));


	close(iFileDesc_map);
	/* Create the data file */
	iFileDesc_file = open(g_pcNVM_AllFile , O_TRUNC | O_CREAT | O_RDWR,
	                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create data file\n");
		return false;
	}
	close(iFileDesc_file);

	for (eBackupType = APBOOT; eBackupType < ALL; eBackupType++) {
		if (iMask[eBackupType] == 1)
			continue;

		switch (eBackupType) {
		case APBOOT:
			NVBAK_LOG("FileOp_BackupData_All APBOOT start !");
			lpSrcDirName = g_pcNVM_APRdebFile;
			break;

		case MDBOOT:
			NVBAK_LOG("FileOp_BackupData_All MDBOOT start !");
			lpSrcDirName = g_pcNVM_MDRdebFile;
			break;

		case APCLN:
			NVBAK_LOG("FileOp_BackupData_All APCLN start !");
			lpSrcDirName = g_pcNVM_APCalFile;
			break;

		case MDCLN:
			NVBAK_LOG("FileOp_BackupData_All MDCLN start !");
			lpSrcDirName = g_pcNVM_MDCalFile;
			break;

		case MDIMP:
			NVBAK_LOG("FileOp_BackupData_All MDIMP start !");
			lpSrcDirName = g_pcNVM_MDImptFile;
			break;

		case MDCOR:
			NVBAK_LOG("FileOp_BackupData_All MDCOR start !");
			lpSrcDirName = g_pcNVM_MDCoreFile;
			break;

		case MDDATA:
			NVBAK_LOG("FileOp_BackupData_All MDDATA start !");
			lpSrcDirName = g_pcNVM_MDDataFile;
			break;
		//Add for second modem for MT658*
		case MD2BOOT:
			if (nvram_platform_header_offset == 0) {
				NVBAK_LOG("FileOp_BackupData_All MD2BOOT start !");
				lpSrcDirName = g_pcNVM_MD2RdebFile;
			}
			break;

		case MD2CLN:
			if (nvram_platform_header_offset == 0) {

				NVBAK_LOG("FileOp_BackupData_All MD2CLN start !");
				lpSrcDirName = g_pcNVM_MD2CalFile;
			}
			break;

		case MD2IMP:
			if (nvram_platform_header_offset == 0) {

				NVBAK_LOG("FileOp_BackupData_All MD2IMP start !");
				lpSrcDirName = g_pcNVM_MD2ImptFile;
			}
			break;

		//End of Comment
		//LTE support
		case MD5BOOT:
			if (nvram_platform_header_offset == 0) {

				NVBAK_LOG("FileOp_BackupData_All MD5BOOT start !");
				lpSrcDirName = g_pcNVM_MD5RdebFile;
			}
			break;

		case MD5CLN:
			if (nvram_platform_header_offset == 0) {
				NVBAK_LOG("FileOp_BackupData_All MD5CLN start !");
				lpSrcDirName = g_pcNVM_MD5CalFile;
			}
			break;

		case MD5IMP:
			if (nvram_platform_header_offset == 0) {
				NVBAK_LOG("FileOp_BackupData_All MD5IMP start !");
				lpSrcDirName = g_pcNVM_MD5ImptFile;
			}
			break;
		case VIA:
			NVBAK_LOG("FileOp_BackupData_All VIA start !");
			lpSrcDirName = g_pcNVM_Via;
			break;
		/*Add for C2K modem*/
		case MD3BOOT:
			if (nvram_platform_header_offset == 0) {
				NVBAK_LOG("FileOp_BackupData_All MD3BOOT start !");
				lpSrcDirName = g_pcNVM_MD3RdebFile;
			}
			break;

		case MD3CLN:
			if (nvram_platform_header_offset == 0) {

				NVBAK_LOG("FileOp_BackupData_All MD3CLN start !");
				lpSrcDirName = g_pcNVM_MD3CalFile;
			}
			break;

		case MD3IMP:
			if (nvram_platform_header_offset == 0) {

				NVBAK_LOG("FileOp_BackupData_All MD3IMP start !");
				lpSrcDirName = g_pcNVM_MD3ImptFile;
			}
			break;
		/*Add for C2K modem -End*/
		case ALL:
		default:
			return false;
			//break;
		}

		/* Check the folder in NvRam is exists */
		NVBAK_LOG("open dir %s !", lpSrcDirName);
		if (lpSrcDirName) {
			dir = opendir(lpSrcDirName);
		}

		if (dir == NULL) {
			NVBAK_LOG("FileOp_BackupData open dir: %s ,this dir not exist!", lpSrcDirName);
			continue;
			//return false;
		}

		while (1) {
			entry = readdir(dir);

			if (entry == NULL) {
				break;
			}

			lstat(entry->d_name, &statbuf);

			if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
				continue;
			}

			sprintf(acSrcPathName, "%s/%s", lpSrcDirName, entry->d_name);
			NVBAK_LOG("FileOp_BackupData: (%s)\n", acSrcPathName);
			if (bWorkForBinRegion) {
				if (eBackupType == APBOOT || eBackupType == APCLN) {
					bMask = true;
					for (i = 0; i < g_Backup_File_Count; i++) {
						if (0 == strcmp(entry->d_name, aBackupToBinRegion[i].cFileName) ) {
							bMask = false;
							break;
						}
					}

					if (bMask == true)
						continue;
				}
			}

			// avoid backup the files in performace array  to PC side.
			if (!bWorkForBinRegion) {
				if (eBackupType == APBOOT || eBackupType == APCLN) {
					bMask = false;
					for (i = 0; i < g_Performance_File_Count; i++) {

						if (0 == strcmp(entry->d_name, aPerformance[i].cFileName) ) {
							NVBAK_LOG("skip (%s)\n", entry->d_name);
							bMask = true;
							break;
						}
					}

					if (bMask == true)
						continue;
				}
			}
			if (!FileOp_MakeFile(acSrcPathName, &iFileNum, &FileSize)) {
				NVBAK_LOG("ERROR FileOp_BackupData (%s)\n", acSrcPathName);
				bRet = false;
				continue;
			}
		}
		closedir(dir);
		NVBAK_LOG("FileSize is (%d) !", FileSize);
		NVBAK_LOG("FileNum is (%u) !", iFileNum);
		if (nvram_platform_header_offset == 0 && !nvram_ecci_c2k_support()) {
			if (nvram_md5_support() && nvram_evdo_support()) {
				FileTitleInfo1.iFileBufLen += FileSize;
				NVBAK_LOG("iFileBufLen is (%d) !", FileTitleInfo1.iFileBufLen);
			} else if (nvram_md5_support() && !nvram_evdo_support()) {
				FileTitleInfo2.iFileBufLen += FileSize;
				NVBAK_LOG("iFileBufLen is (%d) !", FileTitleInfo2.iFileBufLen);
			} else if (!nvram_md5_support() && nvram_evdo_support()) {
				FileTitleInfo3.iFileBufLen += FileSize;
				NVBAK_LOG("iFileBufLen is (%d) !", FileTitleInfo3.iFileBufLen);
			} else {
				FileTitleInfo4.iFileBufLen += FileSize;
				NVBAK_LOG("iFileBufLen is (%d) !", FileTitleInfo4.iFileBufLen);
			}
		} else if (nvram_platform_header_offset == 0 && nvram_ecci_c2k_support()) {
			FileTitleInfo5.iFileBufLen += FileSize;
			NVBAK_LOG("iFileBufLen is (%d) !", FileTitleInfo5.iFileBufLen);
		}

		if (nvram_platform_header_offset != 0 && !nvram_ecci_c2k_support()) {
			if (nvram_md5_support() && nvram_evdo_support()) {
				*((short int *) & (FileTitleInfo1.iFileBufLen) - nvram_platform_header_offset)
				+= FileSize;
				NVBAK_LOG("iFileBufLen is (%d) !",
				          *((short int *) & (FileTitleInfo1.iFileBufLen) - nvram_platform_header_offset));
			} else if (nvram_md5_support() && !nvram_evdo_support()) {
				*((short int *) & (FileTitleInfo2.iFileBufLen) - nvram_platform_header_offset)
				+= FileSize;
				NVBAK_LOG("iFileBufLen is (%d) !",
				          *((short int *) & (FileTitleInfo2.iFileBufLen) - nvram_platform_header_offset));
			} else if (!nvram_md5_support() && nvram_evdo_support()) {
				*((short int *) & (FileTitleInfo3.iFileBufLen) - nvram_platform_header_offset)
				+= FileSize;
				NVBAK_LOG("iFileBufLen is (%d) !",
				          *((short int *) & (FileTitleInfo3.iFileBufLen) - nvram_platform_header_offset));
			} else {
				*((short int *) & (FileTitleInfo4.iFileBufLen) - nvram_platform_header_offset)
				+= FileSize;
				NVBAK_LOG("iFileBufLen is (%d) !",
				          *((short int *) & (FileTitleInfo4.iFileBufLen) - nvram_platform_header_offset));
			}
		}


		switch (eBackupType) {
		case APBOOT:
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				FileTitleInfo1.iApBootNum = iFileNum;
				NVBAK_LOG("APBOOT num (%d) !", FileTitleInfo1.iApBootNum);
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo2.iApBootNum = iFileNum;
				NVBAK_LOG("APBOOT num (%d) !", FileTitleInfo2.iApBootNum);
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo3.iApBootNum = iFileNum;
				NVBAK_LOG("APBOOT num (%d) !", FileTitleInfo3.iApBootNum);
			} else if (!nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo4.iApBootNum = iFileNum;
				NVBAK_LOG("APBOOT num (%d) !", FileTitleInfo4.iApBootNum);
			} else {
				FileTitleInfo5.iApBootNum = iFileNum;
				NVBAK_LOG("APBOOT num (%d) !", FileTitleInfo5.iApBootNum);
			}
			break;

		case MDBOOT:
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				FileTitleInfo1.iMdBootNum = iFileNum;
				NVBAK_LOG("MDBOOT num (%d) !", FileTitleInfo1.iMdBootNum);
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo2.iMdBootNum = iFileNum;
				NVBAK_LOG("MDBOOT num (%d) !", FileTitleInfo2.iMdBootNum);
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo3.iMdBootNum = iFileNum;
				NVBAK_LOG("MDBOOT num (%d) !", FileTitleInfo3.iMdBootNum);
			} else if (!nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo4.iMdBootNum = iFileNum;
				NVBAK_LOG("MDBOOT num (%d) !", FileTitleInfo4.iMdBootNum);
			} else {
				FileTitleInfo5.iMdBootNum = iFileNum;
				NVBAK_LOG("MDBOOT num (%d) !", FileTitleInfo5.iMdBootNum);
			}
			break;

		case APCLN:
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				FileTitleInfo1.iApCleanNum = iFileNum;
				NVBAK_LOG("APCLN num (%d) !", FileTitleInfo1.iApCleanNum);
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo2.iApCleanNum = iFileNum;
				NVBAK_LOG("APCLN num (%d) !", FileTitleInfo2.iApCleanNum);
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo3.iApCleanNum = iFileNum;
				NVBAK_LOG("APCLN num (%d) !", FileTitleInfo3.iApCleanNum);
			} else if (!nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo4.iApCleanNum = iFileNum;
				NVBAK_LOG("APCLN num (%d) !", FileTitleInfo4.iApCleanNum);

			} else {
				FileTitleInfo5.iApCleanNum = iFileNum;
				NVBAK_LOG("APCLN num (%d) !", FileTitleInfo5.iApCleanNum);

			}
			break;

		case MDCLN:
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				FileTitleInfo1.iMdCleanNum = iFileNum;
				NVBAK_LOG("MDCLN num (%d) !", FileTitleInfo1.iMdCleanNum);
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo2.iMdCleanNum = iFileNum;
				NVBAK_LOG("MDCLN num (%d) !", FileTitleInfo2.iMdCleanNum);
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo3.iMdCleanNum = iFileNum;
				NVBAK_LOG("MDCLN num (%d) !", FileTitleInfo3.iMdCleanNum);
			} else if (!nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo4.iMdCleanNum = iFileNum;
				NVBAK_LOG("MDCLN num (%d) !", FileTitleInfo4.iMdCleanNum);
			} else {
				FileTitleInfo5.iMdCleanNum = iFileNum;
				NVBAK_LOG("MDCLN num (%d) !", FileTitleInfo5.iMdCleanNum);
			}
			break;

		case MDIMP:
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				FileTitleInfo1.iMdImpntNum = iFileNum;
				NVBAK_LOG("MDIMP num (%d) !", FileTitleInfo1.iMdImpntNum);
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo2.iMdImpntNum = iFileNum;
				NVBAK_LOG("MDIMP num (%d) !", FileTitleInfo2.iMdImpntNum);
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo3.iMdImpntNum = iFileNum;
				NVBAK_LOG("MDIMP num (%d) !", FileTitleInfo3.iMdImpntNum);
			} else if (!nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo4.iMdImpntNum = iFileNum;
				NVBAK_LOG("MDIMP num (%d) !", FileTitleInfo4.iMdImpntNum);
			} else {
				FileTitleInfo5.iMdImpntNum = iFileNum;
				NVBAK_LOG("MDIMP num (%d) !", FileTitleInfo5.iMdImpntNum);
			}
			break;

		case MDCOR:
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				FileTitleInfo1.iMdCoreNum = iFileNum;
				NVBAK_LOG("MDCOR num (%d) !", FileTitleInfo1.iMdCoreNum);
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo2.iMdCoreNum = iFileNum;
				NVBAK_LOG("MDCOR num (%d) !", FileTitleInfo2.iMdCoreNum);
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo3.iMdCoreNum = iFileNum;
				NVBAK_LOG("MDCOR num (%d) !", FileTitleInfo3.iMdCoreNum);
			} else if (!nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo4.iMdCoreNum = iFileNum;
				NVBAK_LOG("MDCOR num (%d) !", FileTitleInfo4.iMdCoreNum);
			} else {
				FileTitleInfo5.iMdCoreNum = iFileNum;
				NVBAK_LOG("MDCOR num (%d) !", FileTitleInfo5.iMdCoreNum);
			}
			break;

		case MDDATA:
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				FileTitleInfo1.iMdDataNum = iFileNum;
				NVBAK_LOG("MDDATA num (%d) !", FileTitleInfo1.iMdDataNum);
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo2.iMdDataNum = iFileNum;
				NVBAK_LOG("MDDATA num (%d) !", FileTitleInfo2.iMdDataNum);
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo3.iMdDataNum = iFileNum;
				NVBAK_LOG("MDDATA num (%d) !", FileTitleInfo3.iMdDataNum);
			} else if (!nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo4.iMdDataNum = iFileNum;
				NVBAK_LOG("MDDATA num (%d) !", FileTitleInfo4.iMdDataNum);
			} else {
				FileTitleInfo5.iMdDataNum = iFileNum;
				NVBAK_LOG("MDDATA num (%d) !", FileTitleInfo5.iMdDataNum);
			}
			break;
		//Add for second modem for MT658*
		case MD2BOOT:
			if (nvram_platform_header_offset == 0) {
				if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
					FileTitleInfo1.iMd2BootNum = iFileNum;
					NVBAK_LOG("MD2BOOT num (%d) !", FileTitleInfo1.iMd2BootNum);
				} else if (nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo2.iMd2BootNum = iFileNum;
					NVBAK_LOG("MD2BOOT num (%d) !", FileTitleInfo2.iMd2BootNum);
				} else if (!nvram_md5_support() && nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo3.iMd2BootNum = iFileNum;
					NVBAK_LOG("MD2BOOT num (%d) !", FileTitleInfo3.iMd2BootNum);
				} else if (!nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo4.iMd2BootNum = iFileNum;
					NVBAK_LOG("MD2BOOT num (%d) !", FileTitleInfo4.iMd2BootNum);
				} else {
					FileTitleInfo5.iMd2BootNum = iFileNum;
					NVBAK_LOG("MD2BOOT num (%d) !", FileTitleInfo5.iMd2BootNum);
				}
			}
			break;

		case MD2CLN:
			if (nvram_platform_header_offset == 0) {
				if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
					FileTitleInfo1.iMd2CleanNum = iFileNum;
					NVBAK_LOG("MD2CLN num (%d) !", FileTitleInfo1.iMd2CleanNum);
				} else if (nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo2.iMd2CleanNum = iFileNum;
					NVBAK_LOG("MD2CLN num (%d) !", FileTitleInfo2.iMd2CleanNum);
				} else if (!nvram_md5_support() && nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo3.iMd2CleanNum = iFileNum;
					NVBAK_LOG("MD2CLN num (%d) !", FileTitleInfo3.iMd2CleanNum);
				} else if (!nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo4.iMd2CleanNum = iFileNum;
					NVBAK_LOG("MD2CLN num (%d) !", FileTitleInfo4.iMd2CleanNum);
				} else {
					FileTitleInfo5.iMd2CleanNum = iFileNum;
					NVBAK_LOG("MD2CLN num (%d) !", FileTitleInfo5.iMd2CleanNum);
				}
			}
			break;

		case MD2IMP:
			if (nvram_platform_header_offset == 0) {
				if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
					FileTitleInfo1.iMd2ImpntNum = iFileNum;
					NVBAK_LOG("MD2IMP num (%d) !", FileTitleInfo1.iMd2ImpntNum);
				} else if (nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo2.iMd2ImpntNum = iFileNum;
					NVBAK_LOG("MD2IMP num (%d) !", FileTitleInfo2.iMd2ImpntNum);
				} else if (!nvram_md5_support() && nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo3.iMd2ImpntNum = iFileNum;
					NVBAK_LOG("MD2IMP num (%d) !", FileTitleInfo3.iMd2ImpntNum);
				} else if (!nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo4.iMd2ImpntNum = iFileNum;
					NVBAK_LOG("MD2IMP num (%d) !", FileTitleInfo4.iMd2ImpntNum);
				} else {
					FileTitleInfo5.iMd2ImpntNum = iFileNum;
					NVBAK_LOG("MD2IMP num (%d) !", FileTitleInfo5.iMd2ImpntNum);
				}
			}
			break;
		//End of Comment

		//LTE support
		case MD5BOOT:
			if (nvram_platform_header_offset == 0) {
				if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
					FileTitleInfo1.iMd5BootNum = iFileNum;
					NVBAK_LOG("MD5BOOT num (%d) !", FileTitleInfo1.iMd5BootNum);
				} else if (nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo2.iMd5BootNum = iFileNum;
					NVBAK_LOG("MD5BOOT num (%d) !", FileTitleInfo2.iMd5BootNum);
				} else if (!nvram_md5_support() && nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo3.iMd5BootNum = iFileNum;
					NVBAK_LOG("MD5BOOT num (%d) !", FileTitleInfo3.iMd5BootNum);
				} else if (!nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo4.iMd5BootNum = iFileNum;
					NVBAK_LOG("MD5BOOT num (%d) !", FileTitleInfo4.iMd5BootNum);
				} else {
					FileTitleInfo5.iMd5BootNum = iFileNum;
					NVBAK_LOG("MD5BOOT num (%d) !", FileTitleInfo5.iMd5BootNum);
				}
			}
			break;

		case MD5CLN:
			if (nvram_platform_header_offset == 0) {
				if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
					FileTitleInfo1.iMd5CleanNum = iFileNum;
					NVBAK_LOG("MD5CLN num (%d) !", FileTitleInfo1.iMd5CleanNum);
				} else if (nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo2.iMd5CleanNum = iFileNum;
					NVBAK_LOG("MD5CLN num (%d) !", FileTitleInfo2.iMd5CleanNum);
				} else if (!nvram_md5_support() && nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo3.iMd5CleanNum = iFileNum;
					NVBAK_LOG("MD5CLN num (%d) !", FileTitleInfo3.iMd5CleanNum);
				} else if (!nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo4.iMd5CleanNum = iFileNum;
					NVBAK_LOG("MD5CLN num (%d) !", FileTitleInfo4.iMd5CleanNum);
				} else {
					FileTitleInfo5.iMd5CleanNum = iFileNum;
					NVBAK_LOG("MD5CLN num (%d) !", FileTitleInfo5.iMd5CleanNum);
				}
			}
			break;

		case MD5IMP:
			if (nvram_platform_header_offset == 0) {
				if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support() ) {
					FileTitleInfo1.iMd5ImpntNum = iFileNum;
					NVBAK_LOG("MD5IMP num (%d) !", FileTitleInfo1.iMd5ImpntNum);
				} else if (nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo2.iMd5ImpntNum = iFileNum;
					NVBAK_LOG("MD5IMP num (%d) !", FileTitleInfo2.iMd5ImpntNum);
				} else if (!nvram_md5_support() && nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo3.iMd5ImpntNum = iFileNum;
					NVBAK_LOG("MD5IMP num (%d) !", FileTitleInfo3.iMd5ImpntNum);
				} else if (!nvram_md5_support() && !nvram_evdo_support()
				           && !nvram_ecci_c2k_support()) {
					FileTitleInfo4.iMd5ImpntNum = iFileNum;
					NVBAK_LOG("MD5IMP num (%d) !", FileTitleInfo4.iMd5ImpntNum);
				} else {
					FileTitleInfo5.iMd5ImpntNum = iFileNum;
					NVBAK_LOG("MD5IMP num (%d) !", FileTitleInfo5.iMd5ImpntNum);
				}
			}
			break;
		case VIA:
			if (nvram_md5_support() && nvram_evdo_support() && !nvram_ecci_c2k_support()) {
				FileTitleInfo1.iViaNum = iFileNum;
				NVBAK_LOG("VIA num (%d) !", FileTitleInfo1.iViaNum);
			} else if (nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo2.iViaNum = iFileNum;
				NVBAK_LOG("VIA num (%d) !", FileTitleInfo2.iViaNum);
			} else if (!nvram_md5_support() && nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo3.iViaNum = iFileNum;
				NVBAK_LOG("VIA num (%d) !", FileTitleInfo3.iViaNum);
			} else if (!nvram_md5_support() && !nvram_evdo_support()
			           && !nvram_ecci_c2k_support()) {
				FileTitleInfo4.iViaNum = iFileNum;
				NVBAK_LOG("VIA num (%d) !", FileTitleInfo4.iViaNum);
			}
			break;
		case MD3BOOT:
			if (nvram_ecci_c2k_support()) {
				FileTitleInfo5.iMd3BootNum = iFileNum;
				NVBAK_LOG("iMd3BootNum num (%d) !", FileTitleInfo5.iMd3BootNum);
			}
			break;
		case MD3CLN:
			if (nvram_ecci_c2k_support()) {
				FileTitleInfo5.iMd3CleanNum = iFileNum;
				NVBAK_LOG("iMd3CleanNum num (%d) !", FileTitleInfo5.iMd3CleanNum);
			}
			break;
		case MD3IMP:
			if (nvram_ecci_c2k_support()) {
				FileTitleInfo5.iMd3ImpntNum = iFileNum;
				NVBAK_LOG("iMd3ImpntNum num (%d) !", FileTitleInfo5.iMd3ImpntNum);
			}
			break;
		case ALL:
		default:
			return false;
			break;
		}
		iFileNum = 0;
		FileSize = 0;
	}

	iFileDesc_map = open(g_pcNVM_AllMap, O_WRONLY);

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create map data\n");
		return false;
	}
	write(iFileDesc_map, &ulCheckSum, sizeof(unsigned int));
	write(iFileDesc_map, &iOldCommonFileNum, sizeof(unsigned int));
	write(iFileDesc_map, &iOldCustomFileNum, sizeof(unsigned int));
	if (!nvram_ecci_c2k_support()) {
		if (nvram_platform_header_offset == 0 && nvram_md5_support()
		        && nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo1, sizeof(FileTitleInfo1));
		else if (nvram_platform_header_offset == 0 && nvram_md5_support()
		         && !nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo2,
			      sizeof(FileTitleInfo2) - 2 * sizeof(short
			              int));//In order to align, minus 2*sizeof(short int)
		else if (nvram_platform_header_offset == 0 && !nvram_md5_support()
		         && nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo3,
			      sizeof(FileTitleInfo3) - 4 * sizeof(short
			              int));//In order to align, minus 4*sizeof(short int)
		else if (nvram_platform_header_offset == 0 && !nvram_md5_support()
		         && !nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo4,
			      sizeof(FileTitleInfo4) - 4 * sizeof(short int));
		else if (nvram_platform_header_offset != 0 && nvram_md5_support()
		         && nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo1,
			      sizeof(FileTitleInfo1) - nvram_platform_header_offset * 2);
		else if (nvram_platform_header_offset != 0 && nvram_md5_support()
		         && !nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo2,
			      sizeof(FileTitleInfo2) - 2 * sizeof(short int) - nvram_platform_header_offset *
			      2);//In order to align, minus 2*sizeof(short int)
		else if (nvram_platform_header_offset != 0 && !nvram_md5_support()
		         && nvram_evdo_support())
			write(iFileDesc_map, &FileTitleInfo3,
			      sizeof(FileTitleInfo3) - 4 * sizeof(short int) - nvram_platform_header_offset *
			      2);//In order to align, minus 4*sizeof(short int)
		else
			write(iFileDesc_map, &FileTitleInfo4,
			      sizeof(FileTitleInfo4) - 4 * sizeof(short int) - nvram_platform_header_offset *
			      2);
	} else
		write(iFileDesc_map, &FileTitleInfo5, sizeof(FileTitleInfo5));

	close(iFileDesc_map);
	iFileDesc_map = open(g_pcNVM_AllMap, O_RDONLY);
	memset(&FileTitleInfo1, 0, sizeof(File_Title_Header1));
	memset(&FileTitleInfo2, 0, sizeof(File_Title_Header2));
	memset(&FileTitleInfo3, 0, sizeof(File_Title_Header3));
	memset(&FileTitleInfo4, 0, sizeof(File_Title_Header4));
    if(INVALID_HANDLE_VALUE == iFileDesc_map)
    {
        /* Error handling */
        NVBAK_LOG("FileOp_MakeFile cannot open/create map data\n");
        return false;
    }
	iResult = lseek(iFileDesc_map, sizeof(unsigned int) + 2 * sizeof(unsigned int),
	                SEEK_SET);
	if (!nvram_ecci_c2k_support()) {

		if (nvram_platform_header_offset == 0 && nvram_md5_support()
		        && nvram_evdo_support())
			read(iFileDesc_map, &FileTitleInfo1, sizeof(FileTitleInfo1));
		else if (nvram_platform_header_offset == 0 && nvram_md5_support()
		         && !nvram_evdo_support())
			read(iFileDesc_map, &FileTitleInfo2,
			     sizeof(FileTitleInfo2) - 2 * sizeof(short
			             int));//In order to align, minus 2*sizeof(short int)
		else if (nvram_platform_header_offset == 0 && !nvram_md5_support()
		         && nvram_evdo_support())
			read(iFileDesc_map, &FileTitleInfo3,
			     sizeof(FileTitleInfo3) - 4 * sizeof(short
			             int));//In order to align, minus 4*sizeof(short int)
		else if (nvram_platform_header_offset == 0 && !nvram_md5_support()
		         && !nvram_evdo_support())
			read(iFileDesc_map, &FileTitleInfo4,
			     sizeof(FileTitleInfo4) - 4 * sizeof(short int));
		else if (nvram_platform_header_offset != 0 && nvram_md5_support()
		         && nvram_evdo_support())
			read(iFileDesc_map, &FileTitleInfo1,
			     sizeof(FileTitleInfo1) - nvram_platform_header_offset * 2);
		else if (nvram_platform_header_offset != 0 && nvram_md5_support()
		         && !nvram_evdo_support())
			read(iFileDesc_map, &FileTitleInfo2,
			     sizeof(FileTitleInfo2) - 2 * sizeof(short int) - nvram_platform_header_offset *
			     2);//In order to align, minus 2*sizeof(short int)
		else if (nvram_platform_header_offset != 0 && !nvram_md5_support()
		         && nvram_evdo_support())
			read(iFileDesc_map, &FileTitleInfo3,
			     sizeof(FileTitleInfo3) - 4 * sizeof(short int) - nvram_platform_header_offset *
			     2);//In order to align, minus 4*sizeof(short int)
		else
			read(iFileDesc_map, &FileTitleInfo4,
			     sizeof(FileTitleInfo4) - 4 * sizeof(short int) - nvram_platform_header_offset *
			     2);
	} else
		read(iFileDesc_map, &FileTitleInfo5, sizeof(FileTitleInfo5));
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

	close(iFileDesc_map);

	ulCheckSum = FileOp_ComputeCheckSum();
	NVBAK_LOG("FileOp_ComputeCheckSum:%lx", ulCheckSum);
	if (!FileOp_SetCheckSum(ulCheckSum)) {
		NVBAK_LOG("FileOp_SetCheckSum Fail !");
		return false;
	}
	NVBAK_LOG("FileOp_BackupData_All end !");
	return bRet;
}


bool FileOp_BackupData_Special(char * buffer, int count , int mode) {
	int iLen = 0, iTmpStart = 0, iNum = 0, eBackupType, i;
	int iFolderNo;
	bool bResult;

	char strFolder[ALL][10];
	snprintf(strFolder[0], sizeof(strFolder[0]),"%s", "APRDEB");
	snprintf(strFolder[1], sizeof(strFolder[1]),"%s", "APRDCL");
	snprintf(strFolder[2], sizeof(strFolder[2]),"%s", "NVD_IMEI");
	snprintf(strFolder[3], sizeof(strFolder[3]),"%s", "CALIBRAT");
	snprintf(strFolder[4], sizeof(strFolder[4]),"%s", "IMPORTNT");
	snprintf(strFolder[5], sizeof(strFolder[5]),"%s", "NVD_CORE");
	snprintf(strFolder[6], sizeof(strFolder[6]),"%s", "NVD_DATA");


	if (buffer == NULL) {
		NVBAK_LOG("BackupData_Special:The pointer of buffer is NULL!");
		return false;
	}
	NVBAK_LOG("buffer:%s", buffer);

	if ( count < 0 || count >= 1024 || mode < 0 ) {
		NVBAK_LOG("BackupData_Special: the value of count and mode is wrong!");
		return false;
	}

	int iMask[ALL];
	memset(iMask, 0, (ALL * (sizeof(int))));
	char tempBuf[count][(1024 / count) + 1];
	iLen = strlen(buffer);

	for (i = 0; i < iLen; i++) {
		if (buffer[i] == ';') {
			memcpy(tempBuf[iNum], buffer + iTmpStart, i - iTmpStart);
			tempBuf[iNum][i - iTmpStart] = '\0';
			iTmpStart += (i - iTmpStart + 1);
			iNum++;
		}
	}
	if (iNum != count) {
		NVBAK_LOG("BackupData_Special: the num of real string doesn't match with the value of count !");
		return false;
	}
	for (i = 0; i < iNum; i++) {
		for (iFolderNo = 0; iFolderNo < ALL; iFolderNo++) {
			if (0 == strcmp(tempBuf[i], strFolder[iFolderNo])) {
				iMask[iFolderNo] = 1;
				break;
			}
		}
	}
	NVBAK_LOG("iMask:%d,%d,%d,%d,%d,%d,%d\n", iMask[0], iMask[1], iMask[2],
	          iMask[3], iMask[4], iMask[5], iMask[6]);
	bool bWorkForBinRegionBackup = false;
	bResult = FileOp_BackupDataToFiles(iMask, bWorkForBinRegionBackup);
	return bResult;
}

//-----------------------------------------------------------------------------
bool FileOp_RestoreData_All(void) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	const char* lpDestDirName = NULL;
	char acSrcPathName[MAX_NAMESIZE];
	char acDstPathName[MAX_NAMESIZE];
	bool bRet = true;
	int iResult, eBackupType;
	unsigned int ulSavedCheckSum;
	unsigned int ulCheckSum;
	struct stat st;
	unsigned int iFileSize;
	int error_flag = 0;
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

	NVBAK_LOG("[FileOp_RestoreData_All] start !");

	if (!FileOp_GetCheckSum()) {
		NVBAK_LOG("[FileOp_RestoreData_All] GetCheckSum Fail !");
		return false;
	}
	ulSavedCheckSum = stBackupFileInfo.ulCheckSum;
	ulCheckSum = FileOp_ComputeCheckSum();
	NVBAK_LOG("ulCheckSun:%ld\n", ulCheckSum);
	if (stat(g_pcNVM_AllFile, &st) < 0) {
		NVBAK_LOG("Error FileOp_RestoreData stat \n");
		return false;
	}
	iFileSize = st.st_size;
	NVBAK_LOG("iFileSize:%d\n", iFileSize);
	ulCheckSum ^= iFileSize;

	NVBAK_LOG("FileOp_CheckSum:%lx,%lx", ulSavedCheckSum, ulCheckSum);
	if (!FileOp_CompareCheckSum(ulSavedCheckSum, ulCheckSum)) {
		NVBAK_LOG("check sum not match!");
		return false;
	}
	for (eBackupType = APBOOT; eBackupType < ALL; eBackupType++) {
		if (iFileMask[eBackupType] == 1)
			continue;
		bRet = FileOp_RestoreFromFiles(eBackupType);
		if (bRet == false) {
			NVBAK_LOG("RestoreFromFiles Error!! The No.%d file", eBackupType);
			error_flag = 1;
		}
	}
	if (error_flag == 0) {
		NVBAK_LOG("[FileOp_RestoreData_All] end !");
		return true;
	} else {
		NVBAK_LOG("[FileOp_RestoreData_All] some file restore failed !");
		return false;
	}

}

bool FileOp_BackupAll_NvRam(void) {
	bool bRet = true;
	bool bWorkForBinRegionBackup = false;
	NVBAK_LOG("FileOp_BackupAll_NvRam Start !");

	int iFileMask[ALL];
	//need init ifilemark value ,otherwize it cause backup fail on 6592+kk1.mp3+non lte project
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
	if (!(nvram_ecci_c2k_support())) {
		iFileMask[14] = 1;
		iFileMask[15] = 1;
		iFileMask[16] = 1;
	}

	bRet = FileOp_BackupDataToFiles(iFileMask, bWorkForBinRegionBackup);

	NVBAK_LOG("FileOp_BackupAll_NvRam End !");

	return bRet;
}

bool FileOp_RestoreAll_NvRam(void) {
	bool bRet = true;

	NVBAK_LOG("[FileOp_RestoreAll_NvRam] Start !");

	bRet = FileOp_RestoreData_All();

	NVBAK_LOG("[FileOp_RestoreAll_NvRam] End !");

	if (bRet == true) {
		bRet = NVM_DataVerConvertAll(stBackupFileInfo.iCommonFileNum,
		                             stBackupFileInfo.iCustomFileNum);
		if (bRet != true)
			NVBAK_LOG("[FileOp_RestoreAll_NvRam] convert data fail !");

	}
	return bRet;
}

static int iDMFlag = 0x12121212;
char *strDMFileFolderPath = "/mnt/vendor/nvdata/dm";
bool FileOp_BackupToBinRegionForDM() {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	struct stat st;
	char acSrcPathName[MAX_NAMESIZE];
	char *tempBuf = NULL;
	//char *strDMFileFolderPath = "/mnt/vendor/nvdata/dm";
	char *strUSBFilePath = "/mnt/vendor/nvdata/APCFG/APRDEB/OMADM_USB";
	int iFileDesc_map, iFileDesc_file;
	int iFileSize, iFileNum, iResult, fd, iMapFileSize, iDatFileSize;

	iFileSize = 0;
	iFileNum = 0;
	/* Create the map file */
	NVBAK_LOG("enter backup to binregion for DM\n");
	//fix nvram layout issue , reassgined block numerbers by different platform
	if (NVM_Init() < 0) {
		NVBAK_LOG("FileOp_BackupToBinRegionForDM--NVM_Init fail !!!\n");
		return false;
	}

	iFileDesc_map = open(g_pcNVM_AllMap, O_TRUNC | O_CREAT | O_RDWR,
	                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create map data\n");
		return false;
	}
	iResult = write(iFileDesc_map, &iFileNum,
	                sizeof(int)); //alloct the space for iFileNum

	iFileDesc_file = open(g_pcNVM_AllFile , O_TRUNC | O_CREAT | O_RDWR,
	                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create data file\n");
		close(iFileDesc_map);
		return false;
	}

	dir = opendir(strDMFileFolderPath);
	if (dir == NULL) {
		NVBAK_LOG("FileOp_BackupData open dir: %s ,this dir not exist!",
		          strDMFileFolderPath);
		close(iFileDesc_map);
		close(iFileDesc_file);
		return false;
	}

	while (1) {
		entry = readdir(dir);

		if (entry == NULL) {
			break;
		}

		lstat(entry->d_name, &statbuf);

		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
			continue;
		}

		sprintf(acSrcPathName, "%s/%s", strDMFileFolderPath, entry->d_name);
		NVBAK_LOG("FileOp_BackupData (%s)\n", acSrcPathName);
		iFileNum++;
		if (stat(acSrcPathName, &st) < 0) {
			NVBAK_LOG("Error FileOp_RestoreData stat \n");
			closedir(dir);
			close(iFileDesc_map);
			close(iFileDesc_file);
			return false;
		}
		iFileSize = st.st_size;
		tempBuf = (char*)malloc(iFileSize);
		if (tempBuf == NULL) {
			NVBAK_LOG("Error malloc\n");
			closedir(dir);
			close(iFileDesc_map);
			close(iFileDesc_file);
			return false;
		}
		iResult = write(iFileDesc_map, acSrcPathName, MAX_NAMESIZE);
		if (iResult != MAX_NAMESIZE) {
			NVBAK_LOG("Error FileOp_BackupSpecialFile:write1:%d\n", iResult);
			closedir(dir);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(tempBuf);
			return false;
		}

		iResult = write(iFileDesc_map, &iFileSize, sizeof(int));
		if (iResult != sizeof(int)) {
			NVBAK_LOG("Error FileOp_BackupSpecialFile:write2:%d\n", iResult);
			closedir(dir);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(tempBuf);
			return false;
		}
		fd = open(acSrcPathName , O_RDONLY);
		if (INVALID_HANDLE_VALUE == fd) {
			/* Error handling */
			NVBAK_LOG("FileOp_MakeFile cannot open/create data file\n");
			closedir(dir);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(tempBuf);
			return false;
		}

		iResult = read(fd, tempBuf, iFileSize);

		if (iResult != iFileSize) {
			NVBAK_LOG("Error FileOp_BackupSpecialFile:read1:%d\n", iResult);
			closedir(dir);
			close(iFileDesc_map);
			close(iFileDesc_file);
			close(fd);
			free(tempBuf);
			return false;
		}
		/*
		if(strcmp(acSrcPathName,"/mnt/vendor/nvdata/APCFG/APRDEB/BT_Addr")==0)
			{
				int m;
				for(m=0;m<8;m++)
				 NVBAK_LOG("capid:%d\n",*(char*)(tempBuf+m));
			}*/
		iResult = write(iFileDesc_file, tempBuf, iFileSize);
		if (iResult != iFileSize) {
			NVBAK_LOG("Error FileOp_BackupSpecialFile:write dat:%d\n", iResult);
			closedir(dir);
			close(iFileDesc_map);
			close(iFileDesc_file);
			close(fd);
			free(tempBuf);
			return false;
		}
		/*
		if(strcmp(acSrcPathName,"/mnt/vendor/nvdata/APCFG/APRDEB/BT_Addr")==0)
		{
			int m;
			for(m=0;m<8;m++)
			 NVBAK_LOG("capid:%d\n",*(char*)(tempBuf+m));
		}*/
		close(fd);
		free(tempBuf);
		tempBuf = NULL;
	}
	closedir(dir);
	//USB File
	if (stat(strUSBFilePath, &st) >= 0) {
		NVBAK_LOG("FileOp_RestoreData stat:%s \n",strUSBFilePath);
		iFileSize = st.st_size;
		memset(acSrcPathName, 0, MAX_NAMESIZE);
		memcpy(acSrcPathName, strUSBFilePath, strlen(strUSBFilePath) + 1);
		NVBAK_LOG("USB file:%s\n", acSrcPathName);

		iResult = write(iFileDesc_map, acSrcPathName, MAX_NAMESIZE);
		iResult = write(iFileDesc_map, &iFileSize, sizeof(int));
		iFileNum++;
		/*lseek(iFileDesc_map, 0, SEEK_SET);
		iResult = write(iFileDesc_map, &iFileNum, sizeof(int));*/
		tempBuf = (char*)malloc(iFileSize);
		if (tempBuf == NULL) {
			NVBAK_LOG("Error malloc2\n");
			close(iFileDesc_map);
			close(iFileDesc_file);
			return false;
		}
		fd = open(strUSBFilePath , O_RDONLY);
		if (INVALID_HANDLE_VALUE == fd) {
			/* Error handling */
			NVBAK_LOG("FileOp_MakeFile cannot open/create data file\n");
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(tempBuf);
			return false;
		}

		iResult = read(fd, tempBuf, iFileSize);
		if (iResult != iFileSize) {
			NVBAK_LOG("Error FileOp_BackupSpecialFile:read1:%d\n", iResult);
			close(iFileDesc_map);
			close(iFileDesc_file);
			close(fd);
			free(tempBuf);
			return false;
		}

		iResult = write(iFileDesc_file, tempBuf, iFileSize);
		if (iResult != iFileSize) {
			NVBAK_LOG("Error FileOp_BackupSpecialFile:write dat:%d\n", iResult);
			close(iFileDesc_map);
			close(iFileDesc_file);
			close(fd);
			free(tempBuf);
			return false;
		}
	}
	lseek(iFileDesc_map, 0, SEEK_SET);
	iResult = write(iFileDesc_map, &iFileNum, sizeof(int));
	NVBAK_LOG("Write filenum(%d) to ALLMAP\n", iFileNum);
	close(fd);
	free(tempBuf);
	tempBuf = NULL;
	close(iFileDesc_file);
	close(iFileDesc_map);

	//backup to bin region
	stat(g_pcNVM_AllMap, &st);
	iMapFileSize = st.st_size;
	stat(g_pcNVM_AllFile, &st);
	iDatFileSize = st.st_size;

	int iWriteSize, iBlockSize, iPartitionSize, i, pos = 0;
	char cMtdDevName[128] = {0};
	char *bDMBitmap = NULL;
	struct mtd_info_user info;
	struct erase_info_user erase_info;
//    int iBackup_Partition=get_partition_numb("nvram");
	memset(cMtdDevName, 0, sizeof cMtdDevName);
	//strcpy(cMtdDevName, g_NVRAM_BACKUP_DEVICE);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);
	NVBAK_LOG("[NVRAM]:cMtdDevName:%s\n", cMtdDevName);


	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_BackupToBinRegionForDM get device info error\r\n");
		return false;
	}

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		return false;
	}

	iWriteSize = info.writesize;
	iBlockSize = info.erasesize;
	iPartitionSize = info.size;


	if (nvram_platform_layout_version == 0)
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

	bDMBitmap = (char*)malloc(BinRegionBlockTotalNum - NvRamBlockNum);
	if (bDMBitmap == NULL) {
		NVBAK_LOG("[NVRAM]:malloc bDMBitmap Failed!\n");
		close(fd);
		return false;
	}
	memset(bDMBitmap, 0, ((BinRegionBlockTotalNum - NvRamBlockNum)*sizeof(char)));
	NVBAK_LOG("[NVRAM]:mtd get info:iBlockSize:%d,iPartitionSize:%d\n", iBlockSize,
	          iPartitionSize);
	//check whether the  block is a bad block or not
	for (i = (BinRegionBlockTotalNum - 1); i >= NvRamBlockNum; i--) {
		erase_info.start = i * iBlockSize;
		erase_info.length = iBlockSize;

		if ((!nvram_emmc_support()) && (!nvram_ufs_support())) {

			if (!NVM_EraseDeviceBlock(cMtdDevName, erase_info)) {
				NVBAK_LOG("[NVRAM]:mtd erase error,block id:%d\r\n", i);
				bDMBitmap[i - NvRamBlockNum] = 1;
			} else {
				bDMBitmap[i - NvRamBlockNum] = 0;
				pos++;
			}
		} else
			pos++;
	}
	NVBAK_LOG("[NVRAM]DM the available block num:%d\n", pos);
	if ((int)(iMapFileSize + iDatFileSize + 3 * sizeof(int) +
	          (BinRegionBlockTotalNum - NvRamBlockNum)*sizeof(char)) >= (pos * iBlockSize)) {
		NVBAK_LOG("[NVRAM]DM File Size is more than the available block size \n");
		close(fd);
		free(bDMBitmap);
		return false;
	}
	tempBuf = (char*)malloc(pos * iBlockSize);
	if (tempBuf == NULL) {
		NVBAK_LOG("[NVRAM]DM malloc error \n");
		close(fd);
		free(bDMBitmap);
		return false;
	}
	memset(tempBuf, 0xFF, pos * iBlockSize);
	memcpy(tempBuf, &iDMFlag, sizeof(int));
	memcpy(tempBuf + sizeof(int), &iMapFileSize, sizeof(int));
	memcpy(tempBuf + 2 * sizeof(int), &iDatFileSize, sizeof(int));
	memcpy(tempBuf + 3 * sizeof(int), bDMBitmap,
	       (BinRegionBlockTotalNum - NvRamBlockNum)*sizeof(char));
	iFileDesc_map = open(g_pcNVM_AllMap, O_RDONLY);
	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		/* Error handling */
		NVBAK_LOG("DM FileOp_MakeFile cannot open map data\n");
		close(fd);
		free(tempBuf);
		free(bDMBitmap);
		return false;
	}
	iFileDesc_file = open(g_pcNVM_AllFile , O_RDONLY);
	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		/* Error handling */
		NVBAK_LOG("DM FileOp_MakeFile cannot open data file\n");
		close(fd);
		free(tempBuf);
		close(iFileDesc_map);
		free(bDMBitmap);
		return false;
	}
	iResult = read(iFileDesc_map,
	               tempBuf + 3 * sizeof(int) + (BinRegionBlockTotalNum - NvRamBlockNum) * sizeof(
	                   char), iMapFileSize);
	if (iResult != iMapFileSize) {
		NVBAK_LOG("DM FileOp_MakeFile cannot read map file:%d\n", iResult);
		close(fd);
		free(tempBuf);
		close(iFileDesc_map);
		close(iFileDesc_file);
		free(bDMBitmap);
		return false;
	}

	iResult = read(iFileDesc_file,
	               tempBuf + 3 * sizeof(int) + (BinRegionBlockTotalNum - NvRamBlockNum) * sizeof(
	                   char) + iMapFileSize, iDatFileSize);
	if (iResult != iDatFileSize) {
		NVBAK_LOG("DM FileOp_MakeFile cannot read map file:%d\n", iResult);
		close(fd);
		free(tempBuf);
		close(iFileDesc_map);
		close(iFileDesc_file);
		free(bDMBitmap);
		return false;
	}
	close(iFileDesc_map);
	close(iFileDesc_file);

	//write file into bin region
	iWriteSize = 0;
	if ((int)(3 * sizeof(int) + (BinRegionBlockTotalNum - NvRamBlockNum)*sizeof(
	              char) + iMapFileSize + iDatFileSize) > iBlockSize) {
		int offset = 0;
		for (pos = NvRamBlockNum; pos < BinRegionBlockTotalNum; pos++) {
			if (bDMBitmap[pos - NvRamBlockNum] == 0) {
				lseek(fd, pos * iBlockSize, SEEK_SET);
				iResult = write(fd, tempBuf + offset * iBlockSize, iBlockSize);
				if (iResult != iBlockSize) {
					NVBAK_LOG("DM FileOp_MakeFile cannot write file into bin region :%d,%d\n",
					          iResult, pos);
					close(fd);
					free(tempBuf);
					free(bDMBitmap);
					return false;
				}
				offset++;
				iWriteSize += iBlockSize;
				if (iWriteSize >= (int)(3 * sizeof(int) + (BinRegionBlockTotalNum -
				                        NvRamBlockNum)*sizeof(char) + iMapFileSize + iDatFileSize))
					break;
			} else
				continue;

		}

	} else {
		for (pos = NvRamBlockNum; pos < BinRegionBlockTotalNum; pos++) {
			if (bDMBitmap[pos - NvRamBlockNum] == 0) {
				lseek(fd, pos * iBlockSize, SEEK_SET);
				iResult = write(fd, tempBuf, iBlockSize);
				if (iResult != iBlockSize) {
					NVBAK_LOG("DM FileOp_MakeFile cannot write file into bin region :%d,%d\n",
					          iResult, pos);
					close(fd);
					free(tempBuf);
					free(bDMBitmap);
					return false;
				} else
					break;
			} else
				continue;
		}

	}

	NVBAK_LOG("BackupToBinRegion for DM success\n");
	close(fd);
	free(tempBuf);
	free(bDMBitmap);
	return true;
}

bool FileOp_RestoreFromBinRegionForDM() {
	char acSrcPathName[MAX_NAMESIZE];
	int iFileDesc_file, iFileDesc_map, fd, iResult, iBlockNum, iFileDesc;
	int iMapFileSize, iDatFileSize, iBlockSize, i, iDMBackupFlag, iFileNum,
	    iFileSize;

	char cMtdDevName[128] = {0};
	char *bDMBitmap = NULL;
	char *tempBuffer = NULL;
	char *tempBuffer1 = NULL;
	struct mtd_info_user info;
	bool bFound = false;
	// int iBackup_Partition=get_partition_numb("nvram");
	NVBAK_LOG("enter restore from binregion for DM\n");
	//fix nvram layout issue , reassgined block numerbers by different platform
	if (NVM_Init() < 0) {
		NVBAK_LOG("[NVRAM]:FileOp_RestoreFromBinRegionForDM--NVM_Init fail !!!\n");
		return false;
	}
	memset(cMtdDevName, 0, sizeof cMtdDevName);
	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);
	if (!NVM_GetDeviceInfo(cMtdDevName, &info)) {
		NVBAK_LOG("[NVRAM]:FileOp_RestoreFromBinRegionForDM get device info error\r\n");
		return false;
	}

	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		return false;
	}


	iBlockSize = info.erasesize;
	//iPartitionSize=info.size;

	if (nvram_platform_layout_version == 0)
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

	bDMBitmap = (char*)malloc(BinRegionBlockTotalNum - NvRamBlockNum);
	if (bDMBitmap == NULL) {
		NVBAK_LOG("[NVRAM]:malloc bDMBitmap Failed!\n");
		close(fd);
		return false;
	}
	tempBuffer = (char*)malloc((BinRegionBlockTotalNum - NvRamBlockNum) *
	                           iBlockSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		free(bDMBitmap);
		return false;
	}
	int offset = 0;
	for (i = NvRamBlockNum; i < BinRegionBlockTotalNum; i++) {
		lseek(fd, i * iBlockSize, SEEK_SET);
		iResult = read(fd, tempBuffer, iBlockSize);
		NVBAK_LOG("[NVRAM]:read:%d\n", iResult);
		if (iResult <= 0) {
			NVBAK_LOG("[NVRAM]:read size error\r\n");
			close(fd);
			free(tempBuffer);
			free(bDMBitmap);
			return false;
		}
		memcpy(&iDMBackupFlag, tempBuffer, sizeof(int));
		if (iDMBackupFlag == iDMFlag) {
			NVBAK_LOG("[NVRAM]:DM bin region found:%d\n", i);
			bFound = true;
		} else
			continue;
		memcpy(&iMapFileSize, tempBuffer + sizeof(int), sizeof(int));
		memcpy(&iDatFileSize, tempBuffer + 2 * sizeof(int), sizeof(int));
		memcpy(bDMBitmap, tempBuffer + 3 * sizeof(int),
		       (BinRegionBlockTotalNum - NvRamBlockNum)*sizeof(char));
		if ((int)(iMapFileSize + iDatFileSize + 3 * sizeof(int) +
		          (BinRegionBlockTotalNum - NvRamBlockNum)*sizeof(char)) > iBlockSize) {
			int j, iReadSize = iBlockSize;
			for (j = i - NvRamBlockNum + 1; j < BinRegionBlockTotalNum - NvRamBlockNum;
			        j++) {
				if (bDMBitmap[j] == 0) {
					lseek(fd, (j + NvRamBlockNum)*iBlockSize, SEEK_SET);
					iResult = read(fd, tempBuffer + iReadSize, iBlockSize);
					NVBAK_LOG("[NVRAM]:read:%d\n", iResult);
					if (iResult <= 0) {
						NVBAK_LOG("[NVRAM]:read size error\r\n");
						close(fd);
						free(tempBuffer);
						free(bDMBitmap);
						return false;
					}
					iReadSize += iBlockSize;
					if (iReadSize >= (int)(iMapFileSize + iDatFileSize + 3 * sizeof(int) +
					                       (BinRegionBlockTotalNum - NvRamBlockNum)*sizeof(char))) {
						NVBAK_LOG("[NVRAM]:All DM File read from bin region\n");
						break;
					}
				}
			}
			break;
		} else
			break;
	}
	if (!bFound) {
		NVBAK_LOG("[NVRAM]:not find DM file in bin region\n");
		close(fd);
		free(tempBuffer);
		free(bDMBitmap);
		return false;
	}
	free(bDMBitmap);
	iFileDesc_file = open(g_pcNVM_AllFile , O_TRUNC | O_CREAT | O_RDWR,
	                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	iFileDesc_map = open(g_pcNVM_AllMap, O_TRUNC | O_CREAT | O_RDWR,
	                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	if (INVALID_HANDLE_VALUE == iFileDesc_file) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create file data\n");
		free(tempBuffer);
		close(fd);
		if (iFileDesc_map != INVALID_HANDLE_VALUE)
			close(iFileDesc_map);
		return false;
	}

	if (INVALID_HANDLE_VALUE == iFileDesc_map) {
		/* Error handling */
		NVBAK_LOG("FileOp_MakeFile cannot open/create map data\n");
		close(fd);
		free(tempBuffer);
		close(iFileDesc_file);
		return false;
	}
	if (iMapFileSize<0 || iMapFileSize>=INT_MAX) {
		NVBAK_LOG("iMapFileSize overflow\n");
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		close(fd);
		return false;
	}
	if (iDatFileSize<0 || iDatFileSize>=INT_MAX) {
		NVBAK_LOG("iDatFileSize overflow\n");
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		close(fd);
		return false;
	}	
	iResult = write(iFileDesc_map,
	                tempBuffer + 3 * sizeof(int) + (BinRegionBlockTotalNum - NvRamBlockNum) *
	                sizeof(char), iMapFileSize);
	if (iResult != iMapFileSize) {
		NVBAK_LOG("write map file fail:%d\n", iResult);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		close(fd);
		return false;
	}

	iResult = write(iFileDesc_file,
	                tempBuffer + 3 * sizeof(int) + (BinRegionBlockTotalNum - NvRamBlockNum) *
	                sizeof(char) + iMapFileSize, iDatFileSize);
	if (iResult != iDatFileSize) {
		NVBAK_LOG("write dat file fail:%d\n", iResult);
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		close(fd);
		return false;
	}
	free(tempBuffer);
	close(iFileDesc_file);
	close(iFileDesc_map);
	close(fd);

	//creat files based on the temp file
	iFileDesc_file = open(g_pcNVM_AllFile , O_RDONLY);
	iFileDesc_map = open(g_pcNVM_AllMap, O_RDONLY);
    if(INVALID_HANDLE_VALUE == iFileDesc_file)
    {
        /* Error handling */
        NVBAK_LOG("FileOp_MakeFile cannot open file data\n");
			if(iFileDesc_map != INVALID_HANDLE_VALUE)
				close(iFileDesc_map);
        return false;
    }

	if(INVALID_HANDLE_VALUE == iFileDesc_map)
    {
        /* Error handling */
        NVBAK_LOG("FileOp_MakeFile cannot open map data\n");
		close(iFileDesc_file);
        return false;
    }
	tempBuffer = (char*)malloc(iMapFileSize);
	if (tempBuffer == NULL) {
		NVBAK_LOG("malloc tempBuffer fail!\n");
		close(iFileDesc_file);
		close(iFileDesc_map);
		return false;
	}
	tempBuffer1 = (char*)malloc(iDatFileSize);
	if (tempBuffer1 == NULL) {
		NVBAK_LOG("malloc tempBuffer1 fail!\n");
		free(tempBuffer);
		close(iFileDesc_file);
		close(iFileDesc_map);
		return false;
	}
	iResult = read(iFileDesc_map, tempBuffer, iMapFileSize);
	if (iResult != iMapFileSize) {
		NVBAK_LOG("read map file fail:%d\n", iResult);
		free(tempBuffer);
		free(tempBuffer1);
		close(iFileDesc_file);
		close(iFileDesc_map);
		return false;
	}
	iResult = read(iFileDesc_file, tempBuffer1, iDatFileSize);
	if (iResult != iDatFileSize) {
		NVBAK_LOG("read dat file fail:%d\n", iResult);
		free(tempBuffer);
		free(tempBuffer1);
		close(iFileDesc_file);
		close(iFileDesc_map);
		return false;
	}
	iFileNum = 0;
	memcpy(&iFileNum, tempBuffer, sizeof(int));
	offset = 0;
	i = 0;
	NVBAK_LOG("dat file num:%d\n", iFileNum);
	while (iFileNum > 0 && iFileNum <= INT_MAX) {
		memcpy(acSrcPathName, tempBuffer + sizeof(int) + i * (MAX_NAMESIZE + sizeof(
		            int)), MAX_NAMESIZE);
		NVBAK_LOG("dm file:%s\n", acSrcPathName);
		memcpy(&iFileSize, tempBuffer + sizeof(int) + i * (MAX_NAMESIZE + sizeof(
		            int)) + MAX_NAMESIZE, sizeof(int));
		NVBAK_LOG("file size:%d\n", iFileSize);
		if (iFileSize <= 0 || iFileSize >= INT_MAX) {
			/* Error handling */
			NVBAK_LOG("FileOp_RestoreFromFilesForDM cannot create %s\n", acSrcPathName);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(tempBuffer);
			free(tempBuffer1);
			return false;
		}
		iFileDesc = open_file_with_dirs(acSrcPathName, 0660);
		if (INVALID_HANDLE_VALUE == iFileDesc) {
			/* Error handling */
			NVBAK_LOG("FileOp_RestoreFromFilesForDM cannot create %s\n", acSrcPathName);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(tempBuffer);
			free(tempBuffer1);
			return false;
		}

		/*
		if(strcmp(acSrcPathName,"/mnt/vendor/nvdata/APCFG/APRDEB/BT_Addr")==0)
			{
				int m;
				for(m=0;m<8;m++)
				 NVBAK_LOG("capid:%d\n",*(char*)(tempBuffer1+offset+m));
			}
			*/
		iResult = write(iFileDesc, tempBuffer1 + offset, iFileSize);
		if (iResult != iFileSize) {
			NVBAK_LOG("FileOp_RestoreFromFilesForDM cannot write %s,%d\n", acSrcPathName,
			          iResult);
			close(iFileDesc_map);
			close(iFileDesc_file);
			free(tempBuffer);
			free(tempBuffer1);
			close(iFileDesc);
			return false;
		}
		offset += iFileSize;
		iFileNum--;
		i++;
		close(iFileDesc);
	}
	close(iFileDesc_map);
	close(iFileDesc_file);
	free(tempBuffer);
	free(tempBuffer1);
	//       close(iFileDesc);
	NVBAK_LOG("restore from binregion for DM Success\n");
	return true;
}


bool FileOp_RestoreFromBinRegion_ToFile() {
	int iFileDesc_file, iFileDesc_map, fd, iResult, iBlockNum, iFileDesc_backupflag;
	unsigned int iMapFileSize, iDatFileSize, iBlockSize, iPartitionSize, iWriteSize;
	unsigned short iAPBackupFileNum, iMDBackupFileNum;
	bool bRet = true;
	struct stat st;

	char cMtdDevName[128] = {0};
	char *tempBuffer = NULL;
	char *tempBitmap1 = NULL;
	char *tempBitmap2 = NULL;
	int i, j, pos = 0, flag = 0;
	bool bSuccessFound = false;
	struct mtd_info_user info;
	char temp[128];
	//fix nvram layout issue , reassgined block numerbers by different platform
	if (NVM_Init() < 0) {
		NVBAK_LOG("[NVRAM]:FileOp_RestoreFromBinRegion_ToFile--NVM_Init fail !!!\n");
		return false;
	}
	NVBAK_LOG("[NVRAM]:Enter FileOp_RestoreFromBinRegion_ToFile\n");

	snprintf(cMtdDevName, sizeof(cMtdDevName),"%s", g_NVRAM_BACKUP_DEVICE);

	NVBAK_LOG("[NVRAM]:cMtdDevName:%s\n", cMtdDevName);
	bRet = NVM_GetDeviceInfo(cMtdDevName, &info);
	if (bRet == false) {
		NVBAK_LOG("[NVRAM]:FileOp_RestoreFromBinRegion_ToFile get device info error\r\n");
		return false;
	}
	fd = open(cMtdDevName, O_RDWR);
	if (fd < 0) {
		NVBAK_LOG("[NVRAM]:mtd open error\r\n");
		return false;
	}

	iBlockSize = info.erasesize;
	iPartitionSize = info.size;
	iWriteSize = info.writesize;

	if (nvram_platform_layout_version == 0)
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
		NVBAK_LOG("[NVRAM]:memory malloc error\r\n");
		close(fd);
		return false;
	}

	lseek(fd, pos * iBlockSize, SEEK_SET);
	iResult = read(fd, tempBuffer, iBlockSize);

	iMapFileSize = *((unsigned int*)tempBuffer); // get the mapfile
	unsigned int iMemSize;
	int iReservePos = 0;
	int iReserveBlockCount = 0;
	if ((iMapFileSize + 3 * sizeof(unsigned int)) % iWriteSize != 0)
		iMemSize = (((iMapFileSize + 3 * sizeof(unsigned int)) / iWriteSize) + 1) *
		           iWriteSize;
	else
		iMemSize = iMapFileSize + 3 * sizeof(unsigned
		                                     int); //mapfile size, datfile size, cleanboot fla
	NVBAK_LOG("[NVRAM Restore]:map file size:%d\n", iMemSize);

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
	NVBAK_LOG("[NVRAM]:iBlockSize:%d\n", iBlockSize);
	NVBAK_LOG("[NVRAM]:iPartitionSize:%d\n", iPartitionSize);
	NVBAK_LOG("[NVRAM]:BinRegionBlockTotalNum:%d\n", BinRegionBlockTotalNum);
	NVBAK_LOG("[NVRAM]:NvRamBlockNum:%d\n", NvRamBlockNum);
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
		iFileDesc_backupflag = open(g_pcNVM_BackFlag , O_TRUNC|O_CREAT|O_RDWR|O_SYNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
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
	iFileDesc_file = open(g_pcNVM_AllFile_Check, O_TRUNC | O_CREAT | O_RDWR,
	                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	iFileDesc_map = open(g_pcNVM_AllMap_Check,  O_TRUNC | O_CREAT | O_RDWR,
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
		if (tempBitmap1[i] == 0) {
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
		for (i; i < iBlockNum; i++) {
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
		// NVBAK_LOG("[NVRAM]:dat file read size:%d\n", iResult);
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
	NVBAK_LOG("[NVRAM]:Leave FileOp_RestoreFromBinRegion_ToFile\n");
	return true;
}//end FileOp_RestoreFromBinRegion_ToFile
bool FileOp_CheckBackUpResult() {
	unsigned int ulSavedCheckSum;
	unsigned int ulCheckSum;
	struct stat st;
	unsigned int iFileSize;

	NVBAK_LOG("[NVRAM]: Enter FileOp_CheckBackUpResult\n");

	if (!FileOp_GetCheckSum()) {
		NVBAK_LOG("[FileOp_RestoreData_All] GetCheckSum Fail !");
		return false;
	}
	ulSavedCheckSum = stBackupFileInfo.ulCheckSum;
	ulCheckSum = FileOp_ComputeReadBackCheckSum();
	NVBAK_LOG("[NVRAM]ulSavedCheckSum:%ld\n", ulSavedCheckSum);
	NVBAK_LOG("[NVRAM]ulCheckSun:%ld\n", ulCheckSum);
	if (stat(g_pcNVM_AllFile_Check, &st) < 0) {
		NVBAK_LOG("[NVRAM]Error FileOp_CheckBackUpResult stat \n");
		return false;
	}
	iFileSize = st.st_size;
	NVBAK_LOG("[NVRAM]iFileSize:%d\n", iFileSize);
	ulCheckSum ^= iFileSize;

	NVBAK_LOG("[NVRAM]FileOp_CheckSum:%lx,%lx", ulSavedCheckSum, ulCheckSum);
	if (!FileOp_CompareCheckSum(ulSavedCheckSum, ulCheckSum)) {
		NVBAK_LOG("[NVRAM]check sum not match!");
		return false;
	}
	NVBAK_LOG("[NVRAM]: Leave FileOp_CheckBackUpResult\n");
	return true;
}//end of FileOp_CheckBackUpResult
