/*************************************************
Copyright (C), 1997-2016, Shenzhen Huiding Technology Co.,Ltd.
Version:
Description:  手指-心率计算（无touch信息）---sampling frequency: 50HZ
History:
2015.12.09 - Yuyi - created
2015.12.18 - Yuyi - tested
2016.01.26 - Yuyi - first 50 datas will not be displayed
2016.01.28 - Yuyi - deal with the light-dim and displayed data will be removed baseline first
2016.03.04 - Yuyi - debug CalHeartRate内存泄露
2016.03.09 - Yuyi - 初始计算误差大debug，利用波峰信息，取代波谷信息
2016.03.25 - Yuyi - CalHeartRate中取到(pIntervals[szValleyLength - 1])越界,debug
2016.04.01 - Yuyi - 取整导致心率值偏小debug
2016.04.08 - Yuyi - 提示信息加入
2016.05.04 - Yuyi - 极限心率值可计算
2016.09.09 - LiDan - 加快首次显示方案、分配内存bug、心率波形少时应该返回而未返回的bug
2016.09.14 - LiDan - 避免每次显示结果跳变
2016.10.28 - LiDan - 修正WarningInformation里面计算信号质量的bug等3处
2016.11.17 - LiDan - 修正首次显示遇到调光情况的计算误差大；修正低通滤波和去除基线函数
2016.12.02 - LiDan - 修正质量判断等首次显示计算误差大
*************************************************/

#ifndef    __GF_HEART_BEAT_H__
#define    __GF_HEART_BEAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "gf_error.h"
#include "gf_common.h"

#define FIRST2SEC 1//output处，至少提前2秒钟显示
#define DEALWITHJUMP 1//处理两次跳变问题
#define MAXDIFHBD 10//两次心率的最大差值
#define FIX_WARNING_THR 1//修正judge和warning里面的不同的判断心率质量的条件//LD,20161031
#define WAVE_DIF1 1.3//判断上下坡沿比例门限1
#define WAVE_DIF2 1.4//判断上下坡沿比例门限2
#define DEALWITHFIRSTJUMP   1  //处理首次误差大，20161117，LD
#define DEALWITHFIRSTJUMP2  1  //处理首次误差大，原因是调整的时候出现错误，同时，排除异常波形时候比例太宽
#define V12 1                 //G201612010666001, LD
#if V12
#define CHANGELAST150POINTS 1   //基于1.1.1基础上，改善滤波器，替换最后的150个点。G201612010666001
#define GOODWAVETHR 1   //好的波形的比例
#define PEAKVALLYMEAN 1// 波峰和波谷的HR结果的平均值
#define JUDGE2        1   //判断心率波形好与坏，使用方差判断
#endif

/*算法缓存数据的时间(用于计算心率值)*/
#define    PERIOD    4

/*提示信息分析的数据秒数*/
#define    ANALYSIS_SEDS    6

/*data length for each process*/
#define    DATA_SIZE_ALGORITHM    (FREQ * PERIOD)

/*分析提示信息时的数据长度*/
#define    DATA_SIZE_WARNIG_INFO    (FREQ * ANALYSIS_SEDS)

/*可显示的最小心率值*/
#define    MINIMUM_BPM    30

/*可显示的最大心率值*/
#define    MAXIMUM_BPM    220

/*AC大于该值判断为运动信号*/
#define    AC_THRESHOLD_UP   1000

/*AC小于该值判断为信号太弱*/
#define    AC_THRESHOLD_DOWN    15

/*最小AC阈值*/
#define    AC_MIN_ACCEPT    8

/*运动信号的AC均值大于此值*/
#define    AC_MIN_MOVE    200

/*存在相对运动的周期数*/
#define    MOVING_RECORDING    3

/*异常心率值记录*/
#define    ABNORMAL_RECORDING    3

/*每次搜索能存放的最大波谷数*/
#define    MAX_VALLEY_NUM    40

/*极值点搜索窗口大小，确保极限心率值可计算出*/
#define    SEARCH_WINDOW_SIZE    13

/*均值滤波宽度*/
#define    AVERAGE_FILTER_WIDTH    5

/*保存心率值的历史秒数*/
#define    HB_RECORD_SEC    3

/*保存提示信息的历史秒数*/
#define    WARNING_RECORD_SEC    3

/*int compare with float*/
#define    COMPARE_EPS    0.0001

/*first 1.2s will not display*/
#define    UNDISPLAY_SEDS    1.2

/*调光数据差值阈值*/
#define    DIM_THRESHOLD    1200//1200

#define    DIM_THRESHOLD2    900//900：低通滤波器中，判断是否有调光的亮点的差值阈值
#define    DIM_THRESHOLD3    400//400：修正低通滤波函数中，判断是否有调光的亮点的差值阈值
/*低通滤波器阶数*/
#define    FILTER_ORDER    21//11

/*此时间(单位：秒)若未计算出心率值，则给出第一个warning*/
#define    INITIAL_WARNING_TIME    10

#define    UNTOUCH    0 /*"请如图示将手轻放在传感器上，并保持直到测量完成." */
#define    SIGNAL_SEARCHING    1  /*"信号搜索中." */
#define    TESTING    2  /*"心率测量中." */
#define    TOOHARD    3  /*"按压力度太大.*/
#define    BADSIGNAL    4  /*"心率信号不稳定.*/
#define    TEST_FAIL    5  /*"测量失败，请如图示重新将手指轻放在传感器上，并保持直到测量完成.*/

#define    FREQ    50  /*采样率定义可变 20151010 改为50hz，之前采用380hz*/
#define    FRAME_LEN    10  /*分10次传*/
#define    VALID_DATA_NUM    (FREQ / FRAME_LEN)  /*每次传5个点，所以5个点的时间是一样的*/

/*datas from FW to algorithm*/
typedef struct HBDRAWDATA {
    short dacvalue; /*DAC 输出的原始数据*/
    char KeyStatus;
} HBD_RAW_DATA;

/*datas output from algorithm for display*/
typedef struct HBDMESSAGE {
    unsigned char HeartBeatRate; //心率计算结果
    unsigned char HBD_Status; //提示信息 : SIGNAL_SEARCHING/ UNTOUCH/ TESTING / TOOHARD / SIGNAL_MOVING / TEST_FAIL
    signed int DisplayData[FREQ / FRAME_LEN]; //用于绘制界面
    unsigned char BeatIndex; //心跳一次，震动一次，震动提示，反馈1时震动，反馈0时不震动  20151124 add
} HBD_MESSAGE;

/**
 * add wrapper API for heart beat algo
 * **/
gf_error_t gf_heart_beat(uint8_t *buf, uint32_t length, gf_heart_beat_t *message);

void gf_heart_beat_init(void);

unsigned char* gf_get_heart_beat_version(void); //算法版本号

#ifdef __cplusplus
}
#endif

#endif /*__GF_HEART_BEAT_H__*/
