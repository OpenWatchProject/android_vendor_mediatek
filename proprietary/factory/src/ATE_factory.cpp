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
#include <cutils/properties.h>
#include "me_connection.h"
#include <pthread.h>
#include "common.h"
#include "ftm.h"
#include "miniui.h"
#include "utils.h"
#include "item.h"
#include "hardware/ccci_intf.h" 
#ifdef Factory_C2K_SUPPORT  
#ifndef MTK_ECCCI_C2K                                                                              
#include <c2kutils.h>
#endif
#endif
#include "at_command.h"

#include <dirent.h>
#include <signal.h>
#include <termios.h> 
#include "libnvram.h"

#include "CFG_file_lid.h"
#include "CFG_PRODUCT_INFO_File.h"
#include "Custom_NvRam_LID.h"
#include "CFG_Wifi_File.h"
#include "CFG_BT_File.h"

#define TAG "[ATE Factory Mode] "

#define BUF_SIZE 128
int read_ccci=0;
int sync_start=0;

extern Connection modem[5] ;

#define HALT_INTERVAL 50000
#define MAX_MODEM_INDEX 4

pthread_t g_AP_UART_USB_RX;
pthread_t g_AP_CCCI_RX;
int g_mdFlag=1;  
int g_fd_uart = -1; 
int g_hUsbComPort = -1;
int g_thread_flag = 0;

#define CCCI_IOC_MAGIC 'C'
#define CCCI_IOC_ENTER_DEEP_FLIGHT _IO(CCCI_IOC_MAGIC, 14) //CCI will not kill muxd/rild
#define CCCI_IOC_LEAVE_DEEP_FLIGHT _IO(CCCI_IOC_MAGIC, 15) //CCI will kill muxd/rild

#define rmmi_skip_spaces(source_string_ptr)                                  \
      while( source_string_ptr->string_ptr[ source_string_ptr->index ]       \
                                 == RMMI_SPACE )                             \
      {                                                                      \
        source_string_ptr->index++;                                          \
      }

#define RMMI_IS_LOWER( alpha_char )   \
  ( ( (alpha_char >= rmmi_char_a) && (alpha_char <= rmmi_char_z) ) ?  1 : 0 )

#define RMMI_IS_UPPER( alpha_char )   \
   ( ( (alpha_char >= RMMI_CHAR_A) && (alpha_char <= RMMI_CHAR_Z) ) ? 1 : 0 )

CMD_HDLR g_cmd_handler[] = 
						{ 
							{0, "AT+EABT", 0, 0, (HANDLER)rmmi_eabt_hdlr}
							,{1, "AT+EAWIFI", 0, 0, (HANDLER)rmmi_eawifi_hdlr}
							,{2, "AT+EANVBK", 0, 0, (HANDLER)rmmi_eanvbk_hdlr}
							,{3, "AT+EAMDIMG", 0, 0, (HANDLER)rmmi_eamdimg_hdlr}
							,{INVALID_ENUM, "AT+END", 0, 0, NULL}
						};

WIFI_CFG_PARAM_STRUCT g_wifi_nvram;
ap_nvram_btradio_mt6610_struct g_bt_nvram;
F_ID nvram_fd;


extern int open_usb_port(int uart_id, int baudrate, int length, char parity_c, int stopbits);

void open_ate_modem();

int wait_SDIO_ready();

int AT_Pre_Process (char *buf_cmd)
{

static int count = 0;
	  
	  if(!strncmp(buf_cmd, "AT+SWITCH",9))
	  {
		  count++;
		  if(count%2 != 0)
		  {
				LOGD(TAG "AT_Pre_Process count = %d",count);
			#ifdef MTK_DT_SUPPORT
			  g_mdFlag=2;
			  
		  #ifnde fEVDO_FTM_DT_VIA_SUPPORT
		  if(isC2kSupport() == 1){
			  g_mdFlag=3;
			}  
		  #else
			  g_mdFlag=1;
		  #endif
			  LOGD(TAG "g_mdFlag = %d",g_mdFlag);
		  }
		  else
		  {
			  g_mdFlag=1; 
			  LOGD(TAG "g_mdFlag = %d",g_mdFlag);
		  }
		  return 1;
	  }
	
	if(!strncmp(buf_cmd, "AT+START", 8))
	{
		 LOGD(TAG "receive Tool sync command\n"); 
		 read_ccci=1;
		 return 2;
	}
	return 0;

}

void * AP_UART_USB_RX (void* lpParameter)
{
    char buf_cmd [BUF_SIZE] = {0};
    int len = 0;
    int wr_len = 0;
    char result[64] = {0};
    int at_pre_ret = 0;
    char pass[8] = "PASS\r\n";
    char ok_str[8] = "OK\r\n";
	if (lpParameter == NULL) {
		// reserved parameter, for resolving warning
        LOGD(TAG "The parameter is NULL\n");
	}

    LOGD(TAG "%s start \n",__FUNCTION__);
    cmd_handler_init ();

    for (;;)
    {
//        LOGD(TAG "Enter for() cycle");
	    len = read_a_line(g_hUsbComPort, buf_cmd, BUF_SIZE);            
//	    LOGD(TAG "len = %d\n",len);    
	    if (len>0)
	    {
		    buf_cmd[len] = '\0';
        	LOGD(TAG "AP_UART_USB_RX Command: %s, Len: %d\n" ,buf_cmd, len);
	  	    at_pre_ret = AT_Pre_Process (buf_cmd);
	  	    LOGD(TAG "----------->at_pre_ret = %d",at_pre_ret);
            LOGD("buf_cmd:%s\n", buf_cmd);
			if (2 == at_pre_ret)
            {
            	if (0 == sync_start)
            	{
	                wr_len = write_chars (g_hUsbComPort, pass, strlen(pass));
				    if (wr_len != (int)strlen(pass))
				    {
						LOGE(TAG "AP_CCCI_RX: write PASS fail\n");
						sync_start=0;
					}
					else
					{
						sync_start=1;
					}
            	}
                continue;
            }
			else if (1 == at_pre_ret)
            {
             	wr_len = write_chars (g_hUsbComPort, ok_str, strlen(ok_str));
                continue;
            }

            CMD_OWENR_ENUM owner = rmmi_cmd_processor((unsigned char *)buf_cmd, result);
            if (owner == CMD_OWENR_AP)
            {
            	LOGD(TAG "The data write to AP from PC is %s\n", result);
                wr_len = write_chars (g_hUsbComPort, result, strlen(result));
			    if (wr_len != (int)strlen(result))
		            LOGE(TAG "AP_CCCI_RX: wr_len != rd_len\n");
            }
            else
            {
             	LOGD(TAG "The data write to MD from PC is %s\n", buf_cmd);
             	modem[g_mdFlag-1].Write_ATEChannel(buf_cmd,strlen(buf_cmd));

		    }
 	    }
    }
 	  return NULL ;
}

void New_Thread ()
{
	LOGD(TAG "Enter New_Thread() \n");
	if(pthread_create (&g_AP_UART_USB_RX, NULL, AP_UART_USB_RX, NULL) != 0)
	{
		LOGD (TAG "main:Create AP_UART_USB_RX thread failed");
		return;
	}
	LOGD(TAG "Exit New_Thread() \n");
}

int Wait_Thread (pthread_t arg)
{   /* exit thread by pthread_kill -> pthread_join*/
    int err;
    if ((err = pthread_kill(arg, SIGUSR1)))
        return err;

    if ((err = pthread_join(arg, NULL)))
        return err;
    return 0;
}

int Free_Thread ()
{   /* exit thread by pthread_kill -> pthread_join*/
    int err = 0;
    if ( 0 != (err = Wait_Thread(g_AP_UART_USB_RX)) )
        return err;

    if ( 0 != (err = Wait_Thread(g_AP_CCCI_RX)) )
        return err;

    return 0;
}

int com_init_for_ate_factory_mode_usb_uart()
{
     g_fd_uart = open_uart_port(UART_PORT1, 115200, 8, 'N', 1);
	   if(g_fd_uart == -1) 
     {
         LOGE(TAG "Open uart port %d fail\r\n" ,UART_PORT1);
     }
     else
     {
         LOGD(TAG "Open uart port %d success------%d\r\n" ,UART_PORT1,g_fd_uart);
     }
  	
	 g_hUsbComPort = open_usb_port(UART_PORT1, 115200, 8, 'N', 1);
	 if (g_hUsbComPort == -1)
	 {
	     LOGE(TAG "Open usb fail\r\n");
	 }
	 else
	 {
		 LOGD(TAG "Open usb %d success------%d\r\n",UART_PORT1,g_hUsbComPort);
	 }
	 return 0 ;
}

int com_deinit_for_ate_factory_mode_usb_uart()
{
    if (g_fd_uart!= -1)
    {
        close(g_fd_uart);
		    g_fd_uart = -1;
    }
    if (g_hUsbComPort!= -1)
    {
        close(g_hUsbComPort);
		    g_hUsbComPort = -1;
    }
    return 0;
}

// The below is for ATE signal test.
void ate_signal(void)
{
    struct itemview ate;
    text_t info;
    char buf[100] = {0};
    char ccci_path[MAX_MODEM_INDEX][32];
    memset(&ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    char temp_ccci_path[MAX_MODEM_INDEX][32];
    memset(&temp_ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    
    ui_init_itemview(&ate);
    init_text(&info, buf, COLOR_YELLOW);
    ate.set_text(&ate, &info);
    sprintf(buf, "%s", "ATE Signaling Test\nEmergency call is not started\n");
    ate.redraw(&ate);

    LOGD(TAG "Entry ate_signal\n");
    
    com_init_for_ate_factory_mode_usb_uart();	
	New_Thread ();
	while(0 == read_ccci)
	{
	   usleep(HALT_INTERVAL);
	}
	open_ate_modem();
	
	sleep(7);
	
    LOGD(TAG "Com init start\n");
	
    while(1) 
	{
        usleep(HALT_INTERVAL*20);
    }
	
	Free_Thread ();

    com_deinit_for_ate_factory_mode_usb_uart();

}
//#endif

void open_ate_modem()
{
    int modem_number = 0;
    int i = 0;
    int j = 0;
    char ccci_path[MAX_MODEM_INDEX][32];
    memset(&ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    char temp_ccci_path[MAX_MODEM_INDEX][32];
    memset(&temp_ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    modem_number = get_md_count();

    for(i = 0; i < MAX_MODEM_INDEX; i++)
    {
        if(1 == get_ccci_path(i,temp_ccci_path[i]))
        {
            strncpy(ccci_path[j],temp_ccci_path[i],strlen(temp_ccci_path[i]));
            j++;
        }
    }
    
    for (i = 0; i<modem_number; i++)
    {
    	LOGD(TAG "Open ATE channel start: \n");
        if(1 == modem[i].Open_ATEChannel(ccci_path[i],i+1,g_ATE_Callback))
        {
            LOGD(TAG "Open ATE channel successfully: \n"); 
        }
        else
        {
        	LOGD(TAG "Open ATE channel falied: \n");
        }      	
    }
    
    #ifndef EVDO_FTM_DT_VIA_SUPPORT
		#ifdef Factory_C2K_SUPPORT 
    SDIO_Ready_flag = wait_SDIO_ready();
    if(1 == SDIO_Ready_flag)
    {
    	  #ifdef MTK_ECCCI_C2K 
          asciDevice = (char*)malloc(sizeof(char)*32);
          strncpy(asciDevice,ccci_path[modem_number],strlen(ccci_path[modem_number]));
          LOGD(TAG "ECCI support C2K modem path is  = %s\n",asciDevice);
        #else
          asciDevice = viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_AT);
          LOGD(TAG "Not ECCI support C2K modem path is  = %s\n",asciDevice);
        #endif
        if(1 == modem[modem_number+1].Open_ATEChannel(asciDevice,modem_number+1,g_ATE_Callback))
        {
            LOGD(TAG "Open ATE channel successfully for C2K");
        }
        else
        {
        	  LOGD(TAG "Open ATE channel failed for C2K");
        }
        
        #ifdef MTK_ECCCI_C2K
        free(asciDevice);
      	#endif
    }
    #endif
    #endif
}

/*****************************************************************************
 * FUNCTION
 *  rmmi_int_validator_ext
 * DESCRIPTION
 *  
 * PARAMETERS
 *  error_cause             [?]         
 *  source_string_ptr       [?]         
 *  delimiter               [IN]        
 * RETURNS
 *  
 *****************************************************************************/
unsigned int rmmi_int_validator_ext(rmmi_validator_cause_enum *error_cause, rmmi_string_struct *source_string_ptr, unsigned char delimiter)
{
    unsigned int ret_val = RMMI_VALIDATOR_ERROR;
    unsigned int value = 0;
    unsigned int length;
    bool error_flag = false;
    bool some_char_found = false;

    //kal_trace(TRACE_FUNC, FUNC_RMMI_INT_VALIDATOR_ENTRY);
    length = strlen((char*)source_string_ptr->string_ptr);

    /* If there are some leading white spaces, ignore them */
    rmmi_skip_spaces(source_string_ptr);

    /*
     * we have to initial the error so that we can using again and
     * again even if any error occur. so we dont have to init before
     * enter this function
     */
    *error_cause = RMMI_PARSE_OK;

    /*
     * Start checking for the integer, till the delimiter which may
     * * be a comma, a dot etc.
     */

    while ((source_string_ptr->string_ptr[source_string_ptr->index]
            != delimiter)
           &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != S3) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_END_OF_STRING_CHAR) && (source_string_ptr->index < length))
    {
        /* It means we found something between two commas(,)  */
        some_char_found = true;

        /*
         * check whether the character is in 0 - 9 range. If so,
         * * store corresponding integer value for that character
         */
        if ((source_string_ptr->string_ptr[source_string_ptr->index]
             >= RMMI_CHAR_0) && (source_string_ptr->string_ptr[source_string_ptr->index] <= RMMI_CHAR_9))
        {
            value = value * 10 + (source_string_ptr->string_ptr[source_string_ptr->index] - RMMI_CHAR_0);
        }
        else    /* out of range, return immediately */
        {
            error_flag = true;
            break;
        }
        /* If the character is a valid part of integer, then continue */
        source_string_ptr->index++;
    }   /* end of the while loop */

    if (error_flag == true)
    {
        /*
         * Value is not in the valid range. It can also be due to
         * * white space in between two digits, because such white
         * * spaces are not allowed
         */
        /* mark for solve correct input but incorrect end for 1,2,2, */
        /* rmmi_result_code_fmttr (  RMMI_RCODE_ERROR,
           INVALID_CHARACTERS_IN_TEXT_ERRSTRING_ERR ); */
        ret_val = RMMI_VALIDATOR_ERROR;
        *error_cause = RMMI_PARSE_ERROR;
    }
    else if (some_char_found == false)
    {
        /* Nothing is present before the delimiter */
        ret_val = RMMI_VALIDATOR_ERROR;
        *error_cause =  RMMI_PARSE_NOT_FOUND;

        /*
         * Increment the string sliding index to point to the next
         * * character after delimiter, i.e. the next field in the
         * * command line
         */
        source_string_ptr->index++;
    }
    /*
     * If some thing is present and check for the valid range as
     * * specified by the calling function
     */
    else
    {
        ret_val = value;
        /*
         * Increment the string sliding index to point to the next
         * * character after delimiter, i.e. the next field in the
         * * command line
         */
        if (source_string_ptr->string_ptr[source_string_ptr->index] == delimiter)
        {
            source_string_ptr->index++;
            rmmi_skip_spaces(source_string_ptr);
            if (source_string_ptr->string_ptr[source_string_ptr->index] == S3&&
		  source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_END_OF_STRING_CHAR)
            {
                ret_val = RMMI_VALIDATOR_ERROR;
                *error_cause =  RMMI_PARSE_ERROR;
            }
        }
        else
        {
            source_string_ptr->index++;
        }
    }
    return ret_val;
}

void get_len(char *str, int *length)
{
    int i = 0;
    while(str[i] != RMMI_END_OF_STRING_CHAR)
    {
        i++;
    }
    *length = i;
    return;
}

static int convStrtoHex(char*  szStr, char* pbOutput, int dwMaxOutputLen, int*  pdwOutputLen){
    LOGD("Entry %s\n", __FUNCTION__);

    int   dwStrLen;        
    int   i = 0;
    unsigned char ucValue = 0;
    LOGD("before strlen,dwStrLen\n");
    while(szStr[i] != '\0')
    {
        LOGD("szStr[%d]:%c", i, szStr[i]);
        i++;
    }

    LOGD("after while loop\n");
    dwStrLen = strlen(szStr);
//	dwStrLen = i;
	LOGD("after strlen, dwStrLen = %d\n", dwStrLen);
//    LOGD("after strlen,dwStrLen %d\n", dwStrLen);

    if(dwMaxOutputLen < dwStrLen/2){
        return -1;
    }
    i = 0;
    for (i = 0; i < dwStrLen; i ++){
        
    LOGD("in for loop %c\n", szStr[i]);
        switch(szStr[i]){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                ucValue = (ucValue * 16) + (szStr[i] -  '0');
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                ucValue = (ucValue * 16) + (szStr[i] -  'a' + 10);
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                ucValue = (ucValue * 16) + (szStr[i] -  'A' + 10);
                break;
            default:
                return -1;
                break;
        }

        if(i & 0x01){
            pbOutput[i/2] = ucValue;
            LOGD("int pbOutput:%d, ucValue:%d\n", pbOutput[i/2], ucValue);
            LOGD("int pbOutput:%02x, ucValue:%02x\n", pbOutput[i/2], ucValue);
            ucValue = 0;
        }
    }

    *pdwOutputLen = i/2;
    LOGD("Leave %s\n", __FUNCTION__);  

    return 0;
}

void get_write_value(rmmi_string_struct *cmd, char *write_value)
{
    LOGD("Entry %s\n", __FUNCTION__);
    int i = 0;
    while(cmd->string_ptr[cmd->index] != RMMI_DOUBLE_QUOTE)
    {
        cmd->index++;
    }
    cmd->index++;
    while(cmd->string_ptr[cmd->index] != RMMI_DOUBLE_QUOTE)
    {
        write_value[i] = cmd->string_ptr[cmd->index];
        i++;
        cmd->index++;
    }
    LOGD("Leave %s\n", __FUNCTION__);
}

void rmmi_eamdimg_hdlr(rmmi_string_struct *cmd, char *md_idx)
{
    int modem_type = 0;
    int mdimg_type[16] = {0};
    int i = 0,j = 0;
    char ccci_path[MAX_MODEM_INDEX][32];
    memset(&ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    char temp_ccci_path[MAX_MODEM_INDEX][32];
    memset(&temp_ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    int md_num = 0;
    if (cmd == NULL) 
    { 
        LOGD(TAG "The parameter cmd is NULL\n");
        return;
    }

    if(md_idx == NULL)// reserved parameter, for resolving warning
    { 
        LOGD(TAG "The parameter md_idx is NULL\n");
    }

    if((cmd->cmd_owner != CMD_OWENR_AP) || (cmd->cmd_class != RMMI_EXTENDED_CMD))
    {
        sprintf(cmd->result, "%s", return_err);
        return;
    }
    for(i = 0; i < MAX_MODEM_INDEX; i++)
    {
        if(1 == get_ccci_path(i,temp_ccci_path[i]))
        {
            strncpy(ccci_path[j],temp_ccci_path[i],strlen(temp_ccci_path[i]));
            j++;
        }
    }
    memset(cmd->result, 0, 64);
    md_num = get_md_count();
    LOGD(TAG "cmd->cmd_mode=%d, md_flag=%d, cmd->value=%d\n", cmd->cmd_mode, g_mdFlag, cmd->value);
    LOGD(TAG "RMMI_SET_OR_EXECUTE_MODE=%d, RMMI_READ_MODE=%d\n", RMMI_SET_OR_EXECUTE_MODE, RMMI_READ_MODE);
    LOGD(TAG "ccci_handle[%d-1]=%d\n", g_mdFlag, modem[g_mdFlag-1].GetHandle());
    switch(cmd->cmd_mode)
    {
        case RMMI_SET_OR_EXECUTE_MODE://set modem image
            if (0 == ioctl( modem[g_mdFlag-1].GetHandle(), CCCI_IOC_RELOAD_MD_TYPE, &(cmd->value)))
            {
				LOGD(TAG "[Factory mode] set ioctl to reset md");
                if(0 == ioctl( modem[g_mdFlag-1].GetHandle(), CCCI_IOC_MD_RESET))
                {

					LOGD(TAG "[Factory mode] To kill ATE thread (main)");
					LOGD(TAG "[Factory mode] To close CCCI");
					//modem[g_mdFlag-1].Close_ATEChannel();
					for(i = 0; i < md_num; i++)
					{
						modem[i].Close_ATEChannel();
					}
					
					#ifndef EVDO_FTM_DT_VIA_SUPPORT
		       if(isC2kSupport() == 1){
						modem[2].Close_ATEChannel();
					}
					#endif
					LOGD(TAG "[Factory mode] To sleep 5 sec");
                    sleep(3);
					LOGD(TAG "[Factory mode] To open CCCI");
                    //modem[g_mdFlag-1].Open_ATEChannel(ccci_path[g_mdFlag-1],i,g_ATE_Callback);
                    open_ate_modem();
					LOGD(TAG "[Factory mode] To wait AT ready");
					wait_URC(ID_EIND);
                    sprintf(cmd->result, "\r\n\r\n");
                    LOGD(TAG "[Factory mode] Set modem image successfully!\n");
                }
                else
                {
                    sprintf(cmd->result, "\r\nset failed!\r\n");
                    LOGD(TAG "[Factory mode] Set modem image failed in second ioctl!\n");
                }
            }
            else
            {
                sprintf(cmd->result, "\r\nset failed!\r\n");
                LOGD(TAG "[Factory mode] Set modem image failed in first ioctl!\n");
            }
            break;
        case RMMI_READ_MODE://get modem image

            if (0 == cmd->value)//get current modem image index
            {
                if (0 == ioctl(modem[g_mdFlag-1].GetHandle(), CCCI_IOC_GET_MD_TYPE, &modem_type))
                {
                    sprintf(cmd->result, "\r\n%d\r\n", modem_type);
                    LOGD(TAG "current modem_type=%d\n", modem_type);
                }
                else
                {
                    sprintf(cmd->result, "\r\nget failed!\r\n");
                    LOGD(TAG "ioctl fail in RMMI_READ_MODE and cmd->value=0\n");
                }
            }
            else if (1 == cmd->value)//get all modem images index
            {
                if (0 == ioctl(modem[g_mdFlag-1].GetHandle(), CCCI_IOC_GET_MD_IMG_EXIST, &mdimg_type))
                {
                    for(i = 0; i < 16; i++)
                    {
                        if(mdimg_type[i] != 0)
                        {
                            char temp_result[256] = {0};
                            sprintf(temp_result, "\r\n%s%d,", cmd->result, mdimg_type[i]);
                            sprintf(cmd->result, "%s", temp_result);
                        }
                        LOGD(TAG "mdimg_type[%d] %d", i, mdimg_type[i]);
                    }
                }
                else
                {
                    sprintf(cmd->result, "\r\nget failed!\r\n");
                    LOGD(TAG "ioctl fail in RMMI_READ_MODE and cmd->value=1\n");
                }
            }
        break;
        default:
        break;
    }
}


void rmmi_eabt_hdlr (rmmi_string_struct* cmd, char *addr)
{
    char output[bt_length] = {0};
    int rec_size = 0;
    int rec_num = 0;
	unsigned char w_bt[bt_length];
    int length;
    int ret, i = 0;
    char value[13] = {0};
    memset(value, 0, sizeof(value));

    if (cmd == NULL) 
    { 
        LOGD(TAG "The parameter cmd is NULL\n");
        return;
    }

    if(addr == NULL)// reserved parameter, for resolving warning
    { 
        LOGD(TAG "The parameter addr is NULL\n");
    }

    if (cmd->cmd_owner != CMD_OWENR_AP || cmd->cmd_class != RMMI_EXTENDED_CMD)
    {
        sprintf(cmd->result, "%s", return_err);
		return;
    }
	switch (cmd->cmd_mode)
	{
	    case   RMMI_SET_OR_EXECUTE_MODE: //AT+EABT=1\r
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_SET_OR_EXECUTE_MODE,cmd:%s,%c\n", cmd->string_ptr, cmd->string_ptr[cmd->index+2]);
            nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISREAD);
  			LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   			if(1 != rec_num)
   			{
   				LOGD("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
   			}
   			if(sizeof(g_bt_nvram) != rec_size)
   			{
   				LOGD("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
   			}
   			memset(&g_bt_nvram,0,rec_num*rec_size);
   			ret = read(nvram_fd.iFileDesc, &g_bt_nvram, rec_num*rec_size);
   			if(-1 == ret||rec_num*rec_size != ret)
   			{
   				LOGD("error:read bt addr fail!/n");
                sprintf(cmd->result, "%s", return_err);
   			}
   			LOGD("read pre bt addr:%02x%02x%02x%02x%02x%02x\n", 
                   g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5] 
            );
   			NVM_CloseFileDesc(nvram_fd);
   			nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISWRITE);
   			LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   			if(1 != rec_num)
   			{
   				LOGD("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
   			}
   			if(sizeof(g_bt_nvram) != rec_size)
   			{
   				LOGD("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
   			}
   			memset(g_bt_nvram.addr,0,bt_length);
   			memset(w_bt,0,bt_length);
            get_write_value(cmd, value);
            length = strlen(value);
    	    if(length != 12)
		    {
			    LOGD("error:bt address length is not right!\n");
                sprintf(cmd->result, "%s", return_err);
		    }
		    ret = convStrtoHex(value,output,bt_length,&length);
		    if(-1 == ret)
		    {
			    LOGD("error:convert bt address to hex fail\n");
                sprintf(cmd->result, "%s", return_err);
		    }
            else
            {
                LOGD("BT Address:%s\n", output);
            }
   			for(i=0;i<bt_length;i++)
   			{	
   				g_bt_nvram.addr[i] = output[i];
   			}
   			LOGD("write bt addr:%02x%02x%02x%02x%02x%02x, value:%02x%02x%02x%02x%02x%02x\n", 
                    g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5],
                    output[0], output[1], output[2], output[3], output[4], output[5]
                    );
   			ret = write(nvram_fd.iFileDesc, &g_bt_nvram , rec_num*rec_size);
   			if(-1 == ret||rec_num*rec_size != ret)
   			{
   				LOGD("error:write wifi addr fail!\n");
                sprintf(cmd->result, "%s", return_err);
   			}
   			NVM_CloseFileDesc(nvram_fd);
   			LOGD("write bt addr success!\n");
   			if(FileOp_BackupToBinRegion_All())
   			{
   				LOGD("backup nvram data to nvram binregion success!\n");
                sprintf(cmd->result, "%s", return_ok);
   			}
   			else
   			{
   				LOGD("error:backup nvram data to nvram binregion fail!\n");
                sprintf(cmd->result, "%s", return_err);
   			}
   			sync();
			break;
    	case RMMI_READ_MODE:                    //AT+EABT?\r
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_READ_MODE");
             nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISREAD);
  			LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   			if(1 != rec_num)
   			{
   				LOGD("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
   			}
   			if(sizeof(g_bt_nvram) != rec_size)
   			{
   				LOGD("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
   			}
   			memset(&g_bt_nvram,0,rec_num*rec_size);
   			ret = read(nvram_fd.iFileDesc, &g_bt_nvram, rec_num*rec_size);
   			if(-1 == ret||rec_num*rec_size != ret)
   			{
   				LOGD("error:read bt addr fail!/n");
                sprintf(cmd->result, "%s", return_err);
   			}
   			LOGD("read pre bt addr:%02x%02x%02x%02x%02x%02x\n", 
                   g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5] 
            );
//            memcpy(addr, g_bt_nvram.addr, sizeof(g_bt_nvram.addr));
            sprintf(cmd->result, "%s%02x%02x%02x%02x%02x%02x%s%s", "\n\r+EABT:\"", g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5], "\"\n\r", return_ok);
   			NVM_CloseFileDesc(nvram_fd);
			break;
    	case RMMI_TEST_MODE:                     //AT+EABT=?\r
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_TEST_MODE");
            sprintf(cmd->result, "%s%s", "\r\n+EABT:(0,1)(1)", return_ok);
			break;
    	case RMMI_ACTIVE_MODE:                 //AT+EABT\r
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_ACTIVE_MODE");
            sprintf(cmd->result, "%s", return_err);
			break;
				
    	case RMMI_WRONG_MODE:
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_WRONG_MODE");
            sprintf(cmd->result, "%s", return_err);
            break;
		default:
			return;
	}
}
void rmmi_eawifi_hdlr (rmmi_string_struct* cmd, char *addr)
{
    char output[wifi_length] = {0};
    int rec_size = 0;
    int rec_num = 0;
    int ret, length = 0, i = 0;
    char value[13] = {0};
    if (cmd == NULL) 
    { 
        LOGD(TAG "The parameter cmd is NULL\n");
        return;
    }

    if(addr == NULL)// reserved parameter, for resolving warning
    { 
        LOGD(TAG "The parameter addr is NULL\n");
    }

    if (cmd->cmd_owner != CMD_OWENR_AP || cmd->cmd_class != RMMI_EXTENDED_CMD)
		return;
	switch (cmd->cmd_mode)
	{
   		case   RMMI_SET_OR_EXECUTE_MODE: //AT+EAWIFI=1\r
   		    LOGD(TAG "rmmi_eawifi_hdlr:RMMI_SET_OR_EXECUTE_MODE");
            nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISREAD);
            printf("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
    		if(1 != rec_num)
    		{
    			printf("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
    		}
    		if(sizeof(WIFI_CFG_PARAM_STRUCT) != rec_size)
    		{
    			printf("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
    		}
    		memset(&g_wifi_nvram,0,rec_num*rec_size);
    		ret = read(nvram_fd.iFileDesc, &g_wifi_nvram, rec_num*rec_size);
    		if(-1 == ret||rec_num*rec_size != ret)
    		{
    			printf("error:read wifi mac addr fail!/n");
                sprintf(cmd->result, "%s", return_err);
    		}
    		printf("read wifi addr:%02x%02x%02x%02x%02x%02x\n", 
                    g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], g_wifi_nvram.aucMacAddress[4], 
    		g_wifi_nvram.aucMacAddress[5]);
    		NVM_CloseFileDesc(nvram_fd);

            nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISWRITE);
   			LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   			if(1 != rec_num)
   			{
   				LOGD("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
   			}
   			if(sizeof(g_wifi_nvram) != rec_size)
   			{
   				LOGD("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
   			}
   			memset(g_wifi_nvram.aucMacAddress,0,bt_length);
//   			memset(w_bt,0,bt_length);
            get_write_value(cmd, value);
            length = strlen(value);
    	    if(length != 12)
		    {
			    LOGD("error:bt address length is not right!\n");
                sprintf(cmd->result, "%s", return_err);
		    }
		    ret = convStrtoHex(value,output,wifi_length,&length);
		    if(-1 == ret)
		    {
			    LOGD("error:convert wifi address to hex fail\n");
                sprintf(cmd->result, "%s", return_err);
		    }
            else
            {
                LOGD("WIFI Address:%s\n", output);
            }
   			for(i=0;i<bt_length;i++)
   			{	
   				g_wifi_nvram.aucMacAddress[i] = output[i];
   			}
   			LOGD("write wifi addr:%02x%02x%02x%02x%02x%02x, value:%02x%02x%02x%02x%02x%02x\n", 
                    g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], 
                    g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], 
                    g_wifi_nvram.aucMacAddress[4], g_wifi_nvram.aucMacAddress[5],
                    output[0], output[1], output[2], output[3], output[4], output[5]
                    );
   			ret = write(nvram_fd.iFileDesc, &g_wifi_nvram , rec_num*rec_size);
   			if(-1 == ret||rec_num*rec_size != ret)
   			{
   				LOGD("error:write wifi addr fail!\n");
                sprintf(cmd->result, "%s", return_err);
   			}
   			NVM_CloseFileDesc(nvram_fd);
   			LOGD("write wifi addr success!\n");
   			if(FileOp_BackupToBinRegion_All())
   			{
   				LOGD("backup nvram data to nvram binregion success!\n");
                sprintf(cmd->result, "%s", return_ok);
   			} 
   			else
   			{
   				LOGD("error:backup nvram data to nvram binregion fail!\n");
                sprintf(cmd->result, "%s", return_err);
   			}
   			sync();
			break;
    		case   RMMI_READ_MODE:                    //AT+EAWIFI?\r
   		        LOGD(TAG "rmmi_eawifi_hdlr:RMMI_READ_MODE");
    			nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISREAD);
    			printf("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
    			if(1 != rec_num)
    			{
    				LOGD("error:unexpected record num %d\n",rec_num);
                    sprintf(cmd->result, "%s", return_err);
    			}
    			if(sizeof(WIFI_CFG_PARAM_STRUCT) != rec_size)
    			{
    				LOGD("error:unexpected record size %d\n",rec_size);
                    sprintf(cmd->result, "%s", return_err);
    			}
    			memset(&g_wifi_nvram,0,rec_num*rec_size);
    			ret = read(nvram_fd.iFileDesc, &g_wifi_nvram, rec_num*rec_size);
    			if(-1 == ret||rec_num*rec_size != ret)
    			{
    				LOGD("error:read wifi mac addr fail!/n");
                    sprintf(cmd->result, "%s", return_err);
    			}
    			LOGD("read wifi addr:%02x%02x%02x%02x%02x%02x\n", 
                    g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], g_wifi_nvram.aucMacAddress[4], 
    			g_wifi_nvram.aucMacAddress[5]);
//                memcpy(addr, g_wifi_nvram.aucMacAddress, sizeof(g_wifi_nvram.aucMacAddress));
                sprintf(cmd->result, "%s%02x%02x%02x%02x%02x%02x%s%s", "\r\n+EAWIFI:\"", g_wifi_nvram.aucMacAddress[0], 
                g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], 
                g_wifi_nvram.aucMacAddress[4], g_wifi_nvram.aucMacAddress[5], "\"\r\n", return_ok);
    			NVM_CloseFileDesc(nvram_fd);
			break;
    		case   RMMI_TEST_MODE:                     //AT+EAWIFI=?\r
   		        LOGD(TAG "rmmi_eawifi_hdlr:RMMI_TEST_MODE");
                sprintf(cmd->result, "%s%s", "\r\n+EAWIFI:(0,1)(1)", return_ok);
			break;
    		case   RMMI_ACTIVE_MODE:                 //AT+EAWIFI\r
   		        LOGD(TAG "rmmi_eawifi_hdlr:RMMI_ACTIVE_MODE");
                sprintf(cmd->result, "%s", return_err);
			break;
				
    		case   RMMI_WRONG_MODE:
   		        LOGD(TAG "rmmi_eawifi_hdlr:RMMI_WRONG_MODE");
                sprintf(cmd->result, "%s", return_err);
                break;
		default:
            
                sprintf(cmd->result, "%s", return_err);
			    return;
	}

}
void rmmi_eanvbk_hdlr (rmmi_string_struct* cmd)
{
	 rmmi_validator_cause_enum err_code;
	  unsigned int ret =  RMMI_VALIDATOR_ERROR;
	  
	if (cmd->cmd_owner != CMD_OWENR_AP || cmd->cmd_class != RMMI_EXTENDED_CMD)
		return;
	switch (cmd->cmd_mode)
	{
   		case   RMMI_SET_OR_EXECUTE_MODE: //AT+EANVBK=1\r
   		        LOGD(TAG "rmmi_eanvbk_hdlr:RMMI_SET_OR_EXECUTE_MODE");
   			ret = rmmi_int_validator_ext(&err_code, cmd, RMMI_COMMA);
			if (ret == RMMI_VALIDATOR_ERROR)
			{
			    LOGD("RMMI_VALIDATOR_ERROR\n");
                sprintf(cmd->result, "%s", return_err);
				goto err;
			}
			else if (ret != 1)
			{
			    LOGD("ret != 1\n");
                sprintf(cmd->result, "%s", return_err);
				goto err;
			} else
			{
			    LOGD("Backup nvram!\n");
   				if(FileOp_BackupToBinRegion_All())
   			    {
   				    LOGD("backup nvram data to nvram binregion success!\n");
    		        //sprintf(cmd->result, "%s%s", "\r\n+EANVBK:", return_ok);  //return Parameter
    		        sprintf(cmd->result, "%s", return_ok);
   			    }
   			    else
   			    {
   				    LOGD("error:backup nvram data to nvram binregion fail!\n");
                    sprintf(cmd->result, "%s", return_err);
   			    }
   			    sync(); 
			}
			break;
    		case   RMMI_TEST_MODE:                     //AT+EANVBK=?\r
    		    sprintf(cmd->result, "%s%s", "\r\n+EANVBK:(1)", return_ok);  //return Parameter
			break;
err:				
		case   RMMI_READ_MODE:                    //AT+EANVBK?\r
		case   RMMI_ACTIVE_MODE:                 //AT+EANVBK\r
    	case   RMMI_WRONG_MODE:
		default:
            sprintf(cmd->result, "%s", return_err);
	}

	//send ack to pc
	
}



int calc_hashvalue (char *string_ptr, unsigned int *hash_value1, unsigned int *hash_value2)

{
    unsigned int counter = 0;
    int index = 3; // at+XXXX => XXXX->hash value

    /* This variable is used to ensure that the unsigned ints coming on line for
    command are properly broken into set of 5 unsigned ints. So, the correct
    hash value can be calculated based on the parser expression and
    respectively stored in the variables hash_value1 and hash_value2 */
    unsigned int ascii;

    /* Variable used to store the calculated hash value of 2nd 5 unsigned intacters
    as a part of a extended command */
//    unsigned int cmd_found = false;

    /* if command is found, cmd_index is the command enum */
    *hash_value1 = 0;

    /* Variable used to store the calculated hash value of 1st 5 unsigned intacters
       as a part of a extended command */
    *hash_value2 = 0;
    
   /* Parser for Extended AT commands */
    while ((string_ptr[index]
            != RMMI_EQUAL) &&
           (string_ptr[index]
            != RMMI_QUESTION_MARK) &&
           (string_ptr[index]
            != RMMI_COMMA) &&
           (string_ptr[index]
            != RMMI_SEMICOLON) &&
           (string_ptr[index]
            != RMMI_END_OF_STRING_CHAR)&&
           (counter <= RMMI_MAX_EXT_CMD_NAME_LEN) &&
           (string_ptr[index]
            != S3) && (string_ptr[index] != S4))
    {
	 
	 if (RMMI_IS_UPPER(string_ptr[index]))
        {
            ascii = string_ptr[index] - RMMI_CHAR_A;
        }

        else if (RMMI_IS_LOWER(string_ptr[index]))
        {
            ascii = string_ptr[index] - rmmi_char_a;
        }

    #ifdef __CS_SERVICE__
        else if (RMMI_IS_NUMBER(string_ptr[index]))
        {
            ascii = string_ptr[index] - RMMI_CHAR_0 + RMMI_NUMBER_OFFSET_IN_PARSER_TABLE;
        }
    #endif /* __CS_SERVICE__ */ 
        else  if ( string_ptr[index] == '+') 
        {
		//ascii = 1;
		 return false;
        }
	else
	{
            //rmmi_result_code_fmttr(RMMI_RCODE_ERROR, RMMI_ERR_UNRECOGNIZED_CMD);            
            return false;
            //break;
        }

        /**** [MAUI_01319443] mtk02514, 090120 *************************************************************
        *  The new hash value computed method is as follows.
        *  for AT+ABCDEFGH
        *  hash_value1 = hash(A)*38^4 + hash(B)*38^3 + hash(C)*38^2 + hash(D)*38^1 + hash(E)*38^0
        *                    = ((((hash(A)+0)*38 + hash(B))*38 + hash(C))*38 + hash(D))*38 + hash(E)  <== as following statements do.
        *  hash_value2 = hash(F)*38^2 + hash(G)*38^1 + hash(H)*38^0
        *                    = ((hash(F) + 0)*38 + hash(G))*38 + hash(H)  <== as following statements do.
        **********************************************************************************************/
        if (counter < RMMI_HASH_TABLE_SPAN)
            *hash_value1 = (*hash_value1)*(RMMI_HASH_TABLE_ROW+1)+(ascii+1);
        else
            *hash_value2 = (*hash_value2)*(RMMI_HASH_TABLE_ROW+1)+(ascii+1);

        counter++;

        /* Increment the index to get the next unsigned intacter */
        index++;

    }   /* End of while loop */
    return true;
}

int rmmi_calc_hashvalue (rmmi_string_struct *source_string_ptr, unsigned int *hash_value1, unsigned int *hash_value2)

{
    unsigned int counter = 0;
    //source_string_ptr->index = 0; //AT+, AT^

    /* This variable is used to ensure that the unsigned ints coming on line for
    command are properly broken into set of 5 unsigned ints. So, the correct
    hash value can be calculated based on the parser expression and
    respectively stored in the variables hash_value1 and hash_value2 */
    unsigned int ascii;

    /* Variable used to store the calculated hash value of 2nd 5 unsigned intacters
    as a part of a extended command */
//    unsigned int cmd_found = false;

    /* if command is found, cmd_index is the command enum */
    *hash_value1 = 0;

    /* Variable used to store the calculated hash value of 1st 5 unsigned intacters
       as a part of a extended command */
    *hash_value2 = 0;
    
    /* Skip all leading white spaces */
    rmmi_skip_spaces(source_string_ptr);

    /* Parser for Extended AT commands */
    while ((source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_EQUAL) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_QUESTION_MARK) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_COMMA) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_SEMICOLON) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_END_OF_STRING_CHAR)&&
           (counter <= RMMI_MAX_EXT_CMD_NAME_LEN) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != S3) && (source_string_ptr->string_ptr[source_string_ptr->index] != S4))
    {
	
	 
        if (RMMI_IS_UPPER(source_string_ptr->string_ptr[source_string_ptr->index]))
        {
            ascii = source_string_ptr->string_ptr[source_string_ptr->index] - RMMI_CHAR_A;
        }

        else if (RMMI_IS_LOWER(source_string_ptr->string_ptr[source_string_ptr->index]))
        {
            ascii = source_string_ptr->string_ptr[source_string_ptr->index] - rmmi_char_a;
        }

    #ifdef __CS_SERVICE__
        else if (RMMI_IS_NUMBER(source_string_ptr->string_ptr[source_string_ptr->index]))
        {
            ascii = source_string_ptr->string_ptr
                [source_string_ptr->index] - RMMI_CHAR_0 + RMMI_NUMBER_OFFSET_IN_PARSER_TABLE;
        }
    #endif /* __CS_SERVICE__ */ 

        else if ( source_string_ptr->string_ptr[source_string_ptr->index] == '+') 
	 {
	 	//ascii = 1;
	 	return false;
        } else
        {
            //rmmi_result_code_fmttr(RMMI_RCODE_ERROR, RMMI_ERR_UNRECOGNIZED_CMD);            
            return false;
            //break;
        }

        /**** [MAUI_01319443] mtk02514, 090120 *************************************************************
        *  The new hash value computed method is as follows.
        *  for AT+ABCDEFGH
        *  hash_value1 = hash(A)*38^4 + hash(B)*38^3 + hash(C)*38^2 + hash(D)*38^1 + hash(E)*38^0
        *                    = ((((hash(A)+0)*38 + hash(B))*38 + hash(C))*38 + hash(D))*38 + hash(E)  <== as following statements do.
        *  hash_value2 = hash(F)*38^2 + hash(G)*38^1 + hash(H)*38^0
        *                    = ((hash(F) + 0)*38 + hash(G))*38 + hash(H)  <== as following statements do.
        **********************************************************************************************/
        if (counter < RMMI_HASH_TABLE_SPAN)
            *hash_value1 = (*hash_value1)*(RMMI_HASH_TABLE_ROW+1)+(ascii+1);
        else
            *hash_value2 = (*hash_value2)*(RMMI_HASH_TABLE_ROW+1)+(ascii+1);

        counter++;

        /* Increment the index to get the next unsigned intacter */
        source_string_ptr->index++;

        /* skip all leading white  spaces */
        rmmi_skip_spaces(source_string_ptr);

    }   /* End of while loop */
    return true; 
}


int cmd_handler_init ()
{
	int i;
	for (i=0; i<(int)(sizeof(g_cmd_handler)/sizeof(CMD_HDLR)); i++)
	{
		if (g_cmd_handler[i].cmd_index == INVALID_ENUM)
			break;
		calc_hashvalue (g_cmd_handler[i].cmd_string, 
							&g_cmd_handler[i].hash_value1, 
							&g_cmd_handler[i].hash_value2);
		
	}
	return 0;
}

unsigned int cmd_analyzer(unsigned int hash_value1, unsigned int hash_value2,  int *cmd_index_ptr)
{
    unsigned int ret_val = false;
    unsigned int row_index;

    if ((hash_value1 == 0) && (hash_value2 == 0))
    {
        return ret_val;
    }
    for (row_index = 0; row_index < (sizeof(g_cmd_handler)/sizeof(CMD_HDLR)); row_index++)
    {
	    if (g_cmd_handler[row_index].cmd_index == INVALID_ENUM)
			break;
	    if ((hash_value1 == g_cmd_handler[row_index].hash_value1) &&
            (hash_value2 == g_cmd_handler[row_index].hash_value2))
        {
            *cmd_index_ptr = row_index;
            ret_val = true;
            break;
        }
    }

    return ret_val;
}



char custom_get_atcmd_symbol(void)
{
   return (CUSTOM_SYMBOL);
}


int rmmi_find_cmd_value(rmmi_string_struct *source_string_ptr)
{
    int ret_val = 0;
    rmmi_skip_spaces(source_string_ptr);
//    LOGD(TAG "Entry %s, %c, %02x\n", __FUNCTION__, source_string_ptr->string_ptr[source_string_ptr->index], source_string_ptr->string_ptr[source_string_ptr->index]);
//    LOGD(TAG "RMMI_COMMA is %c, %02x\n", RMMI_COMMA, RMMI_COMMA);
    if(source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_COMMA)
    {
//        LOGD(TAG "RMMI_COMMA");
        source_string_ptr->index++;
        rmmi_skip_spaces(source_string_ptr);
        while((source_string_ptr->string_ptr[source_string_ptr->index] != 0)
            && (source_string_ptr->string_ptr[source_string_ptr->index] != '\r')
            && (source_string_ptr->string_ptr[source_string_ptr->index] != '\n')
            && (source_string_ptr->string_ptr[source_string_ptr->index] != ','))
        {
//            LOGD(TAG "The char is %c\n", source_string_ptr->string_ptr[source_string_ptr->index]);
            if ((source_string_ptr->string_ptr[source_string_ptr->index] < '0')
                || (source_string_ptr->string_ptr[source_string_ptr->index] > '9'))
            {
                ret_val = -1;
                break;
            }
            else
            {
               ret_val = ret_val * 10 + (source_string_ptr->string_ptr[source_string_ptr->index] - '0'); 
            }
            LOGD(TAG "ret_val=%d\n", ret_val);
            source_string_ptr->index++;
            rmmi_skip_spaces(source_string_ptr);
        }
    }
    return ret_val;
}

rmmi_cmd_mode_enum rmmi_find_cmd_mode(rmmi_string_struct *source_string_ptr)
{ 
   rmmi_cmd_mode_enum ret_val = RMMI_WRONG_MODE;

    /* Skip all leading white spaces */
    rmmi_skip_spaces(source_string_ptr);

    /*
     * If not read mode, then check for the TEST/SET/EXECUTE mode.
     * * Symbol '=' is common for both SET/EXECUTE and TEST command;
     * * so first check for the '=' symbol.
     */
    if (source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_EQUAL)
    {
        /*
         * If we find '?' after the '=' symbol, then we decide that
         * * given command is TEST command. Else it is assumed to be
         * * either a SET or an EXECUTE command
         */
        source_string_ptr->index++;
        /* Skip white spaces after the '=' symbol */
        rmmi_skip_spaces(source_string_ptr);
        LOGD("The char after = is %c\n", source_string_ptr->string_ptr[source_string_ptr->index]);
        if (source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_QUESTION_MARK)
        {
            /*
             * Since question mark is also found, check whether the
             * * string is terminated properly by a termination character.
             * * White spaces may be present between the question mark and
             * * the termination character.
             */
            source_string_ptr->index++;
            rmmi_skip_spaces(source_string_ptr);

            if (source_string_ptr->string_ptr[source_string_ptr->index] == S3||
	         source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_END_OF_STRING_CHAR)
            {
                ret_val = RMMI_TEST_MODE;
            }
        }
        /* If didn't find '?' after the '=' symbol then we decide that
           given command is SET/EXECUTE command */
        else if(source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_CHAR_0)
        {
            ret_val = RMMI_READ_MODE;
        }
        else
        {
            ret_val = RMMI_SET_OR_EXECUTE_MODE;
        }
    }   /* mtk00468 add for some extend command has no parameter */
    else if ((source_string_ptr->string_ptr[source_string_ptr->index] == S3) ||
             (source_string_ptr->string_ptr[source_string_ptr->index] == S4)||
	      source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_END_OF_STRING_CHAR)

    {
        ret_val = RMMI_ACTIVE_MODE;
    }
    source_string_ptr->index++;
    LOGD(TAG "The end of rmmi_find_cmd_mode is %c, %02x\n", source_string_ptr->string_ptr[source_string_ptr->index], source_string_ptr->string_ptr[source_string_ptr->index]);

    return ret_val;
}
rmmi_cmd_type_enum rmmi_find_cmd_class(rmmi_string_struct *source_string_ptr)
{
    rmmi_cmd_type_enum ret_val = RMMI_INVALID_CMD_TYPE;
    source_string_ptr->index = 0;
    rmmi_skip_spaces(source_string_ptr);        // Skip all leading white spaces 

    /* Check if the first unsigned intacter is neither 'A' nor 'a' i.e. a invalid 
       command prefix */
    if ((source_string_ptr->string_ptr[source_string_ptr->index] != RMMI_CHAR_A) &&
        (source_string_ptr->string_ptr[source_string_ptr->index] != rmmi_char_a))
    {
        return ret_val;
    }

    /* Increment the index to get the next unsigned intacter */
    source_string_ptr->index++;

    /* Skip all white spaces */
    rmmi_skip_spaces(source_string_ptr);

    /* there are two possibilities of unsigned intacters may come after the
       unsigned intacter A. One is '/' and other one is 'T'. First we check for
       the unsigned intacter '/', if not found then check for the unsigned intacter 'T'. */
    if ((source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_FORWARD_SLASH) &&
        (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
    {
        /* Skip all leading spaces, which are coming after the "A/".
           Finally check for the command line termination unsigned intacter */
        source_string_ptr->index++;
        rmmi_skip_spaces(source_string_ptr);

        if ((source_string_ptr->string_ptr[source_string_ptr->index] == S3) &&
            (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
        {
            ret_val = RMMI_PREV_CMD;
        }
        /* else, command line is invalid */
        else
        {
            ret_val = RMMI_INVALID_CMD_TYPE;
        }
    }
    /* We failed to find '/'.the second alternative is 'T'.Check whether
       the second non spaces unsigned intacter is 'T' or not */
    else if (((source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_CHAR_T) ||
              (source_string_ptr->string_ptr[source_string_ptr->index] == rmmi_char_t)) &&
             (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
    {
        /*
         * Skip all leading white space unsigned intacter which are coming after
         * * "AT".Again we can find two different unsigned intacter after "AT".One is
         * * '+' and other one is non '+' unsigned intacter.if we find '+' then we
         * * decided that the give command is a Extended command, otherwise
         * * Basic command.
         */
        /*
         * there's no need of check for command line termination, because that
         * * will be checked during the parsing of commands
         */
        source_string_ptr->index++;
        rmmi_skip_spaces(source_string_ptr);

        if ((source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_CHAR_PLUS) &&
            (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
        {
            /* the '+' unsigned intacter is found,hence it is extended command */
            ret_val = RMMI_EXTENDED_CMD;
            source_string_ptr->index++; /* to get the next unsigned intacter */
        }
        else if ((source_string_ptr->string_ptr[source_string_ptr->index] == (custom_get_atcmd_symbol())) &&
                 (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
        {
            /* the special symbol defined by customer is found,hence it is customer-defined command */
            ret_val = RMMI_CUSTOMER_CMD;
            source_string_ptr->index++; /* to get the next unsigned intacter */
        }
        else
        {
            /* the non '+' unsigned intacter was not found; take it
               to be basic command */
            ret_val = RMMI_BASIC_CMD;
        }
    }

    /* We didn't find the either "AT" or "A/". Either the command was too long, 
       or the unsigned intacters were unrecognizable */
    else if (source_string_ptr->index >= MAX_MULTIPLE_CMD_INFO_LEN)
    {
        ret_val = RMMI_INVALID_CMD_TYPE;
    }
    /* unrecognizable command line prefix */
    else
    {
        ret_val = RMMI_INVALID_CMD_TYPE;
    }

    return ret_val;
}


int rmmi_cmd_analyzer (rmmi_string_struct *source_string_ptr)
{  
    unsigned int hash_value1, hash_value2;
    unsigned int cmd_found = 0;

    source_string_ptr ->cmd_owner = CMD_OWNER_INVALID;
    source_string_ptr->cmd_mode = RMMI_WRONG_MODE;
    source_string_ptr->cmd_class = rmmi_find_cmd_class(source_string_ptr);

    LOGD("RMMI_CMD_ANALYZER:%d\n", source_string_ptr->cmd_class);

    if (RMMI_INVALID_CMD_TYPE==source_string_ptr->cmd_class) 
    {
         cmd_found =  INVALID_ENUM;
	     goto err;
    }
    if (false==rmmi_calc_hashvalue (source_string_ptr, &hash_value1, &hash_value2))
    {
         cmd_found =  INVALID_ENUM;
	     goto err;
    }
    		
    cmd_found = cmd_analyzer(hash_value1, hash_value2, &source_string_ptr->cmd_index);

    source_string_ptr->cmd_mode = rmmi_find_cmd_mode (source_string_ptr);
    source_string_ptr->value = rmmi_find_cmd_value(source_string_ptr);
    source_string_ptr->cmd_owner = (cmd_found==1) ? CMD_OWENR_AP:CMD_OWENR_MD;

err:		
   return cmd_found;

}


CMD_OWENR_ENUM rmmi_cmd_processor(unsigned char *cmd_str, char *result)
{
    LOGD(TAG "Entry rmmi_cmd_processor");
	unsigned int cmd_found = 0;
	rmmi_string_struct source_string_ptr;

	source_string_ptr.index = 0;
    source_string_ptr.result = result;
	source_string_ptr.string_ptr = cmd_str;
	cmd_found = rmmi_cmd_analyzer(&source_string_ptr);
    LOGD(TAG "cmd_found=%d", cmd_found);

	if (source_string_ptr.cmd_owner == CMD_OWENR_AP )
		g_cmd_handler[source_string_ptr.cmd_index].func(&source_string_ptr);
	
    LOGD(TAG "source_string_ptr.cmd_owner=%d", source_string_ptr.cmd_owner);
	return source_string_ptr.cmd_owner;

}
int write_bt(char *value, char *result)
{
    char output[bt_length] = {0};
    int rec_size = 0;
    int rec_num = 0;
    unsigned char w_bt[bt_length];
    int length;
    int ret, i = 0;

    nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISREAD);
  	LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   	if(1 != rec_num)
   	{
   		LOGD("error:unexpected record num %d\n",rec_num);
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	if(sizeof(g_bt_nvram) != rec_size)
   	{
   		LOGD("error:unexpected record size %d\n",rec_size);
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	memset(&g_bt_nvram,0,rec_num*rec_size);
    ret = read(nvram_fd.iFileDesc, &g_bt_nvram, rec_num*rec_size);
   	if(-1 == ret||rec_num*rec_size != ret)
   	{
   		LOGD("error:read bt addr fail!/n");
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	LOGD("read pre bt addr:%02x%02x%02x%02x%02x%02x\n", 
          g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5] 
    );
   	NVM_CloseFileDesc(nvram_fd);
   	nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISWRITE);
   	LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   	if(1 != rec_num)
   	{
   		LOGD("error:unexpected record num %d\n",rec_num);
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	if(sizeof(g_bt_nvram) != rec_size)
   	{
   		LOGD("error:unexpected record size %d\n",rec_size);
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	memset(g_bt_nvram.addr,0,bt_length);
   	memset(w_bt,0,bt_length);
    length = strlen(value);
    if(length != 12)
    {
	    LOGD("error:bt address (%s) length %d is not right!\n", value, length);
        sprintf(result, "%s", return_err);
		return -1;
    }
	ret = convStrtoHex(value,output,bt_length,&length);
	if(-1 == ret)
	{
		LOGD("error:convert bt address to hex fail\n");
        sprintf(result, "%s", return_err);
		return -1;
	}
    else
    {
        LOGD("BT Address:%s\n", output);
    }
    for(i=0;i<bt_length;i++)
   	{	
   		g_bt_nvram.addr[i] = output[i];
   	}
   	LOGD("write bt addr:%02x%02x%02x%02x%02x%02x, value:%02x%02x%02x%02x%02x%02x\n", 
          g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5],
          output[0], output[1], output[2], output[3], output[4], output[5]
    );
   	ret = write(nvram_fd.iFileDesc, &g_bt_nvram , rec_num*rec_size);
   	if(-1 == ret||rec_num*rec_size != ret)
   	{
   		LOGD("error:write wifi addr fail!\n");
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	NVM_CloseFileDesc(nvram_fd);
   	LOGD("write bt addr success!\n");
   	if(FileOp_BackupToBinRegion_All())
   	{
   		LOGD("backup nvram data to nvram binregion success!\n");
        sprintf(result, "%s", return_ok);
   	}
   	else
   	{
   		LOGD("error:backup nvram data to nvram binregion fail!\n");
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	sync();

    return 0;
}

int read_bt(char *result)
{
    int rec_size = 0;
    int rec_num = 0;
    int ret = 0;

    nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISREAD);
  	LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   	if(1 != rec_num)
   	{
   		LOGD("error:unexpected record num %d\n",rec_num);
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	if(sizeof(g_bt_nvram) != rec_size)
   	{
   		LOGD("error:unexpected record size %d\n",rec_size);
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	memset(&g_bt_nvram,0,rec_num*rec_size);
   	ret = read(nvram_fd.iFileDesc, &g_bt_nvram, rec_num*rec_size);
   	if(-1 == ret||rec_num*rec_size != ret)
   	{
   		LOGD("error:read bt addr fail!/n");
        sprintf(result, "%s", return_err);
   		return -1;
   	}
   	LOGD("read pre bt addr:%02x%02x%02x%02x%02x%02x\n", 
           g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5] 
    );

    sprintf(result, "%02x%02x%02x%02x%02x%02x", g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5]);
   	NVM_CloseFileDesc(nvram_fd);

    return 0;
}

int write_wifi(char *value, char *result)
{
    char output[wifi_length] = {0};
    int rec_size = 0;
    int rec_num = 0;
    int ret, length = 0, i = 0;

    nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISREAD);
    printf("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
    if(1 != rec_num)
    {
         printf("error:unexpected record num %d\n",rec_num);
         sprintf(result, "%s", return_err);
         return -1;
    }
    if(sizeof(WIFI_CFG_PARAM_STRUCT) != rec_size)
    {
        printf("error:unexpected record size %d\n",rec_size);
        sprintf(result, "%s", return_err);
        return -1;
    }
    memset(&g_wifi_nvram,0,rec_num*rec_size);
    ret = read(nvram_fd.iFileDesc, &g_wifi_nvram, rec_num*rec_size);
    if(-1 == ret||rec_num*rec_size != ret)
    {
        printf("error:read wifi mac addr fail!/n");
        sprintf(result, "%s", return_err);
        return -1;
    }
    printf("read wifi addr:%02x%02x%02x%02x%02x%02x\n", 
              g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], g_wifi_nvram.aucMacAddress[4], 
              g_wifi_nvram.aucMacAddress[5]);
    NVM_CloseFileDesc(nvram_fd);

    nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISWRITE);
    LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
    if(1 != rec_num)
    {
        LOGD("error:unexpected record num %d\n",rec_num);
        sprintf(result, "%s", return_err);
        return -1;
    }
    if(sizeof(g_wifi_nvram) != rec_size)
    {
        LOGD("error:unexpected record size %d\n",rec_size);
        sprintf(result, "%s", return_err);
        return -1;
    }
    memset(g_wifi_nvram.aucMacAddress,0,bt_length);
    length = strlen(value);
    if(length != 12)
    {
        LOGD("error:bt address length is not right!\n");
        sprintf(result, "%s", return_err);
        return -1;
    }
    ret = convStrtoHex(value,output,wifi_length,&length);
    if(-1 == ret)
    {
        LOGD("error:convert wifi address to hex fail\n");
        sprintf(result, "%s", return_err);
        return -1;
    }
    else
    {
        LOGD("WIFI Address:%s\n", output);
    }
    for(i=0;i<bt_length;i++)
    {   
        g_wifi_nvram.aucMacAddress[i] = output[i];
    }
    LOGD("write wifi addr:%02x%02x%02x%02x%02x%02x, value:%02x%02x%02x%02x%02x%02x\n", 
            g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], 
            g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], 
            g_wifi_nvram.aucMacAddress[4], g_wifi_nvram.aucMacAddress[5],
            output[0], output[1], output[2], output[3], output[4], output[5]
    );
    ret = write(nvram_fd.iFileDesc, &g_wifi_nvram , rec_num*rec_size);
    if(-1 == ret||rec_num*rec_size != ret)
    {
        LOGD("error:write wifi addr fail!\n");
        sprintf(result, "%s", return_err);
        return -1;
    }
    NVM_CloseFileDesc(nvram_fd);
    LOGD("write wifi addr success!\n");
    if(FileOp_BackupToBinRegion_All())
    {
        LOGD("backup nvram data to nvram binregion success!\n");
        sprintf(result, "%s", return_ok);
    } 
    else
    {
        LOGD("error:backup nvram data to nvram binregion fail!\n");
        sprintf(result, "%s", return_err);
        return -1;
    }
    sync();
    return 0;
}

int read_wifi(char *result)
{
    int rec_size = 0;
    int rec_num = 0;
    int ret;
    nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISREAD);
    printf("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
    if(1 != rec_num)
    {
        LOGD("error:unexpected record num %d\n",rec_num);
        sprintf(result, "%s", return_err);
        return -1;
    }
    if(sizeof(WIFI_CFG_PARAM_STRUCT) != rec_size)
    {
        LOGD("error:unexpected record size %d\n",rec_size);
        sprintf(result, "%s", return_err);
        return -1;
    }
    memset(&g_wifi_nvram,0,rec_num*rec_size);
    ret = read(nvram_fd.iFileDesc, &g_wifi_nvram, rec_num*rec_size);
    if(-1 == ret||rec_num*rec_size != ret)
    {
        LOGD("error:read wifi mac addr fail!/n");
        sprintf(result, "%s", return_err);
        return -1;
    }
    LOGD("read wifi addr:%02x%02x%02x%02x%02x%02x\n", 
            g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], g_wifi_nvram.aucMacAddress[4], 
            g_wifi_nvram.aucMacAddress[5]);
    sprintf(result, "%02x%02x%02x%02x%02x%02x", g_wifi_nvram.aucMacAddress[0], 
              g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], 
              g_wifi_nvram.aucMacAddress[4], g_wifi_nvram.aucMacAddress[5]);
    NVM_CloseFileDesc(nvram_fd);
    return 0;
}


void g_ATE_Callback(const char *pdata, int len)
{
    LOGD(TAG "%s start",__FUNCTION__);
	int wr_len = 0;
	char temp[1000] = {0};
	strncpy(temp, pdata,len);
    if(strlen(pdata)!=0)
	{
		wr_len = write_chars (g_hUsbComPort, temp, strlen(temp));
		if (wr_len != (int)strlen(temp))
		{
			LOGE(TAG "AP_CCCI_RX: wr_len != rd_len\n");
		}
	}
	LOGD("%s end",__FUNCTION__);
}


