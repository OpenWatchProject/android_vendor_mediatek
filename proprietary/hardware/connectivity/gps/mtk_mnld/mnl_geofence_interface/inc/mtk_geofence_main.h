#ifndef __MTK_GEOFENCE_H__
#define __MTK_GEOFENCE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>
#include "mtk_gps_type.h"
#include "Geofence.h"
#include "mtk_flp_main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GEOFENCE_VERSION_HEAD 'G','F','C','_','V','E','R','_'
#define GEOFENCE_MAJOR_VERSION '1','7','0','6','1','9','0','1'  // y,y,m,m,d,d,rev,rev
#define GEOFENCE_BRANCH_INFO '_','0','.','5','8','_'
#define GEOFENCE_MINER_VERSION 'N','1'
#define GEOFENCE_VER_INFO GEOFENCE_VERSION_HEAD,GEOFENCE_MAJOR_VERSION,GEOFENCE_BRANCH_INFO,GEOFENCE_MINER_VERSION


//======================================================
// GEOFENCE -> GPS/GEOFENCE HAL
//======================================================
#define GEOFENCE_TO_MNL "gfc_to_mnl"
#define MNL_TO_GEOFENCE "mnl_to_gfc"

#define MTK_GFC_SUCCESS     (0)
#define MTK_GFC_ERROR       (-1)
#define MTK_GFC_TIMEOUT     (-2)

#define MNLD_GFC_CAPABILITY_AP_MODE      1
#define MNLD_GFC_CAPABILITY_OFL_MODE     2
#define MNLD_GFC_CAPABILITY_AP_OFL_MODE  3

#define GFC_MNL_BUFF_SIZE   (1 * 1024)
#define GFC_MNL_PAYLOAD_LEN 256

typedef struct {
    unsigned int type;
    unsigned int session;
    unsigned int length;
    unsigned char  data[GFC_MNL_PAYLOAD_LEN];
} MTK_GFC_MNL_MSG_T;

typedef struct {
    int (*mnld_gfc_attach)();
    void (*mnld_gfc_hbd_gps_open)();
    void (*mnld_gfc_hbd_gps_close)();
    void (*mnld_gfc_ofl_link_open)();
    void (*mnld_gfc_ofl_link_close)();
    int (*mnld_gfc_ofl_link_send)();
} gfc2mnl_interface;


//Function Declarations
extern MTK_GFC_Session_T mnld_gfc_session;

int create_gfchal2mnl_fd();
int mtk_hal2gfc_main_hdlr(int fd, gfc2mnl_interface* hdlr);
int mnld_gfc_gen_new_session();
int mnl2gfc_mnld_reboot();
int mnl2gfc_data_send(unsigned char *buf, unsigned int len);
int mnld_gfc_attach_done();
int mnld_gfc_session_update();
int mnld_gfc_ofl_gps_open_done();
int mnld_gfc_hbd_gps_open_done();
int mnld_gfc_ofl_gps_close_done();
int mnld_gfc_hbd_gps_close_done();
void mtk_gfc_set_sys_mode (unsigned int sysMode);
int mtk_gfc_get_sys_mode ();
int mnl2gfc_hdlr(MTK_GFC_MNL_MSG_T* prmsg);
int gfc_kernel_2gfc_hdlr(MTK_FLP_MSG_T* prmsg);
void mtk_gfc_ofl2mnl_process(MTK_FLP_MSG_T *prmsg);
int mtk_gfc_controller_reset_status(void);
int mtk_gfc_controller_process(MTK_FLP_MSG_T *prmsg);
int gfc2mnl_hdlr(MTK_GFC_MNL_MSG_T  *prMsg);
int mnl_send2gfc(const char* buff, int len);


#ifdef __cplusplus
}
#endif

#endif

