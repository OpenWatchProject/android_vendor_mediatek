/*
 * Copyright (c) 2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef __KEY_LAYOUT_QHD_H__
#define __KEY_LAYOUT_QHD_H__

#include "tlPinpadLayout.h"

/* x/y coordinates for our pin pad as in pictures/1024x600/pinpad.png (determined manually).
 * Use any graphic tool to look up the coordinates of your image.
 */
#define PAD_COL1_LEFT_QHD     (7)
#define PAD_COL1_RIGHT_QHD    (473)
#define PAD_COL2_LEFT_QHD     (487)
#define PAD_COL2_RIGHT_QHD    (953)
#define PAD_COL3_LEFT_QHD     (967)
#define PAD_COL3_RIGHT_QHD    (1433)

/* /!\ yBottom > yTop */
#define PAD_LINE1_TOP_QHD      (1233)
#define PAD_LINE1_BOTTOM_QHD   (1433)
#define PAD_LINE2_TOP_QHD      (1453)
#define PAD_LINE2_BOTTOM_QHD   (1653)
#define PAD_LINE3_TOP_QHD      (1673)
#define PAD_LINE3_BOTTOM_QHD   (1873)
#define PAD_LINE4_TOP_QHD      (1893)
#define PAD_LINE4_BOTTOM_QHD   (2093)

/* TODO: Change this part because the entryfield's shape has changed */
#define TEXTAREA_LEFT_QHD      (128)
#define TEXTAREA_RIGHT_QHD     (1312)
#define TEXTAREA_TOP_QHD       (767)
#define TEXTAREA_BOTTOM_QHD    (967)
#define TEXTAREA_MARGIN_QHD    (0)
#define TEXTAREA_CHARSIZE_QHD  (328)

#define CANCEL_LEFT_QHD       (7)
#define CANCEL_RIGHT_QHD      (473)
#define CANCEL_TOP_QHD        (2113)
#define CANCEL_BOTTOM_QHD     (2313)

#define VALID_LEFT_QHD        (967)
#define VALID_RIGHT_QHD       (1433)
#define VALID_TOP_QHD         (2113)
#define VALID_BOTTOM_QHD      (2313)

static const sLayout_t pinpad_layout_qhd = {
    /* sPinPadKey_t buttons_array[PINPAD_SIZE+1] */ {
  {0, 0, 0, 0, PAD_NOKEY_VAL, NOKEY},
  {PAD_COL1_LEFT_QHD, PAD_COL1_RIGHT_QHD, PAD_LINE1_TOP_QHD, PAD_LINE1_BOTTOM_QHD, 1,                PAD_NUM},
  {PAD_COL2_LEFT_QHD, PAD_COL2_RIGHT_QHD, PAD_LINE1_TOP_QHD, PAD_LINE1_BOTTOM_QHD, 2,                PAD_NUM},
  {PAD_COL3_LEFT_QHD, PAD_COL3_RIGHT_QHD, PAD_LINE1_TOP_QHD, PAD_LINE1_BOTTOM_QHD, 3,                PAD_NUM},
  {PAD_COL1_LEFT_QHD, PAD_COL1_RIGHT_QHD, PAD_LINE2_TOP_QHD, PAD_LINE2_BOTTOM_QHD, 4,                PAD_NUM},
  {PAD_COL2_LEFT_QHD, PAD_COL2_RIGHT_QHD, PAD_LINE2_TOP_QHD, PAD_LINE2_BOTTOM_QHD, 5,                PAD_NUM},
  {PAD_COL3_LEFT_QHD, PAD_COL3_RIGHT_QHD, PAD_LINE2_TOP_QHD, PAD_LINE2_BOTTOM_QHD, 6,                PAD_NUM},
  {PAD_COL1_LEFT_QHD, PAD_COL1_RIGHT_QHD, PAD_LINE3_TOP_QHD, PAD_LINE3_BOTTOM_QHD, 7,                PAD_NUM},
  {PAD_COL2_LEFT_QHD, PAD_COL2_RIGHT_QHD, PAD_LINE3_TOP_QHD, PAD_LINE3_BOTTOM_QHD, 8,                PAD_NUM},
  {PAD_COL3_LEFT_QHD, PAD_COL3_RIGHT_QHD, PAD_LINE3_TOP_QHD, PAD_LINE3_BOTTOM_QHD, 9,                PAD_NUM},
  {PAD_COL2_LEFT_QHD, PAD_COL2_RIGHT_QHD, PAD_LINE4_TOP_QHD, PAD_LINE4_BOTTOM_QHD, 0,                PAD_NUM},
  {PAD_COL3_LEFT_QHD, PAD_COL3_RIGHT_QHD, PAD_LINE4_TOP_QHD, PAD_LINE4_BOTTOM_QHD, PAD_CORRECT_VAL,  PAD_CORRECT},
  {CANCEL_LEFT_QHD,   CANCEL_RIGHT_QHD,   CANCEL_TOP_QHD,    CANCEL_BOTTOM_QHD,    PAD_CANCEL_VAL,   PAD_CANCEL},
  {VALID_LEFT_QHD,    VALID_RIGHT_QHD,    VALID_TOP_QHD,     VALID_BOTTOM_QHD,     PAD_VALIDATE_VAL, PAD_VALIDATE}
    },
    /* sTextAreaLayout_t textarea */ {
    TEXTAREA_LEFT_QHD,
    TEXTAREA_RIGHT_QHD,
    TEXTAREA_TOP_QHD,
    TEXTAREA_BOTTOM_QHD,
    TEXTAREA_MARGIN_QHD,
    TEXTAREA_CHARSIZE_QHD,
    }
};

#endif /* #ifndef __KEY_LAYOUT_QHD_H__ */

