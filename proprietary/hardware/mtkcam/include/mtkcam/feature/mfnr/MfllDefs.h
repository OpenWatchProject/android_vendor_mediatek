#ifndef __MFLLDEFS_H__
#define __MFLLDEFS_H__

/**
 *  The definition that maximum frames that MFLL supports
 */
#define MFLL_MAX_FRAMES                                 10

/**
 *  If invoke logs
 */
#define MFLL_LOG                                        1

/**
 *  If using ALOGV rather than log mechanism from MTK
 */
#define MFLL_USING_ALOG                                 0

/**
 *  If display all AIS frames during taking picture
 */
#define MFLL_DISPLAY_ALL_AIS_FRAMES                     0

/* Avoid unused parameter warning */
#define MFLL_UNUSED(x) (void)(x)

#endif /* __MFLLDEFS_H__ */
