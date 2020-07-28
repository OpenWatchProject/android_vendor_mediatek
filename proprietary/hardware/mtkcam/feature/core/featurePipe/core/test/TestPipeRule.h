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

#ifndef _TEST_RULE_H_
#define _TEST_RULE_H_

/******************************************************************************
* A => B =>
*   => C => D
*
* A - check dummy buffer all ready (in:2, out:3)
*   - call both B and C
* B - delay 50ms
*   - update result: DIV5=(RequestNo/5)
*   - CB partial result
* C - delay 80ms
*   - update result: MOD5=(RequestNo%5)
* D - wait B and C
*   - update result: RESULT=B+C(RequestNo/5 + RequestNo%5)
*   - CB result
******************************************************************************/

#include "TestRequest.h"
#include "TestTool.h"
#include <sstream>

#define IN_BUFFERS 2
#define OUT_BUFFERS 3
#define NODE_A_DELAY_NS 0
#define NODE_B_DELAY_NS 5 * NS_PER_MS
#define NODE_C_DELAY_NS 10 * NS_PER_MS
#define NODE_D_DELAY_NS 8 * NS_PER_MS

#define KEY_RESULT_B "DIV5"
#define KEY_RESULT_C "MOD5"
#define KEY_RESULT_D "Result"

#define STR_PROCESSING  "processing"
#define STR_DONE        "done"

#define TEST_REQUEST_COUNT  100
#define TEST_WAIT_PERIOD_NS 100 * NS_PER_MS

#define TEST_VERSION_STR    "0.0.14"
//#define SHOW_TEST_VERSION

int Func1(int val);
int Func2(int val);
int Func3(int val1, int val2);

#endif // _TEST_RULE_H_
