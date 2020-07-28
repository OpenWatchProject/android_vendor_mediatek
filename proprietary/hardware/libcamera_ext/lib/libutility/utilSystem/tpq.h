
#ifndef _THREAD_POOL_QUEUE_H_
#define _THREAD_POOL_QUEUE_H_

typedef struct rtinfo
{
    int task_id;
    int num_tasks;
    int thread_id;
    int num_threads;

    int group_id;
    int group_task_id;
    int group_num_tasks;
} rtinfo;

typedef struct tpq_* tp_queue;
typedef void *(*tp_func)(void*, rtinfo *);

/*
 * Initialize tp_queue
 */
tp_queue tpq_create(int num_threads, const char *thread_name = "");
tp_queue tpq_group_create(int num_threads, int groups, const char *thread_name = "");

/*
 * Add work to the job queue
 */
int tpq_add_work(tp_queue, tp_func, void* arg_p);
int tpq_add_group_work(int, tp_queue, tp_func, void* arg_p);

/* TODO: async exec/wait fucntions */

/*
 * Trigger the execution & block until finish
 */
void tpq_exec(tp_queue, unsigned int);

/*
 * Release the tp_queue
 */
void tpq_destroy(tp_queue);


#endif
