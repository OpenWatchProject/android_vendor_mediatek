#ifndef __MPE_H__
#define __MPE_H__

#ifdef __cplusplus
extern "C" {
#endif
int mpe_function_init(void);
int mnl_mpe_thread_init();
int mnl2mpe_set_log_path(char* path, int status_flag, int mode_flag);

#ifdef __cplusplus
}
#endif

#endif

