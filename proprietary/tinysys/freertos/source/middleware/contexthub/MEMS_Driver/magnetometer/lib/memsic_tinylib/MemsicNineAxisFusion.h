/*****************************************************************************
 * 九轴融合指南针算法
 * Author: Dong Xiaoguang
 * Created on 2015/12/14
 *****************************************************************************/

#ifndef MEMSIC_NINE_AXIS_FUSION
#define MEMSIC_NINE_AXIS_FUSION

#include "platform.h"
#include "MemsicCommon.h"

//-------------------------------------------------------------------
// 算法主过程
// input:   a--加速度，m/s2
//          m--磁，uT
//          w--角速度，rad/s
//          t--采样间隔, s
//          iRestart--1 means restart, 0 means not
//          enableMagCal--1 means automatic mag calibration is enabled, 0 not
// output:
// return:
void NineAxisFusion(float *a, float *m, float *cm, float *magpara, float *w, float t, int iRestart, int enableMagCal,
                    int *Status);
//-------------------------------------------------------------------
// input:   acc--加速度测量
//          w--校准后的陀螺角速度，rad/s
//          t--采样周期，s
//          iRestart--算法重新运行
// output:  acc--滤波后的重力矢量
// return:  1 means filter is ok, 0 not
int UpdateIMU(float *a, float *cg, float t1, int iRestart, float *g, float *la, float *quat);

DLL_API void GetGameRotVec(float *Gmrv);
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

#endif // MEMSIC_NINE_AXIS_FUSION
