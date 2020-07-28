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
#include <syslog.h>
#include <malloc.h>
#ifdef VOIP_MEM_FREE_DEBUG
#include <time.h>
#endif

#include "common/api_string.h"
#include "os/api_log.h"
#include "os/api_mem.h"
#include "os/api_mutex.h"


#ifdef VOIP_MEM_FREE_DEBUG
#include <time.h>
#define MAX_MEM_LOG_NUM         (4096 << 2)
#define MAX_DUP_MEM_ALLOCATE    (128)
static int debug_mem_count = 0;
static void *debug_mem_mutex = 0;
static int debug_mem_max = 0;
static struct __ss {
    unsigned int           addr;
    char                  *file;
    unsigned int           line;
    unsigned int           size;
    int                    c;
} debug_mem[MAX_MEM_LOG_NUM];
#endif


int
sip_mem_init() {
#ifdef VOIP_MEM_FREE_DEBUG
    if (debug_mem_mutex == 0) {
        memset(debug_mem, 0, sizeof(debug_mem));
        debug_mem_mutex = malloc(MUTX_Size());
        MUTX_Init(debug_mem_mutex, "mem mutex");
        debug_mem_max = 0;
    }
#endif
    return 0;
}


void
sip_mem_status(char *str) {
#ifdef VOIP_MEM_FREE_DEBUG
    int i = 0;

    DbgMsg("###################################");
    DbgMsg("str = [%s], time = %ld", str, (long int)time(0));
    DbgMsg("latest count : %d, max = %d", debug_mem_count, debug_mem_max);
    for (i=0 ; i<debug_mem_max ; ++i) {
        if (debug_mem[i].addr) {
            DbgMsg("not free [%05d], time = %5d, size = %4d, data = (%x), (%s:%d)", i, debug_mem[i].c, debug_mem[i].size, debug_mem[i].addr, debug_mem[i].file, debug_mem[i].line);
        }
    }
    DbgMsg("###################################");
#endif
}


int
sip_mem_get_free() {
    return 0;
}



void *
voip_get_mem(unsigned int size) {
    void *ptr = 0;

    ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    } else {
        VOIP_ABNORMAL_CHECK(1);
    }

    return ptr;
}


void
voip_free_mem(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}


void *
SIP_get_mem(unsigned int size, char *file, int line) {
#ifdef VOIP_MEM_FREE_DEBUG
    void *ptr = 0;

    sip_mem_init();
    MUTX_Get(debug_mem_mutex, WAIT_FOREVER);
    ptr = voip_get_mem(size);
    if (ptr) {
        int i = 0;
        int write = 0;

        for (i=0 ; i<MAX_MEM_LOG_NUM ; ++i) {
            if (debug_mem[i].addr) {
#if 1
                if ((unsigned int)debug_mem[i].addr == (unsigned int)ptr) {
                    ErrMsg("ERROR %p !!!!!!!!!!!!! %s:%d", ptr, file, line);
                    ErrMsg("DATA  %s:%d", debug_mem[i].file, debug_mem[i].line);
                    VOIP_ASSERT(0, "the address is used");
                }
#endif
            } else {
                    debug_mem[i].addr = (unsigned int)ptr;
                    debug_mem[i].size = size;
                    debug_mem[i].file = file;
                    debug_mem[i].line = line;
                    debug_mem[i].c    = debug_mem_count++;
                    write = 1;
                    if (i > debug_mem_max) {
                        debug_mem_max = i;
                    }
                    break;
            }
        }


        if (write == 0) {
            ErrMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
            ErrMsg("no debug space to save this item, ptr = %p, %s:%d", ptr, file, line);
            ErrMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
            sip_mem_status("ERROR");
            VOIP_ASSERT(0, "no debug space to save this item");
        }
    }
    MUTX_Put(debug_mem_mutex);

    return ptr;
#else
    return voip_get_mem(size);
#endif
}


void
SIP_free_mem(void *ptr, char *file, int line) {
#ifdef VOIP_MEM_FREE_DEBUG
    MUTX_Get(debug_mem_mutex, WAIT_FOREVER);
    if (ptr) {
        int i = 0;
        int clear = 0;
        for (i=0 ; i<=debug_mem_max ; ++i) {
            if ((unsigned int)debug_mem[i].addr == (unsigned int)ptr) {
                debug_mem[i].addr = 0;
                debug_mem[i].size = 0;
                debug_mem[i].file = 0;
                debug_mem[i].line = 0;
                debug_mem[i].c    = 0;
                clear = 1;
                break;
            }
        }

        if (clear == 0) {
            ErrMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
            ErrMsg("memory can't find the record, ptr = %p, %s:%d", ptr, file, line);
            ErrMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
            VOIP_ASSERT(0, "memory can't find the record");
        }
    }
    voip_free_mem(ptr);
    MUTX_Put(debug_mem_mutex);
#else
    voip_free_mem(ptr);
#endif
}


void
SIP_test_free_mem(void *ptr, char *file, int line) {
#ifdef VOIP_MEM_FREE_DEBUG
    MUTX_Get(debug_mem_mutex, WAIT_FOREVER);
    if (ptr) {
        int i = 0;
        int clear = 0;
        for (i=0 ; i<=debug_mem_max ; ++i) {
            if ((unsigned int)debug_mem[i].addr == (unsigned int)ptr) {
                clear = i;
                break;
            }
        }

        if (clear == 0) {
            ErrMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
            ErrMsg("[TEST] memory can't find the record, ptr = %p, %s:%d", ptr, file, line);
            ErrMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        } else {
            DbgMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
            DbgMsg("[TEST] memory can find the record, ptr = %p, i = %d, %s:%d", ptr, clear, file, line);
            DbgMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        }
    }
    MUTX_Put(debug_mem_mutex);
#endif
}
























#if defined _UNIT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int max = 0;

int
test1(void *task_entry_ptr) {
    int i;
    int size;
    char *ptr;

    printf("start !!\r\n");
    while (1) {
        for (i=1 ; i < 1024 * 16 ; ++i) {
            size = i;
            if (size > max) max = size;
            ptr = sip_get_mem(size);
            sip_free_mem(ptr);
        }
    }
    printf("end !!\r\n");

    return 0;
}

int
test2(void *task_entry_ptr) {
    int i;
    int size;
    char *ptr;

    printf("start !!\r\n");
    while (1) {
        for (i=16 ; i < 1024 * 16 ; ++i) {
            size = rand() % i + 1;
            if (size > max) max = size;
            ptr = sip_get_mem(size);
            sip_free_mem(ptr);
        }
    }
    printf("end !!\r\n");

    return 0;
}



int
main() {
    int size;
    char *ptr;

    printf(".... %s %s ....\r\n", __DATE__, __TIME__);

    kal_create_task(0, 10, 0, test1, 0, 0);
    kal_create_task(0, 10, 0, test2, 0, 0);
    kal_create_task(0, 10, 0, test2, 0, 0);

    while (1) {
        printf("size ? ");
        scanf("%d", &size);
        printf("malloc = %d (%x), max = %d\r\n", size, size, max);
        ptr = malloc(size); // unit test
        free(ptr);
    }

    return 0;
}

#elif defined _EXE_

#include <stdio.h>

int
main(void) {
    return 0;
}

#endif
