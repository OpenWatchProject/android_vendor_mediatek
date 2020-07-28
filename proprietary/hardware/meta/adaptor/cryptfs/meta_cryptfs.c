/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_fm.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   FM meta implement.
 *
 * Author:
 * -------
 *  LiChunhui (MTK80143)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 03 12 2012 vend_am00076
 * [ALPS00251394] [Patch Request]
 * .
 *
 * 03 02 2012 vend_am00076
 * NULL
 * .
 *
 * 01 26 2011 hongcheng.xia
 * [ALPS00030208] [Need Patch] [Volunteer Patch][MT6620 FM]enable FM Meta mode
 * .
 *
 * 11 18 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 16 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 15 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 15 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 08 28 2010 chunhui.li
 * [ALPS00123709] [Bluetooth] meta mode check in
 * for FM meta enable

 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/reboot.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <log/log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>

#include "MetaPub.h"
#include "meta_cryptfs_para.h"
#include "PortHandle.h"

#define LOGD ALOGD
#define LOGE ALOGE

#undef  LOG_TAG
#define LOG_TAG  "CRYPTFS_META"

int do_cmd_for_cryptfs(char* cmd);
int do_monitor_for_cryptfs(int sock, int stop_after_cmd);
/********************************************************************************
//FUNCTION:
//		META_CRYPTFS_init
//DESCRIPTION:
//		CRYPTFS Init for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		true : success
//      false: failed
//
********************************************************************************/
bool META_CRYPTFS_init()
{
	LOGD("META_CRYPTFS_init ...\n");
	return 1;
}

/********************************************************************************
//FUNCTION:
//		META_CRYPTFS_deinit
//DESCRIPTION:
//		CRYPTFS deinit for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//
********************************************************************************/
void META_CRYPTFS_deinit()
{
	LOGD("META_CRYPTFS_deinit ...\n");
	return;
}

#define DEFAULT_ENCRYPTION_RETRIES 60
void wait_for_vold_decrypt(char* expected_status1, char* expected_status2){
    char vold_decrypt[PROPERTY_VALUE_MAX];
    int i = 0;

    for(i = 0; i < DEFAULT_ENCRYPTION_RETRIES; i++ ) {
       property_get("vold.decrypt", vold_decrypt, "");
       if(expected_status1 && !strcmp(vold_decrypt, expected_status1)) {
          LOGD("vold_decryp wait well. It is already (%s)\n", expected_status1);
          return;
       }
       if(expected_status2 && !strcmp(vold_decrypt, expected_status2)) {
           LOGD("vold_decryp wait well. It is already (%s)\n", expected_status2);
           return;
       }
       sleep(1);
    }
    if(expected_status1) {
       LOGD("vold_decryp wait fail. expected(%s), but still (%s) \n", expected_status1, vold_decrypt);
    }
    if(expected_status2) {
       LOGD("vold_decryp wait fail. expected(%s), but still (%s) \n", expected_status2, vold_decrypt);
    }
}

int get_encrypt_phone_status()
{
    char crypto_state[PROPERTY_VALUE_MAX];
    char crypto_type[PROPERTY_VALUE_MAX];
    char vold_decrypt[PROPERTY_VALUE_MAX];

    property_get("ro.crypto.state", crypto_state, "");
    property_get("ro.crypto.type", crypto_type, "");
    property_get("vold.decrypt", vold_decrypt, "");

    LOGD("crypto_state=(%s), crypto_type=(%s), vold_decrypt=(%s)\n", crypto_state, crypto_type, vold_decrypt);
    if (!strcmp(crypto_state, "")){
       LOGD("We don't encrypt the userdata in meta mode, and treat it as 'unencrypted' \n");
    }
    else if (!strcmp(crypto_state, "encrypted")){
        if (!strcmp(crypto_type, "file")) {
            LOGD("it is FBE, then don't need to popup input box to decrypt \n");
            return 0;
        }

        wait_for_vold_decrypt("trigger_restart_framework",  "trigger_restart_min_framework");
        property_get("vold.decrypt", vold_decrypt, "");

        LOGD("vold_decrypt=(%s)\n", vold_decrypt);
        if(!strcmp(vold_decrypt, "trigger_restart_framework")) {
           /* it already decrypt */
        }
        if(!strcmp(vold_decrypt, "trigger_restart_min_framework")) {
           /* nvdata was already moved out of /data.
              Thus we don't need to decrypt /data in meta when using FDE with passwd. */
           return 0;
        }
    }
    else if (!strcmp(crypto_state, "unencrypted")){
        /* for the device without encryption, just return status directly */

    }
    else {
       /* un-supported status ?? */
    }

    /* it means the meta doens't need to popup dialog */
    return 0;
}

void toHex(char *passwd, char *hex){
    int i = 0;
    int len = strlen(passwd);
    for(i = 0; i < len; i++) {
        hex += sprintf(hex, "%x", passwd[i]);
    }
}

int decrypt_data(char *passwd)
{
    int rtn=0;
    char cmd[255] = {'\0'};

    snprintf(cmd, sizeof(cmd), "cryptfs checkpw %s", passwd);
    rtn = do_cmd_for_cryptfs(cmd);
    if (rtn) {
      LOGE("Fail: cryptfs checkpw, err=%d\n", rtn);
      return  rtn;
    }

    rtn = do_cmd_for_cryptfs("cryptfs restart");
     if (rtn) {
       LOGE("Fail: cryptfs restart, err=%d\n", rtn);
       return  rtn;
     }

    return 0;
}

int do_cmd_for_cryptfs(char* cmd) {

    int ret;
    int sock;
    char final_cmd[255] = "0 "; /* 0 is a (now required) sequence number */
    ret = strlcat(final_cmd, cmd, sizeof(final_cmd));
    if (ret >= (int)sizeof(final_cmd)) {
        LOGE("Fail: the cmd is too long (%s)", final_cmd);
        return (-1);
    }

    if ((sock = socket_local_client("cryptd",
                                     ANDROID_SOCKET_NAMESPACE_RESERVED,
                                     SOCK_STREAM)) < 0) {
        LOGE("Error connecting (%s)\n", strerror(errno));
        exit(4);
    }
    LOGD("do_cmd_for_cryptfs: %s\n", final_cmd);

    if (write(sock, final_cmd, strlen(final_cmd) + 1) < 0) {
        LOGE("Fail: write socket");
        close(sock);
        return errno;
    }

    ret = do_monitor_for_cryptfs(sock, 1);
    close(sock);

    return ret;
}

int do_monitor_for_cryptfs(int sock, int stop_after_cmd) {
    char *buffer = malloc(4096);

    if (!stop_after_cmd)
        LOGD("[Connected to Vold]\n");

    while(1) {
        fd_set read_fds;
        struct timeval to;
        int rc = 0;

        to.tv_sec = 10;
        to.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);

        if ((rc = select(sock +1, &read_fds, NULL, NULL, &to)) < 0) {
            LOGE("Error in select (%s)\n", strerror(errno));
            free(buffer);
            return errno;
        } else if (!rc) {
            continue;
            LOGE("[TIMEOUT]\n");
            return ETIMEDOUT;
        } else if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, 4096);
            if ((rc = read(sock, buffer, 4096)) <= 0) {
                if (rc == 0)
                    LOGE("Lost connection to Vold - did it crash?\n");
                else
                    LOGE("Error reading data (%s)\n", strerror(errno));
                free(buffer);
                if (rc == 0)
                    return ECONNRESET;
                return errno;
            }

            int offset = 0;
            int i = 0;

            for (i = 0; i < rc; i++) {
                if (buffer[i] == '\0') {
                    int code;
					int rtn_code= -1;
                    char tmp[4];
                    char* token;

                    strlcpy(tmp, buffer + offset, sizeof(tmp));
                    code = atoi(tmp);

                    LOGD("'%s'\n", buffer + offset);
                    if (stop_after_cmd) {
                        if (code >= 200 && code < 600) {
                            if (code == 200) {
                                if (strlen(buffer+offset) > 4) {
                                   token = strtok(buffer+offset+4, " ");
								   token = strtok(NULL, " ");
                                   rtn_code = atoi(token);
                                }
                                LOGD("cryptfs cmd, rtn_code=%d\n", rtn_code);
                                free(buffer);
                                return rtn_code;
                            }
                            else {
                                LOGE("invalid cryptfs cmd \n");
                                free(buffer);
                                return -1;
                            }
                        }
                    }
                    offset = i + 1;
                }
            }
        }
    }
    free(buffer);
    return 0;
}

/********************************************************************************
//FUNCTION:
//		META_CRYPTFS_OP
//DESCRIPTION:
//		META CRYPTFS test main process function.
//
//PARAMETERS:
//
//RETURN VALUE:
//		void
//
********************************************************************************/
void META_CRYPTFS_OP(FT_CRYPTFS_REQ *req)
{
	LOGD("req->op:%d\n", req->op);
	FT_CRYPTFS_CNF cryptfs_cnf;
	memcpy(&cryptfs_cnf, req, sizeof(FT_H) + sizeof(CRYPTFS_OP));
	cryptfs_cnf.header.id ++;
	switch (req->op) {
	      case CRYPTFS_OP_QUERY_STATUS:
              {
                  bool encrypted_status = 0;
				  cryptfs_cnf.m_status = META_SUCCESS;
				  encrypted_status = get_encrypt_phone_status();
				  LOGD("encrypted_status:%d \n", encrypted_status);
		    	  cryptfs_cnf.result.query_status_cnf.status = encrypted_status;
				  WriteDataToPC(&cryptfs_cnf, sizeof(FT_CRYPTFS_CNF), NULL, 0);
              }
			break;

	      case CRYPTFS_OP_VERIFY:
              {
                  char* pw = (char*)req->cmd.verify_req.pwd;
				  int pw_len = req->cmd.verify_req.length;

				  cryptfs_cnf.m_status = META_SUCCESS;
	              LOGD("pw = %s, pw_len = %d \n", pw, pw_len);
                  if (pw_len < 4  || pw_len > 16) {
					  cryptfs_cnf.result.verify_cnf.decrypt_result = 0;
					  LOGE("Invalid passwd length =%d \n", pw_len);
					  WriteDataToPC(&cryptfs_cnf, sizeof(FT_CRYPTFS_CNF), NULL, 0);
                      break;
                  }

				  if(!decrypt_data(pw)) {
				     cryptfs_cnf.result.verify_cnf.decrypt_result = 1;
	              }
	              else {
	                cryptfs_cnf.result.verify_cnf.decrypt_result = 0;
	              }

				  LOGD("verify result:%d \n", cryptfs_cnf.result.verify_cnf.decrypt_result);
				  WriteDataToPC(&cryptfs_cnf, sizeof(FT_CRYPTFS_CNF), NULL, 0);
              }
			break;

	      default:
            LOGE("Error: unsupport op code = %d\n", req->op);
			break;
	}
}

