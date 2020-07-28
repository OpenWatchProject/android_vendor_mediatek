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

#ifndef _FTM_UTILS_H_
#define _FTM_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "miniui.h" 
#include "item.h"
#define FACTORY_BOOT 4
#define ATE_FACTORY_BOOT 6

#define MODEM_MAX_NUM 2

#define CCCI_MODEM_MT6252               "/dev/ttyMT0"
#define	CCCI_MODEM_MT8135               "/dev/ttyUSB1"

/*signaltest*/
#define CCCI_MODEM_MT6575		"/dev/ttyC0"                                      
#define CCCI_MODEM2			"/dev/ccci2_tty0"
//#endif

//add for save test report
#define TEST_REPORT_SAVE_FILE "/data/testreport.log"



#define BUFSZ           2048
#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))

typedef enum {
	FALSE = 0,
	TRUE,
} _BOOL;

//#endif

#define NULL_TERM '\0'  /* string termination */
#define INVALID_ENUM -1        /* invalid enum value */
#define S3 13
#define S4 10
#define CUSTOM_SYMBOL  '^'	 // '+'  and '/' and ' \ 'is NOT allowed   
#define RMMI_MAX_EXT_CMD_NAME_LEN   24
#define RMMI_VALIDATOR_ERROR        255
#define RMMI_HASH_TABLE_ROW   37
#define RMMI_HASH_TABLE_SPAN  5

#define wifi_length 6
#define bt_length 6
#define return_ok "\r\nOK\r\n"
#define return_err "\r\nERROR\r\n"

enum
{
#if defined(__LONG_MULTIPLE_CMD_SUPPORT__) || defined(__SP_RIL_SUPPORT__)
    MAX_MULTIPLE_CMD_INFO_LEN = 350,
#elif defined(__SLIM_AT__) 
    /* mtk01616_100112: Only support multiple basic cmd(ex:ATL will use),no multiple extended cmd. 20 shall be enough */
    MAX_MULTIPLE_CMD_INFO_LEN = 20,
#else 
    MAX_MULTIPLE_CMD_INFO_LEN = 40,
#endif 
#ifdef __GATI_ENABLE__
    MAX_SINGLE_CMD_INFO_LEN = 1500,
#else
    MAX_SINGLE_CMD_INFO_LEN = 700,
#endif
    MAX_PRE_ALLOCATED_BASIC_CMD_STRUCT_NODES = 3
};




typedef enum
{
    RMMI_PARSE_OK,
    RMMI_PARSE_ERROR,   //out of range
    RMMI_PARSE_NOT_FOUND,
    RMMI_PARSE_TEXT_TOO_LONG
} rmmi_validator_cause_enum;


typedef enum
{
    RMMI_SPACE = ' ',
    RMMI_EQUAL = '=',
    RMMI_COMMA = ',',
    RMMI_SEMICOLON = ';',
    RMMI_COLON = ':',
    RMMI_AT = '@',
    RMMI_HAT = '^',
    RMMI_DOUBLE_QUOTE = '"',
    RMMI_QUESTION_MARK = '?',
    RMMI_EXCLAMATION_MARK = '!',
    RMMI_FORWARD_SLASH = '/',
    RMMI_L_ANGLE_BRACKET = '<',
    RMMI_R_ANGLE_BRACKET = '>',
    RMMI_L_SQ_BRACKET = '[',
    RMMI_R_SQ_BRACKET = ']',
    RMMI_L_CURLY_BRACKET = '{',
    RMMI_R_CURLY_BRACKET = '}',
    RMMI_CHAR_STAR = '*',
    RMMI_CHAR_POUND = '#',
    RMMI_CHAR_AMPSAND = '&',
    RMMI_CHAR_PERCENT = '%',
    RMMI_CHAR_PLUS = '+',
    RMMI_CHAR_MINUS = '-',
    RMMI_CHAR_DOT = '.',
    RMMI_CHAR_ULINE = '_',
    RMMI_CHAR_TILDE = '~',
    RMMI_CHAR_REVERSE_SOLIDUS = '\\',
    RMMI_CHAR_VERTICAL_LINE = '|',
    RMMI_END_OF_STRING_CHAR = '\0',
    RMMI_CHAR_0 = '0',
    RMMI_CHAR_1 = '1',
    RMMI_CHAR_2 = '2',
    RMMI_CHAR_3 = '3',
    RMMI_CHAR_4 = '4',
    RMMI_CHAR_5 = '5',
    RMMI_CHAR_6 = '6',
    RMMI_CHAR_7 = '7',
    RMMI_CHAR_8 = '8',
    RMMI_CHAR_9 = '9',
    RMMI_CHAR_A = 'A',
    RMMI_CHAR_B = 'B',
    RMMI_CHAR_C = 'C',
    RMMI_CHAR_D = 'D',
    RMMI_CHAR_E = 'E',
    RMMI_CHAR_F = 'F',
    RMMI_CHAR_G = 'G',
    RMMI_CHAR_H = 'H',
    RMMI_CHAR_I = 'I',
    RMMI_CHAR_J = 'J',
    RMMI_CHAR_K = 'K',
    RMMI_CHAR_L = 'L',
    RMMI_CHAR_M = 'M',
    RMMI_CHAR_N = 'N',
    RMMI_CHAR_O = 'O',
    RMMI_CHAR_P = 'P',
    RMMI_CHAR_Q = 'Q',
    RMMI_CHAR_R = 'R',
    RMMI_CHAR_S = 'S',
    RMMI_CHAR_T = 'T',
    RMMI_CHAR_U = 'U',
    RMMI_CHAR_V = 'V',
    RMMI_CHAR_W = 'W',
    RMMI_CHAR_X = 'X',
    RMMI_CHAR_Y = 'Y',
    RMMI_CHAR_Z = 'Z',
    rmmi_char_a = 'a',
    rmmi_char_b = 'b',
    rmmi_char_c = 'c',
    rmmi_char_d = 'd',
    rmmi_char_e = 'e',
    rmmi_char_f = 'f',
    rmmi_char_g = 'g',
    rmmi_char_h = 'h',
    rmmi_char_i = 'i',
    rmmi_char_j = 'j',
    rmmi_char_k = 'k',
    rmmi_char_l = 'l',
    rmmi_char_m = 'm',
    rmmi_char_n = 'n',
    rmmi_char_o = 'o',
    rmmi_char_p = 'p',
    rmmi_char_q = 'q',
    rmmi_char_r = 'r',
    rmmi_char_s = 's',
    rmmi_char_t = 't',
    rmmi_char_u = 'u',
    rmmi_char_v = 'v',
    rmmi_char_w = 'w',
    rmmi_char_x = 'x',
    rmmi_char_y = 'y',
    rmmi_char_z = 'z',
    RMMI_R_BRACKET = ')',  
    RMMI_L_BRACKET = '(', 
    RMMI_MONEY = '$'
} rmmi_char_enum;

typedef enum
{
    RMMI_INVALID_CMD_TYPE = 0,
    RMMI_PREV_CMD,
    RMMI_BASIC_CMD,
    RMMI_EXTENDED_CMD,
    RMMI_EXTENDED_CUSTOM_CMD,  // __RMMI_EXTEND_CUSTOM_CMD__
    RMMI_CUSTOMER_CMD,
} rmmi_cmd_type_enum;

typedef enum
{
    RMMI_WRONG_MODE,
    RMMI_SET_OR_EXECUTE_MODE,
    RMMI_READ_MODE,
    RMMI_TEST_MODE,
    RMMI_ACTIVE_MODE
} rmmi_cmd_mode_enum;

typedef enum
{
    CMD_OWENR_AP,
    CMD_OWENR_MD,
    
    CMD_OWNER_INVALID = INVALID_ENUM,
} CMD_OWENR_ENUM;

typedef enum
{
	FTM_UNKNOWN_COM=0,
	FTM_UART_COM,
	FTM_USB_COM,
}FTM_COM_TYPE;

typedef struct rmmi_string_struct
{
    unsigned int index;  //string index
    unsigned char *string_ptr;
    char *result;
    int cmd_index;       //rmmi_extended_cmd_id_enum
    rmmi_cmd_type_enum cmd_class;     
    rmmi_cmd_mode_enum cmd_mode;
    CMD_OWENR_ENUM cmd_owner;
    int value;
} rmmi_string_struct;

typedef void (*HANDLER) (rmmi_string_struct*);

typedef struct _CMD_HDLR
{
    int cmd_index;
    char cmd_string[32];
    unsigned int hash_value1;  
    unsigned int hash_value2;
    HANDLER func;
	
} CMD_HDLR;


#define wifi_length 6
#define bt_length 6
#define return_ok "\r\nOK\r\n"
#define return_err "\r\nERROR\r\n"

// utils.c
int readSys_int(char const* path);
int writeSys_int(char const* path, int value);
int getBootMode();
FILE * open_file(char* filename, int mode);

// at_command.c
int openDevice(void);
void closeDevice(int fd);
const char* setSleepMode(const int fd);
const char* getSN(char *sn, const unsigned int len, const int fd);
const char* setSN(const char* sn, const int fd);
const char * at_command_set(char *pCommand, const int fd);
void ate_signal(void);
int is_support_modem(int modem);
int send_at (const int fd, const char *pCMD);
int read_ack (const int fd, char *rbuff, int length);
int wait4_ack (const int fd, char *pACK, int timeout);
int wait4_readAck(const int fd, int timeout, char *buf);
void initTermIO(int portFd, int cmux_port_speed);
int openDeviceWithDeviceName(char *deviceName);

int ExitFlightMode_idle (int fd, _BOOL bON);
int ExitFlightMode_DualTalk(int fd,_BOOL bON);
int ExitFlightMode_PowerOffModem(int fd, int ioctl_fd, _BOOL bON);

int get_md_count();
int get_lte_md_count();
int open_ccci(int ccci_handle[]);

CMD_OWENR_ENUM rmmi_cmd_processor(unsigned char *cmd_str, char *result);
int cmd_handler_init ();
int rmmi_cmd_analyzer (rmmi_string_struct *source_string_ptr);
void rmmi_eabt_hdlr (rmmi_string_struct* cmd, char *addr);
void rmmi_eawifi_hdlr (rmmi_string_struct* cmd, char *addr);
void rmmi_eanvbk_hdlr (rmmi_string_struct* cmd);
void rmmi_eamdimg_hdlr(rmmi_string_struct *cmd, char *md_idx);

// uart_op.c
#define UART_PORT1 1
#define STR_ERROR "ERROR"
#define STR_OK "OK"

int open_usb();
void close_usb();
int is_usb_state_plugin(void);

int open_uart_port(int uart_id, int baudrate, int length, char parity_c, int stopbits);
int open_usb_port(int uart_id, int baudrate, int length, char parity_c, int stopbits);
int  read_a_line(int fd, char rbuff[], unsigned int length);
int  read_a_line_for_ata(int fd, char rbuff[], unsigned int length);
int write_chars(int fd, char wbuff[], unsigned int length);

FTM_COM_TYPE getComType(void);
int open_uart(void);

//int COM_Init (int *fd_atcmd, int *fd_uart, int *hUsbComPort);
//int COM_DeInit (int *fd_atcmd, int *fd_uart, int *hUsbComPort);

//iniconfig.c
int read_config(char *filename);

//version.c
int get_barcode_from_nvram(char *barcode_result);
int create_verinfo(char *info);
int create_report(item_t *item, item_t *rpt_items, int maxitems, char *buf, int size);
void ftm_get_key_map(void);
char* display_version(int index, char* result, bool autoreturn);
char* display_version_ata(int index, char* result);

int write_bt(char *value, char *result);
int read_bt(char *result);
int write_wifi(char *value, char *result);
int read_wifi(char *result);
int ftm_get_sim_num(void);

#ifdef __cplusplus
}
#endif


#endif
