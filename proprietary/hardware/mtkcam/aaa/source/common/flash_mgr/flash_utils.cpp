#define LOG_TAG "FlashUtils"

#include <stdio.h>
#include <string.h>
#include <cmath>

/* camera headers */
#include "mtkcam/aaa/aaa_hal_common.h"
#include "camera_custom_awb_nvram.h"
#include "flash_param.h"
#include "flash_utils.h"
#if (CAM3_3A_ISP_30_EN)
#include <ae_param_flow.h>
#endif
#include "flash_platform_adapter.h"

/* Face headers */
#include <faces.h>

/***********************************************************
 * Define macros
 **********************************************************/

#define PROP_FLASH_AAO_RATIO  "vendor.flash_aao_ratio"

/***********************************************************
 * Sort
 **********************************************************/
template <class T>
void flash_sortxy_xinc(int n, T *x, T *y)
{
    int i;
    int j;
    for (i = 0; i < n; i++)
        for (j = i + 1; j < n; j++) {
            if (x[i] > x[j]) {
                T tmp;
                tmp = x[i];
                x[i] = x[j];
                x[j] = tmp;
                tmp = y[i];
                y[i] = y[j];
                y[j] = tmp;
            }
        }
}
template void flash_sortxy_xinc<int>(int, int *, int *);


/***********************************************************
 * BMP export
 **********************************************************/
template <class T>
int arrayToBmp(const char *fname, T *r, T *g, T *b, int w, int h, double maxVal)
{
    char header[54];
    int *pInt;
    short *pShort;

    /* verify arguments */
    if (w <= 0 || h <= 0) {
        logE("arrayToBmp(): error width/height(%d/%d).", w, h);
        return -1;
    }

    /* BMP file header */
    /* type (magic number) */
    header[0] = 'B';
    header[1] = 'M';
    /* size */
    pInt = (int *)(header + 2);
    *pInt = 3 * w * h + 54;
    /* reserved 1 and reserved 2 */
    pInt = (int *)(header + 6);
    *pInt = 0;
    /* offset */
    pInt = (int *)(header + 0xA);
    *pInt = 54;

    /* BMP info header */
    /* size */
    pInt = (int *)(header + 0xE);
    *pInt = 40;
    /* width */
    pInt = (int *)(header + 0x12);
    *pInt = w;
    /* height */
    pInt = (int *)(header + 0x16);
    *pInt = h;
    /* planes */
    pShort = (short *)(header + 0x1A);
    *pShort = 1;
    /* bits */
    pShort= (short *)(header + 0x1C);
    *pShort = 24;
    /* compression */
    pInt = (int *)(header + 0x1E);
    *pInt = 0;
    /* image size */
    pInt = (int *)(header + 0x22);
    *pInt = 0;
    /* x resolution */
    pInt = (int *)(header + 0x26);
    *pInt = 2834; // 72 dpi
    /* y resolution */
    pInt = (int *)(header + 0x2A);
    *pInt = 2834; // 72 dpi
    /* number of colors */
    pInt = (int *)(header + 0x2E);
    *pInt = 0;
    /* important colors */
    pInt = (int *)(header + 0x32);
    *pInt = 0;

    /* raw data */
    int i, j;
    int pos, index = 0;
    int lineBytes;
    unsigned char *data;
    unsigned char *pixeldata;

    lineBytes = ((w * 3 + 3) / 4) * 4;
    data = new unsigned char[lineBytes * h];
    if (!data)
        return -1;

    for (j = 0; j < h; j++) {
        pixeldata = data + (h - 1 - j) * lineBytes;
        pos = 0;
        for (i = 0; i < w; i++) {
            pixeldata[pos] = (unsigned char)(b[index] * 255 / maxVal);
            pos++;
            pixeldata[pos] = (unsigned char)(g[index] * 255 / maxVal);
            pos++;
            pixeldata[pos] = (unsigned char)(r[index] * 255 / maxVal);
            pos++;
            index++;
        }
    }

    /* export to file */
    FILE *fp;
    fp = fopen(fname, "wb");
    if (!fp) {
        delete [] data;
        return -1;
    }
    fwrite(header, 1, 54, fp);
    fwrite(data, 1, lineBytes * h, fp);
    fclose(fp);

    delete [] data;

    return 0;
}

template <class T>
int arrayToBmp(const char *fname, T *r, T *g, T *b, int w, int h, int *convertedFacePos, double maxVal)
{
    char header[54];
    int *pInt;
    short *pShort;

    /* BMP file header */
    /* type (magic number) */
    header[0] = 'B';
    header[1] = 'M';
    /* size */
    pInt = (int *)(header + 2);
    *pInt = 3 * w * h + 54;
    /* reserved 1 and reserved 2 */
    pInt = (int *)(header + 6);
    *pInt = 0;
    /* offset */
    pInt = (int *)(header + 0xA);
    *pInt = 54;

    /* BMP info header */
    /* size */
    pInt = (int *)(header + 0xE);
    *pInt = 40;
    /* width */
    pInt = (int *)(header + 0x12);
    *pInt = w;
    /* height */
    pInt = (int *)(header + 0x16);
    *pInt = h;
    /* planes */
    pShort = (short *)(header + 0x1A);
    *pShort = 1;
    /* bits */
    pShort= (short *)(header + 0x1C);
    *pShort = 24;
    /* compression */
    pInt = (int *)(header + 0x1E);
    *pInt = 0;
    /* image size */
    pInt = (int *)(header + 0x22);
    *pInt = 0;
    /* x resolution */
    pInt = (int *)(header + 0x26);
    *pInt = 2834; // 72 dpi
    /* y resolution */
    pInt = (int *)(header + 0x2A);
    *pInt = 2834; // 72 dpi
    /* number of colors */
    pInt = (int *)(header + 0x2E);
    *pInt = 0;
    /* important colors */
    pInt = (int *)(header + 0x32);
    *pInt = 0;

    /* raw data */
    int i, j, k;
    int pos, index = 0;
    int lineBytes;
    unsigned char *data;
    unsigned char *pixeldata;

    lineBytes = ((w * 3 + 3) / 4) * 4;
    data = new unsigned char[lineBytes * h];
    if (!data)
        return -1;

    for (j = 0; j < h; j++)
    {
        pixeldata = data + (h - 1 - j) * lineBytes;
        pos = 0;
        for (i = 0; i < w; i++)
        {
            pixeldata[pos] = (unsigned char)(b[index] * 255 / maxVal);
            pos++;
            pixeldata[pos] = (unsigned char)(g[index] * 255 / maxVal);
            pos++;
            pixeldata[pos] = (unsigned char)(r[index] * 255 / maxVal);
            pos++;
            index++;
        }
    }

    /*Convert face region to white block*/
    for (k = 0; k < convertedFacePos[0]; k++)
    {
        for (j = 0; j < h; j++)
        {
            for (i = 0; i < w; i++)
            {
                if ( i >= convertedFacePos[4 * k + 1] && i <= convertedFacePos[4 * k + 3] &&
                        j <= (h - convertedFacePos[4 * k + 2]) && j >= (h - convertedFacePos[4 * k + 4]))
                {
                    data[j * lineBytes + 3 * i]     = 255;
                    data[j * lineBytes + 3 * i + 1] = 255;
                    data[j * lineBytes + 3 * i + 2] = 255;
                }
            }
        }
    }

    /* export to file */
    FILE *fp;
    fp = fopen(fname, "wb");
    if (!fp)
    {
        delete [] data;
        return -1;
    }
    fwrite(header, 1, 54, fp);
    fwrite(data, 1, lineBytes * h, fp);
    fclose(fp);

    delete [] data;

    return 0;
}

/***********************************************************
 * Window operations
 **********************************************************/
int resizeLine(int l, double rZoom, int *lStart, int *lResize, int *lBin, int lResizeMin, int lResizeMax)
{
    /* verify arguments */
    if (!lStart || !lResize || !lBin) {
        logE("resizeLine(): error arguments.");
        return -1;
    }
    if (lResizeMax < lResizeMin) {
        logE("resizeLine(): error crop size.");
        return -1;
    }

    /* init crop size and bin */
    float lTar = l / rZoom;
    int lResizeRet = ((int)lTar) / 2 * 2;
    int lBinRet = 1;
    lResizeMin = (lResizeMin + 1) & 0xffe;
    lResizeMax = lResizeMax & 0xffe;

    /* get crop size and bin */
    if ((int)lTar > lResizeMax) {
        int i;
        int lResizeNew;
        float err;
        float minErr = 10000;
        for (i = 0; i <= lTar / 2; i++) {
            lResizeNew = lResizeMax - 2 * i;

            /* break if down to crop minimum */
            if (lResizeNew < lResizeMin)
                break;

            /* if target lengtn is multiple with crop size */
            err = lTar - (lTar / lResizeNew) * lResizeNew;
            if (!err) {
                lResizeRet = lResizeNew;
                lBinRet = lTar / lResizeNew;
                break;
            }

            /* get the less error part */
            if (err < minErr) {
                minErr = err;
                lResizeRet = lResizeNew;
                lBinRet = lTar / lResizeNew;
            }
        }
    }
    *lResize = lResizeRet;
    *lBin = lBinRet;
    *lStart = (l - lResizeRet * lBinRet) / 2;

    return 0;
}

int resizeWindow(double rZoom, short *data, int w, int h,
        int wResizeNoZoom, int hResizeNoZoom, short *rzData, int *rzW, int *rzH)
{
    /* verify arguments */
    if (!data || !rzData || !rzW || !rzH) {
        logE("resizeWindow(): error arguments.");
        return -1;
    }
    if (w < wResizeNoZoom || h < hResizeNoZoom) {
        logE("resizeWindow(): error crop size.");
        return -1;
    }

    /* get resize data */
    int wResize = wResizeNoZoom;
    int hResize = hResizeNoZoom;
    int wBin = w / wResize;
    int hBin = h / hResize;
    int wStart = (w - wBin * wResize) / 2;
    int hStart = (h - hBin * hResize) / 2;

    if (rZoom >= 1.05) {
        resizeLine(w, rZoom, &wStart, &wResize, &wBin, 20, wResizeNoZoom);
        resizeLine(h, rZoom, &hStart, &hResize, &hBin, 15, hResizeNoZoom);
        double wErrRate;
        double hErrRate;
        wErrRate = std::abs((double)(w / rZoom - wResize * wBin) / (w / rZoom));
        hErrRate = std::abs((double)(h / rZoom - hResize * hBin) / (h / rZoom));
        if (wErrRate > 0.1) {
            resizeLine(w, rZoom, &wStart, &wResize, &wBin, 10, wResizeNoZoom);
        }
        if (hErrRate > 0.1) {
            resizeLine(h, rZoom, &hStart, &hResize, &hBin, 10, hResizeNoZoom);
        }
    }

    /* clear resize data */
    memset(rzData, 0, wResize * hResize * sizeof(short));

    /* get resize data with average quad-binning */
    int i;
    int j;
    int iRz;
    int jRz;
    for (j = hStart; j < hStart + hBin * hResize; j++)
        for (i = wStart; i < wStart + wBin * wResize; i++) {
            iRz = (i - wStart) / wBin;
            jRz = (j - hStart) / hBin;
            rzData[iRz + wResize * jRz] += data[i + j * w];
        }
    for (i = 0; i < wResize * hResize; i++)
        rzData[i] = rzData[i] / (wBin * hBin);
    *rzW = wResize;
    *rzH = hResize;

    return 0;
}


/***********************************************************
 * AA statistic
 **********************************************************/
int convertAaSttToYrgbBmp(void *buf, int w, int h,
        NS3Av3::EBitMode_T mode, const char *yFile, const char *rgbFile, int linearOutputEn)
{
    /* verify arguments */
    if (w <= 0 || h <= 0) {
        logE("convertAaSttToYrgbBmp(): error width/height(%d/%d).", w, h);
        return -1;
    }

    short *y = new short[w * h]();
    short *r = new short[w * h]();
    short *g = new short[w * h]();
    short *b = new short[w * h]();
    convertAaSttToYrgb(buf, w, h, y, r, g, b, mode, linearOutputEn);
    arrayToBmp(yFile, y, y, y, w, h);
    arrayToBmp(rgbFile, r, g, b, w, h);
    delete[] y;
    delete[] r;
    delete[] g;
    delete[] b;

    return 0;
}

int convertAaSttToYBmp(void *buf, int w, int h,
                       NS3Av3::EBitMode_T mode, const char *yFacePosFile, int *convertedFacePos)
{
    short *y;
    y = new short[w * h];

    convertAaSttToY(buf, w, h, y, 1, mode);
    arrayToBmp(yFacePosFile, y, y, y, w, h, convertedFacePos);

    delete[] y;
    return 0;
}

void get_1_4_range(int v, int &down, int &up)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(PROP_FLASH_AAO_RATIO, value, "4.0");
    double aaoRatio = atof(value);

    int v_even;
    double v_1_4; // v/4
    int nv_1_4_2; // half of v/4

    v_even = v;
    if (v % 2 == 1)
        v_even--;
    v_1_4 = v_even / aaoRatio;
    nv_1_4_2 = (int)(v_1_4 / 2.0 + 0.5);
    down = v_even / 2 - nv_1_4_2;
    up = v_even / 2 + (nv_1_4_2 - 1);
    if (v % 2 == 1)
        up++;
}

template <class T>
double cal_1_4_mean(T *v, int w, int h)
{
    int i, j;
    int wst, wed;
    int hst, hed;
    int index;
    int count = 0;
    double sum = 0;

    get_1_4_range(w, wst, wed);
    get_1_4_range(h, hst, hed);
    for (i = wst; i <= wed; i++)
    {
        for (j = hst; j <= hed; j++)
        {
            index = j * w + i;
            if(v[index] > 0)
            {
                sum += v[index];
                count++;
            }
        }
    }

    if (count > 0)
        sum /= count;

    return sum;
}

int cal_1_4_yrgb_mean(void *buf, int w, int h, double *yrgb_mean, int linearOutputEn)
{
    /* verify arguments */
    if (w <= 0 || h <= 0) {
        logE("cal_1_4_yrgb_mean(): error width/height(%d/%d).", w, h);
        return -1;
    }

    double *y = new double[w * h]();
    double *r = new double[w * h]();
    double *g = new double[w * h]();
    double *b = new double[w * h]();
    convertAaSttToYrgb(buf, w, h, y, r, g, b, NS3Av3::EBitMode_12Bit, linearOutputEn);
    /* Warning: yrgb size must >= 4 */
    yrgb_mean[0] = cal_1_4_mean(y, w, h);
    yrgb_mean[1] = cal_1_4_mean(r, w, h);
    yrgb_mean[2] = cal_1_4_mean(g, w, h);
    yrgb_mean[3] = cal_1_4_mean(b, w, h);
    delete[] y;
    delete[] r;
    delete[] g;
    delete[] b;
    return 0;
}


