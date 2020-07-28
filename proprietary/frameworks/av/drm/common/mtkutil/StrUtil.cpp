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

#define LOG_NDEBUG 0
#define LOG_TAG "DrmMtkUtil/StrUtil"
#include <utils/Log.h>

#include <StrUtil.h>
#include <DrmDef.h>
#include <utils/String8.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

using namespace android;

// ==== local constants defination =============================================

// static const int HEX_STR_MAX_SIZE = 1024;
static const int HEX_STR_MAX_OUTPUT_SIZE = 2049;

#define MAX_DATETIME_STR 10

// ==== local constants defination end =========================================

const char* StrUtil::toString(long l)
{
    static char loc_buf[20]; /* not thread safe */
    bzero(loc_buf, sizeof(loc_buf));
    if (snprintf(loc_buf, sizeof(loc_buf), "%ld", l) == -1)
    {
        ALOGE("toString(long l): fail");
        return NULL; /* or whatever */
    }
    return loc_buf;
}

const char* StrUtil::toString(int i)
{
    static char loc_buf[20]; /* not thread safe */
    bzero(loc_buf, sizeof(loc_buf));
    if (snprintf(loc_buf, sizeof(loc_buf), "%d", i) == -1)
    {
        ALOGE("toString(int i): fail");
        return NULL; /* or whatever */
    }
    return loc_buf;
}

void StrUtil::toLower(char* p)
{
    if (NULL == p)
    {
        return;
    }
    int i = 0;
    while ('\0' != p[i])
    {
        p[i] = tolower(p[i]);
        i++;
    }
}

String8 StrUtil::toHexStr(String8 str)
{
    return toHexStr((char*)str.string(), str.length());
}

// {num} is limited to HEX_STR_MAX_SIZE bytes
String8 StrUtil::toHexStr(char* ptr, int num)
{
    char ch[HEX_STR_MAX_OUTPUT_SIZE];
    bzero(ch, sizeof(ch));

    for (int i = 0; i < num; i++)
    {
        sprintf(&ch[i * 2], "%02X", ptr[i]);
    }

    String8 str(ch);
    return str;
}

// returns an empty string or the valid time string in "%Y-%m-%d_%H%M%S" format
String8 StrUtil::toTimeStr(time_t* t)
{
    if (*t == 0)
    {
        return String8("");
    }

    struct tm* gm = localtime(t);
    char p[30];
    bzero(p, sizeof(p));
    strftime(p, sizeof(p), "%Y-%m-%d_%H%M%S", gm);
    return String8(p);
}

// the interval string is in "P2Y10M15DT10H30M20S" format
// the reduced format, according to 3.2.6 of [XMLSchema] (ISO8601) should also be supported
bool StrUtil::intervalStrToTick(char* ptr, time_t& tick)
{
    size_t len = strlen(ptr);
    char* p = new char[len + 1];
    bzero(p, len + 1);
    memcpy(p, ptr, len);

    char* pP = strrchr(p, 'P'); // 'P' tag for date string must exist
    char* pT = strrchr(p, 'T'); // 'T' tag for time string

    int year = 0;  // by default these are all 0
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    if (NULL == pP) // the 'P' character should be present
    {
        ALOGE("intervalStrToTick : invalid interval string format, tag 'P' not found");
        delete[] p;
        return false;
    }

    char* pDateStr = NULL;
    char* pTimeStr = NULL;

    if (NULL == pT) // if there's no 'T' tag then there's no time string
    {
        size_t len_p = strlen(p);
        pDateStr = new char[len_p + 1]; // the whole string is date string, pTimeStr remains NULL
        bzero(pDateStr, len_p + 1);
        memcpy(pDateStr, p, len_p);
        ALOGD("intervalStrToTick : there's no 'T' tag and no time info present");
    }
    else // 'T' tag and 'P' tag both exist
    {
        if ((pT - pP - 1) <= 0)
        {
            // there's no date info. pDateStr remains NULL
            ALOGD("intervalStrToTick : there's no date info present");
        }
        else
        {
            size_t len_date = (size_t)(pT - pP);
            pDateStr = new char[len_date + 1]; // the date string is available
            bzero(pDateStr, len_date + 1);
            memcpy(pDateStr, pP, len_date);
        }

        size_t len_time = strlen(p) - (size_t)(pT - pP);
        pTimeStr = new char[len_time + 1]; // the time string is available
        bzero(pTimeStr, len_time + 1);
        memcpy(pTimeStr, pT, len_time);
    }
    delete[] p;
    p = NULL;   // release original p string

    char temp[11];
    char* ps = NULL; // the start point
    char* pe = NULL; // the end point

    if (NULL != pDateStr) // now deal with date string
    {
        ALOGV("intervalStrToTick : the Date string [%s]", pDateStr);
        ps = pDateStr;
        char* pYr = strrchr(pDateStr, 'Y');
        if (NULL != pYr) // year is kept as 0 if no 'Y' tag found
        {
            pe = pYr;
            bzero(temp, sizeof(temp));
            if ((pe-ps) > MAX_DATETIME_STR)
            {
                ALOGE("intervalStrToTick: the year string is too long.");
                goto ERROR;
            }
            memcpy(temp, ps+1, pe - ps);
            // M: @{
            // ALPS00540302 make sure the year string is valid
            if (!validateDatetimeStr(temp, pe - ps -1))
            {
                ALOGE("intervalStrToTick: the date [%s] include invalid year string[%s].",pDateStr,temp);
                goto ERROR;
            }
            // M: @}
            year = atoi(temp);
            ps = pYr; // move the start point after Year
        }

        char* pMon = strrchr(pDateStr, 'M');
        if (NULL != pMon)
        {
            pe = pMon;
            bzero(temp, sizeof(temp));
            if ((pe-ps) > MAX_DATETIME_STR)
            {
                ALOGE("intervalStrToTick: the month string is too long.");
                goto ERROR;
            }
            memcpy(temp, ps+1, pe - ps);
            // M: @{
            // ALPS00540302 make sure the month string is valid
            if (!validateDatetimeStr(temp, pe - ps -1))
            {
                ALOGE("intervalStrToTick: the date[%s] include invalid month string[%s].",pDateStr,temp);
                goto ERROR;
            }
            // M: @}
            month = atoi(temp);
            ps = pMon; // move the start point after Month
        }

        char* pDay = strrchr(pDateStr, 'D');
        if (NULL != pDay)
        {
            pe = pDay;
            bzero(temp, sizeof(temp));
            if ((pe-ps) > MAX_DATETIME_STR)
            {
                ALOGE("intervalStrToTick: the day string is too long.");
                goto ERROR;
            }
            memcpy(temp, ps+1, pe - ps);
            // M: @{
            // ALPS00540302 make sure the day string is valid
            if (!validateDatetimeStr(temp, pe - ps -1))
            {
                ALOGE("intervalStrToTick: the Date String include invalid day string[%s].",temp);
                goto ERROR;
            }
            // M: @}
            day = atoi(temp);
            ps = pDay; // move the start point after Day
        }

        delete[] pDateStr;
        pDateStr = NULL;
    }

    if (NULL != pTimeStr) // now deal with time string
    {
        ALOGV("intervalStrToTick : the Time string [%s]", pTimeStr);
        ps = pTimeStr;
        char* pHr = strrchr(pTimeStr, 'H');
        if (NULL != pHr)
        {
            pe = pHr;
            bzero(temp, sizeof(temp));
            if ((pe-ps) > MAX_DATETIME_STR)
            {
                ALOGE("intervalStrToTick: the hour string is too long.");
                goto ERROR;
            }
            memcpy(temp, ps+1, pe - ps);
            // M: @{
            // ALPS00540302 make sure the hour string is valid
            if (!validateDatetimeStr(temp, pe - ps -1))
            {
                ALOGE("intervalStrToTick: the time[%s] include invalid hour string[%s].",pTimeStr,temp);
                goto ERROR;
            }
            // M: @}
            hour = atoi(temp);
            ps = pHr; // move the start point after Hour
        }

        char* pMin = strrchr(pTimeStr, 'M');
        if (NULL != pMin)
        {
            pe = pMin;
            bzero(temp, sizeof(temp));
            if ((pe-ps) > MAX_DATETIME_STR)
            {
                ALOGE("intervalStrToTick: the minute string is too long.");
                goto ERROR;
            }
            memcpy(temp, ps+1, pe - ps);
            // M: @{
            // ALPS00540302 make sure the minute string is valid
            if (!validateDatetimeStr(temp, pe - ps -1))
            {
                ALOGE("intervalStrToTick: the time[%s] include invalid minute string[%s].",pTimeStr,temp);
                goto ERROR;
            }
            // M: @}
            minute = atoi(temp);
            ps = pMin; // move the start point after Minute
        }

        char* pSec = strrchr(pTimeStr, 'S');
        if (NULL != pSec)
        {
            pe = pSec;
            bzero(temp, sizeof(temp));
            if ((pe-ps) > MAX_DATETIME_STR)
            {
                ALOGE("intervalStrToTick: the second string is too long.");
                goto ERROR;
            }
            memcpy(temp, ps+1, pe - ps);
            // M: @{
            // ALPS00540302 make sure the second string is valid
            if (!validateDatetimeStr(temp, pe - ps -1))
            {
                ALOGE("intervalStrToTick: the time[%s] include invalid second string[%s].",pTimeStr,temp);
                goto ERROR;
            }
            // M: @}
            second = atoi(temp);
            ps = pSec; // move the start point after Second
        }

        delete[] pTimeStr;
        pTimeStr = NULL;
    }

    ALOGV("intervalStrToTick : year:[%d], month:[%d], day:[%d], hour:[%d], min:[%d], sec:[%d]",
            year, month, day, hour, minute, second);
    tick = year * 365 * DrmDef::DT_SEC_PER_DAY
           + month * 30 * DrmDef::DT_SEC_PER_DAY
           + day * DrmDef::DT_SEC_PER_DAY
           + hour * DrmDef::DT_SEC_PER_HOUR
           + minute * DrmDef::DT_SEC_PER_MIN
           + second;
    ALOGV("intervalStrToTick : tick: [%ld]", tick);
    return true;
// M: @{
// ALPS00540302 return false if interval date-time string is invalid
ERROR:
    ALOGE("intervalStrToTick : error, interval string is invalid.");
    delete[] pDateStr;
    delete[] pTimeStr;
    pDateStr = NULL;
    pTimeStr = NULL;
    return false;
// M: @}
}

// the date-time string is in "2007-10-18T00:00:00" format
// the TimeZone is already considered in mktime()!
// used while parsing dr, drc files
bool StrUtil::datetimeStrToTick(char* ptr, time_t& tick)
{
    size_t len = strlen(ptr);

    if (len == 0)
    {
        ALOGV("datetimeStrToTick : DT string is empty string");
        return true;
    }

    char* p = new char[len + 1];
    bzero(p, len + 1);
    memcpy(p, ptr, len);
    // check the datetime string format as "2013-03-04T09:08:20" valid
    // but "2013;03-04U09:08.20" invalid
    if (strlen(p) != 19 || (p[4] != '-' || p[7] != '-' || p[10] != 'T' || p[13] != ':' || p[16] != ':')
        || (!validateDatetimeStr(p,4) || !validateDatetimeStr(p + 5,2) ||
            !validateDatetimeStr(p + 8,2) || !validateDatetimeStr(p + 11,2) ||
            !validateDatetimeStr(p + 14,2) || !validateDatetimeStr(p + 17,2)))
    {
        ALOGE("datetimeStrToTick : DT string: [%s] is not an valid format", p);
        delete[] p;
        return false;
    }

    struct tm time;
    bzero(&time, sizeof(time));

    time.tm_sec = atoi(&p[17]);
    p[16] = '\0';
    time.tm_min = atoi(&p[14]);
    p[13] = '\0';
    time.tm_hour = atoi(&p[11]);
    p[10] = '\0';
    time.tm_mday = atoi(&p[8]);
    p[7] = '\0';
    time.tm_mon = atoi(&p[5]) - 1;
    p[4] = '\0';
    time.tm_year = atoi(&p[0]) - 1900;

    if (!validateDatetime(time))
    {
        ALOGE("datetimeStrToTick : validateDatatime failed");
        delete[] p;
        return false;
    }

    tick = mktime(&time);
    ALOGV("datetimeStrToTick : tick: [%ld]", tick);
    if (tick == -1)
    {
        ALOGE("datetimeStrToTick : fail");
        delete[] p;
        return false;
    }

    delete[] p;
    return true;
}

// remove those space characters at the beginning /
String8& StrUtil::trimLRSpace(String8 &str)
{
    int len = str.length();
    int start = 0;
    int end = 0;
    for (int i = 0; i < len - 1; i++)
    {
        if (str[i] != ' ')  // space
        {
            start = i;
            break;
        }
    }
    for (int i = len - 1; i >= 0; i--)
    {
        if (str[i] != ' ')
        {
            end = i;
            break;
        }
    }
    if (str.setTo(str.string() + start, end - start + 1) != NO_ERROR)
    {
        ALOGE("trimLRSpace: fail");
    }
    return str;
}

// in string s, replace s1 with s2 (if found s1 in s) and returns the modifed s
char* StrUtil::strrpl(char* s, const char* s1, const char* s2)
{
    char* ptr = strstr(s, s1);
    if (ptr != NULL) // found s1 in s
    {
        memmove(ptr + strlen(s2), ptr + strlen(s1), strlen(ptr) - strlen(s1) + 1);
        memcpy(ptr, s2, strlen(s2));
    }
    return s;
}

// the input string is modified to remove the CR, LF, and is also returned as reference
String8& StrUtil::trimRCRLF(String8 &str)
{
    int end = str.length();
    for (int i = end - 1; i >= 0; i--)
    {
        if (str[i] != 0x0a && str[i] != 0x0d && str[i] != 0x00) // not NULL, \n, \r
        {
            if (str.setTo(str.string(), i + 1) != NO_ERROR)
            {
                ALOGE("trimRCRLF: fail");
            }
            return str;
        }
    }

    str.setTo(""); // otherwise case: there's no valid char other than \n, \r, NULL
    return str;    // then it is modified to an empty string
}

// returns an empty string or the valid Content Uri string (cid)
// "cid:123456789" -> "1234567889", removes "cid:"
String8 StrUtil::getContentUri(String8 cid)
{
    int index = (int)cid.find(":");
    if (index == -1)
    {
        ALOGE("getContentUri: failed to find ':' separator.");
        return String8("");
    }
    return String8(cid.string() + index + 1);
}

// Validate the date-time, such as "2005-03-25T19:29:60" is invalid.
// As second and minute can't be bigger than 59.
bool StrUtil::validateDatetime(const struct tm& time) {
    ALOGV("validate date time >>>>");

    if (time.tm_sec < 0 || time.tm_sec >= 60)
    {
        ALOGE("validateDatetime: date time sec[%d] invalid",time.tm_sec);
        return false;
    }

    if (time.tm_min < 0 || time.tm_min >= 60)
    {
        ALOGE("validateDatetime: date time min[%d] invalid",time.tm_min);
        return false;
    }

    if (time.tm_hour < 0 || time.tm_hour >= 24)
    {
        ALOGE("validateDatetime: date time hour[%d] invalid",time.tm_hour);
        return false;
    }

    if (time.tm_mday < 1 || time.tm_mday > 31)
    {
        ALOGE("validateDatetime: date time day[%d] invalid",time.tm_mday);
        return false;
    }

    if (time.tm_mon < 0 || time.tm_mon > 11)
    {
        ALOGE("validateDatetime: date time mon[%d] invalid",time.tm_mon + 1);
        return false;
    }

    if (time.tm_year < 0)
    {
        ALOGE("validateDatetime: date time mon[%d] invalid",time.tm_year);
        return false;
    }
    return true;
}

// Validate the rights interval date-time string, such as "P2Y10M15DT10H30M20S",
// check whether the year string "2Y", or second string "20S" includes non-digit char
// except the last symbolic char.
bool StrUtil::validateDatetimeStr(const char* ptr, const int& len) {

    if (!ptr || strlen(ptr) == 0 || strlen(ptr) < (unsigned int)len)
    {
        ALOGE("validateDatetimeStr: date time string is null or empty");
        return false;
    }

    for(unsigned int i = 0;i < (unsigned int)len; i++)
    {
        if (!isdigit(ptr[i]))
        {
            ALOGE("validateDatetimeStr: date time string[%s] include non-digit[%c].",ptr,ptr[i]);
            return false;
        }
    }
    return true;
}

