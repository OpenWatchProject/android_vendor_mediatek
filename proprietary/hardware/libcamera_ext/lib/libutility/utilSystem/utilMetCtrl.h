#ifndef _UTIL_MET_CTRL_H_
#define _UTIL_MET_CTRL_H_

#include "MTKUtilCommon.h"
/*
#ifdef _CFDK_ANALYSIS_
*/

UTIL_ERRCODE_ENUM UtlMetTagInit(void);

void UtlMetTagUninit(void);

void UtlMetTagStart(unsigned int class_id, const char *name);

void UtlMetTagEnd(unsigned int class_id, const char *name);

void UtlMetTagOneShot(unsigned int class_id, const char *name, unsigned int value);
/*
#else

#define UtlMetTagInit()

#define UtlMetTagStart(class_id, name)

#define UtlMetTagEnd(class_id, name)

#endif
*/
#endif /* _UTIL_MET_CTRL_H_ */