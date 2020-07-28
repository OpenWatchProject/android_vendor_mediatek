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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <wchar.h>
#include <linux/ioctl.h>
#include <ctype.h>
#include <assert.h>
#include <log/log.h>
#include "ccci_fsd.h"
#include "ccci_fs.h"
#include "hardware/ccci_intf.h"
#include "fsd_platform.h"
#include <pthread.h>

#define PROFILE_NVRAM_API

//---------------------macro define------------------------------//
#define MD_DIR_ATTR (0770)


//---------------------static variable define-----------------------//
static char	   FsRootDir[FS_MAX_DIR_NUM][36];
static char    FsRootDir_MD1[FS_MAX_DIR_NUM][36] = {FS_ROOT_DIR_MD1, FS_ROOT_DIR1_MD1, FS_ROOT_DIR2_MD1, FS_ROOT_DIR_DSP, FS_ROOT_COMMON, FS_ROOT_MDLPM, FS_ROOT_DIR3_MD1, FS_ROOT_DIR4_MD1};
static char    FsRootDir_MD2[FS_MAX_DIR_NUM][36] = {FS_ROOT_DIR_MD2, FS_ROOT_DIR1_MD2, FS_ROOT_DIR2_MD2, FS_ROOT_DIR_DSP, FS_ROOT_COMMON};
static char    FsRootDir_MD3[FS_MAX_DIR_NUM][36] = {FS_ROOT_DIR_MD3, FS_ROOT_DIR1_MD3, FS_ROOT_DIR2_MD3, FS_ROOT_DIR_DSP, FS_ROOT_DIR3_MD3};
static char    FsRootDir_MD5[FS_MAX_DIR_NUM][36] = {FS_ROOT_DIR_MD5, FS_ROOT_DIR1_MD5, FS_ROOT_DIR2_MD5, FS_ROOT_DIR_DSP, FS_ROOT_COMMON};
static char    *MdRootDir[FS_MAX_DIR_NUM] = {MD_ROOT_DIR, MD_ROOT_DIR1, MD_ROOT_DIR2, MD_ROOT_DIR_DSP, MD_ROOT_DIR3, MD_ROOT_DIR4, MD_ROOT_DIR5, MD_ROOT_DIR6};
static unsigned DrvIdx[FS_MAX_DIR_NUM] = {'Z', 'X', 'Y', 'W', 'V', 'U', 'T', 'S'};
static int stream_support = 0;
static int DeviceFd = 0;
static FS_INFO g_FsInfo;
static unsigned int FS_MAX_BUF_SIZE = 0x4000;
static unsigned int FS_MAX_ARG_NUM = 5;
static pthread_t thread_id;
static int md_status_fd;

#ifdef PROFILE_NVRAM_API
typedef struct
{
	bool profiling;
	int count;
	int totaltime;
} SLOW_THAN_100MS_T;

SLOW_THAN_100MS_T slower_100ms = {false, 0, 0};
#endif

char md_id = 0;

static int FS_ErrorConv(int error)
{
	int ret = error;

	LOGD("Error_code=%d\n", error);

	switch(error)
	{
	    case EACCES:
	        ret = FS_ACCESS_DENIED;
	        break;

	    case ENOENT:
	        ret = FS_FILE_NOT_FOUND;
	        break;
				
	    case EMFILE:
	        ret = FS_TOO_MANY_FILES;
	        break;
				
	    default:
	        ret = FS_GENERAL_FAILURE;
	        break;
	}

	return ret;
}

static int FS_IsFile(const char* file)
{
	struct stat StatBuf;
	mode_t Mode;
	int local_errno;

	if(stat(file, &StatBuf) == -1)
	{
		local_errno = errno;
		LOGE("IsFile: [error]fail on file: %s \n", file);
			return FS_ErrorConv(local_errno);
	}

	Mode = StatBuf.st_mode;
		
	return (Mode & S_IFREG);
	}

#ifndef FAT_FS

static int FS_IsDIR(const char* file)
{
	struct stat StatBuf;
	mode_t Mode;
	int local_errno;

	if(stat(file, &StatBuf) == -1)
	{
		local_errno = errno;
		LOGE("IsDIR: [error]fail on file: %s \n", file);
		return FS_ErrorConv(local_errno);
	}

	Mode = StatBuf.st_mode;
	return (Mode & S_IFDIR);
}

static int FS_IsReadOnly(const char* file)
{
	struct stat StatBuf;
	mode_t Mode;
	int local_errno;

	if(stat(file, &StatBuf) == -1) {
		local_errno = errno;
		LOGE("FS_IsReadOnly: [error]fail on file: %s, error=%d\n", file, local_errno);
		return FS_ErrorConv(local_errno);
	}
	Mode = StatBuf.st_mode;
	if( (Mode & (S_IRUSR | S_IRGRP | S_IROTH)) &&
		 !(Mode & (S_IWUSR | S_IWGRP | S_IWOTH)) &&
		 !(Mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		return 1;
	else
		return 0;
}

static int FS_AttrReconv(const char* file)
{
	int Attr = 0;
	int value;

	if((value = FS_IsReadOnly(file)) < 0)
	    return value;
	else if(value == 1)
		Attr |= FS_ATTR_READ_ONLY;
	if((value = FS_IsDIR(file)) < 0)
	    return value;
	else if(value == S_IFDIR)
	    Attr |= FS_ATTR_DIR;
	return Attr;
	//Do not support FS_ATTR_HIDDEN
	//Do not support FS_ATTR_SYSTEM
	//Do not support FS_ATTR_ARCHIVE
}

static int FS_GetFileDetail(const char* file, FS_FileDetail *FileDetail)
{
	struct stat StatBuf;
	int local_errno;

	if(stat(file, &StatBuf) == -1)
	{
		local_errno = errno;
		LOGE("FS_GetFileDetail: [error]fail on file: %s, error=%d\n", file, local_errno);
		return FS_ErrorConv(local_errno);
	}
	FileDetail->LastModifiedTime = StatBuf.st_mtime;
	FileDetail->LastAccessTime = StatBuf.st_atime;
	FileDetail->LastStatusChangeTime = StatBuf.st_ctime;
	LOGE("FS_GetFileDetail: mtime: %llu, atime=%llu, ctime=%llu\n",
		FileDetail->LastModifiedTime, FileDetail->LastAccessTime,
		FileDetail->LastStatusChangeTime);

	return 0;
}

#else

static int FS_GetFATAttr(const char *file)
{
	int fd = -1;
	unsigned int attr;

	fd = open(file, O_RDONLY);
	if ( fd < 0 )
		goto err;

	if ( ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attr) )
		goto err;

	close(fd);
	return attr;

err:
	if ( fd >= 0 )
	close(fd);

	LOGE("GetFATAttr: [error]fail get file(%s) Attribute: %d \n", file, errno);
	return -1;
}

static bool FS_IsDIR(const char *file)
{	
	int Attr;
	Attr = FS_GetFATAttr(file);
	
	if(Attr != -1)
		return (Attr & FS_ATTR_DIR);
	else {
		LOGE("IsDIR: [error]fail on file: %s \n", file);
		return false;
	}
}

static bool FS_IsReadOnly(const char *file)
{		
	int Attr;
	Attr = FS_GetFATAttr(file);
	
	if(Attr != -1)
		return (Attr & FS_ATTR_READ_ONLY);
	else {
		LOGE("IsReadOnly: [error]fail on file: %s \n", file);
		return false;
	}
}

static char FS_AttrReconv(const char *file)
{
	char Attr;
	Attr = (char) FS_GetFATAttr(file);

	if(Attr != -1)
		return Attr;
	else {
		LOGE("AttrReconv: [error]fail on file: %s \n", file);
		return 0;
	}
}
#endif

#if 1
static int FS_ConvWcsToCs(const wchar_t* strScr, char* strDst)
{
    char *ptr;
	int   length;

	length = 0;
	ptr    = (char *) strScr;

	while (length < (PATH_MAX-1)) // Reserve 1 byte for string stop charactor
	{
	    if (ptr[length * 2] == '\\')
	    {
	        strDst[length] = '/';
	    }
	    else
	    {
	        strDst[length] = ptr[length * 2];

			if (strDst[length] == '.' && length > 0 && strDst[length - 1] == '.') {
				strDst[++length] = '\0';
				LOGE("Error, can't use relative path for security: %s\n", strDst);

				return -1;
			}
	    }
	    
	    length++;
	    
	    if (ptr[length * 2] == 0)
	    {
			strDst[length] = '\0'; 
			break;
	    }       
	}

	return length;   
}

static int FS_ConvCsToWcs(const char* strScr, wchar_t* strDst, unsigned int MaxLength)
{
    char  *ptr;
    unsigned int   length;

    length = 0;
    ptr    = (char *) strDst;
	    
    while (length < MaxLength)
    {
        if (strScr[length] == '/')
        {
            ptr[length * 2] = '\\';
            ptr[length * 2 + 1] = 0;
        }
        else
        {
            ptr[length * 2] = strScr[length];
            ptr[length * 2 + 1] = 0;           
        }  
        
        length++;
        
        if (strScr[length] == '\0')
        {
        	ptr[length * 2] = 0;
        	ptr[length * 2 + 1] = 0;  
            break;
        }       
    }

    if(length == MaxLength)
    {
        ptr[(length-1) * 2] = 0;
        ptr[(length-1) * 2 + 1] = 0;
        length -= 1;     
    }
        
    return length;   
}
#else
static int FS_ConvWcsToCs(const wchar_t* strScr, char* strDst)
{
	int i;
	int Length;

	Length = wcslen(strScr);

	for(i=0; i<Length; i++)
	{
		if(strScr[i] == L'\\')
			strDst[i] = '/';
		else
			strDst[i] = (char)strScr[i];
	}
	strDst[i] = '\0'; 

	dbg_printf("wcs %d %s %s\n", Length, CString, WCString);
	        
	return i;
}
#endif

//  Case-sensitive pattern match
int PatMatch (const char* pattern, const char* string)
{
	switch (pattern[0])
	{
		case '\0':
		    return !string[0];

		case '*':
		    return PatMatch(pattern+1, string) || (string[0] && PatMatch(pattern, string+1));

		case '?':
		    return string[0] && PatMatch(pattern+1, string+1);


		default:
		    return (pattern[0] == string[0]) && PatMatch(pattern+1, string+1);
	}
}

//  Case-insensitive pattern match
int PatiMatch (const char* pattern, const char* string)
{
	switch (pattern[0])
	{
		case '\0':
		    return !string[0];

		case '*' :
		    return PatiMatch(pattern+1, string) || (string[0] && PatiMatch(pattern, string+1));

		case '?' :
		    return string[0] && PatiMatch(pattern+1, string+1);

		default  :
		    return (toupper(pattern[0]) == toupper(string[0])) && PatiMatch(pattern+1, string+1);
	}
}

static void FS_EntryLinuxToDos(FS_DOSDirEntry *DosFileInfo, const char* file)
{
	struct tm *ModifyTime, *AccessTime;
	struct stat StatBuf;
	static unsigned int total_size;		

	if(stat(file, &StatBuf) == -1)
	{
		LOGE("EntryLinuxToDos: [error]fail get stat on file: %s \n", file);
	}

	//reset DosFileInfo file
	memset((void*)DosFileInfo, 0, sizeof(FS_DOSDirEntry));

	AccessTime = localtime((time_t*)&StatBuf.st_atime);
	ModifyTime = localtime((time_t*)&StatBuf.st_mtime);

	DosFileInfo->Attributes = FS_AttrReconv(file);
	DosFileInfo->CreateTimeTenthSecond = 0;

	if(AccessTime != NULL)
	{    
	    //Second: 5 bit in DOS DateTime stucture
	    //Minute: 6 bit in DOS DateTime stucture
	    //Hour: 5 bit in DOS DateTime stucture
	    //Day: 5 bit in DOS DateTime stucture
	    //Month: 4 bit in DOS DateTime stucture
	    //Year: 7 bit in DOS DateTime stucture and year in DOS is from 1980 but year in Linux is from 1900
	    *(__packed int*)(&(DosFileInfo->CreateDateTime)) = (((AccessTime->tm_sec  & 0x1F)) |
	                                  ((AccessTime->tm_min  & 0x3F) << 5) |
	                                  ((AccessTime->tm_hour & 0x1F) << 11) |
	                                  ((AccessTime->tm_mday & 0x1F) << 16) |
	                                  ((AccessTime->tm_mday & 0x0F) << 21) |
                                      (((AccessTime->tm_year - 80) & 0x7F) << 25));
	}

	DosFileInfo->LastAccessDate = 0;
	DosFileInfo->FirstClusterHi = 0;

	if(ModifyTime != NULL)
	{
	    //Second: 5 bit in DOS DateTime stucture
	    //Minute: 6 bit in DOS DateTime stucture
	    //Hour: 5 bit in DOS DateTime stucture
	    //Day: 5 bit in DOS DateTime stucture
	    //Month: 4 bit in DOS DateTime stucture
	    //Year: 7 bit in DOS DateTime stucture and year in DOS is from 1980 but year in Linux is from 1900
	    *(__packed int*)(&(DosFileInfo->DateTime)) = (((ModifyTime->tm_sec  & 0x1F)) |
	                            ((ModifyTime->tm_min  & 0x3F) << 5) |
	                            ((ModifyTime->tm_hour & 0x1F) << 11) |
	                            ((ModifyTime->tm_mday & 0x1F) << 16) |
	                            ((ModifyTime->tm_mday & 0x0F) << 21) |
                                (((ModifyTime->tm_year - 80) & 0x7F) << 25));    
	}

	DosFileInfo->FirstCluster = 0;
	DosFileInfo->FileSize = (unsigned int)StatBuf.st_size;
	total_size += DosFileInfo->FileSize;

	return;

}

static bool FS_GetPackInfo(FS_PACKET_INFO* pPackInfo, char* pData)
{
	unsigned int PackNum = *((unsigned int*)pData);
	unsigned int Index = 0;
	unsigned int i;
	
	if(PackNum > FS_MAX_ARG_NUM) {
		LOGE("GetPackInfo: [error]invalid packet number: %d \n", PackNum);
		return false;
	}
	
	Index = sizeof(unsigned int);
	for(i = 0; i < PackNum; i++)
	{
		pPackInfo[i].Length = *((unsigned int*)(pData + Index));
		Index += sizeof(unsigned int);
		pPackInfo[i].pData = (pData + Index);
		//4 byte alignment
		Index += ((pPackInfo[i].Length+3)>>2)<<2;
		if(pPackInfo[i].Length == 0) {
			LOGE("GetPackInfo: [error]length is eaqul to 0: packet%d \n", i);
			return false;
		}
	}
	
	if(Index > FS_MAX_BUF_SIZE) {
		LOGE("GetPackInfo: [error]length longer than BUF size: 0x%x \n", Index);
		unsigned int cnt = sizeof(unsigned int);
		for(i = 0; i < PackNum; i++)
		{		
			cnt += sizeof(unsigned int);
			cnt += ((pPackInfo[i].Length+3)>>2)<<2;
			LOGE("GetPackInfo: packet[%d].Length=0x%08X, packet[%d].pData=0x%08X, cnt=0x%08X \n", 
				i, pPackInfo[i].Length, i, *((unsigned int*)pPackInfo[i].pData), cnt);	
		}

		for(i=0; i<7; i++)
			LOGE("0x%08X, 0x%08X, 0x%08X, 0x%08X\n", 
				*((unsigned int*)(pData+16*i)), *((unsigned int*)(pData +16*i+4)),
				*((unsigned int*)(pData+16*i+8)), *((unsigned int*)(pData +16*i+12)));	
			
		return false;
	}
		
	return true;
}

// 5 = CCCI header + Operation ID
unsigned int g_bak[5];

/*
 * @brief Prepare a packet buffer for sending to MD
 * @param
 *     pData [in] A pointer to argument data for sending
 *     data_to_send [in] Size in bytes of argument data to send
 *     ccci_src [in] The pointer to the CCCI header for every sub-packet
 *     op_id [in] Operation ID currently used
 *     again [in] A flag means if we need to set "send again indicator"
 * @return
 *     On success, a pointer to arguments data is returned.
 *     On error, NULL is returned.
 */
void *FS_PreparePktEx(char *pData, unsigned int data_to_send, CCCI_BUFF_T *ccci_src, unsigned int op_id, unsigned int again)
{
    char *bak_ptr = NULL;
    STREAM_DATA *stream = NULL;

    assert(data_to_send <= MAX_FS_PKT_BYTE);
    assert(pData != NULL && ccci_src != NULL);
    assert(pData - (sizeof(CCCI_BUFF_T) + sizeof(unsigned int)) >= ccci_src);
	assert(sizeof(g_bak) == (sizeof(CCCI_BUFF_T) + sizeof(unsigned int))); 

	// move pointer forward to fill in CCCI header, this will replace orignal data there, so we backup them first
    bak_ptr = (char *)(pData - (sizeof(CCCI_BUFF_T) + sizeof(unsigned int)));
    // backup partial data
    memcpy((void*)g_bak, bak_ptr, sizeof(g_bak));
    stream = (STREAM_DATA *)bak_ptr;
	// copy CCCI header from the very fist header of all sub-packets
    if (again)
        stream->header.data[0] = ccci_src->data[0] | CCCI_FS_REQ_SEND_AGAIN;
    else
        stream->header.data[0] = ccci_src->data[0] & ~CCCI_FS_REQ_SEND_AGAIN;
    stream->header.data[1] = data_to_send + sizeof(CCCI_BUFF_T) + sizeof(unsigned int);;
    stream->header.channel = ccci_src->channel;
    stream->header.reserved = ccci_src->reserved;
    stream->payload.OperateID = op_id;

    //LOGD("FS_PreparePktEx() CCCI_H(0x%X)(0x%X)(0x%X)(0x%X), OP ID = 0x%X",
    //     stream->header.data[0], stream->header.data[1], stream->header.channel, stream->header.reserved,
    //     stream->payload.OperateID);
	
    return (void*)stream;
}

/*
 * @brief Determine the prepare data has done
 * @param
 *     pStream [in] A pointer returned from FS_PreparePktEx()
 * @return
 *     None
 */
void FS_PreparePktDone(void *pStream)
{
    assert(pStream != NULL);
    // Restore backuped data
    memcpy(pStream, (void*)g_bak, sizeof(g_bak));
}

static bool FS_WriteToMD(int DeviceFd, int BufIndex, FS_PACKET_INFO* pPacketSrc, int PacketNum)
{
	bool bRet = false;
	int ret = 0;
	char* pData;
	int DataLength = 0, AlignLength, i;
	FS_BUF *pFsBuf;
	fs_stream_msg_t stream_msg;
	int data_len = 0;
	int data_sent = 0;
	CCCI_BUFF_T *ccci_h = NULL;
	void *pkt_ptr = NULL;
	int pkt_size = 0;
	int data_to_send = 0;
	int local_errno = 0;
	STREAM_DATA *buffer_slot = (STREAM_DATA *)((char *)g_FsInfo.pFsBuf + (FS_MAX_BUF_SIZE + sizeof(STREAM_DATA))*BufIndex);

	if(!stream_support) {
		pFsBuf = (FS_BUF *)((char *)g_FsInfo.pFsBuf + (FS_MAX_BUF_SIZE + sizeof(FS_BUF))*BufIndex);
	} else {
		pFsBuf = &buffer_slot->payload;
		DataLength += sizeof(CCCI_BUFF_T);
		DataLength += sizeof(unsigned int); // size of operate ID field
	}
	pFsBuf->OperateID = FS_API_RESP_ID | pFsBuf->OperateID;
	pData = pFsBuf->Buffer;
	*((unsigned int*)pData) = PacketNum;
	pData += sizeof(unsigned int);
	DataLength += sizeof(unsigned int); // size of packet number field

	for(i = 0; i < PacketNum; i++)
	{
		if((DataLength + 2*sizeof(unsigned int) + pPacketSrc[i].Length) > FS_MAX_BUF_SIZE)
		{
			LOGE("WriteToMD: [error]Stream buffer full!! \n");
			goto _Exit;
		}
		*((unsigned int*)pData) = pPacketSrc[i].Length;
		pData += sizeof(unsigned int);
		DataLength += sizeof(unsigned int); // size of length field
		//4 byte aligned
		AlignLength = ((pPacketSrc[i].Length + 3) >> 2) << 2;
		DataLength += AlignLength;
		if(pData != pPacketSrc[i].pData)
			memcpy(pData, pPacketSrc[i].pData, pPacketSrc[i].Length);
		pData += AlignLength;
	}

	stream_msg.length = DataLength;
	stream_msg.index = BufIndex;
	if(!stream_support) {
		msync(pFsBuf, FS_MAX_BUF_SIZE, MS_SYNC);
		ret = ioctl(DeviceFd, CCCI_FS_IOCTL_SEND, &stream_msg);
		if(ret < 0) {
			local_errno = errno;
			LOGE("WriteToMD: [error]fail send FS stream: %d \n", local_errno);
			return bRet;
		}
	} else {
		// data length excluding CCCI header and OP ID
        data_len = DataLength - sizeof(CCCI_BUFF_T) - sizeof(unsigned int);
        ccci_h = &buffer_slot->header;
		ccci_h->channel++; //Rx->Tx

        /* No fragment is needed */
        if (data_len <= MAX_FS_PKT_BYTE) {
            pData = (char*)buffer_slot;
            // Clear "send again indicator"
            ccci_h->data[0] = ccci_h->data[0] & ~CCCI_FS_REQ_SEND_AGAIN;
			ccci_h->data[1] = DataLength;
RETRY_WRITE:
            ret = write(DeviceFd, pData, DataLength);
            if (ret != DataLength) {
		local_errno = errno;
                LOGE("Failed to write only one FS packet(%d)!! (%d/%d)\n", DataLength, ret, local_errno);
		if (local_errno == EINTR)
			goto RETRY_WRITE;
		        return bRet;
		}
            LOGD("Write %d bytes to slot %d, CCCI_H(0x%X)(0x%X)(0x%X)(0x%X)\n",
                 ret, BufIndex, ccci_h->data[0], ccci_h->data[1], ccci_h->channel, ccci_h->reserved);
        } else {
            /* Data fragment is needed */
            //LOGD("Big packet, need fragment.");
            pData = (char*)(&buffer_slot->payload.Buffer);
            while ((int)(data_sent + sizeof(CCCI_BUFF_T) + sizeof(unsigned int)) < DataLength) {
                /* Moret than 2 packets to send */
                /* Each packet includes CCCI header, OP id, and data */
                if ((data_len - data_sent) > MAX_FS_PKT_BYTE) {
                    data_to_send = MAX_FS_PKT_BYTE;
                    pkt_ptr = FS_PreparePktEx(pData, data_to_send, ccci_h, pFsBuf->OperateID, 1);
                } else {
            		/* The last packet */
                    data_to_send = data_len - data_sent;
                    pkt_ptr = FS_PreparePktEx(pData, data_to_send, ccci_h, pFsBuf->OperateID, 0);
                }
                // Add CCCI header and operation ID size to packet size, be aware of that OP_ID is not cosindered as payload, so not counted in MAX_FS_PKT_BYTE
                pkt_size = data_to_send + sizeof(CCCI_BUFF_T) + sizeof(unsigned int);
                // write size = data + CCCI header + OP ID
                ret = write(DeviceFd, pkt_ptr, pkt_size);
                if (ret != pkt_size) {
			local_errno = errno;
                    LOGE("Failed to write FS packet !! (%d)\n", local_errno);
                    break;
                } else {
                    CCCI_BUFF_T *dst_ccci_h = (CCCI_BUFF_T *)pkt_ptr;
                    LOGD("Write %d bytes to slot %d, CCCI_H(0x%X)(0x%X)(0x%X)(0x%X)\n",
                         ret, BufIndex,
                         dst_ccci_h->data[0], dst_ccci_h->data[1], dst_ccci_h->channel, dst_ccci_h->reserved);
                }
                FS_PreparePktDone(pkt_ptr);
                data_sent += data_to_send;
                pData += data_to_send;
            };
        }
		if (ret < 0) {
            LOGE("WriteToMD: [error]fail send FS stream: %d \n", ret);
            return bRet;
        }
        //LOGD("write to MD %d\n", DataLength);
	}
	bRet = true;
	
_Exit:
	return bRet;
}

static unsigned int FS_GetFreeHandle()
{
	unsigned int i;
	if(g_FsInfo.FileNum >= FS_FILE_MAX)
	{
		LOGE("GetFreeHandle: [error]file handle is full: %d \n", g_FsInfo.FileNum);
		return 0xFFFFFFFF;
	}
	
	for(i = 1; i < FS_FILE_MAX; i++)
	{
		if(g_FsInfo.hFileHandle[i].fInUse == false)
			return i;
	}
	return 0xFFFFFFFF;
}

static inline int is_protect_path(char *ConvFileName)
{
        int ret, len = 2;

        ret = strncmp(ConvFileName, MD_ROOT_DIR1, len);
        if (ret)
                ret = strncmp(ConvFileName, MD_ROOT_DIR2, len);

        return !ret;
}

/* This API is only for debug, you can open or close it */
static void CCCI_Dump_FS_Path(const char * FsFileName)
{
	char FsFileName_backup[PATH_MAX] = {0};
	char FsFileName_new[PATH_MAX] = {0};
	char *f_point;
	char *f_point_tail;
	char *f_point_head;
	int i = 0;
	int ret = 0;
	int need_trigger_ne = 0;
	int err_no;

	/* assume that FsFileName is /mnt/vendor/nvdata/xxxx */
	snprintf(FsFileName_backup, PATH_MAX, "%s", FsFileName);

	f_point_tail = f_point_head = strchr(FsFileName_backup, '/');

	do {
		f_point = f_point_tail = strrchr(FsFileName_backup, '/');
		/* now: FsFileName_backup is /mnt/vendor/nvdata */
		if (f_point_tail != NULL) {
			if ((f_point_tail - f_point_head) != 0) {
				*f_point = '\0';
				/* FsFileName_new is /mnt/vendor/nvdata */
				memcpy(FsFileName_new, FsFileName_backup, (f_point_tail - f_point_head + 1));
				ret = access(FsFileName_new, F_OK);
				//LOGE("Open: [error]the path %s exist\n", FsFileName_new);
				if (ret < 0) {
					err_no = errno;
					LOGE("Open: [error]the path %s not exist(%d)\n", FsFileName_new, err_no);
					if (strcmp(FsFileName_new, FS_ROOT_DIR_MD1) == 0)
						need_trigger_ne = 1;
					else if (strcmp(FsFileName_new, "/mnt/vendor/nvdata") == 0)
						need_trigger_ne = 1;
				}
				//memcpy(FsFileName_backup, FsFileName_new, (f_point_tail - f_point_head + 1));
			} else
				break;
		} else {
			LOGE("Open: [error]cannot find '/' in %s\n", FsFileName_backup);
			break;
		}
		i = i + 1;
	}while (i < 3); /* to avoid infinite loop by some surprised */

	if (need_trigger_ne)
		*(unsigned int volatile *)0 = 0;
}

static int FS_CCCI_Open(const wchar_t* FileName, unsigned int Flag)
{
	int Fd = 0;
	unsigned int HandleIndex;
	int ret = FS_GENERAL_FAILURE;
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	unsigned int LinuxFlag=0;
	char ConvFileName[PATH_MAX] = {0};
	int len = 0;
	int i = 0;
	int local_errno;

	if (FS_ConvWcsToCs(FileName, ConvFileName) < 0) {
		ret = FS_ACCESS_DENIED;
		goto _Exit;
	}

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0)
		{
			if( (strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX)
			{
				LOGE("Open: [error]file path too long: %s \n", ConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto _Exit;
			}
			//Convert the file name
			if(strncmp(ConvFileName, MD_ROOT_DIR_DSP, strlen(MD_ROOT_DIR_DSP)) == 0)
			{
				char md_post_fix[MODEM_POST_FIX_LENGTH];
				ioctl(DeviceFd, CCCI_IOC_GET_EXT_MD_POST_FIX, md_post_fix);
				FsFileNameCount = sprintf(FsFileName, "%s/dsp_%s.bin", FsRootDir[i], md_post_fix);
				if(access(FsFileName, F_OK) != 0) {
					LOGE("DSP image %s not exist, try CIP path %s\n", FsFileName, FS_ROOT_DIR_DSP_CIP);
					FsFileNameCount = sprintf(FsFileName, "%s/dsp_%s.bin", FS_ROOT_DIR_DSP_CIP, md_post_fix);
				}
				LOGD("open DSP image %s\n", FsFileName);
			} else {
				FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
			}
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("Open: [error]file path not find %s \n", ConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto _Exit;
	}

	//ToDo: check if flag definitions are the same.
	if(Flag & FS_READ_ONLY)
		LinuxFlag |= O_RDONLY;
	else
		LinuxFlag |= O_RDWR;

	if(Flag & FS_CREATE)
		LinuxFlag |= O_CREAT | O_RDWR;

	if(Flag & FS_CREATE_ALWAYS)
		LinuxFlag |= O_CREAT | O_RDWR | O_TRUNC;

	// Notice here
	//if(Flag & FS_COMMITTED)
	//	LinuxFlag |= O_SYNC;

	if(Flag & FS_NONBLOCK_MODE)
		LinuxFlag |= O_NONBLOCK;

	if((HandleIndex = FS_GetFreeHandle()) == 0xFFFFFFFF)
	{
		LOGE("Open: [error]fail get free handle for %s \n", ConvFileName);
		ret = FS_TOO_MANY_FILES;
		goto _Exit;
	}

	Fd = open(FsFileName, LinuxFlag, 0660);

	if(Fd == INVALID_HANDLE_VALUE)
	{
		local_errno = errno;
		LOGE("Open: [error]fail open %s %s :%d (0x%X)\n", ConvFileName, FsFileName, local_errno, LinuxFlag);
		CCCI_Dump_FS_Path(FsFileName);
		ret = FS_ErrorConv(local_errno);
		goto _Exit;
	}
	else
	{
		g_FsInfo.FileNum++;
		g_FsInfo.hFileHandle[HandleIndex].hFile = Fd;
		g_FsInfo.hFileHandle[HandleIndex].fInUse = true;
		g_FsInfo.hFileHandle[HandleIndex].Flag = Flag;
		g_FsInfo.hFileHandle[HandleIndex].fSearch = false;
		g_FsInfo.hFileHandle[HandleIndex].pFsFileName = malloc(sizeof(char)*(FsFileNameCount+1));
	
		if(g_FsInfo.hFileHandle[HandleIndex].pFsFileName == NULL)
		{
			LOGE("Open: [error]fail malloc memory for %s \n", ConvFileName);
			ret = FS_ERROR_RESERVED;
			goto _Exit;
		}
		memcpy(g_FsInfo.hFileHandle[HandleIndex].pFsFileName, FsFileName, sizeof(char)*(FsFileNameCount+1));
		ret = HandleIndex;
	}

_Exit:
	len = strlen(ConvFileName);
	if(len>=8) {
		if (is_protect_path(ConvFileName))
			LOGW("O: %s, flag 0x%x, ret %d\n", ConvFileName, Flag, ret);
		LOGD("O: [%02X%02X%02X%02X%02X%02X%02X%02X](0x%08X): (%d %d) \n", 
			(ConvFileName[len-1]-32),(ConvFileName[len-2]-32),(ConvFileName[len-3]-32),(ConvFileName[len-4]-32),
			(ConvFileName[len-5]-32),(ConvFileName[len-6]-32),(ConvFileName[len-7]-32),(ConvFileName[len-8]-32),
			Flag, Fd, ret);
	} else {
		LOGD("O: len<8(%d)\n", len);
	}
	return ret;
}	

static int FS_CCCI_Seek(int HandleIndex, int Offset, int Whence)
{
	int Fd;
	int LinuxWhence;
	off_t	NewPos;
	int ret = FS_GENERAL_FAILURE;		
	int local_errno;
	
	if ((unsigned int)HandleIndex >= FS_FILE_MAX) {
		LOGE("Close: [error] Handle index %d out-of-border.\n", HandleIndex);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	} else if(g_FsInfo.FileNum == 0 ||
		 g_FsInfo.hFileHandle[HandleIndex].hFile == INVALID_HANDLE_VALUE ||
		 g_FsInfo.hFileHandle[HandleIndex].fInUse != true)
	{
		LOGE("Seek: [error]Invalid handle for %s: %d, %d, %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex,
			g_FsInfo.hFileHandle[HandleIndex].hFile, g_FsInfo.hFileHandle[HandleIndex].fInUse);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	}
	Fd = g_FsInfo.hFileHandle[HandleIndex].hFile;
	switch(Whence)
	{
		case FS_FILE_BEGIN:
			LinuxWhence = SEEK_SET;
			break;

		case FS_FILE_CURRENT:
			LinuxWhence = SEEK_CUR;
			break;

		case FS_FILE_END:
			LinuxWhence = SEEK_END;
			break;

		default:
			ret = FS_PARAM_ERROR;
			goto Exit;
	}

	NewPos = lseek(Fd, (off_t) Offset, LinuxWhence);

	if(NewPos == (off_t) -1)
	{
		local_errno = errno;
		LOGE("Seek: [error]fail lseek %s: %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}
	else
		ret = NewPos;

Exit:
	LOGD("S: %d %d %d: %d \n", HandleIndex, Offset, Whence, ret);
	return ret;
}

static int FS_CCCI_Read(int HandleIndex, void* pBuffer, int NumOfByte, unsigned int* ReadByte)
{
	int Fd;	 
	int ret = FS_GENERAL_FAILURE;		
	int local_errno;
	int temp_val;

	if ((unsigned int)HandleIndex >= FS_FILE_MAX) {
		LOGE("Close: [error] Handle index %d out-of-border.\n", HandleIndex);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	} else if(g_FsInfo.FileNum == 0 ||
			g_FsInfo.hFileHandle[HandleIndex].hFile == INVALID_HANDLE_VALUE ||
			g_FsInfo.hFileHandle[HandleIndex].fInUse != true)
	{
		LOGE("Read: [error]Invalid handle for %s: %d, %d, %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex,
			g_FsInfo.hFileHandle[HandleIndex].hFile, g_FsInfo.hFileHandle[HandleIndex].fInUse);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	}

	Fd = g_FsInfo.hFileHandle[HandleIndex].hFile;
	//LOGD("Read: %s: (%d,%d)\n", g_FsInfo.hFileHandle[HandleIndex].pFsFileName,
	//HandleIndex, Fd);

    *ReadByte = read(Fd, pBuffer, (size_t)NumOfByte);
	temp_val = (int)*ReadByte;
	if(temp_val == -1) {
		local_errno = errno;
		LOGE("Read: [error]fail read %s(%d): %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}
	else
		ret = FS_NO_ERROR;

Exit:
	LOGD("R: %d %d: %d %d \n", HandleIndex, NumOfByte, *ReadByte, ret);
	return ret;
}

static int FS_CCCI_Write(int HandleIndex, void* pBuffer, int NumOfByte, int* WriteByte)
{
	int Fd;
	int ret = FS_GENERAL_FAILURE;
	int local_errno;
#ifdef PROFILE_NVRAM_API
	struct timespec time_start, time_end;
	long diff;
	int elapsed_in_ms;
#endif

	if ((unsigned int)HandleIndex >= FS_FILE_MAX) {
		LOGE("Close: [error] Handle index %d out-of-border.\n", HandleIndex);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	} else if(g_FsInfo.FileNum == 0 ||
			g_FsInfo.hFileHandle[HandleIndex].hFile == INVALID_HANDLE_VALUE ||
			g_FsInfo.hFileHandle[HandleIndex].fInUse != true)
	{
		LOGE("Write: [error]Invalid handle for %s: %d, %d, %d \n", 
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex, 
			g_FsInfo.hFileHandle[HandleIndex].hFile, g_FsInfo.hFileHandle[HandleIndex].fInUse);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	}

	Fd = g_FsInfo.hFileHandle[HandleIndex].hFile;
#ifdef PROFILE_NVRAM_API
	clock_gettime(CLOCK_REALTIME, &time_start);
#endif
	*WriteByte = write(Fd, pBuffer, (size_t)NumOfByte);
#ifdef PROFILE_NVRAM_API
	if (slower_100ms.profiling)
	{
		clock_gettime(CLOCK_REALTIME, &time_end);

		if (time_end.tv_nsec < time_start.tv_nsec) {
			diff = (long)(1000000000 + time_end.tv_nsec - time_start.tv_nsec) / 1000000;
			diff += (long)(time_end.tv_sec - time_start.tv_sec - 1) * 1000;
		} else {
			diff = (long)(time_end.tv_nsec - time_start.tv_nsec) / 1000000;
			diff += (long)(time_end.tv_sec - time_start.tv_sec) * 1000;
		}
		elapsed_in_ms = (int)diff;

		if (elapsed_in_ms > 100)
		{
			slower_100ms.count++;
			slower_100ms.totaltime += elapsed_in_ms;
			LOGE("Write: [overtime]elapsed_in_ms: %ld %ld %ld %ld, %d", time_start.tv_sec,
				time_start.tv_nsec, time_end.tv_sec, time_end.tv_nsec, elapsed_in_ms);
		}
	}
#endif
	if(*WriteByte == -1)
	{
		local_errno = errno;
		LOGE("Write: [error]fail write %s(%d): numofbytes=%d, error=%d\n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex, NumOfByte, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}
	else
		ret = FS_NO_ERROR;

Exit:
	LOGD("W: %d %d: %d %d \n", HandleIndex, NumOfByte, *WriteByte, ret);
	return ret;
}

static int FS_CCCI_Close(int HandleIndex)
{
	int ret = FS_GENERAL_FAILURE;
	int local_errno;
#ifdef PROFILE_NVRAM_API
	struct timespec time_start, time_end;
	long diff;
	int elapsed_in_ms;
#endif

	if ((unsigned int)HandleIndex >= FS_FILE_MAX) {
		LOGE("Close: [error] Handle index %d out-of-border.\n", HandleIndex);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	} else if(g_FsInfo.FileNum == 0 ||
			g_FsInfo.hFileHandle[HandleIndex].hFile == INVALID_HANDLE_VALUE ||
			g_FsInfo.hFileHandle[HandleIndex].fInUse != true)
	{
		LOGE("Close: [error]Invalid handle for %s: %d, %d, %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex,
			g_FsInfo.hFileHandle[HandleIndex].hFile, g_FsInfo.hFileHandle[HandleIndex].fInUse);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	}

	//LOGD("FS_CCCI_Close:%s: (%d, %d)\n", g_FsInfo.hFileHandle[HandleIndex].pFsFileName,
	//	HandleIndex, g_FsInfo.hFileHandle[HandleIndex].hFile);
	if(g_FsInfo.hFileHandle[HandleIndex].fSearch)
	{
		if(closedir((DIR*)g_FsInfo.hFileHandle[HandleIndex].hFile) != 0)
		{
			local_errno = errno;
			LOGE("Close: [error]fail close %s(%d): %d \n",
				g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex, local_errno);
			ret = FS_ErrorConv(local_errno);
			goto Exit;
		}
	} else {
		if (g_FsInfo.hFileHandle[HandleIndex].Flag & (FS_CREATE|FS_CREATE_ALWAYS|FS_COMMITTED)) {
            LOGD("C:fsync+ %d, %d, FLAG:0x%x\n", HandleIndex, ret, g_FsInfo.hFileHandle[HandleIndex].Flag);
			ret=fsync(g_FsInfo.hFileHandle[HandleIndex].hFile);
			LOGD("C:fsync- %d, %d, FLAG:0x%x\n", HandleIndex, ret,g_FsInfo.hFileHandle[HandleIndex].Flag);
   			if(ret<0)
   			{
   				local_errno = errno;
				LOGE("Close: [error]fail fsync %s(%d): %d \n",
				g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex, local_errno);
				ret = FS_ErrorConv(local_errno);
   			}
		}
#ifdef PROFILE_NVRAM_API
		clock_gettime(CLOCK_REALTIME, &time_start);
#endif
		if(close(g_FsInfo.hFileHandle[HandleIndex].hFile) == -1) {
			local_errno = errno;
			LOGE("Close: [error]fail close %s(%d): %d \n",
				g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex, local_errno);
			ret = FS_ErrorConv(local_errno);
			goto Exit;
		}
#ifdef PROFILE_NVRAM_API
		if (slower_100ms.profiling)
		{
			clock_gettime(CLOCK_REALTIME, &time_end);

			if (time_end.tv_nsec < time_start.tv_nsec) {
				diff = (long)(1000000000 + time_end.tv_nsec - time_start.tv_nsec) / 1000000;
				diff += (long)(time_end.tv_sec - time_start.tv_sec - 1) * 1000;
			} else {
				diff = (long)(time_end.tv_nsec - time_start.tv_nsec) / 1000000;
				diff += (long)(time_end.tv_sec - time_start.tv_sec) * 1000;
			}
			elapsed_in_ms = (int)diff;

			if (elapsed_in_ms > 100)
			{
				slower_100ms.count++;
				slower_100ms.totaltime += elapsed_in_ms;
				LOGE("Close: [overtime]elapsed_in_ms: %ld %ld %ld %ld, %d", time_start.tv_sec,
					time_start.tv_nsec, time_end.tv_sec, time_end.tv_nsec, elapsed_in_ms);
			}
		}
#endif
	}

	g_FsInfo.hFileHandle[HandleIndex].hFile = INVALID_HANDLE_VALUE;
	g_FsInfo.hFileHandle[HandleIndex].fInUse = false;

	free(g_FsInfo.hFileHandle[HandleIndex].pFsFileName);
	g_FsInfo.FileNum--;
	ret = FS_NO_ERROR;

Exit:
	LOGD("C: %d: %d\n", HandleIndex, ret);
	return ret;
}

static int FS_CCCI_CloseAll()
{
	int i;
	int ret = FS_GENERAL_FAILURE;
	int local_errno;

	for(i = 0; i < FS_FILE_MAX; i++)
	{
		if(g_FsInfo.hFileHandle[i].fInUse)
		{
			if(g_FsInfo.hFileHandle[i].fSearch)
			{
				//Todo;
			}
			else
			{
				if(close(g_FsInfo.hFileHandle[i].hFile) == -1)
				{
					local_errno = errno;
					LOGE("CloseAll: [error]fail close %s(%d): %d \n",
						g_FsInfo.hFileHandle[i].pFsFileName, i, local_errno);
					ret = FS_ErrorConv(local_errno);
					goto Exit;
				}
			}

			LOGD("CA:%d, %d\n", i, g_FsInfo.hFileHandle[i].hFile);

			g_FsInfo.hFileHandle[i].hFile = INVALID_HANDLE_VALUE;
			g_FsInfo.hFileHandle[i].fInUse = false;

			free(g_FsInfo.hFileHandle[i].pFsFileName);
			g_FsInfo.FileNum--;
		}
	}
	ret = FS_NO_ERROR;

Exit:
	LOGD("CA: %d \n", ret);
	return ret;
}

static int FS_CCCI_CreateDir(const wchar_t* FileName)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	char ConvFileName[PATH_MAX] = {0};
	struct stat buf;
	int ret = FS_GENERAL_FAILURE;
	int len = 0;
    int i = 0;
	int local_errno;

	FS_ConvWcsToCs(FileName, ConvFileName);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0)
		{
			if((strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX)
			{
				LOGE("CreateDir: [error]file path too long: %s \n", ConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("CreateDir: [error]file path not find %s \n", ConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}
	/* mkdir mdlpm if the drive is a U disk */
	if(ConvFileName[0] == 'U')
	{
		if(stat(FS_ROOT_MDLPM,&buf) < 0){
			LOGD("%s:stat %s dir error, ret=%d errno=%d.\n",
						__func__, FS_ROOT_MDLPM, ret, errno);
			if(mkdir(FS_ROOT_MDLPM, MD_DIR_ATTR) < 0)
				LOGE("%s:mkdir %s failed,ret=%d,errno=%d.\n",
					__func__, FS_ROOT_MDLPM, ret, errno);
		}
	}
	if(mkdir(FsFileName, MD_DIR_ATTR) == -1)
	{
		local_errno = errno;
		LOGE("CreateDir: [error]fail create Dir %s: %d \n", FsFileName, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}
	else
		ret = FS_NO_ERROR;

Exit:
	len = strlen(ConvFileName);
	if(len>=8) {
		LOGD("CD: [%02X%02X%02X%02X%02X%02X%02X%02X]: %d \n",
			(ConvFileName[len-1]-32),(ConvFileName[len-2]-32),(ConvFileName[len-3]-32),(ConvFileName[len-4]-32),
			(ConvFileName[len-5]-32),(ConvFileName[len-6]-32),(ConvFileName[len-7]-32),(ConvFileName[len-8]-32),
			ret);
	} else {
		LOGD("CD: len<8(%d)\n", len);
	}
	return ret;
}

static int FS_CCCI_RemoveDir(const wchar_t* FileName)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	char ConvFileName[PATH_MAX] = {0};
	int ret = FS_GENERAL_FAILURE;
	int len = 0;
	int i = 0;
	int local_errno;

	FS_ConvWcsToCs(FileName, ConvFileName);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("RemoveDir: [error]file path too long: %s \n", ConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("RemoveDir: [error]file path not find %s \n", ConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	if(rmdir(FsFileName) == -1) {
		local_errno = errno;
		LOGE("RemoveDir: [error]fail remove Directory %s: %d \n", FsFileName, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}
	else
		ret = FS_NO_ERROR;

Exit:
	len = strlen(ConvFileName);
	if(len>=8){
		if (is_protect_path(ConvFileName))
			LOGW("MD: %s, ret %d\n", ConvFileName, ret);
		LOGD("MD: [%02X%02X%02X%02X%02X%02X%02X%02X]: %d \n",
			(ConvFileName[len-1]-32),(ConvFileName[len-2]-32),(ConvFileName[len-3]-32),(ConvFileName[len-4]-32),
			(ConvFileName[len-5]-32),(ConvFileName[len-6]-32),(ConvFileName[len-7]-32),(ConvFileName[len-8]-32),
			ret);
	} else {
		LOGD("MD: len<8(%d)\n", len);
	}
	return ret;
}

static int FS_CCCI_GetFileSize(int HandleIndex, unsigned int* pFileSize)
{
	int Fd;
	struct stat StatBuf;
	int ret = FS_GENERAL_FAILURE;
	int local_errno;

	if ((unsigned int)HandleIndex >= FS_FILE_MAX) {
		LOGE("Close: [error] Handle index %d out-of-border.\n", HandleIndex);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	} else if(g_FsInfo.FileNum == 0 ||
		 g_FsInfo.hFileHandle[HandleIndex].hFile == INVALID_HANDLE_VALUE ||
		 g_FsInfo.hFileHandle[HandleIndex].fInUse != true)
	{
		LOGE("GetFileSize: [error]Invalid handle %s: %d, %d, %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex,
			g_FsInfo.hFileHandle[HandleIndex].hFile, g_FsInfo.hFileHandle[HandleIndex].fInUse);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	}

	Fd = g_FsInfo.hFileHandle[HandleIndex].hFile;

	if(fstat(Fd, &StatBuf) == -1) {
		local_errno = errno;
		LOGE("GetFileSize: [error]fail get file size %s: %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	} else {
		*pFileSize = (unsigned int) StatBuf.st_size;
		//LOGD("FS_CCCI_GetFileSize: %s Size = %d\n", g_FsInfo.hFileHandle[HandleIndex].pFsFileName,
		//	(unsigned int)StatBuf.st_size);
		ret = FS_NO_ERROR;
	}

Exit:
	LOGD("F: %d: %d %d \n", HandleIndex, *pFileSize, ret);
	return ret;
}

static int FS_CCCI_Rename(const wchar_t* FileName, const wchar_t* NewFileName)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	char FsNewFileName[PATH_MAX] = {0};
	char ConvFileName[PATH_MAX] = {0};
	char NewConvFileName[PATH_MAX] = {0};
	int ret = FS_GENERAL_FAILURE;
	int len1 = 0;
	int len2 = 0;
	int i = 0;
	int local_errno;

	FS_ConvWcsToCs(FileName, ConvFileName);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("Rename: [error]file path too long: %s \n", ConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("Rename: [error]file path not find %s \n", ConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}


	FS_ConvWcsToCs(NewFileName, NewConvFileName);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(NewConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(NewConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("Rename: [error]file path too long: %s \n", NewConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsNewFileName, sizeof(FsNewFileName), "%s%s", FsRootDir[i], (NewConvFileName + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("Rename: [error]file path not find %s \n", NewConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	if(rename(FsFileName, FsNewFileName) == -1)
	{
		local_errno = errno;
		LOGE("Rename: fail Rename file: %s->%s: %d \n",
			FsFileName, FsNewFileName, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}
	else
		ret = FS_NO_ERROR;

Exit:
	len1 = strlen(ConvFileName);
	len2 = strlen(NewConvFileName);
	if((len1>=8)&&(len2>=8)) {
		LOGD("R: [%02X%02X%02X%02X%02X%02X%02X%02X] -> [%02X%02X%02X%02X%02X%02X%02X%02X]: %d \n", 
			(ConvFileName[len1-1]-32),(ConvFileName[len1-2]-32),(ConvFileName[len1-3]-32),(ConvFileName[len1-4]-32),
			(ConvFileName[len1-5]-32),(ConvFileName[len1-6]-32),(ConvFileName[len1-7]-32),(ConvFileName[len1-8]-32),
			(NewConvFileName[len2-1]-32),(NewConvFileName[len2-2]-32),(NewConvFileName[len2-3]-32),(NewConvFileName[len2-4]-32),
			(NewConvFileName[len2-5]-32),(NewConvFileName[len2-6]-32),(NewConvFileName[len2-7]-32),(NewConvFileName[len2-8]-32),
			ret);
	} else {
		LOGD("R: len1:%d, len2:%d\n", len1, len2);
	}
	return ret;
}

static int FS_CCCI_Delete(const wchar_t* FileName)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	char ConvFileName[PATH_MAX] = {0};
	int ret = FS_GENERAL_FAILURE;
	int len = 0;
	int i = 0;
	int local_errno;

	FS_ConvWcsToCs(FileName, ConvFileName);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("Delete: [error]file path too long: %s \n", ConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("Delete: [error]file path not find %s \n", ConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit; 
	}
		
	if(unlink(FsFileName) == -1)
	{ 
		local_errno = errno;
		LOGE("Delete: [error]fail delete file %s: %d \n", FsFileName, errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}
	else
		ret = FS_NO_ERROR;

Exit:
	len = strlen(ConvFileName);
	if(len>=8) {
		if (is_protect_path(ConvFileName))
			LOGW("D: %s, ret %d\n", ConvFileName, ret);
		LOGD("D: [%02X%02X%02X%02X%02X%02X%02X%02X]: %d \n",
			(ConvFileName[len-1]-32),(ConvFileName[len-2]-32),(ConvFileName[len-3]-32),(ConvFileName[len-4]-32),
			(ConvFileName[len-5]-32),(ConvFileName[len-6]-32),(ConvFileName[len-7]-32),(ConvFileName[len-8]-32),
			 ret);
	} else {
		LOGD("D: len < 8(%d)\n", len);
	}
	return ret;
}

static int FS_CCCI_GetClusterSize(unsigned int DriverIdx)
{
    struct statfs StatfsBuf;
	int ClusterSize;
	int ret = FS_GENERAL_FAILURE;
	int i = 0;
	int local_errno;

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if (DriverIdx == DrvIdx[i])
			break;
	}
	if(i == FS_MAX_DIR_NUM) {
		ret = FS_DRIVE_NOT_FOUND;
		goto Exit;
	}
	if(statfs(FsRootDir[i], &StatfsBuf) == -1)
	{
		local_errno = errno;
		LOGE("GetClusterSize: [error] fail get statfs %s: %d \n", FsRootDir[i], local_errno);
		ret = FS_GENERAL_FAILURE;
		goto Exit;
	}

	//ClusterSize = 2048*64;
	ClusterSize = StatfsBuf.f_bsize;

	ret = ClusterSize;

Exit:
	LOGD("CS: %d: %d \n", DriverIdx, ret);
	return ret;
}

//Flag: Can not support FS_FILTER_SYSTEM_ATTR, FS_FILTER_HIDDEN_ATTR
static int FS_RecursiveSearch(const char* FsDirName, unsigned int Flag, unsigned int* pClusterCount)
{
	DIR* 		pDir;
	struct dirent*	pDirent = NULL;
	struct stat StatBuf;
	char FsFileName[PATH_MAX] = {0};
	unsigned int FoundFileNum = 0;
	unsigned int ClusterCount = 0;
	int ClusterSize;
	int RetVal;
	int local_errno;

	ClusterSize = FS_CCCI_GetClusterSize('Z');

	pDir = opendir(FsDirName);
	if(pDir == NULL)
	{
		local_errno = errno;
		LOGE("RecursiveSearch: fail Open Dir %s: %d \n", FsDirName, local_errno);
		return 0;
	}

	while((pDirent = readdir(pDir))!= NULL)
	{
		if((strlen(FsDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
		{
			LOGE("RecursiveSearch: [error]file path too long: %s \n", FsDirName);
			RetVal = FS_PATH_OVER_LEN_ERROR;
			goto _Exit;
		}

		snprintf(FsFileName, PATH_MAX, "%s/%s", FsDirName, pDirent->d_name);

		if(stat(FsFileName, &StatBuf) == -1)
		{
			local_errno = errno;
			LOGE("RecursiveSearch: [error]Get stat error on file: %s %d\n", pDirent->d_name, local_errno);
			continue;
		}

		if((StatBuf.st_mode & S_IFDIR) &&
			 strcmp(pDirent->d_name, ".") != 0 &&
			 strcmp(pDirent->d_name, "..") != 0)
		{
			if(Flag & FS_DIR_TYPE)
			{
				//Do not support FS_FILTER_SYSTEM_ATTR / FS_FILTER_HIDDEN_ATTR flag
				FoundFileNum++;
			}

			if(Flag & FS_RECURSIVE_TYPE)
			{
				/*
				if((strlen(FsDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
				{
					LOGE("RecursiveSearch: [error]file path too long: %s \n", FsDirName);
					RetVal = FS_PATH_OVER_LEN_ERROR;
					goto _Exit;
				}

	 			strcpy(FsFileName, FsDirName);
	 			strncat(FsFileName, "/", 1);
	 			strcat(FsFileName, pDirent->d_name);
				*/
				ClusterCount = 0;
	 			RetVal = FS_RecursiveSearch(FsFileName, Flag, &ClusterCount);
	 			if(RetVal < 0) {
					LOGE("RecursiveSearch: [error]fail search file %s: %d \n",
					FsFileName, RetVal);
 					goto _Exit;
	 			}
	 			*pClusterCount += ClusterCount;
	 			FoundFileNum += RetVal;
			}
		}
		else if(StatBuf.st_mode & S_IFREG)
		{
			if(Flag & FS_FILE_TYPE)
			{
				//Do not support FS_FILTER_SYSTEM_ATTR / FS_FILTER_HIDDEN_ATTR flag

				if(StatBuf.st_size > 0 && ClusterSize > 0)
					*pClusterCount += (StatBuf.st_size / ClusterSize + 1);
				FoundFileNum++;
			}
		}
	}

	RetVal = FoundFileNum;
_Exit:
	if(pDir != NULL)
		closedir(pDir);
	return RetVal;
}

static int FS_RecursiveDelete(const char* FsDirName, unsigned int Flag)
{
	DIR* 		pDir;
	struct dirent*	pDirent = NULL;
	struct stat StatBuf;
	char FsFileName[PATH_MAX] = {0};
	unsigned int DeleteFileNum = 0;
	int RetVal;
	int local_errno;

	pDir = opendir(FsDirName);
	if(pDir == NULL)
	{
		local_errno = errno;
		LOGE("RecursiveDelete: [error]fail open Dir %s: %d\n", FsDirName, local_errno);
		return 0;
	}

	while((pDirent = readdir(pDir)) != NULL)
	{
		if((strlen(FsDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
		{
			LOGE("FS_RecursiveSearch: [error]S_IFDIR:file path too long: %s\n", FsDirName);
			RetVal = FS_PATH_OVER_LEN_ERROR;
			goto _Exit;
		}
		snprintf(FsFileName, PATH_MAX, "%s/%s", FsDirName, pDirent->d_name);

		LOGD("RecursiveDelete: FsFileName %s\n", FsFileName);

		if(stat(FsFileName, &StatBuf) == -1)
		{
			local_errno = errno;
			LOGE("RecursiveDelete: [error]Get stat error on file: %s %d\n", pDirent->d_name, local_errno);
			continue;
		}

		if((StatBuf.st_mode & S_IFDIR) &&
			 strcmp(pDirent->d_name, ".") != 0 &&
			 strcmp(pDirent->d_name, "..") != 0)
		{
			/*
			if((strlen(FsDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
			{
				LOGE("FS_RecursiveSearch: [error]S_IFDIR:file path too long: %s\n", FsDirName);
				RetVal = FS_PATH_OVER_LEN_ERROR;
				goto _Exit;
			}

		 	strcpy(FsFileName, FsDirName);
		 	strncat(FsFileName, "/", 1);
		 	strcat(FsFileName, pDirent->d_name);
			*/

			if(Flag & FS_RECURSIVE_TYPE)
			{
	 			RetVal = FS_RecursiveDelete(FsFileName, Flag);
	 			if(RetVal < 0) {
					LOGE("RecursiveDelete: [error]S_IFDIR:fail delete file %s: %d\n", FsFileName, RetVal);
 					goto _Exit;
	 			}
	 			DeleteFileNum += RetVal;
			}

			if(Flag & FS_DIR_TYPE)
			{
				if(rmdir(FsFileName) == -1)
				{
					local_errno = errno;
					LOGE("RecursiveDelete: [error]S_IFDIR:fail remove DIR %s: %d\n", FsFileName, local_errno);
					RetVal = FS_ErrorConv(local_errno);
					goto _Exit;
				}
				DeleteFileNum++;
			}
		}
		else if(StatBuf.st_mode & S_IFREG)
		{
			if(Flag & FS_FILE_TYPE)
			{
				/*
				if((strlen(FsDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
				{
					LOGE("RecursiveSearch: [error]S_IFREG:file path too long: %s\n", FsDirName);
					RetVal = FS_PATH_OVER_LEN_ERROR;
					goto _Exit;
				}

	 			strcpy(FsFileName, FsDirName);
	 			strncat(FsFileName, "/", 1);
	 			strcat(FsFileName, pDirent->d_name);
				*/

				if(unlink(FsFileName) == -1)
				{
					local_errno = errno;
					LOGE("RecursiveDelete: [error]S_IFREG:fail delete file %s: err_code=%d\n", FsFileName, errno);
					RetVal = FS_ErrorConv(local_errno);
					goto _Exit;
				}
				DeleteFileNum++;
			}
		}
	}

	RetVal = DeleteFileNum;
_Exit:
	if(pDir != NULL)
		closedir(pDir);
	return RetVal;
}

#if 0
static int FS_RecursiveCopy(const char* FsDirName, const char* FsNewDirName)
{
	DIR* 		pDir;
	struct dirent*	pDirent = NULL;
	struct stat StatBuf;
	char FsFileName[PATH_MAX] = {0};
	char FsNewFileName[PATH_MAX] = {0};
	unsigned int CopyFileNum;
	int RetVal;
	int local_errno;

	if(mkdir(FsNewDirName, MD_DIR_ATTR) == -1)
	{
		local_errno = errno;
		LOGE("RecursiveCopy: [error]fail create DIR %s: %d\n", FsNewDirName, local_errno);
		return FS_ErrorConv(local_errno);
	}

	CopyFileNum = 1;

	pDir = opendir(FsDirName);
	if(pDir == NULL)
	{
		local_errno = errno;
		LOGE("RecursiveCopy: [error]fail open Dir %s: error=%d\n", FsDirName, local_errno);
		return 0;
	}

	while((pDirent = readdir(pDir))!= NULL)
	{
		if((strlen(FsDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
		{
			LOGE("RecursiveCopy: [error]S_IFDIR:original file path too long: %s\n", FsDirName);
			RetVal = FS_PATH_OVER_LEN_ERROR;
			goto _Exit;
		}

		snprintf(FsFileName, PATH_MAX, "%s/%s", FsDirName, pDirent->d_name);

		if(stat(FsFileName, &StatBuf) == -1)
		{
			local_errno = errno;
			LOGE("RecursiveCopy: [error]Get stat error on file: %s, error=%d\n", pDirent->d_name, local_errno);
			continue;
		}

		if((StatBuf.st_mode & S_IFDIR) &&
			 strcmp(pDirent->d_name, ".") != 0 &&
			 strcmp(pDirent->d_name, "..") != 0)
		{
			/*
			if((strlen(FsDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
			{
				LOGE("RecursiveCopy: [error]S_IFDIR:original file path too long: %s\n", FsDirName);
				RetVal = FS_PATH_OVER_LEN_ERROR;
				goto _Exit;
			}

		 	strcpy(FsFileName, FsDirName);
		 	strncat(FsFileName, "/", 1);
		 	strcat(FsFileName, pDirent->d_name);
		 	*/
		 	//create new file path
			if((strlen(FsNewDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
			{
				LOGE("RecursiveCopy: [error]S_IFDIR:new file path too long: %s\n", FsNewDirName);
				RetVal = FS_PATH_OVER_LEN_ERROR;
				goto _Exit;
			}
			snprintf(FsNewFileName, PATH_MAX, "%s/%s", FsNewDirName, pDirent->d_name);

		 	RetVal = FS_RecursiveCopy(FsFileName, FsNewFileName);
		 	if(RetVal < 0) {
				LOGE("RecursiveCopy: [error]S_IFDIR:fail copy file: %s->%s, %d\n", FsFileName, FsNewFileName, RetVal);
	 			goto _Exit;
		 	}
		 	CopyFileNum += RetVal;
		}
		else if(StatBuf.st_mode & S_IFREG)
		{
			/*
			if((strlen(FsDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
			{
				LOGE("RecursiveCopy: [error]S_IFREG:original file path too long: %s\n", FsDirName);
				RetVal = FS_PATH_OVER_LEN_ERROR;
				goto _Exit;
			}

		 	strcpy(FsFileName, FsDirName);
		 	strncat(FsFileName, "/", 1);
		 	strcat(FsFileName, pDirent->d_name);
		 	*/
		 	//create new file path
			if((strlen(FsNewDirName) + strlen(pDirent->d_name) + 2) > PATH_MAX)
			{
				LOGE("RecursiveCopy: [error]S_IFREG:new file path too long: %s\n", FsNewDirName);
				RetVal = FS_PATH_OVER_LEN_ERROR;
				goto _Exit;
			}
			snprintf(FsNewFileName, PATH_MAX, "%s/%s", FsNewDirName, pDirent->d_name);

		 	if(link(FsFileName, FsNewFileName) == -1)
		 	{
				local_errno = errno;
				LOGE("RecursiveCopy: [error]fail to copy file %s->%s: error=%d\n", FsFileName, FsNewFileName, local_errno);
				RetVal = FS_ErrorConv(local_errno);
				goto _Exit;
		 	}
		}
	}

	RetVal = CopyFileNum;
_Exit:
	if(pDir != NULL)
		closedir(pDir);
	return RetVal;
}
#endif

static int FS_CCCI_GetFolderSize(const wchar_t* FullPath, unsigned int Flag)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	unsigned int ClusterCount, ClusterSize;
	int FileCount;
	struct stat StatBuf;
	char ConvFullPath[PATH_MAX] = {0};
	int ret = FS_GENERAL_FAILURE;
	int len = 0;
	int i = 0;
	int local_errno;

	FS_ConvWcsToCs(FullPath, ConvFullPath);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFullPath, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFullPath + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("GetFolderSize: [error]file path too long: %s \n", ConvFullPath);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFullPath + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("GetFolderSize: [error]file path not find %s \n", ConvFullPath);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	ClusterSize = FS_CCCI_GetClusterSize(DrvIdx[i]);

	if(stat(FsFileName, &StatBuf) == -1)
	{
		local_errno = errno;
		LOGE("GetFolderSize: [error]Get the stat %s,error=%d\n", FsFileName, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}

	if(!(StatBuf.st_mode & S_IFDIR)) {
		LOGE("GetFolderSize: [error]file mode not S_IFDIR %s: mode=0x%x\n",
			FsFileName, StatBuf.st_mode);
		ret = FS_PARAM_ERROR;
		goto Exit;
	}

	ClusterCount = 0;
	FileCount = FS_RecursiveSearch(FsFileName, FS_FILE_TYPE | FS_DIR_TYPE | FS_RECURSIVE_TYPE, &ClusterCount);

	if(FileCount < 0) {
		local_errno = errno;
		LOGE("GetFolderSize: [error]fail recursive search file %s: %d, error=%d\n", FsFileName, FileCount, local_errno);
		ret = FileCount;
		goto Exit;
	}

	if(Flag == FS_COUNT_IN_CLUSTER)
		ret = ClusterCount;
	else
		ret = ClusterCount*ClusterSize;

	Exit:
	len = strlen(ConvFullPath);
	if(len>=8) {
		LOGD("FS: [%02X%02X%02X%02X%02X%02X%02X%02X](0x%08X): %d \n",
			(ConvFullPath[len-1]-32),(ConvFullPath[len-2]-32),(ConvFullPath[len-3]-32),(ConvFullPath[len-4]-32),
			(ConvFullPath[len-5]-32),(ConvFullPath[len-6]-32),(ConvFullPath[len-7]-32),(ConvFullPath[len-8]-32),
			Flag, ret);
	} else {
		LOGD("FS: len<8(%d)\n", len);
	}
	return ret;
}

static int FS_CCCI_Count(const wchar_t* FullPath, unsigned int Flag)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	struct stat StatBuf;
	unsigned int ClusterCount;
	char ConvFullPath[PATH_MAX] = {0};
	int ret = FS_GENERAL_FAILURE;
	int len = 0;
	int i = 0;
	int local_errno;

	FS_ConvWcsToCs(FullPath, ConvFullPath);
	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFullPath, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFullPath + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("Count: [error]file path too long: %s \n", ConvFullPath);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFullPath + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("Count: [error]file path not find %s \n", ConvFullPath);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	if(stat(FsFileName, &StatBuf) == -1)
	{
		local_errno = errno;
		LOGE("Count: [error]Get stat error on file: %s,error=%d\n", FsFileName, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}

	if(!(StatBuf.st_mode & S_IFDIR)) {
		LOGE("Count: [error]file mode isn't S_IFDIR %s: mode=0x%x \n", FsFileName, StatBuf.st_mode);
		ret = FS_PARAM_ERROR;
		goto Exit;
	}

	ClusterCount = 0;
	ret = FS_RecursiveSearch(FsFileName, Flag, &ClusterCount);

Exit:
	len = strlen(ConvFullPath);
	if(len>=8) {
		LOGD("CN: [%02X%02X%02X%02X%02X%02X%02X%02X](0x%08X): %d \n",
			(ConvFullPath[len-1]-32),(ConvFullPath[len-2]-32),(ConvFullPath[len-3]-32),(ConvFullPath[len-4]-32),
			(ConvFullPath[len-5]-32),(ConvFullPath[len-6]-32),(ConvFullPath[len-7]-32),(ConvFullPath[len-8]-32),
			Flag, ret);
	} else {
		LOGD("CN: len < 8(%d)\n", len);
	}
	return ret;
}

static int FS_CCCI_GetAttributes(const wchar_t* FileName)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	char ConvFileName[PATH_MAX] = {0};
	int ret = FS_GENERAL_FAILURE;
	int len = 0;
	int i = 0;

	FS_ConvWcsToCs(FileName, ConvFileName);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("GetAttributes: [error]file path too long: %s \n", ConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("GetAttributes: [error]file path not find %s \n", ConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	ret = FS_AttrReconv(FsFileName);

Exit:
	len = strlen(ConvFileName);
	if(len>=8) {
		LOGD("A: [%02X%02X%02X%02X%02X%02X%02X%02X]: %d \n",
			(ConvFileName[len-1]-32),(ConvFileName[len-2]-32),(ConvFileName[len-3]-32),(ConvFileName[len-4]-32),
			(ConvFileName[len-5]-32),(ConvFileName[len-6]-32),(ConvFileName[len-7]-32),(ConvFileName[len-8]-32),
			ret);
	} else {
		LOGD("A: len < 8(%d)\n", len);
	}
	return ret;
}

static int FS_CCCI_GetFileDetail(const wchar_t* FileName, FS_FileDetail *FileDetail)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	char ConvFileName[PATH_MAX] = {0};
	int ret = FS_GENERAL_FAILURE;
	int len = 0;
	int i = 0;

	FS_ConvWcsToCs(FileName, ConvFileName);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("GetFileDetail: [error]file path too long: %s \n", ConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("GetFileDetail: [error]file path not find %s \n", ConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	ret = FS_GetFileDetail(FsFileName, FileDetail);

Exit:
	len = strlen(ConvFileName);
	if(len>=8) {
		LOGD("GetFileDetail: [%02X%02X%02X%02X%02X%02X%02X%02X]: %d \n",
			(ConvFileName[len-1]-32),(ConvFileName[len-2]-32),(ConvFileName[len-3]-32),(ConvFileName[len-4]-32),
			(ConvFileName[len-5]-32),(ConvFileName[len-6]-32),(ConvFileName[len-7]-32),(ConvFileName[len-8]-32),
			ret);
	} else {
		LOGD("GetFileDetail: len < 8(%d)\n", len);
	}
	return ret;
}

static int FS_CCCI_XDelete(const wchar_t* FileName, unsigned int Flag)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	int DelFileNum;
	struct stat StatBuf;
	char ConvFileName[PATH_MAX] = {0};
	int ret = FS_GENERAL_FAILURE;
	int len = 0;
	int i = 0;
	int local_errno;

	FS_ConvWcsToCs(FileName, ConvFileName);
	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("XDelete: [error]file path too long: %s \n", ConvFileName);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("XDelete: [error]not find file: %s \n", ConvFileName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	if(((Flag & FS_FILE_TYPE) == 0) && ((Flag & FS_DIR_TYPE) == 0) && ((Flag & FS_RECURSIVE_TYPE) == 0))
	{
		ret = FS_PARAM_ERROR;
		goto Exit;
	}
	else if(Flag & ~(FS_FILE_TYPE | FS_DIR_TYPE | FS_RECURSIVE_TYPE))
	{
		ret = FS_PARAM_ERROR;
		goto Exit;
	}

	if(stat(FsFileName, &StatBuf) == -1)
	{
		local_errno = errno;
		LOGE("XDelete: [error]Get stat %s error=%d\n", FsFileName, local_errno);
		ret = FS_ErrorConv(local_errno);
		goto Exit;
	}

	if(!(StatBuf.st_mode & S_IFDIR)) {
		LOGE("XDelete: [error]file mode not S_IFDIR %s: mode=0x%x \n", FsFileName, StatBuf.st_mode);
		ret = FS_PARAM_ERROR;
		goto Exit;
	}

	DelFileNum = FS_RecursiveDelete(FsFileName, Flag);

	//Delete this folder if F_DIR_TYPE and FS_RECURSIVE_TYPE are set
	if(Flag & FS_DIR_TYPE)
	{
		if(Flag & FS_RECURSIVE_TYPE)
		{
			if(rmdir(FsFileName) == -1)
			{
				local_errno = errno;
				LOGE("XDelete: fail remove %s: %d \n", FsFileName, local_errno);
				ret = FS_ErrorConv(local_errno);
				goto Exit;
			}
			else
			{
				DelFileNum++;
			}
		}
	}
	ret = DelFileNum;

Exit:
	len = strlen(ConvFileName);
	if(len>=8) {
		if (is_protect_path(ConvFileName))
			LOGW("X: %s, ret %d\n", ConvFileName, ret);
		LOGD("X: [%02X%02X%02X%02X%02X%02X%02X%02X](0x%08X): %d \n",
			(ConvFileName[len-1]-32),(ConvFileName[len-2]-32),(ConvFileName[len-3]-32),(ConvFileName[len-4]-32),
			(ConvFileName[len-5]-32),(ConvFileName[len-6]-32),(ConvFileName[len-7]-32),(ConvFileName[len-8]-32),
			Flag, ret);
	} else {
		LOGD("X: len < 8(%d)\n", len);
	}
	return ret;
}

static int FS_CCCI_Move(const wchar_t* SrcFullPath, const wchar_t* DstFullPath, unsigned int Flag)
{
	int FsFileNameCount;
	static char FsFileName[PATH_MAX] = {0};
	char FsNewFileName[PATH_MAX] = {0};
	char ConvFullPath[PATH_MAX]= {0};
	char DstConvFullPath[PATH_MAX]= {0};
	int ret = FS_GENERAL_FAILURE;
	int len1 = 0;
	int len2 = 0;
	int i = 0;
	int local_errno;

	FS_ConvWcsToCs(SrcFullPath, ConvFullPath);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(ConvFullPath, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(ConvFullPath + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("Move: [error]file path too long: %s \n", ConvFullPath);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFullPath + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("Move: [error]file path not find %s \n", ConvFullPath);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	FS_ConvWcsToCs(DstFullPath, DstConvFullPath);

	for (i = 0; i < FS_MAX_DIR_NUM; i++) {
		if(strncmp(DstConvFullPath, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
			if((strlen(DstConvFullPath + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
				LOGE("Move: [error]file path too long: %s \n", DstConvFullPath);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			//Convert the file name
			FsFileNameCount = snprintf(FsNewFileName, sizeof(FsNewFileName), "%s%s", FsRootDir[i], (DstConvFullPath + strlen(MdRootDir[i])));
			break;
		}
	}

	if (i == FS_MAX_DIR_NUM) {
		LOGE("Move: [error]file path not find %s \n", DstConvFullPath);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	if(strcmp(FsFileName, FsNewFileName) == 0) {
		LOGE("Move: [error]move to the same path: %s->%s \n", FsFileName, FsNewFileName);
		ret = FS_PARAM_ERROR;
		goto Exit;
	}

	if((Flag & FS_MOVE_COPY) == 0 && (Flag & FS_MOVE_KILL) == 0) {
		LOGE("Move: [error]invaid file flag: %d \n", Flag);
		ret = FS_PARAM_ERROR;
		goto Exit;
	}

	if(Flag & ~(FS_MOVE_COPY | FS_MOVE_KILL | FS_MOVE_OVERWRITE)) {
		LOGE("Move: [error]invaid file flag: %d \n", Flag);
		ret = FS_PARAM_ERROR;
		goto Exit;
	}

	if(Flag & FS_MOVE_KILL)
	{
		if(FS_IsReadOnly(FsFileName) >= 1) {
			LOGE("Move: [error]invaid file flag: %d \n", Flag);
			ret = FS_READ_ONLY_ERROR;
			goto Exit;
		}
	}

	if(Flag & FS_MOVE_OVERWRITE)
	{
		if(FS_IsReadOnly(FsNewFileName) >= 1) {
			LOGE("Move: [error]file is read-only \n");
			ret = FS_READ_ONLY_ERROR;
			goto Exit;
		}

		if(FS_IsDIR(FsNewFileName) >= 1) {
			ret = FS_CCCI_XDelete(DstFullPath, FS_FILE_TYPE | FS_DIR_TYPE | FS_RECURSIVE_TYPE);				
			if(ret < 0) {
				LOGE("Move: [error]fail Xdelete dis file %s: %d \n",DstConvFullPath, ret);
				goto Exit;
			}
		} else if(FS_IsFile(FsNewFileName) >= 1) {
			ret = FS_CCCI_Delete(DstFullPath);
			if(ret < 0) {
				LOGE("Move: [error]fail delete dis file %s: %d \n",DstConvFullPath, ret);
				goto Exit;
			}
		}
	}

	if(Flag & FS_MOVE_KILL)
	{
		LOGE("Move: [error]kill %s, NULL\n", FsFileName);
	}

	if(Flag & FS_MOVE_COPY)
	{
		int fd = 0, newfd = 0;
		int size = 0;
		unsigned char* tempbuf = NULL;

		fd = open(FsFileName, O_RDONLY);
		if( fd  < 0)
		{
			local_errno = errno;
			LOGE("Move: [error]fail create %s: %d \n", FsFileName, local_errno);
			ret = FS_ErrorConv(local_errno);
			goto Exit1;
		}

		size = lseek(fd, 0, SEEK_END) - lseek(fd, 0, SEEK_SET);
		if(size < 0)
		{
			LOGE("FMove: [error]invalid length: %d \n", size);
			ret = FS_ERROR_RESERVED;
			goto Exit1;
		}

		dbg_printf("FS_CCCI_Move: len=%d \n", size);

		tempbuf = (unsigned char *)malloc(size);
		if(tempbuf == NULL)
		{
			LOGE("Move: [error]fail alloc memory <%p><%d>! \n", tempbuf, size);
			ret = FS_ERROR_RESERVED;
			goto Exit1;
		}

		lseek(fd, 0, SEEK_SET);

		ret  = read(fd, tempbuf, size);
		if(ret < size )
		{
			if(ret < 0)
			{
				local_errno = errno;
				LOGE("Move: [error]fail read file: %d \n", local_errno);
				ret = FS_ErrorConv(local_errno);
			}
			else
			{
				LOGE("Move: [error]fail read file: size=%d, read=%d \n", size, ret);
				ret = FS_ERROR_RESERVED;
			}
			goto Exit1;
		}

		dbg_printf("Move: read file <%d> \n", ret);

		newfd = creat(FsNewFileName, 0777);
		if( newfd  < 0)
		{
			local_errno = errno;
			LOGE("Move: [error]fail create %s: %d \n", FsNewFileName, local_errno);
			ret = FS_ErrorConv(local_errno);
			goto Exit1;
		}

		ret = write(newfd, tempbuf, size);
		if(ret < size)
		{
			if(ret < 0)
			{
				local_errno = errno;
				LOGE("Move: [error]fail write file: %d \n", local_errno);
				ret = FS_ErrorConv(local_errno);
			}
			else
			{
				LOGE("Move: [error]fail write file: size=%d, write=%d \n", size, ret);
				ret = FS_ERROR_RESERVED;
			}
			goto Exit1;
		}

		dbg_printf("Move: write <%d> \n", ret);

		ret = FS_NO_ERROR;
	Exit1:
		if(tempbuf)
			free(tempbuf);
		if(fd >= 0 )
			close(fd);
		if(newfd >= 0)
			close(newfd);
	}
Exit:
	len1 = strlen(ConvFullPath);
	len2 = strlen(DstConvFullPath);
	if((len1>=8)&&(len2>=8)) {
		if (is_protect_path(ConvFullPath))
			LOGW("D: %s, ret %d\n", ConvFullPath, ret);
		LOGD("M: [%02X%02X%02X%02X%02X%02X%02X%02X] -> [%02X%02X%02X%02X%02X%02X%02X%02X](0x%08X): %d \n", 
			(ConvFullPath[len1-1]-32),(ConvFullPath[len1-2]-32),(ConvFullPath[len1-3]-32),(ConvFullPath[len1-4]-32),
			(ConvFullPath[len1-5]-32),(ConvFullPath[len1-6]-32),(ConvFullPath[len1-7]-32),(ConvFullPath[len1-8]-32),
			(DstConvFullPath[len2-1]-32),(DstConvFullPath[len2-2]-32),(DstConvFullPath[len2-3]-32),(DstConvFullPath[len2-4]-32),
			(DstConvFullPath[len2-5]-32),(DstConvFullPath[len2-6]-32),(DstConvFullPath[len2-7]-32),(DstConvFullPath[len2-8]-32),
			Flag, ret);
	} else {
		LOGD("M: len1:%d, len2:%d\n", len1, len2);
	}
	return ret;	
} 

static int FS_CCCI_FindFirst(const wchar_t* PatternName, char Attr, char AttrMask, FS_DOSDirEntry *pFileInfo, wchar_t* FileName, unsigned int *pMaxLength)
{
		static char FsPatternName[PATH_MAX] = {0};
		char FsDirName[PATH_MAX] = {0};
		char FsFileName[PATH_MAX] = {0};
		int FsFileNameCount;
		int i; 
		char ConvPatternName[PATH_MAX] = {0};
		DIR* 	pDir = NULL;
		struct dirent*	pDirent = NULL;
		char FileAttr;	
		bool bFound = false;
		unsigned int HandleIndex;	
		unsigned int max_len = *pMaxLength;
		int ret = FS_GENERAL_FAILURE;	
		int len1 = 0;
		int len2 = 0;
		int local_errno;

		*pMaxLength = 0;
		FS_ConvWcsToCs(PatternName, ConvPatternName);
		for (i = 0; i < FS_MAX_DIR_NUM; i++) {
			if(strncmp(ConvPatternName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
				if((strlen(ConvPatternName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
					LOGE("FindFirst: [error]file path too long: %s \n", ConvPatternName);
					ret = FS_PATH_OVER_LEN_ERROR;
					goto Exit;
				}
				//Convert the file name
				FsFileNameCount = snprintf(FsPatternName, sizeof(FsPatternName), "%s%s", FsRootDir[i], (ConvPatternName + strlen(MdRootDir[i])));
				break;
			}
		}

		if (i == FS_MAX_DIR_NUM) {
			LOGE("FindFirst: [error]file path not find %s \n", ConvPatternName);
			ret = FS_PATH_NOT_FOUND;
			goto Exit;
		}

		for(i = FsFileNameCount-1; i >= 0; i--)
		{
			if(FsPatternName[i] == '/')
				break;
		}

		if(i <= 0) {
			LOGE("FindFirst: [error]fail find file path:%s \n", FsPatternName);
			ret = FS_PATH_NOT_FOUND;
			goto Exit;
		}

		i = i+1; // '/' must be excluded.
		if(strlen(FsPatternName+i) > NAME_MAX) {
			LOGE("FindFirst: [error]file path[1] too long:%s \n", FsPatternName);
			ret = FS_PATH_OVER_LEN_ERROR;
			goto Exit;
		}
		else
			snprintf(FsFileName, NAME_MAX, "%s", (FsPatternName+i));

		strncpy(FsDirName, FsPatternName, i-1); // skip '/'
		FsDirName[i-1] = '\0';

		dbg_printf("FindFirst: Open Dir <%s> ... \n", FsDirName);

		pDir = opendir(FsDirName);
		if(pDir == NULL)
		{
			local_errno = errno;
			LOGE("FindFirst: [error]fail open %s: %d \n", FsDirName, local_errno);
			ret = FS_ErrorConv(local_errno);
			goto Exit;
		}

		while((pDirent = readdir(pDir))!= NULL)
		{
	       if( strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0 )
			    continue;

			//check if file name match
			if(PatMatch(FsFileName, pDirent->d_name))
			{
				if(i+strlen(pDirent->d_name) > PATH_MAX) {
					LOGE("FindFirst: [error]file path[2] too long: %s \n", FsPatternName);
					ret = FS_PATH_OVER_LEN_ERROR;
					goto Exit;
				}

				snprintf((FsPatternName+i), (PATH_MAX - i), "%s", pDirent->d_name);
				FileAttr = FS_AttrReconv(FsPatternName);
				if((FileAttr & Attr ) == Attr && (FileAttr & AttrMask) ==  0)	{
				    LOGD("FindFirst: FsPatternName = <%s> ... \n", FsPatternName);
					bFound = true;
					break;
				} else {
					LOGD("FindFirst: FsPatternName = <%s> not match:fileAttr=%d, Attr=%d,AttrMask=%d\n",
						FsPatternName, FileAttr, Attr, AttrMask);
				}
			}
		}

        if(bFound == true)
        {
    		if((HandleIndex = FS_GetFreeHandle()) == 0xFFFFFFFF)
    		{
				LOGE("FindFirst: [error]fail get handle index \n");
				ret = FS_TOO_MANY_FILES;
				goto Exit;
    		}
    		dbg_printf("FindFirst: FileNum:%d, HandleIndex:%d \n", g_FsInfo.FileNum,HandleIndex);
		    g_FsInfo.FileNum++;
		    g_FsInfo.hFileHandle[HandleIndex].hFile = (int)pDir;
		    g_FsInfo.hFileHandle[HandleIndex].fInUse = true;
		    g_FsInfo.hFileHandle[HandleIndex].fSearch = true;
		    g_FsInfo.hFileHandle[HandleIndex].Attr = Attr;
		    g_FsInfo.hFileHandle[HandleIndex].AttrMask = AttrMask;
		    g_FsInfo.hFileHandle[HandleIndex].pFsFileName = (char*)malloc(sizeof(char)*(strlen(FsPatternName)+1));
		    if(g_FsInfo.hFileHandle[HandleIndex].pFsFileName == NULL) {
				LOGE("FindFirst: [error]fail alloc memory for FileName \n");
				ret = FS_ERROR_RESERVED;
				goto Exit;
		    }
			memcpy(g_FsInfo.hFileHandle[HandleIndex].pFsFileName, FsPatternName, sizeof(char)*(strlen(FsPatternName)+1));

			g_FsInfo.hFileHandle[HandleIndex].pFsSearchPattern = (char*)malloc(sizeof(char)*(strlen(FsFileName)+1));
            if(g_FsInfo.hFileHandle[HandleIndex].pFsSearchPattern == NULL) {
				LOGE("FindFirst: [error]fail alloc memory for SearchPattern \n");
        		ret = FS_ERROR_RESERVED;
				goto Exit;
            }
			memcpy(g_FsInfo.hFileHandle[HandleIndex].pFsSearchPattern, FsFileName, sizeof(char)*(strlen(FsFileName)+1));				

			FS_EntryLinuxToDos(pFileInfo, FsPatternName);

		    *pMaxLength = FS_ConvCsToWcs(pDirent->d_name, FileName, max_len);
			if(strlen(pDirent->d_name) < max_len)
				pFileInfo->NTReserved = FS_LFN_MATCH;
			else
				pFileInfo->NTReserved = FS_NOT_MATCH;
	}  	else {
        *pMaxLength = 0;
        LOGE("FindFirst: [error]no more files...\n");

		if(pDir != NULL) {
			if(closedir(pDir) != 0) {
				LOGE("FindFirst: [error]fail close dir %s: %d \n", FsDirName, errno);
			}
		}
		ret = FS_NO_MORE_FILES;
		goto Exit;
    }

	ret = HandleIndex;

Exit:
	if(pDirent == NULL) {
		len1 = strlen(FsDirName);
		if(len1>=8) {
			LOGD("FF: [%02X%02X%02X%02X%02X%02X%02X%02X](%d %d %d): [null] %d \n",
			(FsDirName[len1-1]-32),(FsDirName[len1-2]-32),(FsDirName[len1-3]-32),(FsDirName[len1-4]-32),
			(FsDirName[len1-5]-32),(FsDirName[len1-6]-32),(FsDirName[len1-7]-32),(FsDirName[len1-8]-32),
			Attr, AttrMask, *pMaxLength, ret);
		} else {
			LOGD("FF: len1 < 8(%d)\n", len1);
		}
	} else {
		len1 = strlen(FsDirName);
		len2 = strlen(pDirent->d_name);
		if( (len2>=8)&&(len1>=8)) {
			LOGD("FF: [%02X%02X%02X%02X%02X%02X%02X%02X](%d %d %d): [%02X%02X%02X%02X%02X%02X%02X%02X] %d \n", 
			(FsDirName[len1-1]-32),(FsDirName[len1-2]-32),(FsDirName[len1-3]-32),(FsDirName[len1-4]-32),
			(FsDirName[len1-5]-32),(FsDirName[len1-6]-32),(FsDirName[len1-7]-32),(FsDirName[len1-8]-32),
			Attr, AttrMask, *pMaxLength,
			(pDirent->d_name[len2-1]-32),(pDirent->d_name[len2-2]-32),(pDirent->d_name[len2-3]-32),(pDirent->d_name[len2-4]-32),
			(pDirent->d_name[len2-5]-32),(pDirent->d_name[len2-6]-32),(pDirent->d_name[len2-7]-32),(pDirent->d_name[len2-8]-32),
			ret);
		} else {
			LOGD("FF: len1 %d, len2 %d\n", len1, len2);
		}
    }

	return ret;
}

static int FS_CCCI_FindNext(int HandleIndex, FS_DOSDirEntry *pFileInfo, wchar_t* FileName, unsigned int *pMaxLength) 
{
	static char FsPatternName[PATH_MAX] = {0};
	bool bFound = false;
	DIR* 	pDir;
	struct dirent*	pDirent = NULL;
	char FileAttr, Attr, AttrMask;
	char* pSearchPattern;
	int i;
	char ConvFileName[PATH_MAX];
	unsigned int max_len = *pMaxLength;
	int ret = FS_GENERAL_FAILURE;
	int len = 0;

	*pMaxLength = 0;
	FS_ConvWcsToCs(FileName, ConvFileName);

	if ((unsigned int) HandleIndex >= FS_FILE_MAX) {
		LOGE("Close: [error] Handle index %d out-of-border.\n", HandleIndex);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	} else if(g_FsInfo.FileNum == 0 ||
		g_FsInfo.hFileHandle[HandleIndex].hFile == INVALID_HANDLE_VALUE ||
		g_FsInfo.hFileHandle[HandleIndex].fInUse != true)
	{
		LOGE("FindNext: [error]Invalid handle for %s: %d, %d, %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex,
			g_FsInfo.hFileHandle[HandleIndex].hFile, g_FsInfo.hFileHandle[HandleIndex].fInUse);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	}

	pDir = (DIR*)g_FsInfo.hFileHandle[HandleIndex].hFile;
	pSearchPattern = g_FsInfo.hFileHandle[HandleIndex].pFsSearchPattern;
	Attr = g_FsInfo.hFileHandle[HandleIndex].Attr;
	AttrMask = g_FsInfo.hFileHandle[HandleIndex].AttrMask;

	snprintf(FsPatternName, PATH_MAX, "%s", g_FsInfo.hFileHandle[HandleIndex].pFsFileName);

	for(i = strlen(FsPatternName); i > 0; i--)
	{
		if(FsPatternName[i] == '/')
			break;
	}

	if(i <= 0) {
		LOGE("FindNext: [error]invalid file path %s \n", FsPatternName);
		ret = FS_PATH_NOT_FOUND;
		goto Exit;
	}

	i = i+1; // '/' should be excluded.

	while((pDirent = readdir(pDir))!= NULL)
	{
		//check if file name match
		if(PatMatch(pSearchPattern, pDirent->d_name))
		{
			if(i + strlen(pDirent->d_name) > PATH_MAX) {
				LOGE("FindNext: [error]file path[0] too long:%s \n", pDirent->d_name);
				ret = FS_PATH_OVER_LEN_ERROR;
				goto Exit;
			}
			snprintf((FsPatternName+i), (PATH_MAX - i), "%s", pDirent->d_name);

			FileAttr = FS_AttrReconv(FsPatternName);
			if((FileAttr & Attr ) == Attr && (FileAttr & AttrMask) ==  0)	{
			    LOGD("FindNext: FsPatternName = <%s> ... \n", FsPatternName);
				bFound = true;
				break;
			} else {
				LOGD("FindNext: FsPatternName = <%s> not match:fileAttr=%d, Attr=%d,AttrMask=%d\n",
					FsPatternName, FileAttr, Attr, AttrMask);
			}
		}
	}

    if(bFound == true)
    {
	    free(g_FsInfo.hFileHandle[HandleIndex].pFsFileName);

	    g_FsInfo.hFileHandle[HandleIndex].pFsFileName = (char*)malloc(sizeof(char)*(strlen(FsPatternName)+1));				
	    if(g_FsInfo.hFileHandle[HandleIndex].pFsFileName == NULL) {
			LOGE("FindNext: [error]fail alloc memory for FileName \n");
			ret = FS_ERROR_RESERVED;
			goto Exit;
	    }

		memcpy(g_FsInfo.hFileHandle[HandleIndex].pFsFileName, FsPatternName, sizeof(char)*(strlen(FsPatternName)+1));

		FS_EntryLinuxToDos(pFileInfo, FsPatternName);

	    *pMaxLength = FS_ConvCsToWcs(pDirent->d_name, FileName, max_len);
		if(strlen(pDirent->d_name) < max_len)
			pFileInfo->NTReserved = FS_LFN_MATCH;
		else
			pFileInfo->NTReserved = FS_NOT_MATCH;

		ret = FS_NO_ERROR;
    }
    else
    {
    	LOGE("FindNext: [error]no more files...\n");
        *pMaxLength = 0;
    	ret = FS_NO_MORE_FILES;
    }

Exit:
	if(pDirent == NULL)
	{
		LOGD("FN: %d %d: [NULL] %d \n", HandleIndex, *pMaxLength, ret);
    }		
	else
	{
		len = strlen(pDirent->d_name);
		if(len>=8) {
			LOGD("FN: %d %d: [%02X%02X%02X%02X%02X%02X%02X%02X] %d \n", HandleIndex, *pMaxLength, 
			(pDirent->d_name[len-1]-32),(pDirent->d_name[len-2]-32),(pDirent->d_name[len-3]-32),(pDirent->d_name[len-4]-32),
			(pDirent->d_name[len-5]-32),(pDirent->d_name[len-6]-32),(pDirent->d_name[len-7]-32),(pDirent->d_name[len-8]-32),
			ret);
		} else {
			LOGD("FN: stren < 8(%d)\n", len);
		}
    }		
	return ret;
}

static int FS_CCCI_FindClose(int HandleIndex)
{		  		
	int ret = FS_GENERAL_FAILURE;		
	int local_errno;

	if ((unsigned int) HandleIndex >= FS_FILE_MAX) {
		LOGE("Close: [error] Handle index %d out-of-border.\n", HandleIndex);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	} else if(g_FsInfo.FileNum == 0 ||
			g_FsInfo.hFileHandle[HandleIndex].hFile == INVALID_HANDLE_VALUE ||
			g_FsInfo.hFileHandle[HandleIndex].fInUse != true)
	{
		LOGE("FindClose: [error]Invalid handle for %s: %d, %d, %d \n",
			g_FsInfo.hFileHandle[HandleIndex].pFsFileName, HandleIndex,
			g_FsInfo.hFileHandle[HandleIndex].hFile, g_FsInfo.hFileHandle[HandleIndex].fInUse);
		ret = FS_INVALID_FILE_HANDLE;
		goto Exit;
	}

	LOGD("FindClose:%s: (%d, %d)\n", g_FsInfo.hFileHandle[HandleIndex].pFsFileName,
		HandleIndex, g_FsInfo.hFileHandle[HandleIndex].hFile);

	if(g_FsInfo.hFileHandle[HandleIndex].fSearch)
	{
		if(closedir((DIR*)g_FsInfo.hFileHandle[HandleIndex].hFile) != 0)
		{
			local_errno = errno;
			LOGE("FindClose: [error]fail close Dir %s: %d \n", g_FsInfo.hFileHandle[HandleIndex].pFsFileName, errno);
			ret = FS_ErrorConv(local_errno);
			goto Exit;
		}
	}
	else
	{
		if(close(g_FsInfo.hFileHandle[HandleIndex].hFile) == -1)
		{
			local_errno = errno;
			LOGE("FindClose: [error]fail close Dir %s: %d \n", g_FsInfo.hFileHandle[HandleIndex].pFsFileName, errno);
			ret = FS_ErrorConv(local_errno);
			goto Exit;
		}
	}

	g_FsInfo.hFileHandle[HandleIndex].hFile = INVALID_HANDLE_VALUE;
	g_FsInfo.hFileHandle[HandleIndex].fInUse = false;

	if(g_FsInfo.hFileHandle[HandleIndex].pFsFileName != NULL)
	    free(g_FsInfo.hFileHandle[HandleIndex].pFsFileName);

	if(g_FsInfo.hFileHandle[HandleIndex].pFsSearchPattern != NULL)
	    free(g_FsInfo.hFileHandle[HandleIndex].pFsSearchPattern);
	g_FsInfo.FileNum--;

	ret = FS_NO_ERROR;

Exit:
	LOGD("FC: %d: %d \n", HandleIndex, ret);
	return ret;
}

static int FS_CCCI_GetDrive(unsigned int Type, unsigned int Serial, unsigned int AltMask)
{
	int DriveMap[5];
	int ret = FS_GENERAL_FAILURE;

	if(((Type & (FS_DRIVE_I_SYSTEM | FS_DRIVE_V_NORMAL | FS_DRIVE_V_REMOVABLE)) == 0) ||
		 ((Type & ~(FS_DRIVE_I_SYSTEM | FS_DRIVE_V_NORMAL | FS_DRIVE_V_REMOVABLE)) != 0) ||
		 ((Type != FS_DRIVE_V_REMOVABLE) && (Serial > 2)) ||
		 ((Type == FS_DRIVE_V_REMOVABLE) && (Serial > 1)) ||
		 (Serial < 1) ||
		 ((AltMask != FS_NO_ALT_DRIVE) && (AltMask & FS_NO_ALT_DRIVE)) ||
		 ((AltMask != FS_ONLY_ALT_SERIAL) && (AltMask & FS_ONLY_ALT_SERIAL))) {
		LOGE("GetDrive: [error]Invalid parameter:type=0x%x, serial=0x%x, AltMask=0x%x\n",
			Type, Serial, AltMask);
		ret = FS_PARAM_ERROR;
		goto Exit;
	}

	//We only support system drive
	DriveMap[0] = 'Z';

	if(Type == FS_DRIVE_I_SYSTEM)
		ret = DriveMap[0];
	else {
		LOGE("GetDrive: [error]type not FS_DRIVE_I_SYSTEM: 0x%x \n", Type);
		ret = FS_PARAM_ERROR;
	}

Exit:
	LOGD("GD: %d %d %d: %d \n", Type, Serial, AltMask, ret);
	return ret;
}

static void FS_CCCI_ShutDown()
{
	FS_CCCI_CloseAll();
}

static int FS_CCCI_GetDiskInfo(FS_DiskInfo *pDiskInfo)
{
    struct statfs StatfsBuf;

    //dbg_printf("GetDiskInfo: DriveName=%s\n", DriveName);
	
    if(statfs(FsRootDir[0], &StatfsBuf) == -1) {
       LOGE("GetDiskInfo: [error]fail get statfs for %s \n", FsRootDir[0]);
       return -1;
    }

    pDiskInfo->BytesPerSector = 512;
    pDiskInfo->SectorsPerCluster = StatfsBuf.f_bsize / 512;
    pDiskInfo->TotalClusters = StatfsBuf.f_blocks;
    //pDiskInfo->FreeClusters = StatfsBuf.f_bfree;
    pDiskInfo->FreeClusters = 512;

    LOGD("I: %s: %d, %d, %d\n", FsRootDir[0], pDiskInfo->SectorsPerCluster, pDiskInfo->TotalClusters, 
         pDiskInfo->FreeClusters);

    return FS_NO_ERROR;
}

/*
FS_CCCI_Restore: Modem NVRAM could restore file from AP BIN region via this API
pBuf, Len£ºReserved for future use
*/
static int FS_CCCI_Restore(const wchar_t* FileName)
{
    int ret = FS_GENERAL_FAILURE;
    char FsFileName[PATH_MAX] = {0};
    char ConvFileName[PATH_MAX] = {0};
    int i = 0;

    FS_ConvWcsToCs(FileName, ConvFileName);

    for (i = 0; i < FS_MAX_DIR_NUM; i++) {
        if(strncmp(ConvFileName, MdRootDir[i], strlen(MdRootDir[i])) == 0) {
            if((strlen(ConvFileName + strlen(MdRootDir[i])) + strlen(FsRootDir[i]) + 1) > PATH_MAX) {
                LOGE("Restore: [error]file path too long: %s \n", ConvFileName);
                ret = FS_PATH_OVER_LEN_ERROR;
                goto _Exit;
                }
                //Convert the file name
                snprintf(FsFileName, sizeof(FsFileName), "%s%s", FsRootDir[i], (ConvFileName + strlen(MdRootDir[i])));
                break;
            }
        }

    LOGD("try to RS file:%s\n", FsFileName);
    if (i == FS_MAX_DIR_NUM) {
        LOGE("Restore: [error]file path not find %s \n", ConvFileName);
        ret = FS_PATH_NOT_FOUND;
        goto _Exit; 
    }

    if(false == NVM_RestoreFromBinRegion_OneFile(-1, FsFileName)) {
        LOGE("Restore Fail: %s \n", FsFileName);
        goto _Exit;
    }

    LOGD("RS %s PASS\n", FsFileName);

    ret = FS_NO_ERROR;
_Exit:	
    return ret;
}


/*cmpt_read*/
static int FS_CCCI_CMPT_Read(wchar_t* name, nvram_fs_para_cmpt_t *in_para)
{
    unsigned int HandleIndex;
    int          ret[FS_CMPT_MAX_OPID_NUM];
    unsigned int ret_bitmap = 0;
#ifdef PROFILE_NVRAM_API
	struct timespec time_start, time_end;
    long diff;
	int elapsed_in_ms;
#endif

    in_para->act_read = 0; // Init this first

    if(in_para->opid_map & FS_CMPT_OPEN) {

#ifdef PROFILE_NVRAM_API
	clock_gettime(CLOCK_REALTIME, &time_start);
#endif
        HandleIndex =  FS_CCCI_Open(name, in_para->flag);
#ifdef PROFILE_NVRAM_API
	if (slower_100ms.profiling)
	{
		clock_gettime(CLOCK_REALTIME, &time_end);

		diff = (long)(time_end.tv_sec - time_start.tv_sec) * 1000;
		diff += (long)(time_end.tv_nsec - time_start.tv_nsec) / 1000000;
		elapsed_in_ms = (int)diff;

		/*LOGD("profile: debug: elapsed_in_ms: %ld %ld %ld %ld, %d", time_start.tv_sec, time_start.tv_nsec,
				time_end.tv_sec, time_end.tv_nsec, elapsed_in_ms);*/

		if (elapsed_in_ms > 100)
		{
			slower_100ms.count++;
			slower_100ms.totaltime += elapsed_in_ms;
		}
	}
#endif
        ret[0] = HandleIndex;
        if(ret[0] < 0) {
            LOGE("cmpt_Read, open fail\n");
            in_para->ret[0] = ret_bitmap;
            in_para->ret[1] = HandleIndex;
            return -1;
        } else {
	    ret_bitmap |= FS_CMPT_OPEN;
        }

        // Open success, check size if need
        if(in_para->opid_map & FS_CMPT_GETFILESIZE) {
#ifdef PROFILE_NVRAM_API
	clock_gettime(CLOCK_REALTIME, &time_start);
#endif
            ret[1] =  FS_CCCI_GetFileSize(HandleIndex, &(in_para->file_size));
#ifdef PROFILE_NVRAM_API
	if (slower_100ms.profiling)
	{
		clock_gettime(CLOCK_REALTIME, &time_end);

		diff = (long)(time_end.tv_sec - time_start.tv_sec) * 1000;
		diff += (long)(time_end.tv_nsec - time_start.tv_nsec) / 1000000;
		elapsed_in_ms = (int)diff;

		/*LOGD("profile: debug: elapsed_in_ms: %ld %ld %ld %ld, %d", time_start.tv_sec, time_start.tv_nsec,
				time_end.tv_sec, time_end.tv_nsec, elapsed_in_ms);*/
		if (elapsed_in_ms > 100)
		{
			slower_100ms.count++;
			slower_100ms.totaltime += elapsed_in_ms;
		}
	}
#endif
            if(ret[1] < 0) {
                LOGE("cmpt_Read, GetSize fail\n");
                in_para->ret[0] = ret_bitmap;
                in_para->ret[1] = ret[1];
                FS_CCCI_Close(HandleIndex);
                return -2;
            } else {
                ret_bitmap |= FS_CMPT_GETFILESIZE;
            }
        }

        // Open success, Seek file if need
        if(in_para->opid_map & FS_CMPT_SEEK) {
            ret[2] =  FS_CCCI_Seek(HandleIndex, in_para->offset, in_para->whence);
            if(ret[2] < 0) {
                LOGE("cmpt_Read, Seek fail\n");
                in_para->ret[0] = ret_bitmap;
                in_para->ret[1] = ret[2];
                FS_CCCI_Close(HandleIndex);
                return -3;
            } else {
                ret_bitmap |= FS_CMPT_SEEK;
            }
        }

        // Open success, read if need
        if(in_para->opid_map & FS_CMPT_READ) {
            ret[3] =  FS_CCCI_Read(HandleIndex, in_para->data_ptr, in_para->length, &in_para->act_read);
            if(ret[3] < 0) {
                LOGE("cmpt_Read, read fail\n");
                in_para->ret[0] = ret_bitmap;
                in_para->ret[1] = ret[3];
                FS_CCCI_Close(HandleIndex);
                return -4;
            } else {
                ret_bitmap |= FS_CMPT_READ;
            }
        }

	// Open success, Close file
        if(in_para->opid_map & FS_CMPT_CLOSE) {
            ret[4] =  FS_CCCI_Close(HandleIndex);
            if(ret[4] < 0) {
                LOGE("cmpt_Read, close fail\n");
                in_para->ret[0] = ret_bitmap;
                in_para->ret[1] = ret[4];
                return -5;
            } else {
                ret_bitmap |= FS_CMPT_CLOSE;
            }
        } else {
            LOGE("cmpt_Read, miss close flag\n");
            FS_CCCI_Close(HandleIndex);
            return -6;
        }
        
	// All success
        in_para->ret[0] = ret_bitmap;
        in_para->ret[1] = 0;

        return 0;
    } else {
        LOGE("cmpt_Read, miss read flag\n");
        in_para->ret[0] = ret_bitmap;
        in_para->ret[1] = FS_GENERAL_FAILURE;

        return -7;
    }
}

/* cmpt_write */
static int FS_CCCI_CMPT_Write(wchar_t* name, nvram_fs_para_cmptw_t *in_para, void* pBuffer)
{
    int HandleIndex = -1;
    int          ret[FS_CMPT_MAX_OPID_NUM];
    unsigned int ret_bitmap = 0;
#ifdef PROFILE_NVRAM_API
	struct timespec time_start, time_end;
    long diff;
	int elapsed_in_ms;
#endif

    in_para->act_write_size = 0; // Init this first
	if (in_para->file_handle < 0) {
		//first cmpt_write
		if (in_para->opid_map & FS_CMPT_OPEN) {

#ifdef PROFILE_NVRAM_API
		clock_gettime(CLOCK_REALTIME, &time_start);
#endif
        HandleIndex =  FS_CCCI_Open(name, in_para->flag);
#ifdef PROFILE_NVRAM_API
			if (slower_100ms.profiling)
			{
				clock_gettime(CLOCK_REALTIME, &time_end);

				diff = (long)(time_end.tv_sec - time_start.tv_sec) * 1000;
				diff += (long)(time_end.tv_nsec - time_start.tv_nsec) / 1000000;
				elapsed_in_ms = (int)diff;

				/*LOGD("profile: debug: elapsed_in_ms: %ld %ld %ld %ld, %d", time_start.tv_sec, time_start.tv_nsec,
						time_end.tv_sec, time_end.tv_nsec, elapsed_in_ms);*/

				if (elapsed_in_ms > 100)
				{
					slower_100ms.count++;
					slower_100ms.totaltime += elapsed_in_ms;
				}
			}
#endif
	        ret[0] = HandleIndex;
	        if(ret[0] < 0) {
	            LOGE("cmpt_Write, open fail\n");
	            in_para->ret[0] = ret_bitmap;
	            in_para->ret[1] = HandleIndex;
	            return -1;
	        } else {
              in_para->file_handle_ptr = HandleIndex;
              ret_bitmap |= FS_CMPT_OPEN;
	        }
      } else {
          LOGE("cmpt_Write, error: no FS_CMPT_OPEN, no file_handle\n");
          in_para->ret[0] = ret_bitmap;
          in_para->ret[1] = FS_PARAM_ERROR;
          return -2;
      }
    } else {
        HandleIndex = in_para->file_handle;
        //response file_handle to md
        in_para->file_handle_ptr = HandleIndex;
    }
    // Open success, Seek file if need
    if(in_para->opid_map & FS_CMPT_SEEK) {
        ret[2] =  FS_CCCI_Seek(HandleIndex, in_para->offset, in_para->whence);
        if(ret[2] < 0) {
            LOGE("cmpt_Write, Seek fail\n");
            in_para->ret[0] = ret_bitmap;
            in_para->ret[1] = ret[2];
            FS_CCCI_Close(HandleIndex);
            return -3;
        } else {
            ret_bitmap |= FS_CMPT_SEEK;
        }
    }

    // Open success, write if need
    if(in_para->opid_map & FS_CMPT_WRITE) {
        ret[3] =  FS_CCCI_Write(HandleIndex, pBuffer, in_para->length, &in_para->act_write_size);
        if(ret[3] < 0) {
            LOGE("cmpt_Write, write fail\n");
            in_para->ret[0] = ret_bitmap;
            in_para->ret[1] = ret[3];
            FS_CCCI_Close(HandleIndex);
            return -4;
        } else {
            ret_bitmap |= FS_CMPT_WRITE;
        }
    }

    // Open success, Close file
    if(in_para->opid_map & FS_CMPT_CLOSE) {
        ret[4] =  FS_CCCI_Close(HandleIndex);
        if(ret[4] < 0) {
            LOGE("cmpt_Write, close fail\n");
            in_para->ret[0] = ret_bitmap;
            in_para->ret[1] = ret[4];
            return -5;
        } else {
            ret_bitmap |= FS_CMPT_CLOSE;
        }
    }

    // All success
    in_para->ret[0] = ret_bitmap;
    in_para->ret[1] = 0;

    return 0;
}

static int FS_BinRegion_Access(int access_type)
{
	int ret = FS_NO_ERROR;

	switch (access_type) {
	case FS_BIN_REGION_BACKUP:
		ret = FS_PARAM_ERROR;
		LOGE("%s backup bin_region unsupport\n", __func__);
		break;
	case FS_BIN_REGION_RESTORE:
		ret = FS_PARAM_ERROR;
		LOGE("%s restore bin_region unsupport\n", __func__);
		break;
	case FS_BIN_REGION_ERASE:
		ret = FS_PARAM_ERROR;
		LOGE("%s erase bin_region unsupport\n", __func__);
		break;
	default:
		ret = FS_PARAM_ERROR;
		LOGE("%s access type=%d not support\n", __func__, access_type);
		break;
	}
	return ret;
}
void FS_Init(int md_id)
{
	int i;
	int ret = 0;
	struct stat buf;
	g_FsInfo.FileNum = 0;
	g_FsInfo.bNonAck = false;

	umask(0007);
	ret = stat("/mnt/vendor/nvdata",&buf);
	if (ret < 0) {
		LOGD("%s:stat /mnt/vendor/nvdata failed, ret=%d, errno=%d.\n", __func__, ret, errno);
		ret = mkdir("/mnt/vendor/nvdata",MD_DIR_ATTR);
		if (ret < 0)
			LOGE("%s:mkdir /mnt/vendor/nvdata failed,ret=%d, errno=%d.\n", __func__, ret, errno);
	}
	ret = stat(FS_ROOT_COMMON,&buf);
	if (ret < 0) {
		LOGD("%s:stat %s dir error, ret=%d errno=%d.\n",
			__func__, FS_ROOT_COMMON, ret, errno);
		ret = mkdir(FS_ROOT_COMMON, MD_DIR_ATTR);
		if (ret < 0)
			LOGE("%s:mkdir %s failed,ret=%d,errno=%d.\n",
				__func__, FS_ROOT_COMMON, ret, errno);
	}

	switch(md_id) {
	case 0:
		ret = stat(FS_ROOT_DIR_MD1, &buf);
		if (ret < 0) {
			ret = mkdir(FS_ROOT_DIR_MD1, MD_DIR_ATTR);
			if (ret < 0)
				LOGE("%s:mkdir %s for md1 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR_MD1, ret, errno);
			else
				LOGW("No %s dir, mkdir done.\n", FS_ROOT_DIR_MD1);
		}

		if (stat(FS_ROOT_DIR1_MD1 ,&buf)<0) {
			ret = mkdir(FS_ROOT_DIR1_MD1, MD_DIR_ATTR);
			if (ret < 0)
				LOGE("%s:mkdir %s for md1 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR1_MD1, ret, errno);
			else
				LOGW("No %s dir, mkdir done.\n", FS_ROOT_DIR1_MD1);
		}

		if (stat(FS_ROOT_DIR2_MD1,&buf)<0) {
			ret = mkdir(FS_ROOT_DIR2_MD1, MD_DIR_ATTR);
			if (ret < 0)
				LOGE("%s:mkdir %s for md1 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR2_MD1, ret, errno);
			else
				LOGW("No %s dir, mkdir done.\n", FS_ROOT_DIR2_MD1);
		}
		break;

	case 1:
		if (stat(FS_ROOT_DIR_MD2, &buf)<0) {
			LOGD("No %s dir.\n", FS_ROOT_DIR_MD2);
			ret = mkdir(FS_ROOT_DIR_MD2, MD_DIR_ATTR);
			if (ret < 0)
				LOGE("%s:mkdir %s for md2 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR_MD2, ret, errno);
		}

		if (stat(FS_ROOT_DIR1_MD2 ,&buf)<0)	{
			LOGD("No %s dir.\n", FS_ROOT_DIR1_MD2);
			ret = mkdir(FS_ROOT_DIR1_MD2, MD_DIR_ATTR);
			if (ret < 0)
				LOGE("%s:mkdir %s for md2 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR1_MD2, ret, errno);
		}
		if (stat(FS_ROOT_DIR2_MD2,&buf)<0) {
			LOGD("No %s dir.\n", FS_ROOT_DIR2_MD2);
			ret = mkdir(FS_ROOT_DIR2_MD2, MD_DIR_ATTR);
			if (ret < 0)
				LOGE("%s:mkdir %s for md2 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR2_MD2, ret, errno);
		}
		break;
	case 2:
		LOGD("checking md3 nvram folders...!\n");
		if (stat(FS_ROOT_DIR_MD3, &buf)<0) {
			LOGD("No %s dir.\n", FS_ROOT_DIR_MD3);
			if (mkdir(FS_ROOT_DIR_MD3, MD_DIR_ATTR)<0)
				LOGD("%s:mkdir %s for md3 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR_MD3, ret, errno);
		}

		if (stat(FS_ROOT_DIR1_MD3 ,&buf)<0) {
			LOGD("No %s dir.\n", FS_ROOT_DIR1_MD3);
			if (mkdir(FS_ROOT_DIR1_MD3, MD_DIR_ATTR)<0)
				LOGE("%s:mkdir %s for md3 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR1_MD3, ret, errno);
		}

		if (stat(FS_ROOT_DIR2_MD3,&buf)<0) {
			LOGD("No %s dir.\n", FS_ROOT_DIR2_MD3);
			if (mkdir(FS_ROOT_DIR2_MD3, MD_DIR_ATTR)<0)
				LOGE("%s:mkdir %s for md3 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR2_MD3, ret, errno);
		}

		if (stat(FS_ROOT_DIR3_MD3,&buf)<0) {
			LOGD("No %s dir.\n", FS_ROOT_DIR3_MD3);
			if (mkdir(FS_ROOT_DIR3_MD3, MD_DIR_ATTR)<0)
				LOGE("%s:mkdir %s for md3 failed,ret=%d, errno=%d.\n",
					__func__, FS_ROOT_DIR3_MD3, ret, errno);
		}
		break;
	default:
		LOGE("Invalid MD ID.\n");
		break;
	}


	for(i = 0; i < FS_FILE_MAX; i++)
	{
		g_FsInfo.hFileHandle[i].fInUse = false;
		g_FsInfo.hFileHandle[i].hFile = INVALID_HANDLE_VALUE;
	}
}

void FS_Deinit(void)
{
		FS_CCCI_ShutDown();
}

static int exit_signal = 0;
void signal_treatment(int param)
{
	/*
	 * this signal catching design does NOT work...
	 * set property ctl.stop will send SIGKILL to ccci_fsd(check service_stop_or_reset() in init.c),
	 * but SIGKILL is not catchable.
	 * kill pid will send SIGTERM to ccci_fsd, we can catch this signal, but the process is just
	 * terminated, and no time for us to check exit_signal in main().
	 * per system team's comment, kernel will free all resource (memory get from malloc, etc.),
	 * so we do NOT need to take care of these.
	 */
    LOGD("signal number=%d\n", param);
    switch (param) {
        case SIGPIPE:
	case SIGHUP:
	case SIGINT:
	case SIGTERM:
	case SIGUSR1:
	case SIGUSR2:	
	case SIGALRM:
        case SIGKILL:
        default:
        exit_signal = param;
        break;
    }
}

static int ccci_open_md_status_fd(int md_id)
{
	char *dev_path;
	int fd = -1;

	if (!md_id)
		dev_path = "/dev/ccci_md1_sta";

	else if (md_id == 2)
		dev_path = "/dev/ccci_md3_sta";

	else {
		LOGE("errr: incorrect md id %d\n", md_id);
		return -2;
	}

	fd = open(dev_path, O_RDWR);
	if (fd < 0) {
		LOGE("error: open %s fail: %d\n", dev_path, errno);
		return -3;
	}

	return fd;
}

static void ccci_wait_md_status_to_HS1()
{
	int count;
	struct md_status_event status_buf;

	LOGD("[%s] start. md_status_fd = %d", __func__, md_status_fd);
	while (1) {
		count = read(md_status_fd, &status_buf, sizeof(struct md_status_event));
		if (count > 0) {
			LOGD("[%s] curr md status = %d", __func__, status_buf.event_type);

			if (status_buf.event_type == MD_STA_EV_HS1)
				break;

		} else
			LOGE_COM("[%s] warning, read() = %d", __func__, count);

	}
	LOGD("[%s] end.", __func__);
}

static void ccci_status_thread_handle()
{
	int count;
	struct md_status_event status_buf;

	LOGD("[%s] start. md_status_fd = %d", __func__, md_status_fd);
	while (1) {
		count = read(md_status_fd, &status_buf, sizeof(struct md_status_event));
		if (count > 0) {
			LOGD("[%s] curr md status = %d", __func__, status_buf.event_type);

			if (status_buf.event_type == MD_STA_EV_STOP) {
				if (DeviceFd >= 0) {
					close(DeviceFd);
					DeviceFd = -1;
				}

				LOGE_COM("[%s] md status is: MD_STA_EV_STOP, exit.", __func__);
				exit (1);
			}

		} else
			LOGE_COM("[%s] warning, read() = %d", __func__, count);

	}
}

static void ccci_create_md_status_listen_thread()
{
	int ret;

	ret = pthread_create(&thread_id, NULL, (void *)ccci_status_thread_handle, NULL);
	if(ret != 0) {
		LOGE_COM("[%s] error: pthread_create() = %d", __func__, ret);
		exit (1);

	}
	//pthread_join(thread_id, NULL);
}

int main(int argc, char *argv[])
{
	int mdstatus;
	int ReqBufIndex = 0;
	FS_BUF *pFsBuf;
	int PacketNum = 0;
	int RetVal;
	FS_PACKET_INFO *PackInfo;
	FS_DOSDirEntry DosDirEntry;
	FS_DiskInfo DiskInfo;
	static wchar_t FileNameTemp[PATH_MAX] = {0};
	unsigned int Length;
	char dev_node[32];
	int  using_old_ver = 0;
	int  port_open_retry = 600;
	int i = 0;
	CCCI_BUFF_T *ccci_h = NULL;
	char *pkt_buff;//[MAX_FS_BUF_BYTE] = {0};
	STREAM_DATA *stream = NULL; // data packet received from MD
	STREAM_DATA *buffer_slot = NULL; // local buffer slot
	char *p_fs_buff = NULL;
	nvram_fs_para_cmpt_t in_cmpt_para;
	nvram_fs_para_cmptw_t in_cmptw_para;
	FS_FileDetail FileDetail;
	int WriteByte = 0;
	unsigned int ReadByte = 0;
	unsigned int FileSize = 0;
	char ConvFileName[PATH_MAX];

	LOGE_COM("md_fsd Ver:v2.3, CCCI Ver:%d", ccci_get_version());

	//Check if input parameter is valid
	if(argc != 2) {
		md_id = 0;
		LOGE("[Warning]Parameter number not correct,use old version!\n");
		using_old_ver = 1;
		snprintf(dev_node, 32, "/dev/ccci_fs");
		for (i = 0; i < FS_MAX_DIR_NUM; i++) {
			snprintf(FsRootDir[i], 36, "%s", FsRootDir_MD1[i]);
		}
	} else {
		if(strcmp(argv[1],"0")==0) {
			snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_CCCI_FS, MD_SYS1));
			for (i = 0; i < FS_MAX_DIR_NUM; i++) {
				snprintf(FsRootDir[i], 36, "%s", FsRootDir_MD1[i]);
			}
			md_id = 0;
		} else if(strcmp(argv[1],"1")==0) {
			snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_CCCI_FS, MD_SYS2));
			for (i = 0; i < FS_MAX_DIR_NUM; i++) {
				snprintf(FsRootDir[i], 36, "%s", FsRootDir_MD2[i]);
			}
			md_id =1;
		} else if (strcmp(argv[1],"2")==0) {
			snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_CCCI_FS, MD_SYS3));
			for (i = 0; i < FS_MAX_DIR_NUM; i++) {
				snprintf(FsRootDir[i], 36, "%s", FsRootDir_MD3[i]);
			}
			md_id =2;
		} else if(strcmp(argv[1],"4")==0) {
			snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_CCCI_FS, MD_SYS5));
			for (i = 0; i < FS_MAX_DIR_NUM; i++) {
				snprintf(FsRootDir[i], 36, "%s", FsRootDir_MD5[i]);
			}
			md_id =4;
		} else {
			LOGE_COM("Invalid md sys id(%d)!\n", md_id);
			return -1;
		}
	}
	if(md_id == 0 || md_id == 1 || md_id == 2) {
		if(ccci_get_version() == ECCCI || ccci_get_version() == EDSDA || ccci_get_version() == ECCCI_FSM)
			stream_support = 1;
		else
			stream_support = 0;
	} else if(md_id == 4) {
		stream_support = 1;
	}
#ifdef ENABLE_DEBUG_LOG
	get_debug_log_level();
#endif

	md_status_fd = ccci_open_md_status_fd(md_id);
	if (md_status_fd < 0)
		return -1;

	ccci_wait_md_status_to_HS1();

	// Retry to open if dev node attr not ready
	while(1) {
		DeviceFd = open(dev_node, O_RDWR);
		if (DeviceFd < 0) {
			/*
			if(errno != EACCES) { // EACCES(13) means permission deny
				LOGE("%s is not enabled(%d).", dev_node, errno);
				perror("");
				return -1;
			}
			*/
			port_open_retry--;
			if(port_open_retry>0) {
				usleep(10*1000);
				continue;
			} else {
				LOGE("fail to open %s: %d", dev_node, errno);
				perror("");
				return -1;
			}
		} else {
			LOGD("%s is opend(%d).", dev_node, port_open_retry);
			break;
		}
	}

	ccci_create_md_status_listen_thread();

	if(!stream_support) {
		g_FsInfo.pFsBuf = mmap(NULL, sizeof(fs_stream_buffer_t) * 5, PROT_READ | PROT_WRITE, MAP_SHARED, DeviceFd, 0);
	} else {
		// FS_MAX_BUF_SIZE : ccci_h + opid + argc + MAX_ARG * arg.len + MAX_ARG * arg.align + 16KB data + align(128B)
		FS_MAX_BUF_SIZE = (16 + 4 + 4 + 4*12 + 0x4000 + 128);
		FS_MAX_ARG_NUM = 6;
		int alloc_length = (sizeof(STREAM_DATA) + FS_MAX_BUF_SIZE) * FS_BUFFER_SLOT_NUM;
		g_FsInfo.pFsBuf = malloc(alloc_length);
		memset(g_FsInfo.pFsBuf, 0, alloc_length);
	}
	PackInfo = malloc(sizeof(FS_PACKET_INFO) * FS_MAX_ARG_NUM);
	pkt_buff = malloc(MAX_FS_BUF_BYTE);
	if (PackInfo == NULL || pkt_buff == NULL) {
		LOGE("Main: [error]malloc buffer fail:%d \n", errno);
		return -1;
	}

	if(g_FsInfo.pFsBuf == NULL)
	{
		LOGE("Main: [error]mmap buffer fail:%d \n", errno);
		return -1;
	}

	while (1) {
		mdstatus = get_modem_status();
		if (mdstatus != CCCI_MD_STA_INIT)
			break;
		else if (mdstatus == CCCI_MD_STA_UNDEFINED) {
			/*LOGI("get no prop:%d\n", mdstatus);*/
		}
		usleep(100*1000);
	}

	FS_Init(md_id);

	FS_OTP_init(md_id);

	LOGD("register signal hadler\n");
	if(signal(SIGHUP, signal_treatment)==SIG_ERR)
		LOGE("can't catch SIGHUP\n");
	if(signal(SIGPIPE, signal_treatment)==SIG_ERR)
		LOGE("can't catch SIGPIPE\n");
	if(signal(SIGKILL, signal_treatment)==SIG_ERR)
		LOGE("can't catch SIGKILL\n");
	if(signal(SIGINT, signal_treatment)==SIG_ERR)
		LOGE("can't catch SIGINT\n");
	if(signal(SIGUSR1, signal_treatment)==SIG_ERR)
		LOGE("can't catch SIGUSR1\n");
	if(signal(SIGUSR2, signal_treatment)==SIG_ERR)
		LOGE("can't catch SIGUSR2\n");
	if(signal(SIGTERM, signal_treatment)==SIG_ERR)
		LOGE("can't catch SIGTERM\n");
	if(signal(SIGALRM, signal_treatment)==SIG_ERR)
		LOGE("can't catch SIGALRM\n");

	while(exit_signal == 0)
	{
#ifdef PROFILE_NVRAM_API
		mdstatus = get_modem_status();

		if (mdstatus == CCCI_MD_STA_BOOT_UP)
		{
			slower_100ms.profiling = true;
		}
		else if (mdstatus == CCCI_MD_STA_BOOT_READY)
		{
			if (slower_100ms.profiling)
				LOGD("[Profile]slower than 100ms: count=%d, totatime=%d\n", slower_100ms.count, slower_100ms.totaltime);

			memset(&slower_100ms, 0, sizeof(SLOW_THAN_100MS_T));
		}

#endif

		PacketNum = 0;
retry:
		if(!stream_support) {
			ReqBufIndex = ioctl(DeviceFd, CCCI_FS_IOCTL_GET_INDEX, 0);
			FS_WAKE_LOCK();

			if(ReqBufIndex < 0 || ReqBufIndex >= FS_REQ_BUFFER_MUN)
			{
				LOGE("Main: [error]fail get CCCI_FS buffer index: %d \n", errno);
				RetVal = FS_PARAM_ERROR;
				PackInfo[PacketNum].Length = sizeof(unsigned int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				goto _Next;
			}
			pFsBuf = (FS_BUF *)((char *)g_FsInfo.pFsBuf + (FS_MAX_BUF_SIZE + sizeof(FS_BUF))*ReqBufIndex);
		} else {
			while (1) {
                memset(pkt_buff, 0, MAX_FS_BUF_BYTE);
				// add an extra integer as MD consider OP_ID as not part of the "payload"
                RetVal = read(DeviceFd, pkt_buff, (MAX_FS_PKT_BYTE+sizeof(CCCI_BUFF_T)+sizeof(unsigned int)));
                if (RetVal <= 0) {
                    LOGE("Failed to read from FS device (%d) !! errno = %d", RetVal, errno);
                        goto retry;
                } else {
                    LOGD("Read %d bytes from FS device", RetVal);
                }
				FS_WAKE_LOCK();
                stream = (STREAM_DATA *)pkt_buff;
                ccci_h = (CCCI_BUFF_T *)&stream->header;
                ReqBufIndex = ccci_h->reserved;
                LOGD("Read %d bytes from slot %d, CCCI_H(0x%X)(0x%X)(0x%X)(0x%X)",
                     RetVal, ReqBufIndex,
                     ccci_h->data[0], ccci_h->data[1], ccci_h->channel, ccci_h->reserved);
                if (((ccci_h->channel&0xFFFF)  != 0x0e) || (ReqBufIndex < 0) || (ReqBufIndex >= FS_REQ_BUFFER_MUN) ||
                    (ccci_h->data[1] > (sizeof(STREAM_DATA) + FS_MAX_BUF_SIZE) )) {
                    LOGE("Main: [error]packet data check fail: ch =0x%x, fs_buf_idx=%d, data[1] = 0x%x, 0x%x \n",
                        ccci_h->channel, ReqBufIndex, ccci_h->data[1], (sizeof(STREAM_DATA) + FS_MAX_BUF_SIZE));
                    RetVal = FS_PARAM_ERROR;
                    PackInfo[PacketNum].Length = sizeof(unsigned int);
                    PackInfo[PacketNum++].pData = (void*) &RetVal;
                    goto _Next;
                }
				buffer_slot = (STREAM_DATA *)((char *)g_FsInfo.pFsBuf + (FS_MAX_BUF_SIZE + sizeof(STREAM_DATA))*ReqBufIndex);
                p_fs_buff = (char *)buffer_slot;
                /******************************************
                 *
                 *  FSM description for re-sent mechanism
                 *   (ccci_fs_buff_state == CCCI_FS_BUFF_IDLE) ==> initial status & end status
                 *   (ccci_fs_buff_state == CCCI_FS_BUFF_WAIT) ==> need to receive again
                 *
                 ******************************************/
                if (!CCCI_FS_PEER_REQ_SEND_AGAIN(ccci_h)) {
                    if (g_FsInfo.fs_buff_state[ReqBufIndex] == FS_BUFF_IDLE) {
                        /* copy data memory and CCCI header */
                        memcpy(p_fs_buff, ccci_h, ccci_h->data[1]);
                        /* don't need to update FS_Address */
                    } else if (g_FsInfo.fs_buff_state[ReqBufIndex] == FS_BUFF_WAIT) {
                        /* copy data memory and NULL, excluding CCCI header, OP id */
						if (ccci_h->data[1] > (sizeof(CCCI_BUFF_T) + sizeof(unsigned int)))
						{
							memcpy(p_fs_buff + g_FsInfo.fs_buff_offset[ReqBufIndex],
									stream->payload.Buffer,
									ccci_h->data[1] - sizeof(CCCI_BUFF_T) - sizeof(unsigned int));
						} else {
							LOGE("Wrong packet data length: %d\n", ccci_h->data[1]);
						}
                        /* update CCCI header info */
                        memcpy(p_fs_buff, ccci_h, sizeof(CCCI_BUFF_T));
                    } else {
                        /* No such fs_buff_state state */
                        assert(0);
                    }
                    g_FsInfo.fs_buff_state[ReqBufIndex] = FS_BUFF_IDLE;
                    g_FsInfo.fs_buff_offset[ReqBufIndex] = 0;
                } else {
                    if (g_FsInfo.fs_buff_state[ReqBufIndex] == FS_BUFF_IDLE) {
                        /* only "OP id" and "data" size and "CCCI header" */
                        unsigned int length = ccci_h->data[1];
                        memcpy(p_fs_buff, ccci_h, length);
                        g_FsInfo.fs_buff_offset[ReqBufIndex] += length;
                    } else if (g_FsInfo.fs_buff_state[ReqBufIndex] == FS_BUFF_WAIT) {
                        /* only "data" size, excluding CCCI header and OP id */
                        unsigned int length = ccci_h->data[1] - sizeof(CCCI_BUFF_T) - sizeof(unsigned int);
                        memcpy(p_fs_buff + g_FsInfo.fs_buff_offset[ReqBufIndex],
                               stream->payload.Buffer,
                               length);    /* CCCI_HEADER + FS_OP_ID */
                        g_FsInfo.fs_buff_offset[ReqBufIndex] += length;
                    } else {
                        /* No such ccci_fs_buff_state state */
                        assert(0);
                    }
                    g_FsInfo.fs_buff_state[ReqBufIndex] = FS_BUFF_WAIT;
                }
                if (g_FsInfo.fs_buff_state[ReqBufIndex] == FS_BUFF_IDLE)
                    break;
		FS_WAKE_UNLOCK();
            };
			pFsBuf = &buffer_slot->payload;
		}
		//LOGD("Main: operation ID = %x\n", pFsBuf->OperateID);

		if(!FS_GetPackInfo(PackInfo, pFsBuf->Buffer))
		{
			LOGE("Main: [error]fail get packet info: op_id=0x%x, fs_buf_idx=%d \n",
				pFsBuf->OperateID, ReqBufIndex);
			RetVal = FS_PARAM_ERROR;
			PackInfo[PacketNum].Length = sizeof(unsigned int);
			PackInfo[PacketNum++].pData = (void*) &RetVal;
			goto _Next;
		}

		switch(pFsBuf->OperateID)
		{
			/* cmpt_read */
			case FS_CCCI_OP_CMPT_READ:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;

				// For debug only ===
				//char FsFileName[PATH_MAX] = {0};
				//int FsFileNameCount;

				// For debug only ===
				memcpy(&in_cmpt_para, PackInfo[1].pData, sizeof(in_cmpt_para));
				FS_ConvWcsToCs(FileName, ConvFileName);
				//FS_MD2APPath(FileName, ConvFileName, FsFileName, PATH_MAX, &FsFileNameCount);
				LOGD("CMPT File name:%s opid_map:0x%08x Flag:0x%08x Offset:%d Whence:%d Length:%d", 
					ConvFileName, in_cmpt_para.opid_map, in_cmpt_para.flag, in_cmpt_para.offset, 
					in_cmpt_para.whence, in_cmpt_para.length);

				// PacketNum                          1 DWORD
				// PackInfo[0].Length = 2*sizeof(int) 1 DWORD
				// PackInfo[0].pData: ret[0], ret[1]  2 DWORD
				// PackInfo[1].Length = 1*sizeof(int) 1 DWORD
				// PackInfo[1].pData                  1 DWORD
				// PackInfo[2].Length = 1*sizeof(int) 1 DWORD
				// PackInfo[2].pData                  1 DWORD
				// PackInfo[3].Lenght = 1*sizeof(int) 1 DWORD
				// PackInfo[3].pData    <-- here    
				in_cmpt_para.data_ptr = pFsBuf->Buffer + (1+1+2+1+1+1+1+1) * sizeof(int);

				RetVal = FS_CCCI_CMPT_Read(FileName, &in_cmpt_para);

				PackInfo[PacketNum].Length = sizeof(in_cmpt_para.ret);
				PackInfo[PacketNum++].pData = (void*)in_cmpt_para.ret;
				PackInfo[PacketNum].Length = sizeof(unsigned int);
				PackInfo[PacketNum++].pData = (void*)&in_cmpt_para.file_size;
				PackInfo[PacketNum].Length = sizeof(unsigned int);
				PackInfo[PacketNum++].pData = (void*)&in_cmpt_para.act_read;
				PackInfo[PacketNum].Length = in_cmpt_para.act_read;
				PackInfo[PacketNum++].pData = in_cmpt_para.data_ptr; 
				//LOGD("cmpt_read ret (%d)\n", RetVal);
				break;
			}
			/* cmpt_write */
			case FS_CCCI_OP_CMPT_WRITE:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;

				// For debug only ===
				//char FsFileName[PATH_MAX] = {0};
				//int FsFileNameCount;

				// For debug only ===
				memcpy(&in_cmptw_para, PackInfo[1].pData, sizeof(in_cmptw_para));
				void* pBuffer = PackInfo[2].pData;
				FS_ConvWcsToCs(FileName, ConvFileName);
				//FS_MD2APPath(FileName, ConvFileName, FsFileName, PATH_MAX, &FsFileNameCount);
				LOGD("CMPTW File name:%s opid_map:0x%08x Flag:0x%08x Offset:%d Whence:%d Length:%d file_handle:%d",
					ConvFileName, in_cmptw_para.opid_map, in_cmptw_para.flag, in_cmptw_para.offset,
					in_cmptw_para.whence, in_cmptw_para.length, in_cmptw_para.file_handle);

				RetVal = FS_CCCI_CMPT_Write(FileName, &in_cmptw_para, pBuffer);

				PackInfo[PacketNum].Length = sizeof(in_cmptw_para.ret);
				PackInfo[PacketNum++].pData = (void*)in_cmptw_para.ret;
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&in_cmptw_para.file_handle_ptr;
				PackInfo[PacketNum].Length = sizeof(unsigned int);
				PackInfo[PacketNum++].pData = (void*)&in_cmptw_para.act_write_size;
				LOGD("cmpt_write ret (%d)\n", RetVal);
				break;
			}
			case FS_CCCI_OP_RESTORE:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;
				dbg_printf("Main: FS_CCCI_RESTORE \n");
				RetVal = FS_CCCI_Restore(FileName);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&RetVal;
				break;
			}
			case FS_CCCI_OP_OPEN:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;
				int	Flag = *((int*)PackInfo[1].pData);
				dbg_printf("Main: FS_CCCI_OPEN \n");
				RetVal = FS_CCCI_Open(FileName, Flag);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&RetVal;
				break;
			}
			case FS_CCCI_OP_GETFILESIZE:
			{
				int HandleIndex = *((unsigned int*)PackInfo[0].pData);
				dbg_printf("Main: FS_CCCI_GetFileSize \n");
				RetVal = FS_CCCI_GetFileSize(HandleIndex, &FileSize);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				PackInfo[PacketNum].Length = sizeof(unsigned int);
				PackInfo[PacketNum++].pData = (void*) &FileSize;
				break;
			}
			case FS_CCCI_OP_SEEK:
			{
				int HandleIndex = *((unsigned int*)PackInfo[0].pData);
				unsigned int Offset = *((unsigned int*)PackInfo[1].pData);
				unsigned int Whence = *((unsigned int*)PackInfo[2].pData);
				dbg_printf("Main: FS_CCCI_SEEK \n");
				RetVal = FS_CCCI_Seek(HandleIndex, Offset, Whence);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_READ:
			{
				int HandleIndex = *((unsigned int*)PackInfo[0].pData);
				int NumOfByte = *((int*)PackInfo[1].pData);
				void* pBuffer = pFsBuf->Buffer + 6*sizeof(int);
				/*memory corrupt issue: there are only two parameters for read from modem CCCI, so PackInfo[2] is invalid*/
				//void* pBuffer = PackInfo[2].pData;
				dbg_printf("Main: FS_CCCI_READ \n");
				RetVal = FS_CCCI_Read(HandleIndex, pBuffer, NumOfByte, &ReadByte);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				PackInfo[PacketNum].Length = sizeof(unsigned int);
				PackInfo[PacketNum++].pData = (void*) &ReadByte;
				PackInfo[PacketNum].Length = ReadByte;
				PackInfo[PacketNum++].pData = pBuffer;
				break;
			}
			case FS_CCCI_OP_WRITE:
			{
				int HandleIndex = *((unsigned int*)PackInfo[0].pData);
				void* pBuffer = PackInfo[1].pData;
				int NumOfByte = *((int*)PackInfo[2].pData);
				dbg_printf("Main: FS_CCCI_WRITE \n");
				RetVal = FS_CCCI_Write(HandleIndex, pBuffer, NumOfByte, &WriteByte);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				PackInfo[PacketNum].Length = sizeof(unsigned int);
				PackInfo[PacketNum++].pData = (void*) &WriteByte;
				break;
			}
			case FS_CCCI_OP_CLOSE:
			{
				int HandleIndex = *((unsigned int*)PackInfo[0].pData);
				dbg_printf("Main: FS_CCCI_CLOSE \n");
				RetVal = FS_CCCI_Close(HandleIndex);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_CLOSEALL:
			{
				dbg_printf("Main: FS_CCCI_CloseAll \n");
				RetVal = FS_CCCI_CloseAll();
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_CREATEDIR:
			{
				wchar_t* DirName = (wchar_t*)PackInfo[0].pData;
				dbg_printf("Main: FS_CCCI_CreateDir \n");
				RetVal = FS_CCCI_CreateDir(DirName);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_REMOVEDIR:
			{
				wchar_t* DirName = (wchar_t*)PackInfo[0].pData;
				dbg_printf("Main: FS_CCCI_RemoveDir \n");
				RetVal = FS_CCCI_RemoveDir(DirName);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_GETFOLDERSIZE:
			{
				wchar_t* DirName = (wchar_t*)PackInfo[0].pData;
				unsigned int Flag = *((unsigned int*)PackInfo[1].pData);
				dbg_printf("Main: FS_CCCI_GetFolderSize \n");
				RetVal = FS_CCCI_GetFolderSize(DirName, Flag);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_RENAME:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;
				wchar_t* NewFileName = (wchar_t*)PackInfo[1].pData;
				dbg_printf("Main: FS_CCCI_Rename \n");
				RetVal = FS_CCCI_Rename(FileName, NewFileName);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_MOVE:
			{
				wchar_t* SrcFullPath = (wchar_t*)PackInfo[0].pData;
				wchar_t* DstFullPath = (wchar_t*)PackInfo[1].pData;
				unsigned int Flag = *((unsigned int*)PackInfo[2].pData);
				dbg_printf("Main: FS_CCCI_Move \n");
				RetVal = FS_CCCI_Move(SrcFullPath, DstFullPath, Flag);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_COUNT:
			{
				wchar_t* FullPath = (wchar_t*)PackInfo[0].pData;
				unsigned int Flag = *((unsigned int*)PackInfo[1].pData);
				dbg_printf("Main: FS_CCCI_Count \n");
				RetVal = FS_CCCI_Count(FullPath, Flag);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_GETDISKINFO:
			{
				/*char* DriverName = (char*)PackInfo[0].pData;
				unsigned int Flag = *((unsigned int*)PackInfo[1].pData);*/
				dbg_printf("Main: FS_CCCI_GetDiskInfo \n");
				RetVal = FS_CCCI_GetDiskInfo(&DiskInfo);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				PackInfo[PacketNum].Length = sizeof(FS_DiskInfo);
				PackInfo[PacketNum++].pData = (void*) &DiskInfo;
				break;
			}
			case FS_CCCI_OP_DELETE:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;
				dbg_printf("Main: FS_CCCI_Delete \n");
				RetVal = FS_CCCI_Delete(FileName);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_GETATTRIBUTES:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;
				dbg_printf("Main: FS_CCCI_GetAttributes \n");
				RetVal = FS_CCCI_GetAttributes(FileName);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_OPENHINT:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;
				int	Flag = *((int*)PackInfo[1].pData);
				dbg_printf("Main: FS_CCCI_OpenHint \n");
				//LOGD("OpenHint \n");
				RetVal = FS_CCCI_Open(FileName, Flag);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&RetVal;
				PackInfo[PacketNum].Length = 8;
				PackInfo[PacketNum++].pData = PackInfo[2].pData;
				break;
			}
			case FS_CCCI_OP_FINDFIRST:
			{
				wchar_t* PatternName = (wchar_t*)PackInfo[0].pData;
				char Attr = *((char*)PackInfo[1].pData);
				char AttrMask = *((char*)PackInfo[2].pData);
				unsigned int MaxLength = *((unsigned int*)PackInfo[3].pData);
				dbg_printf("Main: FS_CCCI_FindFirst \n");
				RetVal = FS_CCCI_FindFirst(PatternName, Attr, AttrMask, &DosDirEntry, FileNameTemp, &MaxLength);	
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&RetVal;
				PackInfo[PacketNum].Length = sizeof(FS_DOSDirEntry);
				PackInfo[PacketNum++].pData = &DosDirEntry;
				PackInfo[PacketNum].Length = (MaxLength > 0)?((MaxLength+1)*2):0; //wide character
				PackInfo[PacketNum++].pData = (void*)FileNameTemp;
				break;
			}
			case FS_CCCI_OP_FINDNEXT:
			{
				int HandleIndex = *((int*)PackInfo[0].pData);
				unsigned int MaxLength = *((unsigned int*)PackInfo[1].pData);
				dbg_printf("Main: FS_CCCI_FindNext \n");
				RetVal = FS_CCCI_FindNext(HandleIndex, &DosDirEntry, FileNameTemp, &MaxLength);	
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&RetVal;
				PackInfo[PacketNum].Length = sizeof(FS_DOSDirEntry);
				PackInfo[PacketNum++].pData = &DosDirEntry;
				PackInfo[PacketNum].Length = (MaxLength > 0) ? ((MaxLength+1) * 2) : 0; //wide character
				PackInfo[PacketNum++].pData = (void*)FileNameTemp;
				break;
			}
			case FS_CCCI_OP_FINDCLOSE:
			{
				int HandleIndex = *((int*)PackInfo[0].pData);
				dbg_printf("Main: FS_CCCI_FindClose (%d) \n", HandleIndex);
				RetVal = FS_CCCI_FindClose(HandleIndex);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&RetVal;
				break;
			}
			case FS_CCCI_OP_LOCKFAT:
			{
				dbg_printf("Main: FS_CCCI_LockFAT \n");
				RetVal = FS_NO_ERROR;
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				LOGD("LF: %d \n", RetVal);
				break;
			}
			case FS_CCCI_OP_UNLOCKALL:
			{
				dbg_printf("Main: FS_CCCI_UnLockAll \n");
				RetVal = 1;
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				LOGD("UK: %d \n", RetVal);
				break;
			}
			case FS_CCCI_OP_SHUTDOWN:
			{
				dbg_printf("Main: FS_CCCI_Shutdown \n");
				LOGD("Shutdown \n");
				FS_CCCI_ShutDown();
				break;
			}
			case FS_CCCI_OP_XDELETE:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;
				unsigned int Flag = *((unsigned int*)PackInfo[1].pData);
				dbg_printf("Main: FS_CCCI_XDelete \n");
				RetVal = FS_CCCI_XDelete(FileName, Flag);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_CLEARDISKFLAG:
			{
				dbg_printf("Main: FS_CCCI_ClearDiskFlag \n");
				RetVal = FS_NO_ERROR;
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				LOGD("CDF: %d \n", RetVal);
				break;
			}
			case FS_CCCI_OP_GETDRIVE:
			{
				unsigned int Type = *((unsigned int*)PackInfo[0].pData);
				unsigned int Serial = *((unsigned int*)PackInfo[1].pData);
				unsigned int AltMask = *((unsigned int*)PackInfo[2].pData);
				dbg_printf("Main: FS_CCCI_GetDrive \n");
				RetVal = FS_CCCI_GetDrive(Type, Serial, AltMask);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&RetVal;
				break;
			}
			case FS_CCCI_OP_GETCLUSTERSIZE:
			{
				unsigned int DriverIdx = *((unsigned int*)PackInfo[0].pData);
				dbg_printf("Main: FS_CCCI_GetClusterSize \n");
				RetVal = FS_CCCI_GetClusterSize(DriverIdx);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*)&RetVal;
				break;
			}
			case FS_CCCI_OP_SETDISKFLAG:
			{
				dbg_printf("Main: FS_CCCI_SetDiskFlag \n");
				RetVal = FS_NO_ERROR;
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				LOGD("SDF: %d \n", RetVal);
				break;
			}

			case FS_CCCI_OP_OTP_WRITE:
			{
				int devtype;
				unsigned int Offset;
				void * BufferPtr;
				unsigned int  Length;
				devtype = *(int*)PackInfo[0].pData;
				Offset = *(unsigned int*)PackInfo[1].pData;
				BufferPtr = PackInfo[2].pData;
				Length = *(unsigned int*)PackInfo[3].pData;
				dbg_printf("Main: FS_CCCI_OP_OTP_WRITE \n");
				RetVal = FS_OTPWrite(devtype, Offset, BufferPtr, Length);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_OTP_READ:
			{
				int devtype;
				unsigned int Offset;
				void * BufferPtr = pFsBuf->Buffer + 4*sizeof(int);
				unsigned int  Length;

				devtype = *(int*)PackInfo[0].pData;
				Offset = *(unsigned int*)PackInfo[1].pData;
				Length = *(unsigned int*)PackInfo[2].pData;

				dbg_printf("Main: FS_CCCI_OP_OTP_READ \n");
				RetVal = FS_OTPRead(devtype, Offset, BufferPtr, Length);

				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				PackInfo[PacketNum].Length = Length;
				PackInfo[PacketNum++].pData = BufferPtr;
				break;
			}
			case FS_CCCI_OP_OTP_QUERYLEN:
			{
				int devtype;

				devtype = *(int*)PackInfo[0].pData;
				RetVal = FS_OTPQueryLength(devtype, &Length);
				dbg_printf("Main: FS_CCCI_OP_OTP_QUERYLEN \n");
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				PackInfo[PacketNum].Length = sizeof(unsigned int);
				PackInfo[PacketNum++].pData = (void*)&Length;
				break;
			}
			case FS_CCCI_OP_OTP_LOCK:
			{
				int devtype;

				devtype = *(int*)PackInfo[0].pData;
				dbg_printf("Main: FS_CCCI_OP_OTP_LOCK \n");
				RetVal = FS_OTPLock(devtype);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_BIN_REGION_ACCESS:
			{
				int access_type;

				access_type = *(int*)PackInfo[0].pData;
				LOGD("Main: FS_CCCI_OP_BIN_REGION_ACCESS \n");
				RetVal = FS_BinRegion_Access(access_type);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
			}
			case FS_CCCI_OP_GETFILEDETAIL:
			{
				wchar_t* FileName = (wchar_t*)PackInfo[0].pData;
				dbg_printf("Main: FS_CCCI_GetFileDetail \n");
				RetVal = FS_CCCI_GetFileDetail(FileName, &FileDetail);
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				PackInfo[PacketNum].Length = sizeof(FS_FileDetail);
				PackInfo[PacketNum++].pData = (void*)&FileDetail;
				break;
			}
			default:
				LOGE("Main: [error]Unknow File Op ID (%d)\n", pFsBuf->OperateID);
				RetVal = FS_PARAM_ERROR;
				PackInfo[PacketNum].Length = sizeof(int);
				PackInfo[PacketNum++].pData = (void*) &RetVal;
				break;
		}
_Next:
		if ((ReqBufIndex < 0) || (ReqBufIndex >= FS_REQ_BUFFER_MUN) || (pFsBuf == NULL)) {
			LOGE("Main: [error] Request check fail(%d), force md assert\n", ReqBufIndex);
			if(ioctl(DeviceFd, CCCI_IOC_FORCE_MD_ASSERT, &RetVal) != 0)
				LOGD("update modem type to kernel fail: err=%d", errno);
		} else if(!FS_WriteToMD(DeviceFd, ReqBufIndex, PackInfo, PacketNum)) {
			LOGE("Main: [error]fail write fs stream: op_id=%x\n", pFsBuf->OperateID);
			//return -1;
		}
		FS_WAKE_UNLOCK();
	}
	LOGD("ccci_fsd exit, free buffer\n");
	close(DeviceFd);
	free(PackInfo);
	if(stream_support)
		free(g_FsInfo.pFsBuf);
	return 0;
}
