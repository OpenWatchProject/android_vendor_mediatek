#pragma once

#include <stdlib.h>
#include <cutils/properties.h>

/* camera headers */
#include "mtkcam/aaa/aaa_hal_common.h"
#include "flash_param.h"
#include "log_utils.h"

/***********************************************************
 * Log
 **********************************************************/
/* android property */
#define PROP_FLASH_IS_DEBUG "vendor.flash_is_debug"

static inline void setDebug()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(PROP_FLASH_IS_DEBUG, value, "0");
    isDebug = atoi(value);
}


/***********************************************************
 * Sort
 **********************************************************/
template <class T>
void flash_sortxy_xinc(int n, T* x, T* y);


/***********************************************************
 * BMP export
 **********************************************************/
template <class T>
int arrayToBmp(const char *fname, T *r, T *g, T *b, int w, int h, double maxVal = 255);
template <class T>
int arrayToBmp(const char *fname, T *r, T *g, T *b, int w, int h, int *convertedFacePos, double maxVal = 255);



/***********************************************************
 * Window operations
 **********************************************************/
int resizeLine(int l, double rZoom, int *lStart, int *lResize,
        int *lBin, int lResizeMin, int lResizeMax);

int resizeWindow(double rZoom, short *data, int w, int h,
        int wResizeNoZoom, int hResizeNoZoom, short *rzData, int *rzW, int *rzH);


/***********************************************************
 * AA statistic
 **********************************************************/
int convertAaSttToYrgbBmp(void *buf, int w, int h,
        NS3Av3::EBitMode_T mode, const char *yFile, const char *rgbFile, int linearOutputEn = 0);

int convertAaSttToYBmp(void *buf, int w, int h, NS3Av3::EBitMode_T mode, const char *yFacePosFile, int *convertedFacePos);

void get_1_4_range(int v, int &down, int &up);

template <class T>
double cal_1_4_mean(T *v, int w, int h);

int cal_1_4_yrgb_mean(void *buf, int w, int h, double *yrgb_mean, int linearOutputEn = 0);

