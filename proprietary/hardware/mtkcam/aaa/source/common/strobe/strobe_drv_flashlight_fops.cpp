/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "StrobeDrvFlashlightFops"

/***********************************************************
 * Headers
 **********************************************************/
#include <fcntl.h>
#include <cutils/atomic.h>
#include <string.h>

/* strobe headers */
#include "strobe_drv_flashlight_fops.h"
#include "strobe_utils.h"

/* kernel headers */
#include "flashlight.h"


/***********************************************************
 * Macros
 **********************************************************/
/* strobe device file */
#define STROBE_DEV_NAME "/dev/flashlight"


/***********************************************************
 * Member Functions
 **********************************************************/
StrobeDrvFlashlightFops::StrobeDrvFlashlightFops()
    : mStrobeHandle(-1)
    , mUsers(0)
{
}

StrobeDrvFlashlightFops *StrobeDrvFlashlightFops::getInstance()
{
    static StrobeDrvFlashlightFops singleton;
    return &singleton;
}

int StrobeDrvFlashlightFops::openkd()
{
    Mutex::Autolock lock(mLock);

    if (!mUsers) {
        /* open procedure */
        setDebug();

        mStrobeHandle = open(STROBE_DEV_NAME, O_RDWR);

        if (mStrobeHandle < 0) {
            logE("openkd(): failed to open device file(%s) error(%s).",
                    STROBE_DEV_NAME, strerror(errno));
            return -1;
        }
    }
    android_atomic_inc(&mUsers);

    logI("openkd(): mUsers(%d).", mUsers);

    return 0;
}

int StrobeDrvFlashlightFops::closekd()
{
    Mutex::Autolock lock(mLock);

    if (mUsers <= 0) {
        logI("closekd(): error mUsers(%d).", mUsers);
        return -1;
    }

    if (mUsers == 1) {
        /* close procedure */
        if (mStrobeHandle > 0)
            close(mStrobeHandle);
        mStrobeHandle = -1;
    }
    android_atomic_dec(&mUsers);

    logI("closekd(): mUsers(%d).", mUsers);

    return 0;
}

int StrobeDrvFlashlightFops::sendCommand(int cmd, int typeId, int ctId, int arg)
{
    Mutex::Autolock lock(mLock);

    if (mStrobeHandle < 0) {
        logD("sendCommandRet(): failed with no strobe handler.");
        return -1;
    }

    /* setup arguments */
    struct flashlight_user_arg strobeArg;
    strobeArg.type_id = typeId;
    strobeArg.ct_id = ctId;
    strobeArg.arg = arg;

    /* send ioctl */
    int ret = ioctl(mStrobeHandle, cmd, &strobeArg);
    if (ret == -1)
        logI("ioctl failed: type/ct (%d,%d) %s", typeId, ctId, strerror(errno));

    logD("Send command: type/ct(%d,%d), cmd(%d).", typeId, ctId, cmd);

    return ret;
}

int StrobeDrvFlashlightFops::sendCommandRet(int cmd, int typeId, int ctId, int *arg)
{
    Mutex::Autolock lock(mLock);

    if (mStrobeHandle < 0) {
        logD("sendCommandRet(): failed with no strobe handler.");
        return -1;
    }

    /* setup arguments */
    struct flashlight_user_arg strobeArg;
    strobeArg.type_id = typeId;
    strobeArg.ct_id = ctId;
    strobeArg.arg = *arg;

    /* send ioctl */
    int ret = ioctl(mStrobeHandle, cmd, &strobeArg);
    if (!ret)
        *arg = strobeArg.arg;
    else
        logI("ioctl failed: type/ct (%d,%d) %s", typeId, ctId, strerror(errno));

    logD("sendCommandRet(): type/ct(%d,%d), cmd/arg(%d,%d).",
            typeId, ctId, cmd, *arg);

    return ret;
}

