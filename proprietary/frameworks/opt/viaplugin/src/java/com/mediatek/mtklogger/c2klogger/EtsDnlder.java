package com.mediatek.mtklogger.c2klogger;

import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.security.InvalidParameterException;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class EtsDnlder extends EtsDevice {

    private int mSizeOnePackage = 260;

    /**
     * @author MTK81255
     *
     */
    public enum CBPMode {
        Boot, CP, Unknown
    }

    /**
     * download status.
     * @author kma
     *
     */
    public enum DnldStatus {
        Readying, // read for beginning
        WaitingBoot, // waiting for the cbp to boot state
        Erasing, // erasing the flash
        Downloading, // writing the flash
        Finishied, // download finished
        Error // error
    }

    /**
     * callback for downloading image.
     * @author kma
     *
     */
    public interface EtsDnlderCallback {
        /**
         * @param status DnldStatus
         * @param progress int
         * @param info String
         */
        void onProcess(DnldStatus status, int progress, String info);
    }

    private EtsDnlderCallback mCallback = null;

    public EtsDnlderCallback getCallback() {
        return mCallback;
    }

    private EtsDnlderThread mDnlderThr;

    /**
     * the constructor.
     * @param sizeOnePackage int
     * @param callback EtsDnlderCallback
     */
    public EtsDnlder(int sizeOnePackage, EtsDnlderCallback callback) {
        super();

        mSizeOnePackage = sizeOnePackage;

        // set the callback
        if (callback != null) {
            mCallback = callback;
        } else {
            mCallback = new EtsDnlderCallback() {
                public void onProcess(DnldStatus status, int progress, String info) {
                    if (status == DnldStatus.Error) {
                        Log.e("via_ets", info);
                    } else if (status == DnldStatus.Downloading) {
                        Log.i("via_ets", info + " progress:" + progress);
                    } else {
                        Log.i("via_ets", info);
                    }
                }
            };
        }

    }

    private List<String> mImgFiles;
    static public String[] sSectionName = { "BOOT", "CP" };

    public List<String> getImgFiles() {
        return mImgFiles;
    }

    /**
     * @param imgFile String
     * @return byte
     */
    public byte getFlashSectionIndex(String imgFile) {
        byte flashSectionIndex = -1;
        String pathnameFile = imgFile.toLowerCase();

        if (pathnameFile.indexOf("boot") > 0) {
            flashSectionIndex = 0;
        } else if (pathnameFile.indexOf("cp") > 0) {
            flashSectionIndex = 1;
        } else {
            Log.e("via_ets", "unknown section for image:\"" + imgFile + "\"");
        }

        return flashSectionIndex;
    }

    /**
     */
    public void close() {
        this.destroy();
    }

    /*
     * private boolean setResetMode(String mode){ String resetMode =
     * SystemProperties.get("persist.cp.reset.mode"); Log.v("via_ets",
     * "current \"don't reset modem\" = "+ resetMode); if
     * (resetMode.equals(mode)) { return true; } Log.v("via_ets",
     * "set the \"don't reset modem\" = " + mode);
     * SystemProperties.set("persist.cp.reset.mode", mode); resetMode =
     * SystemProperties.get("persist.cp.reset.mode"); Log.v("via_ets",
     * "\"don't reset modem\" = "+ resetMode + " after set."); return
     * resetMode.equals(mode); }
     */

    /**
     * @return boolean
     */
    public boolean test() {
        if (!create("/dev/ttyUSB1")) {
            Log.e("test", "open the ets device failed");
            return false;
        }

        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
            return false;
        }

        /*
         * List<String> imgPaths = new ArrayList<String>();
         * imgPaths.add(android.
         * os.Environment.getExternalStorageDirectory()+"/cbp/img/boot.rom");
         * imgPaths.add(android.os.Environment.getExternalStorageDirectory()+
         * "/cbp/img/cp.rom"); try { start(imgPaths); } catch (EtsException e) {
         * Log.e("via_ets", e.getMessage()); }
         */

        Log.i("test", "do loopback");
        int retry = 3;
        while (retry > 0) {
            if (loopback()) {
                break;
            } else {
                --retry;
            }
        }

        if (retry == 0) {
            close();
            return false;
        }

        Log.i("test", "do jump to boot mode");
        if (!jump2load(CBPMode.Boot, true)) {
            close();
            return false;
        }

        Log.i("test", "do erase the cp section");
        if (!eraseFlash((byte) 1)) {
            close();
            return false;
        }

        Log.i("test", "test end with successful");
        close();
        return true;
    }

    /**
     * @return boolean
     */
    public boolean testJump2Boot() {
        if (!create("/dev/ttyUSB1")) {
            Log.e("test", "open the ets device failed");
            return false;
        }

        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
            return false;
        }

        Log.i("test", "do loopback");
        int retry = 3;
        while (retry > 0) {
            if (loopback()) {
                break;
            } else {
                --retry;
            }
        }

        if (retry == 0) {
            close();
            return false;
        }

        Log.i("test", "do jump to boot mode");
        if (!jump2load(CBPMode.Boot, true)) {
            close();
            return false;
        }

        Log.i("test", "test end with successful");
        close();
        return true;
    }

    /**
     * start download a image to cbp.
     * @param pathDev String
     * @param pathsImg List<String>
     */
    public void start(String pathDev, List<String> pathsImg) {

        // check the filename and get section
        mCallback.onProcess(DnldStatus.Readying, 0, "Checking the image file");
        if (pathsImg == null || pathsImg.isEmpty()) {
            Log.e("via_ets", "no image filenames");
        }
        mImgFiles = pathsImg;

        // create the ets port device
        mCallback.onProcess(DnldStatus.Readying, 0, "Openning the ets device");
        if (!create(pathDev)) {
            Log.e("via_ets", "Open the ets device failed");
        }

        // create the thread and start it
        mDnlderThr = new EtsDnlderThread(this);
        mDnlderThr.start();
    }

    /**
     * boot or cp or error.
     * @return 0:no response, 1:boot, 2:cp
     */
    public CBPMode checkMode() {
        Log.v("via_ets", "do check mode");

        short id = 0x00C8;
        /*
         * EtsMsg msg = sendAndWait(new EtsMsg(id, null), (short) 0xFFFF, 2000);
         * CBPMode ret = CBPMode.Unknown; if(msg!=null){ if (msg.getId()==id){
         * ret = CBPMode.CP; } else { ret = CBPMode.Boot; } }
         */

        EtsMsg msg = sendAndWait(new EtsMsg(id, null), id, 1000);
        CBPMode ret = CBPMode.CP;
        if (msg == null) {
            ret = CBPMode.Boot;
        }

        Log.v("via_ets", ret.name() + " mode");
        return ret; // boot
    }

    /**
     * jump to loader, into boot state.
     * @param toMode CBPMode
     * @param openAgain boolean
     * @return boolean
     */
    public boolean jump2load(CBPMode toMode, boolean openAgain) {
        Log.v("via_ets", "do jump to loader to " + toMode.name());

        CBPMode curMode = checkMode();
        if (curMode == CBPMode.Unknown) {
            Log.e("via_ets", "no response from device?");
            return false;
        }

        if (curMode == toMode) {
            return true;
        }

        short id = 0x00DC; // jump to loader
        write(new EtsMsg(id, null));
        destroy();

        if (openAgain) {
            closeDevice();

            Log.v("via_ets", "wait to open device in " + toMode + " mode");
            try {
                Thread.sleep(toMode == CBPMode.Boot ? 8000 : 6000);
            } catch (InterruptedException e) {
                e.printStackTrace();
                return false;
            }

            // open it again
            try {
                openDevice(null);
            } catch (SecurityException e) {
                Log.e("via_ets", "create port failed");
                e.printStackTrace();
                return false;
            } catch (InvalidParameterException e) {
                Log.e("via_ets", "create port failed");
                e.printStackTrace();
                return false;
            } catch (IOException e) {
                Log.e("via_ets", "create port failed");
                e.printStackTrace();
                return false;
            }

            if (toMode == CBPMode.Boot) {
                id = 0x00E0; // boot to loader
                EtsMsg msg = waitForMsg(id, 5 * 1000);
                if (msg == null) {
                    Log.e("via_ets", "check boot2load msg failed, try to check the mode");
                    if (checkMode() == CBPMode.Boot) {
                        return true;
                    }
                    Log.e("via_ets", "the cbp is not in boot mode");
                    return false;
                }
            }
        }

        return true;
    }

    /**
     * erase the flash section.
     * @param flashSection byte
     * @return boolean
     */
    public boolean eraseFlash(byte flashSection) {
        short id = 0x04B1;
        EtsMsg msg = sendAndWait(new EtsMsg(id, new byte[] { flashSection }), id,
                flashSection == 0 ? 30000 : 500000);
        if (msg == null) {
            Log.e("via_ets", "erase flash time out");
            return false;
        }
        Log.i("via_ets", "erase flash success");
        return true;
    }

    /**
     * download file into flash.
     * @param flashSection byte
     * @param imgPath String
     * @return boolean
     */
    public boolean downloadFlash(byte flashSection, String imgPath) {
        boolean ret = true;

        try {
            File f = new File(imgPath);
            byte[] dataImg = new byte[(int) f.length()];

            FileInputStream fileImg = new FileInputStream(f);
            int sizeImg = fileImg.read(dataImg);
            if (sizeImg != dataImg.length) {
                mCallback.onProcess(DnldStatus.Error, 0, "Read image file failed");
                fileImg.close();
                return false;
            }

            fileImg.close();

            // get length and checksum of the image file
            mCallback.onProcess(DnldStatus.Readying, 0, "Computing the checksum of image");
            byte[] imgLength = EtsUtil.int2bytes(sizeImg);
            byte[] imgChecksum = EtsUtil.int2bytes(EtsUtil.checkSum2(dataImg));

            byte[] imgInfo = new byte[8];
            int index = 0;
            for (byte b : imgLength) {
                imgInfo[index++] = b;
            }
            for (byte b : imgChecksum) {
                imgInfo[index++] = b;
            }

            // begin the process of download
            int seqToWrite = 0;

            // init flash
            mCallback.onProcess(DnldStatus.Erasing, 0, "Erasing flash, section="
                    + EtsDnlder.sSectionName[flashSection]);
            EtsMsg msg = sendAndWait(new EtsMsgProgCmd(++seqToWrite, EtsMsgProgCmd.sEarse,
                    flashSection, imgInfo), EtsMsgProgCmd.sID, flashSection == 0 ? 30000 : 500000);
            if (msg == null) {
                Log.e("via_ets", "erase flash time out");
                return false;
            }
            Log.i("via_ets", "erase flash success");

            // write flash
            mCallback.onProcess(DnldStatus.Downloading, 0, "Downloading flash, section="
                    + EtsDnlder.sSectionName[flashSection]);

            byte[] block = new byte[mSizeOnePackage];

            int numBlocks = (int) (sizeImg / mSizeOnePackage + 2); // +1 for
                                                                   // program
                                                                   // write
                                                                   // sequence
                                                                   // begin with
                                                                   // 2
            int seqAck = 0;

            index = 0;
            do {

                int size = sizeImg - index > mSizeOnePackage ? mSizeOnePackage : sizeImg - index;

                if (size > 0) {
                    // get data
                    byte[] data = block;
                    if (size < mSizeOnePackage) {
                        data = new byte[size];
                    }

                    for (int i = 0; i < size; ++i) {
                        data[i] = dataImg[index++];
                    }

                    write(new EtsMsgProgCmd(
                            ++seqToWrite, EtsMsgProgCmd.sWaite, flashSection, data));
                }

                if (seqToWrite - seqAck > 3 || size <= 0) { // window size is 3
                    msg = waitForMsg(EtsMsgProgCmd.sID, 2000);
                    if (msg == null) {
                        Log.e("via_ets", "don't get response");
                        ret = false;
                        break;
                    }

                    seqAck = msg.getProgRspSequence();
                    if (msg.getProgRspAck() == 0x01) {
                        Log.e("via_ets", "get NAK!");
                        ret = false;
                        break;
                    }
                }

                mCallback.onProcess(DnldStatus.Downloading, (seqAck * 100) / numBlocks,
                        "Downloading flash " + sSectionName[flashSection]);

            } while (seqToWrite > seqAck);

        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        return ret;
    }
}
