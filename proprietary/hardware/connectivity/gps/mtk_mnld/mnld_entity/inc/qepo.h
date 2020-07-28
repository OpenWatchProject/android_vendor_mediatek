#ifndef __QEPO_H__
#define __QEPO_H__


#ifdef __cplusplus
extern "C" {
#endif

#define QEPO_FILE                    "/data/vendor/gps/QEPO.DAT"
#define QEPO_UPDATE_FILE             "/data/vendor/gps/QEPOTMP.DAT"
#define QEPO_BD_FILE                 "/data/vendor/gps/QEPO_BD.DAT"
#define QEPO_BD_UPDATE_FILE          "/data/vendor/gps/QEPOTMP_BD.DAT"
#define QEPO_GA_FILE                 "/data/vendor/gps/QEPO_GA.DAT"
#define QEPO_GA_UPDATE_FILE          "/data/vendor/gps/QEPOTMP_GA.DAT"
#define QEPO_UPDATE_HAL              "/data/vendor/gps/QEPOHAL.DAT"
#define QEPO_BD_UPDATE_HAL           "/data/vendor/gps/QEPO_BD_HAL.DAT"
#define QEPO_BD_HAS_EPO_BIT_MASK     (0x00000001)
#define MTK_QEPO_BD_HEADER_SIZE     72
#define QEPO_BD_DL_RETRY_TIME   3
#define QEPO_GA_UPDATE_HAL           "/data/vendor/gps/QEPO_GA_HAL.DAT"
#define QEPO_GA_HAS_EPO_BIT_MASK     (0x00000001)
#define MTK_QEPO_GA_HEADER_SIZE     72
#define QEPO_GA_DL_RETRY_TIME   3
#define QEPO_GR_DL_RETRY_TIME   3

extern bool qepo_update_flag;
extern int qepo_dl_res;
extern bool qepo_BD_update_flag;
extern int qepo_bd_dl_res;
extern bool qepo_GA_update_flag;
extern int qepo_ga_dl_res;

int qepo_downloader_init();

int qepo_downloader_start();

int qepo_bd_downloader_start();

int qepo_ga_downloader_start();

int is_qepo_download_finished();

int is_qepo_bd_download_finished();

int is_qepo_ga_download_finished();

void qepo_update_quarter_epo_file(int qepo_valid);

void qepo_update_quarter_epo_bd_file(int qepo_bd_valid);

void qepo_update_quarter_epo_ga_file(int qepo_ga_valid);

void gps_mnl_set_gps_time(int wn, int tow, int sys_time);

void qepo_invalid_dl_cnt_clear(void);

#ifdef __cplusplus
}
#endif

#endif



