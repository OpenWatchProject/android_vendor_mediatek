#include <log/log.h>     /*logging in logcat*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "geofencehal_worker.h"

#if 1
#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[gps_dbg_log]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[gps_dbg_log] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[gps_dbg_log] ERR: ", __VA_ARGS__);
#define  TRC(f)       ((void)0)
#else
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#define  TRC(f)       ALOGD("%s", __func__)
#endif
#endif

/**********************************************************
 *  Define                                                *
 **********************************************************/
typedef void*(*threadptr)(void*);
#define GEOFENCE_INJECT_LOC_ENUM 600

#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)


/**********************************************************
 *  Global Variables                                      *
 **********************************************************/
static MTK_GEOFENCE_PROPERTY_T geofence_property[MAX_GOEFENCE];
static float injectFenceInfo[MAX_GOEFENCE][3];
static int gTotalFence = 0; /*current fence number, must < MAX_GEOFENCE*/
static int  g_ThreadExitGfcJniSocket = 0, isgfc_exist = 0;
static pthread_t geofence_hal_jni_thread_id;
static char g_gfc2mnl_path[128] = GEOFENCE_TO_MNL;
int  g_gfc_server_socket_fd;
unsigned int gConfigHalGeofence = 0;

/**********************************************************
 *  Function Declaration                                  *
 **********************************************************/
void mnl2gfchal_jni_thread(void);


/**********************************************************
 *  Socket Function                                       *
 **********************************************************/
static int safe_recvfrom(int sockfd, char* buf, int len) {
    int ret = 0;
    int retry = 10;

    while ((ret = recvfrom(sockfd, buf, len, 0, NULL, NULL)) == -1) {
        //LOGW("ret=%d len=%d\n", ret, len);
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("sendto reason=[%s]\n", strerror(errno));
        break;
    }
    return ret;
}

// -1 means failure
static int safe_sendto(int sockfd, const char* dest, const char* buf, int size) {
    int len = 0;
    struct sockaddr_un soc_addr;
    int retry = 10;

    memset(&soc_addr, 0, sizeof(soc_addr));
    soc_addr.sun_path[0] = 0;
    MNLD_STRNCPY(soc_addr.sun_path + 1, dest,sizeof(soc_addr.sun_path) - 1);
    soc_addr.sun_family = AF_UNIX;

    while ((len = sendto(sockfd, buf, size, 0, (const struct sockaddr *)&soc_addr, sizeof(soc_addr))) == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("sendto dest=[%s] len=%d reason=[%s]\n",dest, size, strerror(errno));
        break;
    }
    return len;
}

static int gfc_send2mnl(const char* buff, int len) {
    int ret = 0;
    int sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        LOGE("gfc_send2mnl() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }

    if (safe_sendto(sockfd, g_gfc2mnl_path, buff, len) < 0) {
        LOGE("gfc_send2mnl safe_sendto failed\n");
        ret = -1;
    }
    close(sockfd);
    return ret;
}

static int bind_udp_socket(char* path) {
    int sockfd;
    struct sockaddr_un soc_addr;

    sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOGE("socket failed reason=[%s]\n", strerror(errno));
        return -1;
    }
    memset(&soc_addr, 0, sizeof(soc_addr));
    soc_addr.sun_path[0] = 0;
    MNLD_STRNCPY(soc_addr.sun_path + 1, path,sizeof(soc_addr.sun_path) - 1);
    soc_addr.sun_family = AF_UNIX;
    unlink(soc_addr.sun_path);

    if (bind(sockfd, (struct sockaddr *)&soc_addr, sizeof(soc_addr)) < 0) {
        LOGE("bind failed path=[%s] reason=[%s]\n", path, strerror(errno));
        close(sockfd);
        return -1;
    }

    return sockfd;
}


/**********************************************************
 *  Geofence sub-Functions                                *
 **********************************************************/
static void set_buff_init_fence() {
    int i;
    memset(&geofence_property, 0, MAX_GOEFENCE*sizeof(MTK_GEOFENCE_PROPERTY_T));
    for(i = 0; i < MAX_GOEFENCE; i++) {
        geofence_property[i].geofence_id = -1;
    }
    gTotalFence = 0;
    return;
}

static int set_buff_pause_fence(const int item) {
    geofence_property[item].alive = 0;
    geofence_property[item].last_transition = GPS_GEOFENCE_UNCERTAIN;
    geofence_property[item].latest_state = uncertain;
    return MTK_GFC_SUCCESS;
}

static int set_buff_resume_fence(const int item, const int transition) {
    geofence_property[item].monitor_transition = transition;
    geofence_property[item].alive = 1;
    return MTK_GFC_SUCCESS;
}

static int set_buff_remove_fence(const int item) {
    geofence_property[item].alive = 0;
    memset(&geofence_property[item],0,sizeof(MTK_GEOFENCE_PROPERTY_T));
    geofence_property[item].geofence_id = -1;
    if(gTotalFence > 0) {
        gTotalFence--;
    }
    return MTK_GFC_SUCCESS;
}

int set_buff_transition_fence(const int32_t fence_id, const int transition) {
    int item;

    if (transition != GPS_GEOFENCE_ENTERED &&
        transition != GPS_GEOFENCE_EXITED &&
        transition != GPS_GEOFENCE_UNCERTAIN) {
        LOGE("transition type is invalid %d", transition);
        return MTK_GFC_ERROR;
    }

    item = check_buff_fence_exist(fence_id);
    if (item < 0) {
        LOGE("transition fence id not exist");
        return MTK_GFC_ERROR;
    }
    geofence_property[item].last_transition = transition;
    geofence_property[item].latest_state = transition >> 1;
    return MTK_GFC_SUCCESS;
}

void mtk_geo_inject_info_init() {
    memset(injectFenceInfo, 0, sizeof(injectFenceInfo[0][0]) * MAX_GOEFENCE * 3);
    //LOGD("GEO_JNI: init fence info\n");
}

static void mtk_geo_set_project_config() {
    const char flp_prop_path[] = "/vendor/etc/geo.prop";
    char propbuf[512];
    FILE *fp = fopen(flp_prop_path, "rb");

    gConfigHalGeofence = 0;
    if(!fp) {
        LOGE("mtk_flp_set_project_config - open fd fail! %d (%s)", errno, strerror(errno));
        return;
    }
    while(fgets(propbuf, sizeof(propbuf), fp)) {
        if(strstr(propbuf, "lowpower.geofence=no")) {
            gConfigHalGeofence |= GEO_CONFIG_MASK_DISABLE_SMD;
        }
    }
    LOGD("mtk_flp_set_project_config %u", gConfigHalGeofence);
    fclose(fp);
    return;
}

void mtk_geo_inject_info_set(int fence_id, float dn, float de) {
    if (fence_id < MAX_GOEFENCE) {
        injectFenceInfo[fence_id][0] = 1; //valid
        injectFenceInfo[fence_id][1] = dn;
        injectFenceInfo[fence_id][2] = de;
        LOGD("GEO_JNI :set fence info, %d %f %f", fence_id, dn, de);
    }
}

void mtk_geo_inject_info_reset(int fence_id) {
    if (fence_id < MAX_GOEFENCE) {
        injectFenceInfo[fence_id][0] = 0; //valid
        injectFenceInfo[fence_id][1] = 0;
        injectFenceInfo[fence_id][2] = 0;
        LOGD("GEO_JNI: reset fence id %d\n", fence_id);
    }
}

void mtk_geo_inject_info_get(int fence_id, float* dn, float* de) {
    *dn = 0;
    *de = 0;
    if (fence_id < MAX_GOEFENCE) {
        if (injectFenceInfo[fence_id][0]) {
            *dn = injectFenceInfo[fence_id][1];
            *de = injectFenceInfo[fence_id][2];
            LOGD("GEO_JNI: get fence info %d %f %f\n", fence_id, *dn, *de);
        }
    }
}

static int check_buff_full_of_fence() {
    if(gTotalFence >= MAX_GOEFENCE) {
        return MTK_GFC_ERROR;
    } else {
        return MTK_GFC_SUCCESS;
    }
}

int check_buff_fence_exist(const int32_t fence_id) {
    int i;

    for(i = 0; i < MAX_GOEFENCE; i++) {
        if(geofence_property[i].geofence_id == fence_id ) {
            return i;
        }
    }
    return MTK_GFC_ERROR;
}

static int check_valid_transition(const int transition) {
    switch(transition) {
        case GPS_GEOFENCE_ENTERED:
        case GPS_GEOFENCE_EXITED:
        case GPS_GEOFENCE_ENTERED|GPS_GEOFENCE_EXITED:
        case GPS_GEOFENCE_ENTERED|GPS_GEOFENCE_EXITED|GPS_GEOFENCE_UNCERTAIN:
            return MTK_GFC_SUCCESS;
        default:
            return MTK_GFC_ERROR;
    }
}

static int get_buff_avalibale_item() {
    int i = 0;

    while(i < MAX_GOEFENCE && geofence_property[i].alive == 1) {
        i++;
    }

    if(i == MAX_GOEFENCE) {
        return MTK_GFC_ERROR;
    }
    return i;
}

static int check_fence_vadility(const MTK_GEOFENCE_PROPERTY_T fence) {
    int ret;
    int ret2;

    ret = check_buff_full_of_fence();
    if(ret < 0) {
        LOGE("too many fences\n");
        ret2 =  -1;
        return ret2;
    }
    ret = check_buff_fence_exist(fence.geofence_id);
    if(ret >= 0) {
        LOGE("fence has been added before\n");
        ret2 =  -2;
        return ret2;
    }
    ret = check_valid_transition(fence.monitor_transition);
    if(ret < 0) {
        LOGE("fence is invalid\n");
        ret2 =  -3;
        return ret2;
    }
    ret2 = 0;
    return ret2;
}

static int set_buff_add_fence(const MTK_GEOFENCE_PROPERTY_T fence) {
    int item;

    item = get_buff_avalibale_item();
    if(item < 0) {
        LOGE("out of range\n");
        return MTK_GFC_ERROR;
    }
    //LOGD("new fenece will be added into item %d\n", item);

    memcpy(&geofence_property[item], &fence, sizeof(MTK_GEOFENCE_PROPERTY_T));
    geofence_property[item].alive = 1;
    gTotalFence++;
    return MTK_GFC_SUCCESS;
}

#ifdef MTK_64_PLATFORM
void mtk_loc_rearrange(unsigned char *loc_in, GpsLocation *loc_out) {
    unsigned char ratio = 2; //32 to 64-bits
    unsigned char padding_diff = 4;
    unsigned int sizeof_loc_in = sizeof(GpsLocation) - sizeof(size_t)/ratio - padding_diff;

    memset(loc_out, 0, sizeof(GpsLocation));
    loc_out->size = sizeof(GpsLocation);
    memcpy(&loc_out->flags, loc_in + sizeof(size_t)/ratio, sizeof(uint16_t));
    memcpy(&loc_out->latitude, loc_in + sizeof(size_t), sizeof_loc_in - sizeof(size_t));
}
#endif

/*************************************************************/
/*  mnl to FLP HAL socket, -1 means failure                  */
/*************************************************************/
int create_mnl2gfchal_fd() {
    int fd = bind_udp_socket(MNL_TO_GEOFENCE);
    return fd;
}

int is_gfc_exist() {
    return isgfc_exist;
}


/*********************************************************/
/* GPS Geofence Interface implementation                 */
/*********************************************************/
void hal2_geofence_init() {
    int  ret;
    #if 0
    int offset = 0;
    char buff[HAL_GFC_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *geo_header =NULL;
    MTK_FLP_MSG_T geo_msg;
    #endif

    //Init client socket thread
    if(isgfc_exist) {
        LOGE("geofence init before");
        return;
    }
    ret = pthread_create(&geofence_hal_jni_thread_id, NULL, (threadptr)mnl2gfchal_jni_thread, NULL);
    if(ret < 0) {
        LOGE("Create mnl2gfchal_jni_thread error\n");
    }
    isgfc_exist = 1;
    #if 0
    geo_header = malloc(sizeof(MTK_FLP_MSG_T) + sizeof(MTK_FLP_MSG_T));
    if(geo_header == NULL) {
        LOGE("init fence malloc error");
        return;
    }

    geo_header->type = MTK_FLP_MSG_OFL_GEOFENCE_CMD;
    geo_header->length = sizeof(MTK_FLP_MSG_T) + sizeof(MTK_FLP_MSG_T);
    geo_msg.type = INIT_GEOFENCE;
    geo_msg.length = sizeof(MTK_FLP_MSG_T);
    memcpy( ((char*)geo_header)+sizeof(MTK_FLP_MSG_T),&geo_msg, sizeof(MTK_FLP_MSG_T));
    put_binary(buff, &offset, (const char*)&geo_header, geo_header->length);
    ret = gfc_send2mnl(buff, offset);
    free(geo_header);
    if(ret < 0) {
        LOGE("MTK_HAL2GFC send error return error");
        return;
    }
    #endif
    set_buff_init_fence();
    mtk_geo_inject_info_init();
    mtk_geo_set_project_config();
    return;
}

void hal2_geofence_add_geofences(const MTK_GEOFENCE_PROPERTY_T dbg_fence) {
    int ret, offset = 0;
    char buff[HAL_GFC_BUFF_SIZE] = {0};
    int32_t partial_size = 1;
    MTK_FLP_MSG_T *geo_header=NULL;
    MTK_FLP_MSG_T geo_msg;

    /* For geofence recover mechanism */
    ret = check_fence_vadility(dbg_fence);
    if (ret == 0) {
        set_buff_add_fence(dbg_fence);
    } else {
        return;
    }

    //construct add fence cmd
    geo_header = malloc(2*sizeof(MTK_FLP_MSG_T)+sizeof(int32_t) + sizeof(MTK_GEOFENCE_PROPERTY_T));
    if(geo_header == NULL) {
        LOGE("add fence malloc error");
        return;
    }

    geo_header->type = MTK_FLP_MSG_OFL_GEOFENCE_CMD;
    geo_header->length = sizeof(MTK_FLP_MSG_T) + sizeof(MTK_FLP_MSG_T) + sizeof(int32_t) + sizeof(MTK_GEOFENCE_PROPERTY_T);
    geo_msg.type = ADD_GEOFENCE_AREA;
    geo_msg.length =  sizeof(MTK_FLP_MSG_T) + sizeof(int32_t) + sizeof(MTK_GEOFENCE_PROPERTY_T);
    memcpy( ((char*)geo_header)+sizeof(MTK_FLP_MSG_T),&geo_msg, sizeof(MTK_FLP_MSG_T));
    memcpy( ((char*)geo_header)+2*sizeof(MTK_FLP_MSG_T),&partial_size, sizeof(int32_t));
    memcpy( ((char*)geo_header)+2*sizeof(MTK_FLP_MSG_T)+sizeof(int32_t),&dbg_fence, sizeof(MTK_GEOFENCE_PROPERTY_T));
    #if 1
    LOGD("HAL Add fence, id=%d, ll=%lf,%lf, last=%d, monitor=%d, notsec=%d, unksec=%d, ned=%f,%f, con=%u",
        dbg_fence.geofence_id, dbg_fence.latitude, dbg_fence.longitude,
        dbg_fence.last_transition, dbg_fence.monitor_transition,
        dbg_fence.notification_period, dbg_fence.unknown_timer,
        dbg_fence.coordinate_dn, dbg_fence.coordinate_de, dbg_fence.config);
    #endif
    put_binary(buff, &offset, (const char*)geo_header, geo_header->length);
    ret = gfc_send2mnl(buff, offset);
    free(geo_header);
    if(ret < 0) {
        LOGE("MTK_HAL2GFC send error return error");
        return;
    }
    return;
}

void hal2_geofence_pause_geofence(const int32_t geofence_id) {
    int ret, offset = 0;
    char buff[HAL_GFC_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *geo_header=NULL;
    MTK_FLP_MSG_T geo_msg;

    /* For geofence recover mechanism */
    ret = check_buff_fence_exist(geofence_id);
    if (ret != MTK_GFC_ERROR) {
        set_buff_pause_fence(ret);
    } else {
        return;
    }

    //LOGD("hal2_geofence_pause_geofence, id: %d fence",geofence_id);
    //construct pause fence cmd
    geo_header = malloc(2*sizeof(MTK_FLP_MSG_T) + sizeof(int32_t));
    if(geo_header == NULL) {
        LOGE("pause fence malloc error");
        return;
    }

    geo_header->type = MTK_FLP_MSG_OFL_GEOFENCE_CMD;
    geo_header->length = sizeof(MTK_FLP_MSG_T) + sizeof(MTK_FLP_MSG_T) + sizeof(int32_t);
    geo_msg.type = PAUSE_GEOFENCE;
    geo_msg.length = sizeof(MTK_FLP_MSG_T) + sizeof(int32_t);
    memcpy( ((char*)geo_header)+sizeof(MTK_FLP_MSG_T),&geo_msg, sizeof(MTK_FLP_MSG_T));
    memcpy( ((char*)geo_header)+2*sizeof(MTK_FLP_MSG_T),&geofence_id, sizeof(int32_t));
    put_binary(buff, &offset, (const char*)geo_header, geo_header->length);
    ret = gfc_send2mnl(buff, offset);
    free(geo_header);
    if(ret < 0) {
        LOGD("MTK_HAL2GFC send error return error");
        return;
    }
    return;
}


void hal2_geofence_resume_geofence(const int32_t geofence_id, const int monitor_transitions) {
    int ret, offset = 0;
    char buff[HAL_GFC_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *geo_header=NULL;
    MTK_FLP_MSG_T geo_msg;

    /* For geofence recover mechanism */
    ret = check_buff_fence_exist(geofence_id);
    if(ret != MTK_GFC_ERROR) {
        set_buff_resume_fence(ret, monitor_transitions);
    } else {
        return;
    }

    //LOGD("hal2_geofence_resume_geofence, id: %d fence",geofence_id);
    //construct resume fence cmd
    geo_header = malloc(2*sizeof(MTK_FLP_MSG_T) + sizeof(int32_t) + sizeof(int));
    if(geo_header == NULL) {
        LOGE("resume fence malloc error");
        return;
    }
    geo_header->type = MTK_FLP_MSG_OFL_GEOFENCE_CMD;
    geo_header->length = 2*sizeof(MTK_FLP_MSG_T) + sizeof(int32_t) + sizeof(int);
    geo_msg.type = RESUME_GEOFENCE;
    geo_msg.length =  sizeof(MTK_FLP_MSG_T) + sizeof(int32_t) + sizeof(int);
    memcpy( ((char*)geo_header)+sizeof(MTK_FLP_MSG_T),&geo_msg, sizeof(MTK_FLP_MSG_T));
    memcpy( ((char*)geo_header)+2*sizeof(MTK_FLP_MSG_T),&geofence_id, sizeof(int32_t));
    memcpy( ((char*)geo_header)+2*sizeof(MTK_FLP_MSG_T)+sizeof(int32_t),&monitor_transitions, sizeof(int));
    put_binary(buff, &offset, (const char*)geo_header, geo_header->length);
    ret = gfc_send2mnl(buff, offset);
    free(geo_header);
    if(ret < 0) {
        LOGE("MTK_HAL2GFC send error return error");
        return;
    }
    return;
}

void hal2_geofence_remove_geofences(const int32_t geofence_id) {
    int ret, offset = 0;
    char buff[HAL_GFC_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *geo_header=NULL;
    MTK_FLP_MSG_T geo_msg;

    /* For geofence recover mechanism */
    ret = check_buff_fence_exist(geofence_id);
    if(ret != MTK_GFC_ERROR) {
        mtk_geo_inject_info_reset(geofence_id);
        set_buff_remove_fence(ret);
    } else {
        return;
    }

    //LOGD("hal2_geofence_remove_geofences, fence id =%d",geofence_id);
    //construct remove fence cmd
    geo_header = malloc(2*sizeof(MTK_FLP_MSG_T) + sizeof(int32_t));
    if(geo_header == NULL) {
        LOGE("resume fence malloc error");
        return;
    }

    geo_header->type = MTK_FLP_MSG_OFL_GEOFENCE_CMD;
    geo_header->length = sizeof(MTK_FLP_MSG_T) + sizeof(MTK_FLP_MSG_T) + sizeof(int32_t);
    geo_msg.type = REMOVE_GEOFENCE;
    geo_msg.length = sizeof(MTK_FLP_MSG_T) + sizeof(int32_t);
    memcpy( ((char*)geo_header)+sizeof(MTK_FLP_MSG_T),&geo_msg, sizeof(MTK_FLP_MSG_T));
    memcpy( ((char*)geo_header)+2*sizeof(MTK_FLP_MSG_T),&geofence_id, sizeof(int32_t));
    put_binary(buff, &offset, (const char*)geo_header, geo_header->length);
    ret = gfc_send2mnl(buff, offset);
    free(geo_header);
    if(ret < 0) {
        LOGE("MTK_HAL2GFC send error return error");
        return;
    }
    return;
}

void hal2_geofence_recover_geofences() {
    int ret, offset = 0;
    char buff[HAL_GFC_BUFF_SIZE] = {0};
    int i;
    int partial_size = 1;
    MTK_FLP_MSG_T* geo_header = NULL;
    MTK_FLP_MSG_T geo_msg;

    LOGE("FLP HAL RECOVER: recover total fence = %d", gTotalFence);
    for(i = 0; i < MAX_GOEFENCE; i++) {
        if(geofence_property[i].alive == 1) {
            geo_header = malloc( sizeof(MTK_FLP_MSG_T)+ sizeof(MTK_FLP_MSG_T) + sizeof(int32_t) + sizeof(MTK_GEOFENCE_PROPERTY_T));
            if (geo_header == NULL){
                LOGE("MTK_HAL2GFC malloc geofence header fail");
                return;
            }
            geo_header->type = MTK_FLP_MSG_OFL_GEOFENCE_CMD;
            geo_header->length =  sizeof(MTK_FLP_MSG_T)+ sizeof(MTK_FLP_MSG_T)+ sizeof(int32_t) + sizeof(MTK_GEOFENCE_PROPERTY_T);
            geo_msg.type = RECOVER_GEOFENCE;
            geo_msg.length =  sizeof(MTK_FLP_MSG_T)+ sizeof(int32_t) + sizeof(MTK_GEOFENCE_PROPERTY_T);
            memcpy(((char*)geo_header)+sizeof(MTK_FLP_MSG_T), &geo_msg, sizeof(MTK_FLP_MSG_T));
            memcpy((((char*)geo_header)+sizeof(MTK_FLP_MSG_T)+sizeof(MTK_FLP_MSG_T)), &partial_size, sizeof(int32_t));
            memcpy((((char*)geo_header)+sizeof(MTK_FLP_MSG_T)+sizeof(MTK_FLP_MSG_T)+sizeof(int32_t)),&geofence_property[i],sizeof(MTK_GEOFENCE_PROPERTY_T));
            LOGE("FLP HAL RECOVER: id=%" PRId32 ",type=%d,%d,%d,%lf,%d",geofence_property[i].geofence_id,geofence_property[i].last_transition,geofence_property[i].monitor_transition,
            geofence_property[i].notification_period,geofence_property[i].latitude,geofence_property[i].unknown_timer);
            offset = 0;
            put_binary(buff, &offset, (const char*)geo_header, geo_header->length);
            ret = gfc_send2mnl(buff, offset);
            free(geo_header);
            if(ret < 0) {
                LOGE("MTK_HAL2GFC send error return error");
                return;
            }
        }
    }
    return;
}

int mnl2gfchal_hdlr (char *buff) {
    char data[1024] = {0};
    int offset = 0;
    int ret = MTK_GFC_ERROR, len;
    unsigned int cmd, msg_len;
    float geo_inject[3] = {0};
    MTK_FLP_MSG_T *prmsg = NULL;

    if(buff == NULL) {
        LOGE("mnl2gfchal_hdlr, recv prmsg is null pointer\r\n");
        return MTK_GFC_ERROR;
    }
    len = get_binary(buff, &offset, data, HAL_GFC_BUFF_SIZE, sizeof(data));
    if((len > 0) && (len<=1024)) {
        prmsg = (MTK_FLP_MSG_T *)&data[0];
    } else {
        LOGE("len err:%d",len);
        return ret;
    }
    cmd = prmsg->type;
    msg_len = prmsg->length;

    LOGD("msg_len, recv prmsg, type %d, len %d\r\n", cmd, msg_len);
    switch (cmd) {
        case MTK_FLP_MSG_SYS_FLPD_RESET_NTF:
            hal2_geofence_recover_geofences();
            break;
        case MTK_FLP_MSG_OFL_GEOFENCE_CALLBACK_NTF:
            mtk_geofence_transition_callbacks_proc(prmsg);
            break;
        case MTK_FLP_MSG_HAL_GEOFENCE_CALLBACK_NTF:
            mtk_geofence_callbacks_proc(prmsg);
            break;
        case GEOFENCE_INJECT_LOC_ENUM:
            if(msg_len< 3*sizeof(float)) {
                LOGE("GEOFENCE_INJECT_LOC_ENUM incrorrect size %d",msg_len);
                return MTK_GFC_ERROR;
            }
            memcpy(geo_inject, (char *)prmsg + sizeof(MTK_FLP_MSG_T), 3*sizeof(float));
            mtk_geo_inject_info_set((int)geo_inject[0],geo_inject[1],geo_inject[2]);
            break;
        default:
            LOGE("invalid geofence cmd:0x%02x",cmd);
            break;
    }
    return MTK_GFC_SUCCESS;
}


/*************************************************************/
/*  mnl to GPS/Geofence HAL main thread                      */
/*************************************************************/
void mnl2gfchal_jni_thread(void) {

    int ret = MTK_GFC_SUCCESS;
    int offset = 0;
    int read_len;
    char buff[HAL_GFC_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T gfc_header;

    LOGD("mtk_gfc_hal_jni_thread, Create\n");

    g_gfc_server_socket_fd = create_mnl2gfchal_fd();

    ///TODO: add system timer function here
    //mtk_flp_sys_timer_create(FLP_TIMER_ID_CHECKCNN);
    //Send SYS_INIT to HAL
    gfc_header.type = MTK_FLP_MSG_HAL_INIT_CMD;
    gfc_header.length = 0;
    put_binary(buff, &offset, (const char*)&gfc_header, sizeof(MTK_FLP_MSG_T));
    ret = gfc_send2mnl(buff, offset);
    //LOGD("gfc:MTK_FLP_MSG_HAL_INIT_CMD");
    if(ret < 0) {
        LOGE("MTK_HAL2GFC send error return error");
    }

    if (g_gfc_server_socket_fd >= 0) {
        while(!g_ThreadExitGfcJniSocket) {
            // - recv msg from socket interface
            read_len = safe_recvfrom(g_gfc_server_socket_fd, buff, sizeof(buff));
            if ((read_len <= 0) || (read_len>HAL_GFC_BUFF_SIZE)) {
                LOGE("mnl2gfc safe_recvfrom failed read_len=%d", read_len);
            }

            if (!g_ThreadExitGfcJniSocket) {
                // Process received message
                mnl2gfchal_hdlr(buff);
            } else {
                LOGE("mtk_flp_hal_jni_thread, read msg fail,exit socket thread\n");
                //read msg fail...
                g_ThreadExitGfcJniSocket = 1;
            }
        }
    }

    //close socket
    LOGD("Closing server_fd,%d\r\n",g_gfc_server_socket_fd);
    if(g_gfc_server_socket_fd >= 0) {
        close(g_gfc_server_socket_fd);
    }

    LOGD("mnl2gfchal_jni_thread, exit\n");

    g_ThreadExitGfcJniSocket = 1;
    pthread_exit(NULL);

    return;
}


