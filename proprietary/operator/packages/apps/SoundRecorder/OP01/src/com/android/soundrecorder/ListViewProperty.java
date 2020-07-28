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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.android.soundrecorder;

import java.util.List;

/** M: class use to save current status of list view in RecordingFileList. */
public class ListViewProperty {
    private List<Integer> mCheckedList;
    private int mCurPos;
    private int mTop;

    /**
     * construction of ListViewProperty.
     *
     * @param list
     *            the index list of current checked item
     * @param curPos
     *            the index of current current position
     * @param top
     *            the index of the top item
     */
    public ListViewProperty(List<Integer> list, int curPos, int top) {
        mCheckedList = list;
        mCurPos = curPos;
        mTop = top;
    }

    /**
     * M: return the current check status of list view.
     *
     * @return the mCheckedList
     */
    public List<Integer> getCheckedList() {
        return mCheckedList;
    }

    /**
     * save checked list.
     *
     * @param checkedList
     *            the mCheckedList to set
     */
    public void setCheckedList(List<Integer> checkedList) {
        this.mCheckedList = checkedList;
    }

    /**
     * M: return the current position of list view.
     *
     * @return the mCurPos
     */
    public int getCurPos() {
        return mCurPos;
    }

    /**
     * M: set the current position of list view.
     *
     * @param curPos
     *            the mCurPos to set
     */
    public void setCurPos(int curPos) {
        this.mCurPos = curPos;
    }

    /**
     * M: get the top position of list view.
     *
     * @return the mTop
     */
    public int getTop() {
        return mTop;
    }

    /**
     * M: set the top position of list view.
     *
     * @param top
     *            the mTop to set
     */
    public void setTop(int top) {
        this.mTop = top;
    }
}
