#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <pthread.h>



#include <audio_log.h>
#include <audio_lock.h>
#include <audio_assert.h>
#include <audio_task.h>
#include <audio_scp_service.h>



#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     LOG
 * =============================================================================
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "audio_scp_service"


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */


/*==============================================================================
 *                     ioctl
 *============================================================================*/

#define AUDIO_SCP_DEVICE_PATH "/dev/audio_scp"
#define AUDIO_SCP_IOC_MAGIC 's'

#define AUDIO_SCP_IOCTL_QUERY_STATUS \
    _IOR(AUDIO_SCP_IOC_MAGIC, 0x0, unsigned int)
#define AUDIO_SCP_IOCTL_RESET_CBK \
    _IOR(AUDIO_SCP_IOC_MAGIC, 0x1, unsigned int)


/*
 * =============================================================================
 *                     struct def
 * =============================================================================
 */

struct g_scp_feature_t {
    /* constructor/destructor */
    scp_start_cbk_t start_cbk;
    scp_stop_cbk_t stop_cbk;
    void *arg;
};

/*
 * =============================================================================
 *                     global var
 * =============================================================================
 */

static struct alock_t *g_scp_drv_lock;

static int g_scp_drv;

static pthread_t g_read_scp_thread_hdl;
bool g_read_scp_thread_enable;

static struct g_scp_feature_t g_scp_feature;

/*
 * =============================================================================
 *                     function declaration
 * =============================================================================
 */

static void *read_audio_scp_calbak_thread(void *arg);

/*==============================================================================
 *                     public functions - implementation
 *============================================================================*/

int is_scp_ready(void) {
    int ret = 0;
    int ready_flag = 0;

    LOCK_ALOCK(g_scp_drv_lock);
    if (g_scp_drv < 0) {
        AUD_LOG_E("%s(), driver %s file node %d!!", __FUNCTION__,
                  AUDIO_SCP_DEVICE_PATH, g_scp_drv);
        UNLOCK_ALOCK(g_scp_drv_lock);
        return -1;
    }
    ret = ioctl(g_scp_drv, AUDIO_SCP_IOCTL_QUERY_STATUS, &ready_flag);
    if (ret != 0) {
        AUD_LOG_E("%s() ioctl fail! ret = %d, errno: %d, ready_flag: %d",
                  __FUNCTION__, ret, errno, ready_flag);
        UNLOCK_ALOCK(g_scp_drv_lock);
        return -errno;
    }
    if (ready_flag != 1) {
        AUD_LOG_W("%s() scp not ready! ret: %d, errno: %d, ready_flag: %d",
                  __FUNCTION__, ret, errno, ready_flag);
    }
    UNLOCK_ALOCK(g_scp_drv_lock);
    return ready_flag;
}

void audio_scp_cbk_register(
    scp_start_cbk_t start_cbk,
    scp_stop_cbk_t stop_cbk,
    void *arg) {
    AUD_LOG_D("%s(), start_cbk %p, stop_cbk %p, arg %p", __FUNCTION__,
              start_cbk, stop_cbk, arg);
    int i = 0;

    if (start_cbk == NULL || stop_cbk == NULL) {
        return;
    }

    LOCK_ALOCK(g_scp_drv_lock);
    if (g_scp_feature.start_cbk != NULL ||
        g_scp_feature.stop_cbk != NULL) {
        AUD_LOG_W("%s(), cbk exist!!", __FUNCTION__);
        goto EXIT;
    }
    g_scp_feature.start_cbk = start_cbk;
    g_scp_feature.stop_cbk = stop_cbk;
    g_scp_feature.arg = arg;


    if (g_scp_feature.start_cbk != NULL && g_scp_feature.stop_cbk != NULL) {
        AUD_LOG_D("%s(), start_cbk = %p",
                  __FUNCTION__,
                  g_scp_feature.start_cbk);
    }

EXIT:
    UNLOCK_ALOCK(g_scp_drv_lock);
}

void audio_scp_cbk_deregister(void) {
    int ret = 0;

    AUD_LOG_D("%s()", __FUNCTION__);

    LOCK_ALOCK(g_scp_drv_lock);

    if (g_scp_feature.start_cbk == NULL ||
        g_scp_feature.stop_cbk == NULL) {
        AUD_LOG_W("%s(), cbk null!!", __FUNCTION__);
        goto EXIT;
    }
    g_scp_feature.start_cbk = NULL;
    g_scp_feature.stop_cbk = NULL;
    g_scp_feature.arg = NULL;

EXIT:
    UNLOCK_ALOCK(g_scp_drv_lock);
}

void audio_scp_service_init(void) {
    int ret = 0;
    int i = 0;


    NEW_ALOCK(g_scp_drv_lock);

    /* init var */
    g_scp_feature.start_cbk = NULL;
    g_scp_feature.stop_cbk = NULL;
    g_scp_feature.arg = NULL;

    /* open driver */
    LOCK_ALOCK(g_scp_drv_lock);
    g_scp_drv = open(AUDIO_SCP_DEVICE_PATH, O_RDONLY);
    if (g_scp_drv < 0) {
        AUD_LOG_E("%s(), fail to open %s, errno: %d", __FUNCTION__,
                  AUDIO_SCP_DEVICE_PATH, errno);
        AUD_WARNING("open audio scp device fail!!");
    }
    UNLOCK_ALOCK(g_scp_drv_lock);

    /* read scp thread */
    g_read_scp_thread_hdl = 0;
    g_read_scp_thread_enable = true;

    ret = pthread_create(&g_read_scp_thread_hdl, NULL,
                         read_audio_scp_calbak_thread,
                         NULL);
    AUD_ASSERT(ret == 0);
}

void audio_scp_service_deinit(void) {

    LOCK_ALOCK(g_scp_drv_lock);
    /* read audio scp thread */
    g_read_scp_thread_enable = false;

    /* close driver */
    if (g_scp_drv >= 0) {
        close(g_scp_drv);
        g_scp_drv = -1;
    }

    UNLOCK_ALOCK(g_scp_drv_lock);
    FREE_ALOCK(g_scp_drv_lock);
}

void *read_audio_scp_calbak_thread(void *arg  __unused) {

    int msg_size = 0;
    int data_size = 0;
    int length_read = 0;
    int ret = 0;
    int i = 0;

    pthread_detach(pthread_self());

    while (g_read_scp_thread_enable) {
        /* ioctrl & lock here util callback */
        ret = ioctl(g_scp_drv, AUDIO_SCP_IOCTL_RESET_CBK, NULL);
        if (ret < 0) {
            AUD_LOG_W("%s(), read scp status fail!! ret: %d errno: %d",
                      __FUNCTION__, ret, errno);
            usleep(2000 * 1000); //2s
            continue;
        }

        if (ret == SCP_EVENT_READY) {
            /* Call StreamManager API */
            if (g_scp_feature.start_cbk != NULL) {
                AUD_LOG_D("%s, event ready", __FUNCTION__);
                g_scp_feature.start_cbk(g_scp_feature.arg);
            } else {
                AUD_LOG_E("%s, event ready, start_cbk is NULL", __FUNCTION__);
            }
        } else {
            /* Call StreamManager API */
            if (g_scp_feature.stop_cbk != NULL) {
                AUD_LOG_D("%s, event stop", __FUNCTION__);
                g_scp_feature.stop_cbk(g_scp_feature.arg);
            } else {
                AUD_LOG_E("%s, event ready, stop_cbk is NULL", __FUNCTION__);
            }
        }
    }

    pthread_exit(NULL);
    return NULL;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
