#ifndef __AINRDEFS_H__
#define __AINRDEFS_H__

/**
 *  The definition that maximum frames that AINR supports
 */
#define AINR_MAX_FRAMES                                 10

/**
 *  If invoke logs
 */
#define AINR_LOG                                        1

/**
 *  If using ALOGV rather than log mechanism from MTK
 */
#define AINR_USING_ALOG                                 0

/**
 *  If display all AIS frames during taking picture
 */
#define AINR_DISPLAY_ALL_AIS_FRAMES                     0

/* Avoid unused parameter warning */
#define AINR_UNUSED(x) (void)(x)

/* Pass2 port */
#define AINR_PORTID_IN 0
#define AINR_PORTID_OUT 1

#define ALIGN(w, a) (((w + (a-1)) / a) * a)


#endif /* __AINRDEFS_H__ */
