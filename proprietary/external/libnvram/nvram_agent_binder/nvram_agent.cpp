/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */



#include "nvram_agent.h"
#include "libnvram.h"
#include "libnvram_log.h"

#include <cutils/properties.h>

void NvRAMAgent::instantiate() {

int ret = (int)defaultServiceManager()->addService(
        descriptor, new NvRAMAgent());

   if(ret != 0){
       ALOGE("Haman, serviceManager not ready");
       exit(1);
   }
   ALOGE("Haman, serviceManager work well");

}

NvRAMAgent::NvRAMAgent()
{
    NVRAM_LOG("NvRAMAgent created");
}

status_t BnNvRAMAgent::onTransact(uint32_t code,
			       const Parcel &data,
			       Parcel *reply,
			       uint32_t flags)
{
	int size = 0;
	int flag = 0;
	char *filename;
	size_t stringlen;
	const char16_t *s16;

    switch(code)
	{
    	case TRANSACTION_readFile:
		{

			ALOGD("readFile\n");
			NVRAM_LOG("NvRAMAgent onTransact TRANSACTION_readFile ");


			data.enforceInterface (descriptor);
			int size=0;
			int file_lid=data.readInt32 ();
			char * ret=readFile (file_lid,size);
			if (ret==NULL)
			{
				NVRAM_LOG("Read Error\n");
				reply->writeInt32 (-1);
			}
			else
			{
				reply->writeInt32(0);
				reply->writeInt32(size);
				reply->write (ret,size);
				NVRAM_LOG("readFile done!\n");
				free (ret);
			}
			return NO_ERROR;
    	} break;
  		case TRANSACTION_readFileByName:
		{
			data.enforceInterface (descriptor);
			s16 = data.readString16Inplace(&stringlen);
			filename = strndup16to8(s16, stringlen);


            if (filename==NULL)
            {
            	NVRAM_LOG("Read Error filename==NULL\n");
			    reply->writeInt32 (-1);
            }
            else
            {
            	char * ret=readFileByName(filename,size);
				NVRAM_LOG("TRANSACTION_readFile %s ", filename);
            	if (ret==NULL)
            	{
            		NVRAM_LOG("Read Error\n");
			    	reply->writeInt32 (-1);
            	}
            	else
            	{
					reply->writeInt32(0);
					reply->writeInt32(size);
			    	reply->write (ret,size);
					NVRAM_LOG("readFile done!\n");
			    	free (ret);
				}
				free(filename);
			}
			return NO_ERROR;
		} break;
		case TRANSACTION_writeFile:
		{
			NVRAM_LOG("writeFile\n");
			data.enforceInterface (descriptor);
			int file_lid=data.readInt32();
			int size=data.readInt32 ();
			int flag=0;
			if (size==-1)
			{ // array is null
			    reply->writeInt32 (-1);
			}
			else
			{
	   		 	char *buff = (char *)malloc(size);
				if(buff == NULL)
	             {
		             NVRAM_LOG("Malloc Error!\n");
		             reply->writeInt32 (-1);
					 return NO_ERROR;
	             }
			    data.read (buff,size);
				flag=writeFile (file_lid,size,buff);
				if(flag > 0)
			    {
				    NVRAM_LOG("writeFile done!");
			    	reply->writeInt32 (0);
					reply->writeInt32(flag);
				}
				else
					reply->writeInt32(-1);
			    free(buff);
			}
		return NO_ERROR;
    	} break;

		case TRANSACTION_writeFileEx:
		{
			NVRAM_LOG("writeFileEx\n");
			data.enforceInterface (descriptor);
			int file_lid=data.readInt32();
			int rec_no = data.readInt32 ();
			int size=data.readInt32 ();

			int flag=0;
			if (size==-1)
			{ // array is null
			    reply->writeInt32 (-1);
			}
			else
			{
				char *buff = (char *)malloc(size);
				if(buff == NULL)
	             {
		             NVRAM_LOG("Malloc Error!\n");
		             reply->writeInt32 (-1);
					 return NO_ERROR;
	             }
	   		 	//char buff[size];
			    data.read (buff,size);
				flag=writeFileEx (file_lid,rec_no,size,buff);
				if(flag > 0)
			    {
				    NVRAM_LOG("writeFileEx done!");
			    	reply->writeInt32 (0);
					reply->writeInt32(flag);
				}
				else
					reply->writeInt32(-1);
				free(buff);
			}
		return NO_ERROR;
    	} break;

   	 	case TRANSACTION_writeFileByName:
		{
			data.enforceInterface (descriptor);
			s16 = data.readString16Inplace(&stringlen);
			filename = strndup16to8(s16, stringlen);
            if (filename==NULL)
            {
            	NVRAM_LOG("writeFileByName filename==NULL\n");
			    reply->writeInt32 (-1);
            }
			else
			{

            	NVRAM_LOG("TRANSACTION_writeFile %s ", filename);

            	size=data.readInt32 ();
            	flag=0;

            	if (size==-1)
            	{ // array is null
            		reply->writeInt32 (-1);
            	}
            	else
            	{
            		char *buff = (char *)malloc(size);
            		if(buff == NULL)
            		{
		             	NVRAM_LOG("Malloc Error!\n");
		             	reply->writeInt32 (-1);
						free(filename);
						return NO_ERROR;
            		}
            		data.read (buff,size);
					flag=writeFileByName(filename,size,buff);
					if(flag)
			    	{
				    	NVRAM_LOG("writeFile done!");
			    		reply->writeInt32 (0);
						reply->writeInt32(flag);
					}
					else
					reply->writeInt32(-1);
					free(buff);
				}
				free(filename);
			}
			return NO_ERROR;
    	}
        case TRANSACTION_getFileDesSize:
        {
            ALOGD("getFileDesSize\n");
			data.enforceInterface (descriptor);
			int file_lid=data.readInt32 ();
            int i4RecSize=0;
            int i4RecNum=0;
			int i4Ret=getFileDesSize(file_lid, i4RecSize, i4RecNum);
			if (i4Ret==0)
			{
				NVRAM_LOG("getFileDesSize Error\n");
				reply->writeInt32 (-1);
			}
			else
			{
				reply->writeInt32(0);
				reply->writeInt32(i4RecSize);
				reply->writeInt32(i4RecNum);
				NVRAM_LOG("getFileDesSize done!\n");
			}
			return NO_ERROR;
    	}

		break;
    	default:
		return BBinder::onTransact(code, data, reply, flags);
  }

    return NO_ERROR;
}


char* NvRAMAgent::readFile(int file_lid,int & size)
{
	int pRecSize=0,pRecNum=0;
	bool IsRead=1;
	char *buff=NULL;
	int nvram_ready_retry = 0;
	char nvram_init_val[PROPERTY_VALUE_MAX];


	/* Sync with Nvram daemon ready */
	do {
		property_get("vendor.service.nvram_init", nvram_init_val, NULL);
		if(0 == strcmp(nvram_init_val, "Ready"))
			break;
		else {
			nvram_ready_retry ++;
			usleep(500000);
		}
	} while(nvram_ready_retry < 10);
	NVRAM_LOG("NvRAMAgent readFile ");


    F_ID fd=NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    if (fd.iFileDesc==-1)
	{
		NVRAM_LOG("open file Error!\n");
		return NULL;
    }
	NVRAM_LOG("RecNum is :%d\n",pRecNum);
    size=pRecSize*pRecNum;
	buff=(char *)malloc(size);
	if(buff == NULL)
	{
		NVRAM_LOG("Malloc Error!\n");
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return NULL;
	}
	if(size == read(fd.iFileDesc,buff,size))
	{
		if(NVM_CloseFileDesc(fd))
		{
			NVRAM_LOG("Read Done!Size is %d\n",size);
			return buff;
   		 }
		else
		{
				NVRAM_LOG("Close file error!\n");
				free(buff);
				return NULL;
		}
	}
	else
	{
			NVRAM_LOG("read File error!\n");
			if(!NVM_CloseFileDesc(fd))
				NVRAM_LOG("close File error!\n");
			free(buff);
			return NULL;
	}
}



int NvRAMAgent::writeFile(int file_lid,int size,char* buff)
{
    if (buff==NULL || size==0) {
	return 0;
    }
	int pRecSize=0,pRecNum=0,looptimes=0, reCount=0;
	bool IsRead=0;
	int nvram_ready_retry = 0;
		char nvram_init_val[PROPERTY_VALUE_MAX];


	/* Sync with Nvram daemon ready */
	do {
		property_get("vendor.service.nvram_init", nvram_init_val, NULL);
		if(0 == strcmp(nvram_init_val, "Ready"))
			break;
		else {
			nvram_ready_retry ++;
			usleep(500000);
		}
	} while(nvram_ready_retry < 10);


	F_ID fd=NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    if (fd.iFileDesc==-1)
	{
		NVRAM_LOG("open file Error!\n");
		return 0;
    }
	if(size != pRecSize)
	{
		NVRAM_LOG("Input size (%d) and RecSize (%d) not match!\n",size,pRecSize);
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return 0;
	}
	// GetFileDesc should return right pos and this would cause pro_info multi lids issue.
	#if 0
	if(0 != lseek(fd.iFileDesc,0,SEEK_SET))
	{
		NVRAM_LOG("lseek error!\n");
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return 0;
	}
	#endif
	looptimes = pRecNum;
	NVRAM_LOG("RecNum is :%d\n",pRecNum);
	while(looptimes--)
	{
		reCount = write(fd.iFileDesc,buff,size);
    	if(size != reCount)
		{
			NVRAM_LOG("write file error!\n");
			if(!NVM_CloseFileDesc(fd))
				NVRAM_LOG("close File error!\n");
			return reCount;
    		}
		}
	if(NVM_CloseFileDesc(fd))
	{
		NVRAM_LOG("Write file Done!\n");
		return reCount;
	}
	else
	{
		NVRAM_LOG("close File error!\n");
		return 0;
	}
}


int NvRAMAgent::writeFileEx(int file_lid,int rec_no, int size,char* buff)
{
    if (buff==NULL || size==0) {
	return 0;
    }
	int pRecSize=0,pRecNum=0,looptimes=0, reCount=0;
	bool IsRead=0;
	int nvram_ready_retry = 0;
		char nvram_init_val[PROPERTY_VALUE_MAX];


	/* Sync with Nvram daemon ready */
	do {
		property_get("vendor.service.nvram_init", nvram_init_val, NULL);
		if(0 == strcmp(nvram_init_val, "Ready"))
			break;
		else {
			nvram_ready_retry ++;
			usleep(500000);
		}
	} while(nvram_ready_retry < 10);


	F_ID fd=NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    if (fd.iFileDesc==-1)
	{
		NVRAM_LOG("open file Error!\n");
		return 0;
    }
	if(size != pRecSize)
	{
		NVRAM_LOG("Input size (%d) and RecSize (%d) not match!\n",size,pRecSize);
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return 0;
	}
	// GetFileDesc should return right pos and this would cause pro_info multi lids issue.
	#if 0
	if(0 != lseek(fd.iFileDesc,0,SEEK_SET))
	{
		NVRAM_LOG("lseek error!\n");
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return 0;
	}
	#endif
	//looptimes = pRecNum;
	NVRAM_LOG("RecNum is :%d\n",pRecNum);

	NVRAM_LOG("input val rec_no =%d,size=%d, pRecSize=%d\n",rec_no,size,pRecSize);
	if(-1 == lseek(fd.iFileDesc,rec_no*pRecSize,SEEK_SET))
	{
		NVRAM_LOG("lseek error! fail: %s\n", (char*)strerror(errno));

		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return 0;
	}
	reCount = write(fd.iFileDesc,buff,size);
	if(size != reCount)
	{
		NVRAM_LOG("write file error!\n");
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return reCount;
	}
	if(NVM_CloseFileDesc(fd))
	{
		NVRAM_LOG("Write file Done!\n");
		return reCount;
	}
	else
	{
		NVRAM_LOG("close File error!\n");
		return 0;
	}
}

char* NvRAMAgent::readFileByName(char* filename,int & size)
{
	int pRecSize=0,pRecNum=0;
	bool IsRead=1;
	char *buff=NULL;
	int file_lid = -1;

	file_lid = NVM_GetLIDByName(filename);
	if(file_lid < 0)
	{
		NVRAM_LOG("Get LID by name fail! %s\n",filename);
		return NULL;
    }

	F_ID fd=NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    if (fd.iFileDesc==-1)
	{
		NVRAM_LOG("open file Error!\n");
		return NULL;
    }
	NVRAM_LOG("RecNum is :%d\n",pRecNum);
    size=pRecSize*pRecNum;
	buff=(char *)malloc(size);
	if(buff == NULL)
	{
		NVRAM_LOG("Malloc Error!\n");
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return NULL;
	}
	if(size == read(fd.iFileDesc,buff,size))
	{
		if(NVM_CloseFileDesc(fd))
		{
			NVRAM_LOG("Read Done!Size is %d\n",size);
			return buff;
   		}
		else

		{
			NVRAM_LOG("Close file error!\n");
			free(buff);
			return NULL;
		}
	}
	else
	{
		NVRAM_LOG("read File error!\n");
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		free(buff);
		return NULL;
	}
}



int NvRAMAgent::writeFileByName(char * filename,int size,char* buff)
{
    if (buff==NULL || size==0) {
	return 0;
    }
	int pRecSize=0,pRecNum=0,looptimes=0;
	bool IsRead=0;
	int file_lid = -1;

    file_lid = NVM_GetLIDByName(filename);
	if(file_lid < 0)
	{
		NVRAM_LOG("Get LID by name fail!\n");
		return NULL;
    }


	F_ID fd=NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    if (fd.iFileDesc==-1)
	{
		NVRAM_LOG("open file Error!\n");
		return 0;
    }
	if(size != pRecSize)
	{
		NVRAM_LOG("Input size (%d) and RecSize (%d) not match!\n",size,pRecSize);
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return 0;
	}

	// GetFileDesc should return right pos and this would cause pro_info multi lids issue.
	#if 0
	if(0 != lseek(fd.iFileDesc,0,SEEK_SET)){
		NVRAM_LOG("lseek error!\n");
		if(!NVM_CloseFileDesc(fd))
			NVRAM_LOG("close File error!\n");
		return 0;
		}
 	#endif
	looptimes = pRecNum;
	NVRAM_LOG("RecNum is :%d\n",pRecNum);
	while(looptimes--)
	{
    	if(size != write(fd.iFileDesc,buff,size))
		{
			NVRAM_LOG("write file error!\n");
			if(!NVM_CloseFileDesc(fd))
				NVRAM_LOG("close File error!\n");
			return 0;
    	}
	}
	if(NVM_CloseFileDesc(fd))
	{
		NVRAM_LOG("Write file Done!\n");
		return 1;
    }
	else
	{
		NVRAM_LOG("close File error!\n");
		return 0;
	}
}

int NvRAMAgent::getFileDesSize(int file_lid, int & recSize, int & recNum)
{
    int pRecSize=0,pRecNum=0;
	bool IsRead=1;
    F_ID fd=NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    if (fd.iFileDesc==-1)
	{
		NVRAM_LOG("open file Error!\n");
		return 0;
    }
    recSize = pRecSize;
    recNum = pRecNum;
	NVRAM_LOG("RecNum is :%d\n",pRecNum);
	if(!NVM_CloseFileDesc(fd))
	{
		NVRAM_LOG("close File error!\n");
		return 0;
	}
    return 1;
}
int
main(int argc, char *argv[])
{
//    daemon (0,0);
     NvRAMAgent::instantiate();
    ProcessState::self()->startThreadPool();
    NVRAM_LOG("NvRAMAgent Service is now ready");
    IPCThreadState::self()->joinThreadPool();
    return(0);
}

