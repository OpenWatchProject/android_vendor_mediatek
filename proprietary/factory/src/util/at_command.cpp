/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "utils.h"
#include <pthread.h>
#include <termios.h> 

#include "libnvram.h"

#include "CFG_file_lid.h"
#include "CFG_PRODUCT_INFO_File.h"
#include "Custom_NvRam_LID.h"
#include "CFG_Wifi_File.h"
#include "CFG_BT_File.h"

#include "hardware/ccci_intf.h"
#include <sys/time.h>

#include "me_connection.h"
#include "at_command.h"
#include <cutils/properties.h>
#define TAG "[AT Command]"
#ifdef FACTORY_C2K_SUPPORT
#ifndef MTK_ECCCI_C2K
#include <c2kutils.h>
#endif
#endif


#define BUF_SIZE 128
#define HALT_INTERVAL 20000
#define MAX_MODEM_INDEX 4
#define MAX_MODEM_INDEX_LTE 3

pthread_mutex_t ccci_mutex = PTHREAD_MUTEX_INITIALIZER;
int g_Flag_CREG = 0 ;
int g_Flag_ESPEECH_ECPI = 0;
int g_Flag_CONN  = 0;
int g_Flag_EIND  = 0;
int g_Flag_VPUP  = 0;
int g_Flag_VPON = 0;
int g_Flag_EUSIM = 0;
int g_Flag_UIMST = 0;

#define CCCI_IOC_MAGIC 'C'
#define CCCI_IOC_ENTER_DEEP_FLIGHT _IO(CCCI_IOC_MAGIC, 14) //CCI will not kill muxd/rild
#define CCCI_IOC_LEAVE_DEEP_FLIGHT _IO(CCCI_IOC_MAGIC, 15) //CCI will kill muxd/rild

static speed_t baud_bits[] = {
    0, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B921600
};


bool isCompMd() { // supports both lte and c2k
    LOGD(TAG "Entry %s!\n", __FUNCTION__) ;
    //default set to false
    bool supportMD = false;
    char property_value[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.vendor.mtk_ril_mode", property_value, "none");
    LOGD(TAG "ro.mtk_ril_mode = %s!\n", property_value) ;
    LOGD(TAG "strcmp(property_value, c6m_1rild) = %d!\n", strcmp(property_value, "c6m_1rild")) ;

    if (strcmp(property_value, "c6m_1rild") == 0) {
        supportMD = true;
    }
    return supportMD;
}

void initTermIO(int portFd, int cmux_port_speed)
{
    struct termios uart_cfg_opt;
    tcgetattr(portFd, &uart_cfg_opt);
    tcflush(portFd, TCIOFLUSH);
    
    /*set standard buadrate setting*/
    speed_t speed = baud_bits[cmux_port_speed];
    cfsetospeed(&uart_cfg_opt, speed);
    cfsetispeed(&uart_cfg_opt, speed);
    
    uart_cfg_opt.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);   
    uart_cfg_opt.c_cflag |= CREAD | CLOCAL | CS8 ;
    
    /* Raw data */
    uart_cfg_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    uart_cfg_opt.c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);
    uart_cfg_opt.c_oflag &=~(INLCR|IGNCR|ICRNL);
    uart_cfg_opt.c_oflag &=~(ONLCR|OCRNL);
    
    /* Non flow control */
    //uart_cfg_opt.c_cflag &= ~CRTSCTS;                /*clear flags for hardware flow control*/
    uart_cfg_opt.c_cflag |= CRTSCTS;
    uart_cfg_opt.c_iflag &= ~(IXON | IXOFF | IXANY); /*clear flags for software flow control*/          
    
    // Set time out
    uart_cfg_opt.c_cc[VMIN] = 1;
    uart_cfg_opt.c_cc[VTIME] = 0;
    
    /* Apply new settings */
    if(tcsetattr(portFd, TCSANOW, &uart_cfg_opt)<0)
    {
        LOGD(TAG "set terminal parameter fail");
    }
    int status = TIOCM_DTR | TIOCM_RTS;
    ioctl(portFd, TIOCMBIS, &status);

}

int is_support_modem(int modem)
{
    int ret = 0;
    switch (modem)
    {
    case 1:
    case 2:
    case 5:
        if (isMDENSupport(modem) > 0)
        {
            ret = 1;
        }
		break;
	case 3:
        #ifdef MTK_DT_SUPPORT
            ret = 1;
        #endif
        break;
	case 4:
        if(isC2kSupport() == 1){
            ret = 1;
        }
        break;
    default:
        break;
    }
    return ret;
}

void closeDevice(int fd)
{
    close(fd);
}

int openDeviceWithDeviceName(char *deviceName)
{
    LOGD(TAG "%s - %s\n", __FUNCTION__, deviceName);
    int fd;
    fd = open(deviceName, O_RDWR | O_NONBLOCK);
    if(fd < 0) {
        LOGD(TAG "Fail to open %s: %s\n", deviceName, strerror(errno));
        return -1;
    }

    return fd;
}

int ExitFlightMode_PowerOffModem(int fd, int ioctl_fd, _BOOL bON){
    LOGD(TAG "[AT]ExitFlightMode_PowerOffModem\n");

    if(bON) {
        LOGD(TAG "[AT]CCCI_IOC_LEAVE_DEEP_FLIGHT \n");        
        int ret_ioctl_val = ioctl(ioctl_fd, CCCI_IOC_LEAVE_DEEP_FLIGHT);
        LOGD("[AT]CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, CCCI_IOC_LEAVE_DEEP_FLIGHT);
    } else {
        do
        {
            send_at (fd, "AT\r\n");
        } while (wait4_ack (fd, NULL, 300));

        LOGD(TAG "[AT]Enable Sleep Mode:\n");        
        if (send_at (fd, "AT+ESLP=1\r\n")) goto err;
        if (wait4_ack (fd, NULL, 3000)) goto err;

        LOGD(TAG "[AT]Power OFF Modem:\n");
        if (send_at (fd, "AT+EFUN=0\r\n")) goto err;
        wait4_ack (fd, NULL, 15000); 
        if (send_at (fd, "AT+EPOF\r\n")) goto err;
        wait4_ack (fd, NULL, 10000);

        LOGD(TAG "[AT]CCCI_IOC_ENTER_DEEP_FLIGHT \n");        

        int ret_ioctl_val = ioctl(ioctl_fd, CCCI_IOC_ENTER_DEEP_FLIGHT);
        LOGD("[AT]CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, CCCI_IOC_ENTER_DEEP_FLIGHT);
    }

err:
    return -1;
}

int get_lte_md_count()
{
    int md_count = 0;
    int i = 0;
    int idx[MAX_MODEM_INDEX_LTE] = {1,2,5};

    if (isCompMd()) { //the project use 93 md has only one modem
        LOGD(TAG "it is 93 md %s!\n", __FUNCTION__) ;
        return 1;
    }

    for(i = 0; i < MAX_MODEM_INDEX_LTE; i++)
    {
        if(is_support_modem(idx[i]))
        {
            md_count++;
        }
    }
    return md_count;
}

int get_md_count()
{
    int md_count = 0;
    if (isCompMd()) { //the project use 93/95 md has only one modem
        LOGD(TAG "it is 93 md %s!\n", __FUNCTION__) ;
        return 1;
    }

    md_count = get_lte_md_count();
    #if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_FTM_DT_VIA_SUPPORT)

        #if defined(PURE_AP_USE_EXTERNAL_MODEM)
            md_count++;
        #endif

    #endif

    if (isC2kSupport()) {
        md_count++;
    }
    LOGD(TAG "md_count = %d \n", md_count);	

    return md_count;
}

int get_ccci_path(int modem_index,char * path)
{
    int idx[MAX_MODEM_INDEX] = {1,2,5,4}; // 4 for C2K
    int md_sys[MAX_MODEM_INDEX] = {MD_SYS1, MD_SYS2, MD_SYS5, MD_SYS3};
    if(is_support_modem(idx[modem_index]))
    {
        if(modem_index == 3)
        {
            if (isC2kSupport()) {
                #if (!defined(MTK_ECCCI_C2K) && defined(FACTORY_C2K_SUPPORT))
                char *asciDevice = NULL;
                asciDevice = viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_AT);
                strncpy(path, asciDevice, strlen(asciDevice));
                #else
                snprintf(path, 32, "%s", ccci_get_node_name(USR_C2K_AT, (CCCI_MD)md_sys[modem_index]));
                #endif
            }
        }
        else
        {
            snprintf(path, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, (CCCI_MD)md_sys[modem_index]));          
        }
        LOGD(TAG "CCCI Path:%s",path); 
        return 1 ;
    } else {
       return 0 ;
    }
}

int send_at (const int fd, const char *pCMD)
{
    int ret = 0;
    unsigned int sent_len = 0;
    LOGD(TAG "Send AT CMD: %s\n", pCMD);
    if((fd == -1) || (pCMD == NULL))
    {
        return -1;
    }
    while (sent_len != strlen(pCMD))
    {
        //LOGD("send_at ccci_mutex try lock\n");
        if (pthread_mutex_lock (&ccci_mutex))
        {
            LOGE(TAG "send_at pthread_mutex_lock ERROR!\n"); 
        }
        //LOGD("send_at ccci_mutex lock done\n");

        ret = write(fd, pCMD, strlen(pCMD));


        //LOGD("send_at ccci_mutex try unlock\n");
        if (pthread_mutex_unlock (&ccci_mutex))
        {
            LOGE(TAG "send_at pthread_mutex_unlock ERROR!\n"); 
        }
        //LOGD("send_at ccci_mutex unlock done\n");
        
        if (ret<0)
        {
            LOGE(TAG "ccci write fail! Error code = 0x%x\n", errno); 
            return ret;
        }
        else
        {    
            sent_len += ret;
            LOGD(TAG "[send_at] lenth = %d\n", sent_len);
        }
    }
    return 0;
}

int read_ack (const int fd, char *rbuff, int length)
{
    int has_read = 0;
    ssize_t      ret_val;

    if(-1 == fd)
    return -1;

    LOGD("Enter read_ack(): uart = %d\n", fd);
    memset (rbuff, 0, length);

    while(has_read<length)
    {

loop:
        usleep(HALT_INTERVAL);
        LOGD("read_ack ccci_mutex try lock\n");
        if (pthread_mutex_lock (&ccci_mutex))
        {
            LOGE( "read_ack pthread_mutex_lock ERROR!\n"); 
        }
        LOGD("read_ack ccci_mutex lock done\n");
        
        ret_val = read(fd, &rbuff[has_read], length);
        LOGD("read_ack ccci_mutex try unlock\n");
        if (pthread_mutex_unlock (&ccci_mutex))
        {
            LOGE( "read_ack pthread_mutex_unlock ERROR!\n"); 
        }
        LOGD("read_ack ccci_mutex unlock done\n");
        LOGD("ret_val %zd",ret_val);
        if(-1 == ret_val)
        {
            if (errno == EAGAIN)
            {
                LOGD("ccci can't read a byte!\n"); 
            }
            else
                LOGE("ccci read fail! Error code = 0x%x\n", errno); 
            
            //continue;  
            goto loop;
        }
        //if( (rbuff[has_read]!='\r')&&(rbuff[has_read]!='\n') )
        if(ret_val>2)
        {
            has_read += (unsigned int)ret_val;
            if (strstr(rbuff, "\r\n"))  break;
        }
    }
    LOGD("read_ack %s",rbuff);
    return has_read;
}

int wait4_readAck(const int fd, int timeout, char *buf) {
    ssize_t ret = -1;
    fd_set rfds;
    struct timeval select_timeout;
    unsigned int has_read = 0;

    memset (buf, 0, BUF_SIZE);

    select_timeout.tv_sec = timeout;
    select_timeout.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    LOGD(TAG "wait4_readAck E\n");

    while (has_read < BUF_SIZE) {
        ret = select(fd + 1, &rfds, NULL, NULL, &select_timeout);
        if (ret == -1) {
            if(errno == EINTR || errno == EAGAIN) {
                ret = -1;
                continue;
            } else {
                LOGE("Fail to select. error: %d", errno);
                return -1;
            }
        } else if (ret == 0) {
            ret = -1;
            break;
        }
        if (FD_ISSET(fd, &rfds)) {
            if (pthread_mutex_lock (&ccci_mutex)) {
                LOGE( "read_ack pthread_mutex_lock ERROR!\n");
            }
            ret = read(fd, &buf[has_read], BUF_SIZE);
            LOGD(TAG "wait4_readAck ret %zu\n", ret);
            if (pthread_mutex_unlock (&ccci_mutex)) {
                LOGE( "2read_ack pthread_mutex_unlock ERROR!\n");
            }
            if(-1 == ret) {
                if (errno == EAGAIN) {
                    LOGD("ccci can't read a byte!\n");
                } else {
                    LOGE("ccci read fail! Error code = 0x%x\n", errno);
                }
            } else {
                if (strlen(buf) >= BUF_SIZE) {
                    break ;
                }
                if (ret > 2) {
                    has_read += (unsigned int)ret;
                    if (strstr(buf, "\r\n")) {
                        break;
                    }
                }
            }
        }
    }
    LOGD("wait4_readAck ret = %zu",ret);
    return ret;
}

int wait4_ack (const int fd, char *pACK, int timeout)
{
    char buf[BUF_SIZE] = {0};
    char *  p = NULL;
    int rdCount = 0, LOOP_MAX;
    int ret = -1;
    LOOP_MAX = timeout*1000/HALT_INTERVAL;
    LOGD(TAG "Wait for AT ACK...: %s; Special Pattern: %s\n", buf, (pACK==NULL)?"NULL":pACK);
    for(rdCount = 0; rdCount < LOOP_MAX; ++rdCount) 
    {
        memset(buf,'\0',BUF_SIZE);    
        if (pthread_mutex_lock (&ccci_mutex))
        {
            LOGE( "read_ack pthread_mutex_lock ERROR!\n"); 
        }
        ret = read(fd, buf, BUF_SIZE);
        if (pthread_mutex_unlock (&ccci_mutex))
        {
            LOGE( "read_ack pthread_mutex_unlock ERROR!\n"); 
        }
        LOGD(TAG "AT CMD ACK: %s.rdCount=%d\n", buf,rdCount);
        p = NULL;
        if (pACK != NULL)  
        {
            p = strstr(buf, pACK);
            if(p) {
                ret = 0; break; 
            }
            p = strstr(buf, "ERROR");
            if(p) {
                ret = -1; break;
            }
            p = strstr(buf, "NO CARRIER");
            if(p) {
                ret = -1; break;
            }
        }
        else
        {
            p = strstr(buf, "OK");
            if(p) {
                LOGD(TAG "Char before OK are %c,%c.\n",*(p-2),*(p-1) );            
                if(*(p-2) == 'E' && *(p-1) == 'P'){
                    char * ptr = NULL;
                    ptr = strstr(p+1, "OK");
                    if(ptr){
                        LOGD(TAG "EPOK detected and OK followed\n");
                        ret = 0; break;
                    } else {
                        LOGD(TAG "EPOK detected and no futher OK\n");
                    }
                }else{    
                    LOGD(TAG "OK response detected\n");                        
                    ret = 0; break;
                }
            }
            p = strstr(buf, "ERROR");
            if(p) {
                ret = -1; break;
            }
            p = strstr(buf, "NO CARRIER");
            if(p) {
                ret = -1; break;
            }
        }
        usleep(HALT_INTERVAL);
    }
    LOGD("ret = %d",ret);
    return ret;
}

int openDevice(void)
{
    int fd;
    fd = open("/dev/ttyC0", O_RDWR | O_NONBLOCK);
    if(fd < 0) {
        LOGD(TAG "Fail to open ttyC0: %s\n", strerror(errno));
        return -1;
    }
    return fd;
}

void handleLPlusGMd1(Connection& modem) {
    char value[100] = {0};
	int sim_switch_flag = 1;
    property_get("persist.vendor.radio.mtk_ps2_rat",value,"G");
    LOGD(TAG "mtk_ps2_rat = %s",value);
    if (strcmp(value, "G") == 0) {
        int ret = modem.Query3GStatus(sim_switch_flag);
        LOGD(TAG "ret = %d, sim_switch_flag:%d\n", ret, sim_switch_flag);
        if (sim_switch_flag == 1) {
            modem.SetUARTOwner(5);
        }
        LOGD(TAG "send ERAT=0 to ps2\n");
        if (modem.ChangeMode(0)) {
            return;
        }
        LOGD(TAG "send ERAT=0 to ps2 done\n");
        if (sim_switch_flag == 1) {
            modem.SetUARTOwner(4);
        }
    }
}

int ExitFlightMode_DualTalk(Connection& modem)
{
    int i = 0 ;
    LOGD(TAG "%s\n", __FUNCTION__);
    if(g_Flag_EIND != 1)
    {
        LOGD(TAG "[AT]Detect modem status:\n");
        if(ER_OK!= modem.QueryModemStatus())
        {
            g_Flag_EIND = 0 ;
            wait_URC(ID_EIND);
        }
        else
        {
            g_Flag_EIND = 1 ;
        }
    }
      
    LOGD(TAG "[AT]Tidy the format:\n");
    if(ER_OK!= modem.SetModemFunc(0))
        return -1 ;
    LOGD(TAG "[AT]Detect MD active status:\n");
    if(ER_OK!= modem.QueryModemStatus())
        return -1 ;
    
      LOGD(TAG "[AT]Disable Sleep Mode:\n");
    if(ER_OK!= modem.SetSleepMode(0))
        return -1 ;
    
    LOGD(TAG "[AT]Reset SIM1:\n");
    if(ER_OK!= modem.QuerySIMStatus(i))
        return -1 ;    
  
    LOGD(TAG "[AT]Set net work flag:\n");
    if(ER_OK!= modem.SetNetworkRegInd(1))
        return -1 ;

    if(isRLTE_VLTESupport()!=0){
        LOGD(TAG "[AT]For iRat");
        if(ER_OK!= modem.EMDSTATUS())
            return -1; 

        LOGD(TAG "[AT]Change Modem mode for ECC call");
        if(ER_OK!= modem.ChangeMode(6))
            return -1;    
    }
    LOGD(TAG "[AT]Open RF test:\n");
    if(ER_OK!= modem.SetNormalRFMode(1))
        return -1 ;
    LOGD(TAG "[AT]Wait for CREG:\n");
    if( -1 == wait_Signal_CREG(40)) {
         return -1 ;
    }
    LOGD(TAG "Wait for CREG done !") ;
    return 0 ;
}

int ExitFlightMode(Connection& modem)
{
    static int bInit = 0; 
    int i ;
    int sim_num = ftm_get_sim_num();
      
    if(g_Flag_EIND != 1)
    {
        LOGD(TAG "[AT]Detect modem status:\n");
        if(ER_OK!= modem.QueryModemStatus())
        {
            g_Flag_EIND = 0 ;
            wait_URC(ID_EIND);
        }
        else
        {
            g_Flag_EIND = 1 ;
        }
    }

    LOGD(TAG "[AT]Tidy the format:\n");
    if(ER_OK!= modem.SetModemFunc(0))
    return -1 ;
    
    LOGD(TAG "[AT]Disable Sleep Mode:\n");
    if(ER_OK!= modem.SetSleepMode(0))
    return -1 ;
    if (sim_num >= 2) {
        if (bInit == 0)
        {
            LOGD(TAG "[AT]Reset SIM1:\n");
            if(ER_OK!= modem.QuerySIMStatus(i))
                return -1 ;
        
            #ifndef FEATURE_FTM_TWO_KEY
            LOGD(TAG "[AT]Switch to UART2:\n");
            if (ER_OK!= modem.SetUARTOwner(5))
            return -1 ;
    
            LOGD(TAG "[AT]Reset SIM2:\n");
            if(ER_OK!= modem.QuerySIMStatus(i))
            return -1 ;
    
            LOGD(TAG "[AT]Switch to UART1:\n");
            if (ER_OK!= modem.SetUARTOwner(4))
            return -1 ;
            #endif
        
            bInit = 1;
        }
      
        if(ER_OK!= modem.SetNetworkRegInd(1))
        return -1 ;

        if(isRLTE_VLTESupport()!=0){
            LOGD(TAG "[AT]For iRat");
            if(ER_OK!= modem.EMDSTATUS())
            return -1; 
    
            LOGD(TAG "[AT]Change Modem mode for ECC call");
            if(ER_OK!= modem.ChangeMode(6))
            return -1;
        }
 
        LOGD(TAG "[AT]Turn ON RF:\n");
        if (sim_num == 3) {
            if(ER_OK!= modem.SetMTKRFMode(7))
                return -1 ;
        }
    
        if (sim_num == 4) {
            if(ER_OK!= modem.SetMTKRFMode(15))
            return -1 ;
        }
        if (sim_num == 2) {
          if(ER_OK!= modem.SetMTKRFMode(3))
          return -1 ;
        }

        LOGD(TAG "[AT]Wait for CREG:\n");
        if( -1 == wait_Signal_CREG(40))
        {
             return -1 ;
        }
        LOGD(TAG "Wait for CREG done !") ;
    }else if (sim_num == 1) {
        if (bInit == 0)
        {
            LOGD(TAG "[AT]Reset SIM1:\n");
            if(ER_OK!= modem.QuerySIMStatus(i))
                return -1 ;
                bInit = 1;
        }
          
        if (ER_OK!= modem.SetNetworkRegInd(1))
        return -1 ;
          
        if(isRLTE_VLTESupport()!=0) {
            LOGD(TAG "[AT]For iRat");
            if(ER_OK!= modem.EMDSTATUS())
            return -1;
        }
      
        LOGD(TAG "[AT]Turn ON RF:\n");
        #ifdef MTK_LTE_DC_SUPPORT
            if(ER_OK!= modem.SetMTKRFMode(2))
            return -1 ;
        #else
            if(ER_OK!= modem.SetMTKRFMode(1))
            return -1 ;
        #endif
        LOGD(TAG "[AT]Wait for CREG:\n");
        if(-1 == wait_Signal_CREG(40))
        {
            return -1 ;
        }
        LOGD(TAG "Wait for CREG done !") ;

    }

    return 0;
}

int C2Kmodemsignaltest(Connection& modem)
{
    struct itimerval v;      //set a timer
    memset(&v, 0, sizeof(struct itimerval));
    signal(SIGALRM,deal_URC_VPUP);
    v.it_value.tv_sec=30;
    int i ;
    if(g_Flag_VPUP != 1)
    {
        if(ER_OK!= modem.QueryModemStatus())
        {
            g_Flag_VPUP = 0 ;
            wait_URC(ID_VPUP);
        }
        else
        {
              g_Flag_VPUP = 1 ;
        }
    }
      
    LOGD(TAG "[AT]Reset C2K modem config:\n");
    if(ER_OK!= modem.ResetConfig())//ATZ
        return -1 ;
      
    LOGD(TAG "[AT]Tidy C2K modem response format:\n");
    if(ER_OK!= modem.SetModemFunc(1))//ATE0Q0V1
        return -1 ;
    
    LOGD(TAG "[AT]Check the network:\n");
    if(ER_OK!= modem.SetNetworkRegInd(1))
        return -1 ;
    
    LOGD(TAG "[AT]Turn off phone:\n");
    modem.TurnOffPhone();

    sleep(5);
        
    LOGD(TAG "[AT]For iRat");
    if(ER_OK!= modem.EMDSTATUS())
    return -1;
        
    wait_URC(ID_UIMST);
        
    LOGD(TAG "[AT]Turn on phone:\n");
    if(ER_OK!= modem.TurnOnPhone())
    return -1 ;
    
    wait_URC(ID_VPON);
    
    LOGD(TAG "[AT]Wait for CREG:\n");
    if( -1 == wait_Signal_CREG(40))
    {
        LOGD(TAG "Wait for CREG error!") ;
    }
    else
    {
        LOGD(TAG "Wait for CREG OK !") ;
    }
    LOGD(TAG "Wait for CREG done !") ;
    
    LOGD(TAG "[AT]Detect the siganl power:\n");
    if(ER_OK!= modem.DetectSignal())
    return -1 ;
    
    LOGD(TAG "[AT]Check the SIM status:\n");
    if(ER_OK!= modem.QuerySIMStatus(i))
    return -1 ;
         
    return 0 ;      
}

/********************
       ATD112;
********************/
int dial112(Connection& modem)
{
    LOGD(TAG "%s start\n", __FUNCTION__);
    int flag_ATH = 0 ;
    char epci_data[16] = "4294967295";
    char lte_112_data[4] = "112";
    struct itimerval v;
    memset(&v, 0, sizeof(struct itimerval));
    signal(SIGALRM,deal_URC_ESPEECH_ECPI);
    v.it_value.tv_sec=30;
    
    #ifdef MTK_LTE_DC_SUPPORT
    if (ER_OK!= modem.SetUARTOwner(5))
    return -1 ;
    #endif

    LOGD(TAG "[AT]Dail Up 112:\n");

    flag_ATH = modem.TerminateCall() ;
    LOGD(TAG "flag_ATH = %d",flag_ATH);
    if(!((flag_ATH == ER_OK)||(flag_ATH == ER_UNKNOWN)))
    {
        LOGD(TAG "TerminateCall fail");
        return -1 ;
    }
    else
    {
        LOGD(TAG "TerminateCall Successfully");    
    }

    LOGD(TAG "[AT]Set ECPI:\n");
    if(ER_OK!= modem.SetCallInfo(epci_data))
    return -1 ;

    if(ER_OK!= modem.MakeMOCall(lte_112_data))
    return -1 ;

    if(g_Flag_ESPEECH_ECPI != 1)
    {
        LOGD(TAG "[Dial112]Set Timer!");
        setitimer(ITIMER_REAL,&v,0);
        pthread_cond_wait(&COND_ESPEECH_ECPI,&M_ESPEECH_ECPI);
        LOGD(TAG "[Dial112]Wait cond back!");
        if(g_Flag_ESPEECH_ECPI != 1)
        {
            return -1 ;    
        }
    }
    
    if(ER_OK!= modem.TerminateCall())
    return -1 ;

    #ifdef MTK_LTE_DC_SUPPORT 
    if (ER_OK!= modem.SetUARTOwner(4))
    return -1 ;
    #endif
    
    return 1;

}

int dial112C2K(Connection& modem)
{   
    LOGD(TAG "%s start:\n",__FUNCTION__);
    char c2k_112_data[16] = "112,1";
    struct itimerval v;      //set a tinmer
    memset(&v, 0, sizeof(struct itimerval));
    signal(SIGALRM,deal_URC_CONN);
    v.it_value.tv_sec=30; 
    int flag_C2K_ATH = 0 ;
    int i = 0;
    
    flag_C2K_ATH = modem.HangUpC2K() ;
    
    LOGD(TAG "flag_C2K_ATH = %d",flag_C2K_ATH);
    if(!((flag_C2K_ATH == ER_OK)||(flag_C2K_ATH == ER_UNKNOWN)))
    {
        LOGD(TAG "C2K modem hang up fail");
        return -1 ;    
    }
      
    LOGD(TAG "[AT]Detect SIM status:\n");
    if(ER_OK != modem.QuerySIMStatus(i))
    return -1 ;
      
    LOGD(TAG "[AT]C2K modem dial 112:\n");
    if(ER_OK != modem.C2KCall(c2k_112_data))
    return -1 ;
      
    setitimer(ITIMER_REAL,&v,0);
    pthread_cond_wait(&COND_CONN,&M_CONN);

    if(-1 == g_Flag_CONN)
    {
        return -1 ;
    }
    
    LOGD(TAG "[AT]Dail Up 112:\n");
    if(ER_OK != modem.HangUpC2K())
    return -1 ;

    return 1;
}

void SIGNAL1_Callback(void(*pdata))
{
    SIGNAL_Callback(pdata);
}

void SIGNAL2_Callback(void(*pdata))
{
    SIGNAL_Callback(pdata);
}

void SIGNAL3_Callback(void(*pdata))
{
    SIGNAL_Callback(pdata);
}

void SIGNAL4_Callback(void(*pdata))
{
    SIGNAL_Callback(pdata);
}

void SIGNAL_Callback(void(*pdata))
{
    ATResult *pRet = (ATResult*)pdata ;
    if(pRet != NULL)
    {
        switch(pRet->urcId)
        {
            case ID_EIND:
            if(128 == pRet->resultLst[0].eleLst[1].int_value)
            {
                g_Flag_EIND = 1 ;
                pthread_cond_signal(&COND_EIND);
            }
            break;
            case ID_CREG:
                g_Flag_CREG = 1 ;                 
                pthread_cond_signal(&COND_CREG);  
            break;
            case ID_ECPI: 
                if((1 == pRet->resultLst[0].eleLst[1].int_value)&&((2 == pRet->resultLst[0].eleLst[2].int_value)||\
                (3 == pRet->resultLst[0].eleLst[2].int_value)||(5 == pRet->resultLst[0].eleLst[2].int_value)||\
                (6 == pRet->resultLst[0].eleLst[2].int_value)))
                {
                    g_Flag_ESPEECH_ECPI = 1;
                    pthread_cond_signal(&COND_ESPEECH_ECPI);
                }
                break ;
            case ID_ESPEECH:
                g_Flag_ESPEECH_ECPI = -1 ;
                pthread_cond_signal(&COND_ESPEECH_ECPI);
            break ;
            case ID_VPUP:
                g_Flag_VPUP = 1 ;
                pthread_cond_signal(&COND_VPUP); 
            break ;
            case ID_CONN:
                g_Flag_CONN = 1 ;
                pthread_cond_signal(&COND_CONN); 
            break;
            case ID_VPON:
                g_Flag_VPON = 1;
                pthread_cond_signal(&COND_VPON);
            break;
            case ID_EUSIM:
                g_Flag_EUSIM = 1;
                pthread_cond_signal(&COND_EUSIM);
            break;
            case ID_UIMST:
                g_Flag_UIMST = 1;
                pthread_cond_signal(&COND_UIMST);
            break;
            default:
            break;
        }
    }
}

void deal_URC_ESPEECH_ECPI(int s)
{
    LOGD(TAG "%s start",__FUNCTION__);
    s = 0; //reserved parameter, for resolving warning
    g_Flag_ESPEECH_ECPI = -1 ;
    pthread_cond_signal(&COND_ESPEECH_ECPI);
    LOGD(TAG "%s end",__FUNCTION__);
}

void deal_URC_CREG(int s)
{
    LOGD(TAG "%s start",__FUNCTION__);
    s = 0; //reserved parameter, for resolving warning
    g_Flag_CREG = -1 ;
    pthread_cond_signal(&COND_CREG);
    LOGD(TAG "%s end",__FUNCTION__);
}

void deal_URC_CONN(int s)
{
    LOGD(TAG "%s start",__FUNCTION__);
    s = 0; //reserved parameter, for resolving warning
    g_Flag_CONN = -1 ;
    pthread_cond_signal(&COND_CONN); 
    LOGD(TAG "%s end",__FUNCTION__);       
}

void deal_URC_EIND(int s)
{
    LOGD(TAG "%s start",__FUNCTION__);
    s = 0; //reserved parameter, for resolving warning
    g_Flag_EIND = -1 ;
    pthread_cond_signal(&COND_EIND);
    LOGD(TAG "%s end",__FUNCTION__);
}

void deal_URC_VPUP(int s)
{
    LOGD(TAG "%s start",__FUNCTION__);
    s = 0; //reserved parameter, for resolving warning
    g_Flag_VPUP = -1 ;
    pthread_cond_signal(&COND_VPUP);
    LOGD(TAG "%s end",__FUNCTION__);
}

void deal_URC_VPON(int s)
{
    LOGD(TAG "%s start",__FUNCTION__);
    s = 0; //reserved parameter, for resolving warning
    g_Flag_VPON = -1 ;
    pthread_cond_signal(&COND_VPON);
    LOGD(TAG "%s end",__FUNCTION__);
}

void deal_URC_EUSIM(int s)
{
    LOGD(TAG "%s start",__FUNCTION__);
    s = 0; //reserved parameter, for resolving warning
    g_Flag_EUSIM = -1 ;
    pthread_cond_signal(&COND_EUSIM);
    LOGD(TAG "%s end",__FUNCTION__);
}

void deal_URC_UIMST(int s)
{
    LOGD(TAG "%s start",__FUNCTION__);
    s = 0; //reserved parameter, for resolving warning
    g_Flag_UIMST = -1 ;
    pthread_cond_signal(&COND_UIMST);
    LOGD(TAG "%s end",__FUNCTION__);
}

int wait_Signal_CREG(int time)
{
    int i = 0 ;
    for(i=0 ;i<time ;i++)
    {
        if(1 == g_Flag_CREG)
        {
              LOGD(TAG "Wait CREG come back done");
            return 1 ;
        }
        else
        {
            usleep(500000);
        }    
    }
    return -1 ;
}

int wait_URC(int i)
{
    struct itimerval v;
    memset(&v, 0, sizeof(struct itimerval));
    if(ID_EIND == i)
    {
        v.it_value.tv_sec=10;
        signal(SIGALRM,deal_URC_EIND);
        LOGD(TAG "Setitimer for EIND!");
        setitimer(ITIMER_REAL,&v,0);
        pthread_cond_wait(&COND_EIND,&M_EIND);
        memset(&v, 0, sizeof(struct itimerval));    //\CF\FB\B3\FD\B6\A8?\C6\F7
        LOGD(TAG "EIND COND wait back!");
        if(-1 == g_Flag_EIND)
        {
            return -1 ;
        }
        else
        {
            return 0 ;    
        }
    }
    else if(ID_VPUP == i)
    {
        v.it_value.tv_sec=10;
        signal(SIGALRM,deal_URC_VPUP);
        LOGD(TAG "Setitimer for VPUP!");
        setitimer(ITIMER_REAL,&v,0);
        pthread_cond_wait(&COND_VPUP,&M_VPUP);
        memset(&v, 0, sizeof(struct itimerval));    //\CF\FB\B3\FD\B6\A8?\C6\F7
        LOGD("VPUP COND wait back!");
        if(-1 == g_Flag_VPUP)
        {
            return -1 ;
        }
        else
        {
            return 0 ;    
        }       
    }
    else if (ID_VPON == i)
    {
        v.it_value.tv_sec=10;
        signal(SIGALRM,deal_URC_VPON);
        LOGD(TAG "Settimer for VPON!");
        setitimer(ITIMER_REAL,&v,0);
        pthread_cond_wait(&COND_VPON,&M_VPON);
        memset(&v, 0, sizeof(struct itimerval));    //clear timer
        LOGD("VPON COND wait back!");
        if(-1 == g_Flag_VPON)
        {
            return -1 ;
        }
        else
        {
            return 0 ;    
        }       
    }
    else if (ID_EUSIM == i)
    {
        v.it_value.tv_sec=10;
        signal(SIGALRM,deal_URC_EUSIM);
        LOGD(TAG "Settimer for EUSIM!");
        setitimer(ITIMER_REAL,&v,0);
        pthread_cond_wait(&COND_EUSIM,&M_EUSIM);
        memset(&v, 0, sizeof(struct itimerval));    //clear timer
        LOGD("EUSIM COND wait back!");
        if(-1 == g_Flag_EUSIM)
        {
            return -1 ;
        }
        else
        {
            return 0 ;    
        }       
    }
    else if (ID_UIMST == i)
    {
        v.it_value.tv_sec=10;
        signal(SIGALRM,deal_URC_UIMST);
        LOGD(TAG "Settimer for UIMST!");
        setitimer(ITIMER_REAL,&v,0);
        pthread_cond_wait(&COND_UIMST,&M_UIMST);
        memset(&v, 0, sizeof(struct itimerval));    //clear timer
        LOGD("UIMST COND wait back!");
        if(-1 == g_Flag_UIMST)
        {
            return -1 ;
        }
        else
        {
            return 0 ;    
        }       
    }
    return -1 ;     
}

int isC2kSupport()
{
    int supportC2k = 0;
    char property_value[PROPERTY_VALUE_MAX] = {0};
    //default set to false
    property_get("ro.vendor.mtk_ps1_rat", property_value, "");
    if(strstr(property_value, "C") != NULL)
        supportC2k = 1;
    return supportC2k;
}

int isECCCI_C2KSupport()
{
    int supportECCCI_C2K = 0;
    char property_value[PROPERTY_VALUE_MAX] = {0};
    //default set to false
    property_get("ro.vendor.mtk_eccci_c2k", property_value, "0");
    supportECCCI_C2K = atoi(property_value);
    return supportECCCI_C2K;
}

int isMDENSupport(int index)
{
    int supportMD = 0;
    char property_value[PROPERTY_VALUE_MAX] = {0};
    //default set to false
    if(1 == index){
        property_get("ro.vendor.mtk_md1_support", property_value, "0");
    }
    else if(2 == index){
        property_get("ro.vendor.mtk_md2_support", property_value, "0");
    }
    else if(3 == index){
        property_get("ro.vendor.mtk_md3_support", property_value, "0");
    }
    else if(5 == index){
        property_get("ro.vendor.mtk_md5_support", property_value, "0");
    }
    
    supportMD = atoi(property_value);
    return supportMD;
}

int isRLTE_VLTESupport()
{
    int supportRLTE_VLTE = 0;
    char property_value[PROPERTY_VALUE_MAX] = {0};
    //default set to false
    property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    supportRLTE_VLTE = atoi(property_value);
    return supportRLTE_VLTE;
}
