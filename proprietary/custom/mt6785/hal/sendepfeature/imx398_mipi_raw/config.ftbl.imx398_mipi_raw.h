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

//#ifndef _MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBL__H_
//#define _MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBL__H_
#if 1
//


/*******************************************************************************
 *
 ******************************************************************************/
FTABLE_DEFINITION(SENSOR_DRVNAME_IMX398_MIPI_RAW)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_INDEP()
    //==========================================================================
#if 1
    //  Scene Mode
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_SCENE_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::SCENE_MODE_AUTO),
            ITEM_AS_VALUES_(
                MtkCameraParameters::SCENE_MODE_AUTO,
//                MtkCameraParameters::SCENE_MODE_NORMAL,
                MtkCameraParameters::SCENE_MODE_PORTRAIT,
                MtkCameraParameters::SCENE_MODE_LANDSCAPE,
                MtkCameraParameters::SCENE_MODE_NIGHT,
                MtkCameraParameters::SCENE_MODE_NIGHT_PORTRAIT,
                MtkCameraParameters::SCENE_MODE_THEATRE,
                MtkCameraParameters::SCENE_MODE_BEACH,
                MtkCameraParameters::SCENE_MODE_SNOW,
                MtkCameraParameters::SCENE_MODE_SUNSET,
                MtkCameraParameters::SCENE_MODE_STEADYPHOTO,
                MtkCameraParameters::SCENE_MODE_FIREWORKS,
                MtkCameraParameters::SCENE_MODE_SPORTS,
                MtkCameraParameters::SCENE_MODE_PARTY,
                MtkCameraParameters::SCENE_MODE_CANDLELIGHT,
                MtkCameraParameters::SCENE_MODE_HDR,
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  Effect
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_EFFECT),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::EFFECT_NONE),
            ITEM_AS_VALUES_(
                MtkCameraParameters::EFFECT_NONE,
                MtkCameraParameters::EFFECT_MONO,
                MtkCameraParameters::EFFECT_NEGATIVE,
                MtkCameraParameters::EFFECT_SEPIA,
                MtkCameraParameters::EFFECT_AQUA,
                MtkCameraParameters::EFFECT_WHITEBOARD,
                MtkCameraParameters::EFFECT_BLACKBOARD,
                MtkCameraParameters::EFFECT_POSTERIZE,
                MtkCameraParameters::EFFECT_NASHVILLE,
                MtkCameraParameters::EFFECT_HEFE,
                MtkCameraParameters::EFFECT_VALENCIA ,
                MtkCameraParameters::EFFECT_XPROII ,
                MtkCameraParameters::EFFECT_LOFI,
                MtkCameraParameters::EFFECT_SIERRA ,
//                MtkCameraParameters::EFFECT_KELVIN ,
                MtkCameraParameters::EFFECT_WALDEN ,
//                MtkCameraParameters::EFFECT_F1977 ,
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  Picture Size (Both width & height must be 16-aligned)
    //  For CTS: the largest preview-size must have same aspect ratio as the largest picture-size
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_PICTURE_SIZE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("2560x1920"),
            ITEM_AS_VALUES_(
                "320x240",      "640x480",      "720x480",      "1024x768",     "1280x720",     "1280x960",
                "1600x1200",    "1920x1088",    "2048x1536",    "2560x1440",    "2560x1920",    "2880x1728",
                "3264x2448",    "3328x1872",    "3600x2160",    "4608x3456"
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  Preview Size
    //  For CTS: the largest preview-size must have same aspect ratio as the largest picture-size
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_PREVIEW_SIZE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("640x480"),
            ITEM_AS_VALUES_(
                "176x144",      "320x240",      "352x288",      "480x320",      "480x368",
                "640x480",      "720x480",      "800x480",      "864x480",
                "960x540",      "1280x720",     "1440x1080",    "1920x1080",    "1664x1248",
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  Video Size
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_SIZE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("640x480"),
            ITEM_AS_VALUES_(
                "176x144",      "320x240",      "352x288",     "480x320",      "640x480",
                "864x480",      "1280x720",     "1280x960",    "1920x1080",    "1920x1088",
                "3840x2176",    "4096x2160"
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  High Speed Video Size
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_HSVR_PRV_SIZE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("1280x720"),
            ITEM_AS_VALUES_(
                "0x0",
                "1280x720",
                "0x0",
                "0x0",
            )
        ),
    )

    //==========================================================================

    //  High Speed Video FPS
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_HSVR_PRV_FPS),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("60"),
            ITEM_AS_VALUES_(
                "0",
                "60",
                "0",
                "0",
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  Preview Frame Rate Range
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_PREVIEW_FPS_RANGE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("5000,30000"),
            ITEM_AS_USER_LIST_(
                "(15000,15000)",
                "(20000,20000)",
                "(24000,24000)",
                "(5000,30000)",
                "(30000,30000)",
#if (1 == SLOW_MOTION_VIDEO_SUPPORTED)
                "(60000,60000)",
                "(120000,120000)",
#endif
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  Exposure Compensation
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_EXPOSURE_COMPENSATION),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("0"),
            ITEM_AS_USER_LIST_(
                "-3",       //min exposure compensation index
                "3",        //max exposure compensation index
                "1.0",      //exposure compensation step; EV = step x index
            )
        ),
        //......................................................................
        #if 1   //  SCENE HDR
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_HDR,
            ITEM_AS_DEFAULT_("0"),
            ITEM_AS_USER_LIST_(
                "0",        //min exposure compensation index
                "0",        //max exposure compensation index
                "1.0",      //exposure compensation step; EV = step x index
            )
        )
        #endif
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  Anti-banding (Flicker)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_ANTIBANDING),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::ANTIBANDING_AUTO),
            ITEM_AS_VALUES_(
                MtkCameraParameters::ANTIBANDING_OFF,
                MtkCameraParameters::ANTIBANDING_50HZ,
                MtkCameraParameters::ANTIBANDING_60HZ,
                MtkCameraParameters::ANTIBANDING_AUTO,
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  Video Snapshot
#if (1 == VSS_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::TRUE),
        ),
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE),
        ),
    )
#endif
#endif
    //==========================================================================
#if 1
    //  DNG
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_DNG_SUPPORTED),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::TRUE),
        ),
    )
#endif
    //==========================================================================
#if (1 == DUAL_ZOOM_SUPPORTED)
    //  Zoom
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_ZOOM),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("0"),  //Zoom Index
            ITEM_AS_USER_LIST_(
                //Zoom Ratio
                "100", "101", "102", "104", "105", "106", "107", "108", "110", "111",
                "112", "114", "115", "116", "117", "119", "120", "122", "123", "124",
                "126", "127", "129", "130", "132", "133", "135", "136", "138", "140",
                "141", "143", "145", "146", "148", "150", "151", "153", "155", "157",
                "158", "160", "162", "164", "166", "168", "170", "172", "174", "176",
                "178", "180", "182", "184", "186", "188", "191", "193", "195", "197",
                "200", "202", "204", "207", "209", "211", "214", "216", "219", "221",
                "224", "226", "229", "232", "234", "237", "240", "243", "245", "248",
                "251", "254", "257", "260", "263", "266", "269", "272", "275", "279",
                "282", "285", "288", "292", "295", "299", "302", "305", "309", "313",
                "316", "320", "324", "327", "331", "335", "339", "343", "347", "351",
                "355", "359", "363", "367", "372", "376", "380", "385", "389", "394",
                "398", "403", "407", "412", "417", "422", "427", "432", "437", "442",
                "447", "452", "457", "462", "468", "473", "479", "484", "490", "495",
                "501", "507", "513", "519", "525", "531", "537", "543", "550", "556",
                "562", "569", "575", "582", "589", "596", "603", "610", "617", "624",
                "631", "638", "646", "653", "661", "668", "676", "684", "692", "700",
                "708", "716", "724", "733", "741", "750", "759", "767", "776", "785",
                "794", "804", "813", "822", "832", "841", "851", "861", "871", "881",
                "891", "902", "912", "923", "933", "944", "955", "966", "977", "989",
                "1000",
            )
        ),
    )
    //  dual Zoom for main2 camera
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_SUPCAM_ZOOM_RATIO),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("2"), // main2 camera id
            ITEM_AS_USER_LIST_(
                //Zoom Ratio
                "106", "106", "106", "106", "106", "106", "106", "106", "106", "106",
                "106", "106", "106", "106", "106", "106", "106", "106", "106", "106",
                "106", "106", "106", "106", "106", "106", "106", "106", "106", "106",
                "106", "106", "106", "106", "106", "106", "106", "106", "106", "106",
                "106", "106", "106", "106", "106", "106", "106", "106", "106", "106",
                "106", "106", "106", "106", "106", "106", "106", "106", "106", "106",
                "106", "108", "109", "110", "111", "113", "114", "115", "117", "118",
                "119", "121", "122", "124", "125", "126", "128", "129", "131", "132",
                "134", "136", "137", "139", "140", "142", "144", "145", "147", "149",
                "150", "152", "154", "156", "157", "159", "161", "163", "165", "167",
                "169", "171", "173", "175", "177", "179", "181", "183", "185", "187",
                "189", "191", "194", "196", "198", "200", "203", "205", "207", "210",
                "212", "215", "217", "220", "222", "225", "227", "230", "233", "235",
                "238", "241", "244", "247", "249", "252", "255", "258", "261", "264",
                "267", "270", "274", "277", "280", "283", "286", "290", "293", "296",
                "300", "303", "307", "310", "314", "318", "321", "325", "329", "333",
                "336", "340", "344", "348", "352", "356", "361", "365", "369", "373",
                "378", "382", "386", "391", "395", "400", "405", "409", "414", "419",
                "424", "429", "433", "438", "444", "449", "454", "459", "464", "470",
                "475", "481", "486", "492", "498", "503", "509", "515", "521", "527",
                "533",
            )
        ),
    )
#else
#if 1
    //  Zoom
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_ZOOM),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("0"),  //Zoom Index
            ITEM_AS_USER_LIST_(
                //Zoom Ratio
                "100", "114", "132", "151", "174",
                "200", "229", "263", "303", "348",
                "400",
            )
        ),
    )
#endif
#endif
    //==========================================================================
#if 1
    //  Zsd
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_ZSD_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::ON),
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF,
                MtkCameraParameters::ON
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  (Shot) Capture Mode
    if(facing == 1)
    {
       FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
           KEY_AS_(MtkCameraParameters::KEY_CAPTURE_MODE),
           SCENE_AS_DEFAULT_SCENE(
               ITEM_AS_DEFAULT_(MtkCameraParameters::CAPTURE_MODE_NORMAL),
               ITEM_AS_VALUES_(
                   MtkCameraParameters::CAPTURE_MODE_NORMAL,
                   MtkCameraParameters::CAPTURE_MODE_CONTINUOUS_SHOT,
                   #if (1 == SMILE_SHOT_SUPPORTED)
                   MtkCameraParameters::CAPTURE_MODE_SMILE_SHOT,
                   #endif
                   MtkCameraParameters::CAPTURE_MODE_BEST_SHOT,
                   #if (1 == ASD_SHOT_SUPPORTED)
                   MtkCameraParameters::CAPTURE_MODE_ASD_SHOT,
                   #endif
                   #if (1 == MOTION_TRACK_SUPPORTED)
                   MtkCameraParameters::CAPTURE_MODE_MOTION_TRACK_SHOT,
                   #endif
               )
           ),
       )
    }
    else
    {
          FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_CAPTURE_MODE),
        SCENE_AS_DEFAULT_SCENE(
               ITEM_AS_DEFAULT_(MtkCameraParameters::CAPTURE_MODE_NORMAL),
               ITEM_AS_VALUES_(
                   MtkCameraParameters::CAPTURE_MODE_NORMAL,
                   MtkCameraParameters::CAPTURE_MODE_CONTINUOUS_SHOT,
                   #if (1 == SMILE_SHOT_SUPPORTED)
                   MtkCameraParameters::CAPTURE_MODE_SMILE_SHOT,
                   #endif
                   MtkCameraParameters::CAPTURE_MODE_BEST_SHOT,
                   #if (1 == AUTORAMA_SUPPORTED)
                   MtkCameraParameters::CAPTURE_MODE_AUTO_PANORAMA_SHOT,
                   #endif
                   #if (1 == MAV_SUPPORTED)
                   MtkCameraParameters::CAPTURE_MODE_MAV_SHOT,
                   #endif
                   #if (1 == ASD_SHOT_SUPPORTED)
                   MtkCameraParameters::CAPTURE_MODE_ASD_SHOT,
                   #endif
                   #if (1 == MOTION_TRACK_SUPPORTED)
                   MtkCameraParameters::CAPTURE_MODE_MOTION_TRACK_SHOT,
                   #endif
               )
           ),
       )
    }
#endif
    //==========================================================================
#if 1

    //  Video Hdr

#if (1 == VHDR_SUPPORTED)

    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_HDR),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF),
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF,
                MtkCameraParameters::ON,
            )
        ),
    )
#else

    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_HDR),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF),
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF
            )
        ),
    )
#endif
#endif
    //==========================================================================
#if 1
    //  Video Hdr Mode
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_HDR_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::VIDEO_HDR_MODE_MVHDR),
            ITEM_AS_VALUES_(
                MtkCameraParameters::VIDEO_HDR_MODE_MVHDR,
            )
        ),
    )
#endif
    //==========================================================================
    //  Video Face Beauty
#if (1 == VIDEO_FACEBEAUTY_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_FACE_BEAUTY),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE),
            ITEM_AS_SUPPORTED_(
                MtkCameraParameters::FALSE
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  MFB
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_MFB_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF),
            ITEM_AS_VALUES_(
#if (0 == MFB_SUPPORTED)
                MtkCameraParameters::OFF,
#else
                MtkCameraParameters::OFF,
                MtkCameraParameters::KEY_MFB_MODE_MFLL,
                MtkCameraParameters::KEY_MFB_MODE_AIS,
                MtkCameraParameters::KEY_MFB_MODE_AUTO,
#endif
            )
        ),
    )
#endif
    //==========================================================================
#if 1
    //  Slow Motion
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_HSVR_SIZE_FPS),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("640x480x120"),
            ITEM_AS_VALUES_(
                "640x480x120"
            )
        ),
    )
#endif
    //==========================================================================
END_FTABLE_SCENE_INDEP()
//------------------------------------------------------------------------------
/*******************************************************************************
 *
 ******************************************************************************/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_DEP()
    //==========================================================================
#if 1
    //  Focus Mode
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FOCUS_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FOCUS_MODE_AUTO),
            ITEM_AS_VALUES_(
                MtkCameraParameters::FOCUS_MODE_AUTO,
                MtkCameraParameters::FOCUS_MODE_MACRO,
                MtkCameraParameters::FOCUS_MODE_INFINITY,
                MtkCameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE,
                MtkCameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO,
                "manual",   "fullscan",
            )
        ),
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISO
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_ISO_SPEED),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("auto"),
            ITEM_AS_VALUES_(
                "auto",
            )
        ),
        //......................................................................
        #if 1   //  SCENE AUTO
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_AUTO,
            ITEM_AS_DEFAULT_("auto"),
            ITEM_AS_VALUES_(
                "auto", "100", "200", "400", "800", "1600",
            )
        )
        #endif
        //......................................................................
        #if 1   //  SCENE NORMAL
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_NORMAL,
            ITEM_AS_DEFAULT_("auto"),
            ITEM_AS_VALUES_(
                "auto", "100", "200", "400", "800", "1600",
            )
        )
        #endif
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  White Balance.
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_WHITE_BALANCE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::WHITE_BALANCE_AUTO),
            ITEM_AS_VALUES_(
                MtkCameraParameters::WHITE_BALANCE_AUTO,            MtkCameraParameters::WHITE_BALANCE_INCANDESCENT,
                MtkCameraParameters::WHITE_BALANCE_FLUORESCENT,     MtkCameraParameters::WHITE_BALANCE_WARM_FLUORESCENT,
                MtkCameraParameters::WHITE_BALANCE_DAYLIGHT,        MtkCameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT,
                MtkCameraParameters::WHITE_BALANCE_TWILIGHT,        MtkCameraParameters::WHITE_BALANCE_SHADE,
            )
        ),
        //......................................................................
        #if 1   //  SCENE LANDSCAPE
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_LANDSCAPE,
            ITEM_AS_DEFAULT_(MtkCameraParameters::WHITE_BALANCE_DAYLIGHT),
            ITEM_AS_VALUES_(
                MtkCameraParameters::WHITE_BALANCE_AUTO,            MtkCameraParameters::WHITE_BALANCE_INCANDESCENT,
                MtkCameraParameters::WHITE_BALANCE_FLUORESCENT,     MtkCameraParameters::WHITE_BALANCE_WARM_FLUORESCENT,
                MtkCameraParameters::WHITE_BALANCE_DAYLIGHT,        MtkCameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT,
                MtkCameraParameters::WHITE_BALANCE_TWILIGHT,        MtkCameraParameters::WHITE_BALANCE_SHADE,
            )
        )
        #endif
        //......................................................................
        #if 1   //  SCENE SUNSET
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_SUNSET,
            ITEM_AS_DEFAULT_(MtkCameraParameters::WHITE_BALANCE_DAYLIGHT),
            ITEM_AS_VALUES_(
                MtkCameraParameters::WHITE_BALANCE_AUTO,            MtkCameraParameters::WHITE_BALANCE_INCANDESCENT,
                MtkCameraParameters::WHITE_BALANCE_FLUORESCENT,     MtkCameraParameters::WHITE_BALANCE_WARM_FLUORESCENT,
                MtkCameraParameters::WHITE_BALANCE_DAYLIGHT,        MtkCameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT,
                MtkCameraParameters::WHITE_BALANCE_TWILIGHT,        MtkCameraParameters::WHITE_BALANCE_SHADE,
            )
        )
        #endif
        //......................................................................
        #if 1   //  SCENE CANDLELIGHT
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_CANDLELIGHT,
            ITEM_AS_DEFAULT_(MtkCameraParameters::WHITE_BALANCE_INCANDESCENT),
            ITEM_AS_VALUES_(
                MtkCameraParameters::WHITE_BALANCE_AUTO,            MtkCameraParameters::WHITE_BALANCE_INCANDESCENT,
                MtkCameraParameters::WHITE_BALANCE_FLUORESCENT,     MtkCameraParameters::WHITE_BALANCE_WARM_FLUORESCENT,
                MtkCameraParameters::WHITE_BALANCE_DAYLIGHT,        MtkCameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT,
                MtkCameraParameters::WHITE_BALANCE_TWILIGHT,        MtkCameraParameters::WHITE_BALANCE_SHADE,
            )
        )
        #endif
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Edge
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_EDGE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE),
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH,
            )
        ),
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Hue
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_HUE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE),
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH,
            )
        ),
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Saturation
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_SATURATION),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE),
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH,
            )
        ),
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Brightness
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_BRIGHTNESS),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE),
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH,
            )
        ),
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Contrast
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_CONTRAST),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE),
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH,
            )
        ),
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
END_FTABLE_SCENE_DEP()
//------------------------------------------------------------------------------
END_FTABLE_DEFINITION()


/*******************************************************************************
 *
 ******************************************************************************/
#endif
//#endif //_MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBL__H_

