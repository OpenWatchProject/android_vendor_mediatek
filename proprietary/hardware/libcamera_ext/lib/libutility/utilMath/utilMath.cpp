#define LOG_TAG "utilMath"

#include <stdio.h>
#include <string.h>
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define LOGD printf
#endif

#include "utilMath.h"

MINT32 utilFloorf(MFLOAT i)
{
    MINT32 x;
    x = (i<0) ? (MINT32) i-1: (MINT32) i;
    return x;
}

//warning!!input is integer. less precise
MUINT32 utilSqrt(const MUINT32 x, const MUINT32 _FRAC_BITS)
{
    MUINT32 root = 0, rh = 0, rl = x, tdiv, count = 15 + (_FRAC_BITS>>1);

    do {
        rh = (rh<<2) | (rl>>30);
        rl <<= 2;
        root <<= 1;
        tdiv = (root<<1) + 1;
        if (rh >= tdiv)
        {
            rh -= tdiv;
            root += 1;
        }
    } while (count-- != 0);

    return root;
}

MFLOAT utilPow(MFLOAT a, MFLOAT b)
{
    MFLOAT LogBodge=0.346607f;
    MFLOAT PowBodge=0.33971f;
    MFLOAT x,y,z;
    MFLOAT out;
    MINT32 temp;

    memcpy(&temp, &a, sizeof(MFLOAT));
    x=(MFLOAT)temp;
    x*= 1.1920929e-007f; // 1/pow(2,23);
    x=x-127;
    y=x-utilFloorf(x);;
    y=(y-y*y)*LogBodge;
    z= (x+y)*b;

    y=(MFLOAT)(z-utilFloorf(z));
    y=(y-y*y)*PowBodge;

    out=z+127-y;
    out*= (1<<23); //pow(2,23);
    temp = (MINT32)out;
    memcpy(&out, &temp, sizeof(MFLOAT));
    return out;
}

// 3rd term taylor expansion of 3^x where x is near zero
MFLOAT utilPow3E(MFLOAT x)
{
    MFLOAT result;
    MFLOAT ln3 = 1.098612f;

    result = 1 + ln3*x + ln3*ln3*x*x/2;
    return result;
}

// 2nd term taylor expansion of cos(x) where x is near zero
MFLOAT utilCosE(MFLOAT x)
{
    MFLOAT result;

    result = 1 - x*x/2;
    return result;
}

// 2nd term taylor expansion of sin(x) where x is near zero
MFLOAT utilSinE(MFLOAT x)
{
    MFLOAT result;

    result = x - x*x*x/6;
    return result;
}

MFLOAT utilFastLog2 (MFLOAT val)
{
    MINT32 x;
    memcpy(&x, &val, sizeof(MFLOAT));
    const MINT32 log_2 = ((x >> 23) & 255) - 128;
    x &= ~(255 << 23);
    x += 127 << 23;
    memcpy(&val, &x, sizeof(MFLOAT));
    val = ((-1.0f/3) * val + 2) * val - 2.0f/3;

    return (val + log_2);
}


/**********************************/
/* Matrix and Vector operations   */
/**********************************/
// 3x3 matrix multiplication
void utilMatMul (MFLOAT *dst, MFLOAT *A, MFLOAT *B)
{
    MFLOAT *temp = dst;
    *(temp++) = (*(A  )) * (*(B  )) + (*(A+1)) * (*(B+3)) + (*(A+2)) * (*(B+6));
    *(temp++) = (*(A  )) * (*(B+1)) + (*(A+1)) * (*(B+4)) + (*(A+2)) * (*(B+7));
    *(temp++) = (*(A  )) * (*(B+2)) + (*(A+1)) * (*(B+5)) + (*(A+2)) * (*(B+8));
    *(temp++) = (*(A+3)) * (*(B  )) + (*(A+4)) * (*(B+3)) + (*(A+5)) * (*(B+6));
    *(temp++) = (*(A+3)) * (*(B+1)) + (*(A+4)) * (*(B+4)) + (*(A+5)) * (*(B+7));
    *(temp++) = (*(A+3)) * (*(B+2)) + (*(A+4)) * (*(B+5)) + (*(A+5)) * (*(B+8));
    *(temp++) = (*(A+6)) * (*(B  )) + (*(A+7)) * (*(B+3)) + (*(A+8)) * (*(B+6));
    *(temp++) = (*(A+6)) * (*(B+1)) + (*(A+7)) * (*(B+4)) + (*(A+8)) * (*(B+7));
    *(temp  ) = (*(A+6)) * (*(B+2)) + (*(A+7)) * (*(B+5)) + (*(A+8)) * (*(B+8));
}

// 3x3 matrix multiplies with 3x1 vector
void utilMatVecMul(MFLOAT *dst, MFLOAT *mtx, MFLOAT *vec)
{
    // comput vec3 = mtx*vec2 (mx1)
    MFLOAT *temp = dst;
    (*temp++) = (*(mtx  )) * (*(vec  )) +
                (*(mtx+1)) * (*(vec+1)) +
                (*(mtx+2)) * (*(vec+2)) ;
    (*temp++) = (*(mtx+3)) * (*(vec  )) +
                (*(mtx+4)) * (*(vec+1)) +
                (*(mtx+5)) * (*(vec+2)) ;
    (*temp  ) = (*(mtx+6)) * (*(vec  )) +
                (*(mtx+7)) * (*(vec+1)) +
                (*(mtx+8)) * (*(vec+2)) ;
}

// vector scaling
void utilVecScale(MFLOAT *dst, const MFLOAT *src, MFLOAT scale)
{
    MINT32 i;
    for (i=0; i<RANK; i++)
        dst[i] = src[i]*scale;
}

// matrix inverse
void utilMatInv (MFLOAT *dst, MFLOAT *src, MINT32 n)
{
    MFLOAT tmp[RANK][2*RANK];
    MINT32  i, j, k;
    MFLOAT swap;
    // Generate the Extensive Identity matrix
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            tmp[i][j] = *(src+i*n+j);
        }
        for (j=n; j<2*n; j++) {
            if (j==(i+n)) tmp[i][j] = 1.0;
            else          tmp[i][j] = 0.0;
        }
    }

    // Guass-jordan
    for (i=0; i<n; i++)
    {
        //Search for the pivot element
        int max = i;
        for (j=i+1; j<n; j++) {
            if ( UTL_FABS(tmp[j][i])>UTL_FABS(tmp[max][i]) ) max = j;
        }

        // swap the whole raw
        if (max != i) {
            for (k=0; k<2*n; k++) {
                swap = tmp[i][k];
                tmp[i][k] = tmp[max][k];
                tmp[max][k] = swap;
            }
        }

        // Elimilation
        if (i==0) {
            for (j=i+1; j<n; j++) {
                for (k=2*n-1; k>=i; k--) {
                    tmp[j][k] = tmp[j][k] - tmp[i][k]*(tmp[j][i]/tmp[i][i]);
                }
            }
        }
        else {
            for (j=0; j<n; j++) {
                if (j!=i) {
                    for (k=2*n-1; k>=0; k--) {
                        tmp[j][k] = tmp[j][k] - tmp[i][k]*(tmp[j][i]/tmp[i][i]);
                    }
                }
            }
        }
    }

    // Get the inverse value
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            *(dst+i*n+j) = tmp[i][j+n]/tmp[i][i];
        }
    }
}


/**********************************/
/* Sorting functions              */
/**********************************/
void utilSwap(MFLOAT *a, MFLOAT *b)
{
    MFLOAT t=*a; *a=*b; *b=t;
}

void utilQuickSort(MFLOAT arr[], MINT32 beg, MINT32 end)
{
    if (end > beg + 1)
    {
        MFLOAT piv = arr[beg];
        MINT32 l = beg + 1, r = end;
        while (l < r)
        {
            if (arr[l] <= piv)
            {
                l++;
            }
            else
            {
                utilSwap(&arr[l], &arr[--r]);
            }
        }
        utilSwap(&arr[--l], &arr[beg]);
        utilQuickSort(arr, beg, l);
        utilQuickSort(arr, r, end);
    }
}


/**********************************/
/* LEVMAR non-linear optimization */
/**********************************/
MINT32 utilAxEqBLu(MFLOAT *A, MFLOAT *B, MFLOAT *x, MINT32 m, size_t buffer)
{
    MINT32 i, j, k;
    MINT32 *idx, maxi=-1;
    MINT32 a_sz=m*m;
    MFLOAT *a, *work, max, sum, tmp;

    if(!A)
        return 1;

    a=(MFLOAT *)buffer;
    work=a+a_sz;
    idx=(MINT32 *)(work+m);

    for(i=0; i<m; ++i)
    {
        a[i]=A[i];
        x[i]=B[i];
    }
    for(  ; i<a_sz; ++i)
    {
        a[i]=A[i];
    }

    for(i=0; i<m; ++i)
    {
        max=0.0;
        for(j=0; j<m; ++j)
        {
            if((tmp=UTL_FABS(a[i*m+j]))>max)
            {
                max=tmp;
            }
        }
        if(max==0.0)
        {
            return 0;
        }
        work[i]=LM_CNST(1.0)/max;
    }

    for(j=0; j<m; ++j)
    {
        for(i=0; i<j; ++i)
        {
            sum=a[i*m+j];
            for(k=0; k<i; ++k)
            {
                sum-=a[i*m+k]*a[k*m+j];
            }
            a[i*m+j]=sum;
        }
        max=0.0;
        for(i=j; i<m; ++i)
        {
            sum=a[i*m+j];
            for(k=0; k<j; ++k)
            {
                sum-=a[i*m+k]*a[k*m+j];
            }
            a[i*m+j]=sum;
            if((tmp=work[i]*UTL_FABS(sum))>=max)
            {
                max=tmp;
                maxi=i;
            }
        }
        if(j!=maxi)
        {
            for(k=0; k<m; ++k)
            {
                tmp=a[maxi*m+k];
                a[maxi*m+k]=a[j*m+k];
                a[j*m+k]=tmp;
            }
            work[maxi]=work[j];
        }

        idx[j]=maxi;

        if(a[j*m+j]==0.0)
        {
            a[j*m+j]=LM_REAL_EPSILON;
        }

        if(j!=m-1)
        {
            tmp=LM_CNST(1.0)/(a[j*m+j]);
            for(i=j+1; i<m; ++i)
            {
                a[i*m+j]*=tmp;
            }
        }
    }

    for(i=k=0; i<m; ++i)
    {
        j=idx[i];
        sum=x[j];
        x[j]=x[i];
        if(k!=0)
        {
            for(j=k-1; j<i; ++j)
            {
                sum-=a[i*m+j]*x[j];
            }
        }
        else
        {
            if(sum!=0.0)
            {
                k=i+1;
            }
        }
        x[i]=sum;
    }

    for(i=m-1; i>=0; --i)
    {
        sum=x[i];
        for(j=i+1; j<m; ++j)
        {
            sum-=a[i*m+j]*x[j];
        }
        x[i]=sum/a[i*m+i];
    }

    return 1;
}

MINT32 utilLevmarBoxCheck(MFLOAT *lb, MFLOAT *ub, MINT32 num_para)
{
    MINT32 i;

    if(!lb || !ub)
    {
        return 1;
    }

    for(i=0; i<num_para; ++i)
    {
        if(lb[i]>ub[i])
        {
            return 0;
        }
    }

    return 1;
}

MFLOAT utilLevmarL2nrmxmy(MFLOAT *e, MFLOAT *x, MFLOAT *y, MINT32 n)
{
    MINT32 i;
    MINT32 blockn;
    MFLOAT sum0=0.0;

    size_t addr_e, addr_x, addr_y;
    addr_e = (size_t)e;
    addr_x = (size_t)x;
    addr_y = (size_t)y;

    blockn = (n>>2)<<2;

    for(i=blockn-1; i>0; i-=4)
    {
        (*e) = (*x++) - (*y++); sum0 += (*e) * (*e);    e++;
        (*e) = (*x++) - (*y++); sum0 += (*e) * (*e);    e++;
        (*e) = (*x++) - (*y++); sum0 += (*e) * (*e);    e++;
        (*e) = (*x++) - (*y++); sum0 += (*e) * (*e);    e++;
    }

    for(i=n-blockn; i>0; i--)
    {
        (*e) = (*x++) - (*y++); sum0 += (*e) * (*e);    e++;
    }

    e = (MFLOAT *)addr_e;
    x = (MFLOAT *)addr_x;
    y = (MFLOAT *)addr_y;

    return sum0;
}

void utilLevmarTransMatMatMult(MFLOAT *a, MFLOAT *b, MINT32 n, MINT32 m)
{

    MINT32 i, j, k, jj, kk;
    MFLOAT sum, *bim, *akm;
    const MINT32 bsize=BLOCK_SIZE;

    for(jj=0; jj<m; jj+=bsize)
    {
        for(i=0; i<m; ++i)
        {
            bim=b+i*m;
            for(j=UTL_MAX(jj, i); j<UTL_MIN(jj+bsize, m); ++j)
            {
                bim[j]=0.0;
            }
        }

        for(kk=0; kk<n; kk+=bsize)
        {
            for(i=0; i<m; ++i)
            {
                bim=b+i*m;
                for(j=UTL_MAX(jj, i); j<UTL_MIN(jj+bsize, m); ++j)
                {
                    sum=0.0;
                    for(k=kk; k<UTL_MIN(kk+bsize, n); ++k)
                    {
                        akm=a+k*m;
                        sum+=akm[i]*akm[j];
                    }
                    bim[j]+=sum;
                }
            }
        }
    }

    for(i=0; i<m; ++i)
    {
        for(j=0; j<i; ++j)
        {
            b[i*m+j]=b[j*m+i];
        }
    }
}

void utilLnsrch(void* ParaIn, void* ParaOut, LEVMAR_CAL_STRUCT *pLevmarInfo)
{
    MINT32 i, j;
    MFLOAT lambda, tlmbda, rmnlmb;
    MFLOAT scl, rln, sln, slp;
    MFLOAT tmp1, tmp2;
    MFLOAT fpls;
    MFLOAT steptl=LM_CNST(0.34526697);
    LNSRCH_INPUT_STRUCT* pInputData = (LNSRCH_INPUT_STRUCT*)ParaIn ;
    LNSRCH_OUTPUT_STRUCT* pOutputData = (LNSRCH_OUTPUT_STRUCT*)ParaOut;
    MINT32 m=pLevmarInfo->num_para;
    MFLOAT *x =pInputData->x;
    void (*cost_func)(MFLOAT *p, MFLOAT *hx) = pLevmarInfo->cost_func;

    (pInputData->f)*=LM_CNST(0.5);
    pOutputData->iretcd = 2;
    tmp1 = 0.;

    for (i = 0; i < m; ++i)
    {
        tmp1 += pInputData->p[i] * pInputData->p[i];
    }

    sln = (MFLOAT)utilSqrt((MUINT32)tmp1,23);
    if (sln > pInputData->stepmx)
    {
        scl = pInputData->stepmx / sln;
        for(i=0; i<m; ++i)
        {
            pInputData->p[i]*=scl;
        }
        sln = pInputData->stepmx;
    }

    for(i=0, slp=0.; i<m; ++i)
    {
        slp+=pInputData->g[i]*pInputData->p[i];
    }

    rln = 0.;

    for (i = 0; i < m; ++i)
    {
        tmp1 = (UTL_FABS(x[i])>=LM_CNST(1.))? UTL_FABS(x[i]) : LM_CNST(1.);
        tmp2 = UTL_FABS(pInputData->p[i])/tmp1;
        if(rln < tmp2)
        {
            rln = tmp2;
        }
    }

    rmnlmb = steptl / rln;
    lambda = LM_CNST(1.0);

    for(j=__LSITMAX; j>=0; --j)
    {
        for (i = 0; i < m; ++i)
        {
            pOutputData->xpls[i] = x[i] + lambda * pInputData->p[i];
        }

        cost_func(pOutputData->xpls, pLevmarInfo->hx2);
        ++(pLevmarInfo->nfev);

        tmp1=utilLevmarL2nrmxmy(pLevmarInfo->hx2, pLevmarInfo->x, pLevmarInfo->hx2, pLevmarInfo->num_measure);

        fpls=LM_CNST(0.5)*tmp1;

        *(pOutputData->ffpls)=tmp1;

        if (fpls <= pInputData->f + slp * pInputData->alpha * lambda)
        {
            pOutputData->iretcd = 0;
            return;
        }

        if (lambda >= rmnlmb)
        {
            if (LM_FINITE(fpls))
            {
                tlmbda = -lambda * slp / (fpls - pInputData->f - slp);
                lambda = (lambda>tlmbda*5) ? lambda*0.1f : tlmbda*0.5f;
            }
            else
            {
                lambda *= LM_CNST(0.1);
            }
        }
        else
        {
            pOutputData->iretcd = 1;
            return;
        }
    }
    pOutputData->iretcd = 1; /* failed */
    return;
} /* utilLnsrch */

void utilBoxProject(MFLOAT *p, MFLOAT *lb, MFLOAT *ub, MINT32 num_para)
{
    MINT32 i;

    if(!lb)
    { /* no lower bounds */
        if(!ub) /* no upper bounds */
        {
            return;
        }
        else
        { /* upper bounds only */
            for(i=0; i<num_para; ++i)
            {
                if(p[i]>ub[i])
                {
                    p[i]=ub[i];
                }
            }
        }
    }
    else
    {
        if(!ub)
        { /* lower bounds only */
            for(i=0; i<num_para; ++i)
            {
                if(p[i]<lb[i])
                {
                    p[i]=lb[i];
                }
            }
        }
        else /* box bounds */
        {
            for(i=0; i<num_para; ++i)
            {
                p[i]=__MEDIAN3(lb[i], p[i], ub[i]);
            }
        }
    }
}

MINT32 utilLevmarBcDer(LEVMAR_CAL_STRUCT *pLevmarInfo, MINT32 para_max_iter)
{
    LOGD("[utilLevmarBcDer] Enter\n");
    MINT32 i, j;
    MFLOAT mu;
    MFLOAT tmp;
    MINT32 k,l,issolved;

    /* temp work arrays */
    MFLOAT *e,
    *jacTe,
    *jac,
    *jacTjac,
    *Dp,
    *diag_jacTjac,
    *pDp;

    MFLOAT p_eL2, jacTe_inf, pDp_eL2;
    MFLOAT p_L2, Dp_L2=LM_REAL_MAX, dF, dL;
    MFLOAT tau, eps1, eps2, eps2_sq, eps3;
    MFLOAT init_p_eL2;
    MFLOAT t, t0;
    MFLOAT jacTeDp;
    MINT32 gprevtaken=0;
    MINT32 numactive;
    MINT32 nu=2, nu2, stop=0, njev=0, nlss=0;
    const MINT32 nm=pLevmarInfo->num_measure*pLevmarInfo->num_para;
    const MINT32 mm=pLevmarInfo->num_para*pLevmarInfo->num_para;
    LNSRCH_INPUT_STRUCT InputData; // HY, move to global if stack overflow
    LNSRCH_OUTPUT_STRUCT OutputData; // HY, move to global if stack overflow
    void (*cost_func)(MFLOAT *p, MFLOAT *hx) = pLevmarInfo->cost_func;

    InputData.alpha=ALPHA;
    mu=jacTe_inf=t=0.0;

    if(pLevmarInfo->num_measure<pLevmarInfo->num_para)
    {
        LOGD("Function: %s, line: %d\n", __FUNCTION__, __LINE__);
        return LM_ERROR;
    }
    if(!utilLevmarBoxCheck(pLevmarInfo->lb, pLevmarInfo->ub, pLevmarInfo->num_para))
    {
        LOGD("Function: %s, line: %d\n", __FUNCTION__, __LINE__);
        return LM_ERROR;
    }
    pLevmarInfo->opts[3] = (MFLOAT)pLevmarInfo->num_measure;


    //<<<<<<<<<<< Clang build error fix
    tau=pLevmarInfo->opts[0];
    eps1=pLevmarInfo->opts[1];
    eps2=pLevmarInfo->opts[2];
    eps2_sq=eps2*eps2;
    eps3=pLevmarInfo->opts[3];
    //=============================
    //if(pLevmarInfo->opts)
    //{
    //    tau=pLevmarInfo->opts[0];
    //    eps1=pLevmarInfo->opts[1];
    //    eps2=pLevmarInfo->opts[2];
    //    eps2_sq=eps2*eps2;
    //    eps3=pLevmarInfo->opts[3];
    //}
    //else
    //{ // use default values
    //    tau=LM_INIT_MU;
    //    eps1=LM_STOP_THRESH;
    //    eps2=LM_STOP_THRESH;
    //    eps2_sq=eps2*eps2;
    //    eps3=LM_STOP_THRESH;
    //}
    //>>>>>>>>> Original

    /* set up work arrays */
    e=(MFLOAT *) pLevmarInfo->ProcBufAddr;
    jacTe=e + pLevmarInfo->num_measure;
    jac=jacTe + pLevmarInfo->num_para;
    jacTjac=jac + nm;
    Dp=jacTjac + mm;
    diag_jacTjac=Dp + pLevmarInfo->num_para;
    pDp=diag_jacTjac + pLevmarInfo->num_para;
    pLevmarInfo->ProcBufAddr =(size_t)(pDp+pLevmarInfo->num_para);

    for(i=0; i<pLevmarInfo->num_para; ++i)
    {
        pDp[i]=pLevmarInfo->p[i];
    }

    utilBoxProject(pLevmarInfo->p, pLevmarInfo->lb, pLevmarInfo->ub, pLevmarInfo->num_para);
    for(i=0; i<pLevmarInfo->num_para; ++i)
    {
        if(pDp[i]!=pLevmarInfo->p[i])
        {
            //TBD, add error handle code
        }
    }

    cost_func(pLevmarInfo->p, pLevmarInfo->hx2);
    pLevmarInfo->nfev=1;


    p_eL2=utilLevmarL2nrmxmy(e, pLevmarInfo->x, pLevmarInfo->hx2, pLevmarInfo->num_measure);
    init_p_eL2=p_eL2;

    if(!LM_FINITE(p_eL2))
    {
        stop=7;
        LOGD("Function: %s, line: %d\n", __FUNCTION__, __LINE__);
    }

    int proc_cnt = 0;
    for(k=0; k<para_max_iter && !stop; ++k)
    {
        if(p_eL2<=eps3)
        {
            stop=6;
            break;
        }

        utilLmbcDifJacf(jac, pLevmarInfo);
        ++njev;


        if(nm<BLOCK_SIZE_SQUARE)
        {
            MINT32 l, im;
            MFLOAT alpha, *jaclm;

            for(i=mm; i-->0; )
            {
                jacTjac[i]=0.0;
            }
            for(i=pLevmarInfo->num_para; i-->0; )
            {
                jacTe[i]=0.0;
            }

            for(l=pLevmarInfo->num_measure; l-->0; )
            {
                jaclm=jac+l*pLevmarInfo->num_para;
                for(i=pLevmarInfo->num_para; i-->0; )
                {
                    im=i*pLevmarInfo->num_para;
                    alpha=jaclm[i];
                    for(j=i+1; j-->0; )
                    {
                        jacTjac[im+j]+=jaclm[j]*alpha;
                    }
                    jacTe[i]+=alpha*e[l];
                }
            }

            for(i=pLevmarInfo->num_para; i-->0; )
            {
                for(j=i+1; j<pLevmarInfo->num_para; ++j)
                {
                    jacTjac[i*pLevmarInfo->num_para+j]=jacTjac[j*pLevmarInfo->num_para+i];
                }
            }
        }
        else
        {
            utilLevmarTransMatMatMult(jac, jacTjac, pLevmarInfo->num_measure, pLevmarInfo->num_para);

            for(i=0; i<pLevmarInfo->num_para; ++i)
            {
                jacTe[i]=0.0;
            }

            for(i=0; i<pLevmarInfo->num_measure; ++i)
            {
                MFLOAT *jacrow;
                for(l=0, jacrow=jac+i*pLevmarInfo->num_para, tmp=e[i]; l<pLevmarInfo->num_para; ++l)
                {
                    jacTe[l]+=jacrow[l]*tmp;
                }
            }
        }

        for(i=j=numactive=0, p_L2=jacTe_inf=0.0; i<pLevmarInfo->num_para; ++i)
        {
            if(pLevmarInfo->ub && pLevmarInfo->p[i]==pLevmarInfo->ub[i])
            {
                ++numactive;
                if(jacTe[i]>0.0)
                {
                    ++j;
                }
            }
            else if(pLevmarInfo->lb && pLevmarInfo->p[i]==pLevmarInfo->lb[i])
            {
                ++numactive;
                if(jacTe[i]<0.0)
                {
                    ++j;
                }
            }
            else if(jacTe_inf < (tmp=UTL_FABS(jacTe[i])))
            {
                jacTe_inf=tmp;
            }
            diag_jacTjac[i]=jacTjac[i*pLevmarInfo->num_para+i];
            p_L2+=pLevmarInfo->p[i]*pLevmarInfo->p[i];
        }

        if(j==numactive && (jacTe_inf <= eps1))
        {
            Dp_L2=0.0;
            stop=1;
            break;
        }

        if(k==0)
        {
            if(!pLevmarInfo->lb && !pLevmarInfo->ub)
            {
                for(i=0, tmp=LM_REAL_MIN; i<pLevmarInfo->num_para; ++i)
                {
                    if(diag_jacTjac[i]>tmp)
                    {
                        tmp=diag_jacTjac[i];
                    }
                    mu=tau*tmp;
                }
            }
            else
            {
                mu=LM_CNST(0.5)*tau*p_eL2;
            }
        }

        while(proc_cnt<MAX_PROC_CNT)
        {
            proc_cnt++;
            for(i=0; i<pLevmarInfo->num_para; ++i)
            {
                jacTjac[i*pLevmarInfo->num_para+i]+=mu;
            }

            issolved=utilAxEqBLu(jacTjac, jacTe, Dp, pLevmarInfo->num_para, pLevmarInfo->ProcBufAddr);
            ++nlss;

            if(issolved)
            {
                for(i=0; i<pLevmarInfo->num_para; ++i)
                {
                    pDp[i]=pLevmarInfo->p[i] + Dp[i];
                }

                utilBoxProject(pDp, pLevmarInfo->lb, pLevmarInfo->ub, pLevmarInfo->num_para);
                for(i=0, Dp_L2=0.0; i<pLevmarInfo->num_para; ++i)
                {
                    Dp[i]=tmp=pDp[i]-pLevmarInfo->p[i];
                    Dp_L2+=tmp*tmp;
                }

                if(Dp_L2<=eps2_sq*p_L2)
                {
                    stop=2;
                    break;
                }

                if(Dp_L2>=(p_L2+eps2)/(EPSILON*EPSILON))
                {
                    stop=4;
                    LOGD("Function: %s, line: %d\n", __FUNCTION__, __LINE__);
                    break;
                }

                cost_func(pDp, pLevmarInfo->hx2);
                ++pLevmarInfo->nfev;

                pDp_eL2=utilLevmarL2nrmxmy(pLevmarInfo->hx2, pLevmarInfo->x, pLevmarInfo->hx2, pLevmarInfo->num_measure);

                if(!LM_FINITE(pDp_eL2))
                {
                    stop=7;
                    LOGD("Function: %s, line: %d\n", __FUNCTION__, __LINE__);
                    break;
                }

                if(pDp_eL2<=GAMMA_SQ*p_eL2)
                {
                    for(i=0, dL=0.0; i<pLevmarInfo->num_para; ++i)
                    {
                        dL+=Dp[i]*(mu*Dp[i]+jacTe[i]);
                    }

                    if(dL>0.0)
                    {
                        dF=p_eL2-pDp_eL2;
                        tmp=(LM_CNST(2.0)*dF/dL-LM_CNST(1.0));
                        tmp=LM_CNST(1.0)-tmp*tmp*tmp;
                        mu=mu*( (tmp>=ONE_THIRD)? tmp : ONE_THIRD );
                    }
                    else
                    {
                        mu=(mu>=pDp_eL2)? pDp_eL2 : mu;
                    }

                    nu=2;

                    for(i=0 ; i<pLevmarInfo->num_para; ++i)
                    {
                        pLevmarInfo->p[i]=pDp[i];
                    }

                    for(i=0; i<pLevmarInfo->num_measure; ++i)
                    {
                        e[i]=pLevmarInfo->hx2[i];
                    }
                    p_eL2=pDp_eL2;
                    gprevtaken=0;
                    break;
                }
            }
            else
            {
                mu*=nu;
                nu2=nu<<1;
                if(nu2<=nu)
                {
                    stop=5;
                    break;
                }
                nu=nu2;

                for(i=0; i<pLevmarInfo->num_para; ++i)
                {
                    jacTjac[i*pLevmarInfo->num_para+i]=diag_jacTjac[i];
                }

                continue;
            }

            for(i=0, jacTeDp=0.0; i<pLevmarInfo->num_para; ++i)
            {
                jacTe[i]=-jacTe[i];
                jacTeDp+=jacTe[i]*Dp[i];
            }

            InputData.x=pLevmarInfo->p;
            InputData.f=p_eL2;
            InputData.g=jacTe;
            InputData.p=Dp;
            OutputData.xpls=pDp;
            OutputData.ffpls=&pDp_eL2;

            if(jacTeDp<=-THO*utilPow(Dp_L2, _POW_/LM_CNST(2.0)))
            {
                tmp=(MFLOAT)utilSqrt((MUINT32)p_L2,23);
                InputData.stepmx=LM_CNST(1e3)*( (tmp>=LM_CNST(1.0))? tmp : LM_CNST(1.0) );

                utilLnsrch(&InputData, &OutputData, pLevmarInfo);

                if(OutputData.iretcd!=0)
                {
                    goto gradproj;
                }

                gprevtaken=0;
            }
            else
            {
            gradproj:

            for(i=0, tmp=0.0; i<pLevmarInfo->num_para; ++i)
            {
                tmp+=jacTe[i]*jacTe[i];
            }
            tmp=(MFLOAT)utilSqrt((MUINT32)tmp,23);  //HY, the result will be a little different by using utilSqrt
            tmp=LM_CNST(100.0)/(LM_CNST(1.0)+tmp);
            t0=(tmp<=LM_CNST(1.0))? tmp : LM_CNST(1.0);

            for(t=(gprevtaken)? t : t0; t>TMING; t*=BETA)
            {
                for(i=0; i<pLevmarInfo->num_para; ++i)
                {
                    pDp[i]=pLevmarInfo->p[i] - t*jacTe[i];
                }

                utilBoxProject(pDp, pLevmarInfo->lb, pLevmarInfo->ub, pLevmarInfo->num_para);

                for(i=0; i<pLevmarInfo->num_para; ++i)
                {
                    Dp[i]=pDp[i]-pLevmarInfo->p[i];
                }


                cost_func(pDp, pLevmarInfo->hx2);
                ++pLevmarInfo->nfev;


                pDp_eL2=utilLevmarL2nrmxmy(pLevmarInfo->hx2, pLevmarInfo->x, pLevmarInfo->hx2, pLevmarInfo->num_measure);

                if(!LM_FINITE(pDp_eL2))
                {
                    stop=7;
                    LOGD("Function: %s, line: %d\n", __FUNCTION__, __LINE__);
                    goto breaknested;
                }

                for(i=0, tmp=0.0; i<pLevmarInfo->num_para; ++i)
                tmp+=jacTe[i]*Dp[i];

                if(gprevtaken && pDp_eL2<=p_eL2 + LM_CNST(2.0)*LM_CNST(0.99999)*tmp)
                {
                    t=t0;
                    gprevtaken=0;
                    continue;
                }
                if(pDp_eL2<=p_eL2 + LM_CNST(2.0)*ALPHA*tmp)
                {
                    break;
                }
            }

            gprevtaken=1;
            }

            for(i=0, Dp_L2=0.0; i<pLevmarInfo->num_para; ++i)
            {
                tmp=pDp[i]-pLevmarInfo->p[i];
                Dp_L2+=tmp*tmp;
            }

            if(Dp_L2<=eps2_sq*p_L2)
            {
                stop=2;
                break;
            }

            for(i=0 ; i<pLevmarInfo->num_para; ++i)
            {
                pLevmarInfo->p[i]=pDp[i];
            }

            for(i=0; i<pLevmarInfo->num_measure; ++i)
            {
                e[i]=pLevmarInfo->hx2[i];
            }
            p_eL2=pDp_eL2;
            break;
        }
    }

    breaknested:

    if(k>=para_max_iter)
    {
        stop=3;
    }

    for(i=0; i<pLevmarInfo->num_para; ++i)
    {
        jacTjac[i*pLevmarInfo->num_para+i]=diag_jacTjac[i];
    }

    //Clang build error fix, array pointer "pLevmarInfo->info" alway be true(non-null)
    //if(pLevmarInfo->info)
    {
        pLevmarInfo->info[0]=init_p_eL2;
        pLevmarInfo->info[1]=p_eL2;
        pLevmarInfo->info[2]=jacTe_inf;
        pLevmarInfo->info[3]=Dp_L2;
        for(i=0, tmp=LM_REAL_MIN; i<pLevmarInfo->num_para; ++i)
        {
            if(tmp<jacTjac[i*pLevmarInfo->num_para+i])
            {
                tmp=jacTjac[i*pLevmarInfo->num_para+i];
            }
        }
        pLevmarInfo->info[4]=mu/tmp;
        pLevmarInfo->info[5]=(MFLOAT)k;
        pLevmarInfo->info[6]=(MFLOAT)stop;
        pLevmarInfo->info[7]=(MFLOAT)pLevmarInfo->nfev;
        pLevmarInfo->info[8]=(MFLOAT)njev;
        pLevmarInfo->info[9]=(MFLOAT)nlss;
    }

    LOGD("[utilLevmarBcDer] Process count = %d\n", proc_cnt);
    LOGD("[utilLevmarBcDer] Exit\n");
    return (stop!=4 && stop!=7)?  k : LM_ERROR;
}

void utilLmbcDifJacf(MFLOAT *jac, LEVMAR_CAL_STRUCT *pLevmarInfo)
{
    MFLOAT *p = pLevmarInfo->p;
    void (*cost_func)(MFLOAT *p, MFLOAT *hx) = pLevmarInfo->cost_func;
    MINT32 i, j;
    MFLOAT tmp;
    MFLOAT d;


    if(pLevmarInfo->ffdif)
    {
        cost_func(p, pLevmarInfo->hx1);

        for(j=0; j<pLevmarInfo->num_para; ++j)
        {
            d=LM_CNST(1E-04)*p[j];
            d=UTL_FABS(d);
            if(d<pLevmarInfo->delta)
            {
                d=pLevmarInfo->delta;
            }

            tmp=p[j];
            p[j]+=d;

            cost_func(p, pLevmarInfo->hxx);

            p[j]=tmp;

            d=LM_CNST(1.0)/d;
            for(i=0; i<pLevmarInfo->num_measure; ++i)
            {
                jac[i*pLevmarInfo->num_para+j]=(pLevmarInfo->hxx[i]-pLevmarInfo->hx1[i])*d;
            }
        }
    }
    else
    {
        for(j=0; j<pLevmarInfo->num_para; ++j)
        {
            d=LM_CNST(1E-04)*p[j];
            d=UTL_FABS(d);
            if(d<pLevmarInfo->delta)
            {
                d=pLevmarInfo->delta;
            }

            tmp=p[j];
            p[j]-=d;
            cost_func(p, pLevmarInfo->hx1);

            p[j]=tmp+d;
            cost_func(p, pLevmarInfo->hxx);

            p[j]=tmp;

            d=LM_CNST(0.5)/d;
            for(i=0; i<pLevmarInfo->num_measure; ++i)
            {
                jac[i*pLevmarInfo->num_para+j]=(pLevmarInfo->hxx[i]-pLevmarInfo->hx1[i])*d;
            }
        }
    }
}

MINT32 utilLevmarBufferSizeQuery(MINT32 NumPara, MINT32 NumMeasure, MINT32 IterNum, MINT32 ImgNum)
{
    MINT32 buffer_size = 0;
    MINT32 ri_data_size = NumMeasure*4 + 9;
    MINT32 lmbcder_size = (NumMeasure*5 + NumPara*8);
    MINT32 axeqblu_size = ((NumPara+2) * NumPara) * IterNum;

    buffer_size += ri_data_size * sizeof(MFLOAT);       // adata
    buffer_size += NumPara * sizeof(MFLOAT);            // p
    buffer_size += NumPara * sizeof(MFLOAT);            // lb
    buffer_size += NumPara * sizeof(MFLOAT);            // ub
    buffer_size += NumMeasure * sizeof(MFLOAT);         // x
    buffer_size += ri_data_size * sizeof(MFLOAT);       // hx1
    buffer_size += NumMeasure * sizeof(MFLOAT);         // hx2
    buffer_size += NumMeasure * sizeof(MFLOAT);         // hxx
    buffer_size += ImgNum * (lmbcder_size + axeqblu_size) * sizeof(MFLOAT);

    return buffer_size;
}

void utilLevmarInit(LEVMAR_CAL_STRUCT *pLevmarInfo, void *pBuffer, MINT32 NumPara, MINT32 NumMeasure, void (*pCostFunc)(MFLOAT *p, MFLOAT *hx))
{
    MINT32 ri_data_size = NumMeasure*4 + 9;

    pLevmarInfo->num_para = NumPara;
    pLevmarInfo->adata=(MFLOAT*)(pBuffer);
    pLevmarInfo->p = pLevmarInfo->adata + ri_data_size;
    pLevmarInfo->lb = pLevmarInfo->p + pLevmarInfo->num_para;
    pLevmarInfo->ub = pLevmarInfo->lb + pLevmarInfo->num_para;
    pLevmarInfo->x = pLevmarInfo->ub + pLevmarInfo->num_para;
    pLevmarInfo->hx1 = pLevmarInfo->x + NumMeasure;
    pLevmarInfo->hx2 = pLevmarInfo->hx1 + ri_data_size;
    pLevmarInfo->hxx = pLevmarInfo->hx2 + NumMeasure;
    pLevmarInfo->ProcBufAddr = (size_t)(pLevmarInfo->hxx + NumMeasure);

    // init
    for(MINT32 i=0;i<pLevmarInfo->num_para;i++)
    {
        pLevmarInfo->p[i]=0;
        pLevmarInfo->lb[i]=-1.f;
        pLevmarInfo->ub[i]=1.f;
    }
    pLevmarInfo->opts[0] = (MFLOAT)1E-02;           // initial step size LM_INIT_MU      1E-03
    pLevmarInfo->opts[1] = (MFLOAT)1E-12;           // ||J^T e||_inf :   LM_STOP_THRESH  1E-17
    pLevmarInfo->opts[2] = (MFLOAT)1E-12;           // ||Dp||_2 :        LM_STOP_THRESH  1E-17
    pLevmarInfo->opts[3] = (MFLOAT)1E-08;           // ||e||_2 :         LM_STOP_THRESH  1E-17
    pLevmarInfo->opts[4] = (MFLOAT)1E-04;           // 1E-04;  //        LM_DIFF_DELTA   1E-06
    memset(pLevmarInfo->x, 0, NumMeasure*sizeof(MFLOAT));

    // cost function
    pLevmarInfo->cost_func = pCostFunc;
}

MINT32 utilLevmarBcDif(LEVMAR_CAL_STRUCT *pLevmarInfo, MINT32 para_max_iter)
{
    LOGD("[utilLevmarBcDif] Enter\n");
    MINT32 ret;

    //Clang build error fix, array pointer "pLevmarInfo->opts" alway be true(non-null)
    //pLevmarInfo->ffdif=(!pLevmarInfo->opts) || (pLevmarInfo->opts[4]>=0.0);
    //pLevmarInfo->delta=(pLevmarInfo->opts)? UTL_FABS(pLevmarInfo->opts[4]) : LM_DIFF_DELTA;
    pLevmarInfo->ffdif = (pLevmarInfo->opts[4]>=0.0);
    pLevmarInfo->delta = UTL_FABS(pLevmarInfo->opts[4]);

    ret=utilLevmarBcDer(pLevmarInfo, para_max_iter);

    //Clang build error fix, array pointer "pLevmarInfo->info" alway be true(non-null)
    //if(pLevmarInfo->info)
    {
        if(pLevmarInfo->ffdif)
        {
            pLevmarInfo->info[7]+=pLevmarInfo->info[8]*(pLevmarInfo->num_para+1);
        }
        else
        {
            pLevmarInfo->info[7]+=pLevmarInfo->info[8]*(2*pLevmarInfo->num_para);
        }
    }
    LOGD("[utilLevmarBcDif] Exit\n");
    return ret;
}


bool utilsolveLS_Sym_int(MINT32 *A, MINT32 *x, const MUINT32 n, const MUINT32 _FRAC_BITS)
{
    // [20131011] avoid div0 error
    if( utilCholeskyDecomposition_int(A, n, _FRAC_BITS) != true)                                // Cholesky decomposition
    {
        LOGD("[Error] in %s!\n", __FUNCTION__);
        return false;
    }
    utilforElim_int(A, x, n, _FRAC_BITS);                                                       // forward elimination
    utiltranspose_int(A, n);                                                                    // transpose matrix
    utilbackSub_int(A, x, n, _FRAC_BITS);                                                       // backward substitution
    return true;
}


void utilforElim_int(const MINT32 *L, MINT32 *y, const MUINT32 n, const MUINT32 _FRAC_BITS)
{
    const MINT32* const y_end = y + n;
    MINT32 v;
    MINT32 *y_ptr0 = NULL, *y_ptr1 = NULL;
    const MINT32 *row_start = NULL, *row = NULL;

    for (y_ptr0 = y, row_start = L; y_ptr0 != y_end; row_start += n, y_ptr0++) {
        v = (*y_ptr0);
        for (y_ptr1 = y, row = row_start; y_ptr1 != y_ptr0; y_ptr1++, row++)
            v -= utilxmul_64((*y_ptr1), (*row), _FRAC_BITS);
        (*y_ptr0) = utilxdiv(v, (*row), _FRAC_BITS);
    }
}


void utilbackSub_int(const MINT32 *U, MINT32 *y, const MUINT32 n, const MUINT32 _FRAC_BITS)
{
    MINT32* const y_last = y + n - 1;
    MINT32 v;
    MINT32 *y_ptr0 = NULL, *y_ptr1 = NULL;
    const MINT32 *row_start = NULL, *row = NULL;

    for (y_ptr0 = y_last, row_start = (U + n * n - 1); y_ptr0 != (y - 1); row_start -= n, y_ptr0--) {
        v = (*y_ptr0);
        for (y_ptr1 = y_last, row = row_start; y_ptr1 != y_ptr0; y_ptr1--, row--)
            v -= utilxmul_64((*y_ptr1), (*row), _FRAC_BITS);
        (*y_ptr0) = utilxdiv(v, (*row), _FRAC_BITS);
    }
}

bool utilCholeskyDecomposition_int(MINT32* A, const MUINT32 n, const MUINT32 _FRAC_BITS)
{
    MUINT32 i, j, k;
    MINT32 v;
    for (k = 0; k < n; k++)
    {
        for (i = 0; i < k; i++)
        {
            v = A[k * n + i];
            for (j = 0; j < i; j++)
            {
                v -= utilxmul_64(A[k * n + j], A[i * n + j], _FRAC_BITS);
            }
            // [20131011] avoid div0 error
            if (A[i * n + i] == 0)
            {
                LOGD("[%s] Error div0!\n", __FUNCTION__);
                return false;
            }
            A[k * n + i] = utilxdiv(v, A[i * n + i], _FRAC_BITS);
        }

        v = A[k * n + k];

        for (j = 0; j < k; j++)
        {

            v -= utilxmul_64(A[k * n + j], A[k * n + j], _FRAC_BITS);
        }

        // [20131011] avoid div0 error
        if (v <= 0)
        {
            LOGD("[%s] Error v <= 0!\n", __FUNCTION__);
            return false;
        }
        //A[k * n + k] = xsqrt(v, _FRAC_BITS);
        A[k * n + k] = utilSqrt(v, _FRAC_BITS);
    }
    return true;
}


void utiltranspose_int(MINT32 *M, const MUINT32 n)
{
    const MINT32* const M_end = (M + n * n);                                           // end position of M
    MINT32 t;
    MINT32 *M_ptr = NULL, *row_ptr = NULL, *col_ptr = NULL;                   // M_ptr: point to each diagonal element, row_ptr: pointer to a row, col_ptr: pointer to a column

    for (M_ptr = M; M_ptr < M_end; M_ptr += (n + 1)) {
        // swap row & column elements
        for (row_ptr = (M_ptr + 1), col_ptr = (M_ptr + n); col_ptr < M_end; row_ptr++, col_ptr += n) {
            t = (*row_ptr);
            (*row_ptr) = (*col_ptr);
            (*col_ptr) = t;
        }
    }
}


MINT32 utilxdiv(MINT32 numerator, MINT32 denominator, MUINT32 BITS)
{
    MUINT32 i;
    MINT32 sign1, sign2, sign;                                                           // sign, positive: 00000...000, negative: 11111...111
    MUINT32 divisor, reminder, answer;

    sign1 = numerator>>31;                                                                  // get sign of numerator
    reminder = (numerator ^ sign1) - sign1;                                                 // reminder is the abs(numerator)
    sign2 = denominator>>31;                                                                // get sign of denominator
    divisor = (denominator ^ sign2) - sign2;                                                // divisor is the abs(denominator)
    sign = sign1 ^ sign2;                                                                   // sign is the sign mask for output result

    answer = reminder / divisor;
    reminder -= answer * divisor;
    answer = answer<<BITS;

    for (i = BITS; i > 0; i--) {
        reminder = reminder<<1;
        if (reminder >= divisor)
        {
            reminder -= divisor;
            answer += (1<<(i - 1));
        }
    }

    return (MINT32)((answer + sign) ^ sign);
}

MINT32 utilxmul_64(MINT32 x, MINT32 y, MUINT32 BITS)
{
    MINT32 sign1, sign2, sign;
    MUINT32 _x, _y, _x0, _x1, _y0, _y1, v0, v1, v2, v;

    sign1 = x>>31;
    _x = (x ^ sign1) - sign1;
    sign2 = y>>31;
    _y = (y ^ sign2) - sign2;
    sign = sign1 ^ sign2;

    _x1 = _x>>16;
    _x0 = _x & 0x0000FFFF;
    _y1 = _y>>16;
    _y0 = _y & 0x0000FFFF;
    v0  = _x0 * _y0;
    v2  = _x1 * _y1;
    v1  = (_x0 + _x1) * (_y0 + _y1) - v2 - v0 + (v0>>16);
    v2 += v1>>16;
    v1  = v1 & 0x0000FFFF;
    v   = (v2<<(32 - BITS));

    if (BITS >= 16)
        v = v | (v1>>(BITS - 16));
    else
        v = v | (v1<<(16 - BITS)) | ((v0 & 0x0000FFFF)>>BITS);
    return (MINT32) ((v + sign) ^ sign);
}
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
void utilinverse(float* M, const unsigned int n, float* MI)
{
    unsigned int i, j, k;
    float m = 0;

    // initialize MI
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
            MI[i * n + j] = 0;
        MI[i * n + i] = 1;
    }

    // compute Gaussian elimination with partial pivoting
    for (i = 0; i < n; i++)
    {
        // find the max element of this column
        m = UTL_FABS(M[i * n + i]);
        k = i;
        for (j = (i + 1); j < n; j++)
        {
            if (UTL_FABS(M[j * n + i]) > m)
            {
                m = UTL_FABS(M[j * n + i]);
                k = j;
            }
        }

        // inter-change the row of M and MI
        if (k != i)
        {
            for (j = i; j < n; j++)
            {
                m = M[i * n + j];
                M[i * n + j] = M[k * n + j];
                M[k * n + j] = m;
            }
            for (j = 0; j < n; j++)
            {
                m = MI[i * n + j];
                MI[i * n + j] = MI[k * n + j];
                MI[k * n + j] = m;
            }
        }

        // added by Maggie
        if ( M[i * n + i] > 0 && M[i * n + i] < 1e-6 )
        {
            M[i * n + i] = 1e-6;
        }
        else if ( M[i * n + i] < 0 && M[i * n + i] > -1e-6 )
        {
            M[i * n + i] = -1e-6;
        }

        // scale the current row
        m = 1.0f / M[i * n + i];
        M[i * n + i] = 1;
        for (j = i + 1; j < n; j++)
            M[i * n + j] *= m;
        for (j = 0; j < n; j++)
            MI[i * n + j] *= m;

        // eliminate the other rows
        for (j = 0; j < n; j++)
        {
            if (j == i)
                continue;
            m = -M[j * n + i];
            M[j * n + i] = 0;
            for (k = i + 1; k < n; k++)
                M[j * n + k] += (m * M[i * n + k]);
            for (k = 0; k < n; k++)
                MI[j * n + k] += (m * MI[i * n + k]);
        }
    }
}

#ifdef DS5
int _isfinitef(float __x)
{
    return ((__FLT(__x) >> 23) & 0xff) != 0xff;
}
#endif
