/*
 * utils.h
 *
 *  Created on: July 11, 2018
 *      Author: mtk11515
 */

#ifndef UTILS_H_
#define UTILS_H_

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "loghidlvendorservice"
/**
 * These log definition was used for Android version later than JB
 */
#ifndef LOGD
#define LOGD ALOGD
#endif
#ifndef LOGI
#define LOGI ALOGI
#endif
#ifndef LOGE
#define LOGE ALOGE
#endif

#endif /* UTILS_H_ */
