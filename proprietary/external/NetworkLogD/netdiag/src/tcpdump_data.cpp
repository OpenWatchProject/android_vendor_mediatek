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
#include <unistd.h>
//#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include <dlfcn.h>

#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/statfs.h>

#include <cutils/log.h>
#include <cutils/sockets.h>

#include <cutils/properties.h>
#include <sys/system_properties.h>

#include "utils.h"

#include <string>
#include <vector>
#include <functional>
#include <algorithm>



using namespace std;
using ::std::vector;
typedef ::std::vector<string> StringVector;
typedef ::std::vector<string>::iterator StringVectorIter;

int mLogFileMaxNum = 5;
const int MAX_FILE_SIZE = 100;// 60M
StringVector mNetLogFileArray;
bool mEnableNewRotation = true;
bool mEnableTestRotation = false; // need false after test
bool mEnableDebugLog = false;
long mTotalFileIndex = 1;
const char * LoNg_SUFFIX_STR = ".LoNg";


#define MAX 128
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define DIR_MODE (FILE_MODE | S_IXUSR | S_IXGRP | S_IWGRP | S_IWOTH | S_IXOTH)

#define _PATH_PROCNET_ROUTE "/proc/net/route"
#define _PATH_PROCNET_DEV "/proc/net/dev"

#define maxfd 10
#define MAX_NAME_LEN 256
//When log storage drop down below this level, stop network log
#define RESVERED_STORAGE 10
extern char mStoragePath[128];
int global_fd_tcpdump;
extern int remaining_time;

void stop_tcpdump(bool setStopProp);
int my_system(const char* cmd_str);

int apply_rohc_compress = 0;
static int is_support_rohc_compress = 0;
int rohc_total_file = 10;
bool mEnableSaveLogInData = false;


int support_rohc(){

#if defined(__LP64__)
    char compress_so_file[] = "/system/lib64/libcompress.so";
#else
    char compress_so_file[] =  "/system/lib/libcompress.so";
#endif
    void *so_libcompress = NULL;
    int support = 0;
    if (access(compress_so_file, F_OK) == 0) {
        so_libcompress = dlopen(compress_so_file, RTLD_LOCAL | RTLD_LAZY);
        if (so_libcompress) {
            if (dlsym(so_libcompress, "comp_handle_packet") &&
                dlsym(so_libcompress, "comp_flush_buff_data") &&
                dlsym(so_libcompress, "comp_free_sessions")) {
                if ((dlerror()) != NULL)  {
                    LOGE("get function comp_handle_packet point fail %s", dlerror());
                } else {
                    support = 1;
                }
            } else {
                LOGE("dlsym file %s function fail:%s",compress_so_file,strerror(errno));
            }
        } else {
            LOGE("dlopen file fail:%s,%s",compress_so_file,dlerror());
        }
    } else {
        LOGE("access file fail:%s, %s",compress_so_file,strerror(errno));
    }

    if (so_libcompress) {
        dlclose(so_libcompress);
        so_libcompress = NULL;
    }
    is_support_rohc_compress = support;
    return support;
}

static int apply_rohc()
{
    return (apply_rohc_compress && is_support_rohc_compress);
}


int check_dir(char *root, const char *target) {
    DIR *dir;
    struct dirent *ptr;

    dir = opendir(root);
    if (dir == NULL && (!mEnableSaveLogInData)) {
        LOGD("has some error check_dir %s, opendir is null, errno:%d, uid=%d",
                root, errno, getuid());
        return -1;
    }

    while (dir != NULL && (ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, target) == 0) {
            closedir(dir);
            return 1;
        }

    }
    if (dir != NULL) {
        closedir(dir);
    }
    return 0;
}

int myMkdir(const char *szPath, mode_t mode_in) {
    mode_t mode = mode_in;
    if (mEnableSaveLogInData) {
        mode = S_IRWXU | S_IRWXG;
        LOGD("myMkdir Save log in data change dir mode[%d]", mode);
    } else {
        LOGD("myMkdir use default dir mode[%d] szPath = %s", mode, szPath);
    }
    int result = mkdir(szPath, mode);
    if (result != -1) {
        return result;
    }
    LOGI("mkdir: Failed.[%s] errno=%d",szPath, errno);
    if (errno == 17) {
        usleep(10000);
        if (access(szPath, F_OK) == -1) {
            result = mkdir(szPath, mode);
        } else {
            LOGI("mkdir: access file ok");
            return result = 0;
        }
     }

    if (errno == EINTR || errno == 17) {
        int nRetry = 10;
        while (nRetry-- > 0) {
            usleep(1000);
            result = mkdir(szPath, mode);
            if (result != -1) {
                LOGI("myMkdir retry ok");
                return result;
            } else {
                if (errno == 17) {
                    usleep(10000);
                    if (access(szPath, F_OK) != -1) {
                        LOGI("myMkdir retry access ok");
                        return 0;
                    }
                }
            }
        }
    }
    LOGE("myMkdir retry fail:directory [%s], errno=%d", szPath, errno);
    return result;
}

int makeDir(const char *szPath) {
    if (szPath == NULL) {
        return 0;
    }
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
    if (mEnableSaveLogInData) {
        mode = S_IRWXU | S_IRWXG;
        LOGD("Save log in data change dir mode[%d]", mode);
    } else {
        LOGD("makeDir with all mode[%d]", mode);
    }
    size_t len = strlen(szPath) + 1;
    char currentLevelDir[MAX_NAME_LEN];
    char createdPath[MAX_NAME_LEN];
    char *pCreatedPath = NULL;
    char *pCurrentLevelDir  = NULL;
    memset(createdPath, 0, len);
    pCreatedPath = createdPath;
    pCurrentLevelDir = currentLevelDir;
    for (unsigned long i = 0; i < len; i++) {
        *pCreatedPath = *szPath;
        *pCurrentLevelDir = *szPath;
        pCreatedPath++;
        szPath++;
        if ('/' == *pCurrentLevelDir || len - 1 == i) {
            *pCurrentLevelDir = '\0';
            if (strlen(currentLevelDir) != 0) {
                // Create current level path if not exist
                if (access(createdPath, F_OK) == -1) {
                    if (myMkdir(createdPath, mode) != -1) {
                        LOGD("Make Dir: Create directory [%s]", createdPath);
                    } else {
                        LOGE("Make Dir: Failed to create directory [%s], errno=%d", createdPath, errno);
                        if (errno == 17) {
                            usleep(500000);
                            if (access(createdPath, F_OK) == -1) {
                                LOGE("Make Dir: Failed to access directory after sleep 200ms, errno=%d", errno);
                                if (myMkdir(createdPath,mode) != -1) {
                                    LOGD("ReMake Dir: Create directory [%s]", createdPath);
                                } else {
                                    LOGE("ReMake Dir: Fail Create directory [%s]", createdPath);
                                    return 0;
                                }
                            } else {
                                LOGD("Make Dir: Can Access exist directory [%s]", createdPath);
                            }
                        } else {
                            return 0;
                        }
                    }
                }
            }
            pCurrentLevelDir = currentLevelDir;
            continue;
        }
        pCurrentLevelDir++;
    }
    return 1;
}

/*
static void change_name(char *name) {
    char *head = name;

    while (*head != '\0') {
        if (*head == ':') {
            *head = '-';
        }
        ++head;
    }
}*/

static int get_proc_name(pid_t pid, char *proc_name, int proc_name_sz) {
    int fd;
    int ret = 0;
    char filename[MAX_NAME_LEN];

    if (proc_name == NULL) {
        return -1;
    }

    memset(filename, '\0', MAX_NAME_LEN * sizeof(char));
    
    sprintf(filename, "/proc/%d/cmdline", pid);

    memset(proc_name, '\0', proc_name_sz * sizeof(char));

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    ret = read(fd, proc_name, (proc_name_sz - 1) * sizeof(char));

    close(fd);

    return ret;
}

int find_pid(pid_t * tcpdump_pid_str) {
//    int i, j;
    pid_t pid;
//    pid_t tcpdump_pid = 0;
    DIR *dir;
    struct dirent *dirent;

    int found = 0;

    errno = 0;
    dir = opendir("/proc");
    if (!dir)
        return -1;
    char *proc_name = (char *) malloc(MAX_NAME_LEN * 2);
    while ((dirent = readdir(dir))) {

        if (isdigit(dirent->d_name[0])) {
            pid = atoi(dirent->d_name);
        } else {
            continue;
        }

        if (pid < 50) {
            continue;
        }
        memset(proc_name, '\0', MAX_NAME_LEN * 2 * sizeof(char));
        if (get_proc_name(pid, proc_name, MAX_NAME_LEN * 2) > 0) {
            if (proc_name != NULL && strstr(proc_name, "trace") != NULL) {
                tcpdump_pid_str[found] = pid;
                LOGI("find trace process name: %s,pid: %d\n", proc_name, pid);
                found++;
                if (found >= 4) {
                    break;
                }
            }
        }
    }

    closedir(dir);
    //free(token);
    free(proc_name);
    proc_name = NULL;
    //if (!found)
    // return 0;

    return found;
}

int tcpdump_exist() {
    char buffer_find[1024];
    int bufflen;
    FILE * fp = popen("ps aux trace", "r");

    memset(buffer_find, '\0', 1024 * sizeof(char));
    if (fp == NULL)
        return -1;
    bufflen = fread(buffer_find, sizeof(char), sizeof(buffer_find) - 1 * sizeof(char), fp);
    if (bufflen)
        buffer_find[bufflen] = 0;
    pclose(fp);

    if (strlen(buffer_find) > 0) {
        LOGD("buffer_find:%s\n", buffer_find);
        if (strstr(buffer_find, "trace") != NULL) {
            LOGD("has found trace running\n");
            return 1;
        } else {
            LOGD("has not found trace running\n");
            return 0;
        }
    } else
        return -1;
}

#define FILE_CONFIG_LEN 512
/*get sdpath from property or config-file(/data/data/com.android.ActivityNetwork/files/NetworkingSetting)*/
int get_sdpath_from_configfile(char * sd_path) {
    char file_config[MAX_NAME_LEN] = "/system/etc/mtklog-config.prop";
    int fd_config;
    char * logpath_config = NULL;
    int ret_config = 0, i_config = 0, ret_code = 1;

    char sdcardpath[256] = { 0 };
    property_get(_KEY_CONFIG_LOG_PATH, sdcardpath, "");
    if (strcmp(sdcardpath, "") == 0) /*property not set*/
    {
        LOGD("open file_config:%s ...", file_config);
        fd_config = open(file_config, O_RDONLY);
        if (fd_config < 0) {
            LOGD("open file_config:%s failed, errno=%d(%s)", file_config, errno,
                    strerror(errno)); /*action_code_review(2)  free? goto label?*/
            return -1;
        }
        char * content_config = (char *) malloc(FILE_CONFIG_LEN * sizeof(char));

        if (content_config == NULL) {
            LOGD("malloc content_config failed, errno=%d(%s)", errno,
                    strerror(errno));
            close(fd_config);
            return -1;
        }
        memset(content_config, '\0', FILE_CONFIG_LEN * sizeof(char));
        ret_config = read(fd_config, content_config, (FILE_CONFIG_LEN - 1) * sizeof(char));
        LOGD(
                "content from %s:\n ret_config:%d, errno:%d(%s), content_config:%s",
                file_config, ret_config, errno, strerror(errno),
                content_config);
        if (ret_config == -1) {
            ret_code = -1;
            goto ret_bak;
        } else if (ret_config == 0) {
            ret_code = 0;
            goto ret_bak;
        }

//        if((logpath_config =strstr(content_config,"log_path="))!=NULL)
        if ((logpath_config = strstr(content_config, _KEY_CUSTOM_LOG_PATH))
                != NULL) {
            logpath_config += strlen(_KEY_CUSTOM_LOG_PATH);
            while ((*logpath_config != 0xA) && (*logpath_config != '\0')) {
                if ((*logpath_config == '_') || (*logpath_config == '/')
                        || (*logpath_config >= '0' && *logpath_config <= '9')
                        || (*logpath_config >= 'a' && *logpath_config <= 'z')
                        || (*logpath_config >= 'A' && *logpath_config <= 'Z')) {
                    //Valid path string character
                    sd_path[i_config] = *logpath_config;
                    i_config++;
                }
                logpath_config++;
            }
            sd_path[i_config] = 0;
            LOGD(
                    "Got log path property from configure file :%s, set to system property",
                    sd_path);
            property_set(_KEY_CONFIG_LOG_PATH, sd_path);
        } else {
            ret_code = 0;
            goto ret_bak;
        }
        ret_bak: free(content_config);
        content_config = NULL;
        close(fd_config);
        return ret_code;
    } else {
        strncpy(sd_path, sdcardpath, sizeof(sd_path) - 1);
        sd_path[sizeof(sd_path) - 1] = '\0';
        LOGD("property get :%s", sd_path);
        return 1;
    }
}

void terminate_tcpdump_data_sig_handler(int s) {
    LOGE("Receive terminate signal for trace, signal = %d.", s);
    char log_index[256];
    sprintf(log_index, "%ld", mTotalFileIndex);
    int retValue = property_set(_KEY_FLAG_LOG_INDEX, log_index);
    if (retValue < 0) {
        LOGE("At stop time, property_set log index. Return value %d", retValue);
    } else {
        int mRetryCount = 120;
        while (mRetryCount-- > 0) {
            property_get(_KEY_FLAG_LOG_INDEX, log_index, "0");
            int mTemp = atoi(log_index);
            if (mTemp <= 0) {
                LOGE("At stop time, property_set log index. retry time %d", mRetryCount);
                usleep(10000);
            } else {
                LOGD("At stop time, property_get log index ok");
                break;
            }
        }            
    }
    pthread_exit(0);
}

bool cmp(string &str1,string &str2)
{
    long index1 = 0;
    long index2 = 0;
    const char* filePath1 = str1.c_str();
    const char* filePath2 = str2.c_str();
    char strNum [256] = {0};
    char strPath [256] = {0};
    char* str_start = NULL;
    if(filePath2 == NULL) {
        return true;
    }

    if (filePath1 == NULL) {
        return false;
    }

    if (mEnableDebugLog) {
        LOGD("filePath1: %s",filePath1);
        LOGD("filePath2: %s",filePath2);
    }
    strncpy(strPath, filePath1, sizeof(strPath) - 1);
    strPath[sizeof(strPath) - 1] = '\0';
    str_start = strstr(strPath, "_start");
    char index_char = *(str_start + (strlen("_start") + 1) * sizeof(char));
    if (mEnableDebugLog) {
        LOGD("index_char: %c",index_char);
    }
    int i = 0;
    if (index_char >= '1' && index_char <= '9') {
        memset(strNum, '\0', sizeof(char) * 256);
        for(i = 0;index_char != '.'&& index_char >= '0' && index_char <= '9';) {
            strNum[i++] = index_char;
            index_char = *(str_start + (strlen("_start") + 1 + i) * sizeof(char));
            if (mEnableDebugLog) {
                LOGD("index_char++: %c",index_char);
            }
        }
        if (mEnableDebugLog) {
            LOGD("strNum: %s",strNum);
        }
        index1 = atol(strNum);
    } else {
        index1 = 0;
    } 

    memset(strPath, '\0', sizeof(char) * 256);
    strncpy(strPath, filePath2, sizeof(strPath) - 1);
    strPath[sizeof(strPath) - 1] = '\0';
    str_start = strstr(strPath, "_start");
    index_char = *(str_start + (strlen("_start") + 1) * sizeof(char));
    if (mEnableDebugLog) {
        LOGD("index_char: %c",index_char);
    }
    if (index_char >= '1' && index_char <= '9') {
        memset(strNum, '\0', sizeof(char) * 256);
        for(i = 0;index_char != '.'&& index_char >= '0' && index_char <= '9';) {
            strNum[i++] = index_char;
            index_char = *(str_start + (strlen("_start") + 1 + i) * sizeof(char));
        }
        if (mEnableDebugLog) {
            LOGD("strNum: %s",strNum);
        }
        index2 = atol(strNum);
    } else {
        index2 = 0;
    } 

    if (mEnableDebugLog) {
        LOGD("index1: %ld",index1);
        LOGD("index2: %ld",index2);
    }

    return  index1 < index2;
}

void rotate_old_log_file(char* folder_name, char* suffix) {
    if (folder_name == NULL) {
        LOGE("-->rotate_old_log_file(), folder_name is null");
        return;
    }

    if (strlen(folder_name) > 256) {
        LOGE("-->rotate_old_log_file(), folder_name length is above 256");
        return;
    }

    DIR *dir;
    struct dirent *ptr;

    dir = opendir(folder_name);
    if (dir == NULL) {
        LOGD("fail to open given folder:");
        LOGD("%s", folder_name);
        return;
    }

    mNetLogFileArray.clear();

    //collect all former recording log file first
    while ((ptr = readdir(dir)) != NULL) {
        char* str_suffix = NULL;
        if ((str_suffix = strstr(ptr->d_name, suffix)) != NULL) {
            char* str_start = strstr(ptr->d_name, "_start");
            char* str_loNg = strstr(ptr->d_name,LoNg_SUFFIX_STR);
            char index_char = *(str_start + strlen("_start") + 1);
            if (index_char >= '1' && index_char <= '9' && str_loNg == NULL) {
                mNetLogFileArray.push_back(ptr->d_name);
            }
            //else if((str_suffix = strstr(ptr->d_name, ".tmp")) == NULL){
               // mNetLogFileArray.push_back(ptr->d_name);
              //  LOGD("Is log file:%s", ptr->d_name);
            //} 
        } 
    }
    closedir(dir);

    int mAllFile = mNetLogFileArray.size();
    if (mAllFile > 1) {
        sort(mNetLogFileArray.begin(),mNetLogFileArray.end(),cmp);
        mAllFile = mNetLogFileArray.size();
    }

    char oldest_log_name[256] = { 0 };
    StringVectorIter mPathIterator;
 //   int mRecycleIndex = mAllFile;
 //   bool mNeedRecycle = false;
    if (mLogFileMaxNum <= mAllFile) {
        mPathIterator = mNetLogFileArray.begin();
        string s_path = *mPathIterator;
        const char* _Source = s_path.c_str();
        if (_Source != NULL) {

            char folder_path[256] = { 0 };
            strncpy(folder_path, folder_name, sizeof(folder_path) - 1);
            folder_path[sizeof(folder_path) - 1] = '\0';
            if (folder_path[strlen(folder_path) - 1] != '/') {
                strncat(folder_path,"/",strlen("/"));
            }
            memset(oldest_log_name, '\0', sizeof(char) * 256);
            strncpy(oldest_log_name, folder_path, sizeof(oldest_log_name) - 1);
            oldest_log_name[sizeof(oldest_log_name) - 1] = '\0';
            
            strncat(oldest_log_name,_Source,strlen(_Source));
            if (remove(oldest_log_name) == 0) {
                LOGD("mAllFile = %d, mLogFileMaxNum = %d, Remove file:%s",mAllFile,mLogFileMaxNum, _Source);
            } else {
                LOGE("remove fail:%d, %s",errno,strerror(errno));
            }
        } else {
            LOGE("Old file _Source is null");
        }
    } else {
       // LOGD("rotate_old_log_file() mAllFile = %d,mLogFileMaxNum = %d",mAllFile,mLogFileMaxNum);
    }
/*
    for (mPathIterator = mNetLogFileArray.begin();
            mPathIterator != mNetLogFileArray.end(); mPathIterator++) {
        string s = *mPathIterator;
        const char* _Source1 = s.c_str();
        LOGE("Old file [%s]",_Source1);
    }
*/
/*
    char folder_path[256] = { 0 };
    strncpy(folder_path, folder_name);
    if (folder_path[strlen(folder_path) - 1] != '/') {
        strcat(folder_path, "/");
    }
    char index_suffix[256] = { 0 };
    char old_log_name[256] = { 0 };
    for (mPathIterator = mNetLogFileArray.begin();
            mPathIterator != mNetLogFileArray.end(); mPathIterator++) {
        if (mNeedRecycle) {
            mNeedRecycle = false;
            mPathIterator++; // need recycle
        }

        if (mPathIterator == mNetLogFileArray.end()) {
            break;
        }
        
        string s = *mPathIterator;
        const char* _Source = s.c_str();

        if (_Source == NULL) {
            LOGE("Old file _Source is NULL. not a valid log file .");
            continue;
        }
        
        char log_file_name[256] = { 0 };
        strncpy(log_file_name, _Source);

        char* str_index = strstr(log_file_name, "_start");
        if (str_index == NULL) {
            LOGE("Old file [%s] is not a valid log file.",_Source);
            continue;
        }              
        memset(old_log_name, '\0', sizeof(char) * 256);
        strncpy(old_log_name, folder_path);
        strncat(old_log_name, _Source);

        memset(new_log_name, '\0', sizeof(char) * 256);        
        strncpy(new_log_name, folder_path);
        
        memset(index_suffix, '\0', sizeof(char) * 256);
        sprintf(index_suffix,"_%d",mRecycleIndex);
        mRecycleIndex--;

        str_index += strlen("_start");
        *str_index = '\0';
        strcat(log_file_name, index_suffix);
        strcat(log_file_name, suffix); 
        strcat(new_log_name, log_file_name);

        rename(old_log_name, new_log_name);

//        LOGD("Old log file path:%s", old_log_name);
//        LOGD("Rename to new file. path:%s", new_log_name);
    }*/

}

/**
 * We will give the latest log file a name without any suffix, then suffix 1, 2, 3, 4
 * So when need to add a new log file, the former existing ones need to be renamed,
 * with suffix +1.
 *
 *   **_start.cap
 *   **_start_1.cap
 *   **_start_2.cap
 *   **_start_3.cap
 *   **_start_4.cap
 */
void refact_existing_log_file(char* folder_name, char* suffix) {
    if (folder_name == NULL) {
        LOGE("-->refact_existing_log_file(), folder_name is null");
        return;
    }

    if (strlen(folder_name) > 256) {
        LOGE("-->refact_existing_log_file(), folder_name length is above 256");
        return;
    }
    //LOGD("-->refact_existing_log_file()");

    if (strlen(folder_name) + strlen(suffix) < 230) {
        LOGD("folder_name=%s, suffix=%s", folder_name, suffix);
    }
    DIR *dir;
    struct dirent *ptr;

    dir = opendir(folder_name);
    if (dir == NULL) {
        LOGD("fail to open given folder:");
        LOGD("%s", folder_name);
        return;
    }

    char old_log_name_arr[5][256];
    memset(old_log_name_arr[0], '\0', 256 * sizeof(char));
    memset(old_log_name_arr[1], '\0', 256 * sizeof(char));
    memset(old_log_name_arr[2], '\0', 256 * sizeof(char));
    memset(old_log_name_arr[3], '\0', 256 * sizeof(char));
    memset(old_log_name_arr[4], '\0', 256 * sizeof(char));

    //collect all former recording log file first
    int mRecycleIndex = 0;
    while ((ptr = readdir(dir)) != NULL) {
        char* str_suffix = NULL;
        if ((str_suffix = strstr(ptr->d_name, suffix)) != NULL) {
            char* str_start = strstr(ptr->d_name, "_start");
            char index_char = *(str_start + strlen("_start") + 1);
            if (index_char >= '1' && index_char <= '4') {
                strncpy(old_log_name_arr[index_char - '0'], ptr->d_name, sizeof(old_log_name_arr[index_char - '0']) - 1);
                old_log_name_arr[index_char - '0'][sizeof(old_log_name_arr[index_char - '0']) - 1] = '\0';
                if (index_char == '4') {
                    mRecycleIndex = 4;
                    //LOGI("This file may be recycled by rename:%s",ptr->d_name);
                } else {
                     //LOGD("Is log file:%s", ptr->d_name);
                }
            } else if((str_suffix = strstr(ptr->d_name, ".tmp")) == NULL){
                strncpy(old_log_name_arr[0], ptr->d_name, sizeof(old_log_name_arr[0]) - 1);
                old_log_name_arr[0][sizeof(old_log_name_arr[0]) - 1] = '\0';
                //LOGD("Is log file:%s", ptr->d_name);
            } else {
                //LOGD("Not rename/recycle Tmp file:%s",ptr->d_name);
            }
        } else {
//              LOGV("Not log file");
        }
    }
    closedir(dir);

    if (mRecycleIndex < 4) {
         //LOGI("This time rename flow will be NO recycle happen.");
    }

    char folder_path[256] = { 0 };
    strncpy(folder_path, folder_name, sizeof(folder_path) - 1);
    folder_path[sizeof(folder_path) - 1] = '\0';
    if (folder_path[strlen(folder_path) - 1] != '/') {
        strncat(folder_path,"/",strlen("/"));
    }

    //Rename former log files one by one
    int i;
    for (i = 3; i >= 0; i--) {
        //LOGD("Old cap file Array index %d, File name length=%u above 0 will be rename", i, strlen(old_log_name_arr[i]));
        if (strlen(old_log_name_arr[i]) > 0) {
            char old_log_name[256] = { 0 };
            char new_log_name[256] = { 0 };
            char log_file_name[256] = { 0 };

            strncpy(old_log_name, folder_path, sizeof(old_log_name) - 1);
            old_log_name[sizeof(old_log_name) - 1] = '\0';
            strncat(old_log_name,old_log_name_arr[i],255 - strlen(old_log_name));
                            
            strncpy(new_log_name, folder_path, sizeof(new_log_name) - 1);
            new_log_name[sizeof(new_log_name) - 1] = '\0';
      
            strncpy(log_file_name, old_log_name_arr[i], sizeof(log_file_name) - 1);
            log_file_name[sizeof(log_file_name) - 1] = '\0';

            if (i > 0) {
                char* str_index = strstr(log_file_name, "_start_");
                if (str_index == NULL) {
                    LOGE("Old file [%s] is not a valid log file.",
                            log_file_name);
                    continue;
                }
                str_index += strlen("_start_");
                *str_index = *str_index + 1;
                LOGD("new file[%d] name=%s", i, log_file_name);
            } else {
                char* str_index = strstr(log_file_name, "_start");
                if (str_index == NULL) {
                    LOGE("Old file [%s] is not a valid log file .",
                            log_file_name);
                    continue;
                }
                str_index += strlen("_start");
                *str_index = '\0';
                strncat(log_file_name,"_1",strlen("_1"));
                strncat(log_file_name,suffix,strlen(suffix));
                LOGD("new file[%d] name=%s", i, log_file_name);
            }
            strncat(new_log_name,log_file_name,strlen(log_file_name));

            if (i >=3 && mRecycleIndex == 4) {
                LOGI("Recycle happen by Rename old file [%s] to above new  file.", old_log_name);
            } else {
                LOGI("Rename old file [%s] to above new file.", old_log_name);
            }
            rename(old_log_name, new_log_name);
        }
    }
}

/**
 * Get remaining storage in the given path, unit MB
 */
int getAvailableStorageSize(char* storage_path) {
    if (storage_path == NULL) {
        LOGE("Invalid storage path.");
        return 0;
    }

    struct statfs storage_stat;
    memset(&storage_stat, 0, sizeof(storage_stat));
    int retry_num = 5;
    while (retry_num > 0) {
        if (statfs(storage_path, &storage_stat) == -1) {
            LOGE(
                    "Fail to get storage information from given path[%s], retry_num=%d, error=%d(%s)",
                    storage_path, retry_num, errno, strerror(errno));
        } else {
            break;
        }
        usleep(200000);
        retry_num--;
    }
    if (retry_num <= 0) {
        LOGE("Fail to get storage information from given path[%s] after retry",
                storage_path);
        return 0;
    }

    if (storage_stat.f_blocks == 0) {
        LOGE("Storage is unavailable, maybe not mounted yet.");
        return 0;
    }
    long block_size = storage_stat.f_bsize;
    long block_num = storage_stat.f_bavail;
    int remaining_size = (int) (block_size * (block_num / 1024.0) / 1024);
    return remaining_size;
}

/**
 * Since trace will keep saving log into the same file(if size too large, add a number suffix to it),
 * in order to uniform recording log file, rename the temp log file dynamically
 *
 * log_still_running: whether log is still running, if running, do not operate the latest tmp file
 */
void rename_temp_log_file(char* log_folder_path, int log_still_running) {
    DIR *dir;
    struct dirent *ptr;
//    time_t t;
//    struct tm *timenow;

    dir = opendir(log_folder_path);
    if (dir == NULL) {
        LOGD(" rename_temp_log_file(), fail to open given folder: %s",
                log_folder_path);
        return;
    }

    long tmp_file_num = 0;
    long oldest_tmp_index = 0;
    char oldest_log_name[256] = { 0 };
    memset(oldest_log_name, '\0', 256 * sizeof(char));
    while ((ptr = readdir(dir)) != NULL) {
        char* tmp_suffix = NULL;
        if ((tmp_suffix = strstr(ptr->d_name, ".tmp")) != NULL) {
            char* tmp_index = tmp_suffix + strlen(".tmp");
            long index = atol(tmp_index);
            if (tmp_file_num == 0) {
                oldest_tmp_index = index;
                memset(oldest_log_name, '\0', 256 * sizeof(char));
                strncpy(oldest_log_name, ptr->d_name, sizeof(oldest_log_name) - 1);
                oldest_log_name[sizeof(oldest_log_name) - 1] = '\0';
            }
            tmp_file_num++;
            if (index < oldest_tmp_index) {
                oldest_tmp_index = index;
                memset(oldest_log_name, '\0', 256 * sizeof(char));
                strncpy(oldest_log_name, ptr->d_name, sizeof(oldest_log_name) - 1);
                oldest_log_name[sizeof(oldest_log_name) - 1] = '\0';
            }
        }
    }
    closedir(dir);

    //Rename the oldest temp file's name to **_start.cap
    if ((log_still_running != 0 && tmp_file_num > 1)
            || (log_still_running == 0 && tmp_file_num > 0)) {
        //LOGD("Should rename temp log file, and the oldest one is [%s] index is %d",
        //        oldest_log_name, oldest_tmp_index);
        char folder_path[256] = { 0 };
        strncpy(folder_path, log_folder_path, sizeof(folder_path) - 1);
        folder_path[sizeof(folder_path) - 1] = '\0';
        if (folder_path[strlen(folder_path) - 1] != '/') {
            strncat(folder_path, "/",strlen("/"));
        }

        char old_log_name[256] = { 0 };
        char new_log_name[256] = { 0 };
        char time_name_str[64] = { 0 };
        memset(old_log_name, '\0', 256 * sizeof(char));
        memset(new_log_name, '\0', 256 * sizeof(char));
        memset(time_name_str, '\0', 64 * sizeof(char));
        if (mEnableNewRotation) {
            strncpy(old_log_name, folder_path, sizeof(old_log_name) - 1);
            old_log_name[sizeof(old_log_name) - 1] = '\0';
            strncat(old_log_name, oldest_log_name, 255 - strlen(old_log_name));
            strncpy(new_log_name, folder_path, sizeof(new_log_name) - 1);
            new_log_name[sizeof(new_log_name) - 1] = '\0';
////////////////////////////////////////////////////////
            //// use rename point time
/*          time(&t);
            timenow = localtime(&t);
            strftime(time_name_str, 64, "tcpdump_NTLog_V2_%Y_%m%d_%H%M%S", timenow);
            strncat(new_log_name, time_name_str);*/
////////////////////////////////////////////////////////////// 
            /// use trace time and add V2 and index
            char * strName = strstr(oldest_log_name, "_start");
            if (strName != NULL) {
                *strName = '\0';
                char * strTime = &oldest_log_name[strlen("tcpdump_NTLog_")];
                strncat(new_log_name, "tcpdump_NTLog_V2_", 255 - strlen(new_log_name));
                strncat(new_log_name, strTime, 255 - strlen(new_log_name));
            } else {
                LOGE("Can not find _start string");
                return;
            }

//////////////////////////////////////////////////////////
            memset(time_name_str, '\0', 64 * sizeof(char));
            if (apply_rohc()){
                sprintf(time_name_str,"_start_%ld.snap",mTotalFileIndex);
            } else {
                sprintf(time_name_str,"_start_%ld.cap",mTotalFileIndex);
            }
            strncat(new_log_name, time_name_str, 255 - strlen(new_log_name));
            if (rename(old_log_name, new_log_name) == 0) {
                LOGD("New flow to [%s]", new_log_name);
                mTotalFileIndex++;
                char log_index[256];
                sprintf(log_index, "%ld", mTotalFileIndex);
                property_set(_KEY_FLAG_LOG_INDEX, log_index);
                usleep(10000);
            } else {
                LOGE("rename fail:%d, %s",errno,strerror(errno));
                LOGE("Fail rename to [%s]", new_log_name);
            }
            if (log_still_running != 0 || tmp_file_num > 1) {
                char mSuffix[256] = {0};
                memset(mSuffix, '\0', 256 * sizeof(char));
                if (apply_rohc()) {
                    strncpy(mSuffix, ".snap", sizeof(mSuffix) - 1);
                } else {
                    strncpy(mSuffix, ".cap", sizeof(mSuffix) - 1);
                }
                mSuffix[sizeof(mSuffix) - 1] = '\0';
                rotate_old_log_file(log_folder_path, mSuffix);
                rename_temp_log_file(log_folder_path, log_still_running);
            } else {
                LOGD("Log is already stopped, change the last temp log file's name and stop.");
            }
        } else {
            strncpy(old_log_name, folder_path, sizeof(old_log_name) - 1);
            old_log_name[sizeof(old_log_name) - 1] = '\0';
            strncat(old_log_name, oldest_log_name, 255 - strlen(old_log_name));
            strncpy(new_log_name, old_log_name, sizeof(new_log_name) - 1);
            new_log_name[sizeof(new_log_name) - 1] = '\0';

            char* break_point = strstr(new_log_name, ".tmp");
            if (break_point != NULL) {
                *break_point = '\0';
                //LOGD(" Should rename file [%s]", old_log_name);
                LOGD("Old flow to [%s]", new_log_name);
                rename(old_log_name, new_log_name);
                if (log_still_running != 0 || tmp_file_num > 1) {
                    char mSuffix[256] = {0};
                    memset(mSuffix, '\0', 256 * sizeof(char));
                    if (apply_rohc()) {
                        strncpy(mSuffix, ".snap", sizeof(mSuffix) - 1);
                    } else {
                        strncpy(mSuffix, ".cap", sizeof(mSuffix) - 1);
                    }
                    mSuffix[sizeof(mSuffix) - 1] = '\0';
                    refact_existing_log_file(log_folder_path, mSuffix);
                    rename_temp_log_file(log_folder_path, log_still_running);
                } else {
                    LOGD("Log is already stopped, change the last temp log file's name and stop.");
                }
            } else {
                LOGE("Found oldest tmp log file name [%s] is invalid",old_log_name);
            }
        }
    }
}

/**
 * Use this struct to transfer two parameter to thread
 */
struct threadpara {
    char* para1;
    char* para2;
};

/**
 * Check whether remaining storage is still enough for saving log
 */
void* monitor_storage(void* argv) {
    struct threadpara* paras = (struct threadpara *) argv;
    char root_path[256] = { 0 };
    char log_folder_path [256] =  { 0 };
    memset(root_path, '\0', 256 * sizeof(char));
    memset(log_folder_path, '\0', 256 * sizeof(char));

    strncpy(root_path, (*paras).para1, sizeof(root_path) - 1);
    root_path[sizeof(root_path) - 1] = '\0';

    strncpy(log_folder_path, (*paras).para2, sizeof(log_folder_path) - 1);
    log_folder_path[sizeof(log_folder_path) - 1] = '\0';

    LOGD("-->monitor_storage(), root path=%s, log folder=%s", root_path,
            log_folder_path);
//    int count = 0;
    char buffer[4] = "mo";
    while (1) {
        int remaining_storage = getAvailableStorageSize(root_path);
        if (remaining_storage < RESVERED_STORAGE) {
            buffer[1] = 'l';
            LOGE("Now we are running out of storage, remaining %d MB, Kill trace process now! pid=%d",
                    remaining_storage, getpid());
            property_set(_KEY_LOG_STOP_REASON, "ml");
            property_set(_KEY_FLAG_LOG_RUNNING, "0");
        } else if (strlen(log_folder_path) > 0) {
            //Use this to monitor whether current writing log folder still exist
            DIR *dir = opendir(log_folder_path);
            if (dir == NULL) {
                int retry_num = 10;
                while (retry_num > 0) {
                    LOGE(
                            "Fail to access current recording log folder, retry_num=%d, error=%d(%s)",
                            retry_num, errno, strerror(errno));
                    usleep(200000);
                    dir = opendir(log_folder_path);
                    if (dir != NULL) {
                        closedir(dir);
                        LOGE(
                                "Current recording log folder appear again, retry_num=%d",
                                retry_num);
                        break;
                    }
                    retry_num--;
                }
                if (retry_num <= 0) {
                    LOGE(
                            "Current recording log folder[%s] lost, stop NetworkLog now",
                            log_folder_path);
                    buffer[1] = 'g';
                    property_set(_KEY_LOG_STOP_REASON, "mg");
                }
            } else {
                closedir(dir);
            }
        }
        if (buffer[1] != 'o') {
            LOGE("Result flag = [%c], global_fd_tcpdump=%d", buffer[1], global_fd_tcpdump);

            //The trace process also need to be killed
            remaining_time = 10;
            stop_tcpdump(true);
            int deamon_pid = getpid();
            kill(deamon_pid, SIGTERM);
            break;
        }

        rename_temp_log_file(log_folder_path, 1);
        sleep(10);
    }
    pthread_exit(0);
    return (void*) NULL;
}

bool generateLogTreeFile(char* filePath, const char *data, size_t len, bool isnewcreate) {
    LOGD("Generate log folder tree file, isNewCreat:%d", isnewcreate);
//    char filePath[256];
 //   sprintf(filePath, FILE_TREE_NAME, SD_LOG_ROOT);
    if (filePath == NULL) {
        LOGE("File tree path is null");
        return false;
    }
    FILE *pFile = NULL;
    if (isnewcreate) {
            pFile = fopen(filePath, "wb+");
        } else {
            pFile = fopen(filePath, "ab+");
        }
    if (NULL == pFile) {
        LOGE("Failed to open/create tree file %s, errno=%d", filePath, errno);
        return false;
    }
    LOGD("File %s open.", filePath);

    size_t res = fwrite(data, sizeof(char), len, pFile);

    if (res != len) {
        fclose(pFile);
        pFile = NULL;
        return false;
    }

    size_t length = strlen("\r");
    res = fwrite("\r", sizeof(char), length, pFile);
    if (res != length) {
        fclose(pFile);
        pFile = NULL;
        return false;
    }

    LOGD("Success to write file tree file.");

    if (fclose(pFile) == EOF) {
        LOGE("Failed to close file tree file %s, errno=%d", filePath, errno);
        pFile = NULL;
        return false;
    }
    pFile = NULL;
    return true;
}

//int main(int argc , char **argv)
void tcpdump_data(int fd, char *local, char * NTLog_name, char * parameters) {
    LOGD(" tcpdump_data: fd = %d, local = %s, NTLog_name = %s, parameters = %s",
         fd, local, NTLog_name, parameters);
//    pid_t pid;
//    pid_t tcpdump_pid = 0;
    char buffer[4] = "to"; //t:trace,command has received
    int ret = 0;
//    time_t t;
//    struct tm *timenow;
    char file_path_time[MAX];
    //int flag = 0;
    //int pipe_fd[2];
    //FILE *fpout;
    int i;
    int file_size_5parts = 0;
    //-s parameter
    int package_limit_size = 0;
    char file_size_str[16];
    char package_size_str[16];
    char* dotpos = strchr(parameters, ','); //parameters may look like this: 200,-s90
    if (dotpos == NULL) {
        LOGD("-->tcpdump_data(), Parameter only contains log size, value=%s",
                parameters);
        if (mEnableNewRotation) {
            file_size_5parts = atoi(parameters);
            if (mEnableTestRotation) {
                file_size_5parts = 30;
            }
            if ( file_size_5parts / mLogFileMaxNum > MAX_FILE_SIZE) {
                mLogFileMaxNum = file_size_5parts / MAX_FILE_SIZE; // Max log file size 200M 
                if (file_size_5parts > mLogFileMaxNum * MAX_FILE_SIZE) {
                    mLogFileMaxNum++;
                    file_size_5parts = MAX_FILE_SIZE;
                } else {
                    file_size_5parts = file_size_5parts / mLogFileMaxNum; // Max log file size is 200
                }
            } else {
                  file_size_5parts = (file_size_5parts / mLogFileMaxNum); // Average file size below 200M
            }

        } else {
            file_size_5parts = (atoi(parameters) / 5); //trace log size will be divied into 5 parts
            if (file_size_5parts > 2147) {
                LOGE(" trace supported max log file size is 2147MB, limit %d to 2147", file_size_5parts);
                file_size_5parts = 2147;
            }
        }
        LOGD("each file size=%d, mLogFileMaxNum=%d ",file_size_5parts,mLogFileMaxNum);

    } else {
        LOGD(" -->tcpdump_data(), Extra parameters string=%s", dotpos);
        char* iter = parameters;
        int i = 0;
        while (*iter != ',') {
            file_size_str[i++] = *iter++;
        }
        file_size_str[i] = '\0';
        if (strlen(file_size_str) > 0) {
            
            if (mEnableNewRotation) {
                file_size_5parts = atoi(file_size_str);
                if (mEnableTestRotation) {
                    file_size_5parts = 30;
                }
                if ( file_size_5parts / mLogFileMaxNum > MAX_FILE_SIZE) {
                    mLogFileMaxNum = file_size_5parts / MAX_FILE_SIZE; // Max log file size 200M 
                    if (file_size_5parts > mLogFileMaxNum * MAX_FILE_SIZE) {
                        mLogFileMaxNum++;
                        file_size_5parts = MAX_FILE_SIZE;
                    } else {
                        file_size_5parts = file_size_5parts / mLogFileMaxNum; // Max log file size is 200
                    }
                } else {
                    file_size_5parts = (file_size_5parts / mLogFileMaxNum); // Average file size below 200M
                }
            
            } else {
                file_size_5parts = (atoi(file_size_str) / 5);
                if (file_size_5parts > 2147) {
                    LOGE("trace supported max log file size is 2147MB, limit %d to 2147", file_size_5parts);
                    file_size_5parts = 2147;
                }
            }
        }
        LOGD("file_size_str=%s, each file size=%d, mLogFileMaxNum=%d ", file_size_str,
                file_size_5parts,mLogFileMaxNum);

        i = 0;
        char* package_limit_flag_pos = strstr(dotpos, "-s");
        if (package_limit_flag_pos != NULL) {
            package_limit_flag_pos += 2;
            char c = *package_limit_flag_pos++;
            while (c != '\0' && c >= '0' && c <= '9') {
                package_size_str[i++] = c;
                c = *package_limit_flag_pos++;
            }
        }
        package_size_str[i] = '\0';
        if (strlen(package_size_str) > 0) {
            package_limit_size = atoi(package_size_str);
        }
        LOGD(" package_size_str=%s, each package limit:%d", package_size_str,
                package_limit_size);
    }

    char file_size_5parts_char[16] = { 0 };

    pthread_t monitor_storage_thread;
    char storage_root_path[256] = { 0 };
    support_rohc();
    LOGD("tcpdump_data:apply_rohc_compress=%d,is_support_rohc_compress=%d,rohc_total_file=%d",apply_rohc_compress,is_support_rohc_compress,rohc_total_file);

    if (is_support_rohc_compress && apply_rohc_compress) {
        mLogFileMaxNum = rohc_total_file;
        LOGD("apply mLogFileMaxNum to rohc total file:mLogFileMaxNum=%d",mLogFileMaxNum);
    } else {
        LOGE("Not apply mLogFileMaxNum to rohc total file:mLogFileMaxNum=%d",mLogFileMaxNum);
    }
    sprintf(file_size_5parts_char, "%d", file_size_5parts);
    char location[256] = { 0 };
    char sd_path_config[32] = { 0 };
//    char sd_path_property[256] = { 0 };
//    char * sd_path_env = NULL;
//    char file_config[MAX_NAME_LEN] =
//            "/data/data/com.android.ActivityNetwork/files/NetworkingSetting";
//    int fd_config;
//    char * logpath_config = NULL;
//    int ret_config = 0, i_config = 0;
    int getsd_result = get_sdpath_from_configfile(sd_path_config);
    LOGD("tcpdump_data(), getsd_result = %d, sd_path_config=%s", getsd_result,
            sd_path_config);
    if (getsd_result == -1) {
        LOGE(
                "Could not found or open configuration file, set it to %s by default",
                _VALUE_CONFIG_LOG_PATH_EMMC);
        strncpy(sd_path_config, _VALUE_CONFIG_LOG_PATH_EMMC, sizeof(sd_path_config) - 1);
        sd_path_config[sizeof(sd_path_config) - 1] = '\0';
    } else if (getsd_result == 0) {
        LOGE(
                "Could not found log path item from configuration file, set it to %s by default",
                _VALUE_CONFIG_LOG_PATH_EMMC);
        strncpy(sd_path_config, _VALUE_CONFIG_LOG_PATH_EMMC, sizeof(sd_path_config) - 1);
        sd_path_config[sizeof(sd_path_config) - 1] = '\0';
    }

    /*get correct sdpath*/
    if (!strcmp(sd_path_config, _VALUE_CONFIG_LOG_PATH_DATA)) {
        strncpy(location, "/data", sizeof(location) - 1);
        location[sizeof(location) - 1] = '\0';
    }
    if (strlen(mStoragePath) > 2) {
        strncpy(location, mStoragePath, sizeof(location) - 1);
        location[sizeof(location) - 1] = '\0';
        LOGD("location set as mStoragePath  %s", location);
    } else {
        LOGD("location is  %s", location);
    }

    strncpy(storage_root_path, location, sizeof(storage_root_path) - 1);
    storage_root_path[sizeof(storage_root_path) - 1] = '\0';
    global_fd_tcpdump = fd;
/*
    i = check_dir(location, "debuglogger");
    if (i == 0) {
        if (chdir(location) < 0) {
            perror("chdir");
            buffer[1] = 'd';
            LOGD("has some error check_dir debugloggerfolder, write to connection %s",
                    buffer);
            goto ret_back;
        }
        if (mkdir("debuglogger", DIR_MODE) == -1) {
            //Create debuglogger folder fail, maybe it has already been created by other log tool, re-check this
            usleep(200);
            i = check_dir(location, "debuglogger");
            if (i != 1) {
                perror("mkdir incorrect");
                buffer[1] = 'd';
                LOGD(" mkdir debuglogger fail, write to connection %s", buffer);
                goto ret_back;
            } else {
                LOGD(
                        " mkdir debuglogger fail, but this folder exist again, just go on");
            }
        }
    } else if (i == -1) {
        buffer[1] = 'd';

        LOGD("has some error check_dir debuglogger");
        goto ret_back;
    } else if (i == 1) {
        LOGD("check_dir debuglogger has exist");
    }*/
    /////////////////////////////////////////////////////////////
    char debugloggerfolder[16] = "/debuglogger";
//    int i_5part = 0;
//    char * str_start = NULL;
    char tcpdump_stop[256];
    char tcpdump_cmd[256];
    char * argv[11] = { 0 };
    char tcpdump_test_log_path[16] = "/netlog";
    char interface[8] = "any";
    char mFileTreePath[MAX_NAME_LEN] = {0};
    char file_name[128] = { 0 };
    int retry_number = 0;
    char str_200[8] = "200";

    i = check_dir(location, "debuglogger");
    if (i == 0) {
        strncat(location, debugloggerfolder,strlen(debugloggerfolder));
        i = strlen(location);
        strncat(location, "/",strlen("/"));
        if (makeDir(location) <= 0) {
            buffer[1] = 'd';
            LOGE("makeDir debuglogger fail 2016/10/26");
            goto ret_back;
        }
        location[i] = '\0';
    } else if (i == -1) {
        buffer[1] = 'd';
        LOGE("has some error check_dir -1 debugloggerfolder 2016/10/26");
        goto ret_back;
    } else if (i == 1) {
        LOGD("check_dir debugloggerfolder has exist 2016/10/26");
        strncat(location, debugloggerfolder,strlen(debugloggerfolder));
    }
    ///////////////////////////////////////////////////////////
    i = check_dir(location, "netlog");
    if (i == 0) {
        if (chdir(location) < 0) {
            perror("chdir");
            buffer[1] = 'd';
            LOGD("has some error checkdir netlog");
            goto ret_back;
            //exit(-1);
        }
        if (myMkdir("netlog", DIR_MODE) == -1) {
            LOGE("mkdir: Failed to create directory netlog, errno=%d",  errno);
            buffer[1] = 'd';
            LOGD("has some error mkdir netlog, errno=%d", errno);
            goto ret_back;
            //exit(1);
        }

    } else if (i == -1) {
        buffer[1] = 'd';
        LOGD("has some error check_dir netlog");
        goto ret_back;
    } else if (i == 1) {
        LOGD("check_dir netlog has exist");
    }

    strncat(location, tcpdump_test_log_path,strlen(tcpdump_test_log_path));

    sprintf(mFileTreePath, "%s/file_tree.txt", location);

    LOGD("checkdir NTLog_name:%s begin~~", NTLog_name);
    i = check_dir(location, NTLog_name);
    if (i == 0) {
        if (chdir(location) < 0) {
            perror("chdir");
            LOGD("checkdir :%s error", location);
            buffer[1] = 'd';
            goto ret_back;
        }
        if (myMkdir(NTLog_name, DIR_MODE) == -1) {
            LOGE("mkdir: Failed to create directory NTLog_name, errno=%d",  errno);
            LOGE("mkdir :%s error", location);
            buffer[1] = 'd';
            goto ret_back;
        }

    } else if (i == -1) {
        buffer[1] = 'd';
        LOGD("has some error check_dir NTLog");
        goto ret_back;
    } else if (i == 1) {
        LOGD("check_dir NTLog has exist");
    }
    strncat(location, "/",strlen("/"));
    strncat(location, NTLog_name,strlen(NTLog_name));

    if (property_set(_KEY_LOG_SAVING_PATH, location) == 0) {
        LOGD("persist debuglogger system property success");
    } else {
        LOGE("persist debuglogger system property fail");
    }
    LOGD("Log was started, set log running flag to 1");
    property_set(_KEY_FLAG_LOG_RUNNING, "1");

    if (fd) {
    	char resultBuffer[128] = "tcpdump_sdcard_start_";
    	int success = 1;
    	if (buffer[1] != 'o') {
    		success = 0;
    	}
    	sprintf(resultBuffer, "tcpdump_sdcard_start_%s,%d", parameters, success);
        ret = write(fd, resultBuffer, strlen(resultBuffer));
        LOGD("run trace command, write to connection %s", resultBuffer);
    }

    struct threadpara paras;
    paras.para1 = storage_root_path;
    paras.para2 = location;

    //Begin check remaining storage thread
    if (pthread_create(&monitor_storage_thread, NULL, monitor_storage,
            &paras)) {
        LOGE("Fail to create log storage monitor thread.");
        return;
    }
    generateLogTreeFile(mFileTreePath,location,strlen(location),false);

    mTotalFileIndex = 1;
    property_set(_KEY_FLAG_LOG_INDEX, "1");
    struct sigaction act, oact;
    act.sa_handler = terminate_tcpdump_data_sig_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGTERM, &act, &oact) < 0) {
        LOGE("sigaction for trace data failed");
    }
    
    retry_number = 0;
    while (1) {
        property_get(_KEY_FLAG_LOG_BREAKLOOP, tcpdump_stop, "0");
        if (strcmp(tcpdump_stop, "1") == 0) {
            LOGD("Stop flag become to 1, stop dump now.");
            sleep(2);
            return;
        }
        if (retry_number > 5) {
            LOGD("has some error call trace, even after many retry times");
            property_set(_KEY_LOG_STOP_REASON, "tf");
            return;
        }
        strncpy(file_name, location, sizeof(file_name) - 1);
        file_name[sizeof(file_name) - 1] = '\0';
        if (apply_rohc()) {
            sprintf(file_path_time, "/tcpdump_%s_start.snap.tmp -!", NTLog_name);
        } else {
            sprintf(file_path_time, "/tcpdump_%s_start.cap.tmp", NTLog_name);
        }
        strncat(file_name, file_path_time,strlen(file_path_time));
        char str_tcpdump[16] = "trace";
        char str_s[4] = "-s";
        char str_0[4] = "0";
        char str_u[4] = "-U";
        char str_i[4] = "-i";
        char str_w[4] = "-w";
        char str_c[4] = "-C";

        argv[0] = str_tcpdump;
        argv[1] = str_s;
        argv[2] = str_0;
        argv[3] = str_u;
        argv[4] = str_i;
        argv[5] = interface;
        argv[6] = str_w;
        argv[7] = file_name;
        argv[8] = str_c;
        argv[10] = NULL;

        LOGD("checkdir NTLog_name:%s begin~~, log size:%s", file_name,
                file_size_5parts_char);
        if (file_size_5parts > 0) {
            argv[9] = file_size_5parts_char;
        } else {
            argv[9] = str_200;
        }
        //Whether we need to limit each package's size
        int count = 0;
        if (package_limit_size <= 0) {
            count = (atoi(argv[9])*1000000)/900;
            //modify trace command because of high loss log rate
            sprintf(tcpdump_cmd, "trace -i %s -s 0 -c %d -w %s 2>&1",
                                interface,count, file_name);
        } else {
            count = (atoi(argv[9])*1000000)/package_limit_size;
            sprintf(tcpdump_cmd, "trace -i %s -s %s -c %d -w %s  2>&1",
                                           interface, package_size_str, count, file_name);
        }

        LOGD("system mtk_trace_cmd:%s ", tcpdump_cmd);
//        tcpdump_main(10, (char **)argv);

        int uid = getuid();
        LOGE("Directly call trace in system begin, uid=%d, errno=%d, retry_num=%d",
                uid, errno, retry_number);

        int status = my_system(tcpdump_cmd);
        retry_number++;
//        int status = system(tcpdump_cmd);
//        int result = execv("/system/xbin/trace", argv);
//        int status = popen(tcpdump_cmd, "r");

        //Judge trace running status
        if (status == -1) {
            LOGE("system() error! error num=%d (%s)", errno, strerror(errno));
        } else {
            LOGD("system() exit status value=%d, error num=%d (%s)", status,
                    errno, strerror(errno));
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                    LOGD(" execute trace successfully.");
                } else {
                    LOGE(" execute trace fail, exit code = %d",
                            WEXITSTATUS(status));
                }
            } else {
                LOGE(" call trace fail, exit code = %d", WEXITSTATUS(status));
            }
        }

        //Add a sleep to avoid dead loop print too much duplicated log
        sleep(1);
    }

ret_back:
        if (fd) {
        	char resultBuffer[128] = "tcpdump_sdcard_start_";
        	int success = 1;
        	if (buffer[1] != 'o') {
        		success = 0;
        	}
        	sprintf(resultBuffer, "tcpdump_sdcard_start_%s,%d", parameters, success);
        	ret = write(fd, resultBuffer, strlen(resultBuffer));
        	LOGD("run trace command, write to connection %s", resultBuffer);
    }

    return;
}

void terminate_thread_sig_handler(int s) {
    LOGE("Receive terminate signal =%d in rename log file sub thread, exit.", s);
    pthread_exit(0);
}

int flag_rename_log_done = 0;
void* check_logfile_when_stop(void* argv) {
    support_rohc();
    LOGD("-->check_logfile_when_stop() begin1 apply_rohc_compress=%d,is_support_rohc_compress=%d",apply_rohc_compress,is_support_rohc_compress);
    //Receive signal to terminate this thread
    struct sigaction act, oact;
    act.sa_handler = terminate_thread_sig_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGUSR1, &act, &oact) < 0) {
        LOGE("sigaction in stop to rename log thread failed");
//        return;
    }

    char log_index[256];
    property_get(_KEY_FLAG_LOG_INDEX, log_index,"1");
    mTotalFileIndex = atol(log_index);                
    struct threadpara* paras = (struct threadpara *) argv;
    char* root_path = (*paras).para1;
    rename_temp_log_file(root_path, 0);
    flag_rename_log_done = 1;
    LOGD("<--check_logfile_when_stop() end");
    pthread_exit(0);
    return (void*) NULL;
}

void stop_tcpdump(bool setStopProp) {
    LOGD("-->stop_tcpdump(): setStopProp = %d",setStopProp);
    char log_folder_path[256];
    property_get(_KEY_LOG_SAVING_PATH, log_folder_path, "");
    property_set(_KEY_FLAG_LOG_BREAKLOOP, "1");

    //Stop existing trace process
    pid_t tcpdump_pid = 0;
    int tcpdump_pid_no = 0;
    int pid_j = 0;
    pid_t tcpdump_pid_arr[4] = { 0 };
    tcpdump_pid_no = find_pid(tcpdump_pid_arr);
    if (tcpdump_pid_no != 0) {
        LOGD("At stop time, find %d running trace process", tcpdump_pid_no);
        for (pid_j = 0; pid_j < tcpdump_pid_no; pid_j++) {
            tcpdump_pid = tcpdump_pid_arr[pid_j];
            LOGD("trace is running,pid is %d, try to kill it now.",
                    tcpdump_pid);
            if ((tcpdump_pid != 0) && (tcpdump_pid != -1)) {
                kill(tcpdump_pid, SIGTERM);
            }
        }
    }
    sleep(1);
    remaining_time--;
    pthread_t rename_logfile_thread;
    if (strlen(log_folder_path) > 0) {
        LOGD("Log saving path=[%s], need rename temp log file at stop time",
            log_folder_path);
    //        rename_temp_log_file(log_folder_path, 0);
        flag_rename_log_done = 0;
        struct threadpara paras;
        paras.para1 = log_folder_path;
        if (pthread_create(&rename_logfile_thread, NULL,
            check_logfile_when_stop, &paras)) {
            LOGE("Fail to create log file check thread at stop time");
            flag_rename_log_done = 1;
        } else {
            pthread_join(rename_logfile_thread, NULL);
            LOGI("Wait rename log file thread done");
        }
    } else {
        flag_rename_log_done = 1;
    }
    //set check log folder timeout standard to 10s
    while (!flag_rename_log_done && remaining_time > 0) {
        sleep(1);
        remaining_time--;
        LOGD("Waiting checking log file at stop time stop, remaining time=%d s, flag_rename_log_done=%d",
            remaining_time, flag_rename_log_done);
    }
    if (!flag_rename_log_done) {
        LOGE("Fail to finish rename temp log file name in given time, just force it finish directly");
        pthread_kill(rename_logfile_thread, SIGUSR1);
        LOGD("Terminate not finished stop thread done.");
        sleep(1);
        remaining_time--;
    }
    if (setStopProp) {
        property_set(_KEY_FLAG_LOG_RUNNING, "0");
        LOGD("stop_tcpdump set running property 0.");
    }
}

int my_system(const char* cmd) {
    LOGD("-->my_system(), cmd=%s", cmd);
    FILE * fp;
    int res;
    char buf[1024];
    if (cmd == NULL) {
        LOGE(" my_system commandd is NULL!");
        return -1;
    }
    if ((fp = popen(cmd, "r")) == NULL) {
        LOGE(" popen error: %d[%s]", errno, strerror(errno));
        return -1;
    } else {
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            LOGD(" my_system response str is [%s]", buf);
        }
        if ((res = pclose(fp)) == -1) {
            LOGE(" close popen file pointer fp error: %d[%s]", errno,
                    strerror(errno));
            return res;
        } else if (res == 0) {
            LOGD(" my_system response value=%d", res);
            return res;
        } else {
            LOGD(" my_system response value=%d", res);
            return res;
        }
    }
}

