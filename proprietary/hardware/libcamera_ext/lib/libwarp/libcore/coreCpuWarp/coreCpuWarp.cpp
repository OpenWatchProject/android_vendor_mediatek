#define MTK_LOG_ENABLE 1
#include "coreCpuWarp.h"

#ifdef SIM_MAIN
#define    MY_LOGD        printf
#else
#include <android/log.h>
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,##__VA_ARGS__) 
#endif

/*
    Public
*/
void CoreCpuWarp::CpuWarpingInit(void)
{
}

void CoreCpuWarp::CpuWarpingMain(void)
{
    unsigned char *I = (unsigned char *)core_info.SrcBuffer;
    unsigned char *Iout = (unsigned char *)core_info.DstBuffer;
    int W = core_info.Width;
    int H = core_info.Height;
    int Wout = core_info.ClipWidth;
    int Hout = core_info.ClipHeight;

    int w = core_info.WarpMapSize[0][0];
    int h = core_info.WarpMapSize[0][1];
    int *WarpX = (int *)core_info.WarpMapAddr[0][0];
    int *WarpY = (int *)core_info.WarpMapAddr[0][1];

    int i, j, x1, y1, idx, idx2, xs, ys;
    int ptx, pty, psz1, psz2, qsz1, qsz2;
    int xa, ya, warpx, warpy;
    int interpY;
    int wa = w-1;
    int Wa = W-1; // 0 map to 0, w-1 map to W-1
    int ha = h-1;
    int Ha = H-1; // 0 map to 0, w-1 map to W-1

    int w1, w2,w3,w4;

    psz1 = H*W;
    qsz1 = (psz1>>2);
    psz2 = Hout*Wout;
    qsz2 = (psz2>>2);

    for (i = 0; i < Hout; i++) {
        for (j = 0; j < Wout; j++) {
            idx = i * W + j;
            ptx = j * wa * 32 / Wa; // coordinate in QVGA image, scaleup 5bits
            pty = i * ha * 32 / Ha;

            x1 = (ptx>>5);
            y1 = (pty>>5);

            if(y1>ha-1){
                y1 = ha-1;
                pty = (y1<<5)+32;

                if (x1>wa-1 ){
                 x1 = wa-1;
                 ptx = (x1<<5)+32;
                }
               }

            idx2 = y1*w+x1;

            xa = ptx - (x1<<5);
            ya = pty - (y1<<5);

            w1 = (32-xa)*(32-ya);
            w2 = (xa)*(32-ya);
            w3 = (32-xa)*(ya);
            w4 = (xa)*(ya);

            // MergeWarp: scaleup 4bits
            warpx = WarpX[idx2]*w1;
            warpx += WarpX[idx2+1]*w2;
            warpx += WarpX[idx2+w]*w3;
            warpx += WarpX[idx2+w+1]*w4; // scaleup 10bits

            warpy = WarpY[idx2]*w1;
            warpy += WarpY[idx2+1]*w2;
            warpy += WarpY[idx2+w]*w3;
            warpy += WarpY[idx2+w+1]*w4; // scaleup 10bits

            ptx = (warpx / 1024);
            pty = (warpy / 1024); // scaleup 4 bits
            x1 = (ptx / 16);
            y1 = (pty / 16);

            if (x1<0 || x1>W-2 || y1<0 || y1>H-2){
                xs = MAX(x1, 0);
                xs = MIN(xs, W-1);
                ys = MAX(y1, 0);
                ys = MIN(ys,H-1);

                idx = i * Wout + j;
                idx2 = ys * W + xs;
                Iout[idx] = I[idx2]; // Y

                idx = (i/2)*(Wout/2)+(j/2)+ psz2 ; // V
                idx2 = (y1/2)*(W/2)+(x1/2)+ psz1; // V
                Iout[idx] = I[idx2]; // V

                idx = idx + qsz2 ; // U
                idx2 = idx2 + qsz1 ; // U
                Iout[idx] = I[idx2];  // U
                continue;
            }

            idx2 = y1*W+x1;
            xa = ptx - (x1<<4);
            ya = pty - (y1<<4);

            w1 = (16-xa)*(16-ya);
            w2 = (xa)*(16-ya);
            w3 = (16-xa)*(ya);
            w4 = (xa)*(ya);

            interpY = I[idx2] * w1;
            interpY += I[(idx2+1)] * w2;
            interpY += I[(idx2+W)] * w3;
            interpY += I[(idx2+W+1)] * w4; // scaleup 8 bits

            idx = i * Wout + j;
            Iout[idx] = interpY>>8; // Y

            // YUV 420 3p
            /*
            idx = (i/2)*(Wout/2)+(j/2)+ psz2 ;
            idx2 = (y1/2)*(W/2)+(x1/2)+ psz1;

            Iout[idx] = I[idx2]; // v

            idx = idx + qsz2;
            idx2 = idx2 + qsz1;

            Iout[idx] = I[idx2]; // U
            */


            idx = (i/2)*(Wout/2)+(j/2)+ psz2 ;
            ptx = ptx/2;
            pty = pty/2;
            x1 = (ptx / 16);
            y1 = (pty / 16);

            idx2 = y1*W/2+x1+psz1;
            xa = ptx - (x1<<4);
            ya = pty - (y1<<4);

            w1 = (16-xa)*(16-ya);
            w2 = (xa)*(16-ya);
            w3 = (16-xa)*(ya);
            w4 = (xa)*(ya);

            interpY = I[idx2] * w1;
            interpY += I[(idx2+1)] * w2;
            interpY += I[(idx2+W/2)] * w3;
            interpY += I[(idx2+W/2+1)] * w4; // scaleup 8 bits

            Iout[idx] = interpY>>8; // V

            idx = idx + qsz2;
            idx2 = idx2 + qsz1;

            interpY = I[idx2] * w1;
            interpY += I[(idx2+1)] * w2;
            interpY += I[(idx2+W/2)] * w3;
            interpY += I[(idx2+W/2+1)] * w4; // scaleup 8 bits

            Iout[idx] = interpY>>8; // U

        }
    }

}

void CoreCpuWarp::CpuWarpingReset(void)
{
}

/*
    Private
*/
bool CoreCpuWarp::CpuWarping()
{
    bool OK = true;
    //bool NoOK = false;


    return OK;
}