/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSINGUTILS_H_
#define _MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSINGUTILS_H_
//
#include <cutils/properties.h>
//
#include <mtkcam/Log.h>
//
//
#ifdef MY_LOGV
#undef MY_LOGV
#endif
#ifdef MY_LOGD
#undef MY_LOGD
#endif
#ifdef MY_LOGI
#undef MY_LOGI
#endif
#ifdef MY_LOGW
#undef MY_LOGW
#endif
#ifdef MY_LOGE
#undef MY_LOGE
#endif
#ifdef MY_LOGA
#undef MY_LOGA
#endif
#ifdef MY_LOGF
#undef MY_LOGF
#endif
//
#ifdef MY_LOGV_IF
#undef MY_LOGV_IF
#endif
#ifdef MY_LOGD_IF
#undef MY_LOGD_IF
#endif
#ifdef MY_LOGI_IF
#undef MY_LOGI_IF
#endif
#ifdef MY_LOGW_IF
#undef MY_LOGW_IF
#endif
#ifdef MY_LOGE_IF
#undef MY_LOGE_IF
#endif
#ifdef MY_LOGA_IF
#undef MY_LOGA_IF
#endif
#ifdef MY_LOGF_IF
#undef MY_LOGF_IF
#endif
//
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s][x%X] " fmt, \
                                    __FUNCTION__, mOpenID, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s][x%X] " fmt, \
                                    __FUNCTION__, mOpenID, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s][x%X] " fmt, \
                                    __FUNCTION__, mOpenID, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s][x%X] " fmt, \
                                    __FUNCTION__, mOpenID, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s][x%X] " fmt, \
                                    __FUNCTION__, mOpenID, ##arg)
#define MY_LOGA(fmt, arg...)    CAM_LOGA("[%s][x%X] " fmt, \
                                    __FUNCTION__, mOpenID, ##arg)
#define MY_LOGF(fmt, arg...)    CAM_LOGF("[%s][x%X] " fmt, \
                                    __FUNCTION__, mOpenID, ##arg)
//
#define MY_LOGV_IF(cond, ...)   do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); }\
                                }while(0)
#define MY_LOGD_IF(cond, ...)   do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); }\
                                }while(0)
#define MY_LOGI_IF(cond, ...)   do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); }\
                                }while(0)
#define MY_LOGW_IF(cond, ...)   do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); }\
                                }while(0)
#define MY_LOGE_IF(cond, ...)   do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); }\
                                }while(0)
#define MY_LOGA_IF(cond, ...)   do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); }\
                                }while(0)
#define MY_LOGF_IF(cond, ...)   do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); }\
                                }while(0)
//
#ifdef FUNC_BGN
#undef FUNC_BGN
#endif
#define FUNC_BGN do { if (mLogLevel > 0) { CAM_LOGI("[%s][x%X] +",\
                    __FUNCTION__, mOpenID); } } while(0)
//
#ifdef FUNC_END
#undef FUNC_END
#endif
#define FUNC_END do { if (mLogLevel > 0) { CAM_LOGI("[%s][x%X] -",\
                    __FUNCTION__, mOpenID); } } while(0)
//

#endif//_MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSINGUTILS_H_

