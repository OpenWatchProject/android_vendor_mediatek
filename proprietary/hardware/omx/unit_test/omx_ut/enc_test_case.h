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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   dec_test_case.h
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video unit test code
 *
 * Author:
 * -------
 *   Bruce Hsu (mtk04278)
 *
 ****************************************************************************/

#ifndef __ENC_TEST_CASE_H__
#define __ENC_TEST_CASE_H__

#include "enc_test.h"

struct EncTestCaseConf {
    int     iCodec;
    //char    *szInFile1;
    //int     iWidth1;
    //int     iHeight1;
    //char    *szInFile2;
    //char    *szGolden;
    //int     iWidth2;
    //int     iHeight2;
};

class EncTestCases {
    public:
        //conformance
        int lastFrameTest(EncTestCaseConf  &tCaseConf);
        //int portReconfigTest1(EncTestCaseConf  &tCaseConf);   //not supported
        //int portReconfigTest2(EncTestCaseConf  &tCaseConf);   //not supported
        //error handle
        //int frame1NotITest(EncTestCaseConf  &tCaseConf);      //not supported
        //int partialFrameTest(EncTestCaseConf  &tCaseConf);    //not supported
        int bufferFlagLeakTest(EncTestCaseConf  &tCaseConf);
        int noEOSFlagTest(EncTestCaseConf  &tCaseConf);
        //int portReconfigFailTest(EncTestCaseConf  &tCaseConf);//not supported
        int outBufFullTest(EncTestCaseConf  &tCaseConf);
        //int noSequenceHeadTest(EncTestCaseConf  &tCaseConf);  //not supported
        //int corruptDataTest(EncTestCaseConf  &tCaseConf);     //not supported
        //special
        //int loopPlaybackTest(EncTestCaseConf  &tCaseConf);    //not supported
        //int trickPlayTest(EncTestCaseConf  &tCaseConf);       //not supported

        int multiInstancesTest(EncTestCaseConf &tCaseConf);

        EncTestConf    mConf;
    private:
        int encTestConfig(int iCodec);
};

#endif//__ENC_TEST_CASE_H__

