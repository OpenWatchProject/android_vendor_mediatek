/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.memory;

import android.app.TabActivity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;

import java.io.IOException;

/**
 * Class for showing flash information.
 *
 */
@SuppressWarnings("deprecation")
public class Flash extends TabActivity {

    private static final String TAG = "Memory/Flash";
    private static final String FILE_NAND = "/proc/driver/nand";
    private static final String FILE_MOUNTS = "/proc/mounts";
    private static final String FILE_PARTITION = "/proc/partitions";
    private static final String FILE_MTD = "/proc/mtd";
    private static final String FILE_CID = "/sys/block/mmcblk0/device/cid";
    private static final String FILE_DUMCHAR_INFO = "/proc/dumchar_info";
    private static final String FILE_PARTINFO = "/proc/partinfo";
    private static final String READ_COMMAND = "cat ";
    private static final String EMMC_ID_HEADER = "emmc ID: ";
    private boolean mHaveEmmc = false;

    private TextView mTvCommInfo = null;
    private TextView mTvFSInfo = null;
    private TextView mTvPartInfo = null;
    private String mFileSysTabName = null;
    private String mPartitionTabName = null;
    private String mTabId = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mFileSysTabName = getString(R.string.memory_file_sys_info);
        mPartitionTabName = getString(R.string.memory_partition_info);
        TabHost tabHost = getTabHost();
        LayoutInflater.from(this).inflate(R.layout.memory_tabs,
                tabHost.getTabContentView(), true);

        // tab1
        tabHost.addTab(tabHost.newTabSpec(mFileSysTabName).setIndicator(
                mFileSysTabName).setContent(R.id.file_sys_view));

        // tab2
        tabHost.addTab(tabHost.newTabSpec(mPartitionTabName).setIndicator(
                mPartitionTabName).setContent(R.id.partition_view));
        mTvFSInfo = (TextView) findViewById(R.id.file_sys_info);
        mTvPartInfo = (TextView) findViewById(R.id.partition_info);
        tabHost.setOnTabChangedListener(new OnTabChangeListener() {
            public void onTabChanged(String tabId) {
                mTabId = tabId;
                showTabContent();
            }
        });

        // init
        mTabId = mFileSysTabName;
        showTabContent();
    }

    /**
     * Show TAB content.
     */
    private void showTabContent() {
        if (mTabId.equals(mFileSysTabName)) {
            mTvFSInfo.setText(getInfo(FILE_MOUNTS));
        } else if (mTabId.equals(mPartitionTabName)) {
            mTvPartInfo.setText(getInfo(FILE_PARTITION));

        }
    }

     /**
     * Get file content.
     *
     * @param file
     *            The file's path
     * @return The file's content
     */
    private String getInfo(String file) {
        String result = null;
        try {
            int ret = ShellExe.execCommand(READ_COMMAND + file);
            if (0 == ret) {
                result = ShellExe.getOutput();
            } else {
                result = getString(R.string.memory_getinfo_error);
            }
        } catch (IOException e) {
            Elog.w(TAG, e.toString());
            result = e.toString();
        }
        return result;
    }


    @SuppressWarnings("deprecation")
    @Override
    protected void onResume() {
        super.onResume();
        showTabContent();
    }

}
