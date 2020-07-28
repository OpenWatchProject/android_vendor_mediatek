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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/**
 * @file PrivilegeManager.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c PrivilegeManager class providing privilege related functions to check session permissions.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */
#ifndef __PRIVILEGE_MANAGER_H__
#define __PRIVILEGE_MANAGER_H__
#include <map>
#include <time.h>
#include <string>
#include <vector>
#include "libmdmonitor.h"
namespace libmdmonitor {

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class provides privilege related functions to check session permissions.
 */
struct cmp_str
{
    bool operator() (const char *left, const char *right) const
    {
        return std::strcmp(left, right) < 0;
    }
};

class PrivilegeManager
{
public:
    /**
     * @brief Convert the @a secret string to privilege level.
     *
     * @param secret [IN] The secret string.
     *
     * @return Privilege level.
     */
    PRIVILEGE_LEVEL ToPrivilegeLevel(const char *secret);

    /**
     * @brief Check whether the given privilege level is a managing user.
     *
     * @param privLevel [IN] The input privilege level.
     *
     * @return true: The given privilege level is a managing user.<br>
     *        false: Otherwise.
     */
    bool IsManagingUser(PRIVILEGE_LEVEL privLevel);

    /**
     * @brief Given last access timestamp, check whether the session should be expired. 
     *
     * @param lastAccessTime [IN] The UNIX timestamp of last access time.
     *
     * @return true: The session should be expired.<br>
     *        false: Otherwise.
     */
    bool IsExpired(time_t lastAccessTime);

    /**
     * @brief Check if the client is authorized to access MDM framework.
     *        Now is only support to check if APK client can access MDML.
     *
     * @param client [IN] Information of MDM framework client, e.g. package name of APK client
     */
    bool isAuthorized(const char *client);

    /**
     * @brief Create a empty privilege manager.
     */
    PrivilegeManager();

    /**
     * @brief Cleanup.
     */
    virtual ~PrivilegeManager();

    struct AuthPlatform
    {
        const char* szClient;
        const char* szChip;
        const char* szAndroidVer;
    };
private:
    std::map<const char*, std::vector<AuthPlatform>*, cmp_str> mAuthPlatforms;
};
} //namespace libmdmonitor {

#endif
