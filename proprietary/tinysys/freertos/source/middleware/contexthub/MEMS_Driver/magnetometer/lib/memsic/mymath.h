/*****************************************************************************
 * matrix operations\vector operations\buffer operations\mean and variance calculations
 * quaternion operations\mean filter\med value filter\general filter
 * Author: Dong Xiaoguang
 * Created on 2015/11/24
 *****************************************************************************/

#ifndef MY_MATH_H_INCLUDED
#define MY_MATH_H_INCLUDED
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>

#include "MemsicCommon.h"

#define DLL_API

//===========================REAL or float=========================
#define PRECISION_float 0
#if PRECISION_float
#define REAL float
#else
#define REAL float
#endif

//===========================some constants==========================
#if PRECISION_float
#define PI 3.14159265358979323846
#define TWO_PI 6.28318530717959
#define HALF_PI 1.5707963267949
#define R2D 57.2957795130823
#define D2R 0.01745329251994
#define UNIT_QUAT {1.0 ,0.0, 0.0, 0.0}
#else
#define PI 3.14159265358979323846F
#define TWO_PI 6.28318530717959F
#define HALF_PI 1.5707963267949F
#define R2D 57.2957795130823F
#define D2R 0.01745329251994F
#define UNIT_QUAT {1.0f ,0.0f, 0.0f, 0.0f}
#endif
//======================data struct definitions======================
typedef struct _buffer {
    REAL *d;// data storage, each column represents a set of data
    int m;//row number
    int n;//column number
    int i;//index for data being put into the buffer, -1 means buffer is empty
    int full;//1 means buffer is full, 0 not
    int num;//data number in the buffer
} Buffer;

//===================================================================
#define max(a,b)    ((a) > (b) ? (a) : (b))
#define min(a,b)    ((a) > (b) ? (b) : (a))
#define myabs(x)    ( (x)>=(float)0.0 ? (x) : -(x) )

//===========================math operations=========================
//-------------------------------------------------------------------
// print all the elements in a vector
// input:   v--the vector
//          n--vector length
// output:
// return:
void printVec(REAL *v, int n);

//-------------------------------------------------------------------
// print all the elements in a matrix
// input:   a--the matrix
//          m--row number
//          n--column number
// output:
// return:
void printMtx(REAL *a, int m, int n);

//-------------------------------------------------------------------
// vector add
// a, b and c are vectors with n elements
// c = a+b
void vecAdd(REAL* a, REAL* b, REAL* c, int n);

//-------------------------------------------------------------------
// vector subtraction
// a, b and c are vectors with n elements
// c = a-b
void vecSub(REAL* a, REAL* b, REAL* c, int n);

//-------------------------------------------------------------------
// dot product
// a and b are vectors with n elements
// return a.b
REAL dot(REAL* a, REAL* b, int len);

//-------------------------------------------------------------------
// vector 2-norm
// return the 2-norm of a
REAL norm2(REAL* a, int n);

//-------------------------------------------------------------------
// normalize a vector, 1e-15 needs further validation, support in place normalization
// input:   a--vector to be normalized
//          len--vector length
// output:  aN--normalized vector
// return:  2-norm of a
REAL vecNormalize(REAL* a, REAL* aN, int len);

//-------------------------------------------------------------------
// mean value of a vector
// input:   a--vector
//          n--length of the vector
// output:
// return: mean value of the first n elements of vector
REAL vecMean(REAL *a, int n);

//-------------------------------------------------------------------
// variance of a vector
// input:   a--vector
//          am--mean value
//          n--length of the vector
// output:
// return: variance of the first n elements of vector
REAL vecVar(REAL *a, REAL am, int n);

//-------------------------------------------------------------------
// sort a vector, ascending
// input:   a--vector
//          n--length of the vector
// output:  a--sorted vector
// return:
void vecSort(REAL *a, int n);

//-------------------------------------------------------------------
// vector coross product
// input:   a--vector
//          b--vector
// output:   c--axb
// return:
void cross(REAL* a, REAL* b, REAL* axb);

//-------------------------------------------------------------------
// vector multiplied by a constant
// intput:  a--n vector
//          b--constant scalar
//          n--vector length
// output:  c--b*a,n vector
void vecMultiplyConst(REAL* a, REAL b, REAL* c, int n);

//-------------------------------------------------------------------
// vector duplication
// input:   vSrc--source vector
//          n--source vector length
// output:  vDst--dest vector
// return:
void vecDuplicate(REAL* vSrc, REAL* vDst, int n);

//-------------------------------------------------------------------
// max absolute elements in a matrix
// input:   a--a matrix
//          m--rows
//          n--columns
// output:
// return:  max absolute value
REAL mtxMaxAbs(REAL *a, int m, int n);

//-------------------------------------------------------------------
// one column of a matrix
// input:   a--matrix
//          m--row size
//          n--col size
//          r--index of the column to be extracted
// output:  b--the r column of a
void mtxCol(REAL *a, int m, int n, int r, REAL *b);

//-------------------------------------------------------------------
// matrix duplication
// input:   aSrc--source matrix
//          m--row number of the source matrix
//          n--column number of the source matrix
// output:  aDst--dest matrix
// return:
void mtxDuplicate(REAL* aSrc, REAL* aDst, int m, int n);

//-------------------------------------------------------------------
// matrix multiplication
// input: a--matrix(aRow x aCol), b--matrix(aCol x bCol)
// output: c--matrix(aRow x bCol)
void mtxMultiply(REAL* a, REAL* b, REAL* c, int aRow, int aCol, int bCol);

//-------------------------------------------------------------------
// matrix multiplied by a vector
// input: a--mxn matrix, b--nx1 vector
// return: c--a*b,mx1 vector
void mtxMultiplyVec(REAL* a, REAL* b, REAL* c, int m, int n);

//-------------------------------------------------------------------
// 3x3 matrix inverse
// input: a--matrix(3 x 3)
// output: x--inv(x)
int mtxInverse3(REAL* a, REAL* x);
//int mtxInverse3(double* a, double* x);

//-------------------------------------------------------------------
// generate diagonal matrix
// input:   a--nx1 vector containing the diagonal elements
//          n--length of the vector
// output:  b = diag(a), nxn
// return:
void diag(REAL* a, REAL* b, int n);

//-------------------------------------------------------------------
// generate identity matrix
// input:   a--matrix
//          n--size of the identity matrix
// output:  a = eye(n)
// return:
void eye(REAL *a, int n);

//-------------------------------------------------------------------
// generate zeros matrix
// input:   a--matrix
//          m--rows
//          n--columns
// output:  a = zeros(n)
// return:
void zeros(REAL *a, int m, int n);

//-------------------------------------------------------------------
// new a buffer
// input:   bf--pointer to the buffer
//          d--pointer to the memory for data storage
//          m--row number(length of each set of data)
//          n--column number(number of sets of data)
// output:
// return:
void bfNew(Buffer *bf, REAL*d, int m, int n);

//-------------------------------------------------------------------
// put data into the buffer
// input:   bf--buffer pointer
//          d--pointer to the data being put into the buffer
// output:
// return:
void bfPut(Buffer *bf, REAL* d);

//-------------------------------------------------------------------
// read data from the buffer
// input:   bf--buffer pointer
//          idx--data index, idx=0 means the latest data, idx=1 means data before the latest...
// output:
// return: 1 menas OK, 0 menas idx out of bound
int bfGet(Buffer *bf, REAL *d, int idx);

//-------------------------------------------------------------------
// clear the buffer
// input:   bf--buffer pointer
// output:
// return:
void bfClear(Buffer *bf);

//-------------------------------------------------------------------
// normalize a quaternion
// input:   q--input quat
// output:  qN--normalized quat
// return:
void quatNormalize(REAL* q, REAL* qN);

//-------------------------------------------------------------------
// conver a quat to DCM
// input:   q--input quat
// output:  dcm--DCM
// return:
void quat2DCM(REAL *q, REAL *dcm);

//-------------------------------------------------------------------
// conver a DCM to a quat
// input:   a--input DCM
// output:  q--quat
// return:
void dcm2Quat(REAL *a, REAL *q);

//-------------------------------------------------------------------
// quaternion conjungate
// input:   q--input quat
// output:  qconj--conjungate
// return:
void quatConj(REAL *q, REAL *qconj);

//-------------------------------------------------------------------
// quat multiplication
// input:   q1--input quat
//          q2--input quat
// output:  q--q1xq2
// return:
void quatMultiply(REAL* q1, REAL* q2, REAL* q);

//-------------------------------------------------------------------
// integration of a quaternion based on anguler velocity
// input:   q--input quat
//          w--angular velocity
//          t--integration step, s
// output:  q--quat after integration
// return:
void quatIntegrate(REAL* q, REAL* w, REAL t);
//void quatIntegrate2(REAL* q, REAL* w, REAL t);

//-------------------------------------------------------------------
// get quaternion corresponding to the rotation(w*t, rad)
// input:   w--angular velocity
//          t--integration step, s
// output:  pq--quaternion corresponding to w and t
// return:
void RotationQuat(REAL *w, REAL t, REAL *pq);

//-------------------------------------------------------------------
// 30 points to linear align
// input:   data--the array to store the data, data[0] is he oldest one
//          n--the length of the array
// output:  a--2D array, y = a[0]x + a[1]
// return;
void PolyFit1_30(REAL *data, REAL *a, int n);

//-------------------------------------------------------------------
// modulus
// input:   x--
//          y--
// output:
// return:  x - y*floor(x/y)
REAL mod(REAL x, REAL y);


//-------------------------------------------------------------------
// sine (http://stackoverflow.com/questions/345085/how-do-trigonometric-functions-work/345117#345117)
// input:   x--
// output:
// return:  sin(x)
REAL FastSin(REAL x);

#endif // MY_MATH_H_INCLUDED
