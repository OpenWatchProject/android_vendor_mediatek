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

#ifndef ANDROID_TESTCASE_H
#define ANDROID_TESTCASE_H

/*
 * Notice
 * 1. remember to set the all SCN off in each test case.
 * 2. keywords in pattern
 *    - SCN: scenario, must follow by scn num and state. ex. SCN,A,B
 *    - SLP: unit ms, used for delay operation, ex. SLP,A
 *    - CHK: check the cpu cores ex. CHK,A
 * 3. the cpu num check should be specified in test pattern to reduce test program logic
 *
 *
 * TODO:
 * 1. change the test case to other configurable files thus we don't need build code to do the test.
 * 
 */
struct {
	const char *testDescription;
	const char *testPattern;
} testcase_set[] = {
	{"Test SCN Idx 1...",
	"SCN,1,ON,SLP,100,CHK,2,SLP,100,SCN,1,OFF"},
	{"Test SCN Idx 2...",
	"SCN,2,ON,SLP,100,CHK,2,SLP,100,SCN,2,OFF"},
#if 0
	{"Test SCN Idx 3...",
	"SCN,3,ON,SLP,100,CHK,2,SLP,100,SCN,3,OFF"},
#endif
	/* SCN_2_ON -> SCN_1_ON -> SCN_2_OFF -> SCN_1_OFF */
	{"Test SCN combination (1)...",
	"SCN,2,ON,SLP,100,SCN,1,ON,SLP,100,CHK,2,SCN,2,OFF,SLP,100,CHK,2,SCN,1,OFF"},
	/* SCN_2_ON -> SCN_1_ON -> SCN_1_OFF -> SCN_2_OFF */
	{"Test SCN combination (2)...",
	"SCN,2,ON,SLP,100,SCN,1,ON,SLP,100,CHK,2,SCN,1,OFF,SLP,100,CHK,2,SCN,2,OFF"},
	/* SCN_1_ON -> SCN_2_ON -> SCN_1_OFF -> SCN_2_OFF */
	{"Test SCN combination (3)...",
	"SCN,1,ON,SLP,100,SCN,2,ON,SLP,100,CHK,2,SCN,1,OFF,SLP,100,CHK,2,SCN,2,OFF"},
	/* SCN_1_ON -> SCN_2_ON -> SCN_2_OFF -> SCN_1_OFF */
	{"Test SCN combination (4)...",
	"SCN,1,ON,SLP,100,SCN,2,ON,SLP,100,CHK,2,SCN,2,OFF,SLP,100,CHK,2,SCN,1,OFF"},
#if 0
	/* SCN_1_ON -> SCN_3_ON -> SCN_1_OFF -> SCN_3_OFF */
	{"Test SCN combination (5)...",
	"SCN,1,ON,SLP,100,SCN,3,ON,SLP,100,CHK,2,SCN,1,OFF,SLP,100,CHK,2,SCN,3,OFF"},
	/* SCN_1_ON -> SCN_3_ON -> SCN_3_OFF -> SCN_1_OFF */
	{"Test SCN combination (6)...",
	"SCN,1,ON,SLP,100,SCN,3,ON,SLP,100,CHK,2,SCN,3,OFF,SLP,100,CHK,2,SCN,1,OFF"},
	/* SCN_2_ON -> SCN_3_ON -> SCN_2_OFF -> SCN_3_OFF */
	{"Test SCN combination (5)...",
	"SCN,2,ON,SLP,100,SCN,3,ON,SLP,100,CHK,2,SCN,2,OFF,SLP,100,CHK,2,SCN,3,OFF"},
	/* SCN_2_ON -> SCN_3_ON -> SCN_3_OFF -> SCN_2_OFF */
	{"Test SCN combination (6)...",
	"SCN,2,ON,SLP,100,SCN,3,ON,SLP,100,CHK,2,SCN,3,OFF,SLP,100,CHK,2,SCN,2,OFF"},
#endif
	{NULL,NULL},
};

#endif //ANDROID_TESTCASE_H
