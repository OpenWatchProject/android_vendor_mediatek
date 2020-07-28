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
#define LOG_TAG "MMProfile"

#define MTK_LOG_ENABLE 1
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <cutils/log.h>
#include <linux/mmprofile_internal.h>
#include "MMProfileFile.h"
#include <stdlib.h>

#define  LogPrint(...) SLOGD(__VA_ARGS__)

static MMProfile_Global_t* pMMProfileGlobals = NULL;
static MMProfile_Event_t*  pMMProfileBuffer  = NULL;
static int bMMProfileInitBuffer = 0;
static int MMProfile_FD = -1;
static unsigned int MMProfileBufferSizeRecord = 0;
pthread_mutex_t MMProfile_MemoryMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MMProfile_FDMutex = PTHREAD_MUTEX_INITIALIZER;

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

static const char KUsage[] =
{
"\n"
"adb shell mmp command [param]\n"
"command:\n"
"    enable          : enable  MMProfile\n"
"    start           : start   MMProfile\n"
"    disable         : disable MMProfile\n"
"    stop            : stop    MMProfile\n"
"    enable_event    : enable  event\n"
"    disable_event   : diable  event\n"
"    state           : current state\n"
"    dump            : dump    data\n"
"Example:\n"
"    adb shell mmp enable\n"
"                    enable MMProfile\n"
"    adb shell mmp state\n"
"                    check current state\n"
"    adb shell mmp enable_event name=XXX r\n"
"                    enable events of XXX and its children\n"
"    adb shell mmp start\n"
"                    start MMProfile\n"
"    adb shell mmp dump /sdcard/XXX.mmp -FTrace\n"
"                    dump mmp and ftrace to file in sdcard\n"
"\n"
};

static unsigned int MMProfileInitFD(void)
{
    unsigned int ret = 1;
    pthread_mutex_lock(&MMProfile_FDMutex);
    if (unlikely(MMProfile_FD < 0))
    {
        MMProfile_FD = open("/sys/kernel/debug/mmprofile/mmp", O_RDWR);
        if (MMProfile_FD == -1)
	        MMProfile_FD = open("/dev/mmp", O_RDWR);

        if (MMProfile_FD == -1)
            ret = 0;
    }
    pthread_mutex_unlock(&MMProfile_FDMutex);
    return ret;
}

static void MMProfileHookGlobals(void)
{
    if (MMProfileInitFD() == 0)
        return;
    pthread_mutex_lock(&MMProfile_MemoryMutex);
    if (!pMMProfileGlobals)
    {
        char* pBuf;
        if (ioctl(MMProfile_FD, MMP_IOC_SELECTBUFFER, MMProfileGlobalsBuffer))
        {
            pthread_mutex_unlock(&MMProfile_MemoryMutex);
            return;
        }
        pBuf = (char*)mmap(0, MMProfileGlobalsSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, MMProfile_FD,0);
        LogPrint("Globals = 0x%p\n", pBuf);
        if (pBuf == MAP_FAILED)
        {
            pthread_mutex_unlock(&MMProfile_MemoryMutex);
            return;
        }
        pMMProfileGlobals = (MMProfile_Global_t*) pBuf;
    }
    pthread_mutex_unlock(&MMProfile_MemoryMutex);
}

unsigned int MMProfileDump(char* filename, unsigned int bFTrace)
{
    unsigned int ret = 1;
    unsigned int i = 0;
    MMPFile_Header_t Header;
    unsigned int TagIndex = 0;
    unsigned int FilePointer = sizeof(MMPFile_Header_t);
    MMPFile_TAG_t* pTags = NULL;
    MMPFile_TAG_EventInfo_t EventInfoTag;
    MMPFile_EventInfo_t RegEvent;
    MMPFile_TAG_Record_t RecordTag;
    MMProfile_EventInfo_t* pEventInfo = NULL;
    unsigned long long start_time;
    unsigned long long end_time;
    unsigned char* pMetaDataBuf = NULL;
    int seek_ret = 0;

    if (unlikely(!pMMProfileGlobals))
        MMProfileHookGlobals();
    if ((pMMProfileGlobals == 0) || (pMMProfileGlobals->enable == 0))
        return 0;
    if (MMProfileInitFD() == 0)
        return 0;
    ioctl(MMProfile_FD, MMP_IOC_START, 0);

    if (unlikely(!bMMProfileInitBuffer))
    {
        // Map buffer.
        pthread_mutex_lock(&MMProfile_MemoryMutex);
        if (!pMMProfileBuffer)
        {
            char* pBuf;
            if (ioctl(MMProfile_FD, MMP_IOC_SELECTBUFFER, MMProfilePrimaryBuffer))
            {
                pthread_mutex_unlock(&MMProfile_MemoryMutex);
                return 0;
            }
            pBuf = (char*)mmap(0, pMMProfileGlobals->buffer_size_bytes, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, MMProfile_FD,0);
            LogPrint("Buffer = 0x%08x\n", (unsigned int)pBuf);
            if ((unsigned int)pBuf == 0xFFFFFFFF)
            {
                pthread_mutex_unlock(&MMProfile_MemoryMutex);
                return 0;
            }
            pMMProfileBuffer = (MMProfile_Event_t*) pBuf;
            bMMProfileInitBuffer = 1;
        }
        pthread_mutex_unlock(&MMProfile_MemoryMutex);
    }

    memset(&Header, 0, sizeof(MMPFile_Header_t));
    memset(&EventInfoTag, 0, sizeof(MMPFile_TAG_EventInfo_t));
    memset(&RegEvent, 0, sizeof(MMPFile_EventInfo_t));
    memset(&RecordTag, 0, sizeof(MMPFile_TAG_Record_t));
    FILE* fp = fopen(filename, "wb");
    if (!fp)
    {
        LogPrint("Can not open file to write.");
        return 0;
    }

    // Save file header.
    LogPrint("Save file header.");
    Header.FileID = MMPFILE_ID;
    Header.Version = MMPFILE_FILE_VERSION;
    Header.TagCount = 4;
    Header.Reserved = 0;
    fwrite(&Header, sizeof(MMPFile_Header_t), 1, fp);

    // Prepare Tags.
    LogPrint("Prepare Tags.");
    FilePointer += Header.TagCount * sizeof(MMPFile_TAG_t);
    FilePointer = (FilePointer + 15)/16*16;
    pTags = malloc(sizeof(MMPFile_TAG_t)*Header.TagCount);
    if (NULL == pTags)
    {
        ret = 0;
        goto exit;
    }
    memset(pTags, 0, sizeof(MMPFile_TAG_t) * Header.TagCount);

    // Event Info
    LogPrint("Save event info.");
    pTags[TagIndex].Type = MMPFILE_TAG_EVENTINFO;
    pTags[TagIndex].Offset = FilePointer;
    seek_ret = fseek(fp, pTags[TagIndex].Offset, SEEK_SET);
    EventInfoTag.NameLen = MMProfileEventNameMaxLen;
    EventInfoTag.Flag = MMPFILE_EVENTFLAG_MULBYTE;
    fwrite(&EventInfoTag, sizeof(MMPFile_TAG_EventInfo_t), 1, fp);
    // Write an invalid event. Cover index 0.
    memset(&RegEvent, 0, sizeof(MMPFile_EventInfo_t));
    fwrite(&RegEvent, sizeof(MMPFile_EventInfo_t), 1, fp);
    // Allocate event info.
    pEventInfo = (MMProfile_EventInfo_t*) malloc(sizeof(MMProfile_EventInfo_t)*MMProfileMaxEventCount);
    if (NULL == pEventInfo)
    {
        ret = 0;
        goto exit;
    }
    ioctl(MMProfile_FD, MMP_IOC_DUMPEVENTINFO, pEventInfo);
    // Search for start time and end time.
    start_time = 0;
    start_time--;
    end_time = 0;
    for (i=0; i<pMMProfileGlobals->buffer_size_record; i++)
    {
        MMPFile_Record_t Record;
        unsigned long long time;
        unsigned int RecordIndex = (pMMProfileGlobals->write_pointer+i)%(pMMProfileGlobals->buffer_size_record);
        if ((pMMProfileBuffer[RecordIndex].lock > 0) ||
            (pMMProfileBuffer[RecordIndex].id == 0))
            continue;
        time = (((unsigned long long)(pMMProfileBuffer[RecordIndex].timeHigh))<<32) + pMMProfileBuffer[RecordIndex].timeLow;
        if (time < start_time)
            start_time = time;
        if (time > end_time)
            end_time = time;
    }
    //printf("\nstart_time: %llx\n", start_time);
    //printf("end_time: %llx\n", end_time);
    for (i=1, EventInfoTag.EventCount=1; i<MMProfileMaxEventCount; i++)
    {
        if (pEventInfo[i].name[0] != 0)
        {
            LogPrint("Event: %s", pEventInfo[i].name);
            strncpy(RegEvent.name, pEventInfo[i].name, sizeof(RegEvent.name) -1);
	    RegEvent.name[sizeof(RegEvent.name) - 1] = '\0';
            RegEvent.parent = pEventInfo[i].parentId;
            RegEvent.match_with_data1 = 0;
            RegEvent.start_time = start_time;
            RegEvent.end_time = end_time;
            RegEvent.flag = 0;
            if (pMMProfileGlobals->event_state[i] == 0)
                RegEvent.flag |= MMProfileEvent_Disabled;
            fwrite(&RegEvent, sizeof(MMPFile_EventInfo_t), 1, fp);
            EventInfoTag.EventCount++;
        }
    }
    free(pEventInfo);
    pEventInfo = NULL;
    LogPrint("Event count is %d.", EventInfoTag.EventCount);
    seek_ret = fseek(fp, pTags[TagIndex].Offset, SEEK_SET);
    fwrite(&EventInfoTag, sizeof(MMPFile_TAG_EventInfo_t), 1, fp);
    pTags[TagIndex].Size = sizeof(MMPFile_TAG_EventInfo_t) + sizeof(MMPFile_EventInfo_t)*EventInfoTag.EventCount;
    seek_ret = fseek(fp, sizeof(MMPFile_Header_t) + TagIndex * sizeof(MMPFile_TAG_t), SEEK_SET);
    fwrite(&pTags[TagIndex], sizeof(MMPFile_TAG_t), 1, fp);
    FilePointer += pTags[TagIndex].Size;
    FilePointer = (FilePointer + 15)/16*16;
    TagIndex++;

    // Records
    LogPrint("Save record.");
    pTags[TagIndex].Type = MMPFILE_TAG_RECORD;
    pTags[TagIndex].Offset = FilePointer;
    seek_ret = fseek(fp, pTags[TagIndex].Offset, SEEK_SET);
    RecordTag.RecordCount = 0;
    RecordTag.FreqLow = 1000000000;
    RecordTag.FreqHigh = 0;
    fwrite(&RecordTag, sizeof(MMPFile_TAG_Record_t), 1, fp);
    LogPrint("write pointer is %d\n", pMMProfileGlobals->write_pointer);
    for (i=0; i<pMMProfileGlobals->buffer_size_record; i++)
    {
        MMPFile_Record_t Record;
        unsigned int RecordIndex = (pMMProfileGlobals->write_pointer+i)%(pMMProfileGlobals->buffer_size_record);
        if ((pMMProfileBuffer[RecordIndex].lock > 0) ||
            (pMMProfileBuffer[RecordIndex].id == 0))
            continue;
        memcpy(&Record, &(pMMProfileBuffer[RecordIndex].id), sizeof(MMPFile_Record_t));
        LogPrint("Dump #%d record of size %d at 0x%08x\n", RecordIndex, sizeof(MMPFile_Record_t), (unsigned int)fp);
        fwrite(&Record, sizeof(MMPFile_Record_t), 1, fp);
        RecordTag.RecordCount++;
    }
    LogPrint("record count is %d\n", RecordTag.RecordCount);
    pTags[TagIndex].Size = sizeof(MMPFile_TAG_Record_t) + sizeof(MMProfile_Event_t)*RecordTag.RecordCount;
    seek_ret = fseek(fp, sizeof(MMPFile_Header_t) + TagIndex * sizeof(MMPFile_TAG_t), SEEK_SET);
    fwrite(&pTags[TagIndex], sizeof(MMPFile_TAG_t), 1, fp);
    seek_ret = fseek(fp, pTags[TagIndex].Offset, SEEK_SET);
    fwrite(&RecordTag, sizeof(MMPFile_TAG_Record_t), 1, fp);
    FilePointer += pTags[TagIndex].Size;
    FilePointer = (FilePointer + 15)/16*16;
    TagIndex++;

    // FTrace
    LogPrint("Save FTrace Data.");
    pTags[TagIndex].Type = MMPFILE_TAG_FTRACERAWDATA;
    pTags[TagIndex].Offset = FilePointer;
    pTags[TagIndex].Size = 0;
    seek_ret = fseek(fp, sizeof(MMPFile_Header_t) + TagIndex * sizeof(MMPFile_TAG_t), SEEK_SET);
    fwrite(&pTags[TagIndex], sizeof(MMPFile_TAG_t), 1, fp);
    {
        if (bFTrace)
        {
            FILE* fpFTrace;
            fpFTrace = fopen("/sys/kernel/debug/tracing/trace", "rb");
            if (fpFTrace)
            {
                unsigned int FTraceSize = 0;
                unsigned int ReadSize;
                char* pFTraceBuf;
                pFTraceBuf = (char*) malloc(0x100000);
                seek_ret = fseek(fp, pTags[TagIndex].Offset, SEEK_SET);
                while(1)
                {
                    ReadSize = fread(pFTraceBuf, 1, 0x100000, fpFTrace);
		    if(!ReadSize){
				fwrite(pFTraceBuf, ReadSize, 1, fp);
		    } else { break; }
                    FTraceSize += ReadSize;
                    //printf("Read FTrace size = 0x%X\n", ReadSize);
                    if (feof(fpFTrace))
                        break;
                }
                free(pFTraceBuf);
                fclose(fpFTrace);
                //printf("FTrace total size = 0x%X\n", FTraceSize);
                pTags[TagIndex].Size = FTraceSize;
                seek_ret = fseek(fp, sizeof(MMPFile_Header_t) + TagIndex * sizeof(MMPFile_TAG_t), SEEK_SET);
                fwrite(&pTags[TagIndex], sizeof(MMPFile_TAG_t), 1, fp);
            }
        }
    }
    FilePointer += pTags[TagIndex].Size;
    FilePointer = (FilePointer + 15)/16*16;
    TagIndex++;

    // MetaData
    LogPrint("Save Meta Data.");
    pMetaDataBuf = malloc(pMMProfileGlobals->meta_buffer_size);
    if (NULL == pMetaDataBuf)
    {
        printf("Cannot allocate buffer for meta data.\n");
        LogPrint("Cannot allocate buffer for meta data.\n");
        goto exit;
    }
    else
    {
        MMPFile_TAG_MetaData_t MetaDataTag = {0,0,{0,0}};
        ioctl(MMProfile_FD, MMP_IOC_DUMPMETADATA, pMetaDataBuf);
        MetaDataTag.DataCount = *(unsigned int*)pMetaDataBuf;
        MetaDataTag.DataSize = *(unsigned int*)(pMetaDataBuf+4);
        pTags[TagIndex].Type = MMPFILE_TAG_METADATA;
        pTags[TagIndex].Offset = FilePointer;
        pTags[TagIndex].Size = MetaDataTag.DataSize+sizeof(MMPFile_TAG_MetaData_t);
        seek_ret = fseek(fp, sizeof(MMPFile_Header_t) + TagIndex * sizeof(MMPFile_TAG_t), SEEK_SET);
        fwrite(&pTags[TagIndex], sizeof(MMPFile_TAG_t), 1, fp);
        seek_ret = fseek(fp, pTags[TagIndex].Offset, SEEK_SET);
        fwrite(&MetaDataTag, sizeof(MMPFile_TAG_MetaData_t), 1, fp);
        fwrite(pMetaDataBuf+8, MetaDataTag.DataSize, 1, fp);
        free(pMetaDataBuf);
    }

exit:
    fclose(fp);
    if (NULL != pTags)
        free(pTags);
    return ret;
}

void PrepareConfigFile(int enable)
{
    FILE* fp = NULL;
    unsigned int value;
    fp = fopen(CONFIG_MMPROFILE_PATH, "rb");
    if (fp == NULL)
    {
        fp = fopen(CONFIG_MMPROFILE_PATH, "wb");
        if (NULL != fp)
        {
		value = (enable ? 1 : 0);
		fwrite(&value, 4, 1, fp);    // Default enable
		fwrite(&value, 4, 1, fp);    // Default start
		value = 0x10000;
		fwrite(&value, 4, 1, fp);   // Default buffer size in records.
		value = 0x800000;
		fwrite(&value, 4, 1, fp);   // Default meta buffer size in bytes.
		fclose(fp);
		//chmod(CONFIG_MMPROFILE_PATH, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        }
    }
    else
    {
        fclose(fp);
    }
}

int DumpRegTable(char* filename)
{
    MMProfile_EventInfo_t* pEventInfo = NULL;
    if (NULL != filename)
    {
        printf("Dump reg table to %s ... ", filename);
        LogPrint("Dump reg table to %s ... ", filename);
    }
    pEventInfo = (MMProfile_EventInfo_t*) malloc(sizeof(MMProfile_EventInfo_t)*MMProfileMaxEventCount);
    if (NULL == pEventInfo)
    {
        printf("\nError. Cannot allocate memory for reg table\n");
        LogPrint("\nError. Cannot allocate memory for reg table\n");
        free(pEventInfo);
        return -1;
    }
    if (ioctl(MMProfile_FD, MMP_IOC_DUMPEVENTINFO, pEventInfo) == 0)
    {
        unsigned int i = 0;
        if (NULL != filename)
        {
            FILE* fpEventInfo = fopen(filename, "wb");
            if (NULL == fpEventInfo)
            {
                printf("\nError. Cannot create file for reg table\n");
                LogPrint("\nError. Cannot create file for reg table\n");
                free(pEventInfo);
                return -2;
            }
            fwrite(pEventInfo, sizeof(MMProfile_EventInfo_t)*MMProfileMaxEventCount, 1, fpEventInfo);
            fclose(fpEventInfo);
        }
        printf("%-8s%-32s%-32s%-16s\n","ID", "Event", "Parent", "Enable");
        for (i=0; i<MMProfileMaxEventCount; i++)
        {
            if (pEventInfo[i].name[0])
            {
                unsigned int data;
                printf("%-8d", i);
                printf("%-32s", pEventInfo[i].name);
                if (pEventInfo[pEventInfo[i].parentId].name[0])
                    printf("%-32s", pEventInfo[pEventInfo[i].parentId].name);
                else
                    printf("%-32s", " ");
                data = i;
                ioctl(MMProfile_FD, MMP_IOC_ISENABLE, &data);
                printf("%-16d", data);
                printf("\n");
            }
        }
        printf("Done\n");
        LogPrint("Done\n");
    }
    else
    {
        printf("\nError! IOCTL failed.\n");
        LogPrint("\nError! IOCTL failed.\n");
        free(pEventInfo);
        return -3;
    }
    free(pEventInfo);
    return 0;
}

int main(int argc, char **argv)
{
	int i = 0;
	char *fbp = 0;
	FILE *fp;
    MMP_Event events[4];
    if (argc > 1 && strcmp(argv[1], "version") == 0)
    {
        unsigned int version = 1;
        fp = fopen("/data/mmpversion.bin", "wb");
        if (fp)
        {
            fwrite(&version, 4, 1, fp);
            fclose(fp);
        }
        printf("mmp version is %d!\n", version);
        LogPrint("mmp version is %d!\n", version);
        return 0;
    }
    for (i=0; i<argc; i++)
        LogPrint("Command Line %d: %s\n", i, argv[i]);
    if (argc < 2)
    {
        printf("%s",KUsage);
        LogPrint("%s",KUsage);
        goto exit;
    }

    if (MMProfileInitFD() == 0)
    {
        printf("Error: Cannot connect to mmp driver!\n");
        LogPrint("Error: Cannot connect to mmp driver!\n");
        return 0;
    }


    if (unlikely(!pMMProfileGlobals))
        MMProfileHookGlobals();
    if (!pMMProfileGlobals)
    {
        printf("Error: Cannot hook mmprofile global variables!\n");
        LogPrint("Error: Cannot hook mmprofile global variables!\n");
        return 0;
    }

    if (((strcmp(argv[1], "enable") != 0) &&
         (strcmp(argv[1], "disable") != 0) &&
         (strcmp(argv[1], "default_enable") != 0) &&
         (strcmp(argv[1], "default_start") != 0) &&
         (strcmp(argv[1], "default_record_count") != 0) &&
         (strcmp(argv[1], "default_metabuffer_size") != 0) &&
         (strcmp(argv[1], "read_config") != 0) &&
         (strcmp(argv[1], "state") != 0)) &&
        ((pMMProfileGlobals == 0) ||
         (pMMProfileGlobals->enable == 0)))
    {
        printf("MMProfile is not enabled!\n");
        LogPrint("MMProfile is not enabled!\n");
        return 0;
    }

    if (strcmp(argv[1], "enable") == 0)
    {
        ioctl(MMProfile_FD, MMP_IOC_ENABLE, 1);
        printf("MMProfile enabled!\n");
        LogPrint("MMProfile enabled!\n");
    }
    else if (strcmp(argv[1], "disable") == 0)
    {
        ioctl(MMProfile_FD, MMP_IOC_ENABLE, 0);
        printf("MMProfile disabled!\n");
        LogPrint("MMProfile disabled!\n");
    }
    else if (strcmp(argv[1], "read_config") == 0)
    {
        int enable = 0;
        int start = 0;
        size_t read_cnt = 0;
        FILE* fp = NULL;
        fp = fopen(CONFIG_MMPROFILE_PATH, "rb");
        if (fp)
        {
            read_cnt = fread(&enable, 4, 1, fp);
            read_cnt = fread(&start, 4, 1, fp);
            read_cnt = fread(&(pMMProfileGlobals->new_buffer_size_record), 4, 1, fp);
            read_cnt = fread(&(pMMProfileGlobals->new_meta_buffer_size), 4, 1, fp);
            fclose(fp);
            ioctl(MMProfile_FD, MMP_IOC_ENABLE, enable);
            ioctl(MMProfile_FD, MMP_IOC_START, start);
            printf("MMProfile read_config: enable=%d start=%d!\n", enable, start);
            LogPrint("MMProfile read_config: enable=%d start=%d!\n", enable, start);
        }
        else
        {
#ifdef DEFAULT_ENABLE_MMPROFILE
            ioctl(MMProfile_FD, MMP_IOC_ENABLE, 1);
            ioctl(MMProfile_FD, MMP_IOC_START, 1);
            printf("MMProfile read_config: default enable & start\n");
            LogPrint("MMProfile read_config: default enable & start");
#endif
        }
    }
    else if ((strcmp(argv[1], "default_enable") == 0) ||
             (strcmp(argv[1], "default_start") == 0) ||
             (strcmp(argv[1], "default_record_count") == 0) ||
             (strcmp(argv[1], "default_metabuffer_size") == 0))
    {
        if (argc < 3)
        {
            printf("Error: mmp expects argument for %s!\n", argv[1]);
            LogPrint("Error: mmp expects argument for %s!\n", argv[1]);
            goto exit;
        }
        {
            FILE* fp = NULL;
            unsigned int value;
            int seek_ret = 0;
            PrepareConfigFile(0);
            sscanf(argv[2], "%d", &value);

            fp = fopen(CONFIG_MMPROFILE_PATH, "rb+");
            if (fp)
            {
                if (strcmp(argv[1], "default_enable") == 0)
                    seek_ret = fseek(fp, 0, SEEK_SET);
                else if (strcmp(argv[1], "default_start") == 0)
                    seek_ret = fseek(fp, 4, SEEK_SET);
                else if (strcmp(argv[1], "default_record_count") == 0)
                    seek_ret = fseek(fp, 8, SEEK_SET);
                else if (strcmp(argv[1], "default_metabuffer_size") == 0)
                    seek_ret = fseek(fp, 12, SEEK_SET);
                fwrite(&value, 4, 1, fp);
                fclose(fp);
                printf("MMProfile %s is set to %d!\n", argv[1], value);
                LogPrint("MMProfile %s is set to %d!\n", argv[1], value);
            }
            else
            {
                printf("Error: Cannot write configuration file!\n");
                LogPrint("Error: Cannot write configuration file!\n");
                goto exit;
            }
        }
    }
    else if (strcmp(argv[1], "record_count") == 0)
    {
        sscanf(argv[2], "%d", &(pMMProfileGlobals->new_buffer_size_record));
        printf("Record count is set to 0x%X!\n", pMMProfileGlobals->new_buffer_size_record);
        LogPrint("Record count is set to 0x%X!\n", pMMProfileGlobals->new_buffer_size_record);
    }
    else if (strcmp(argv[1], "metabuffer_size") == 0)
    {
        sscanf(argv[2], "%d", &(pMMProfileGlobals->new_meta_buffer_size));
        printf("Record count is set to 0x%X!\n", pMMProfileGlobals->new_meta_buffer_size);
        LogPrint("Record count is set to 0x%X!\n", pMMProfileGlobals->new_meta_buffer_size);
    }
    else if ((strcmp(argv[1], "enable_event") == 0) ||
             (strcmp(argv[1], "disable_event") == 0))
    {
        /*unsigned int data[3];*/
        struct MMProfile_EventSetting_t setting = {0,0,0,0};
        /*unsigned int cmd;*/
        if (argc < 3)
        {
            printf("Error: mmp expects argument for %s!\n", argv[1]);
            LogPrint("Error: mmp expects argument for %s!\n", argv[1]);
            goto exit;
        }
        if (strcmp(argv[1], "enable_event") == 0) {
            /*data[1] = 1;*/
            setting.enable = 1;
        } else {
            /*data[1] = 0;*/
            setting.enable = 0;
	}
        if ((argc >= 4) && (strcmp(argv[3], "r") == 0)) {
            /*data[2] = 1;*/
            setting.recursive = 1;
        } else {
            /*data[2] = 0;*/
            setting.recursive = 0;
	}
        if (strncmp(argv[2], "id=", 3) == 0)
        {
            /*sscanf(argv[2]+3, "%d", data);*/
            sscanf(argv[2]+3, "%d", &setting.event);
            /*ioctl(MMProfile_FD, MMP_IOC_ENABLEEVENT, &data);*/
            ioctl(MMProfile_FD, MMP_IOC_ENABLEEVENT, &setting);
            /*printf("Event id=%d is %s!\n", data[0], (data[1])?"enabled":"disabled");*/
            /*LogPrint("Event id=%d is %s!\n", data[0], (data[1])?"enabled":"disabled");*/
            printf("Event id=%d is %s!\n", setting.event, (setting.enable)?"enabled":"disabled");
            LogPrint("Event id=%d is %s!\n", setting.event, (setting.enable)?"enabled":"disabled");
        }
        else if (strncmp(argv[2], "name=", 5) == 0)
        {
            MMProfile_EventInfo_t event_info = {0,""};
            event_info.parentId = 0;
            strncpy(event_info.name, argv[2]+5, MMProfileEventNameMaxLen);
            ioctl(MMProfile_FD, MMP_IOC_FINDEVENT, &event_info);
            if (0 != event_info.parentId)
            {
                /*data[0] = event_info.parentId;*/
                setting.event = event_info.parentId;
                /*ioctl(MMProfile_FD, MMP_IOC_ENABLEEVENT, &data);*/
                ioctl(MMProfile_FD, MMP_IOC_ENABLEEVENT, &setting);
                /*printf("Event name=%s id=%d is %s!\n", event_info.name, data[0], (data[1])?"enabled":"disabled");*/
                /*LogPrint("Event name=%s id=%d is %s!\n", event_info.name, data[0], (data[1])?"enabled":"disabled");*/
                printf("Event name=%s id=%d is %s!\n",
			event_info.name, setting.event, (setting.enable)?"enabled":"disabled");
                LogPrint("Event name=%s id=%d is %s!\n",
			event_info.name, setting.event, (setting.enable)?"enabled":"disabled");
            }
            else
            {
                printf("Cannot find event with name=%s!\n", event_info.name);
                LogPrint("Cannot find event with name=%s!\n", event_info.name);
            }
        }
        else
        {
            printf("Invalid command!\n");
            LogPrint("Invalid command!\n");
        }
    }
    else if (strcmp(argv[1], "start") == 0)
    {
        ioctl(MMProfile_FD, MMP_IOC_START, 1);
        printf("MMProfile started!\n");
        LogPrint("MMProfile started!\n");
    }
    else if (strcmp(argv[1], "remotestart") == 0)
    {
        ioctl(MMProfile_FD, MMP_IOC_REMOTESTART, 1);
        printf("MMProfile remote started!\n");
        LogPrint("MMProfile remote started!\n");
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        ioctl(MMProfile_FD, MMP_IOC_START, 0);
        printf("MMProfile stopped!\n");
        LogPrint("MMProfile stopped!\n");
    }
    else if (strcmp(argv[1], "dump") == 0)
    {
        unsigned int bFTrace = 0;
        if (argc < 3)
        {
            printf("Error: mmp expects filename for dump!\n");
            LogPrint("Error: mmp expects filename for dump!\n");
            goto exit;
        }
        printf("Dump data to %s ... ", argv[2]);
        LogPrint("Dump data to %s ... ", argv[2]);
        if (argc >= 4 && strcmp(argv[3], "-FTrace") == 0)
            bFTrace = 1;
        if (MMProfileDump(argv[2], bFTrace))
        {
            printf("Done\n");
            LogPrint("Done\n");
        }
        else
        {
            printf("\nError! Cannot dump data to file.\n");
            LogPrint("\nError! Cannot dump data to file.\n");
        }
    }
    else if (strcmp(argv[1], "reg_table") == 0)
    {
        char* filename = NULL;
        if (argc >= 3)
        {
            filename = argv[2];
        }
        DumpRegTable(filename);
    }
    else if (strcmp(argv[1], "state") == 0)
    {
        printf("MMProfile Globals:\n");
        printf("enable = %d\n", pMMProfileGlobals->enable);
        printf("start = %d\n", pMMProfileGlobals->start);
        printf("write_pointer = %d (0x%X)\n", pMMProfileGlobals->write_pointer, pMMProfileGlobals->write_pointer);
        printf("reg_event_index = %d\n", pMMProfileGlobals->reg_event_index);
        printf("buffer_size_record = %d (0x%X)\n", pMMProfileGlobals->buffer_size_record, pMMProfileGlobals->buffer_size_record);
        printf("buffer_size_bytes = %d (0x%X)\n", pMMProfileGlobals->buffer_size_bytes, pMMProfileGlobals->buffer_size_bytes);
        printf("record_size = %d\n", pMMProfileGlobals->record_size);
        printf("meta_buffer_size = %d (0x%X)\n", pMMProfileGlobals->meta_buffer_size, pMMProfileGlobals->meta_buffer_size);
        printf("new_buffer_size_record = %d (0x%X)\n", pMMProfileGlobals->new_buffer_size_record, pMMProfileGlobals->new_buffer_size_record);
        printf("new_meta_buffer_size = %d (0x%X)\n", pMMProfileGlobals->new_meta_buffer_size, pMMProfileGlobals->new_meta_buffer_size);
        printf("selected_buffer = %d\n", pMMProfileGlobals->selected_buffer);
        printf("\n");
        DumpRegTable(NULL);
    }
    else if (strcmp(argv[1], "dump_ring_buffer") == 0)
    {
        FILE* fp;
        if (argc < 3)
        {
            printf("Error: Expect filename!\n");
            LogPrint("Error: Expect filename!\n");
            goto exit;
        }
        if (unlikely(!bMMProfileInitBuffer))
        {
            // Map buffer.
            pthread_mutex_lock(&MMProfile_MemoryMutex);
            if (!pMMProfileBuffer)
            {
                char* pBuf;
                if (ioctl(MMProfile_FD, MMP_IOC_SELECTBUFFER, MMProfilePrimaryBuffer))
                {
                    pthread_mutex_unlock(&MMProfile_MemoryMutex);
                    printf("Select buffer failed.\n");
                    goto exit;
                }
                pBuf = (char*)mmap(0, pMMProfileGlobals->buffer_size_bytes, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, MMProfile_FD,0);
                LogPrint("Buffer = 0x%08x\n", (unsigned int)pBuf);
                if ((unsigned int)pBuf == 0xFFFFFFFF)
                {
                    pthread_mutex_unlock(&MMProfile_MemoryMutex);
                    printf("Map buffer failed.\n");
                    goto exit;
                }
                pMMProfileBuffer = (MMProfile_Event_t*) pBuf;
                bMMProfileInitBuffer = 1;
            }
            pthread_mutex_unlock(&MMProfile_MemoryMutex);
        }
        fp = fopen(argv[2], "wb");
        if (!fp) {printf("Cannot open file.\n");goto exit;}
        fwrite(pMMProfileBuffer, pMMProfileGlobals->buffer_size_bytes, 1, fp);
        fclose(fp);
        printf("Raw ring buffer dumpped. Size=0x%X\n", pMMProfileGlobals->buffer_size_bytes);
    }
    else if (strcmp(argv[1], "test") == 0)
    {
        ioctl(MMProfile_FD, MMP_IOC_TEST, 0);
    }
    else
    {
        printf("%s",KUsage);
        LogPrint("%s",KUsage);
    }

exit:

    if (pMMProfileBuffer)
        munmap(pMMProfileBuffer, pMMProfileGlobals->buffer_size_bytes);
    if (pMMProfileGlobals)
        munmap(pMMProfileGlobals, MMProfileGlobalsSize);

    if (MMProfile_FD >= 0)
        close(MMProfile_FD);

	return 0;
}
