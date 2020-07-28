/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "RmcCdmaBcRangeParser.h"

/*****************************************************************************
 * Class RmcCdmaBcRangeParser
 *****************************************************************************/
void RmcCdmaBcRangeParser::split(char *src, const char *sep, char dest[][MAX_RANGE_CHARACTER], int* num) {
    char *next;
    int count = 0;
    if (src == NULL || strlen(src) == 0) {
        return;
    }

    if (sep == NULL || strlen(sep) == 0) {
        return;
    }
    next = strtok(src, sep);
    while (next != NULL) {
        strncpy(*dest++, next, MAX_RANGE_CHARACTER - 1);
        count++;
        next = strtok(NULL, sep);
    }
    *num =count;
}

char* RmcCdmaBcRangeParser::trim(char *s) {
    int i;
    if (s == NULL) {
        return NULL;
    }
    char *start = NULL;
    while (*s) {
        if ((*s != '\t') && (*s != '\n') && (*s != ' ')) {
            start = s;
            break;
        }
        s++;
    }
    if (start == NULL) {
        return NULL;
    }

    for (i = strlen(s) - 1; i >= 0; i--) {
        if ((s[i] != '\t') && (s[i] != '\n') && (s[i] != ' ')) {
            break;
        }
        s[i] = '\0';
    }
    return start;
}

char* RmcCdmaBcRangeParser::skipQuote(char *s) {
    int i;
    if (s == NULL) {
        return NULL;
    }
    i = strlen(s) - 1;
    if (s[i] == '\"') {
        s[i] = '\0';
    }
    if (s[0] == '\"') {
        s++;
    }
    return s;
}



int RmcCdmaBcRangeParser::getRangeFromModem(char *cateogry, Range *r) {
    char ranges[MAX_RANGE][MAX_RANGE_CHARACTER];
    memset(ranges, 0, sizeof(ranges));
    int num;
    int validNum = 0;
    int i;
    split(cateogry, ",", ranges, &num);
    for (i = 0; i < num; i++) {
        char range[MAX_RANGE][MAX_RANGE_CHARACTER];
        memset(range, 0, sizeof(range));
        int n;
        split(skipQuote(trim(ranges[i])), "-", range, &n);
        if (n == VALID_RANGE_ENDPOINT_NUM) {
            r[validNum].start = atoi(range[0]);
            r[validNum].end = atoi(range[1]);
            validNum ++;
        }
    }
    return validNum;
}


void RmcCdmaBcRangeParser::getRange(Vector<Range> &range, SortedVector<int> &input) {
    int num = 0;
    int i;
    int start = input[0];
    int n = input.size();
    for (i = 1; i < n; i++) {
        if (input[i] > input[i - 1] + 1) {
            Range r;
            r.start = start;
            r.end = input[i - 1];
            range.push(r);
            start = input[i];
        }
    }
    Range r;
    r.start = start;
    r.end = input[i - 1];
    range.push(r);
    return;
}