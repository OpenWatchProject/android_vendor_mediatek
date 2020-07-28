#include <stdio.h>
#include <string.h>
#include <platform_mtk.h>
#include <interrupt.h>
#include <utils.h>
#include <tinysys_config.h>

#include <FreeRTOS.h>
#include <task.h>

#include "feature01_os.h"
#include "feature01_sample.h"

#ifdef CFG_DO_ENABLED
struct f01_cb_funcs_t m_f01_cb_funcs = {
    .f01_cb1 = NULL,
    .f01_cb1 = NULL
};
#else
extern struct f01_cb_funcs_t m_f01_cb_funcs;
#endif

void fb_sample_task(void* pvParameters){

    while(1){
        vTaskDelay(3000);
        if(m_f01_cb_funcs.f01_cb1)
            PRINTF_D("m_f01_cb_funcs.f01_cb1(1): %d\n", m_f01_cb_funcs.f01_cb1(1));
        if(m_f01_cb_funcs.f01_cb2)
            PRINTF_D("m_f01_cb_funcs.f01_cb2(2): %d\n", m_f01_cb_funcs.f01_cb2(2));
        PRINTF_D("feature01 task running...\n\r");
    }

}

void fb_sample_task_init(void){

    // create task
    kal_xTaskCreate(
            (TaskFunction_t)fb_sample_task,
            "feat01_task",
            256,
            (void*)NULL, 3, NULL);
}


