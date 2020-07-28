/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "common/api_string.h"
#include "os/api_log.h"
#include "os/api_mem.h"

int
voip_strlen(char *str) {
    return ((str)?strlen(str):0);
}


int
voip_strncpy(char *dest, const char *src, int len) {
    int tmp_len;
    if (src) {
        tmp_len = strlen(src);
        if (tmp_len > 0 && len > 0) {
            if (tmp_len >= len) {
                tmp_len = (len-1);
            }
            memcpy(dest, src, tmp_len);
            *(dest+tmp_len) = '\0';
            return tmp_len;
        }
    }
    return 0;
}


char *
VOIP_strdup(const char *str, char *file, int line) {
    char *ptr;
    unsigned int len;

    VOIP_CHECK_AND_RETURN_VAL((!str || !*str), 0);
    len = strlen(str) + 1;
    //ptr = (char *)sip_get_mem(len);
    ptr = (char*) SIP_get_mem(len, file, line);
    VOIP_CHECK_AND_RETURN_VAL((!ptr), 0);
    //memcpy(ptr, str, len);
    strncpy(ptr, str, len);
    return (ptr);
}


signed short
voip_strcasecmp(const char *s1, const char *s2) {
    int ret = strcasecmp(s1, s2);
    return (ret>0)?1:((ret<0)?-1:0);
}


char *
voip_strtok_r(char *str, const char *delim, char **ptrptr) {
    char *ret;

    VOIP_CHECK_AND_RETURN_VAL((!ptrptr), 0);
    if (!str) {
        str = *ptrptr;
    }
    VOIP_CHECK_AND_RETURN_VAL((!str), 0);

    while (*str && strchr(delim, *str)) {
        ++str;
    }
    VOIP_CHECK_AND_RETURN_VAL((!(*str)), 0);

    ret = str;
    while (*str && !strchr(delim, *str)) {
        ++str;
    }

    if (*str) {
        *str++ = 0;
    }
    *ptrptr = str;

    return ret;
}


char *
voip_char_ucase(char *s) {
    char *t;

    for (t = s; *t; t++) {
        if (*t >= 'a' && *t <= 'z') {
            *t &= 0xDF;
        }
    }
    return s;
}


char *
voip_char_lcase(char *s) {
    char *t;

    for (t = s; *t; t++) {
        if (*t >= 'A' && *t <= 'Z') {
            *t |= 0x20;
        }
    }
    return s;
}


char *
voip_triml(char *str) {
    return (str + strspn(str, " \t"));
}


char *
voip_trimr(char *str) {
    char *ptr = str + strlen(str);
    while ((--ptr > str) && (' ' == *ptr || '\t' == *ptr)) {
        *ptr = 0;
    }
    return str;
}


char *
voip_trim(char *str) {
    return voip_trimr(voip_triml(str));
}


int
voip_snprintf(char *buffer, int len, const char* fmt, ...) {
    int     tmp_len;
    va_list ap;

    VOIP_CHECK_AND_RETURN_VAL(len<=0, -1);

    va_start(ap, fmt);
    tmp_len = vsnprintf(buffer, len, fmt, ap);
    va_end(ap);

    return (tmp_len >= len) ? -1 : tmp_len;
}

char *
voip_alloc_sprintf(const char* fmt, ...) {
    int     string_len;
#define VOIP_GUESS_TEMP_BUFFER_SIZE 100
    char    temp_buffer[VOIP_GUESS_TEMP_BUFFER_SIZE];
    char    *new_string = NULL;
    va_list ap;

    va_start(ap, fmt);
    string_len = vsnprintf(&temp_buffer, VOIP_GUESS_TEMP_BUFFER_SIZE, fmt, ap);
    va_end(ap);
    if (string_len >= 0) {
        string_len++;
        new_string = sip_get_mem(string_len);
        if (new_string) {
            if (string_len <= VOIP_GUESS_TEMP_BUFFER_SIZE) {
                (void)memcpy(new_string, temp_buffer, string_len);
            } else {
                va_start(ap, fmt);
                (void)vsnprintf(new_string, string_len, fmt, ap);
                va_end(ap);
            }
        }
    }
    return new_string;
}

/*
    mode : 0, remove []
    mode : 1, keep []
*/
int
voip_parser_address_str(char *addr_str, char **addr, int *port, int mode) {
    char *ptr = 0, *tmp = 0, *tmp_addr = 0;

    if (!addr_str) {
        return -1;
    }

    if (!addr || !port) {
        return -2;
    }

    ptr = (addr_str + strspn(addr_str, " \t"));
    if (*ptr == '[') {
        int addr_len = 0;

        tmp = strchr(ptr+1, ']');
        if (!tmp) {
            return -3;
        }

        addr_len = (tmp-ptr);
        if (mode) {
            addr_len += 2;
        }

        tmp_addr = sip_get_mem(addr_len);
        if (!(tmp_addr)) {
            return -4;
        }

        if (mode) {
            strncpy(tmp_addr, ptr, addr_len-1);
        } else {
            strncpy(tmp_addr, ptr+1, addr_len-1);
        }
        tmp_addr[addr_len-1] = 0;
        *addr = tmp_addr;

        ptr = strchr(tmp, ':');
        if (ptr) {
            *port = strtoul(ptr+1, 0, 10);
        }

        return 0;
    } else {
        tmp = strchr(ptr, ':');

        if (tmp) {
            if (strchr(tmp+1, ':')) {
                // more than one ':' ==>  only address
                tmp_addr = voip_strdup(ptr);
                if (!(tmp_addr)) {
                    return -4;
                }
                *addr = tmp_addr;
            } else {
                // only one ':' ==> address:port
                tmp_addr = sip_get_mem(tmp-ptr+1);
                if (!(tmp_addr)) {
                    return -4;
                }

                strncpy(tmp_addr, ptr, tmp-ptr);
                tmp_addr[tmp-ptr] = 0;
                *addr = tmp_addr;
                *port = strtoul(tmp+1, 0, 10);
            }
            return 0;
        } else {
            // without ':' ==> only address
            tmp_addr = voip_strdup(ptr);
            if (!(tmp_addr)) {
                return -4;
            }
            *addr = tmp_addr;
            return 0;
        }
    }

}



#if defined _UNIT_
#include <base/utest.h>
#include <stdio.h>
#include <string.h>

exc_t
case_voip_strcasecmp(void *p) {
    int ret;

    ret = voip_strcasecmp("ABCDEF", "ABCDE");
    UTST(ret == 1);
    ret = voip_strcasecmp("BBCDE", "ABCDE");
    UTST(ret == 1);
    ret = voip_strcasecmp("ABCDE", "ABCDEF");
    UTST(ret == -1);
    ret = voip_strcasecmp("ABCDE", "BBCDE");
    UTST(ret == -1);
    ret = voip_strcasecmp("ABCDE", "ABCDE");
    UTST(ret == 0);

    ret = voip_strcasecmp("AbcDEF", "ABCDE");
    UTST(ret == 1);
    ret = voip_strcasecmp("AbcDE", "ABCDEF");
    UTST(ret == -1);
    ret = voip_strcasecmp("AbcDE", "ABCDE");
    UTST(ret == 0);

  CERR_Ok();
}


exc_t
case_voip_strncpy(void *p) {
    char buffer1[16], buffer2[16];
    int len;

    strcpy(buffer1, "ABCDEFGH");
    memset(buffer2, 0, sizeof(buffer2));
    voip_strncpy(buffer2, buffer1, 16);
    UTST(strcmp(buffer1, buffer2) == 0);

    strcpy(buffer1, "ABCDEFGH");
    memset(buffer2, 0, sizeof(buffer2));
    len = strlen(buffer1);
    voip_strncpy(buffer2, buffer1, len);
    buffer1[len-1] = 0;
    UTST(strcmp(buffer1, buffer2) == 0);

    strcpy(buffer1, "ABCDEFGH");
    memset(buffer2, 0, sizeof(buffer2));
    len = 4;
    voip_strncpy(buffer2, buffer1, len);
    buffer1[len-1] = 0;
    UTST(strcmp(buffer1, buffer2) == 0);

    strcpy(buffer1, "12345667");
    strcpy(buffer2, buffer1);
    voip_strncpy(buffer2, "AAAA", 0);
    UTST(strcmp(buffer1, buffer2) == 0);

    strcpy(buffer1, "12345667");
    strcpy(buffer2, buffer1);
    voip_strncpy(buffer2, 0, 10);
    UTST(strcmp(buffer1, buffer2) == 0);

    strcpy(buffer1, "12345667");
    strcpy(buffer2, buffer1);
    voip_strncpy(buffer2, "", 10);
    UTST(strcmp(buffer1, buffer2) == 0);

    CERR_Ok();
}


exc_t
case_voip_char_ucase(void *p) {
    char *ptr;
    char buffer[128];

    strcpy(buffer, "ABCDEFG");
    ptr = voip_char_ucase(buffer);
    UTST(strcmp(ptr, buffer)==0);
    UTST(strcmp(ptr, "ABCDEFG")==0);

    strcpy(buffer, "AbCDeFG");
    ptr = voip_char_ucase(buffer);
    UTST(strcmp(ptr, buffer)==0);
    UTST(strcmp(ptr, "ABCDEFG")==0);

    strcpy(buffer, "abcdefg");
    ptr = voip_char_ucase(buffer);
    UTST(strcmp(ptr, buffer)==0);
    UTST(strcmp(ptr, "ABCDEFG")==0);

    CERR_Ok();
}


exc_t
case_voip_char_lcase(void *p) {
    char *ptr;
    char buffer[128];

    strcpy(buffer, "ABCDEFG");
    ptr = voip_char_lcase(buffer);
    UTST(strcmp(ptr, buffer)==0);
    UTST(strcmp(ptr, "abcdefg")==0);

    strcpy(buffer, "AbCDeFG");
    ptr = voip_char_lcase(buffer);
    UTST(strcmp(ptr, buffer)==0);
    UTST(strcmp(ptr, "abcdefg")==0);

    strcpy(buffer, "abcdefg");
    ptr = voip_char_lcase(buffer);
    UTST(strcmp(ptr, buffer)==0);
    UTST(strcmp(ptr, "abcdefg")==0);

    CERR_Ok();
}


exc_t
case_voip_triml(void *p) {
    char *token = "Hello world";
    char buf[32];
    char *ptr;

    sprintf(buf, " \t  %s", token);
    ptr = voip_triml( buf ) ;
    UTST(strcmp(ptr, token) == 0);

    sprintf(buf, "%s", token);
    ptr = voip_triml( buf ) ;
    UTST(strcmp(ptr, token) == 0);

    CERR_Ok();
}


exc_t
case_voip_trimr(void *p) {
    char *token = "Hello world";
    char buf[32];
    char *ptr;

    sprintf(buf, "%s  \t ", token);
    ptr = voip_trimr( buf ) ;
    UTST(strcmp(ptr, token) == 0);

    sprintf(buf, "%s", token);
    ptr = voip_trimr( buf ) ;
    UTST(strcmp(ptr, token) == 0);

    CERR_Ok();
}


exc_t
case_voip_trim(void *p) {
    char *token = "Hello world";
    char buf[32];
    char *ptr;

    sprintf(buf, " \t  %s  \t ", token);
    ptr = voip_trim( buf ) ;
    UTST(strcmp(ptr, token) == 0);

    sprintf(buf, " \t  %s", token);
    ptr = voip_trim( buf ) ;
    UTST(strcmp(ptr, token) == 0);

    sprintf(buf, "%s  \t ", token);
    ptr = voip_trim( buf ) ;
    UTST(strcmp(ptr, token) == 0);

    sprintf(buf, "%s", token);
    ptr = voip_trim( buf ) ;
    UTST(strcmp(ptr, token) == 0);

    CERR_Ok();
}


exc_t
case_voip_snprintf(void *p) {
    char *data = "0123456789";
    char buf[128];
    int  len, size, i;

    /* buf > data */
    memset(buf, 'z', sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    len = voip_snprintf(buf, sizeof(buf), data);
    UTST(len == strlen(data));
    UTST(strlen(buf) == strlen(data));
    UTST(strcmp(buf, data) == 0);


    /* buf == strlen(data) */
    size = strlen(data);
    memset(buf,'z', sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    len = voip_snprintf(buf, size, data);
    UTST(len < 0); // error
    UTST(strlen(buf) == size-1);
    UTST(strncmp(buf, data, size-1) == 0);

    /* buf == strlen(data) + null */
    size = strlen(data) + 1;
    memset(buf,'z', sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    len = voip_snprintf(buf, size, data);
    UTST(len == size-1);
    UTST(strlen(buf) == size-1);
    UTST(strcmp(buf, data) == 0);

    /* buf > data */
    for (i=0 ; i<3 ; ++i) {
        size = strlen(data) + 1;
        memset(buf,'z', sizeof(buf));
        buf[sizeof(buf)-1] = 0;
        len = voip_snprintf(buf, size+i, data);
        UTST(len == size-1);
        UTST(strlen(buf) == size-1);
        UTST(strcmp(buf, data) == 0);
    }

    /* buf < strlen(data) */
    size = strlen(data)-1;
    memset(buf,'z', sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    len = voip_snprintf(buf, size, data);
    UTST(len < 0);
    UTST(strlen(buf) == size-1);
    UTST(strncmp(buf, data, size-1) == 0);

    /* buf == 1 */
    size = 1;
    memset(buf,'z', sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    len = voip_snprintf(buf, size, data);
    UTST(len < 0);
    UTST(strlen(buf) == 0);
    UTST(buf[0] == 0);

    /* size == 0 */
    memset(buf,'z', sizeof(buf));
    len = voip_snprintf(buf, 0, data);
    UTST(len < 0); // error
    UTST(buf[0] == 'z');

    /* size < 0 */
    for (i=-3 ; i<0 ; ++i) {
        memset(buf,'z', sizeof(buf));
        len = voip_snprintf(buf, i, data);
        UTST(len < 0); // error
        UTST(buf[0] == 'z');
    }

    CERR_Ok();
}


exc_t
case_voip_parser_dialing_str(void *p) {
    char pattern[128];
    char user[128];
    char server[128];
    int ret;

    memset(user, 0, sizeof(user));
    memset(server, 0, sizeof(server));
    strcpy(pattern, "1000#192*168*0*123");
    printf("[Y] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 1);
    UTST(strcmp(user, "1000") == 0);
    UTST(strcmp(server, "192.168.0.123") == 0);

    memset(user, 0, sizeof(user));
    memset(server, 0, sizeof(server));
    strcpy(pattern, "1000#192*168*0*5#5060");
    printf("[Y] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 1);
    UTST(strcmp(user, "1000") == 0);
    UTST(strcmp(server, "192.168.0.5:5060") == 0);

    strcpy(pattern, "1000");
    printf("[N] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 0);

    strcpy(pattern, "192.168.0.3");
    printf("[N] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 0);

    strcpy(pattern, "192.168.0.3#5060");
    printf("[N] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 0);

    strcpy(pattern, "1000#192*16*8*0*5#5060");
    printf("[N] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 0);

    strcpy(pattern, "1000#192*168*0*5##");
    printf("[N] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 0);

    strcpy(pattern, "1000#192*168*0*5#123#");
    printf("[N] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 0);

    strcpy(pattern, "1000#192*168*0*5#123*");
    printf("[N] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 0);

    strcpy(pattern, "1000#192*168*0*5#*");
    printf("[N] : [%s]\r\n", pattern);
    ret = voip_parser_dialing_str(pattern, user, server);
    UTST(ret == 0);


    CERR_Ok();
}



int main(void)
{
    printf("%s - %s\r\n", __DATE__, __TIME__);

    UTST_Add(case_voip_strcasecmp);
    UTST_Add(case_voip_strncpy);
    UTST_Add(case_voip_char_ucase);
    UTST_Add(case_voip_char_lcase);
    UTST_Add(case_voip_triml);
    UTST_Add(case_voip_trimr);
    UTST_Add(case_voip_trim);
    UTST_Add(case_voip_snprintf);
    UTST_Add(case_voip_parser_dialing_str);

    UTST_Run(0);

    return 0;
}

#endif
