/*
 *    Filename: c2k_support.h
 *
 * Description: Flashless functions for CBP FSM file 
 */

#include "libnvram.h"
#include "libfile_op.h"


#define FSM_FILE_PATH_RF "/mnt/vendor/nvdata/md_via/fsm_rf.img"
#define FSM_FILE_PATH_CL "/mnt/vendor/nvdata/md_via/fsm_cl.img"

extern int fsm_calibration_backup(const char *path);
extern int fsm_calibration_restore(const char *path);
extern int reset_modem_host(void);

extern char *get_customized_external_sdcard_path(void);
extern char *get_customized_internal_sdcard_path(void);
extern void notify_taglog_ramdump_event(char *ramdump_img_path);
