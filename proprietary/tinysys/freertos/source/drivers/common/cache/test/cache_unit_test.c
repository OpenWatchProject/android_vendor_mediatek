#include <FreeRTOS.h>
#include <task.h>
#include <cache_internal.h>
#include <tinysys_config.h>
#ifdef CFG_XGPT_SUPPORT
#include <interrupt.h>
#include <mt_gpt.h>
#endif

#define TEST_PRINTF     PRINTF_E
#define INT_TIMER       TMR0

DRAM_REGION_VARIABLE char dram_addr_a[1028];
DRAM_REGION_VARIABLE char dram_addr_b[512];
DRAM_REGION_VARIABLE char dram_addr_c[256];

static void task1_s_d(void *pvParameters)
{
    unsigned int *dram_addr = (unsigned int *)dram_addr_a;
    portTickType xLastExecutionTime, xDelayTime;
    unsigned int task1_param;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = ((portTickType) 223 / portTICK_RATE_MS);

    do {
        task1_param = 10000;
        TEST_PRINTF("...> task 1 works\n");

        while (task1_param) {
            *dram_addr = 0xaa5555aa;
            task1_param--;
        }
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
    } while (1);
}

DRAM_REGION_FUNCTION static void task2_d_d(void *pvParameters)
{
    unsigned int *dram_addr = (unsigned int *) dram_addr_b;
    portTickType xLastExecutionTime, xDelayTime;
    unsigned int task2_param;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = ((portTickType) 101 / portTICK_RATE_MS);

    do {
        task2_param = 20000;
        TEST_PRINTF("...> task 2 works\n");

        while (task2_param) {
            *dram_addr = 0xaa5555aa;
            task2_param--;
        }
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
    } while (1);
}

DRAM_REGION_FUNCTION static void task3_d_s(void *pvParameters)
{
    unsigned int sram;
    unsigned int *sram_addr = (unsigned int *)&sram;
    portTickType xLastExecutionTime, xDelayTime;
    unsigned int task3_param;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = ((portTickType) 53 / portTICK_RATE_MS);

    do {
        task3_param = 10000;
        TEST_PRINTF("...> task 3 works\n");

        while (task3_param) {
            *sram_addr = 0xaa5555aa;
            task3_param--;
        }

        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
    } while (1);
}

static void task4_s_d(void *pvParameters)
{
    unsigned int *dram_addr = (unsigned int *)dram_addr_c;
    portTickType xLastExecutionTime, xDelayTime;
    unsigned int task4_param;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = ((portTickType) 167/ portTICK_RATE_MS);

    do {
        task4_param = 10000;
        TEST_PRINTF("...> task 4 works\n");

        while (task4_param) {
            *dram_addr = 0xaa5555aa;
            task4_param--;
        }
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
    } while (1);
}

DRAM_REGION_FUNCTION static void task5_d_d(void *pvParameters)
{
    unsigned int *dram_addr = (unsigned int *) dram_addr_a;
    portTickType xLastExecutionTime, xDelayTime;
    unsigned int task5_param;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = ((portTickType) 313 / portTICK_RATE_MS);

    do {
        task5_param = 20000;
        TEST_PRINTF("...> task 5 works\n");

        while (task5_param) {
            *dram_addr = 0xaa5555aa;
            task5_param--;
        }
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
    } while (1);
}

DRAM_REGION_FUNCTION static void task6_d_s(void *pvParameters)
{
    unsigned int sram;
    unsigned int *sram_addr = (unsigned int *)&sram;
    portTickType xLastExecutionTime, xDelayTime;
    unsigned int task6_param;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = ((portTickType) 71 / portTICK_RATE_MS);

    do {
        task6_param = 10000;
        TEST_PRINTF("...> task 6 works\n");

        while (task6_param) {
            *sram_addr = 0xaa5555aa;
            task6_param--;
        }

        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
    } while (1);
}

static void prvTestTimerStart(void)
{
    struct timer_device *pxDev = id_to_dev(INT_TIMER);
    unsigned int ulBaseAddr = pxDev->base_addr;

    DRV_WriteReg32(ulBaseAddr + TIMER_RST_VAL, TIME_TO_TICK_MS(97));
    DRV_SetReg32(ulBaseAddr + TIMER_IRQ_CTRL_REG, TIMER_ENABLE);
    /* select clock source to clk_32k and enable timer */
    DRV_SetReg32(ulBaseAddr + TIMER_EN,
                 (TIMER_CLK_SRC_CLK_32K << TIMER_CLK_SRC_SHIFT) | TIMER_IRQ_ENABLE);
}

DRAM_REGION_FUNCTION static void prvTestTimerIntTask()
{
    struct timer_device *pxDev = id_to_dev(INT_TIMER);
    unsigned int ulBaseAddr = pxDev->base_addr;

    DRV_SetReg32(ulBaseAddr + TIMER_IRQ_CTRL_REG, TIMER_IRQ_CLEAR);
    TEST_PRINTF("...> timer interrupt\n");
}

void dmgr_test(void)
{
    struct timer_device *pxDev;

    kal_xTaskCreate(task1_s_d, "task1", 128, NULL, PRI_TEST, NULL);
    kal_xTaskCreate(task2_d_d, "task2", 128, NULL, PRI_TEST, NULL);
    kal_xTaskCreate(task3_d_s, "task3", 128, NULL, PRI_TEST, NULL);
    kal_xTaskCreate(task4_s_d, "task4", 128, NULL, PRI_TEST, NULL);
    kal_xTaskCreate(task5_d_d, "task5", 128, NULL, PRI_TEST, NULL);
    kal_xTaskCreate(task6_d_s, "task6", 128, NULL, PRI_TEST, NULL);

    pxDev = id_to_dev(INT_TIMER);
    request_irq(pxDev->irq_id, prvTestTimerIntTask, "TST_Timer");
    prvTestTimerStart();
}


