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
#ifndef __RMC_CDMA_BC_RANGE_PARSER_H__
#define __RMC_CDMA_BC_RANGE_PARSER_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <Vector.h>
#include <SortedVector.h>

/*****************************************************************************
 * Name Space
 *****************************************************************************/
using android::Vector;
using android::SortedVector;

/*****************************************************************************
 * Typedef
 *****************************************************************************/
/*
 * The structure stands for cell broadcast ranges
 * such as 4096-4096, 4098-4101
 */
typedef struct {
    int start;
    int end;
} Range;

/*****************************************************************************
 * Define
 *****************************************************************************/
/*
 * The max cell broadcast cateogory range number
 * supported by MD
 */
#define MAX_RANGE    10

/*
 * The max character number of a range
 * include '\0'
 * range example: "65534-65535", total 14 character,
 * allow to include some spaces
 */
#define MAX_RANGE_CHARACTER       20

/* A range include a start endpoint and a end endpoint */
#define VALID_RANGE_ENDPOINT_NUM  2

/*****************************************************************************
 * Class RmcCdmaBcRangeParser
 *****************************************************************************/
class RmcCdmaBcRangeParser {
public:
    static int getRangeFromModem(char *cateogry, Range *r);
    static void getRange(Vector<Range> &range, SortedVector<int> &input);
private:
    static void split(char *src, const char *sep, char dest[][MAX_RANGE_CHARACTER], int* num);
    static char* trim(char *s);
    static char* skipQuote(char *s);
};
#endif /* __RMC_CDMA_BC_RANGE_PARSER_H__ */
