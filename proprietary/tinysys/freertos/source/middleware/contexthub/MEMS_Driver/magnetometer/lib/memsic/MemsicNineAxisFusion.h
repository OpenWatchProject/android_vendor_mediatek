/*****************************************************************************
 * 九轴融合指南针算法
 * Author: Dong Xiaoguang
 * Created on 2015/12/14
 *****************************************************************************/

#ifndef MEMSIC_NINE_AXIS_FUSION
#define MEMSIC_NINE_AXIS_FUSION

#include "mymath.h"
#include "MemsicCommon.h"

//-----------------------------------------------------------------------------
//初始化参数
typedef struct _ini_para
{
    float t;
    float *iniCalPara;
    int iniAccuracy;
    float *si;
    float *magVar;
    int enableGyroCal;
    int outlierGyroRestart;
    float *iniGyroBias;
    int gyroGrade;
    int *extraSampleCount;
} IniPara;

//===================================================================
//-------------------------------------------------------------------
// 算法初始化，设置采样周期、文件中保存的硬磁校准结果、磁测量噪声的方差、软磁
// input:		iniPara，具体见IniPara定义
// output:
// return:
DLL_API void InitializeAlgo(IniPara iniPara);

//-------------------------------------------------------------------
// 算法主过程
// input:   a--加速度，m/s2
//          m--磁，uT
//          w--角速度，rad/s
//          t--采样间隔, s
//          iRestart--1 means restart, 0 means not
//			enableMagCal--1 means automatic mag calibration is enabled, 0 not
// output:
// return:
DLL_API void MainAlgoProcess(float *a, float *m, float *w, float t, int iRestart, int enableMagCal);

//-------------------------------------------------------------------
// input:   a--加速度，m/s2
//          m--磁，uT
//          w--角速度，rad/s
//          t--采样间隔, s
//          iRestart--1 means restart, 0 means not
//			enableMagCal--1 means automatic mag calibration is enabled, 0 not
// output:
// return:
DLL_API void NineAxisFusion(float *acc, float *thisCalMag, float *cg, float ts, int iRestart);

//-------------------------------------------------------------------
// 获取磁饱和状态
// input:
// output:
// return: 1 means saturation and 0 means no saturation
DLL_API int GetMagSatStatus(void);

//-------------------------------------------------------------------
// 获取指南针精度
// input:
// output:
// return:  磁精度，0、1、2和3
DLL_API int GetAccuracy(void);

//-------------------------------------------------------------------
// 获取磁精度
// input:
// output:
// return:  磁精度，0、1、2和3
DLL_API int GetMagCalAccuracy(void);

//-------------------------------------------------------------------
// 获取校准后的磁
// input:
// output:  cm--校准后的磁，uT
// return:
DLL_API void GetCalibratedMag(float *cm);

//-------------------------------------------------------------------
// 获取校准后的陀螺
// input:
// output:  cg--校准后的磁，rad/s
// return:
DLL_API int GetCalibratedGyro(float *cg);

//-------------------------------------------------------------------
// 获取磁校准参数
// input:
// output:  cp--磁校准参数，[ox,oy,oz,magEarth]，uT
// return:  1 means the param is confirmed and need to be saved, 0 not
DLL_API int GetCalibrationPara(float *cp);

//-------------------------------------------------------------------
// 获取陀螺常偏
// input:
// output:  b--陀螺常偏，rad/s
// return:
DLL_API int GetGyroCalPara(float *b);

//-------------------------------------------------------------------
// 获取rotation vector
// input:
// output:	rv--vector part of the attitude quaternion
// return:
DLL_API void GetRotVec(float *rv);

//-------------------------------------------------------------------
// 获取orientation
// input:
// output:   ori--[yaw,pitch,roll]，deg
// return:
DLL_API void GetOri(float *ori);

//-------------------------------------------------------------------
// 获取线加速度
// input:
// output:  la--线加速度，m/s2
// return:
DLL_API void GetLinearAcc(float *la);

//-------------------------------------------------------------------
// 获取重力加速度
// input:
// output: g--重力加速度，m/s2
// return
DLL_API void GetGravity(float *g);

DLL_API void GetMagRotVec(float *Magrv);
DLL_API void GetGameRotVec(float *Gmrv);


#endif // MEMSIC_NINE_AXIS_FUSION
