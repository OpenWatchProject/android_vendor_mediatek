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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __CCCI_FSD_H__
#define __CCCI_FSD_H__

#define FS_API_RESP_ID			0xFFFF0000
#define FS_FILE_MAX			129
#define FS_MAX_RETRY			7
#define FS_REQ_BUFFER_MUN		5
#define FS_MAX_DIR_NUM			8

#define MD1_FS_TAG	"ccci_fsd(1)" //"ccci_fsd"
#define MD2_FS_TAG	"ccci_fsd(2)" //"ccci2_fsd"
#define MD3_FS_TAG	"ccci_fsd(3)" //"ccci2_fsd"
#define MD5_FS_TAG	"ccci_fsd(5)" //"eemcs_fsd"
#define MD_COMN_TAG "ccci_fsd(0)"
#define dbg_printf(...) //__android_log_print(ANDROID_LOG_DEBUG, MD_COMN_TAG, __VA_ARGS__);

#define FS_ROOT_DIR_MD1			"/mnt/vendor/nvdata/md"
#define FS_ROOT_DIR1_MD1	        "/mnt/vendor/protect_f/md"
#define FS_ROOT_DIR2_MD1	        "/mnt/vendor/protect_s/md"
#define FS_ROOT_DIR3_MD1	        "/vendor/etc/mdota"
#define FS_ROOT_DIR4_MD1	        "/mnt/vendor/nvcfg"
//#define FS_ROOT_DIR1_MD1	        "/mnt/vendor/nvdata/md_pf"
//#define FS_ROOT_DIR2_MD1	        "/mnt/vendor/nvdata/md_ps"

#define FS_ROOT_DIR_MD2			"/mnt/vendor/nvdata/md2"
#define FS_ROOT_DIR1_MD2	        "/mnt/vendor/protect_f/md2"
#define FS_ROOT_DIR2_MD2	        "/mnt/vendor/protect_s/md2"
//#define FS_ROOT_DIR1_MD2	        "/mnt/vendor/nvdata/md2_pf"
//#define FS_ROOT_DIR2_MD2	        "/mnt/vendor/nvdata/md2_ps"

#define FS_ROOT_DIR_MD3			"/mnt/vendor/nvdata/md3"
#define FS_ROOT_DIR1_MD3	        "/mnt/vendor/protect_f/md3"
#define FS_ROOT_DIR2_MD3	        "/mnt/vendor/protect_s/md3"
#define FS_ROOT_DIR3_MD3		"/mnt/vendor/nvdata/md3_v"
#define FS_ROOT_COMMON		        "/mnt/vendor/nvdata/md_cmn"

#define FS_ROOT_MDLPM			"/data/vendor/mdlpm"
#define MD_ROOT_DIR			"Z:"
#define MD_ROOT_DIR1 	                "X:"
#define MD_ROOT_DIR2 	                "Y:"
#define MD_ROOT_DIR3 	                "V:"
#define MD_ROOT_DIR4 	                "U:"
#define MD_ROOT_DIR5 	                "T:"
#define MD_ROOT_DIR6 	                "S:"

#define INVALID_HANDLE_VALUE	-1

//typedef enum
//{
//	false=0,
//	true=1
//} bool;

/* --------------------- FS ERROR CODE ---------------------- */
#define FS_NO_ERROR											0
#define FS_ERROR_RESERVED								-1
#define	FS_PARAM_ERROR									-2
#define FS_INVALID_FILENAME							-3
#define FS_DRIVE_NOT_FOUND							-4
#define FS_TOO_MANY_FILES								-5
#define FS_NO_MORE_FILES								-6
#define FS_WRONG_MEDIA									-7
#define FS_INVALID_FILE_SYSTEM					-8
#define	FS_FILE_NOT_FOUND								-9
#define FS_INVALID_FILE_HANDLE					-10
#define FS_UNSUPPORTED_DEVICE						-11
#define FS_UNSUPPORTED_DRIVER_FUNCTION	-12
#define FS_CORRUPTED_PARTITION_TABLE		-13
#define FS_TOO_MANY_DRIVES							-14
#define FS_INVALID_FILE_POS							-15
#define FS_ACCESS_DENIED								-16
#define FS_STRING_BUFFER_TOO_SAMLL			-17
#define FS_GENERAL_FAILURE							-18
#define FS_PATH_NOT_FOUND								-19
#define FS_FAT_ALLOC_ERROR							-20
#define FS_ROOT_DIR_FULL								-21
#define FS_DISK_FULL										-22
#define FS_TIMEOUT											-23
#define FS_BAD_SECTOR										-24
#define FS_DATA_ERROR										-25
#define FS_MEDIA_CHANGED								-26
#define FS_SECTOR_NOT_FOUND							-27
#define FS_ADDRESS_MARK_NOT_FOUND				-28
#define FS_DRIVE_NOT_READY							-29
#define FS_WRITE_PROTECTION							-30
#define FS_DMA_OVERRUN									-31							
#define FS_CRC_ERROR										-32
#define FS_DEVICE_RESOURCE_ERROR				-33
#define FS_INVALID_SECTOR_SIZE					-34
#define FS_OUT_OF_BUFFERS								-35
#define FS_FILE_EXISTS									-36
#define FS_LONG_FILE_POS								-37
#define FS_FILE_TOO_LARGE								-38
#define FS_BAD_DIR_ENTRY								-39
#define FS_ATTR_CONFLICT								-40
#define FS_CHECKDISK_RETRY							-41
#define FS_LACK_OF_PROTECTION_SPACE			-42
#define FS_SYSTEM_CRASH									-43
#define FS_FAIL_GET_MEM									-44
#define FS_READ_ONLY_ERROR							-45
#define FS_DEVICE_BUSY									-46
#define FS_ABORTED_ERROR								-47
#define FS_QUOTA_OVER_DISK_SPACE				-48
#define FS_PATH_OVER_LEN_ERROR					-49
#define FS_APP_QUOTA_FULL								-50
#define FS_VF_MAP_ERROR									-51
#define FS_DEVICE_EXPORTED_ERROR				-52	
#define FS_DISK_FRAGMENT								-53	
#define FS_DIRCACHE_EXPIRED							-54
#define FS_QUOTA_USAGE_WARNING					-55

#define FS_MSDC_MOUNT_ERROR							-100
#define FS_MSDC_READ_SECTOR_ERROR				-101
#define FS_MSDC_WRITE_SECTOR_ERROR			-102
#define FS_MSDC_DISCARD_SECTOR_ERROR		-103
#define FS_MSDC_PRESENT_NOT_READY				-104
#define FS_MSDC_NOT_PRESENT							-105

#define FS_EXTERNAL_DEVICE_NOT_PRESENT	-106
#define FS_HIGH_LEVEL_FORMAT_ERROR			-107

#define FS_FLASH_MOUNT_ERROR						-120
#define FS_FLASH_ERASE_BUSY							-121
#define FS_NAND_DEVICE_NOT_SUPPORTED		-122
#define FS_FLASH_OTP_UNKNOWERR					-123
#define FS_FLASH_OTP_OVERSCOPE					-124
#define FS_FLASH_OTP_WRITEFAIL					-125
#define FS_FDM_VERSION_MISMATCH					-126
#define FS_FLASH_OTP_LOCK_ALREADY				-127
#define FS_FDM_FORMAT_ERROR							-128

#define FS_LOCK_MUTEX_FAIL							-141
#define FS_NO_NONBLOCKMODE							-142
#define FS_NO_PROTECTIONMODE						-143

#define FS_INTERRUPT_BY_SIGNAL                      -512


/* -----------------  FS Setting ------------------------*/
#define FS_ATTR_READ_ONLY					0x01		
#define FS_ATTR_HIDDEN						0x02		
#define FS_ATTR_SYSTEM						0x04		
#define FS_ATTR_VOLUME						0x08		
#define FS_ATTR_DIR								0x10		
#define FS_ATTR_ARCHIVE						0x20		
#define FS_LONGNAME_ATTR					0x0F		

/* ----------------- Parameter for APIs -------------------*/
//FS_Open Parameter
#define FS_READ_WRITE 						0x00000000L
#define FS_READ_ONLY							0x00000100L
#define FS_OPEN_SHARED						0x00000200L
#define FS_OPEN_NO_DIR						0x00000400L
#define FS_OPEN_DIR								0x00000800L
#define FS_CREATE									0x00010000L
#define FS_CREATE_ALWAYS					0x00020000L
#define FS_COMMITTED							0x01000000L
#define FS_CACHE_DATA							0x02000000L
#define FS_LAZY_DATA							0x04000000L
#define	FS_NONBLOCK_MODE					0x10000000L
#define FS_PROTECTION_MODE				0x20000000L
                              		
//FS_GetDiskInfo Parameter    		
#define FS_DI_BASIC_INFO					0x00000001L
#define FS_DI_FREE_SPACE					0x00000002L
#define FS_DI_FAT_STATISTICS			0x00000004L
                              		
//FS_GetDrive Parameter       		
#define FS_NO_ALT_DRIVE						0x00000001
#define FS_ONLY_ALT_SERIAL				0x00000002
#define FS_DRIVE_I_SYSTEM					0x00000004
#define FS_DRIVE_V_NORMAL					0x00000008
#define FS_DRIVE_V_REMOVABLE			0x00000010
#define FS_DRIVE_V_EXTERNAL				0x00000020
#define FS_DRIVE_V_SIMPLUS				0x00000040
                              		
//FS_Move, FS_Count, FS_GetFolderSize, FS_XDelete, FS_XFindReset (Sorting) Parameter and Flag Passing
#define FS_MOVE_COPY							0x00000001
#define FS_MOVE_KILL							0x00000002
                              		
#define FS_FILE_TYPE							0x00000004
#define FS_DIR_TYPE								0x00000008
#define FS_RECURSIVE_TYPE					0x00000010
                              		
#define FS_NO_SORT								0x00000020
#define FS_SORT_NAME							0x00000040
#define FS_SORT_SIZE							0x00000080
#define FS_SORT_ATTR							0x00000100
#define FS_SORT_TYPE							0x00000200
#define FS_SORT_TIME							0x00000400
                              		
#define FS_COUNT_SIZE							0x00000800
#define FS_REMOVE_CHECK						0x00001000
#define FS_FILTER_SYSTEM_ATTR			0x00002000
#define FS_REC_COPYRIGHT_DEL			0x00004000
#define FS_REC_COPYRIGHT_LIST			0x00008000
#define FS_MOVE_OVERWRITE					0x00010000
#define FS_XDEL_ABORT_WATCH				0x00020000
#define FS_FILTER_HIDDEN_ATTR			0x00040000

//Quota Management
#define FS_QMAX_NO_LIMIT					0xf1f2f3f4 //~3.8GB
#define FS_COUNT_IN_BYTE					0x00000001
#define FS_COUNT_IN_CLUSTER				0x00000002

#define FS_BIN_REGION_BACKUP	0x1
#define FS_BIN_REGION_RESTORE	0x2
#define FS_BIN_REGION_ERASE	0x3
#define FS_BIN_REGION_CHECK	0x4
//FS_Seek Parameter
typedef enum{
		FS_FILE_BEGIN,
		FS_FILE_CURRENT,
		FS_FILE_END
}FS_SEEK_POS_ENUM;

//Find Series Return Value
typedef enum
{
		FS_NOT_MATCH,
		FS_LFN_MATCH,
		FS_SFN_MATCH
}FS_FIND_ENUM;

typedef enum
{
		FS_CCCI_OP_OPEN = 0x1001,
		FS_CCCI_OP_SEEK = 0x1002,
		FS_CCCI_OP_READ = 0x1003,
		FS_CCCI_OP_WRITE = 0x1004,
		FS_CCCI_OP_CLOSE = 0x1005,
		FS_CCCI_OP_CLOSEALL = 0x1006,
		FS_CCCI_OP_CREATEDIR = 0x1007,
		FS_CCCI_OP_REMOVEDIR = 0x1008,
		FS_CCCI_OP_GETFILESIZE = 0x1009,
		FS_CCCI_OP_GETFOLDERSIZE = 0x100A,
		FS_CCCI_OP_RENAME = 0x100B,
		FS_CCCI_OP_MOVE = 0x100C,
		FS_CCCI_OP_COUNT = 0x100D,
		FS_CCCI_OP_GETDISKINFO = 0x100E,
		FS_CCCI_OP_DELETE = 0x100F,
		FS_CCCI_OP_GETATTRIBUTES = 0x1010,
		FS_CCCI_OP_OPENHINT = 0x1011,
		FS_CCCI_OP_FINDFIRST = 0x1012,
		FS_CCCI_OP_FINDNEXT = 0x1013,
		FS_CCCI_OP_FINDCLOSE = 0x1014,
		FS_CCCI_OP_LOCKFAT = 0x1015,
		FS_CCCI_OP_UNLOCKALL = 0x1016,
		FS_CCCI_OP_SHUTDOWN = 0x1017,
		FS_CCCI_OP_XDELETE = 0x1018,
		FS_CCCI_OP_CLEARDISKFLAG = 0x1019,
		FS_CCCI_OP_GETDRIVE = 0x101A,
		FS_CCCI_OP_GETCLUSTERSIZE = 0x101B,
		FS_CCCI_OP_SETDISKFLAG = 0x101C,
		//support MD OTP Interface
		FS_CCCI_OP_OTP_WRITE = 0x101D,
		FS_CCCI_OP_OTP_READ = 0x101E,
		FS_CCCI_OP_OTP_QUERYLEN = 0x101F,
		FS_CCCI_OP_OTP_LOCK = 0x1020,
		//axs: add modem nvram restore API, 2012-02-06
		FS_CCCI_OP_RESTORE = 0x1021,
		/* cmpt_read */
		FS_CCCI_OP_CMPT_READ = 0x1022,
		/*Nvram Bin region access*/
		FS_CCCI_OP_BIN_REGION_ACCESS = 0x1023,
		/* cmpt_write */
		FS_CCCI_OP_CMPT_WRITE = 0x1024,
		/* get last modified time */
		FS_CCCI_OP_GETFILEDETAIL = 0x1025,
} FS_CCCI_OP_ID;

typedef struct
{
		int 	OperateID;
		char 	Buffer[0];
}FS_BUF;

typedef struct FS_PACKET_INFO_STRUCT
{
	unsigned int 	Length;
	void 					*pData;
} FS_PACKET_INFO;

typedef struct
{
		unsigned int	fInUse;
		unsigned int	fSearch;
		int		hFile;
		unsigned int Flag;
		char	Attr;
		char	AttrMask;
		char*	pFsFileName;
		char* pFsSearchPattern;
} FS_FILE_HANDLE;

typedef struct
{
		unsigned int data[2];
		unsigned int channel;
		unsigned int reserved;
} CCCI_BUFF_T;

typedef struct
{
		CCCI_BUFF_T header;
		FS_BUF payload;
}STREAM_DATA;

typedef enum {
    FS_BUFF_IDLE = 0,   // current port is not waiting for more data
    FS_BUFF_WAIT        // current port is waiting for more data to come in
} FS_BUFF_STATUS;

#define FS_ROOT_DIR_MD5			"/mnt/vendor/nvdata/md5"
#define FS_ROOT_DIR1_MD5	    "/vendor/protect_f/md5"
#define FS_ROOT_DIR2_MD5	    "/vendor/protect_s/md5"
#define FS_ROOT_DIR_DSP         "/vendor/firmware"
#define FS_ROOT_DIR_DSP_CIP     "/custom/etc/firmware"
#define MD_ROOT_DIR_DSP 	    "W:"

#define FS_BUFFER_SLOT_NUM 5
#define MAX_FS_PKT_BYTE (3584-128)
#define MAX_FS_BUF_BYTE 4096

#define CCCI_FS_REQ_SEND_AGAIN 0x80000000
#define CCCI_FS_PEER_REQ_SEND_AGAIN(_p) (((((CCCI_BUFF_T*)(_p))->data[0] & CCCI_FS_REQ_SEND_AGAIN) != 0)? 1: 0)

#define MODEM_POST_FIX_LENGTH (12)

typedef struct
{
		unsigned int		FileNum;
		FS_FILE_HANDLE	hFileHandle[FS_FILE_MAX];
		void*					pFsBuf;
        unsigned char       fs_buff_state[FS_BUFFER_SLOT_NUM];
        unsigned int        fs_buff_offset[FS_BUFFER_SLOT_NUM];
		int							bNonAck;
}FS_INFO;

typedef __packed struct
{
		unsigned int Second:5;
		unsigned int Minute:6;
		unsigned int Hour:5;
		unsigned int Day:5;
		unsigned int Month:4;
		unsigned int Year1980:7;
} FS_DOSDateTime;

typedef struct
{
		__packed char				FileName[8];
		__packed char				Extension[3];
		__packed char				Attributes;
		__packed char				NTReserved;
		__packed char				CreateTimeTenthSecond;
		__packed int                CreateDateTime;
		__packed unsigned short		LastAccessDate;
		__packed unsigned short		FirstClusterHi;
		__packed int                DateTime;
		__packed unsigned short		FirstCluster;
		__packed unsigned int		FileSize;
        // FS_FileOpenHint members (!Note that RTFDOSDirEntry structure is not changed!)
        unsigned int                Cluster;
        unsigned int                Index;
        unsigned int                Stamp;
        unsigned int                Drive;
        unsigned int                SerialNumber;		
}FS_DOSDirEntry;							

typedef struct
{
		char					Label[24];
		char					DriveLetter;
		char					WriteProtect;
		char					Reserved[2];
		unsigned int	SerialNumber;
		unsigned int	firstPhysicalSector;
		unsigned int	FATType;
		unsigned int	FATCount;
		unsigned int	MaxDirEntries;
		unsigned int	BytesPerSector;
		unsigned int	SectorsPerCluster;
		unsigned int	TotalClusters;
		unsigned int	BadClusters;
		unsigned int	FreeClusters;
		unsigned int	Files;
		unsigned int	FileChains;
		unsigned int	FreeChains;
		unsigned int	LargestFreeChain;
} FS_DiskInfo;

typedef struct
{
		unsigned long long	LastModifiedTime;
  		unsigned long long	LastAccessTime;
		unsigned long long	LastStatusChangeTime;
} FS_FileDetail;

// Bit mask for compact operation (align opid)
#define FS_CMPT_OPEN		(1<<0)
#define FS_CMPT_GETFILESIZE	(1<<1)
#define FS_CMPT_SEEK		(1<<2)
#define FS_CMPT_READ		(1<<3)
#define FS_CMPT_CLOSE		(1<<4)
#define FS_CMPT_WRITE		(1<<5)

#define FS_CMPT_MAX_OPID_NUM	(6)

typedef struct _nvram_fs_para_cmpt
{
	unsigned int opid_map;		// Indicate which operation shal be taken: open, get file size, seek, read, close
	unsigned int ret[2];		// return value from AP FSD: first is bitmap, second is error code
	unsigned int flag;		// FS_OPEN [in] param
	unsigned int file_size;		// FS_GETFILESIZE [out] param (not same to MD side defination)
	unsigned int offset;		// FS_SEEK [in] param
	unsigned int whence;		// FS_SEEK [in] param
	void         *data_ptr;		// FS_READ [out] param
	unsigned int length;		// FS_READ [in] param
	unsigned int act_read;		// FS_READ [out] param (not same to MD side defination)
} nvram_fs_para_cmpt_t;

typedef struct _nvram_fs_para_cmptw
{
   unsigned int opid_map;          // Indicate which operation may be taken: open, seek, write, close
   unsigned int ret[2];                 // return value from AP FSD: the first is bitmap, and second is error code
   unsigned int flag;                 // FS_OPEN [in] parameter
   unsigned int offset;             // FS_SEEK [in] parameter
   unsigned int whence;         // FS_SEEK [in] parameter
   void       *data_ptr;           // FS_WRITE[in] parameter : data buffer point
   unsigned int length;             // FS_WRITE [in] parameter: data length
   int file_handle;     		// FS_WRITE [in] parameter: used to write without no close operation
   int file_handle_ptr;  		// FS_WRITE [out] parameter (get from AP side when open the file)
   unsigned int act_write_size;    // FS_WRITE [out] parameter (get from AP side when after write the file)
} nvram_fs_para_cmptw_t;

extern char md_id;
enum {
    CCCI_MD_STA_UNDEFINED = -3,
    CCCI_MD_STA_INVALID = -2,
    CCCI_MD_STA_INIT = -1,
    CCCI_MD_STA_BOOT_READY = 0,
    CCCI_MD_STA_BOOT_UP,
    CCCI_MD_STA_RESET,
    CCCI_MD_STA_STOP,
    CCCI_MD_STA_FLIGHT_MODE,
    CCCI_MD_STA_EXCEPTION,

};
        
#endif //__CCCI_FSD_H__		
