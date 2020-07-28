package com.mediatek.mtklogger.c2klogger;

import android.util.Log;

import com.mediatek.mtklogger.c2klogger.EtsDnlder.CBPMode;
import com.mediatek.mtklogger.c2klogger.EtsDnlder.DnldStatus;

/**
 * @author MTK81255
 *
 */
public class EtsDnlderThread extends Thread {

    private EtsDnlder mDnlder;

    /**
     * @param dnlder EtsDnlder
     */
    public EtsDnlderThread(EtsDnlder dnlder) {
        mDnlder = dnlder;
    }

    private void close() {
        mDnlder.close();

    }

    @Override
    public void run() {
        Log.i("via_ets", "Donwloader Thread started");

        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // loopback test
        int retry = 10;
        while (retry > 0) {
            if (mDnlder.loopback()) {
                break;
            } else {
                --retry;
            }
        }

        if (retry == 0) {
            mDnlder.getCallback().onProcess(DnldStatus.Error, 0,
                    "Do loopback failed");
            close();
            return;
        }
        mDnlder.getCallback().onProcess(DnldStatus.Readying, 0,
                "Do loopback success");

        // check the mode of cbp

        // to boot
        if (!mDnlder.jump2load(CBPMode.Boot, true)) {
            mDnlder.getCallback().onProcess(DnldStatus.Error, 0,
                    "Reset device to boot failed");
            close();
            return;
        }
        mDnlder.getCallback().onProcess(DnldStatus.WaitingBoot, 0,
                "Reset device to boot succuss");

        for (String imgPath : mDnlder.getImgFiles()) {
            byte flashSection = mDnlder.getFlashSectionIndex(imgPath);
            if (flashSection < 0
                    || flashSection >= EtsDnlder.sSectionName.length) {
                continue;
            }

            if (!mDnlder.downloadFlash(flashSection, imgPath)) {
                close();
                mDnlder.getCallback().onProcess(DnldStatus.Error, 0,
                        "Download flash failed");
                return;
            }

        }

        mDnlder.jump2load(CBPMode.CP, false);
        mDnlder.getCallback().onProcess(DnldStatus.Finishied, 0,
                "Download flash finished");
    }

}
