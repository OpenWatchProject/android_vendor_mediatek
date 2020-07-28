#ifndef MEMSIC_GYRO_MAG_CAL_H_INCLUDED
#define MEMSIC_GYRO_MAG_CAL_H_INCLUDED

//-------------------------------------------------------------------
// main algorithm for magnetometer calibration aided by gyro
// input: m--magnetometer, unit is uT
//        w--gyroscope, unit is rad/s
//        bNewMag--1 means new mag data, 0 means only new gyro data
//        iRestart--1 means restart, 0 means not
// output:
// return: quality
int GyroAidedMagCal(float* m, float* w, int bNewMag, int iRestart);

//-------------------------------------------------------------------
// get calibratoin parameters
// input:
// output: calPara--offset and squared magnetic field magnitude, [ox,oy,oz,magEarth^2]
// return:
void GetCalPara(float *calPara);

//-------------------------------------------------------------------
// get mag scale factor for quality 2 or 3
// input:
// output:
// retrun: scaleQ23[0];
float GetScaleHighQuality(void);

//-------------------------------------------------------------------
// get the flag if set is needed by calibration process
// input:
// output:
// return:  1 means set is needed, 0 not
int GetNeedSetFromCal(void);

//-------------------------------------------------------------------
// get the calibration status
// input:
// output:
// return:  lowest two bits are used [lsb]=1 means mag offset estimation converges, 0 not
//          [lsb+1]=1 means calibration is running, 0 means calibration stops
int GetCalStatus(void);


//-------------------------------------------------------------------
// set sampling period, noise variance and initial calibration parameters
// input: t--sampling period, unit is second
//        v--noise variance, unit is uT
//        c--calibration parameter [ox oy oz magEarth]
// output:
// return:
void IniGyroMagCal(float t, float *v, float *c, int iniAccuracy, int *extraSampleCount);

//-------------------------------------------------------------------
// set sampling period for magnetometer calibration aided by gyro
// input: t--sampling period, unit is second
// output:
// return:
void SetTsForMagCal(float t);

//-------------------------------------------------------------------
// set noise variance of magnetometer measurements
// input: v--noise variance, unit is uT
// output:
// return:
void SetVarMNoiseForMagCal(float *v);

//-------------------------------------------------------------------
// set initial calibration parameters (unit: uT)
// input:   c--calibration parameter [ox oy oz magEarth^2]
//          iniAccuracy--accuracy of the initial calibration parameters
// output:
// return:
void SetIniCalParaForMagCal(float *c, int iniAccuracy);

//-------------------------------------------------------------------
// manually reset algorithm when saturation
// input:   m0--mag measurements to initiazlie the state vector X
//          afterSet--1 means this function is called after set(saturation), 0 not
// output:
// return:
void ResetGyroAidedMagCal(float *m0, int afterSet);

//-------------------------------------------------------------------
// stop calibration
// input:
// output:
// return:
void StopCal(void);

#endif // MEMSIC_GYRO_MAG_CAL_H_INCLUDED
