#ifndef _UTIL_MATH_H_
#define _UTIL_MATH_H_

#include <stdio.h>
#include "MTKUtilCommon.h"

/// dimension of matrix
#define RANK (3)

/// \name comparison and assign
/// @{
#define UTL_CGT(a, b)   (((a) >  (b)) ? (a) : (b))      ///< compare greater than
#define UTL_CGE(a, b)   (((a) >= (b)) ? (a) : (b))      ///< compare greater than or equal
#define UTL_CLT(a, b)   (((a) <  (b)) ? (a) : (b))      ///< compare less than
#define UTL_CLE(a, b)   (((a) <= (b)) ? (a) : (b))      ///< compare less than or equal
#define UTL_MAX         UTL_CGT                         ///< maximum
#define UTL_MIN         UTL_CLT                         ///< minimum
#define UTL_ABS(a)      (((a) > 0   ) ? (a) : -(a))     ///< absolute value of integer
#define UTL_FABS(a)     (((a) > 0.0f) ? (a) : -(a))     ///< absolute value of floating
#define UTL_CLIP(a, min, max)  ((a > max) ? max : ((a < min) ? min : a))
///@}

/************************/
/* common math function */
/************************/
/**
 * \details floor
 * \fn MINT32 utilFloorf(MFLOAT i)
 * \param[in] i floating number
 * \return integer number
 */
MINT32 utilFloorf(MFLOAT i);

/**
 * \details square root
 * \fn MUINT32 utilSqrt(const MUINT32 x, const MUINT32 _FRAC_BITS)
 * \param[in] x input fix-point integer
 * \param[in] _FRAC_BITS bit of fractional part
 * \return integer number
 */
MUINT32 utilSqrt(const MUINT32 x, const MUINT32 _FRAC_BITS);

/**
 * \details power value = a^b
 * \fn MFLOAT utilPow(MFLOAT a, MFLOAT b)
 * \param[in] a input floating number
 * \param[in] b input power
 * \return floating number
 */
MFLOAT utilPow(MFLOAT a, MFLOAT b);

/**
 * \details estimation for power of 3 (near zero)
 * \fn MFLOAT utilPow3E(MFLOAT x)
 * \param[in] x input floating number
 * \return floating number
 */
MFLOAT utilPow3E(MFLOAT x);

/**
 * \details triangular estimation for sine (near zero)
 * \fn MFLOAT utilSinE(MFLOAT x)
 * \param[in] x input floating number
 * \return floating number
 */
MFLOAT utilSinE(MFLOAT x);

/**
 * \details triangular estimation for cosine (near zero)
 * \fn MFLOAT utilCosE(MFLOAT x)
 * \param[in] x input floating number
 * \return floating number
 */
MFLOAT utilCosE(MFLOAT x);

/**
 * \details log2
 * \fn MFLOAT utilFastLog2(MFLOAT val)
 * \param[in] val input floating number
 * \return floating number
 */
MFLOAT utilFastLog2 (MFLOAT val);

/**
 * \details matrix(3x3) multiplies with matrix(3x3)
 * \fn void utilMatMul(MFLOAT *dst, MFLOAT *A, MFLOAT *B)
 * \param[out] dst output matrix
 * \param[in] A first input matrix
 * \param[in] B second input matrix
 */
void utilMatMul(MFLOAT *dst, MFLOAT *A, MFLOAT *B);

/**
 * \details matrix(3x3) multiplies with vector(3x1)
 * \fn void utilMatVecMul(MFLOAT *dst, MFLOAT *mtx, MFLOAT *vec)
 * \param[out] dst output vector
 * \param[in] mtx input matrix
 * \param[in] vec input vector
 */
void utilMatVecMul(MFLOAT *dst, MFLOAT *mtx, MFLOAT *vec);

/**
 * \details vector scaling
 * \fn void utilVecScale(MFLOAT *dst, const MFLOAT *src, MFLOAT scale)
 * \param[out] dst output vector
 * \param[in] src input vector
 * \param[in] scale input scale
 */
void utilVecScale(MFLOAT *dst, const MFLOAT *src, MFLOAT scale);

/**
 * \details matrix(3x3) inverse
 * \fn void utilMatInv(MFLOAT *dst, MFLOAT *src, MINT32 n)
 * \param[out] dst output matrix
 * \param[in] src input matrix
 * \param[in] n matrix rank
 */
void utilMatInv(MFLOAT *dst, MFLOAT *src, MINT32 n);

/**
 * \details data swap
 * \fn void utilSwap(MFLOAT *a, MFLOAT *b)
 * \param[in,out] a first floating number
 * \param[in,out] b second floating number
 */
void utilSwap(MFLOAT *a, MFLOAT *b);

/**
 * \details quick sort
 * \fn void utilQuickSort(MFLOAT arr[], MINT32 beg, MINT32 end)
 * \param[in,out] arr[] input data array
 * \param[in] beg beginning index
 * \param[in] end end index
 */
void utilQuickSort(MFLOAT arr[], MINT32 beg, MINT32 end);

/**********************************/
/* LEVMAR non-linear optimization */
/**********************************/
/// \name LM method
/// @{
#include <float.h>
#include <math.h>
#ifdef SIM_MAIN
#define inline __inline // MSVC
#ifdef __ARMCC_VERSION
#define LM_FINITE isfinite // RVDS
#elif defined(__unix)
#define LM_FINITE isfinite // UNIX
#else
#define LM_FINITE _finite // MSVC
#endif
#else
#define inline __inline // RVDS
#ifdef DS5
#define __FLT(x) (*(unsigned *)&(x))
int _isfinitef(float __x);
#define LM_FINITE _isfinitef // DS5
#else
#define LM_FINITE isfinite // RVDS
#endif
#endif

#define __SUBCNST(x)        x##F
#define LM_CNST(x)          __SUBCNST(x)
#define _POW_               LM_CNST(2.1)
#define ALPHA               LM_CNST(1e-4)
#define BETA                LM_CNST(0.9)
#define GAMMA_SQ            LM_CNST(0.99995)*LM_CNST(0.99995)   ///< gamma=LM_CNST(0.99995),
#define THO                 LM_CNST(1e-8)
#define TMING               LM_CNST(1e-18)                      ///< minimum step length for LS and PG steps
#define __LSITMAX           (150)                               ///< max number of iterations for line search
#define LM_ERROR            (-1)
#define LM_INIT_MU          LM_CNST(1E-03)
#define LM_STOP_THRESH      LM_CNST(1E-17)
#define LM_DIFF_DELTA       LM_CNST(1E-06)
#define LM_REAL_MAX         LM_CNST(3.402823466e+38)
#define LM_REAL_MIN         LM_CNST(1.175494351e-38)
#define LM_REAL_EPSILON     LM_CNST(1.192092896e-07)
#define EPSILON             LM_CNST(1E-12)
#define ONE_THIRD           LM_CNST(0.3333333334)
#define M_PI_R              LM_CNST(0.017453)
#define BLOCK_SIZE          (32)
#define BLOCK_SIZE_SQUARE   (BLOCK_SIZE)*(BLOCK_SIZE)
#define MAX_PROC_CNT        (1000000)
#define LM_OPTS_SZ          (5)
#define LM_INFO_SZ          (10)

/// find the median of 3 numbers
#define __MEDIAN3(a, b, c) ( ((a) >= (b))?\
        ( ((c) >= (a))? (a) : ( ((c) <= (b))? (b) : (c) ) ) : \
        ( ((c) >= (b))? (b) : ( ((c) <= (a))? (a) : (c) ) ) )
/// @}

typedef struct LEVMAR_CAL_STRUCT
{
    size_t ProcBufAddr;
    MINT32 num_para;            /**< I: parameter vector dimension (i.e. number of unknowns) */
    MINT32 num_measure;         /**< I: measurement vector dimension */
    MFLOAT *p;                  /**< I/O: initial parameter estimates. On output has the estimated solution */
    MFLOAT *lb;                 /**< I: vector of lower bounds. If NULL, no lower bounds apply */
    MFLOAT *ub;                 /**< I: vector of upper bounds. If NULL, no upper bounds apply */
    MFLOAT *x;                  /**< I: measurement vector. NULL implies a zero vector */
    MFLOAT opts[LM_OPTS_SZ];    /**< I: opts[0-4] = minim. options [\f$ \mu, \epsilon1, \epsilon2, \epsilon3, \delta \f$]. Respectively the
                                 *   scale factor for initial \f$ \mu \f$, stopping thresholds for \f$ ||J^T e||_inf, ||Dp||_2 and ||e||_2 \f$ and
                                 *   the step used in difference approximation to the Jacobian. Set to NULL for defaults to be used.
                                 *   If \f$ \delta<0 \f$, the Jacobian is approximated with central differences which are more accurate
                                 *   (but slower!) compared to the forward differences employed by default.
                                 */
    MFLOAT info[LM_INFO_SZ];    /**< O: information regarding the minimization. Set to NULL if don't care
                                 *   info[0] = \f$ ||e||_2 \f$ at initial p.
                                 *   info[1-4] = \f$ [ ||e||_2, ||J^T e||_inf,  ||Dp||_2, mu/max[J^T J]_ii ] \f$, all computed at estimated p.
                                 *   info[5]= number of  iterations,
                                 *   info[6]=reason for terminating: 1 - stopped by small gradient \f$ J^T \f$ e
                                 *                                   2 - stopped by small Dp
                                 *                                   3 - stopped by itmax
                                 *                                   4 - singular matrix. Restart from current p with increased mu
                                 *                                   5 - no further error reduction is possible. Restart with increased mu
                                 *                                   6 - stopped by small \f$ ||e||_2 \f$
                                 *                                   7 - stopped by invalid (i.e. NaN or Inf) "func" values. This is a user error
                                 *   info[7]= number of function evaluations
                                 *   info[8]= number of Jacobian evaluations
                                 *   info[9]= number of linear systems solved, i.e. number of attempts for reducing error
                                 */
    MINT32 ffdif;               /**< nonzero if forward differencing is used */
    MINT32 nfev;
    MFLOAT delta;
    MFLOAT *hx1;
    MFLOAT *hx2;                /**< \f$ \hat{x}_i \f$, nx1 */
    MFLOAT *hxx;
    MFLOAT *adata;              /**< pointer to possibly additional data, passed uninterpreted to func.
                                 *   Set to NULL if not needed */

    /// cost function pointer
    void (*cost_func)(MFLOAT *p, MFLOAT *hx);
} LEVMAR_CAL_STRUCT;

typedef struct FUNC_STATE_STRUCT
{
    MINT32 n, *nfev;
    MFLOAT *hx, *x;
    void *adata;
} FUNC_STATE_STRUCT;

typedef struct LMBC_DIF_DATA_STRUCT
{
    MINT32 ffdif;       ///< nonzero if forward differencing is used
    MFLOAT *hx, *hxx;
    void *adata;
    MFLOAT delta;
} LMBC_DIF_DATA_STRUCT;

typedef struct LNSRCH_INPUT_STRUCT
{
    MFLOAT *x;          ///< old iterate:    x[k-1]
    MFLOAT f;           ///< function value at old iterate, f(x)
    MFLOAT *g;          ///< gradient at old iterate, g(x), or approximate
    MFLOAT *p;          ///< non-zero newton step
    MFLOAT alpha;       ///< fixed constant < 0.5 for line search (see above)
    MFLOAT stepmx;      ///< maximum allowable step size
} LNSRCH_INPUT_STRUCT;

typedef struct LNSRCH_OUTPUT_STRUCT
{
    MFLOAT* xpls;       ///< new iterate x[k]
    MFLOAT* ffpls;      ///< function value at new iterate, f(xpls)
    MINT32 iretcd;
} LNSRCH_OUTPUT_STRUCT;

/// \name LM_method internal functions
/// @{
MINT32 utilAxEqBLu(MFLOAT *A, MFLOAT *B, MFLOAT *x, MINT32 m, size_t buffer);
MINT32 utilLevmarBoxCheck(MFLOAT *lb, MFLOAT *ub, MINT32 num_para);
MFLOAT utilLevmarL2nrmxmy(MFLOAT *e, MFLOAT *x, MFLOAT *y, MINT32 n);
void utilLevmarTransMatMatMult(MFLOAT *a, MFLOAT *b, MINT32 n, MINT32 m);
void utilLnsrch(void* ParaIn, void* ParaOut, LEVMAR_CAL_STRUCT *pLevmarInfo);
void utilBoxProject(MFLOAT *p, MFLOAT *lb, MFLOAT *ub, MINT32 num_para);
MINT32 utilLevmarBcDer(LEVMAR_CAL_STRUCT *pLevmarInfo, MINT32 para_max_iter);
void utilLmbcDifJacf(MFLOAT *jac, LEVMAR_CAL_STRUCT *pLevmarInfo);
/// @}

/**
 *  \details LM_moethod buffer size query function
 *  \fn MINT32 utilLevmarBufferSizeQuery(MINT32 NumPara, MINT32 NumMeasure, MINT32 IterNum, MINT32 ImgNum)
 *  \param[in] NumPara number of parameters
 *  \param[in] NumMeasure number of measurement
 *  \param[in] IterNum number of iteration
 *  \param[in] ImgNum number of images
 *  \return required buffer size
 */
MINT32 utilLevmarBufferSizeQuery(MINT32 NumPara, MINT32 NumMeasure, MINT32 IterNum, MINT32 ImgNum);

/**
 *  \details LM_method init function
 *  \fn void utilLevmarInit(LEVMAR_CAL_STRUCT *pLevmarInfo, void *pBuffer, MINT32 NumPara, MINT32 NumMeasure, void (*pCostFunc)(MFLOAT *p, MFLOAT *hx))
 *  \param[in] pLevmarInfo LM method info
 *  \param[in] pBuffer buffer pointer
 *  \param[in] NumPara number of parameters
 *  \param[in] NumMeasure number of measurement
 *  \param[in] pCostFunc cost function
 */
void utilLevmarInit(LEVMAR_CAL_STRUCT *pLevmarInfo, void *pBuffer, MINT32 NumPara, MINT32 NumMeasure, void (*pCostFunc)(MFLOAT *p, MFLOAT *hx));

/**
 *  \details LM_method entry function
 *  \fn MINT32 utilLevmarBcDif(LEVMAR_CAL_STRUCT *pLevmarInfo, MINT32 para_max_iter)
 *  \param[in] pLevmarInfo LM method info
 *  \param[in] para_max_iter maximum iteration
 *  \return LM method error code
 */
MINT32 utilLevmarBcDif(LEVMAR_CAL_STRUCT *pLevmarInfo, MINT32 para_max_iter);


/************************************/
/* FixedPoint Solve Linear System   */
/************************************/


/**
 *  \details utilsolveLS_Sym_int entry function
 *  \fn void utilsolveLS_Sym_int(MINT32 *A, MINT32 *x, const MUINT32 n, const MUINT32 _FRAC_BITS)
 *  \Solve a Linear System
 *      A * x = b
 *  where A is a symmetric n-by-n square matrix.
 *  This function use cholesky decomposition to solve this system.
 *  \param[in] A n-by-n 2D matrix. A[i * n + j] is the i-th row, j-th column element
 *  \param[in] x n-by-1 column vector, x[i] is the i-th element. It holds the input vector b, and also stores the result vector x.
 *  \param[in] n size
 *  \param[out] A the upper triangle matrix of cholesky decomposition result
 *  \param[out] x n-by-1 column vector, x[i] is the i-th element
 */
bool utilsolveLS_Sym_int(MINT32 *A, MINT32 *x, const MUINT32 n, const MUINT32 _FRAC_BITS);


/*! \name utilsolveLS_Sym_int internal functions
 * @{
 */

/**
 * \fn void utilforElim_int(const MINT32 *L, MINT32 *y, const MUINT32 n, const MUINT32 _FRAC_BITS)
 * solve L * x = y
 * L: n-by-n lower triangular square matrix
 * This is forward elimination algorithm, and the result x is stored in y
 * \param[in] L n-by-n 2D matrix, L[i * n + j] is the i-th row, j-th column element
 * \param[in] y n-by-1 column vector, y[i] is the i-th row element
 * \param[in] n size
 * \param[out] y n-by-1 column vector, result
 */
void utilforElim_int(const MINT32 *L, MINT32 *y, const MUINT32 n, const MUINT32 _FRAC_BITS);

/**
 * \fn void utilbackSub_int(const MINT32 *U, MINT32 *y, const MUINT32 n, const MUINT32 _FRAC_BITS)
 * solve U * x = y
 * U: n-by-n upper triangular square matrix
 * This is backward substitution algorithm, and the result x is stored in y
 * \param[in] U n-by-n 2D matrix, L[i * n + j] is the i-th row, j-th column element
 * \param[in] y n-by-1 column vector, y[i] is the i-th row element
 * \param[in] n size
 * \param[out] y n-by-1 column vector, result
 */
void utilbackSub_int(const MINT32 *U, MINT32 *y, const MUINT32 n, const MUINT32 _FRAC_BITS);

/**
 * \fn void utilCholeskyDecomposition_int(MINT32* A, const MUINT32 n, const MUINT32 _FRAC_BITS)
 * Do Cholesky decomposition of a square matrix A
 *     A = H H^T
 * A: is a n-by-n square matrix, H is a n-by-n lower triangular matrix.
 * The decomposition result, H, is stored in matrix A
 * \param[in] A n-by-n 2D matrix, A[i * n + j] is the i-th row, j-th column element
 * \param[in] n size
 * \param[out] A n-by-n 2D matrix, H[i * n + j] is the i-th row, j-th column element
 */
bool utilCholeskyDecomposition_int(MINT32* A, const MUINT32 n, const MUINT32 _FRAC_BITS);

/**
 * \fn void utiltranspose_int(MINT32 *M, const MUINT32 n)
 * transpose a square matrix M
 *    M = M^T
 * This is backward substitution algorithm, and the result x is stored in y
 * \param[in] M n-by-n 2D matrix, M[i * n + j] is the i-th row, j-th column element, transposed matrix is also stored in M
 * \param[in] n size
 * \param[out] M n-by-n 2D matrix, M[i * n + j] is the i-th row, j-th column element
 */
void utiltranspose_int(MINT32 *M, const MUINT32 n);

/**
 * \fn MINT32 utilxmul_64(MINT32 x, MINT32 y, MUINT32 BITS)
 * fixed-point division
 * It computes (numerator / denominator)
 * NOTE: not take cares of
 *   1. denominator == 0
 *   2. overflow
 * \param[in] x numerator
 * \param[in] y denominaotr
 * \param[in] BITS required precision
 * \param[out]  division result, in (31-BITS).BITS format
 */
MINT32 utilxmul_64(MINT32 x, MINT32 y, MUINT32 BITS);

/**
 * \fn MINT32 utilxdiv(MINT32 numerator, MINT32 denominator, MUINT32 BITS)
 * fixed-point division for smaller than 1
 * It computes (numerator / denominator)
 * where |numerator| < |denominator|
 * NOTE: not take cares of
 *   1. denominator == 0
 *   2. overflow
 * \param[in] x numerator
 * \param[in] y denominaotr
 * \param[in] BITS required precision
 * \param[out]  division result, in (31-BITS).BITS format
 */
MINT32 utilxdiv(MINT32 numerator, MINT32 denominator, MUINT32 BITS);
/*! @} */

void utilinverse(float* M, const unsigned int n, float* MI);
/*
 * inverse a square matrix
 *    MI = M^-1, MI * M = I
 * This function uses Gaussian-Jordan elimination with partial pivoting
 *
 * Input:
 * - M: n-by-n 2D square matrix. M[i * n + j] is the i-th row, j-th column element
 * - n: size
 *
 * Output:
 * - MI: n-by-n 2D square matrix. MI[i * n + j] is the i-th row, j-th column element
 */

#endif /* _UTIL_MATH_H_ */

