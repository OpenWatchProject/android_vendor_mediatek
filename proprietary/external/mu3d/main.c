/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>

extern void CLI_Init(void);

#define CLI_MAGIC 'C'
//#define CLI_MAGIC 'CLI_MU3D'
#define IOCTL_READ _IOR(CLI_MAGIC, 0, int)
#define IOCTL_WRITE _IOW(CLI_MAGIC, 1, int)

int fd = -1;

int write_cmd(char *buf)
{
	int ret;

	if ((ret = ioctl(fd, IOCTL_WRITE, buf)) != 0)
		printf("[FAIL]IOCTL_WRITE ioctl=%x, errno=%x\n", IOCTL_WRITE, errno);
	else
		printf("[PASS]IOCTL_WRITE: %s\n", buf);

	//negative value is returned if there is problem
	return ret;
}

int read_msg(char *buf)
{
	int ret;

	if ((ret = ioctl(fd, IOCTL_READ, buf)) < 0)
		printf("IOCTL_READ ERROR");
	else
		printf("IOCTL_READ: %s\r\n", buf);

	//negative value is returned if there is problem
	return ret;
}

// eddie
void SetTTY(int restore)
{
	struct termios tty;
	static struct termios otty;
	//    int STDIN_FILENO = fileno(stdin);
	if (!restore)
	{
		// Get original tty settings and save them in otty
		tcgetattr(STDIN_FILENO, &otty);
		//tty = otty;
		memcpy(&tty, &otty, sizeof(struct termios));
		// Now set the terminal to char-by-char input
		tty.c_lflag = tty.c_lflag & (unsigned int) (~(ECHO | ECHOK | ICANON));
		tty.c_cc[VTIME] = 1;
		tcsetattr(STDIN_FILENO, TCSANOW, &tty);
	}
	else{
		// Reset to the original settings
		memcpy(&tty, &otty, sizeof(struct termios));
		tty.c_lflag = tty.c_lflag | (unsigned int) ((ECHO | ECHOK | ICANON));
		tcsetattr(STDIN_FILENO, TCSANOW, &tty);
		//        tcsetattr(STDIN_FILENO, TCSANOW, &otty);
	}
}


int main(int argc, const char ** argv)
{
	char cmd_buf[256] = "";
	int i ;
	//open cli character device
	//if ((fd = open("/dev/mu3d_mtk_test", O_RDWR)) < 0)
	if ((fd = open("/dev/cli", O_RDWR)) < 0)
	{
		printf("cannot open /dev/cli");
		return -1;
	}
	//eddie
	SetTTY(0);
#if 0	
	CLI_Init();
#else	
	// printf("[U3D_CLI] input , argc = %d\n", argc);

	// printf("[U3D_CLI] input , argv =[", argc);
	
	for (i= 1 ; i < argc ; i++){
		
		printf("%s ", argv[i]);
		strcat(cmd_buf, argv[i]) ;
		strcat(cmd_buf, " ") ;
	}		

	printf("]\n") ;

	printf("[U3D_CLI] the cmd buffer is %s\n", cmd_buf) ;

	write_cmd(cmd_buf) ;
#endif

	close(fd);
	SetTTY(1);
	return 0;
}

