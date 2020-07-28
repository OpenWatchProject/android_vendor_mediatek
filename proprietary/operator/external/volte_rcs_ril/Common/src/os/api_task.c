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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "sys_def.h"
#include "os/api_log.h"
#include "os/api_mem.h"
#include "os/api_task.h"
#include "os/api_btimer.h"


static int TASK_Free(void *o);
typedef void (*os_task_entry_fn_t) (void *);

typedef struct _os_task {
    pthread_t           tid;
    pthread_attr_t      attr;
    os_task_entry_fn_t  entry_fn;
    void               *arg;
} os_task_t;


static void *
task_stub(void *arg) {
    os_task_t* o = (os_task_t *)arg;
    //pthread_setspecific(task_key, o);
    o->entry_fn(o->arg);

    return 0;
}


static void *
TASK_Init(os_task_entry_fn_t func, void *arg, int priority) {
    os_task_t  *task = 0;
    int         ret = 0;

    task = (os_task_t *)sip_get_mem(sizeof(os_task_t));
    if (!task) {
        ErrMsg("can't allocate the memory");
        return 0;
    }

    task->entry_fn = func;
    task->arg      = arg;

    pthread_attr_init (&(task->attr));
    pthread_attr_setdetachstate(&(task->attr), PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&(task->tid), &(task->attr), task_stub, task);
    if (ret < 0) {
        ErrMsg("pthread_create error");
        goto err_free;
    }

    return task;

err_free :
    sip_free_mem(task);
    return 0;
}


typedef struct _os_task2 {
    os_task_t          *task;
    void               *arg;
    os_task_entry_fn_t  entry_fn;
    os_task_entry_fn_t  completed_fn;
} os_task2_t;

static void
task_stub2(void *arg) {
    os_task2_t* task = (os_task2_t *)arg;

    DbgMsg("task_stub2 start");
    if (task->entry_fn) {
        task->entry_fn( task->arg );
    }

    if (task->completed_fn) {
        task->completed_fn( task->arg );
    }
    TASK_Free( task->task );
    sip_free_mem(task);
}


int
TASK_Run(os_task_entry_fn_t entry_fn, os_task_entry_fn_t completed_fn, void *arg, int priority) {
    os_task2_t *task = (os_task2_t *)sip_get_mem(sizeof(os_task2_t));

    if (!task) {
        return -1;
    }

    task->entry_fn     = entry_fn;
    task->arg          = arg;
    task->completed_fn = completed_fn;
    task->task         = TASK_Init(task_stub2, task, priority);

    return 0;
}


static int
TASK_Free(void *o) {
    os_task_t  *task = (os_task_t *)o;
    if (task) {
#if 0
        int kill_rc = pthread_kill((task->tid), 0);

        if (kill_rc != ESRCH && kill_rc != EINVAL) {
            pthread_kill((task->tid), SIGQUIT);
        }
#endif
        sip_free_mem(task);
    }
    return 0;
}


static int
TASK_Sleep(unsigned tick){
    struct timespec req, rem;
    req.tv_sec = tick / HZ;
    tick %= HZ;
    req.tv_nsec = 1000000000/HZ*tick;

    while(nanosleep(&req, &rem)){
        if(errno != EINTR) break;
        req.tv_sec = rem.tv_sec;
        req.tv_nsec = rem.tv_nsec;
    }
    return 0;
}



/**
 * @brief init a task
 * @param o task object
 * @param name the name of the task
 * @param entry_function the entry point of the task
 * @param arg the argument which will be passed to entry function
 * @param stack_start the start address of stack
 * @param stack_sz the stack size
 * @param priority the task priority
 */
void *
voip_task_create(char* task_name, unsigned char priority, unsigned int stack_size, voip_task_fn_t entry_func, void* entry_param, unsigned char options) {
    void *task_ptr = 0;
   // char    *stack_ptr = 0;

    VOIP_ASSERT(0 != entry_func,  "invalid argument-[IN]entry_func");

    // create memory for ctask & stack
    //task_ptr = (ctask_t *)SYS_InitAlloc(TASK_Size());
    task_ptr = TASK_Init(entry_func, entry_param, priority);
    VOIP_ASSERT(0 != task_ptr, "TASK_Init failed");
    //TASK_Resume(task_ptr);

    return task_ptr;
}


void
voip_task_destroy(void *task_ptr) {
    VOIP_ASSERT(0 != task_ptr, "invalid argument-[IN]ext_task_id");
    TASK_Free(task_ptr);
}


void
voip_task_sleep(unsigned int time_in_ticks) {
    TASK_Sleep( time_in_ticks );
}


#if _UNIT_


void
task_main(void *task_entry_ptr) {
    char *ptr = 0;
    int count = 0;

    while (1) {
        printf("count = %d\r\n", count++);
    }

}


int main(int argc , char **argv)
{
    void *task_id = 0;

    if (argc > 1) {
        int size = strtoul(argv[1], 0, 10);
        printf("stack size = %d\r\n", size);
        task_id = voip_task_create(0, 10, size, task_main, 0, 0);
    }

    while (1) {
        sleep(1);
    }

    return 0;
}

#elif _EXE_
int main()
{
    return 0;
}
#endif
