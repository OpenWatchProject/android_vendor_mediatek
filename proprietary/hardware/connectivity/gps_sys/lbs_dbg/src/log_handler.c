#include "log_handler.h"
#include "mtk_socket_data_coder.h"
#include "LbsLogInterface.h"


//sender, used for Main Thread to dispatch data to Work thread.
bool Lbs_openLog(const char* dist, char* filepath);
bool Lbs_writeLog(const char* dist, char* log, int log_size);
bool Lbs_closeLog(const char* dist);

//receiver, used for Work Thread as basic hanlder
//init function
int gps_debug_logd_init();
static void lbs_debug_logd_database_init();

//all work will be handled by mnld2lbslogd_hdlr
static bool mnld2lbslogd_hdlr(int fd, lbs_log_thread_database* database, lbslogd_handler_interface* hdlr);


//for all log, we provide three operation as below
static bool lbslogd_handle_open_logfile(const char* path, lbs_log_thread_database* database);
static bool lbsogd_handle_write_logdata(char* data, int len, lbs_log_thread_database* database);
static bool lbslogd_handle_close_logfile(lbs_log_thread_database* database);

//below is for gps log
static bool gps_log_file_rename(lbs_log_thread_database* database);
static bool gps_log_file_split(lbs_log_thread_database* database);
static bool gps_log_dir_check(lbs_log_thread_database* database);
static bool find_gps_log_file(char* filename, lbs_log_thread_database* database);

//below is for mpe log
static bool mpe_log_file_split(lbs_log_thread_database* database);
static bool mpe_log_dir_check(lbs_log_thread_database* database);

//below is for dump log
static bool dump_log_file_rename(lbs_log_thread_database* database);


//below is basic func useful
bool split_path(const char* file, char* pathname, char* filename, int len_pathname, int len_filename);
bool split_ext(const char* name, char* basename, char* extension, int len_basename, int len_extension);
bool pthread_mkdirs(const char* muldir, bool* dir_created);
int get_file_size(char* folder, char *filename);
bool cmp_file_time(char* dirname1, char *filename1, char* dirname2, char *filename2, int* deltaT);
bool lbs_dbg_create_mutex(lbs_dbg_mutex_enum mutex_idx);
void lbs_dbg_take_mutex(lbs_dbg_mutex_enum mutex_idx);
void lbs_dbg_give_mutex(lbs_dbg_mutex_enum mutex_idx);

static lbslogd_handler_interface g_lbslogd_handler_interface = {
    lbslogd_handle_open_logfile,
    lbsogd_handle_write_logdata,
    lbslogd_handle_close_logfile
};

static lbs_log_thread_database gpslog_database;
static lbs_log_thread_database mpelog_database;
static lbs_log_thread_database dumplog_database;

pthread_mutex_t g_lbsdMutex[LBS_DBG_MUTEX_MAX];

bool lbs_dbg_create_mutex(lbs_dbg_mutex_enum mutex_idx) {
    if (mutex_idx >= LBS_DBG_MUTEX_MAX) {
        LOGD("lbs_dbg_create_mutex fail,mutex_idx error\n");
        return false;
    }

    if (pthread_mutex_init(&g_lbsdMutex[mutex_idx], NULL)) {
        LOGD("lbs_dbg_create_mutex fail(%s)\n", strerror(errno));
        return false;
    }
    return true;
}

void lbs_dbg_take_mutex(lbs_dbg_mutex_enum mutex_idx) {
    if (mutex_idx >= LBS_DBG_MUTEX_MAX) {
        LOGD("lbs_dbg_take_mutex fail,mutex_idx error\n");
        return;
    }

    if (pthread_mutex_lock(&g_lbsdMutex[mutex_idx])) {
        LOGD("lbs_dbg_take_mutex fail(%s)\n", strerror(errno));
    }
}

void lbs_dbg_give_mutex(lbs_dbg_mutex_enum mutex_idx) {
    if (mutex_idx >= LBS_DBG_MUTEX_MAX) {
        LOGD("lbs_dbg_give_mutex fail,mutex_idx error\n");
        return;
    }

    if (pthread_mutex_unlock(&g_lbsdMutex[mutex_idx])) {
        LOGD("lbs_dbg_give_mutex fail(%s)\n", strerror(errno));
    }
}

static bool dump_log_file_rename(lbs_log_thread_database* database) {
    char filename[MAX_PATH_LEN] = {0};

    if (NULL == database) {
       LOGE("dump_log_file_rename, database not valid");
       return false;
    }

    if (0 == strlen(database->log_name_base)) {
        LOGE("[%s]dump_log_file_rename base name is NULL", database->sock_addr);
        return false;
    }

    MNLD_STRNCPY(filename, database->log_name_base, MAX_PATH_LEN);
    filename[strlen(filename)-1] = '\0';

    if(rename(database->log_name_base, filename) != 0) {
        LOGE("[%s]rename file %s fail(%s)", database->sock_addr, filename, strerror(errno));
        return false;
    }
    return true;
}

static bool gps_log_file_split(lbs_log_thread_database* database) {
    char file_name_split[MAX_PATH_LEN] = {0};
    char file_name_split_suffix[MAX_PATH_LEN] = {0};
    FILE* fp = NULL;

    if (NULL == database) {
       LOGE("gps_log_file_split, database not valid");
       return false;
    }

    if (database->current_log_size >= database->max_log_size) {
        if (!lbslogd_handle_close_logfile(database)) {
            LOGE("[%s]gps_log_file_split close file fail", database->sock_addr);
        }
        database->split_number++;

        snprintf(file_name_split_suffix, MAX_PATH_LEN, "-%d", database->split_number);
        if (strlen(file_name_split_suffix) + strlen(database->log_name_base) + strlen(database->log_name_suffix_writing) >= MAX_PATH_LEN) {
            LOGE("[%s]gps_log_file_split(), log name too long, %ld, %ld", database->sock_addr,
                strlen(file_name_split_suffix),strlen(database->log_name_base));
            database->split_number--;
            return false;
        } else {
            snprintf(file_name_split, MAX_PATH_LEN, "%s%s%s", database->log_name_base, file_name_split_suffix, database->log_name_suffix_writing);
            fp = fopen(file_name_split, "wb");

            if (NULL == fp) {
                LOGE("[%s]open log: %s fail,  reason=[%s]", database->sock_addr, file_name_split, strerror(errno));
                database->split_number--;
                return false;
            }
            database->file = fp;
            database->current_log_size = 0;
            database->current_log_num += 1;
            return true;
        }
    }

    return true;
}


bool cmp_file_time(char* dirname1, char *filename1, char* dirname2, char *filename2, int* deltaT) {
    char full_filename1[MAX_PATH_LEN] = {0};
    char full_filename2[MAX_PATH_LEN] = {0};
    struct stat statbuff1;
    struct stat statbuff2;


    if (NULL == filename1 || NULL == filename2 || NULL == dirname1 || NULL == dirname2) {
        LOGE("[cmp_file_time][error]: File name is NULL!!\r\n");
        return false;
    }

    if (((strlen(dirname1) + strlen(filename1)) >= MAX_PATH_LEN)
        ||((strlen(dirname2) + strlen(filename2)) >= MAX_PATH_LEN)) {
        LOGE("log name too long\n");
        return false;
    }

    snprintf(full_filename1, sizeof(full_filename1), "%s%s", dirname1, filename1);
    snprintf(full_filename2, sizeof(full_filename2), "%s%s", dirname2, filename2);

    if (stat(full_filename1, &statbuff1) < 0) {
        LOGD("[cmp_file_time][error]: open file(%s) state  fail(%s)!!\r\n", full_filename1, strerror(errno));
        return false;
    }

    if (stat(full_filename2, &statbuff2) < 0) {
        LOGD("[cmp_file_time][error]: open file(%s) state  fail(%s)!!\r\n", full_filename2, strerror(errno));
        return false;
    }
    *deltaT = statbuff1.st_mtime - statbuff2.st_mtime;
    return true;
}


int get_file_size(char* folder, char *filename) {
    char dir_filename[MAX_PATH_LEN] = {0};
    struct stat statbuff;

    if ((NULL == filename) || (NULL == folder)) {
        LOGE("[get_file_size][error]: File name is NULL!!\r\n");
        return 0;
    }

    if (strlen(folder) + strlen(filename) >= MAX_PATH_LEN) {
        LOGE("[get_file_size][error]: File name too long!!\r\n");
        return 0;
    }

    snprintf(dir_filename, sizeof(dir_filename), "%s%s", folder, filename);

    if (stat(dir_filename, &statbuff) < 0) {
        LOGE("[get_file_size][error]: open file(%s) state fail(%s)!\r\n", dir_filename, strerror(errno));
        return 0;
    } else {
        return statbuff.st_size;  // return the file size
    }
}


static bool find_gps_log_file(char* filename, lbs_log_thread_database* database) {
    int i;
    int len = 0;

    if (NULL == database) {
       LOGE("find_gps_log_file, database not valid");
       return false;
    }

    len = strlen(database->log_name_prefix);

    for (i = 0; i < len; i++) {
        if (filename[i]!= *(database->log_name_prefix + i)) {
            return false;
        }
    }
    return true;
}

static bool gps_log_dir_check(lbs_log_thread_database* database) {   // file size check

    char temp_filename[MAX_PATH_LEN];
    char OldGpsFile[MAX_PATH_LEN] = {0};
    char OldFile[MAX_PATH_LEN];
    unsigned int DirLogSize_temp;
    struct dirent *p_dirent;
    int total_file_count = 0;
    int deltaT = 0;
    DIR *p_dir;

    if (NULL == database) {
       LOGE("gps_log_dir_check, database not valid");
       return false;
    }

    /*when android boot up or creat new folder, current_folder_size=0 and
    current_log_num = 0, so it will  Mandatory check dir */
    if ((database->current_folder_size != 0) || (database->current_log_num !=0)) {
        if ((database->current_folder_size <= database->max_folder_size)
            && (database->current_log_num <= database->max_log_num)) {
            return true;
        }
    }

    do {
        //LOGD("[%s]gps_log_dir_check begin", database->sock_addr);
        if (0 != access(database->log_folder_name, F_OK|R_OK)) {
            LOGE("[%s]Access gps debug log dir fail(%s)", database->sock_addr, strerror(errno));
            return false;
        }

        if ((p_dir = opendir(database->log_folder_name)) == NULL) {
            LOGE("[%s]open dir error(%s)", database->sock_addr, strerror(errno));
            return false;
        }

        total_file_count = 0;
        memset(OldGpsFile, 0x00, MAX_PATH_LEN);  // For compare file name,set a max char value
        DirLogSize_temp = 0;

        while (1) {
            p_dirent = readdir(p_dir);
            if (NULL == p_dirent) {
                break;
            }

            if (0 != access(database->log_folder_name, F_OK)) {  // return value 0:success, -1 : fail
                LOGE("[%s]Access gps debug log dir fail(%s)", database->sock_addr, database->log_folder_name);
                break;
            }
            if (strcmp(p_dirent->d_name, ".") == 0 || strcmp(p_dirent->d_name, "..") == 0) {  // Ignore the "." & ".."
                continue;  // while((p_dirent=readdir(p_dir)) && !g_gpsdbglogThreadExit)
            }

            if (MAX_PATH_LEN > strlen(p_dirent->d_name)) {
                memset(temp_filename, 0 , MAX_PATH_LEN);
                MNLD_STRNCPY(temp_filename, (void *)&p_dirent->d_name, MAX_PATH_LEN - 1);
            } else {  // The length of d_name is too long, ignore this file
                LOGW("[%s]d_name is too long", database->sock_addr);
                continue;
            }

            if (find_gps_log_file(temp_filename, database)) {
                DirLogSize_temp += get_file_size(database->log_folder_name, temp_filename);
                total_file_count++;
                if (strncmp(OldGpsFile, temp_filename, strlen(database->log_name_prefix)) != 0) {
                    MNLD_STRNCPY(OldGpsFile, temp_filename, MAX_PATH_LEN - 1);
                    //LOGD("copy file name to OldGpsFile: %s, and continue\r\n", OldGpsFile);
                    continue;  // while((p_dirent=readdir(p_dir)) && !g_gpsdbglogThreadExit)
                }

                // Find the latest old file
                if (cmp_file_time(database->log_folder_name, temp_filename, database->log_folder_name, OldGpsFile, &deltaT)) {
                    if (deltaT < 0) {
                        memset(OldGpsFile, '\0', MAX_PATH_LEN);
                        MNLD_STRNCPY(OldGpsFile, temp_filename, MAX_PATH_LEN - 1);
                    }
                }
            }
        }
        LOGD("folder size measured:%d, folder size theoretical:%d, logNum measured:%d, logNum theoretical:%d, latest file:%s",
            DirLogSize_temp, database->current_folder_size, total_file_count, database->current_log_num, OldGpsFile);
        closedir(p_dir);
        if ((DirLogSize_temp > database->max_folder_size) || (total_file_count > database->max_log_num)) {
            // Over size or the number of GPS debug log file over the limitation
            // when OldGpsFile is small, it will cause many re-calculation in the second loop.  need to avoid it.
            int ret = 0;

            ret = get_file_size(database->log_folder_name, OldGpsFile);
            memset(OldFile, 0x00, sizeof(OldFile));
            snprintf(OldFile, sizeof(OldFile), "%s%s", database->log_folder_name, OldGpsFile);

            //LOGD("[%s]need delete OldFile:%s\r\n", database->sock_addr, OldFile);

            if (remove(OldFile) != 0) {  // Error handle
                LOGE("[%s]Remove file %s error(%s)", database->sock_addr, OldFile, strerror(errno));
            }
            DirLogSize_temp = DirLogSize_temp - ret;
            total_file_count--;
        }
        //LOGD("[%s]After remove file gpsdebug log dir size:%d!\r\n", database->sock_addr, DirLogSize_temp);
    }while ((DirLogSize_temp > database->max_folder_size)
                    || (total_file_count > database->max_log_num));

    //LOGD("[%s]dir size:%d\r\n", database->sock_addr, DirLogSize_temp);
    database->current_folder_size = DirLogSize_temp;
    database->current_log_num = total_file_count;
    return true;
}


static bool gps_log_file_rename(lbs_log_thread_database* database) {
    char tmp_suffix = 'a'-1;
    char filename_current[MAX_PATH_LEN] = {0};
    char filename_rename[MAX_PATH_LEN] = {0};
    char filename_base[MAX_PATH_LEN] = {0};

    if (NULL == database) {
       LOGE("gps_log_file_rename, database not valid");
       return false;
    }

    if (0 == strlen(database->log_name_base)) {
        LOGE("[%s]gps_log_file_rename base name is NULL", database->sock_addr);
        return false;
    }

    if (0 == database->split_number) {
        MNLD_STRNCPY(filename_current, database->log_name_base, MAX_PATH_LEN);
        snprintf(filename_current, MAX_PATH_LEN, "%s%s", database->log_name_base, database->log_name_suffix_writing);
        snprintf(filename_base, MAX_PATH_LEN, "%s", database->log_name_base);
    } else {
        snprintf(filename_current, MAX_PATH_LEN, "%s-%d%s", database->log_name_base, database->split_number, database->log_name_suffix_writing);
        snprintf(filename_base, MAX_PATH_LEN, "%s-%d", database->log_name_base, database->split_number);
    }

    if (access(filename_current, F_OK) != 0) {
        LOGE("[%s]gps_log_file_rename filename_current : %s not exit!", database->sock_addr, filename_current);
        return false;
    }

    //Get new file name , storage into filename2
    do {
        if(tmp_suffix < 'a') {
            snprintf(filename_rename, MAX_PATH_LEN, "%s%s", filename_base, database->log_name_suffix);
        } else if(tmp_suffix <= 'z') {
            snprintf(filename_rename, MAX_PATH_LEN, "%s_%c%s", filename_base, tmp_suffix, database->log_name_suffix);
        } else {
            snprintf(filename_rename, MAX_PATH_LEN, "%s%s", filename_base, database->log_name_suffix);
            break;
        }
        tmp_suffix++;
    } while (0 == access(filename_rename, F_OK));  //File has existed

    //LOGD("[%s]Writing file: %s, rename to:%s", database->sock_addr, filename_current, filename_rename);
    if(rename(filename_current, filename_rename) != 0) {
       LOGE("[%s]gps_log_file_rename rename fail:%s", database->sock_addr, strerror(errno));
    }
    return true;
}

static bool mpe_log_file_split(lbs_log_thread_database* database) {
    char file_name_split[MAX_PATH_LEN] = {0};
    char file_name_split_suffix[MAX_PATH_LEN] = {0};
    FILE* fp = NULL;

    if (NULL == database) {
       LOGE("mpe_log_file_split, database not valid");
       return false;
    }

    if (database->current_log_size >= database->max_log_size) {
        if (!lbslogd_handle_close_logfile(database)) {
            LOGE("[%s]mpe_log_file_split close file fail", database->sock_addr);
        }
        database->split_number++;

        snprintf(file_name_split_suffix, MAX_PATH_LEN, "-%d", database->split_number);
        if (strlen(file_name_split_suffix) + strlen(database->log_name_base) + strlen(database->log_name_suffix_writing) >= MAX_PATH_LEN) {
            LOGE("[%s]mpe_log_file_split(), log name too long, %ld, %ld", database->sock_addr,
                strlen(file_name_split_suffix),strlen(database->log_name_base));
            database->split_number--;
            return false;
        } else {
            snprintf(file_name_split, MAX_PATH_LEN, "%s%s%s", database->log_name_base, file_name_split_suffix, database->log_name_suffix_writing);
            fp = fopen(file_name_split, "wb");

            if (NULL == fp) {
                LOGE("[%s]open log: %s fail,  reason=[%s]", database->sock_addr, file_name_split, strerror(errno));
                database->split_number--;
                return false;
            }
            database->file = fp;
            database->current_log_size = 0;
            database->current_log_num += 1;
            return true;
        }
    }

    return true;
}

static bool mpe_log_dir_check(lbs_log_thread_database* database) {   // file size check

    char temp_filename[MAX_PATH_LEN];
    char OldGpsFile[MAX_PATH_LEN] = {0};
    char OldFile[MAX_PATH_LEN];
    unsigned int DirLogSize_temp;
    struct dirent *p_dirent;
    int total_file_count = 0;
    int deltaT = 0;
    DIR *p_dir;

    if (NULL == database) {
       LOGE("mpe_log_dir_check, database not valid");
       return false;
    }

    /*when android boot up or creat new folder, current_folder_size=0 and
    current_log_num = 0, so it will  Mandatory check dir */
    if ((database->current_folder_size != 0) || (database->current_log_num !=0)) {
        if ((database->current_folder_size <= database->max_folder_size)
            && (database->current_log_num <= database->max_log_num)) {
            return true;
        }
    }

    do {
        //LOGD("[%s]mpe_log_dir_check begin", database->sock_addr);
        if (0 != access(database->log_folder_name, F_OK|R_OK)) {
            LOGE("[%s]Access mpe debug log dir fail(%s)", database->sock_addr, strerror(errno));
            return false;
        }

        if ((p_dir = opendir(database->log_folder_name)) == NULL) {
            LOGE("[%s]open dir error(%s)", database->sock_addr, strerror(errno));
            return false;
        }

        total_file_count = 0;
        memset(OldGpsFile, 0x00, MAX_PATH_LEN);  // For compare file name,set a max char value
        DirLogSize_temp = 0;

        while (1) {
            p_dirent = readdir(p_dir);
            if (NULL == p_dirent) {
                break;
            }

            if (0 != access(database->log_folder_name, F_OK)) {  // return value 0:success, -1 : fail
                LOGE("[%s]Access mpe debug log dir fail(%s)", database->sock_addr, database->log_folder_name);
                break;
            }
            if (strcmp(p_dirent->d_name, ".") == 0 || strcmp(p_dirent->d_name, "..") == 0) {  // Ignore the "." & ".."
                continue;  // while((p_dirent=readdir(p_dir)) && !g_gpsdbglogThreadExit)
            }

            if (MAX_PATH_LEN > strlen(p_dirent->d_name)) {
                memset(temp_filename, 0 , MAX_PATH_LEN);
                MNLD_STRNCPY(temp_filename, (void *)&p_dirent->d_name, MAX_PATH_LEN - 1);
            } else {  // The length of d_name is too long, ignore this file
                LOGW("[%s]d_name is too long", database->sock_addr);
                continue;
            }

            if (find_gps_log_file(temp_filename, database)) {
                DirLogSize_temp += get_file_size(database->log_folder_name, temp_filename);
                total_file_count++;
                if (strncmp(OldGpsFile, temp_filename, strlen(database->log_name_prefix)) != 0) {
                    MNLD_STRNCPY(OldGpsFile, temp_filename, MAX_PATH_LEN - 1);
                    //LOGD("copy file name to OldGpsFile: %s, and continue\r\n", OldGpsFile);
                    continue;  // while((p_dirent=readdir(p_dir)) && !g_gpsdbglogThreadExit)
                }

                // Find the latest old file
                if (cmp_file_time(database->log_folder_name, temp_filename, database->log_folder_name, OldGpsFile, &deltaT)) {
                    if (deltaT < 0) {
                        memset(OldGpsFile, '\0', MAX_PATH_LEN);
                        MNLD_STRNCPY(OldGpsFile, temp_filename, MAX_PATH_LEN - 1);
                    }
                }
            }
        }
        LOGD("folder size measured:%d, folder size theoretical:%d, logNum measured:%d, logNum theoretical:%d, latest file:%s",
            DirLogSize_temp, database->current_folder_size, total_file_count, database->current_log_num, OldGpsFile);
        closedir(p_dir);
        if ((DirLogSize_temp > database->max_folder_size) || (total_file_count > database->max_log_num)) {
            // Over size or the number of GPS debug log file over the limitation
            // when OldGpsFile is small, it will cause many re-calculation in the second loop.  need to avoid it.
            int ret = 0;

            ret = get_file_size(database->log_folder_name, OldGpsFile);
            memset(OldFile, 0x00, sizeof(OldFile));
            snprintf(OldFile, sizeof(OldFile), "%s%s", database->log_folder_name, OldGpsFile);

            //LOGD("[%s]need delete OldFile:%s\r\n", database->sock_addr, OldFile);

            if (remove(OldFile) != 0) {  // Error handle
                LOGE("[%s]Remove file %s error(%s)", database->sock_addr, OldFile, strerror(errno));
            }
            DirLogSize_temp = DirLogSize_temp - ret;
            total_file_count--;
        }
        //LOGD("[%s]After remove file mpedebug log dir size:%d!\r\n", database->sock_addr, DirLogSize_temp);
    }while ((DirLogSize_temp > database->max_folder_size)
                    || (total_file_count > database->max_log_num));

    //LOGD("[%s]dir size:%d\r\n", database->sock_addr, DirLogSize_temp);
    database->current_folder_size = DirLogSize_temp;
    database->current_log_num = total_file_count;
    return true;
}

bool split_ext(const char* name, char* basename, char* extension, int len_basename, int len_extension) {

    char fullname[MAX_PATH_LEN];
    char* p = NULL;

    if ((strlen(name) ==0) || (strlen(name) >= MAX_PATH_LEN)) {
        LOGE("path lenth out of range,  len=%ld", strlen(name));
        return false;
    }

    memset(fullname, 0x0, MAX_PATH_LEN);
    memset(basename, 0x0, len_basename);
    memset(extension, 0x0, len_extension);
    MNLD_STRNCPY(fullname, name, MAX_PATH_LEN);

    p = strrchr(fullname, '.');

    //"." not found
    if (p == NULL) {
        MNLD_STRNCPY(basename, name, len_basename);
    //"." is in the end of a file, it is a error.
    } else if (p == (fullname + strlen(fullname) - 1)) {
        return false;
    } else {
        MNLD_STRNCPY(extension, p, len_extension);
        *p = '\0';
        MNLD_STRNCPY(basename, fullname, len_basename);
    }

    LOGD("split_ext(), basename: %s, extension :%s", basename, extension);
    return true;
}


bool split_path(const char* file, char* pathname, char* filename, int len_pathname, int len_filename) {

    char fullpath[MAX_PATH_LEN];
    char* p = NULL;

    if ((strlen(file) ==0) || (strlen(file) >= MAX_PATH_LEN)) {
        LOGE("path lenth out of range,  len=%ld", strlen(file));
        return false;
    }

    memset(fullpath, 0x0, MAX_PATH_LEN);
    memset(pathname, 0x0, len_pathname);
    memset(filename, 0x0, len_filename);
    MNLD_STRNCPY(fullpath, file, MAX_PATH_LEN);

    p = strrchr(fullpath, '/');

    if (p == NULL) {
        MNLD_STRNCPY(filename, fullpath, len_filename);
    } else if (p == (fullpath + strlen(fullpath) - 1)) {
        MNLD_STRNCPY(pathname, fullpath, len_pathname);
    } else {
        p++;
        MNLD_STRNCPY(filename, p, len_filename);
        *p = '\0';
        MNLD_STRNCPY(pathname, fullpath, len_pathname);
    }

    return true;
}


/*
parameter:
    dir_created: if true, mkdirs did makedir before
             if false, mkdirs did not create dir before
    note, dir_created == false, did not mean we call this api fail.

return value:
    true: access dir successfully, maybe did not create dir before.
    false: access dir fail, due to dir not exist and create fail.
*/
bool pthread_mkdirs(const char* muldir, bool* dir_created) {

    int i;
    char str[MAX_PATH_LEN];
    //*dir_created = false;
    int mkdir_ret = 0;

    if ((strlen(muldir) ==0) || (strlen(muldir) >= MAX_PATH_LEN)) {
        LOGE("path lenth out of range,  len=%ld", strlen(muldir));
        return false;
    }

    memset(str, 0x0, sizeof(str));
    MNLD_STRNCPY(str, muldir, MAX_PATH_LEN);

    for (i = 1; i < MAX_PATH_LEN; i++) {
        if (str[i] == '/') {
            str[i] = '\0';
            if(access(str, 0) != 0) {
                lbs_dbg_take_mutex(LBS_DBG_CREATE_DIR);
                mkdir_ret = mkdir(str, 0770);
                lbs_dbg_give_mutex(LBS_DBG_CREATE_DIR);
                if (mkdir_ret == -1) {
                    LOGE("mkdirs mkdir %s fail,  reason=[%s]%d",
                            str, strerror(errno), errno);
                    return false;
                } else {
                    *dir_created = true;
                }
            }
            str[i]='/';
        }
    }

    return true;
}


int safe_recvfrom(int fd, char* buff, int len) {
    int ret = 0;
    int retry = 10;

    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){
        LOGE("fcntl failed reason=[%s]%d",
                    strerror(errno), errno);
    }

    while ((ret = recvfrom(fd, buff, len, 0,
         NULL, NULL)) == -1) {
        LOGD("safe_recvfrom() ret=%d len=%d", ret, len);
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("safe_recvfrom() recvfrom() failed reason=[%s]%d",
            strerror(errno), errno);
        break;
    }
    return ret;
}

static bool lbslogd_handle_open_logfile(const char* full_file_name, lbs_log_thread_database* database) {
    #define MKDIR_RETRY_MAX_NUM  10
    char pathname[MAX_PATH_LEN] = {0};//folder name such as : /storage/emulated/0/mtklog/connsyslog/gpshost/CSLog_2018_0702_095210
    char filename[MAX_PATH_LEN] = {0};//file name received from mnld such as : /storage/emulated/0/mtklog/connsyslog/gpshost/CSLog_2018_0702_095210/GPS_HOST_2018_0702_100223
    char file_name_with_suffix[MAX_PATH_LEN] = {0};//file name with suffix .nma.curf such as : /storage/emulated/0/mtklog/connsyslog/gpshost/CSLog_2018_0702_095210/GPS_HOST_2018_0702_100223.nma.curf
    bool dir_created = false;
    int i = 0;
    FILE* fp = NULL;

    if (NULL == database) {
       LOGE("lbslogd_handle_open_logfile, database not valid");
       return false;
    }

    LOGD("[%s]lbslogd_handle_open_logfile", database->sock_addr);

    if (!split_path(full_file_name, pathname, filename, sizeof(pathname), sizeof(filename))) {
        LOGE("[%s]lbslogd_handle_open_logfile split_path error", database->sock_addr);
        return false;
    }

    if ((strlen(pathname) ==0) || (strlen(filename) ==0)) {
        LOGE("[%s]lbslogd_handle_open_logfile split_path fail, pathname=%s, filename=%s", database->sock_addr, pathname, filename);
        return false;
    }

    if ((strlen(database->log_name_suffix_writing))+strlen(full_file_name) >= MAX_PATH_LEN) {
        LOGE("[%s]lbslogd_handle_open_logfile file path too long, full_file_name=%s", database->sock_addr, full_file_name);
        return false;
    }


    for (i=0; i<MKDIR_RETRY_MAX_NUM; i++) {
        if (!pthread_mkdirs(pathname, &dir_created)) {
           LOGE("[%s]lbslogd_handle_open_logfile mkdirs error", database->sock_addr);
           usleep(100*1000);
        } else {
            break;
        }
    }

    snprintf(file_name_with_suffix, MAX_PATH_LEN, "%s%s", full_file_name, database->log_name_suffix_writing);
    fp = fopen(file_name_with_suffix, "wb");

    if (NULL == fp)
    {
        LOGE("[%s]lbslogd_handle_open_logfile open log: %s fail,  reason=[%s]", database->sock_addr, file_name_with_suffix, strerror(errno));
        return false;
    }

    if (!lbslogd_handle_close_logfile(database)) { // protect
        LOGE("[%s]lbslogd_handle_open_logfile close log when open %s fail,  reason=[%s]", database->sock_addr, file_name_with_suffix, strerror(errno));
    }

    memset(database->log_folder_name, 0x0, MAX_PATH_LEN);
    memset(database->log_name_base, 0x0, MAX_PATH_LEN);
    MNLD_STRNCPY(database->log_folder_name, pathname, MAX_PATH_LEN);
    MNLD_STRNCPY(database->log_name_base, full_file_name, MAX_PATH_LEN);
    database->file = fp;
    database->current_log_size = 0;
    database->split_number = 0;
    database->current_log_num += 1;

    if (false == dir_created) {
        if (database->check_dir) {
            if (!database->check_dir(database)) {
                LOGE("[%s]lbslogd_handle_open_logfile check dir fail", database->sock_addr);
            }
        }
    } else {
        database->current_folder_size = 0;
        database->current_log_num = 0;
        LOGD("[%s]lbslogd_handle_open_logfile mkdir:%s success", database->sock_addr, pathname);
    }
    LOGD("[%s]Create debug log file %s successfully", database->sock_addr, file_name_with_suffix);
    return true;
}

static bool lbsogd_handle_write_logdata(char* data, int len, lbs_log_thread_database* database) {
    int count = 0;

    if (NULL == database) {
       LOGE("lbsogd_handle_write_logdata, database not valid");
       return false;
    }

    if (database->file != NULL) {
        if (database->check_dir) {
            if (!database->check_dir(database)) {
                LOGE("[%s]lbsogd_handle_write_logdata check dir fail", database->sock_addr);
            }
        }
        count = fwrite(data, 1, len, database->file);

        if (0 == count) {
           LOGE("[%s]lbsogd_handle_write_logdata write fail", database->sock_addr);
           return false;
        }

        database->current_log_size += count;
        database->current_folder_size += count;

        //LOGD("[%s]lbsogd_handle_write_logdata, write % data, current log size = %d", database->sock_addr, count, database->current_log_size);

        if (database->split_log) {
            if (!database->split_log(database)) {
                LOGE("[%s]lbsogd_handle_write_logdata split_log fail", database->sock_addr);
            }
        }
    }
    return true;
}

static bool lbslogd_handle_close_logfile(lbs_log_thread_database* database) {
    bool ret = true;
    LOGD("[%s]lbslogd_handle_close_logfile", database->sock_addr);
    if (database->file != NULL) {
        LOGD("[%s]lbslogd_handle_close_logfile, file = %p", database->sock_addr, database->file);
        if (fclose(database->file) != 0) {
           LOGE("[%s]lbslogd_handle_close_logfile close fail, reason=[%s]", database->sock_addr, strerror(errno));
           ret = false;
        }
        database->file = NULL;

        if (database->log_rename) {
            if (!database->log_rename(database)) {
                LOGE("[%s]lbslogd_handle_close_logfile log_rename fail", database->sock_addr);
            }
        }
    }
    return ret;
}

static bool mnld2lbslogd_hdlr(int fd, lbs_log_thread_database* database, lbslogd_handler_interface* hdlr) {
    char buff[LBS_LOG_INTERFACE_BUFF_SIZE] = {0};
    int offset = 0;

    LbsLogInterface_message_id cmd;
    int read_len = 0;
    bool ret = true;

    if (NULL == database) {
       LOGE("mnld2lbslogd_hdlr, database not valid");
       return false;
    }

    if (NULL == hdlr) {
       LOGE("[%s]mnld2lbslogd_hdlr, hdlr not valid", database->sock_addr);
       return false;
    }

    if (fd < 0) {
       LOGE("[%s]mnld2lbslogd_hdlr, fd not valid", database->sock_addr);
       return false;
    }
    read_len = safe_recvfrom(fd, buff, sizeof(buff));
    if (read_len <= 0) {
        LOGE("[%s]mnld2lbslogd_hdlr() safe_recvfrom() failed read_len=%d", database->sock_addr, read_len);
        return false;
    }

    cmd = mtk_socket_get_int(buff, &offset);

    switch (cmd) {
    case LBS_LOG_INTERFACE_OPEN_LOG: {
        if (hdlr->create_logfile) {
            char path[LBS_LOG_INTERFACE_BUFF_SIZE] = {0};
            mtk_socket_get_string(buff, &offset, path, sizeof(path));
            if (!hdlr->create_logfile(path, database)) {
                LOGE("[%s]mnld2lbslogd_hdlr() create_logfile fail", database->sock_addr);
            }
        } else {
            LOGE("[%s]mnld2lbslogd_hdlr() create_logfile is NULL", database->sock_addr);
            ret = false;
        }
        break;
    }

    case LBS_LOG_INTERFACE_WRITE_LOG: {
        if (hdlr->write_logdata) {
            unsigned int len_logdata = mtk_socket_get_int(buff, &offset);
            if (len_logdata <= LBS_LOG_INTERFACE_BUFF_SIZE) {
                if (!hdlr->write_logdata(buff+offset, len_logdata, database)) {
                    LOGE("[%s]mnld2lbslogd_hdlr() write_logdata fail", database->sock_addr);
                }
            } else {
                LOGE("[%s]mnld2lbslogd_hdlr() len_logdata:%u overflow", database->sock_addr, len_logdata);
                ret = false;
            }
        } else {
            LOGE("[%s]mnld2lbslogd_hdlr() write_logdata is NULL", database->sock_addr);
            ret = false;
        }
        break;
    }

    case LBS_LOG_INTERFACE_CLOSE_LOG: {
        if (hdlr->close_logfile) {
            if (!hdlr->close_logfile(database)) {
                LOGE("[%s]mnld2lbslogd_hdlr() close_logfile fail", database->sock_addr);
            }
        } else {
            LOGE("[%s]mnld2lbslogd_hdlr() close_logfile is NULL", database->sock_addr);
            ret = false;
        }
        break;
    }

    default: {
        LOGE("[%s]mnld2lbslogd_hdlr() unknown cmd=%d", database->sock_addr, cmd);
        ret = false;
        break;
    }
    }

    return ret;
}

static void* log_write_thread_pool(void *database) {
    #define EPOLL_MAX_NUM 4
    int fd = -1;
    int i = 0;
    int n = 0;
    int epfd = epoll_create(EPOLL_MAX_NUM);
    struct epoll_event events[EPOLL_MAX_NUM];
    lbs_log_thread_database* data = (lbs_log_thread_database*)database;

    if (NULL == database) {
        LOGE("log_write_thread_pool exit, database not valid");
        return 0;
    }

    LOGD("[%s]log_write_thread_pool created", data->sock_addr);


    if (-1 == epfd) {
        LOGE("[%s]log_write_thread_pool epoll_create failure, reason=[%s]", data->sock_addr, strerror(errno));
        return 0;
    }

    fd = socket_bind_udp(data->sock_addr);

    if (fd < 0) {
        LOGE("[%s]log_write_thread_pool create fd failed, reason=[%s]", data->sock_addr, strerror(errno));
        return 0;
    }

    if (epoll_add_fd(epfd, fd) == -1) {
        LOGE("[%s]log_write_thread_pool failed for fd_hal failed, reason=[%s]", data->sock_addr, strerror(errno));
        return 0;
    }

    while (1) {
        n = epoll_wait(epfd, events, EPOLL_MAX_NUM , -1);
        if (-1 == n) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("[%s]log_write_thread_pool epoll_wait failure reason=[%s]", data->sock_addr, strerror(errno));
                return 0;
            }
        }
        for (i = 0; i < n; i++) {
            if (events[i].data.fd == fd) {
                if (events[i].events & EPOLLIN) {
                    if (!mnld2lbslogd_hdlr(fd, data, &g_lbslogd_handler_interface)) {
                        LOGE("[%s]mnld2lbslogd_hdlr failure reason=[%s]", data->sock_addr, strerror(errno));
                    }
                }

            } else {
                LOGE("[%s]mnld_main_thread() unknown fd=%d", data->sock_addr, events[i].data.fd);
            }
        }
    }
    LOGE("[%s]log_write_thread_pool exit", data->sock_addr);
    return 0;
}


static void lbs_debug_logd_database_init() {
    memset(&gpslog_database, 0x0, sizeof(gpslog_database));
    memset(&mpelog_database, 0x0, sizeof(mpelog_database));
    memset(&dumplog_database, 0x0, sizeof(dumplog_database));

    //GPS LOG config
    gpslog_database.sock_addr  = "gpsLog";
    gpslog_database.log_name_suffix = ".nma";
    gpslog_database.log_name_suffix_writing = ".nma.curf";
    gpslog_database.log_name_prefix = "GPS_HOST";
    gpslog_database.max_log_size = 25*1024*1024;
    gpslog_database.max_folder_size = 1024*1024*1024;
    gpslog_database.max_log_num = 1000;

    gpslog_database.log_rename = gps_log_file_rename;
    gpslog_database.check_dir = gps_log_dir_check;
    gpslog_database.split_log = gps_log_file_split;

    //MPE LOG config
    mpelog_database.sock_addr  = "mpeLog";
    mpelog_database.log_name_suffix = ".txt";
    mpelog_database.log_name_suffix_writing  = ".txt";
    mpelog_database.log_name_prefix  = "se_ut";
    mpelog_database.max_log_size = 25*1024*1024;
    mpelog_database.max_folder_size = 1024*1024*1024;
    mpelog_database.max_log_num = 1000;

    mpelog_database.check_dir = mpe_log_dir_check;
    mpelog_database.split_log = mpe_log_file_split;

    //DUMP LOG config
    dumplog_database.sock_addr  = "dumpLog";
    dumplog_database.log_name_suffix = ".dump";
    dumplog_database.log_name_prefix = "";
    dumplog_database.log_name_suffix_writing = "";
    dumplog_database.log_rename = dump_log_file_rename;
}


int gps_debug_logd_init() {
    pthread_t pthread_gps;
    pthread_t pthread_mpe;
    pthread_t pthread_dump;

    LOGD("gps_debug_logd_init");
    lbs_debug_logd_database_init();
    if (!lbs_dbg_create_mutex(LBS_DBG_CREATE_DIR)) {
        LOGE("gps_debug_logd_init create mutex Fail, reason=[%s]", strerror(errno));
        return -1;
    }
    pthread_create(&pthread_gps, NULL, log_write_thread_pool, (char*)(&gpslog_database));
    pthread_create(&pthread_mpe, NULL, log_write_thread_pool, (char*)(&mpelog_database));
    pthread_create(&pthread_dump, NULL, log_write_thread_pool, (char*)(&dumplog_database));
    return 0;
}


//sender
bool Lbs_openLog(const char* dist, char* filepath) {
    char _buff[LBS_LOG_INTERFACE_BUFF_CMD_SIZE] = {0};
    int offset = 0;
    memset(_buff, 0x0, sizeof(_buff));

    mtk_socket_put_int(_buff, &offset, LBS_LOG_INTERFACE_OPEN_LOG);
    mtk_socket_put_string(_buff, &offset, filepath);
    return safe_sendto(dist, _buff, offset);
}

bool Lbs_writeLog(const char* dist, char* log, int log_size) {
    char _buff[LBS_LOG_INTERFACE_BUFF_SIZE] = {0};
    int offset = 0;
    memset(_buff, 0x0, sizeof(_buff));

    mtk_socket_put_int(_buff, &offset, LBS_LOG_INTERFACE_WRITE_LOG);
    mtk_socket_put_char_array(_buff, &offset, log, log_size);
    return safe_sendto(dist, _buff, offset);
}
bool Lbs_closeLog(const char* dist) {
    char _buff[LBS_LOG_INTERFACE_BUFF_CMD_SIZE] = {0};
    int offset = 0;
    memset(_buff, 0x0, sizeof(_buff));

    mtk_socket_put_int(_buff, &offset, LBS_LOG_INTERFACE_CLOSE_LOG);
    return safe_sendto(dist, _buff, offset);
}



