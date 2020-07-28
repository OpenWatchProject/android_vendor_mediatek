#ifndef __OP01_LOG_H__
#define __OP01_LOG_H__


#ifdef __cplusplus
extern "C" {
#endif

//#define MTK_GNSS_OP01_LOG_SUPPORT

int op01_log_init();

int op01_log_gps_start();
int op01_log_gps_stop();
int op01_log_gps_location(double lat, double lng, int ttff);

#ifdef __cplusplus
}
#endif

#endif



