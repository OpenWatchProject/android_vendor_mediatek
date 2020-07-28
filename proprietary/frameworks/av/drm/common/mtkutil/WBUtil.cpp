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

#define LOG_NDEBUG 1
#define LOG_TAG "DrmMtkUtil/WBUtil"
#include <utils/Log.h>

#include <WBUtil.h>
#include <DrmDef.h>
#include <DrmUtil.h>
#include <StrUtil.h>

#include <stdlib.h>
#include <string.h>
#include <strings.h>

using namespace android;

#define MIN_START_TIME 0x80000000
#define MAX_END_TIME 0x7fffffff

bool WBUtil::parseWbXml(char* buffer, int length, RO* ro)
{
    WB wb(buffer, length);
    // check 3 byte header
    if (!(wb.p[0] == 0x03 && wb.p[1] == 0x0e && wb.p[2] == 0x6a))
    {
        ALOGE("error: 3 bytes header of wbxml is invalid");
        return false;
    }

    // buffer[3] should be 0x00
    wb.offset = 4;

    // find rights-start-tag
    if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_RIGHTS_TAG)
    {
        ALOGE("Not found rights-start-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found rights-start-tag, offset:[%d]", wb.offset);

    // find rights-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found rights-end-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found rights-end-tag, offset:[%d]", wb.offset);

    // find context-start-tag
    if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_CONTEXT_TAG)
    {
        ALOGE("Not found context-start-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found context-start-tag, offset:[%d]", wb.offset);

    // find version-start-tag
    if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_VERSION_TAG)
    {
        ALOGE("Not found version-start-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found version-start-tag, offset:[%d]", wb.offset);

    // check version, shoud be "0.1"
    char* ptr = &wb.p[wb.offset + 1];
    if (strncmp(ptr, "1.0", 3) != 0)
    {
        ALOGE("error: version:[%s] must be 1.0", ptr);
        return false;
    }

    // find version-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found version-end-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found version-end-tag, offset:[%d]", wb.offset);

    // find context-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found context-end-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found context-end-tag, offset:[%d]", wb.offset);

    // find agreement-start-tag
    if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_AGREEMENT_TAG)
    {
        ALOGE("Not found agreement-start-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found agreement-end-tag, offset:[%d]", wb.offset);

    // find asset-start-tag
    if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_ASSET_TAG)
    {
        ALOGE("Not found asset-start-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found asset-end-tag, offset:[%d]", wb.offset);

    // find context-start-tag
    if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_CONTEXT_TAG)
    {
        ALOGE("Not found context-start-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found context-start-tag, offset:[%d]", wb.offset);

    // find uid-start-tag
    if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_UID_TAG)
    {
        ALOGE("Not found uid-start-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found uid-start-tag, offset:[%d]", wb.offset);

    // get cid
    ptr = &wb.p[wb.offset + 1];
    bzero(ro->rights.cid, DrmDef::DRM_MAX_META_LENGTH);

    String8 content_uri = StrUtil::getContentUri(String8(ptr));
    if (content_uri.length() == 0)
    {
        ALOGE("error: content uri (cid) invalid (empty)");
        return false;
    }
    ALOGV("trim left, content uri (cid):[%s]", content_uri.string());

    snprintf(ro->rights.cid, sizeof(ro->rights.cid), "%s", content_uri.string());
    wb.offset += (strlen(ptr) + 2);

    // find uid-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found uid-end-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found uid-end-tag, offset:[%d]", wb.offset);

    // find context-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found context-end-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found context-end-tag, offset:[%d]", wb.offset);

    // CD has no keyinfo & keyvalue, SD has.
    char ch = 0;
    if (!peekNextStartTag(wb, ch))
    {
        ALOGE("Not found start-tag below context-end-tag, offset:[%d]", wb.offset);
        return false;
    }
    if ((ch & 0x3f) == DrmDef::DRM_WB_KEYINFO_TAG)
    {
        // find keyinfo-start-tag
        if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_KEYINFO_TAG)
        {
            ALOGE("Not found keyinfo-start-tag, offset:[%d]", wb.offset);
            return false;
        }
        ALOGV("found keyinfo-start-tag, offset:[%d]", wb.offset);

        // find keyvalue-start-tag
        if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_KEYVALUE_TAG)
        {
            ALOGE("Not found keyvalue-start-tag, offset:[%d]", wb.offset);
            return false;
        }
        ALOGV("found keyvalue-start-tag, offset:[%d]", wb.offset);

        // get key
        memcpy(ro->rights.key, &wb.p[wb.offset + 2], DrmDef::DRM_MAX_KEY_LENGTH);
        wb.offset += 18;

        // find keyvalue-end-tag
        if (!gotoNextEndTag(wb))
        {
            ALOGE("Not found keyvalue-end-tag, offset:[%d]", wb.offset);
            return false;
        }
        ALOGV("found keyvalue-end-tag, offset:[%d]", wb.offset);

        // find keyinfo-end-tag
        if (!gotoNextEndTag(wb))
        {
            ALOGE("Not found keyinfo-end-tag, offset:[%d]", wb.offset);
            return false;
        }
        ALOGV("found keyinfo-end-tag, offset:[%d]", wb.offset);
    }

    // find asset-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found asset-end-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found asset-end-tag, offset:[%d]", wb.offset);

    // find permission-start-tag
    if (!gotoNextStartTag(wb) || (wb.p[wb.offset - 1] & 0x3f) != DrmDef::DRM_WB_PERMISSION_TAG)
    {
        ALOGE("Not found permission-start-tag, offset:[%d]", wb.offset);
        return false;
    }
    ALOGV("found permission-start-tag, offset:[%d]", wb.offset);

    // allocate entry memory
    ro->rights.entryPtr = new Entry[DrmDef::PERMISSION_TOTAL_INDEX];
    bzero(ro->rights.entryPtr, DrmDef::PERMISSION_TOTAL_INDEX);

    // find display/play/execute/print-start-tag
    while (true)
    {
        if (!gotoNextStartTag(wb))
        {
            ALOGV("Not found start-tag below permission-start-tag, offset:[%d]", wb.offset);
            break;
        }
        int permission = DrmDef::PERMISSION_NONE_INDEX;
        if ((wb.p[wb.offset - 1] & 0x3f) == DrmDef::DRM_WB_PLAY_TAG)
        {
            permission = DrmDef::PERMISSION_PLAY_INDEX;
            ALOGV("found play-tag, offset:[%d]", wb.offset);
        }
        else if ((wb.p[wb.offset - 1] & 0x3f) == DrmDef::DRM_WB_DISPLAY_TAG)
        {
            permission = DrmDef::PERMISSION_DISPLAY_INDEX;
            ALOGV("found display-tag, offset:[%d]", wb.offset);
        }
        else if ((wb.p[wb.offset - 1] & 0x3f) == DrmDef::DRM_WB_EXECUTE_TAG)
        {
            permission = DrmDef::PERMISSION_EXECUTE_INDEX;
            ALOGV("found execute-tag, offset:[%d]", wb.offset);
        }
        else if ((wb.p[wb.offset - 1] & 0x3f) == DrmDef::DRM_WB_PRINT_TAG)
        {
            permission = DrmDef::PERMISSION_PRINT_INDEX;
            ALOGV("found print-tag, offset:[%d]", wb.offset);
        }

        if (permission != DrmDef::PERMISSION_NONE_INDEX)
        {
            ro->rights.permission |= permission;
            ro->rights.num[permission] = 1;
            ro->rights.best[permission] = permission;
            ro->rights.entryPtr[permission].type = DrmDef::CONSTRAINT_UNINIT;
            ro->rights.entryPtr[permission].next = 0xFF;

            if ((wb.p[wb.offset - 1] & 0x40)) // e.g. is <display>
            {
                ALOGV("parseConstraint, offset:[%d]", wb.offset);
                if (!parseConstraint(wb, permission, ro))
                {
                    return false; // the constraint is failed to be parsed
                }
                // find display-end-tag
                if (!gotoNextEndTag(wb))
                {
                    ALOGE("Not found %s-end-tag, offset:[%d]", DrmDef::getPermissionStr(permission).string(), wb.offset);
                    return false;
                }
            }
            else
            {
                ALOGV("<%s/> happened, offset:[%d]", DrmDef::getPermissionStr(permission).string(), wb.offset);
            }

            // if the entry is valid, we just ignore it
            if (!DrmUtil::isEntryValid(&ro->rights.entryPtr[permission]))
            {
                ro->rights.num[permission] = 0;
                ro->rights.best[permission] = 0xFF;
                ALOGW("remove invalid entry for [%s].", DrmDef::getPermissionStr(permission).string());
            }
        }
    }

    // find permission-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found permission-end-tag, offset:[%d]", wb.offset);
        return false;
    }

    // find agreement-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found agreement-end-tag, offset:[%d]", wb.offset);
        return false;
    }

    // find rights-end-tag
    if (!gotoNextEndTag(wb))
    {
        ALOGE("Not found rights-end-tag, offset:[%d]", wb.offset);
        return false;
    }
    return true;
}

bool WBUtil::parseConstraint(WB& wb, int permission, RO* ro)
{
    ALOGV("parseConstraint ---->");
    ALOGV("1. [%d]", wb.offset);
    char ch = 0;
    if (peekNextStartTag(wb, ch) && (ch & 0x3f) == DrmDef::DRM_WB_CONSTRAINT_TAG) // constraint-start-tag
    {
        ALOGV("2. [%d]", wb.offset);
        if (!gotoNextStartTag(wb))
        {
            ALOGE("gotoNextStartTag() failed, offset:[%d]", wb.offset);
            goto ERROR;
        }
        ALOGV("found constraint-start-tag, offset:[%d]", wb.offset);

        while (true)
        {
            char ch = 0;
            if (peekNextStartTag(wb, ch)) // have start-tag below constraint-start-tag
            {
                if ((ch & 0x3f) == DrmDef::DRM_WB_COUNT_TAG) // count-start-tag
                {
                    if (!gotoNextStartTag(wb))
                    {
                        ALOGE("gotoNextStartTag() failed, offset:[%d]", wb.offset);
                        goto ERROR;
                    }
                    ALOGV("found count-start-tag, offset:[%d]", wb.offset);
                    ALOGV("count str:[%s]", &wb.p[wb.offset + 1]);
                    ALOGV("count:[%d]", atoi(&wb.p[wb.offset + 1]));
                    ro->rights.entryPtr[permission].type |= DrmDef::CONSTRAINT_COUNT;
                    ro->rights.entryPtr[permission].total_count = atoi(&wb.p[wb.offset + 1]);
                    ro->rights.entryPtr[permission].used_count = ro->rights.entryPtr[permission].total_count;

                    if (!gotoNextEndTag(wb)) // count-end-tag
                    {
                        ALOGE("Not found count-end-tag, offset:[%d]", wb.offset);
                        goto ERROR;
                    }
                    ALOGV("found count-end-tag, offset:[%d]", wb.offset);
                }
                else if ((ch & 0x3f) == DrmDef::DRM_WB_INTERVAL_TAG) // interval-start-tag
                {
                    if (!gotoNextStartTag(wb))
                    {
                        ALOGE("gotoNextStartTag failed, offset:[%d]", wb.offset);
                        goto ERROR;
                    }
                    ALOGV("found interval-start-tag, offset:[%d]", wb.offset);
                    ALOGV("interval str:[%s]", &wb.p[wb.offset + 1]);
                    if (!StrUtil::intervalStrToTick(&wb.p[wb.offset + 1], ro->rights.entryPtr[permission].interval))
                    {
                        ALOGE("interval string:[%s] is not valid.", &wb.p[wb.offset + 1]);
                        goto ERROR;
                    }
                    ro->rights.entryPtr[permission].type |= DrmDef::CONSTRAINT_INTERVAL;

                    if (!gotoNextEndTag(wb)) // interval-end-tag
                    {
                        ALOGE("Not found interval-end-tag, offset:[%d]", wb.offset);
                        goto ERROR;
                    }
                    ALOGV("found interval-end-tag, offset:[%d]", wb.offset);
                }
                else if ((ch & 0x3f) == DrmDef::DRM_WB_DATETIME_TAG) // datetime-start-tag
                {
                    if (!gotoNextStartTag(wb))
                    {
                        ALOGE("gotoNextStartTag failed, offset:[%d]", wb.offset);
                        goto ERROR;
                    }
                    ALOGV("found datetime-start-tag, offset:[%d]", wb.offset);
                    // M: whether the datatime start-tag and end-tag is found @{
                    bool bStartTag = true;
                    bool bEndTag = true;
                    // M: @}
                    if (peekNextStartTag(wb, ch) && (ch & 0x3f) == DrmDef::DRM_WB_START_TAG)
                    {
                        if (!gotoNextStartTag(wb))
                        {
                            ALOGE("gotoNextStartTag failed, offset:[%d]", wb.offset);
                            goto ERROR;
                        }
                        ALOGV("found start-start-tag, offset:[%d]", wb.offset);
                        ALOGV("start:[%s]", &wb.p[wb.offset + 1]);
                        //M: @{
                        //make sure the rights can be installed successfully
                        //if not specify the start date-time as <o-dd:start></o-dd:start>
                        if (wb.p[wb.offset] == 0x01)
                        {   // assume start date_time valid if not specify start date_time
                            ALOGV("date-time start time not specify, assign default start time");
                        //M: @}
                        } else if (!StrUtil::datetimeStrToTick(&wb.p[wb.offset + 1], ro->rights.entryPtr[permission].start_time))
                        {
                            ALOGE("date-time string:[%s] is not valid.", &wb.p[wb.offset + 1]);
                            goto ERROR;
                        }

                        ro->rights.entryPtr[permission].type |= DrmDef::CONSTRAINT_DATETIME;

                        if (!gotoNextEndTag(wb)) // start-end-tag
                        {
                            ALOGE("Not found start-end-tag, offset:[%d]", wb.offset);
                            goto ERROR;
                        }
                        ALOGV("found start-end-tag, offset:[%d]", wb.offset);
                    } else
                    {
                        ALOGD("datetime constraint:start tag not found");
                        bStartTag = false;
                    }
                    if (peekNextStartTag(wb, ch) && (ch & 0x3f) == DrmDef::DRM_WB_END_TAG)
                    {
                        if (!gotoNextStartTag(wb))
                        {
                            ALOGE("gotoNextStartTag failed, offset:[%d]", wb.offset);
                            goto ERROR;
                        }
                        ALOGV("found end-start-tag, offset:[%d]", wb.offset);
                        ALOGV("end:[%s]", &wb.p[wb.offset + 1]);
                        //M: @{
                        //make sure the rights can be installed successfully
                        //if not specify the end date-time as <o-dd:end></o-dd:end>
                        if (wb.p[wb.offset] == 0x01)
                        {   // assume end date-time valid if not specify end date-time
                            ALOGV("date-time end time not specify, assign default end time");
                        //M: @}
                        } else if (!StrUtil::datetimeStrToTick(&wb.p[wb.offset + 1], ro->rights.entryPtr[permission].end_time))
                        {
                            ALOGE("date-time string:[%s] is not valid.", &wb.p[wb.offset + 1]);
                            goto ERROR;
                        }

                        ro->rights.entryPtr[permission].type |= DrmDef::CONSTRAINT_DATETIME;

                        if (!gotoNextEndTag(wb)) // end-end-tag
                        {
                            ALOGE("Not found end-end-tag, offset:[%d]", wb.offset);
                            goto ERROR;
                        }
                        ALOGV("found end-end-tag, offset:[%d]", wb.offset);
                    } else
                    {
                        ALOGD("datetime constraint:end tag not found");
                        bEndTag = false;
                    }
                    // M: check the date time @{
                    if (!bStartTag && !bEndTag)
                    {
                        ALOGD("datetime: both start-tag and end-tag not found, assume this constraint has no meaning and ignore");

                    } else if (ro->rights.entryPtr[permission].start_time == (int) MIN_START_TIME &&
                        ro->rights.entryPtr[permission].end_time == (int) MAX_END_TIME)
                    {
                        ALOGE("datetime not specify both start_time and end_time");
                        goto ERROR;
                    } else if (ro->rights.entryPtr[permission].start_time >= ro->rights.entryPtr[permission].end_time)
                    {
                        ALOGE("datetime constraint: start time >= end time");
                        goto ERROR;
                    }
                    // M: @}

                    if (!gotoNextEndTag(wb)) // datetime-end-tag
                    {
                        ALOGE("Not found datetime-end-tag, offset:[%d]", wb.offset);
                        goto ERROR;
                    }
                    ALOGV("found datetime-end-tag, offset:[%d]", wb.offset);
                }
                else  // not count / date time / interval tag
                {
                    break;
                }
            }
            else // do not have start-tag below constraint-start-tag
            {
                break; // -> check constraint-end-tag
            }
        }

        if (!gotoNextEndTag(wb)) // find constraint-end-tag
        {
            ALOGE("Not found constraint-end-tag, offset:[%d]", wb.offset);
            goto ERROR;
        }
        ALOGV("found constraint-end-tag, offset:[%d]", wb.offset);
    }

    ALOGV("parseConstraint finihsed. <----");
    return true;

ERROR:
    ALOGE("parseConstraint failed. <----");
    return false;
}

bool WBUtil::gotoNextStartTag(WB& wb)
{
    return gotoNextTag(wb, true);
}

bool WBUtil::gotoNextEndTag(WB& wb)
{
    return gotoNextTag(wb, false);
}

bool WBUtil::peekNextStartTag(WB& wb, char& ch)
{
    int i = wb.offset;
    while (i < wb.len)
    {
        char c = wb.p[i] & 0x3f; // clear 2 high bit to 0
        if (c >= DrmDef::DRM_WB_RIGHTS_TAG && c <= DrmDef::DRM_WB_INTERVAL_TAG)
        {
            ch = wb.p[i];
            return true; // found tag
        }
        i++;
    }
    return false; // not found tag
}

bool WBUtil::gotoNextTag(WB& wb, bool start)
{
    int i = wb.offset;
    while (i < wb.len)
    {
        char c = wb.p[i] & 0x3f; // clear 2 high bit to 0
        if ((start && c >= DrmDef::DRM_WB_RIGHTS_TAG && c <= DrmDef::DRM_WB_INTERVAL_TAG)
            || (!start && c == 0x01))
        {
            wb.offset = i + 1;
            return true; // found tag
        }
        i++;
    }
    return false; // not found tag
}
