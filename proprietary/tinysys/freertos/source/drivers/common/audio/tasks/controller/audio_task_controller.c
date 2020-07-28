#include <audio_task_controller.h>

#include <stdint.h>
#include <stdbool.h>

#include <string.h>

#include <errno.h>


#include <tinysys_config.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <interrupt.h>


#include <audio_log.h>
#include <audio_assert.h>

#include <audio_task.h>
#include <audio_task_factory.h>

#include <audio_memory_control.h>
#include <audio_messenger_ipi.h>


#include <audio_controller_msg_id.h>

#include <audio_ipi_dma.h>


#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <aurisys_controller.h>
#include <aurisys_adb_command.h>
#endif
#if defined(CFG_SMARTPA_USE_CACHE) && defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
#include <cache_internal.h>
#endif

/*
 * =============================================================================
 *                     log
 * =============================================================================
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[AUD][CTRL]"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "%s(), " fmt "\n" , __func__


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#ifdef CFG_MTK_AUDIODSP_SUPPORT
#define LOCAL_TASK_STACK_SIZE (4096)
#define MAX_MSG_QUEUE_SIZE (8)
#define LOCAL_TASK_SCENE (TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A)
#else
#define LOCAL_TASK_STACK_SIZE (768)
#define MAX_MSG_QUEUE_SIZE (4)
#define LOCAL_TASK_SCENE (TASK_SCENE_AUDIO_CONTROLLER_CM4)
#endif
#define LOCAL_TASK_NAME "aud_ctl"
#define LOCAL_TASK_PRIORITY (3)


/*
 * =============================================================================
 *                     private functions - declaration
 * =============================================================================
 */

static void task_controller_task_loop(void *arg);

static void task_controller_parsing_message(
    struct AudioTask *this,
    ipi_msg_t *ipi_msg);

#if defined(CFG_SMARTPA_USE_CACHE) && defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
SRAM_REGION_FUNCTION
#endif
static uint8_t get_queue_idx(struct AudioTask *this)
{
    uint8_t queue_idx = this->queue_idx;

    this->queue_idx++;
    if (this->queue_idx == MAX_MSG_QUEUE_SIZE) {
        this->queue_idx = 0;
    }

    return queue_idx;
}


/*==============================================================================
 *                     task functions - declaration
 *============================================================================*/

static void task_controller_constructor(struct AudioTask *this);
static void task_controller_destructor(struct AudioTask *this);

static void task_controller_create_task_loop(struct AudioTask *this);
static void task_controller_destroy_task_loop(struct AudioTask *this);

static status_t  task_controller_recv_message(struct AudioTask *this, struct ipi_msg_t *ipi_msg);
static void      task_controller_irq_hanlder(struct AudioTask *this, uint32_t irq_type);
static void      task_controller_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type);



/*
 * =============================================================================
 *                     task
 * =============================================================================
 */

AudioTask *task_controller_new()
{
    /* alloc object here */
    AudioTask *task = (AudioTask *)kal_pvPortMalloc(sizeof(AudioTask));
    if (task == NULL) {
        ALOGE("kal_pvPortMalloc fail!!");
        return NULL;
    } else {
        ALOGD("%s create, task = %p, Heap free = %d\n",
              LOCAL_TASK_NAME, task, xPortGetFreeHeapSize());
    }
    memset(task, 0, sizeof(AudioTask));

    /* only assign methods, but not class members here */
    task->constructor       = task_controller_constructor;
    task->destructor        = task_controller_destructor;

    task->create_task_loop  = task_controller_create_task_loop;
    task->destroy_task_loop = task_controller_destroy_task_loop;

    task->recv_message      = task_controller_recv_message;
    task->irq_hanlder       = task_controller_irq_hanlder;

    task->hal_reboot_cbk    = task_controller_hal_reboot_cbk;

    return task;
}


void task_controller_delete(AudioTask *task)
{
    ALOGD("(-)");
    if (task == NULL) {
        ALOGE("task is NULL!!");
        return;
    }

    kal_vPortFree(task);
}


static void task_controller_constructor(struct AudioTask *this)
{
    if (this == NULL) {
        ALOGE("AudioTask == NULL!!!");
        AUD_ASSERT(this != NULL);
        return;
    } else {
        ALOGD("%s create, AudioTask=%p, Heap free = %d\n",
              LOCAL_TASK_NAME, this, xPortGetFreeHeapSize());
    }
    /* assign initial value for class members */
    this->scene = LOCAL_TASK_SCENE;
    this->state = AUDIO_TASK_IDLE;


    /* queue */
    this->queue_idx = 0;
    this->num_queue_element = 0;

    this->msg_array = (ipi_msg_t *)kal_pvPortMalloc(
                          MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));
    AUD_ASSERT(this->msg_array != NULL);
    memset(this->msg_array, 0, MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));

    this->msg_idx_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE, sizeof(uint8_t));
    AUD_ASSERT(this->msg_idx_queue != NULL);


    /* alloc private memory here */


}


static void task_controller_destructor(struct AudioTask *this)
{
    if (this == NULL) {
        ALOGE("AudioTask == NULL!!!");
        AUD_ASSERT(this != NULL);
        return;
    }
    AUD_ASSERT(this->state == AUDIO_TASK_IDLE);


    /* dealloc object here */


    /* dealloc private memory */
    if (this->msg_array != NULL) {
        kal_vPortFree(this->msg_array);
        this->msg_array = NULL;
    }
    if (this->msg_idx_queue != NULL) {
        vQueueDelete(this->msg_idx_queue);
        this->msg_idx_queue = NULL;
    }
}


static void task_controller_create_task_loop(struct AudioTask *this)
{
    BaseType_t xReturn = pdFAIL;
    ALOGD("(+) %s create xTaskCreate, AudioTask=%p, Heap free = %d\n",
              LOCAL_TASK_NAME, this, xPortGetFreeHeapSize());

    xReturn = kal_xTaskCreate(
                  task_controller_task_loop,
                  LOCAL_TASK_NAME,
                  LOCAL_TASK_STACK_SIZE,
                  (void *)this,
                  LOCAL_TASK_PRIORITY,
                  &this->freertos_task);
    ALOGD("(-) %s create, AudioTask=%p, Heap free = %d\n",
              LOCAL_TASK_NAME, this, xPortGetFreeHeapSize());

    AUD_ASSERT(xReturn == pdPASS);
    AUD_ASSERT(this->freertos_task != 0);
}


static void task_controller_destroy_task_loop(struct AudioTask *this)
{
#if (INCLUDE_vTaskDelete == 1)
    if (this->freertos_task != 0) {
        vTaskDelete(this->freertos_task);
        this->freertos_task = 0;
    }
#endif
}


static void task_controller_irq_hanlder(
    struct AudioTask *this,
    uint32_t irq_type)
{
    ipi_msg_t ipi_msg;

    ipi_msg.magic        = IPI_MSG_MAGIC_NUMBER;
    ipi_msg.task_scene   = LOCAL_TASK_SCENE;
    ipi_msg.target_layer = AUDIO_IPI_LAYER_TO_DSP;
    ipi_msg.data_type    = AUDIO_IPI_MSG_ONLY;
    ipi_msg.ack_type     = AUDIO_IPI_MSG_BYPASS_ACK;
    ipi_msg.msg_id       = AUD_CTL_MSG_D2D_IRQ;
    ipi_msg.param1       = irq_type;
    ipi_msg.param2       = 0;

    task_controller_recv_message(this, &ipi_msg);
}

#if defined(CFG_SMARTPA_USE_CACHE) && defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
SRAM_REGION_FUNCTION
#endif
static status_t task_controller_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    ipi_msg_t *ipi_msg_in_q = NULL;
    uint8_t queue_idx = 0;
    int i = 0;

    this->num_queue_element++;
    queue_idx = get_queue_idx(this);
    ipi_msg_in_q = &this->msg_array[queue_idx];

    if (ipi_msg_in_q->magic != 0) {
        ALOGE("queue_idx = %d", queue_idx);
        for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
            ALOGE("[%d] id = 0x%x", i, this->msg_array[i].msg_id);
        }
        AUD_WARNING("queue full!!");
    }
    memcpy(ipi_msg_in_q, ipi_msg, sizeof(ipi_msg_t));

    xQueueSendToBackFromISR(
        this->msg_idx_queue,
        &queue_idx,
        &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return 0;
}


static void task_controller_task_loop(void *arg)
{
    AudioTask *this = (AudioTask *)arg;
    uint8_t local_queue_idx = 0xFF;
    struct ipi_msg_t *ipi_msg = NULL;

    if (arg == NULL) {
        ALOGE("%s(), arg = NULL!!!!", __func__);
        AUD_ASSERT(arg != NULL);
    }

    while (1) {
        xQueueReceive(this->msg_idx_queue, &local_queue_idx, portMAX_DELAY);
        if (local_queue_idx >= MAX_MSG_QUEUE_SIZE) {
            ALOGE("xQueueReceive fail!! idx %d", local_queue_idx);
            continue;
        }
        ipi_msg = &this->msg_array[local_queue_idx];

        /* process message */
        task_controller_parsing_message(this, ipi_msg);

        /* send ack back if need */
        audio_send_ipi_msg_ack_back(ipi_msg);

        /* clean msg */
        memset(ipi_msg, 0, sizeof(ipi_msg_t));

        /* TODO: pop */
        this->num_queue_element--;
    }
}


static void task_controller_parsing_message(struct AudioTask *this, ipi_msg_t *ipi_msg)
{
    void *data_buf = NULL;
    uint32_t data_size = 0;
    ALOGV("msg_id 0x%x", ipi_msg->msg_id);

    switch (ipi_msg->msg_id) {
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    case AUD_CTL_MSG_A2D_AURISYS_CFG:
        audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
        init_aurisys_controller(data_buf, data_size);
        AUDIO_FREE_POINTER(data_buf);
        break;
    case AUD_CTL_MSG_A2D_AURISYS_SET_PARAM:
        audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
        ipi_msg->param1 = aurisys_set_parameter(parse_adb_cmd(data_buf, data_size));
        AUDIO_FREE_POINTER(data_buf);
        break;
    case AUD_CTL_MSG_A2D_AURISYS_GET_PARAM:
        audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
        ipi_msg->param1 = aurisys_get_parameter(
                              parse_adb_cmd(data_buf, data_size),
                              ipi_msg->dma_info.wb_dram.addr,
                              ipi_msg->dma_info.wb_dram.memory_size,
                              &ipi_msg->dma_info.wb_dram.data_size);
        AUDIO_FREE_POINTER(data_buf);
        break;
    case AUD_CTL_MSG_A2D_AURISYS_ENABLE:
        set_aurisys_on(ipi_msg->param1);
        break;
#endif
    case AUD_CTL_MSG_A2D_HAL_REBOOT:
        notify_audio_task_hal_reboot(ipi_msg->param1);
        break;
    case AUD_CTL_MSG_A2D_DMA_INIT:
        init_audio_ipi_dma(ipi_msg);
        break;
    case AUD_CTL_MSG_A2D_DMA_UPDATE_REGION:
        audio_ipi_dma_update_region_info(ipi_msg);
        break;
    case AUD_CTL_MSG_D2D_IRQ:
        DUMP_IPI_MSG("TODO: irq", ipi_msg);
        break;
    default:
        ALOGW("unknown msg_id 0x%x, drop!!", ipi_msg->msg_id);
        if (ipi_msg->data_type == AUDIO_IPI_DMA) {
            audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
            AUDIO_FREE_POINTER(data_buf);
        }
    }
}


static void task_controller_hal_reboot_cbk(
    struct AudioTask *this,
    const uint8_t hal_type)
{
    ALOGD("scene %d", this->scene);

#ifdef CFG_MTK_AUDIODSP_SUPPORT
    deinit_aurisys_controller();
#endif
}



