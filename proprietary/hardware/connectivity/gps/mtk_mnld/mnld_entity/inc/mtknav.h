#ifndef __MTKNAV_H__
#define __MTKNAV_H__


#ifdef __cplusplus
extern "C" {
#endif

#define MTKNAV_DL_RETRY_TIME   3
#define MTKNAV_MD5_FILE     "/data/vendor/gps/MTKNAV.MD5"
#define MTKNAV_MD5_FILE_HAL "/data/vendor/gps/MTKNAVHAL.MD5"
#define MTKNAV_DAT_FILE     "/data/vendor/gps/MTKNAV.DAT"
#define MTKNAV_DAT_FILE_HAL "/data/vendor/gps/MTKNAVHAL.DAT"

extern bool mtknav_update_flag;
extern int mtknav_res;

int mtknav_downloader_init();

int mtknav_downloader_start();

void mtknav_update_mtknav_file(int mtknav_valid);

#ifdef __cplusplus
}
#endif

#endif



