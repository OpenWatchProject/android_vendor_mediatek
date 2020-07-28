#include <audio_messenger_ipi.h>

#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>

#include <audio_log.h>
#include <audio_lock.h>
#include <audio_assert.h>


#include <audio_task.h>
#include <audio_controller_msg_id.h>

#ifdef MTK_AUDIODSP_SUPPORT
#include "AudioAurisysPcmDump.h"
#include <audio_dsp_service.h>
#endif

#if defined(MTK_AUDIO_SCP_RECOVERY_SUPPORT)
#include <audio_scp_service.h>
#endif

#include <audio_dsp_controller.h>


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
#define LOG_TAG "audio_messenger_ipi"



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define MAX_DSP_DMA_WRITE_SIZE (0x10000)



/*==============================================================================
 *                     ioctl
 *============================================================================*/

#define AUDIO_IPI_DEVICE_PATH "/dev/audio_ipi"
#define AUDIO_IPI_IOC_MAGIC 'i'

#define AUDIO_IPI_IOCTL_SEND_MSG_ONLY _IOW(AUDIO_IPI_IOC_MAGIC, 0, unsigned int)
#define AUDIO_IPI_IOCTL_SEND_PAYLOAD  _IOW(AUDIO_IPI_IOC_MAGIC, 1, unsigned int)
#define AUDIO_IPI_IOCTL_SEND_DRAM     _IOW(AUDIO_IPI_IOC_MAGIC, 2, unsigned int)

#define AUDIO_IPI_IOCTL_LOAD_SCENE   _IOW(AUDIO_IPI_IOC_MAGIC, 10, unsigned int)

#define AUDIO_IPI_IOCTL_INIT_DSP     _IOW(AUDIO_IPI_IOC_MAGIC, 20, unsigned int)
#define AUDIO_IPI_IOCTL_REG_DMA      _IOW(AUDIO_IPI_IOC_MAGIC, 21, unsigned int)



/*
 * =============================================================================
 *                     struct def
 * =============================================================================
 */

struct audio_ipi_reg_dma_t {
    uint32_t magic_header;
    uint8_t task;
    uint8_t reg_flag; /* 1: register, 0: unregister */
    uint16_t __reserved;

    uint32_t a2d_size;
    uint32_t d2a_size;
    uint32_t magic_footer;
};

struct audio_ipi_dma_client_t {
    audio_ipi_dma_cbk_t cbk;
    void *arg;
};

#ifdef MTK_AUDIODSP_SUPPORT
struct audio_ipi_reg_feature_t {
    uint16_t reg_flag;
    uint16_t feature_id;
};
#endif


struct audio_task_info_t {
    uint32_t dsp_id;            /* dsp_id_t */
    uint8_t  is_dsp_support;    /* dsp_id supported or not */
    uint8_t  is_adsp;           /* adsp(HiFi) or not */
    uint8_t  is_scp;            /* scp(CM4) or not */
    uint8_t  task_ctrl;         /* task controller scene # */
};



/*
 * =============================================================================
 *                     global var
 * =============================================================================
 */

static struct alock_t *g_drv_lock;
static int g_ipi_drv;

#if defined(MTK_AUDIO_IPI_DMA_SUPPORT)
static struct alock_t *g_dma_lock;

static pthread_t g_read_ipi_thread_hdl;
static bool      g_read_ipi_thread_enable;
static uint8_t  *g_read_ipi_buf;

static struct audio_ipi_dma_client_t g_ipi_dma_client[TASK_SCENE_SIZE];
#endif

static struct audio_task_info_t g_audio_task_info[TASK_SCENE_SIZE];


/*
 * =============================================================================
 *                     function declaration
 * =============================================================================
 */

#if defined(MTK_AUDIO_IPI_DMA_SUPPORT)
static void *read_ipi_thread(void *arg);

static void process_dma_msg(
    struct ipi_msg_t *msg,
    void *buf,
    uint32_t size,
    void *arg);

static void audio_ipi_dma_task_controller_reg(bool isRegister);
#endif



/*
 * =============================================================================
 *                     init
 * =============================================================================
 */

void audio_messenger_ipi_init(void)
{
    int ret = 0;
    int i = 0;

    if (g_drv_lock != NULL) {
        AUD_LOG_W("%s(), double init, return", __FUNCTION__);
        return;
    }

    NEW_ALOCK(g_drv_lock);

    /* open driver */
    LOCK_ALOCK(g_drv_lock);
    g_ipi_drv = open(AUDIO_IPI_DEVICE_PATH, O_RDONLY);
    if (g_ipi_drv < 0) {
        AUD_LOG_E("%s(), fail to open %s, errno: %d", __FUNCTION__,
                  AUDIO_IPI_DEVICE_PATH, errno);
        AUD_WARNING("open audio ipi driver fail!!");
        UNLOCK_ALOCK(g_drv_lock);
        return;
    }

#if defined(MTK_AUDIODSP_SUPPORT) || defined(MTK_AUDIO_SCP_RECOVERY_SUPPORT)
    /* init dsp */
    ret = ioctl(g_ipi_drv, AUDIO_IPI_IOCTL_INIT_DSP, g_audio_task_info);
    if (ret != 0) {
        AUD_LOG_E("%s(), ioctl fail! ret = %d, errno: %d", __FUNCTION__, ret, errno);
        UNLOCK_ALOCK(g_drv_lock);
        return;
    }
#if defined(MTK_AUDIODSP_SUPPORT)
    audio_dsp_service_init();
#endif

#if defined(MTK_AUDIO_SCP_RECOVERY_SUPPORT)
    audio_scp_service_init();
#endif

#endif /* end of MTK_AUDIODSP_SUPPORT || MTK_AUDIO_SCP_SUPPORT */

#ifdef MTK_AUDIO_DSP_RECOVERY_SUPPORT
    audio_dsp_controller_init();
#endif

    UNLOCK_ALOCK(g_drv_lock);


#if defined(MTK_AUDIO_IPI_DMA_SUPPORT)
    NEW_ALOCK(g_dma_lock);

    LOCK_ALOCK(g_dma_lock);
    for (i = 0; i < TASK_SCENE_SIZE; i++) {
        g_ipi_dma_client[i].cbk = NULL;
        g_ipi_dma_client[i].arg = NULL;
    }
    UNLOCK_ALOCK(g_dma_lock);

    audio_ipi_dma_task_controller_reg(true);

    /* read ipi thread */
    g_read_ipi_buf = (uint8_t *)malloc(MAX_DSP_DMA_WRITE_SIZE);
    if (g_read_ipi_buf == NULL) {
        AUD_LOG_E("%s(), g_read_ipi_buf NULL", __FUNCTION__);
        return;
    }
    g_read_ipi_thread_enable = true;
    g_read_ipi_thread_hdl = 0;
    ret = pthread_create(&g_read_ipi_thread_hdl, NULL,
                         read_ipi_thread,
                         NULL);
    AUD_ASSERT(ret == 0);
#endif
}


void audio_messenger_ipi_deinit(void)
{
#if defined(MTK_AUDIO_IPI_DMA_SUPPORT)
    /* dma */
    audio_ipi_dma_task_controller_reg(false);

    /* read ipi thread */
    g_read_ipi_thread_enable = false;
    pthread_join(g_read_ipi_thread_hdl, NULL);

    if (g_read_ipi_buf != NULL) {
        free(g_read_ipi_buf);
        g_read_ipi_buf = NULL;
    }

    FREE_ALOCK(g_dma_lock);
#endif


    /* close driver */
    LOCK_ALOCK(g_drv_lock);

    if (g_ipi_drv >= 0) {
        close(g_ipi_drv);
        g_ipi_drv = -1;
    }

    UNLOCK_ALOCK(g_drv_lock);

    FREE_ALOCK(g_drv_lock);

#ifdef MTK_AUDIODSP_SUPPORT
    audio_dsp_service_deinit();
#endif

#ifdef MTK_AUDIO_SCP_RECOVERY_SUPPORT
    audio_scp_service_deinit();
#endif

#ifdef MTK_AUDIO_DSP_RECOVERY_SUPPORT
    audio_dsp_controller_deinit();
#endif
}



/*
 * =============================================================================
 *                     ipi msg
 * =============================================================================
 */

static uint32_t get_message_buf_size(const struct ipi_msg_t *ipi_msg)
{
    if (ipi_msg->data_type == AUDIO_IPI_MSG_ONLY) {
        return IPI_MSG_HEADER_SIZE;
    } else if (ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
        return (IPI_MSG_HEADER_SIZE + ipi_msg->payload_size);
    } else if (ipi_msg->data_type == AUDIO_IPI_DMA) {
        return (IPI_MSG_HEADER_SIZE + IPI_MSG_DMA_INFO_SIZE);
    } else {
        return 0;
    }
}


static int check_msg_format(const struct ipi_msg_t *p_ipi_msg, uint32_t len)
{
    if (p_ipi_msg->magic != IPI_MSG_MAGIC_NUMBER) {
        AUD_LOG_W("%s(), magic 0x%x error!!",
                  __func__, p_ipi_msg->magic);
        return -1;
    }

    if (p_ipi_msg->task_scene >= TASK_SCENE_SIZE) {
        AUD_LOG_W("%s(), task_scene %d error!!",
                  __func__, p_ipi_msg->task_scene);
        return -1;
    }

    if (p_ipi_msg->source_layer >= AUDIO_IPI_LAYER_FROM_SIZE) {
        AUD_LOG_W("%s(), source_layer %d error!!",
                  __func__, p_ipi_msg->source_layer);
        return -1;
    }

    if (p_ipi_msg->target_layer >= AUDIO_IPI_LAYER_TO_SIZE) {
        AUD_LOG_W("%s(), target_layer %d error!!",
                  __func__, p_ipi_msg->target_layer);
        return -1;
    }

    if (p_ipi_msg->data_type >= AUDIO_IPI_TYPE_SIZE) {
        AUD_LOG_W("%s(), data_type %d error!!",
                  __func__, p_ipi_msg->data_type);
        return -1;
    }

    if (p_ipi_msg->ack_type > AUDIO_IPI_MSG_DIRECT_SEND &&
        p_ipi_msg->ack_type != AUDIO_IPI_MSG_CANCELED) {
        AUD_LOG_W("%s(), ack_type %d error!!",
                  __func__, p_ipi_msg->ack_type);
        return -1;
    }

    if (get_message_buf_size(p_ipi_msg) != len) {
        AUD_LOG_W("%s(), len 0x%x error!!", __func__, len);
        return -1;
    }

    if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
        if (p_ipi_msg->payload_size == 0 ||
            p_ipi_msg->payload_size > MAX_IPI_MSG_PAYLOAD_SIZE) {
            AUD_LOG_W("%s(), payload_size %u error!!",
                      __func__, p_ipi_msg->payload_size);
            return -1;
        }
    }

    if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
        if (p_ipi_msg->dma_info.hal_buf.addr == NULL) {
            AUD_LOG_W("%s(), dma addr null!!", __func__);
            return -1;
        }
        if (p_ipi_msg->dma_info.hal_buf.data_size == 0) {
            AUD_LOG_W("%s(), dma data_size %u error!!",
                      __func__, p_ipi_msg->dma_info.hal_buf.data_size);
            return -1;
        }
    }

    return 0;
}


int audio_send_ipi_msg(
    struct ipi_msg_t *p_ipi_msg,
    uint8_t task_scene, /* task_scene_t */
    uint8_t target_layer, /* audio_ipi_msg_target_layer_t */
    uint8_t data_type, /* audio_ipi_msg_data_t */
    uint8_t ack_type, /* audio_ipi_msg_ack_t */
    uint16_t msg_id,
    uint32_t param1, /* data_size for payload & dma */
    uint32_t param2,
    void    *data_buffer)   /* buffer for payload & dma */
{
    struct ipi_msg_dma_info_t *dma_info = NULL;
    uint32_t ipi_msg_len = 0;

    int ret = 0;


    memset(p_ipi_msg, 0, MAX_IPI_MSG_BUF_SIZE);

    p_ipi_msg->magic        = IPI_MSG_MAGIC_NUMBER;
    p_ipi_msg->task_scene   = task_scene;
    p_ipi_msg->source_layer = AUDIO_IPI_LAYER_FROM_HAL;
    p_ipi_msg->target_layer = target_layer;
    p_ipi_msg->data_type    = data_type;
    p_ipi_msg->ack_type     = ack_type;
    p_ipi_msg->msg_id       = msg_id;
    p_ipi_msg->param1       = param1;
    p_ipi_msg->param2       = param2;

    if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
        if (data_buffer == NULL) {
            AUD_LOG_W("payload data_buffer NULL, return");
            return -1;
        }
        if (p_ipi_msg->payload_size > MAX_IPI_MSG_PAYLOAD_SIZE) {
            AUD_LOG_W("payload_size %u error!!",
                      p_ipi_msg->payload_size);
            return -1;
        }
        memcpy(p_ipi_msg->payload, data_buffer, p_ipi_msg->param1);
    } else if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
        if (data_buffer == NULL) {
            AUD_LOG_W("dma data_buffer NULL, return");
            return -1;
        }
        p_ipi_msg->dma_addr = (char *)data_buffer;

        if (param1 > 0) {
            dma_info = &p_ipi_msg->dma_info;
            dma_info->data_size = 0;

            dma_info->hal_buf.data_size = param1;
            dma_info->hal_buf.memory_size = param2; /* not 0: write back */
            dma_info->hal_buf.addr = data_buffer;
        }
    }

#ifdef MTK_AUDIO_DSP_RECOVERY_SUPPORT
    if (is_audio_dsp_ready(task_scene) == false) {
        DUMP_IPI_MSG("dsp not ready", p_ipi_msg);
        return 0;
    }
#endif

    DUMP_IPI_MSG("hal send", p_ipi_msg);

    ipi_msg_len = get_message_buf_size(p_ipi_msg);
    if (check_msg_format(p_ipi_msg, ipi_msg_len) != 0) {
        AUD_LOG_W("%s(), drop msg due to ipi fmt err", __FUNCTION__);
        return -1;
    }


    LOCK_ALOCK(g_drv_lock);

    if (g_ipi_drv < 0) {
        AUD_LOG_W("%s(), g_ipi_drv = %d, return", __FUNCTION__, g_ipi_drv);
        UNLOCK_ALOCK(g_drv_lock);
        return -1;
    }

    switch (p_ipi_msg->data_type) {
    case AUDIO_IPI_MSG_ONLY:
        ret = ioctl(g_ipi_drv, AUDIO_IPI_IOCTL_SEND_MSG_ONLY, p_ipi_msg);
        break;
    case AUDIO_IPI_PAYLOAD:
        ret = ioctl(g_ipi_drv, AUDIO_IPI_IOCTL_SEND_PAYLOAD, p_ipi_msg);
        break;
    case AUDIO_IPI_DMA:
        ret = ioctl(g_ipi_drv, AUDIO_IPI_IOCTL_SEND_DRAM, p_ipi_msg);
        break;
    default:
        AUD_LOG_W("%s() type %d not support!!",
                  __FUNCTION__, p_ipi_msg->data_type);
    }
    if (ret != 0) {
        AUD_LOG_E("%s() ioctl fail! ret = %d, errno: %d", __FUNCTION__, ret, errno);
    }

    UNLOCK_ALOCK(g_drv_lock);

    return ret;
}



/*
 * =============================================================================
 *                     Audio IPI DMA
 * =============================================================================
 */

#if defined(MTK_AUDIO_IPI_DMA_SUPPORT)
static void audio_ipi_dma_task_controller_reg(bool isRegister) {

    if (isRegister) {
#if defined(MTK_AUDIO_CM4_DMA_SUPPORT)
        audio_ipi_dma_cbk_register(
            TASK_SCENE_AUDIO_CONTROLLER_CM4,
            0x20000,
            0x48000,
            process_dma_msg,
            NULL);
#endif
#if defined(MTK_AUDIODSP_SUPPORT)
        audio_ipi_dma_cbk_register(
            TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A,
            0x20000,
            0x48000,
            process_dma_msg,
            NULL);
        audio_ipi_dma_cbk_register(
            TASK_SCENE_AUDIO_CONTROLLER_HIFI3_B,
            0x20000,
            0x48000,
            process_dma_msg,
            NULL);
#endif
    } else {
#if defined(MTK_AUDIO_CM4_DMA_SUPPORT)
        audio_ipi_dma_cbk_deregister(TASK_SCENE_AUDIO_CONTROLLER_CM4);
#endif
#if defined(MTK_AUDIODSP_SUPPORT)
        audio_ipi_dma_cbk_deregister(TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A);
        audio_ipi_dma_cbk_deregister(TASK_SCENE_AUDIO_CONTROLLER_HIFI3_B);
#endif
    }
}


void audio_ipi_dma_cbk_register(
    const uint8_t task_scene,
    const uint32_t a2dSize,
    const uint32_t d2aSize,
    audio_ipi_dma_cbk_t cbk,
    void *arg)
{
    struct audio_ipi_reg_dma_t dma_reg;
    int ret = 0;


    AUD_LOG_D("%s(), task %d, sz %u/%u, cbk %p, arg %p", __FUNCTION__,
              task_scene, a2dSize, d2aSize, cbk, arg);

    if (task_scene >= TASK_SCENE_SIZE) {
        return;
    }
    if (a2dSize == 0 && d2aSize == 0) {
        return;
    }
    if (cbk == NULL) {
        return;
    }

    LOCK_ALOCK(g_dma_lock);

    if (g_ipi_dma_client[task_scene].cbk != NULL) {
        AUD_LOG_W("%s(), task %d cbk exist!!", __FUNCTION__, task_scene);
        UNLOCK_ALOCK(g_dma_lock);
        return;
    }
    g_ipi_dma_client[task_scene].cbk = cbk;
    g_ipi_dma_client[task_scene].arg = arg;

    UNLOCK_ALOCK(g_dma_lock);


    /* alloc region */
    dma_reg.task = task_scene;
    dma_reg.reg_flag = 1;
    dma_reg.a2d_size = a2dSize;
    dma_reg.d2a_size = d2aSize;
    dma_reg.magic_header = rand();
    dma_reg.magic_footer = 0xFFFFFFFF - dma_reg.magic_header;


    LOCK_ALOCK(g_drv_lock);
    if (g_ipi_drv < 0) {
        AUD_LOG_E("%s(), driver %s file node %d!!", __FUNCTION__,
                  AUDIO_IPI_DEVICE_PATH, g_ipi_drv);
        UNLOCK_ALOCK(g_drv_lock);
        return;
    }

    ret = ioctl(g_ipi_drv, AUDIO_IPI_IOCTL_REG_DMA, &dma_reg);
    if (ret != 0) {
        AUD_LOG_E("%s() ioctl fail! ret = %d, errno: %d", __FUNCTION__, ret, errno);
    }
    UNLOCK_ALOCK(g_drv_lock);
}


void audio_ipi_dma_cbk_deregister(const uint8_t task_scene)
{
    struct audio_ipi_reg_dma_t dma_reg;
    int ret = 0;



    AUD_LOG_D("%s(), task %d", __FUNCTION__, task_scene);

    if (task_scene >= TASK_SCENE_SIZE) {
        return;
    }

    LOCK_ALOCK(g_dma_lock);

    if (g_ipi_dma_client[task_scene].cbk == NULL) {
        AUD_LOG_W("%s(), task %d cbk null!!", __FUNCTION__, task_scene);
        UNLOCK_ALOCK(g_dma_lock);
        return;
    }
    g_ipi_dma_client[task_scene].cbk = NULL;
    g_ipi_dma_client[task_scene].arg = NULL;

    UNLOCK_ALOCK(g_dma_lock);


    /* free region */
    dma_reg.task = task_scene;
    dma_reg.reg_flag = 0;
    dma_reg.a2d_size = 0;
    dma_reg.d2a_size = 0;
    dma_reg.magic_header = rand();
    dma_reg.magic_footer = 0xFFFFFFFF - dma_reg.magic_header;

    LOCK_ALOCK(g_drv_lock);
    if (g_ipi_drv < 0) {
        AUD_LOG_E("%s(), driver %s file node %d!!", __FUNCTION__,
                  AUDIO_IPI_DEVICE_PATH, g_ipi_drv);
        UNLOCK_ALOCK(g_drv_lock);
        return;
    }

    ret = ioctl(g_ipi_drv, AUDIO_IPI_IOCTL_REG_DMA, &dma_reg);
    if (ret != 0) {
        AUD_LOG_E("%s() ioctl fail! ret = %d, errno: %d", __FUNCTION__, ret, errno);
    }

    UNLOCK_ALOCK(g_drv_lock);

}


void *read_ipi_thread(void *arg  __unused)
{
    struct ipi_msg_t *msg = NULL;
    uint8_t *data_buf = NULL;

    int msg_size = 0;
    int data_size = 0;
    int length_read = 0;

    int ret = 0;
    int i = 0;

    msg_size = sizeof(struct ipi_msg_t);

    while (g_read_ipi_thread_enable) {
        /* read */
        length_read = read(g_ipi_drv, g_read_ipi_buf, MAX_DSP_DMA_WRITE_SIZE);
        if (length_read <= msg_size || length_read > MAX_DSP_DMA_WRITE_SIZE) {
            AUD_LOG_W("%s(), read dsp fail!! length_read: %d, errno: %d",
                      __FUNCTION__, (int32_t)length_read, errno);
            usleep(500 * 1000);
            continue;
        }

        /* parse msg */
        msg = (struct ipi_msg_t *)g_read_ipi_buf;
        if (msg->magic != IPI_MSG_MAGIC_NUMBER) {
            AUD_LOG_W("%s(), msg msgic 0x%x fail!!", __FUNCTION__, msg->magic);
            usleep(500 * 1000);
            continue;
        }

        /* data buf */
        data_size = length_read - msg_size;
        data_buf = g_read_ipi_buf + msg_size;
        if (msg->task_scene < TASK_SCENE_SIZE) {
            if (g_ipi_dma_client[msg->task_scene].cbk != NULL) {
                g_ipi_dma_client[msg->task_scene].cbk(
                    msg,
                    data_buf,
                    data_size,
                    g_ipi_dma_client[msg->task_scene].arg);
            }
        }
    }

    pthread_exit(NULL);
    return NULL;
}


static void process_dma_msg(
    struct ipi_msg_t *msg,
    void *buf,
    uint32_t size,
    void *arg __unused)
{
    AUD_LOG_V("%s(), msg_%p, buf %p, size %u", __FUNCTION__, msg, buf, size);
    switch (msg->msg_id) {
#if defined(MTK_AUDIODSP_SUPPORT)
    case AUD_CTL_MSG_D2A_AURISYS_DUMP:
        process_aurisys_dsp_dump(msg, buf, size);
        break;
#endif
    default:
        DUMP_IPI_MSG("dma", msg);
        (void)buf;
        (void)size;
        break;
    }
}
#endif /* end of MTK_AUDIO_IPI_DMA_SUPPORT */


/*
 * =============================================================================
 *                     OpenDSP ioctl (TODO: refine)
 * =============================================================================
 */

void audio_load_task_scene(const uint8_t task_scene)
{
    int ret = 0;

    LOCK_ALOCK(g_drv_lock);
    if (g_ipi_drv < 0) {
        AUD_LOG_E("%s(), driver %s file node %d!!", __FUNCTION__,
                  AUDIO_IPI_DEVICE_PATH, g_ipi_drv);
        UNLOCK_ALOCK(g_drv_lock);
        return;
    }

    ret = ioctl(g_ipi_drv, AUDIO_IPI_IOCTL_LOAD_SCENE, task_scene);
    if (ret != 0) {
        AUD_LOG_E("%s() ioctl fail! ret = %d, errno: %d", __FUNCTION__, ret, errno);
    }

    UNLOCK_ALOCK(g_drv_lock);
}


uint32_t audio_get_dsp_id(const uint8_t task_scene) {
    if (task_scene >= TASK_SCENE_SIZE) {
        AUD_LOG_E("%s() task_scene %d fail", __FUNCTION__, task_scene);
        return AUDIO_OPENDSP_ID_INVALID;
    }

    return g_audio_task_info[task_scene].dsp_id;
}


bool is_audio_dsp_ready(const uint8_t task_scene) {
    uint32_t opendsp_id = 0;
    bool dsp_ready = false;

    if (task_scene >= TASK_SCENE_SIZE) {
        AUD_LOG_E("%s() task_scene %d fail", __FUNCTION__, task_scene);
        return false;
    }

    opendsp_id = g_audio_task_info[task_scene].dsp_id;
    if (g_audio_task_info[task_scene].is_dsp_support == false) {
        AUD_LOG_E("%s() task_scene %d dsp %u not support", __FUNCTION__,
                  task_scene, opendsp_id);
        return false;
    }

    switch (opendsp_id) {
    case AUDIO_OPENDSP_USE_CM4_A:
#if defined(MTK_AUDIO_SCP_RECOVERY_SUPPORT)
        dsp_ready = (is_scp_ready() == 1) ? true : false;
#endif
        break;
    case AUDIO_OPENDSP_USE_HIFI3_A:
#if defined(MTK_AUDIODSP_SUPPORT)
        dsp_ready = (is_adsp_ready(ADSP_A_ID) == 1) ? true : false;
#endif
        break;
    case AUDIO_OPENDSP_USE_HIFI3_B:
#if defined(MTK_AUDIODSP_SUPPORT)
        dsp_ready = (is_adsp_ready(ADSP_B_ID) == 1) ? true : false;
#endif
        break;
    default:
        AUD_LOG_W("%s(), opendsp_id %u not support", __FUNCTION__, opendsp_id);
        dsp_ready = false;
    }

    return dsp_ready;
}


uint8_t get_audio_controller_task(const uint8_t task_scene) {
    if (task_scene >= TASK_SCENE_SIZE) {
        AUD_LOG_E("%s() task_scene %d fail", __FUNCTION__, task_scene);
        return TASK_SCENE_INVALID;
    }

    return g_audio_task_info[task_scene].task_ctrl;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

