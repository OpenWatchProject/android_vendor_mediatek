/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "ftm.h"
#include "utils.h"

 
#define TAG "[Port] "

#if 0
int COM_Init (int *fd_atcmd, int *fd_uart, int *hUsbComPort)
{

    if((fd_atcmd == NULL) || (fd_uart == NULL) || (hUsbComPort == NULL))
    {
        return -1;
    }
    if(is_support_modem(1))
    {
    	  if(isMDENSupport(1)>0){
           *fd_atcmd = openDevice();
        }
    }

    else if(is_support_modem(2))
    {
        *fd_atcmd = openDeviceWithDeviceName("/dev/ccci2_tty0");
    }

    if((is_support_modem(1) || is_support_modem(2)) && (*fd_atcmd == -1)) 
    {
		LOGE(TAG "Open ccci port fail\r\n" );
        return *fd_atcmd;
    }
    
    //ATE Tool use 115200 baud rate
    *fd_uart = open_uart_port(UART_PORT1, 115200, 8, 'N', 1);
	if(*fd_uart == -1) 
    {
        LOGE(TAG "Open uart port %d fail\r\n" ,UART_PORT1);
        return *fd_uart;
    }
    else
    {
        LOGD(TAG "Open uart port %d success\r\n" ,UART_PORT1);
    }
  	
           
	//*hUsbComPort = open("/dev/ttyGS0",O_RDWR | O_NOCTTY | /*O_NONBLOCK | */O_NDELAY);
	*hUsbComPort = open_usb_port(UART_PORT1, 115200, 8, 'N', 1);
	if(*hUsbComPort == -1)
	{
		LOGE(TAG "Open usb fail\r\n");
		return *hUsbComPort;
	}
	else
	{
		//initTermIO(*hUsbComPort);
		LOGD(TAG "Open usb success\r\n");
	}

	return 0;
}

int COM_DeInit (int *fd_atcmd, int *fd_uart, int *hUsbComPort)
{

        
    if((fd_atcmd == NULL) || (fd_uart == NULL) || (hUsbComPort == NULL))
    {
        return -1;
    }

	    //release the handle
    if (*fd_atcmd != -1)
    {
        close(*fd_atcmd);
		*fd_atcmd = -1;
    }
    if (*fd_uart != -1)
    {
        close(*fd_uart);
		*fd_uart = -1;
    }
    if (*hUsbComPort != -1)
    {
        close(*hUsbComPort);
		*hUsbComPort = -1;
    }
	return 0;
}
#endif



 
