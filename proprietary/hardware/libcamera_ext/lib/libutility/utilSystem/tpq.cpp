#define LOG_TAG "utilTPQ"

#ifdef __ANDROID__
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "tpq.h"

// android log header
#if defined(NDK)
#include <sys/prctl.h>
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#elif defined (ANDROID)
#include <cutils/log.h>
#include <sys/prctl.h>
#define LOGD(fmt, arg...)   ALOGD(fmt, ##arg)
#define LOGE(fmt, arg...)   ALOGE(fmt, ##arg)
#else
#define LOGD(fmt, ...) printf("[%s] " fmt, LOG_TAG, ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[%s] Error(%d) " fmt, LOG_TAG, __LINE__, ##__VA_ARGS__)
#define prctl(...)
#endif

/* Job */
typedef struct job
{
    struct job*  next;                              /* next job                 */
    tp_func function;                               /* function pointer         */
    void*  arg;                                     /* argument                 */
    int task_id;                                    /* global task id           */

    int group_id;
    int group_task_id;                              /* group task id            */
} job;

/* linked-list Job Queue */
typedef struct jobq
{
    pthread_mutex_t lock;                           /* lock for critical access */
    job  *head;                                     /* head of queue            */
    job  *tail;                                     /* tail of queue            */
    volatile int  len;                              /* # of jobs                */
} jobq;

typedef struct group
{
    int num_tasks;
} group;

/* Thread */
typedef struct thread
{
    pthread_t pthread;                              /* pointer to actual thread */
    struct tpq_* tpq_p;                             /* access to tpq            */
    rtinfo    thread_rtinfo;
    pthread_mutex_t     start_mutex;
    pthread_mutex_t     end_mutex;
} thread;


/* Threadpool Queue*/
typedef struct tpq_
{
    int                 num_threads;
    thread**            threads;                    /* thread pool              */

    volatile int        keepalive;
    volatile int        num_alive;                  /* # of alive threads       */

    pthread_mutex_t     mutex;

    jobq*               jobq_p;                     /* job queue                */

    int                 num_groups;
    group               *groups;
    const char          *thread_name;               /* thread name              */
} tpq_;


/* declaration of static infrastructure functions */
//static void  thread_create(tpq_* tpq_p, struct thread** thread_p, int id);
static void  thread_create(tpq_* tpq_p);
static void  thread_get_tasks(tpq_* tpq_p);
static void* thread_do(void* thread_ptr);
static void  thread_destroy(tpq_* tpq_p);

static int          jobq_init(tpq_* tpq_p);
static void         jobq_clear(tpq_* tpq_p);
static void         jobq_enq(tpq_* tpq_p, struct job* newjob_p);
static struct job*  jobq_deq(tpq_* tpq_p);
static void         jobq_destroy(tpq_* tpq_p);

/* create an instance of Thread Pool Queue */
struct tpq_* tpq_group_create(int num_threads, int num_groups, const char *thread_name)
{
    tpq_* tpq_p = NULL;
    if ( num_threads <= 0){
        num_threads = 4;
    }

    /* create new thread pool */
    tpq_p = (struct tpq_*)calloc(sizeof(struct tpq_), 1);
    if (tpq_p == NULL){
        LOGE("tpq_create(): Could not allocate memory for thread pool\n");
        return NULL;
    }
    tpq_p->num_threads = num_threads;
    tpq_p->keepalive = 1;

    if(num_groups > 0){
        tpq_p->num_groups = num_groups;
        tpq_p->groups = (struct group*)calloc(sizeof(struct group), 1);
    }

    /* init the job queue */
    if (jobq_init(tpq_p) == -1){
        LOGE("tpq_create(): Could not allocate memory for job queue\n");
        free(tpq_p);
        return NULL;
    }

    /* create thread pool */
    tpq_p->threads = (struct thread**)calloc(num_threads * sizeof(struct thread), 1);
    if (tpq_p->threads == NULL){
        LOGE("tpq_create(): Could not allocate memory for threads\n");
        jobq_destroy(tpq_p);
        free(tpq_p->jobq_p);
        free(tpq_p);
        return NULL;
    }

    /* pass thread name */
    tpq_p->thread_name = thread_name;

    pthread_mutex_init(&(tpq_p->mutex), NULL);

    thread_create(tpq_p);
    LOGD("tpq_create(): %p %s %dthreads\n",tpq_p,tpq_p->thread_name,num_threads);
    return tpq_p;
}

struct tpq_* tpq_create(int num_threads, const char *thread_name)
{
    return tpq_group_create(num_threads, 0, thread_name);
}

/* enqueue a work to thread pool queue*/
int tpq_add_group_work(int gidx, tpq_* tpq_p, tp_func func_ptr, void* arg_ptr)
{

    job* newjob;
    newjob = (struct job*)calloc(sizeof(struct job), 1);
    if (newjob == NULL){
        LOGE("tpq_add_work(): Could not allocate memory for new job\n");
        return -1;
    }
    /* add function and argument */
    newjob->function = func_ptr;
    newjob->arg = arg_ptr;

    if(tpq_p->num_groups){
        newjob->group_id = gidx;
        if(gidx >= 0)
            newjob->group_task_id = tpq_p->groups[gidx].num_tasks++;
    }

    /* add job to queue */
    jobq_enq(tpq_p, newjob);

    return 0;
}

int tpq_add_work(tpq_* tpq_p, tp_func func_ptr, void* arg_ptr)
{
    return tpq_add_group_work(-1, tpq_p, func_ptr, arg_ptr);
}

/* Wait until all jobs have finished */
void tpq_exec(tpq_* tpq_p, unsigned int timeout_us)
{
    int i;

    thread_get_tasks(tpq_p);

    /* trigger thread pool for tasks */
    for(i = 0; i < tpq_p->num_threads; i++)
        pthread_mutex_unlock(&(tpq_p->threads[i]->start_mutex));

    for(i = 0; i < tpq_p->num_threads; i++)
        pthread_mutex_lock(&(tpq_p->threads[i]->end_mutex));


    for(i = 0; i < tpq_p->num_groups; i++)
        tpq_p->groups[i].num_tasks = 0;
}

/* destroy the thread pool queue */
void tpq_destroy(tpq_* tpq_p)
{

    int i, accu_time = 0;
    LOGD("tpq_destroy(): Start waiting %p %s threads\n",tpq_p,tpq_p->thread_name);
    /* End each thread 's infinite loop */
    tpq_p->keepalive = 0;

    for(i = 0; i < tpq_p->num_threads; i++)
        pthread_mutex_unlock(&(tpq_p->threads[i]->start_mutex));

    void *tmp;
 /*   for(i = 0; i < tpq_p->num_threads; i++)
    {
        struct thread** thread_p = &tpq_p->threads[i];
        pthread_join((*thread_p)->pthread,&tmp);
    }*/
 //   LOGD("tpq_destroy(): Finish join all %p %sthreads\n",tpq_p,tpq_p->thread_name);

    /* Give one second to kill idle threads */
    while(accu_time < 1000000 && tpq_p->num_alive){
#ifdef __ANDROID__
        usleep(250);
#else
        Sleep(1);
#endif
        accu_time += 250;
    }
    LOGD("tpq_destroy(): sleep 0 %p\n",tpq_p);

#if 0 // seldom timeout issue in monkey
    //WHEN BLOCKS HERE => some threads not return successfully
    /* Poll remaining threads, TODO: what if blocked thread ? */
    while(tpq_p->num_alive){
#ifdef __ANDROID__
        sleep(1);
#else
        Sleep(1000);
#endif
    }
    LOGD("tpq_destroy(): sleep 1 %p\n",tpq_p);
#endif

    pthread_mutex_destroy(&tpq_p->mutex);

    /* Job queue cleanup */
    jobq_destroy(tpq_p);
    free(tpq_p->jobq_p);

    if(tpq_p->groups)
        free(tpq_p->groups);

    /* Deallocs */
    thread_destroy(tpq_p);
    free(tpq_p->threads);
    free(tpq_p);
}

/* create a thread in the thread pool */
static void thread_create(tpq_* tpq_p)
{
    int n;
    for(n = 0; n < tpq_p->num_threads; n++){
        struct thread** thread_p = &tpq_p->threads[n];
        *thread_p = (struct thread*)calloc(sizeof(struct thread), 1);
        if(*thread_p == NULL){
            LOGE("tpq_create(): Could not allocate memory for thread\n");
            exit(1);
        }

        (*thread_p)->tpq_p    = tpq_p;
        (*thread_p)->thread_rtinfo.num_threads = tpq_p->num_threads;
        (*thread_p)->thread_rtinfo.thread_id = n;

        pthread_mutex_init(&((*thread_p)->start_mutex), NULL);
        pthread_mutex_lock(&((*thread_p)->start_mutex));
        pthread_mutex_init(&((*thread_p)->end_mutex), NULL);
        pthread_mutex_lock(&((*thread_p)->end_mutex));

        int res = pthread_create(&(*thread_p)->pthread, NULL, thread_do, (*thread_p));
        if (res == 0)
        {
            /* Mark thread as alive (initialized) */
            pthread_mutex_lock(&tpq_p->mutex);
            tpq_p->num_alive++;
            pthread_mutex_unlock(&tpq_p->mutex);
        }
        else
        {
            //TODO: handle pthread_create error exception.
        }
        pthread_detach((*thread_p)->pthread);

    }
}

static void thread_get_tasks(tpq_ *tpq_p)
{
    int n;
    for(n = 0; n < tpq_p->num_threads; n++){
        struct thread* thread_p = tpq_p->threads[n];
        thread_p->thread_rtinfo.num_tasks = tpq_p->jobq_p->len;
    }
}

/* Worker Thread Loop */
static void* thread_do(void* thread_ptr)
{
    /* type casting */
    struct thread* thread_p = (struct thread*)thread_ptr;

    /* Assure all threads have been created before starting serving */
    tpq_* tpq_p = thread_p->tpq_p;

    /* set thread name */
    if (0 != strcmp(tpq_p->thread_name, ""))
    {
        prctl(PR_SET_NAME, tpq_p->thread_name, 0, 0, 0);
    }

    while(tpq_p->keepalive){

        pthread_mutex_lock(&thread_p->start_mutex);

        while(tpq_p->keepalive && tpq_p->jobq_p->len){
            tp_func func_buff;
            void*  arg_buff;
            job* job_p;

            /* Read job from queue and execute it */
            job_p = jobq_deq(tpq_p);
            if (job_p) {
                func_buff = job_p->function;
                arg_buff  = job_p->arg;
                thread_p->thread_rtinfo.task_id = job_p->task_id;
                if(tpq_p->num_groups){
                    thread_p->thread_rtinfo.group_id = job_p->group_id;
                    thread_p->thread_rtinfo.group_task_id = job_p->group_task_id;
                    if(job_p->group_id >= 0)
                        thread_p->thread_rtinfo.group_num_tasks = tpq_p->groups[job_p->group_id].num_tasks;
                }
                func_buff(arg_buff, &thread_p->thread_rtinfo);
                free(job_p);
            }

        }

        pthread_mutex_unlock(&thread_p->end_mutex);
    }

    pthread_mutex_lock(&tpq_p->mutex);
    tpq_p->num_alive--;
    pthread_mutex_unlock(&tpq_p->mutex);

    pthread_exit(NULL);
}

/* Frees a thread  */
static void thread_destroy(tpq_* tpq_p)
{
    int n;
    for(n = 0; n < tpq_p->num_threads; n++){
        pthread_mutex_destroy(&(tpq_p->threads[n]->start_mutex));
        pthread_mutex_destroy(&(tpq_p->threads[n]->end_mutex));
        free(tpq_p->threads[n]);
    }
}

/*
 * TPQ Internal Infrastructure
 */

/* Initialize queue */
static int jobq_init(tpq_* tpq_p)
{
    tpq_p->jobq_p = (struct jobq*)calloc(sizeof(struct jobq), 1);
    if (tpq_p->jobq_p == NULL){
        return -1;
    }

    pthread_mutex_init(&(tpq_p->jobq_p->lock), NULL);

    return 0;
}

/* clear job queue */
static void jobq_clear(tpq_* tpq_p)
{
    while(tpq_p->jobq_p->len){
        free(jobq_deq(tpq_p));
    }

    tpq_p->jobq_p->len = 0;
    tpq_p->jobq_p->head = NULL;
    tpq_p->jobq_p->tail  = NULL;

}

/* enqueue a job */
static void jobq_enq(tpq_* tpq_p, struct job* newjob)
{

    pthread_mutex_lock(&tpq_p->jobq_p->lock);

    newjob->task_id = tpq_p->jobq_p->len;
    newjob->next = NULL;

    if(tpq_p->jobq_p->len++)
        tpq_p->jobq_p->tail->next = newjob;
    else
        tpq_p->jobq_p->head = newjob;

    tpq_p->jobq_p->tail = newjob;

    pthread_mutex_unlock(&tpq_p->jobq_p->lock);
}

/* dequeue a job */
static struct job* jobq_deq(tpq_* tpq_p)
{
    job* job_p;
    pthread_mutex_lock(&tpq_p->jobq_p->lock);
    job_p = tpq_p->jobq_p->head;

    if(tpq_p->jobq_p->head){
        tpq_p->jobq_p->head = job_p->next;
        tpq_p->jobq_p->len--;
    }

    pthread_mutex_unlock(&tpq_p->jobq_p->lock);
    return job_p;
}

/* release job queue resources */
static void jobq_destroy(tpq_* tpq_p)
{
    jobq_clear(tpq_p);
    pthread_mutex_destroy(&(tpq_p->jobq_p->lock));
}
