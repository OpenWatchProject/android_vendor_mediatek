/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
#include <stdio.h>
#include <string.h>
#include <cutils/properties.h>
#include "me_connection.h"
#include "ftm.h"
#include "utils.h"
#include "at_command.h"
#define TAG        "[VERSION]"
#define MAX_MODEM_INDEX 4
int g_nr_lines;
extern sp_ata_data return_data;

extern char SP_ATA_PASS[16];

int getIMEI(int sim, Connection& modem ,char *result)
{
    char imei[128] = {0};
    if(sim==1)
    {
        modem.QueryModemRevision(7, imei);
    }
    else if(sim==2)
    {
        modem.QueryModemRevision(10, imei);
    }
    else if(sim==3)
    {
        modem.QueryModemRevision(11, imei);
    }
    else
    {
        modem.QueryModemRevision(12, imei);
    }

    LOGD(TAG "get IMEI is %s",imei);

    strncpy(result,imei,strlen(imei));

    if(strlen(result) <= 0)
    {
   	    LOGD(TAG "IMEI Can't read back");
        strncpy(result, "unknown", strlen("unknown"));
    }
   
    LOGE("getIMEI %s",result);
    return 0;
}

int getModemVersion(Connection& modem, char *result)
{
    const int BUF_SIZE = 128;
    char ver[BUF_SIZE] = {0};
    modem.QueryFWVersion(ver);
    LOGD("ver %s",ver);

    strncpy(result,ver,strlen(ver));

    if(strlen(result) <= 0)
    {
   	    LOGD(TAG "Modemversion Can't read back");
        strncpy(result, "unknown", strlen("unknown"));
    }

    LOGD(TAG "getModemVersion result = %s\n", result);
    return 0;
}

int getBarcode(Connection& modem, char *result)
{
    const int BUF_SIZE = 128;
    char barcode[BUF_SIZE]={0};

    if(ER_OK != modem.QueryModemRevision(5, barcode))
    return -1 ;

    LOGD("Barcode %s",barcode);

    strncpy(result,barcode,strlen(barcode));

    if(strlen(result) <= 0)
    {
        strncpy(result, "unknown", strlen("unknown"));
    }

    LOGE("getBarcode result = %s\n", result);
    return 0;    
}

int getMEID(Connection& modem, char *result)
{   
    const int BUF_SIZE = 10000;
    char meid[BUF_SIZE] = {0};
    char *p = NULL ;
    char *ptr = NULL ;

    if(ER_OK != modem.ResetConfig())
    return -1 ;

    if(ER_OK != modem.SetModemFunc(1))
    return -1 ;

    modem.QueryMEID(meid);
    LOGD("MEID: %s",meid);
    p = strchr(meid, 'x');
    if(NULL == p)
    {
        LOGE("get MEID error,can't find 'x'");
        return -1 ;
    }
    p++;
    strncpy(result, p, strlen(p));
    ptr = strchr(result, '\n');
    if (ptr != NULL)
    {
        *ptr = 0;
    }
    else
    {
        LOGE("get MEID error");
        return -1;
    }
    if(strlen(result) <= 0)
    {
        strncpy(result, "unknown", strlen("unknown"));
    }
    else
    {
        if(result[strlen(result)-1] == '\r')
        {
            result[strlen(result)-1] = 0;
        }
    }
    LOGD(TAG "getMEID result = %s\n", result);
    return 0;
}

void print_verinfo(char *info, int *len, const char *tag, char *msg)
{
    char buf[256] = {0};
    int _len = 0;
    int tag_len = 0;
    int max_len=0;
    if((info == NULL) || (len == NULL) || (tag == NULL) || (msg == NULL))
    {
        return;
    }

    _len = *len;
    tag_len = strlen(tag);

    #if defined SUPPORT_GB2312
        max_len = gr_fb_width() / CHAR_WIDTH*2;
    #else
        max_len = gr_fb_width() / CHAR_WIDTH;
    #endif

    int msg_len = strlen(msg);

    int buf_len = gr_fb_width() / CHAR_WIDTH;

    _len += sprintf(info + _len, "%s", tag);
    _len += sprintf(info + _len, ": ");

    if(msg_len>max_len-tag_len-2)
    {
        _len += sprintf(info+_len,"\n    ");
        g_nr_lines++;
    }

    while(msg_len>0)
    {
        buf_len = max_len - 4;
        buf_len = (msg_len > buf_len ? buf_len : msg_len);
        strncpy(buf, msg, 256);
        buf[buf_len] = 0;

        _len += sprintf(info + _len, "%s", buf);
        _len += sprintf(info + _len, "\n");
        g_nr_lines++;
        msg_len-=buf_len;
        msg = &(msg[buf_len]);
        while(msg_len>0 && msg[0]==' ')
        {
            msg_len--;
            msg = &(msg[1]);
        }

        if(msg_len>0)
        {
            for(buf_len=0; buf_len < 4; buf_len++) buf[buf_len]=' ';
            buf[buf_len]=0;
            _len += sprintf(info + _len, "%s", buf);
        }
    }
    *len = _len;
}    

static int create_md_verinfo(char *info, int *len) {
    char ccci_path[MAX_MODEM_INDEX][32];
    memset(ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    char temp_ccci_path [MAX_MODEM_INDEX][32];
    memset(temp_ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);  
    char imei[4][128];
    memset(imei, 0, sizeof(char) * 4 * 128);
    char meid[128];
    memset(meid, 0, sizeof(char) * 128);
    char md_ver[2][128];
    memset(md_ver, 0, sizeof(char) * 2 * 128);
    char barcode[2][128];
    memset(barcode, 0, sizeof(char) * 2 * 128);
    Connection modem[5];
    int md_num = get_md_count();
    int lte_md_num = get_lte_md_count();
    int sim_num = ftm_get_sim_num();
    int i = 0, j = 0;
    for(i = 0; i < MAX_MODEM_INDEX; i++)
    {
        if(1 == get_ccci_path(i,temp_ccci_path[i]))
        {
            strncpy(ccci_path[j],temp_ccci_path[i], strlen(temp_ccci_path[i]));
            j++ ;
        }
    }
    for (int i = 0; i < md_num; i++)
    {
        if (strlen(ccci_path[i]) > 0) {
            if(0 == modem[i].Conn_Init(ccci_path[i],i,g_SIGNAL_Callback[i])) {
                LOGD(TAG "modem %d open fail",(i));
            } else {
                LOGD(TAG "modem %d open OK",(i));
            }
        }
    }

    if (1 == isC2kSupport())
    {
        if(g_Flag_VPUP != 1)
        {
            if(ER_OK!= modem[md_num-1].QueryModemStatus())
            {
                g_Flag_VPUP = 0;
                wait_URC(ID_VPUP);
            }
            else
            {
                g_Flag_VPUP = 1 ;
            }
        }
        LOGD("modem c2k open successfully");
        getMEID(modem[md_num-1], meid);
    }

    #ifndef FEATURE_FTM_WIFI_ONLY
    for (int i = 0; i < lte_md_num; i++)
    {
        if(g_Flag_EIND != 1)
        {
            if(ER_OK!= modem[i].QueryModemStatus())
            {
                g_Flag_EIND = 0 ;
                wait_URC(ID_EIND);
            }
            else
            {
                g_Flag_EIND = 1 ;
            }
        }
    }
    #endif

    getIMEI(1, modem[0], imei[0]);
    getModemVersion(modem[0], md_ver[0]);
    getBarcode(modem[0], barcode[0]);
    if (lte_md_num == 1) {
       if (sim_num == 2) {
           getIMEI(2, modem[0], imei[1]);
       } else if (sim_num == 3) {
           getIMEI(2, modem[0], imei[1]);
           getIMEI(3, modem[0], imei[2]);
       } else if (sim_num == 4) {
           getIMEI(2, modem[0], imei[1]);
           getIMEI(3, modem[0], imei[2]);
           getIMEI(4, modem[0], imei[3]);
       }
    } else if (lte_md_num == 2) {
        getIMEI(2, modem[1], imei[1]);
        getModemVersion(modem[1], md_ver[1]);
        getBarcode(modem[1], barcode[1]);
    } 
    for(int i = 0; i < md_num; i++)
    {
        if(1 == modem[i].Conn_DeInit())
        {
            LOGD(TAG "Deinit the port successfully\n");
        }
        else
        {
           LOGD(TAG "Deinit the port failed \n");
        }
    }
    #ifdef FEATURE_FTM_3GDATA_SMS
    #elif defined FEATURE_FTM_3GDATA_ONLY
    #elif defined FEATURE_FTM_WIFI_ONLY
    #else
    if (sim_num == 1) {
        print_verinfo(info, len,  "IMEI1       ", imei[0]);
    } else if (sim_num == 2) {
        print_verinfo(info, len,  "IMEI1       ", imei[0]);
        print_verinfo(info, len,  "IMEI2       ", imei[1]);
    } else if (sim_num == 3) {
        print_verinfo(info, len,  "IMEI1       ", imei[0]);
        print_verinfo(info, len,  "IMEI2       ", imei[1]);
        print_verinfo(info, len,  "IMEI3       ", imei[2]);
    } else if (sim_num == 4) {
        print_verinfo(info, len,  "IMEI1       ", imei[0]);
        print_verinfo(info, len,  "IMEI2       ", imei[1]);
        print_verinfo(info, len,  "IMEI3       ", imei[2]);
        print_verinfo(info, len,  "IMEI4       ", imei[3]);
    }
    #endif

    #ifndef EVDO_FTM_DT_VIA_SUPPORT
    if(isC2kSupport() == 1){
        print_verinfo(info, len,  "MEID        ", meid);
    }
    #endif

    #ifndef FEATURE_FTM_WIFI_ONLY
        if(lte_md_num == 1)
        {
            print_verinfo(info, len,  "Modem Ver.  ", md_ver[0]);
            sprintf(return_data.version.modem_ver,"%s", md_ver[0]);
            print_verinfo(info, len,  "Bar code    ", barcode[0]);
        }
        else if(lte_md_num == 2)
        {
            print_verinfo(info, len,  "Modem Ver.  ", md_ver[0]);
            sprintf(return_data.version.modem_ver,"%s", md_ver[0]);
            print_verinfo(info, len,  "Modem Ver2.  ", md_ver[1]);
            print_verinfo(info, len,  "Bar code    ", barcode[0]);
            print_verinfo(info, len,  "Bar code2    ", barcode[1]);
        }

    #else
        print_verinfo(info, len,  "Bar code    ", barcode[0]);
    #endif
    return 0;

}

static int create_ap_verinfo(char *info, int *len)
{
char val[128] = {0};
    char kernel_ver[256] = "unknown";
    int kernel_ver_fd = -1;
    char buffer[1024] = {0};
    char *ptr= NULL, *pstr = NULL;
    int i = 0;

    kernel_ver_fd = open("/proc/version",O_RDONLY);
    if(kernel_ver_fd!=-1)
    {
        read(kernel_ver_fd, kernel_ver, 255);
		LOGD("kernel_ver = %s",kernel_ver);
        close(kernel_ver_fd);
    }
	else
	{
		LOGD(TAG "Open file /proc/version fail \n");	
	}

    if(kernel_ver[strlen(kernel_ver)-1]=='\n')
	{
		kernel_ver[strlen(kernel_ver)-1]=0;
    }

	ptr = &(kernel_ver[0]);
    for(i=0;i<(int)strlen(kernel_ver);i++)
    {
        if(kernel_ver[i]>='0' && kernel_ver[i]<='9')
        {
            ptr = &(kernel_ver[i]);
			break;
        }
    }

	for(i=0;i<(int)strlen(ptr);i++)
    {
    	if((ptr[i]>='0' && ptr[i]<='9')||ptr[i]=='.')
		{
			continue;
    	}
		else
		{
			ptr[i]='\0';	
		}
	}

	LOGD("kernel_ver = %s",ptr);

    property_get("ro.board.platform", val, "unknown");
    print_verinfo(info, len,  "BB Chip     ", val);
    property_get("ro.product.device", val, "unknown");
    print_verinfo(info, len,  "MS Board.   ", val);
    property_get("ro.build.date", val, "TBD");
    print_verinfo(info, len,  "Build Time  ", val);
    print_verinfo(info, len,  "Kernel Ver. ", ptr);
    property_get("ro.build.version.release", val, "unknown");
    print_verinfo(info, len,  "Android Ver.", val);
    property_get("ro.vendor.mediatek.version.release", val, "unknown");
    print_verinfo(info, len,  "SW Ver.     ", val);
    sprintf(return_data.version.sw_ver,"%s", val);

    return *len;
}

int create_verinfo(char *info)
{
    int len = 0;
    g_nr_lines = 0;

    create_ap_verinfo(info, &len);
    create_md_verinfo(info, &len);

    return 0;
}

char ** trans_verinfo(const char *str, int *line)
{
    char **pstrs = NULL;
    int  len     = 0;
    int  row     = 0;
    const char *start;
    const char *end;

    if((str == NULL) || (line == NULL))
    {
        return NULL;
    }

    len = strlen(str) + 1;
    start  = str;
    end    = str;
    pstrs = (char**)malloc(g_nr_lines * sizeof(char*));

    if (!pstrs)
    {
        LOGE("In factory mode: malloc failed\n");
        return NULL;
    }
    while (len--)
    {
        if ('\n' == *end)
        {
            pstrs[row] = (char*)malloc((end - start + 1) * sizeof(char));
            if (!pstrs[row])
            {
                LOGE("In factory mode: malloc failed\n");
                return NULL;
            }
            strncpy(pstrs[row], start, end - start);
            pstrs[row][end - start] = '\0';
            start = end + 1;
            row++;
        }
        end++;
    }
    *line = row;
    return pstrs;
}

void tear_down(char **pstr, int row)
{
    int i;
    if(pstr == NULL)
    {
        return;
    }
    for (i = 0; i < row; i++)
    {
        if (pstr[i])
        {
            free(pstr[i]);
            pstr[i] = NULL;
        }
    }

    if (pstr)
    {
        free(pstr);
        pstr = NULL;
    }
}

/*
    autoreturn:  if the function called by ata, then true;
    if called by main, then false;
*/
char* display_version_ata(int index, char* result)
{
    if(result == NULL)
    {
        return NULL;
    }
    return display_version(index, result, true);
}

char* display_version(int index, char* result, bool autoreturn)
{
    char *buf = NULL;
    struct textview vi; /* version info */
    text_t vi_title;
    int nr_line;
    text_t info;
    int avail_lines = 0;
    text_t rbtn;
    index = 0; // reserved parameter, for resolving warning
    buf = (char *)malloc(BUFSZ);
    init_text(&vi_title, uistr_version, COLOR_YELLOW);
    init_text(&info, buf, COLOR_YELLOW);

    avail_lines = get_avail_textline();
    init_text(&rbtn, uistr_key_back, COLOR_YELLOW);
    ui_init_textview(&vi, textview_key_handler, &vi);
    vi.set_btn(&vi, NULL, NULL, &rbtn);
    create_verinfo(buf);
    LOGD(TAG "after create_verinfo");
    vi.set_title(&vi, &vi_title);
    vi.set_text(&vi, &info);
    vi.m_pstr = trans_verinfo(info.string, &nr_line);
    vi.m_nr_lines = g_nr_lines;
    LOGD(TAG "g_nr_lines is %d, avail_lines is %d\n", g_nr_lines, avail_lines);
    vi.m_start = 0;
    vi.m_end = (nr_line < avail_lines ? nr_line : avail_lines);
    LOGD(TAG "vi.m_end is %d\n", vi.m_end);

    if(autoreturn)
    {
        vi.redraw(&vi);
        strncpy(result, SP_ATA_PASS, strlen(SP_ATA_PASS));
    }
    else
    {
        vi.run(&vi);
    }

    LOGD(TAG "Before tear_down\n");
    tear_down(vi.m_pstr, nr_line);
    if (buf)
    {
        free(buf);
        buf = NULL;
    }
    LOGD(TAG "End of %s\n", __FUNCTION__);
 
    return SP_ATA_PASS;
}
