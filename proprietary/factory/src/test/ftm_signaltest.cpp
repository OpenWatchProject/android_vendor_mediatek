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
#include "at_command.h"

#define TAG "[Signal Test] "

extern int bg_arr[MAX_ROWS];
extern char result[3][16];
pthread_mutex_t M_EIND;
pthread_cond_t  COND_EIND;

pthread_mutex_t M_VPUP;
pthread_cond_t  COND_VPUP;

pthread_mutex_t M_CREG;
pthread_cond_t  COND_CREG;

pthread_mutex_t M_ESPEECH_ECPI;
pthread_cond_t  COND_ESPEECH_ECPI;

pthread_mutex_t M_CONN;
pthread_cond_t  COND_CONN;

pthread_mutex_t M_VPON;
pthread_cond_t  COND_VPON;

pthread_mutex_t M_EUSIM;
pthread_cond_t  COND_EUSIM;

pthread_mutex_t M_UIMST;
pthread_cond_t  COND_UIMST;

Connection modem[5] ;

enum {
    ITEM_CALL_FOR_MODEM_1,
    ITEM_CALL_FOR_MODEM_2,
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t sigtest_items_single_modem[] = {
    item(ITEM_CALL_FOR_MODEM_1,   uistr_info_emergency_call_in_modem1),
    #ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    #endif
    item(-1, NULL),
};

static item_t sigtest_items_multi_modem[] = {
    item(ITEM_CALL_FOR_MODEM_1,   uistr_info_emergency_call_in_modem1),
    item(ITEM_CALL_FOR_MODEM_2,   uistr_info_emergency_call_in_modem2),
    #ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    #endif

    item(-1, NULL),
};

static item_t sigtest_item_auto[] =
{
    item(-1, NULL),
};
struct sigtest {
    char info[1024];
    bool exit_thd;
    int test_type;
    text_t title;
    text_t text;
    pthread_t update_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};

#define mod_to_sigtest(p)  (struct sigtest*)((char*)(p) + sizeof(struct ftm_module))
#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)
#define HALT_INTERVAL 50000
#define MAX_MODEM_INDEX 4

void (*g_SIGNAL_Callback[4])(void(*pdata))={SIGNAL1_Callback,SIGNAL2_Callback,SIGNAL3_Callback,SIGNAL4_Callback};

int wait_SDIO_ready();
void exit_flight_mode_internal(Connection& conn, int md_num)
{
    LOGD(TAG "Entry %s!\n", __FUNCTION__) ;
    LOGD(TAG "md_num == %d %s!\n", md_num, __FUNCTION__) ;
    if ((md_num == 2) && (isC2kSupport() == 0)) {
        ExitFlightMode_DualTalk(conn);
    } else {
        ExitFlightMode(conn);
    }
}

void query_modem_status(Connection& modem)
{
    if(g_Flag_EIND != 1) {
        LOGD(TAG "[AT]Detect modem status:\n");
        if(ER_OK!= modem.QueryModemStatus()) {
            g_Flag_EIND = 0 ;
            wait_URC(ID_EIND);
        } else {
            g_Flag_EIND = 1 ;
        }
    }
}

void md_init(int md_num) {

    int i = 0, j = 0;
    char ccci_path[MAX_MODEM_INDEX][32];
    memset(ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    char temp_ccci_path[MAX_MODEM_INDEX][32];
    memset(temp_ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    //get all ccci path, c2k is the last one if the c2k modem is a independent part for the platform
    for(i = 0; i < MAX_MODEM_INDEX; i++) {
        if(1 == get_ccci_path(i,temp_ccci_path[i])) {
            strncpy(ccci_path[j],temp_ccci_path[i],31);
            LOGD(TAG "ccci_path[%d] is  = %s\n", j, ccci_path[j]);
            j++ ;
        }
    }
    // init all modem, c2k is the last one if the platform contain c2k modem such as 90 and 91
    // c2k modem is not independent modem in 93 md, so it will not init c2k part
    for (int i = 0; i < md_num; i++) {
        if (strlen(ccci_path[i]) > 0) {
            if(0 == modem[i].Conn_Init(ccci_path[i],i,g_SIGNAL_Callback[i])) {
                LOGD(TAG "modem %d open fail",(i));
            } else {
                LOGD(TAG "modem %d open OK",(i));
            }
        }
    }
}

int lte_md_reset(Connection& modem) {
    query_modem_status(modem);
    if(isRLTE_VLTESupport()!=0){
        if(ER_OK!= modem.SetMTKRFMode(0))
            return -1 ;
        wait_URC(ID_EUSIM);
        LOGD(TAG "[AT]For iRat");
        if(ER_OK!= modem.EMDSTATUS())
             return -1;
        }
    return 0;
}

int sigtest_entry(struct ftm_param *param, void *priv)
{
    bool exit = (bool)FALSE;
    int chosen = 0 ;
    struct sigtest *st = (struct sigtest *)priv ;
    struct itemview *iv = NULL;
    int pret = 0 ;
    char ccci_path[MAX_MODEM_INDEX][32];
    memset(ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    char temp_ccci_path[MAX_MODEM_INDEX][32];
    memset(temp_ccci_path, 0, sizeof(char) * MAX_MODEM_INDEX * 32);
    int test_result_temp = TRUE;
    int modem_number = 0;
    int i = 0;//, j = 0;
    int temp_result[MODEM_MAX_NUM] = {0};
    item_t *sigtest_items= NULL;

    LOGD(TAG "%s\n", __FUNCTION__);
    init_COND();
    modem_number = get_md_count();
    LOGD(TAG "modem_number is %d\n",modem_number);
    if(modem_number == 1) {
        sigtest_items = sigtest_items_single_modem;
    } else if(modem_number == 2) {
        sigtest_items = sigtest_items_multi_modem;
    }
    md_init(modem_number);

	    /* set rat to 2G only if L/W/G+G project */
    if (isMDENSupport(1) > 0) {
        handleLPlusGMd1(modem[0]);
    }

    init_text(&st->title, param->name, COLOR_YELLOW);
    init_text(&st->text, &st->info[0], COLOR_YELLOW);
    memset(&st->info[0], 0, sizeof(st->info));
    st->exit_thd = FALSE ;
    st->test_type = param->test_type;

    if (!st->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        st->iv = iv;
    }

    if (FTM_MANUAL_ITEM == param->test_type) {
        iv = st->iv;
        #ifdef FEATURE_FTM_TOUCH_MODE
        text_t lbtn ;
        text_t cbtn ;
        text_t rbtn ;
        init_text(&lbtn, uistr_key_fail, COLOR_YELLOW);
        init_text(&cbtn, uistr_key_back, COLOR_YELLOW);
        init_text(&rbtn, uistr_key_pass, COLOR_YELLOW);
        iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
        #endif
        iv->set_title(iv, &st->title);
        iv->set_text(iv, &st->text);
        sprintf(st->info, "%s\n", uistr_info_emergency_call_not_start);
        iv->set_items(iv, sigtest_items, 0);

        do {
            chosen = iv->run(iv, &exit);
            switch (chosen) {
                case ITEM_CALL_FOR_MODEM_1:
                case ITEM_CALL_FOR_MODEM_2:
                    pret = 0 ;
                    sprintf(st->info, "%s\n", uistr_info_emergency_call_testing);
                    iv->redraw(iv);
                    if (isC2kSupport() && chosen == ITEM_CALL_FOR_MODEM_2) { //md2 is c2k
                        for(i=0 ;i<modem_number-1; i++) // close rf of lte md, the last member of modem[] is c2k
                        {
                            int reset = lte_md_reset(modem[i]);
                            if (reset == -1) {
                                goto exit_func;
                            }
                        }
                        C2Kmodemsignaltest(modem[chosen]);
                        pret = dial112C2K(modem[chosen]);
                        modem[chosen].TurnOffPhone();
                    } else { // md1 and md2 is not c2k
                        exit_flight_mode_internal(modem[chosen], modem_number);
                        pret = dial112(modem[chosen]);
                        if(ER_OK!= modem[chosen].SetNormalRFMode(4))
                            goto exit_func;
                    }
                    if(1 == pret) {
                        LOGD(TAG "Dial 112 Success in modem%d \n", chosen+1);
                        sprintf(st->info, "%s%d\n", uistr_info_emergency_call_success_in_modem, chosen+1);
                    } else {
                        LOGD(TAG "Dial 112 Fail in modem%d \n", chosen+1);
                        sprintf(st->info, "%s%d\n", uistr_info_emergency_call_fail_in_modem, chosen+1);
                    }
                break;
                #ifndef FEATURE_FTM_TOUCH_MODE
                case ITEM_PASS:
                case ITEM_FAIL:
                    if (chosen == ITEM_PASS) {
                        LOGD(TAG "chosen == ITEM_PASS\n");
                        st->mod->test_result = FTM_TEST_PASS;
                    } else if (chosen == ITEM_FAIL) {
                        LOGD(TAG "chosen == ITEM_FAIL\n");
                        st->mod->test_result = FTM_TEST_FAIL;
                    }
                exit = TRUE;
                break;
                #else
                case L_BTN_DOWN:
                    st->mod->test_result = FTM_TEST_FAIL;
                exit = TRUE;
                break;
                    case C_BTN_DOWN:
                exit = TRUE;
                break;
                case R_BTN_DOWN:
                    st->mod->test_result = FTM_TEST_PASS;
                    exit = TRUE;
                break;
                default:
                    exit = TRUE;
                break;
                #endif
            }
            if(exit) {
                break;
            }
        } while (1);
    }
    else if((FTM_AUTO_ITEM == param->test_type) || (FTM_ASYN_ITEM == param->test_type)) {
        iv = st->iv;
        iv->set_title(iv, &st->title);
        iv->set_items(iv, sigtest_item_auto, 0);
        iv->set_text(iv, &st->text);
        iv->start_menu(iv,0);
        iv->redraw(iv);
        for (int i = 0; i < modem_number; i++) {
            sprintf(st->info, "%s in modem%d\n", uistr_info_emergency_call_testing, i+1);
            iv->redraw(iv);
            if ((1 == isC2kSupport()) && (modem_number > 1) && (i == (modem_number-1)))
            { //C2K modem and it's the last member
                g_Flag_CONN = 0 ;
                g_Flag_CREG = 0 ;
                g_Flag_EUSIM  = 0;
                g_Flag_UIMST  = 0;
                pret = 0 ;
                for(int j  =0 ;j < (modem_number-1); j++) {
                    int reset = lte_md_reset(modem[j]);
                    if (reset == -1) {
                        goto exit_func;
                    }
                }
                C2Kmodemsignaltest(modem[i]);
                pret = dial112C2K(modem[i]);
                if(1 == pret) {
                    LOGD(TAG "Dial 112 Success in c2k modem \n");
                    sprintf(st->info, "%s%d\n", uistr_info_emergency_call_success_in_modem, i+1);
                    temp_result[i] = TRUE ;
                } else {
                    LOGD(TAG "Dial 112 Fail in c2k modem \n");
                    sprintf(st->info, "%s%d\n", uistr_info_emergency_call_fail_in_modem, i+1);
                    temp_result[i] = FALSE ;
                }
                modem[i].TurnOffPhone();
                iv->redraw(iv);
            }
			else if (strlen(ccci_path[i]) > 0) {//non c2k modem
                g_Flag_CREG = 0 ;
                g_Flag_ESPEECH_ECPI = 0 ;
                pret = 0 ;
                exit_flight_mode_internal(modem[i], modem_number);
                pret = dial112(modem[i]);
                if(1 == pret) {
                    LOGD(TAG "Dial 112 Success in modem 1\n");
                    sprintf(st->info, "%s%d\n", uistr_info_emergency_call_success_in_modem, i+1);
                    temp_result[i] = TRUE ;
                } else {
                    LOGD(TAG "Dial 112 Fail in modem 1\n");
                    sprintf(st->info, "%s%d\n", uistr_info_emergency_call_fail_in_modem, i+1);
                    temp_result[i] = FALSE ;
                }
                if(ER_OK!= modem[i].SetNormalRFMode(4))
                    goto exit_func;
                iv->redraw(iv);
            }
        }
        for(i = 0 ;i < modem_number ;i++ ) {
            test_result_temp = test_result_temp && temp_result[i] ;
        }
        if(1 == test_result_temp) {
            st->mod->test_result = FTM_TEST_PASS;
        } else {
            st->mod->test_result = FTM_TEST_FAIL;
        }
        iv->redraw(iv);
    }
exit_func:
    for(i=0;i<modem_number;i++) {
        LOGD(TAG "modem %d deinit start \n",i);
        if(1 == modem[i].Conn_DeInit()) {
        	LOGD(TAG "Deinit the port successfully\n");
        } else {
           LOGD(TAG "Deinit the port failed \n");
        }
        LOGD(TAG "modem %d deinit end \n",i);
    }
    deinit_COND();
    return 0;
}

struct ftm_module* sigtest_init(void)
{
    int ret;
    struct ftm_module *mod;
    struct sigtest *st;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_SIGNALTEST, sizeof(struct sigtest));
    st  = mod_to_sigtest(mod);

    st->mod = mod;
    
    if (!mod)
        return NULL;

    ret = ftm_register(mod, sigtest_entry, (void*)st);

    if(ret == 0)
    {
        LOGD(TAG "ftm_register success!\n");
        return mod;
    }
    else
    {
        LOGD(TAG "ftm_register fail!\n");
        return NULL;
    }
}

void init_COND()
{
    pthread_mutex_init(&M_EIND,0);
    pthread_cond_init(&COND_EIND,0);
    pthread_mutex_init(&M_CREG,0);
    pthread_cond_init(&COND_CREG,0);
    pthread_mutex_init(&M_ESPEECH_ECPI,0);
    pthread_cond_init(&COND_ESPEECH_ECPI,0);
    pthread_mutex_init(&M_CONN,0);
    pthread_cond_init(&COND_CONN,0);
    pthread_mutex_init(&M_VPUP,0);
    pthread_cond_init(&COND_VPUP,0);
    pthread_mutex_init(&M_VPON,0);
    pthread_cond_init(&COND_VPON,0);
    pthread_mutex_init(&M_EUSIM,0);
    pthread_cond_init(&COND_EUSIM,0);
    pthread_mutex_init(&M_UIMST,0);
    pthread_cond_init(&COND_UIMST,0);
}

void deinit_COND()
{
    pthread_cond_destroy(&COND_EIND);
    pthread_mutex_destroy(&M_EIND);
    pthread_cond_destroy(&COND_CREG);
    pthread_mutex_destroy(&M_CREG);
    pthread_cond_destroy(&COND_ESPEECH_ECPI);
    pthread_mutex_destroy(&M_ESPEECH_ECPI);
    pthread_cond_destroy(&COND_CONN);
    pthread_mutex_destroy(&M_CONN);
    pthread_cond_destroy(&COND_VPUP);
    pthread_mutex_destroy(&M_VPUP);
    pthread_cond_destroy(&COND_VPON);
    pthread_mutex_destroy(&M_VPON);
    pthread_cond_destroy(&COND_EUSIM);
    pthread_mutex_destroy(&M_EUSIM);
    pthread_cond_destroy(&COND_UIMST);
    pthread_mutex_destroy(&M_UIMST);
}
int wait_SDIO_ready()
{
    int ret = 0;
    char value[100] = {0};
    while(1)
    {
        usleep(2000);
        ret = property_get("net.cdma.mdmstat",value,NULL);
        LOGD(TAG "wait_SDIO_ready ret = %d",ret);
        if (ret >0)
        {
            if (!strncmp(value,"ready",5))
            {
                LOGD(TAG "Get SDIO ready");
                break ;
            }
        }
    }
    return 1;
}

