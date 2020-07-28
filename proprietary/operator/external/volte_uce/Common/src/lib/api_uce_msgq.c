#include <pthread.h>
#include <linux/param.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "api_uce_msgq.h"
#include "api_uce_mem.h"
#include "debug.h"

#include <android/log.h>

enum
{
    MSG_STATE_UNUSED = 0,
    MSG_STATE_USED = 1,
    MSG_STATE_NOT_READY
};

typedef struct _os_msg
{
    int priority;
    void *data;
} os_msg_t;

typedef struct _os_msgq
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int size;
    int count;
    int timer_count;
    int read;
    int write;
    //unsigned int       *msg;
    //char               *valid;
    os_msg_t *queue;
    char name[8];
    RCS_CMSGQ_callback func;
} os_msgq_t;

#ifdef DELAY_TIMER_ENABLED
#include "os/api_btimer.h"

typedef struct _os_delay_msg
{
    void *timer;
    void *msgq;
    void *msg;
    int priority;
} os_delay_msg_t;


/*****************************************************************************
 * FUNCTION
 *  msgq_expire_handler
 * DESCRIPTION
 *
 * PARAMETERS
 *  data        [?]
 * RETURNS
 *  void
 *****************************************************************************/
static void msgq_expire_handler(void *data)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    os_delay_msg_t *msg = (os_delay_msg_t*) data;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    while (msg)
    {
        os_msgq_t *msgq = (os_msgq_t*) msg->msgq;

        if (!msgq)
        {
            break;
        }

        pthread_mutex_lock(&(msgq->mutex));
        msgq->timer_count--;
        if (msgq->timer_count + msgq->count < msgq->size)
        {
            int count = 0;
            int index = 0;
            int priority = msg->priority;
            void *data = msg->msg;
            int priority_org = priority;

            index = msgq->read;
            count = msgq->count;
            while (count-- > 0)
            {
                if (priority_org > msgq->queue[index].priority)
                {
                    void *tmp_data = msgq->queue[index].data;
                    int tmp_priority = msgq->queue[index].priority;

                    msgq->queue[index].data = data;
                    msgq->queue[index].priority = priority;

                    data = tmp_data;
                    priority = tmp_priority;
                }
                index = (index < msgq->size - 1) ? (index + 1) : 0;
            }

            msgq->queue[msgq->write].data = data;
            msgq->queue[msgq->write].priority = priority;

            msgq->count++;
            msgq->write = (msgq->write < msgq->size - 1) ? (msgq->write + 1) : 0;

            // msgq->msg[msgq->write] = (unsigned int)msg->msg;
            // msgq->valid[msgq->write] = MSG_STATE_USED;
        }
        pthread_cond_signal(&(msgq->cond));

        pthread_mutex_unlock(&(msgq->mutex));
        break;

    }

    if (msg)
    {
        if (msg->timer)
        {
            TIME_Deactivate(msg->timer);
            TIME_Dest(msg->timer);
            MEM_Free(msg->timer);
        }
        MEM_Free(msg);
    }
}
#endif /* DELAY_TIMER_ENABLED */


/*****************************************************************************
 * FUNCTION
 *  UCE_CMSGQ_Init
 * DESCRIPTION
 *
 * PARAMETERS
 *  size        [IN]
 *  name        [?]
 *  func        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void *UCE_CMSGQ_Init(int size, char *name, RCS_CMSGQ_callback func)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    os_msgq_t *msgq = 0;
    int ret = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /* int i = 0; */

    msgq = (os_msgq_t*) MEM_Allocate(sizeof(os_msgq_t));
    if (!msgq)
    {
        ErrMsg("can't allocate the memory");
        return 0;
    }
    memset(msgq, 0, sizeof(os_msgq_t));

    msgq->queue = (os_msg_t*) MEM_Allocate(sizeof(os_msgq_t) * size);

    msgq->size = size;
    ret = pthread_mutex_init(&(msgq->mutex), 0);
    if (ret < 0)
    {
        ErrMsg("pthread_mutex_init error");
        goto err_free;
    }
    ret = pthread_cond_init(&(msgq->cond), 0);
    if (ret < 0)
    {
        ErrMsg("pthread_cond_init error");
        goto err_free;
    }

    msgq->timer_count = 0;
    msgq->count = 0;
    msgq->read = 0;
    msgq->write = 0;
    msgq->func = func;
    snprintf(msgq->name, sizeof(msgq->name) - 1, "%s", name);

    return msgq;

  err_free:
    UCE_CMSGQ_Free(msgq);
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *  RCS_CMSGQ_Put
 * DESCRIPTION
 *
 * PARAMETERS
 *  o       [?]
 *  msg     [?]
 * RETURNS
 *
 *****************************************************************************/
int UCE_CMSGQ_Put(void *o, void *msg)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    os_msgq_t *msgq = (os_msgq_t*) o;
    int ret = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!msgq)
    {
        ErrMsg("msgq pointer is NULL");
        return -1;
    }

    pthread_mutex_lock(&(msgq->mutex));

    if (msgq->func)
    {
        int i = msgq->read;
        int c = msgq->count;

        while (c-- > 0)
        {
            /* ret = msgq->func((void *)msg, (void *)msgq->msg[i]); */
            ret = msgq->func((void*)msg, (void*)msgq->queue[i].data);
            if (ret == UCE_CMSGQ_CHECK_DROP)
            {
                pthread_mutex_unlock(&(msgq->mutex));
                return 2;
            }
            i = (i < msgq->size - 1) ? (i + 1) : 0;
        }
    }

    if (msgq->timer_count + msgq->count < msgq->size)
    {
        // msgq->msg[msgq->write] = (unsigned int)msg;
        // msgq->valid[msgq->write] = MSG_STATE_USED;
        msgq->queue[msgq->write].data = (void*)msg;
        msgq->queue[msgq->write].priority = MSG_DEFAULT_PRIORITY;

        msgq->count++;
        msgq->write = (msgq->write < msgq->size - 1) ? (msgq->write + 1) : 0;
        ret = 0;
        pthread_cond_signal(&(msgq->cond));
    }
    else
    {
        ret = 1;
        ErrMsg("msgq is FULL (%s)", msgq->name);
        pthread_cond_signal(&(msgq->cond));
    }

    pthread_mutex_unlock(&(msgq->mutex));

    return ret;
}


/*****************************************************************************
 * FUNCTION
 *  UCE_CMSGQ_Put2
 * DESCRIPTION
 *
 * PARAMETERS
 *  o               [?]
 *  msg             [?]
 *  priority        [IN]
 * RETURNS
 *
 *****************************************************************************/
int UCE_CMSGQ_Put2(void *o, void *msg, int priority)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    os_msgq_t *msgq = (os_msgq_t*) o;
    int ret = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!msgq)
    {
        ErrMsg("msgq pointer is NULL");
        return -1;
    }

    pthread_mutex_lock(&(msgq->mutex));

    if (msgq->func)
    {
        int i = msgq->read;
        int c = msgq->count;

        while (c-- > 0)
        {
            /* ret = msgq->func((void *)msg, (void *)msgq->msg[i]); */
            ret = msgq->func((void*)msg, (void*)msgq->queue[i].data);
            if (ret == UCE_CMSGQ_CHECK_DROP)
            {
                pthread_mutex_unlock(&(msgq->mutex));
                return 2;
            }
            i = (i < msgq->size - 1) ? (i + 1) : 0;
        }
    }

    if (msgq->timer_count + msgq->count < msgq->size)
    {
        int count = 0;
        int index = 0;
        int priority_org = priority;

        index = msgq->read;
        count = msgq->count;
        while (count-- > 0)
        {
            if (priority_org > msgq->queue[index].priority)
            {
                void *tmp_data = msgq->queue[index].data;
                int tmp_priority = msgq->queue[index].priority;

                msgq->queue[index].data = (void*)msg;
                msgq->queue[index].priority = priority;

                msg = tmp_data;
                priority = tmp_priority;
            }
            index = (index < msgq->size - 1) ? (index + 1) : 0;
        }

        msgq->queue[msgq->write].data = (void*)msg;
        msgq->queue[msgq->write].priority = priority;

        msgq->count++;
        msgq->write = (msgq->write < msgq->size - 1) ? (msgq->write + 1) : 0;

        ret = 0;
        pthread_cond_signal(&(msgq->cond));
    }
    else
    {
        ret = 1;
        ErrMsg("msgq is FULL (%s)", msgq->name);
        pthread_cond_signal(&(msgq->cond));
    }

    pthread_mutex_unlock(&(msgq->mutex));

    return ret;
}

#ifdef DELAY_TIMER_ENABLED


/*****************************************************************************
 * FUNCTION
 *  UCE_CMSGQ_DelayPut
 * DESCRIPTION
 *
 * PARAMETERS
 *  o               [?]
 *  data            [?]
 *  delay           [IN]
 *  priority        [IN]
 * RETURNS
 *
 *****************************************************************************/
int UCE_CMSGQ_DelayPut(void *o, void *data, int delay, int priority)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    os_msgq_t *msgq = (os_msgq_t*) o;
    os_delay_msg_t *msg = 0;
    int ret = -1;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!msgq)
    {
        ErrMsg("msgq pointer is NULL");
        return -1;
    }

    msg = (os_delay_msg_t*) MEM_Allocate(sizeof(os_delay_msg_t));
    if (!msg)
    {
        ErrMsg("can't allocate the memory - delay msg");
        return -1;
    }

    msg->timer = MEM_Allocate(TIME_Size());
    if (!(msg->timer))
    {
        ErrMsg("can't allocate the memory - delay msg");
        MEM_Free(msg);
        return -1;
    }

    msg->msg = data;
    msg->priority = priority;
    msg->msgq = msgq;

    pthread_mutex_lock(&(msgq->mutex));
    if (msgq->count + msgq->timer_count < msgq->size)
    {
        if (TIME_Init(msg->timer, "RCS_CMSGQ_TIMER", msgq_expire_handler, msg, delay, 0))
        {
            MEM_Free(msg->timer);
            MEM_Free(msg);
            ErrMsg("Can't create timer");
            ret = -1;
        }
        else
        {
            msgq->timer_count++;
            ret = 0;
        }
    }
    else
    {
        MEM_Free(msg->timer);
        MEM_Free(msg);
        ret = -1;
        ErrMsg("msgq is FULL - timer");
    }
    pthread_mutex_unlock(&(msgq->mutex));

    return ret;
}
#endif /* DELAY_TIMER_ENABLED */


/*****************************************************************************
 * FUNCTION
 *  UCE_CMSGQ_Get
 * DESCRIPTION
 *
 * PARAMETERS
 *  o           [?]
 *  flag        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void *UCE_CMSGQ_Get(void *o, int flag)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    os_msgq_t *msgq = (os_msgq_t*) o;
    os_msg_t *msg = 0;
    void *ret_data = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!msgq)
    {
        ErrMsg("msgq pointer is NULL");
        return 0;
    }

    pthread_mutex_lock(&(msgq->mutex));

    if (flag == UCE_CMSGQ_FLAG_WAIT)
    {
        while (msgq->count == 0)
            pthread_cond_wait(&(msgq->cond), &(msgq->mutex));
    }

    if (msgq->count > 0)
    {
        msg = (os_msg_t*) & (msgq->queue[msgq->read]);
        ret_data = msg->data;
        /* msgq->valid[msgq->read] = MSG_STATE_UNUSED; */
        msgq->count--;
        msgq->read = (msgq->read < msgq->size - 1) ? (msgq->read + 1) : 0;
    }
    else
    {
    #ifndef _UNIT_
        ErrMsg("msgq is EMPTY (%s)", msgq->name);
    #endif
    }

    pthread_mutex_unlock(&(msgq->mutex));

    return ret_data;
}


/*****************************************************************************
 * FUNCTION
 *  UCE_CMSGQ_GetMsg
 * DESCRIPTION
 *
 * PARAMETERS
 *  o           [?]
 *  func        [IN]
 *  data        [?]
 * RETURNS
 *  void
 *****************************************************************************/
void *UCE_CMSGQ_GetMsg(void *o, RCS_CMSGQ_callback func, void *data)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    os_msgq_t *msgq = (os_msgq_t*) o;
    os_msg_t *msg = 0;
    void *ret_data = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!msgq)
    {
        ErrMsg("msgq pointer is NULL");
        return 0;
    }

    pthread_mutex_lock(&(msgq->mutex));

    if (msgq->count > 0)
    {
        if (func)
        {
            int i = msgq->read;
            int c = msgq->count;
            int find = 0;

            while (c-- > 0)
            {
                if (find == 0)
                {
                    int ret = 0;

                    msg = (os_msg_t*) & (msgq->queue[i]);
                    ret = func((void*)(msg->data), (void*)data);
                    if (ret == UCE_CMSGQ_CHECK_RETURN)
                    {
                        ret_data = msg->data;
                        msgq->count--;
                        find = 1;
                    }
                }

                if (find)
                {
                    if (c > 0)
                    {
                        int k = (i < msgq->size - 1) ? (i + 1) : 0;

                        msgq->queue[i].data = msgq->queue[k].data;
                        msgq->queue[i].priority = msgq->queue[k].priority;
                    }
                    else
                    {
                        msgq->queue[i].data = 0;
                    }
                }

                i = (i < msgq->size - 1) ? (i + 1) : 0;
            }
        }
    }
    else
    {
    #ifndef _UNIT_
        ErrMsg("msgq is EMPTY (%s)", msgq->name);
    #endif
    }

    pthread_mutex_unlock(&(msgq->mutex));

    return ret_data;
}


/*****************************************************************************
 * FUNCTION
 *  UCE_CMSGQ_Free
 * DESCRIPTION
 *
 * PARAMETERS
 *  o       [?]
 * RETURNS
 *
 *****************************************************************************/
int UCE_CMSGQ_Free(void *o)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    os_msgq_t *msgq = (os_msgq_t*) o;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (msgq)
    {
        if (msgq->queue)
        {
            MEM_Free(msgq->queue);
        }
        /*
         * if (msgq->valid) {
         * free(msgq->valid);
         * }
         */
        MEM_Free(msgq);
    }
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *  UCE_CMSGQ_MsgAllocate
 * DESCRIPTION
 *
 * PARAMETERS
 *  size        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void *UCE_CMSGQ_MsgAllocate(int size)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    return MEM_Allocate(size);
}


/*****************************************************************************
 * FUNCTION
 *  UCE_CMSGQ_MsgFree
 * DESCRIPTION
 *
 * PARAMETERS
 *  ptr     [?]
 * RETURNS
 *
 *****************************************************************************/
int UCE_CMSGQ_MsgFree(void *ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    MEM_Free(ptr);
    return 0;
}

#if defined _UNIT_

#include <unistd.h>

#define UT_CHECK(f)     do { if ((f)) { printf("\tPass @ %d\n",  __LINE__); break; } else { printf("\tError @ %d\n", __LINE__); goto test_end; } } while (0)


/*****************************************************************************
 * FUNCTION
 *  test1
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/
int test1()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    void *q = 0;
    int ret = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    printf("UT : %s\n", __FUNCTION__);

    q = UCE_CMSGQ_Init(5, "test", 0);

    ret = UCE_CMSGQ_Put(q, (void*)1);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put(q, (void*)2);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put(q, (void*)3);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put(q, (void*)4);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put(q, (void*)5);
    UT_CHECK(ret == 0);

    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 1);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 2);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 3);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 4);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 5);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 0);

    UCE_CMSGQ_Free(q);

    return 0;

  test_end:
    UCE_CMSGQ_Free(q);
    return -1;
}


/*****************************************************************************
 * FUNCTION
 *  test2
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/
int test2()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    void *q = 0;
    int ret = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    printf("UT : %s\n", __FUNCTION__);

    q = UCE_CMSGQ_Init(10, "test", 0);

    ret = UCE_CMSGQ_Put2(q, (void*)1, 10);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)2, 8);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)3, 6);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)4, 10);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)5, 12);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)6, 12);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)7, 12);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)8, 6);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)9, 8);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)10, 9);
    UT_CHECK(ret == 0);

    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 5);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 6);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 7);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 1);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 4);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 10);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 2);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 9);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 3);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 8);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_NOWAIT);
    UT_CHECK(ret == 0);

    UCE_CMSGQ_Free(q);

    return 0;

  test_end:
    UCE_CMSGQ_Free(q);
    return -1;
}


/*****************************************************************************
 * FUNCTION
 *  test3
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/
int test3()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    void *q = 0;
    int ret = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    printf("UT : %s\n", __FUNCTION__);

    q = UCE_CMSGQ_Init(5, "test", 0);

    ret = UCE_CMSGQ_DelayPut(q, (void*)1, 400, 3);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_DelayPut(q, (void*)2, 300, 4);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_DelayPut(q, (void*)3, 200, 2);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_DelayPut(q, (void*)4, 100, 5);
    UT_CHECK(ret == 0);

    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_WAIT);
    UT_CHECK(ret == 4);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_WAIT);
    UT_CHECK(ret == 3);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_WAIT);
    UT_CHECK(ret == 2);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_WAIT);
    UT_CHECK(ret == 1);

    UCE_CMSGQ_Free(q);

    return 0;

  test_end:
    UCE_CMSGQ_Free(q);
    return -1;
}


/*****************************************************************************
 * FUNCTION
 *  test4
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/
int test4()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    void *q = 0;
    int ret = 0;
    time_t t = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    printf("UT : %s\n", __FUNCTION__);

    q = UCE_CMSGQ_Init(5, "test", 0);

    ret = UCE_CMSGQ_DelayPut(q, (void*)1, 400, 3);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_DelayPut(q, (void*)2, 300, 4);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_DelayPut(q, (void*)3, 200, 2);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_DelayPut(q, (void*)4, 100, 5);
    UT_CHECK(ret == 0);

    t = time(0);
    printf("\t");
    while (t + 6 > time(0))
    {
        printf(".");
        fflush(stdout);
        sleep(1);
    }
    printf("\n");

    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_WAIT);
    UT_CHECK(ret == 4);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_WAIT);
    UT_CHECK(ret == 2);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_WAIT);
    UT_CHECK(ret == 1);
    ret = (int)UCE_CMSGQ_Get(q, UCE_CMSGQ_FLAG_WAIT);
    UT_CHECK(ret == 3);

    UCE_CMSGQ_Free(q);

    return 0;

  test_end:
    UCE_CMSGQ_Free(q);
    return -1;
}


/*****************************************************************************
 * FUNCTION
 *  check_cb
 * DESCRIPTION
 *
 * PARAMETERS
 *  new_msg     [?]
 *  msg         [?]
 * RETURNS
 *
 *****************************************************************************/
int check_cb(void *new_msg, void *msg)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    int curr = (int)new_msg;
    int priv = (int)msg;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (curr > priv)
    {
        return UCE_CMSGQ_CHECK_RETURN;
    }
    return UCE_CMSGQ_CHECK_CONTINUE;
}


/*****************************************************************************
 * FUNCTION
 *  test5
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/
int test5()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    void *q = 0;
    int ret = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    printf("UT : %s\n", __FUNCTION__);

    q = UCE_CMSGQ_Init(10, "test", 0);

    ret = UCE_CMSGQ_Put2(q, (void*)1, 10);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)2, 8);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)3, 6);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)4, 10);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)5, 12);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)6, 12);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)7, 12);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)8, 6);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)9, 8);
    UT_CHECK(ret == 0);
    ret = UCE_CMSGQ_Put2(q, (void*)10, 9);
    UT_CHECK(ret == 0);

    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)5);
    UT_CHECK(ret == 6);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)5);
    UT_CHECK(ret == 7);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)5);
    UT_CHECK(ret == 10);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)5);
    UT_CHECK(ret == 9);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)5);
    UT_CHECK(ret == 8);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)5);
    UT_CHECK(ret == 0);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)0);
    UT_CHECK(ret == 5);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)0);
    UT_CHECK(ret == 1);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)0);
    UT_CHECK(ret == 4);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)0);
    UT_CHECK(ret == 2);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)0);
    UT_CHECK(ret == 3);
    ret = (int)UCE_CMSGQ_GetMsg(q, check_cb, (void*)0);
    UT_CHECK(ret == 0);

    UCE_CMSGQ_Free(q);

    return 0;

  test_end:
    UCE_CMSGQ_Free(q);
    return -1;
}


/*****************************************************************************
 * FUNCTION
 *  main
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/
int main()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /*
     * test1();
     * test2();
     * test3();
     * test4();
     */
    test5();

    return 0;
}

#endif /* defined _UNIT_ */

