/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
 * Description:
 *   Data structures and Algorithms for video applications.
 */

#ifndef VAL_ALGO_H
#define VAL_ALGO_H

#ifdef __cplusplus
extern "C" {
#endif

#define _POSIX_

#ifdef _POSIX_
#include <pthread.h>
typedef struct _CpuStat_s
{
    unsigned long valid;
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
    unsigned long irq;
    unsigned long softirq;
    unsigned long steal;
    unsigned long all_nonidle;
    unsigned long all_idle;
} CpuStat_s;
#else
typedef struct _CpuStat_s
{
    unsigned long valid;
    unsigned long all_nonidle;
    unsigned long all_idle;
} CpuStat_s;
#endif

typedef struct _Elem_s
{
    int type;
    double data;
    double timestamp;
} Elem_s;

typedef struct _Node_s
{
    Elem_s elem;
    int key;
    struct _Node_s *left;
    struct _Node_s *right;
} Node_s;

typedef struct _CirQueue_s
{
    int head;
    int tail;
    int count;
    int total;
    Elem_s *data;
    Node_s *headnode;
    Node_s *tailnode;
    double timeoffset;
    CpuStat_s cpustat;
#ifdef _POSIX_
    pthread_rwlock_t rwlock;
#endif

    int (*isempty)(struct _CirQueue_s *);
    int (*empty)(struct _CirQueue_s *);
    int (*empty_type)(struct _CirQueue_s *, int);
    int (*capacity)(struct _CirQueue_s *);
    int (*size)(struct _CirQueue_s *);
    int (*push)(struct _CirQueue_s *, double);
    int (*push_type)(struct _CirQueue_s *, double, int);
    int (*pop)(struct _CirQueue_s *, double *, int *);
    int (*pop_type)(struct _CirQueue_s *, double *, int);
    int (*peek)(struct _CirQueue_s *, double *);
    int (*peek_type)(struct _CirQueue_s *, double *, int);
    int (*last)(struct _CirQueue_s *, double *);
    int (*last_type)(struct _CirQueue_s *, double *, int);
    int (*print)(struct _CirQueue_s *);
    int (*average)(struct _CirQueue_s *, int, double *);
    int (*average_type)(struct _CirQueue_s *, int, double *, int);
    int (*stddev)(struct _CirQueue_s *, int, double *);
    int (*cov)(struct _CirQueue_s *, int, double *);
    int (*iqr)(struct _CirQueue_s *, int, double *);
    int (*timestamp)(struct _CirQueue_s *);
    int (*timestamp_type)(struct _CirQueue_s *, int);
    int (*timestamp_mark)(struct _CirQueue_s *, double *, int);
    int (*timestamp_diff)(struct _CirQueue_s *, int, int);
    int (*type_mark)(struct _CirQueue_s *, double, int);
    int (*type_diff)(struct _CirQueue_s *, double, int, int);
    int (*type_delta)(struct _CirQueue_s *, double, int);
    int (*usage_mark)(struct _CirQueue_s *);
    int (*usage_diff)(struct _CirQueue_s *, double *, double *, int);
    int (*serialize)(struct _CirQueue_s *, const char *, int);

} CirQueue_s;

CirQueue_s* CirQueue_Clone(int capacity, CirQueue_s *source, int type);
CirQueue_s* CirQueue_Create(int capacity);
CirQueue_s* CirQueue_CreateByMemory(int size);
int CirQueue_Destroy(CirQueue_s *queue);

#ifdef __cplusplus
}
#endif

#endif /* VAL_ALGO_H */
