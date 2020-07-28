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

#ifndef _MTK_CAMERA_FEATURE_PIPE_I_STREAMING_FEATURE_PIPE_VAR_H_
#define _MTK_CAMERA_FEATURE_PIPE_I_STREAMING_FEATURE_PIPE_VAR_H_


#define VAR_APP_MODE            "common.app.mode"
#define VAR_P1_TS               "common.p1.ts"
#define VAR_DEBUG_DUMP          "common.debug.dump"
#define VAR_DEBUG_DUMP_HINT     "common.debug.dump.hint"

#define VAR_EXTREME_VFB "vfb.ext"
#define VAR_PREV_HOLDER "prev_holder"
#define VAR_CURR_HOLDER "curr_holder"
#define VAR_PREV_RSSO "prev_rsso"
#define VAR_CURR_RSSO "curr_rsso"

#define VAR_EIS_SENSOR_SIZE     "eis.sensor.size"
#define VAR_EIS_SCALER_CROP     "eis.scaler.rect"
#define VAR_EIS_SCALER_SIZE     "eis.scaler.outsize"
#define VAR_EIS_GMV_X           "eis.gmv_x"
#define VAR_EIS_GMV_Y           "eis.gmv_y"
#define VAR_EIS_CONF_X          "eis.confX"
#define VAR_EIS_CONF_Y          "eis.confY"
#define VAR_EIS_EXP_TIME        "eis.expTime"
#define VAR_EIS_TIMESTAMP       "eis.timeStamp"
#define VAR_EIS_SENSOR_ID       "eis.sensor.id"
#define VAR_EIS_LONGEXP_TIME    "eis.LongExpTime"
#define VAR_EIS_RRZO_CROP       "eis.rrzo.crop"
#define VAR_EIS_FACTOR          "eis.factor"
#define VAR_EIS_LMV_DATA        "eis.lmv.data"

//IMGO2IMGI
#define VAR_IMGO_2IMGI_ENABLE   "imgo.2imgi.enabled"
#define VAR_IMGO_2IMGI_P1CROP   "imgo.2imgi.p1_scalar_crop"

#define VAR_DUAL_SLAVE_PARAM    "slave"
#define VAR_EIS_SKIP_RECORD     "eis.skip.record"
#define VAR_EIS_TSQ_TS          "eis.tsq.ts"

// DualCam SyncManager
#define VAR_DUALCAM_FRAME_NO    "dualcam.frameNo"
#define VAR_DUALCAM_TIMESTAMP   "dualcam.timestamp"
#define VAR_DUALCAM_DO_SYNC     "dualcam.doFrameSync"
#define VAR_DUALCAM_DROP_REQ    "dualcam.dropReq"

// DualCam for FOVNode
#define VAR_DUALCAM_DO_FOV          "dualcam.doFOV"
#define VAR_DUALCAM_FOV_RECT        "dualcam.fovRect"
#define VAR_DUALCAM_ZOOM_RATIO      "dualcam.zoomratio"
#define VAR_DUALCAM_FOV_SLAVE_PARAM "dualcam.slave_param"
#define VAR_DUALCAM_FOV_MASTER_ID   "dualcam.master_id"
#define VAR_DUALCAM_FOV_SLAVE_ID    "dualcam.slave_id"
#define VAR_DUALCAM_FOV_SENSOR_MARGIN "dualcam.fov.sensor.margin"
#define VAR_DUALCAM_FOV_RRZO_MARGIN   "dualcam.fov.rrz.margin"
#define VAR_DUALCAM_FOV_ONLINE "dualcam.fov.online"
#define VAR_DUALCAM_FOV_CALIB_INFO  "dualcam.fov.calibration.info"

#define VAR_P1RAW_TWIN_STATUS   "p1raw.twin.status"

// for 3DNR LMV
#define VAR_LMV_SWITCH_OUT_RST  "lmv.SwitchOutResult"
#define VAR_LMV_VALIDITY        "lmv.Validity"
// for 3DNR
#define VAR_3DNR_ISO_THRESHOLD   "3dnr.isoThreshold"
#define VAR_3DNR_GYRO            "3dnr.gyro"
#define VAR_3DNR_ISO             "3dnr.iso"
#define VAR_3DNR_EIS_IS_CRZ_MODE "3dnr.eis.isCRZMode"
#define VAR_3DNR_EIS_X_INT       "3dnr.eis.x_int"
#define VAR_3DNR_EIS_Y_INT       "3dnr.eis.y_int"
#define VAR_3DNR_EIS_GMVX        "3dnr.eis.gmvX"
#define VAR_3DNR_EIS_GMVY        "3dnr.eis.gmvY"
#define VAR_3DNR_EIS_CONFX       "3dnr.eis.confX"
#define VAR_3DNR_EIS_CONFY       "3dnr.eis.confY"

// for FSC
#define VAR_FSC_RRZO_CROP_REGION        "fsc.rrzo.crop.region"
#define VAR_FSC_RSSO_CROP_REGION        "fsc.rsso.crop.region"
#define VAR_FSC_RRZO_WARP_DATA          "fsc.rrzo.warp.data"
#define VAR_FSC_CROP_RESULT             "fsc.crop.result"

// for DualCam Denoise
#define VAR_DUALCAM_AF_STATE    "dualcam.afState"
#define VAR_DUALCAM_LENS_STATE  "dualcam.lensState"
#define VAR_DUALCAM_DO_N3D      "dualcam.doN3D"
#define VAR_DUALCAM_EXIF_ISO    "dualcam.exifISO"
// for N3D
#define VAR_N3D_ISAFTRIGGER     "n3d.isaftrigger"
#define VAR_N3D_SHOTMODE        "n3d.shotmode"

// VendorFOV
#define VAR_DUALCAM_P1_MAGICNUM "daulcam.p1.magic.num"

// DualFeatureNode
#define VAR_DUAL_FEATURE_PROCESS_DATA "dual.feature.process.data"
#define VAR_DUAL_FEATURE_INSTANCE "dual.feature.instance"

#endif // _MTK_CAMERA_FEATURE_PIPE_I_STREAMING_FEATURE_PIPE_VAR_H_
