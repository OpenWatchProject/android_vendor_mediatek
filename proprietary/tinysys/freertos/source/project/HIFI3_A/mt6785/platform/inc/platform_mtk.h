/*
 * platform_mtk.h
 *
 *  Created on: 2017/9/28
 *      Author: MTK09865
 */
#ifndef __PLATFORM_H
#define __PLATFORM_H
#include <hifi3_irq.h>

#define IPC_SHARE_BUFFER_ADDR    __attribute__ ((section(".IPCBuffer")))
#define OS_TIMER_ADDR            __attribute__ ((section(".OSTimer")))
#define MPU_MEM_INFO_ADDR        __attribute__ ((section(".MPUMemory")))

#endif

