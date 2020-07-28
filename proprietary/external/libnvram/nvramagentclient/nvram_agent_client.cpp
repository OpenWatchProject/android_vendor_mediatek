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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

#define LOG_NDEBUG 0
#define LOG_TAG "nvram_agent_client(Native)"
#include <log/log.h>

#include <utils/String8.h>
#include <utils/Vector.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <sys/system_properties.h>
#include "nvram_agent_client.h"
#include "libnvram.h"
#include "libnvram_log.h"

#define MT_NORMAL_BOOT 0
#define MT_META_BOOT 1
#define MT_RECOVERY_BOOT 2
#define MT_SW_REBOOT 3
#define MT_FACTORY_BOOT 4
#define MT_ADVMETA_BOOT 5
#define MT_ATE_FACTORY_BOOT 6
#define MT_ALARM_BOOT 7
#define MT_UNKNOWN_BOOT 8
#if defined (MTK_KERNEL_POWER_OFF_CHARGING_SUPPORT)
#define MT_KERNEL_POWER_OFF_CHARGING_BOOT 8
#define MT_LOW_POWER_OFF_CHARGING_BOOT 9
#undef MT_UNKNOWN_BOOT
#define MT_UNKNOWN_BOOT 10
#endif


Mutex NvRAMAgentClient::sMutex;
sp<INvRAMAgent> NvRAMAgentClient::sNvRAMAgentService;
sp<NvRAMAgentClient::DeathNotifier> NvRAMAgentClient::sDeathNotifier;

NvRAMAgentClient:: NvRAMAgentClient()
{
    ALOGD("NvRAMAgentClient::NvRAMAgentClient()");
}

NvRAMAgentClient::~NvRAMAgentClient()
{
    ALOGD("NvRAMAgentClient::~NvRAMAgentClient()");
}

NvRAMAgentClient* NvRAMAgentClient::create()
{
    ALOGD("NvRAMAgentClient::create()");
    return new NvRAMAgentClient();
}

const sp<INvRAMAgent>& NvRAMAgentClient::getNvRAMAgentService()
{
    Mutex::Autolock lock(sMutex);
    int tryCnt = 0;
    if (NULL == sNvRAMAgentService.get())
    {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        binder = sm->getService(String16("NvRAMAgent"));
        if (binder != 0)
        {
            ALOGD("getNvRAMAgentService success");
            if (NULL == sDeathNotifier.get())
            {
                sDeathNotifier = new DeathNotifier();
            }
            binder->linkToDeath(sDeathNotifier);
            sNvRAMAgentService = interface_cast<INvRAMAgent>(binder);
        }else
        {
            ALOGW("NvRAMAgentService not published.");
        }
    }
    return sNvRAMAgentService;
}


static int get_boot_mode(void)
{
  int fd;
  size_t s;
  int boot_mode_val = 0;
  char boot_mode[4] = {'0'};

  fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
  if (fd < 0)
  {
    ALOGV("fail to open: %s,%s\n", "/sys/class/BOOT/BOOT/boot/boot_mode",(char*)strerror(errno));
    return -1;
  }

  s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
  close(fd);

  if(s <= 0)
  {
    ALOGV("could not read boot mode sys file:%s\n",(char*)strerror(errno));
    return -1;
  }

  boot_mode[s] = '\0';

  sscanf(boot_mode,"%d",&boot_mode_val);
  ALOGV("get_boot_mode: the mode=%d\n",boot_mode_val);
  return boot_mode_val;
  
}
/**
 * The client should free the returned buf.
 * This method may return NULL, if read failed
 */
char* NvRAMAgentClient::readFile(int file_lid, int & size)
{
    ALOGV("NvRAMAgentClient::readFile():%d, size = %d", file_lid, size);
    
    int nvram_ready_retry = 0;
	char nvram_init_val[PROPERTY_VALUE_MAX];
	F_ID fd;
	bool IsRead = 1;
	int pRecSize =0,pRecNum = 0;
	char *buff = NULL;
	int mt_boot_mode = 0;
   


	#if 0 
	/* Sync with Nvram daemon ready */
	do {
		property_get("nvram_init", nvram_init_val, NULL);
		if(0 == strcmp(nvram_init_val, "Ready"))
			break;
		else {
			nvram_ready_retry ++;
			usleep(500000);
		}
	} while(nvram_ready_retry < 10);
	#endif
	mt_boot_mode = get_boot_mode();
        if(mt_boot_mode < 0)
          {
             ALOGE("get_boot_mode failed:%s",(char*)strerror(errno));
             return NULL;
          }
	
    ////
    if(mt_boot_mode == MT_NORMAL_BOOT || mt_boot_mode == MT_ALARM_BOOT)
	{
    if (getNvRAMAgentService().get() == NULL)
    {
        size = 0;
        ALOGE("getNvRAMAgentService failed");
        return NULL;
    }
    ALOGV("getNvRAMAgentService success");
	    buff = getNvRAMAgentService()->readFile(file_lid, size);
	    ALOGD("readFile:buf = %p, size = %d", buff,size);
	    #if 0
	    int i = 0;
	    for(;i < size && buff; ++i)
	    {
	        ALOGD("%x",buff[i]);
	    }
	    #endif
		return buff;
	}
	else
	{

		///
    	fd = NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    	if (fd.iFileDesc == -1) 
		{
			ALOGV("open file Error!\n");
			return NULL;
    	        } 

		ALOGV("RecNum is :%d\n",pRecNum);
	    size = pRecSize * pRecNum;
		buff = (char *)malloc(size);
		if(buff == NULL)
		{
			ALOGV("Malloc Error!\n");
			if(!NVM_CloseFileDesc(fd))
				NVRAM_LOG("close File error!\n");
			return NULL;
		}
		if(size == read(fd.iFileDesc,buff,size))
		{
			if(NVM_CloseFileDesc(fd))
			{
				ALOGV("Read Done!Size is %d\n",size);
 					return buff;
	   		} 
			else
			{
				ALOGV("Close file error!\n");
				free(buff);
				return NULL;
			}
		}
		else
		{
			ALOGV("read File error!\n");
			if(!NVM_CloseFileDesc(fd))
				ALOGV("close File error!\n");
			free(buff);
			return NULL;
		}
	}
    
    ALOGD("%%%%%%%%%%%%%%%%%%%%%%%%%%");
	return NULL;
}

/**
 * Return 1 if write success, otherwise return 0
 */
int NvRAMAgentClient::writeFile(int file_lid, int size, char *buff)
{
    ALOGV("NvRAMAgentClient::writeFile():%d, size = %d", file_lid, size);
    if (getNvRAMAgentService().get() == NULL)
    {
        ALOGE("getNvRAMAgentService failed");
        return 0;
    }
    ALOGV("getNvRAMAgentService success");
    return getNvRAMAgentService()->writeFile(file_lid, size, buff);
}

/**
 * Return 1 if write success, otherwise return 0
 */
int NvRAMAgentClient::writeFileEx(int file_lid, int rec_no, int size, char *buff)
{
    ALOGV("NvRAMAgentClient::writeFileEx():%d, size = %d", file_lid, size);
    if (getNvRAMAgentService().get() == NULL)
    {
        ALOGE("getNvRAMAgentService failed");
        return 0;
    }
    ALOGV("getNvRAMAgentService success");
    return getNvRAMAgentService()->writeFileEx(file_lid,rec_no, size, buff);
}


/**
 * The client should free the returned buf.
 * This method may return NULL, if read failed
 */
char* NvRAMAgentClient::readFileByName(char* file_name, int & size)
{
    ALOGV("NvRAMAgentClient::readFileByName():%s, size = %d", file_name, size);
    if (getNvRAMAgentService().get() == NULL)
    {
        size = 0;
        ALOGE("getNvRAMAgentService failed");
        return NULL;
    }
    ALOGV("getNvRAMAgentService success");
    return getNvRAMAgentService()->readFileByName(file_name, size);
}

/**
 * Return 1 if write success, otherwise return 0
 */
int NvRAMAgentClient::writeFileByName(char* file_name, int size, char *buff)
{
    ALOGV("NvRAMAgentClient::writeFileByName():%s, size = %d", file_name, size);
    if (getNvRAMAgentService().get() == NULL)
    {
        ALOGE("getNvRAMAgentService failed");
        return 0;
    }
    ALOGV("getNvRAMAgentService success");
    return getNvRAMAgentService()->writeFileByName(file_name, size, buff);
}

/**
 * Return 1 if write success, otherwise return 0
 * Now it's only ready on AOSP branch
 */

 int NvRAMAgentClient::getFileDesSize(int file_lid, int & recSize, int & recNum)
{
    ALOGV("NvRAMAgentClient::getFileDesSize():%d", file_lid);
    if (getNvRAMAgentService().get() == NULL)
    {
        ALOGE("getNvRAMAgentService failed");
        return 0;
    }
    ALOGV("getNvRAMAgentService success");
    return getNvRAMAgentService()->getFileDesSize(file_lid, recSize, recNum);
}

NvRAMAgentClient::DeathNotifier::DeathNotifier()
{
    ALOGD("DeathNotifier()");
}

NvRAMAgentClient::DeathNotifier::~DeathNotifier()
{
    ALOGD("~DeathNotifier()");
    Mutex::Autolock lock(sMutex);
    if (NULL != sNvRAMAgentService.get())
    {
        sNvRAMAgentService->asBinder(sNvRAMAgentService)->unlinkToDeath(this);
    }
}

void NvRAMAgentClient::DeathNotifier::binderDied(const wp<IBinder>& who __unused)
{
    Mutex::Autolock lock(sMutex);
    NvRAMAgentClient::sNvRAMAgentService.clear();
    ALOGW("NvRAMAgent server died!");
}

