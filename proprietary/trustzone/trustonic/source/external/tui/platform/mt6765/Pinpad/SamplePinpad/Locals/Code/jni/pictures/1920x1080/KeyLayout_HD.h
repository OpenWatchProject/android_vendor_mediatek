/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef __KEY_LAYOUT_HD_H__
#define __KEY_LAYOUT_HD_H__

#include "tlPinpadLayout.h"

/* x/y coordinates for our pin pad as in pictures/1024x600/pinpad.png (determined manually).
 * Use any graphic tool to look up the coordinates of your image.
 */
#define PAD_COL1_LEFT_HD     (5)
#define PAD_COL1_RIGHT_HD    (355)
#define PAD_COL2_LEFT_HD     (365)
#define PAD_COL2_RIGHT_HD    (715)
#define PAD_COL3_LEFT_HD     (725)
#define PAD_COL3_RIGHT_HD    (1075)

/* /!\ yBottom > yTop */
#define PAD_LINE1_TOP_HD      (925)
#define PAD_LINE1_BOTTOM_HD   (1075)
#define PAD_LINE2_TOP_HD      (1090)
#define PAD_LINE2_BOTTOM_HD   (1240)
#define PAD_LINE3_TOP_HD      (1255)
#define PAD_LINE3_BOTTOM_HD   (1405)
#define PAD_LINE4_TOP_HD      (1420)
#define PAD_LINE4_BOTTOM_HD   (1570)

/* TODO: Change this part because the entryfield's shape has changed */
#define TEXTAREA_LEFT_HD      (96)
#define TEXTAREA_RIGHT_HD     (984)
#define TEXTAREA_TOP_HD       (575)
#define TEXTAREA_BOTTOM_HD    (725)
#define TEXTAREA_MARGIN_HD    (0)
#define TEXTAREA_CHARSIZE_HD  (246)

#define CANCEL_LEFT_HD       (5)
#define CANCEL_RIGHT_HD      (355)
#define CANCEL_TOP_HD        (1585)
#define CANCEL_BOTTOM_HD     (1735)

#define VALID_LEFT_HD        (725)
#define VALID_RIGHT_HD       (1075)
#define VALID_TOP_HD         (1585)
#define VALID_BOTTOM_HD      (1735)

static const sLayout_t pinpad_layout_hd = {
    /* sPinPadKey_t buttons_array[PINPAD_SIZE+1] */ {
  {0, 0, 0, 0, PAD_NOKEY_VAL, NOKEY},
  {PAD_COL1_LEFT_HD, PAD_COL1_RIGHT_HD, PAD_LINE1_TOP_HD, PAD_LINE1_BOTTOM_HD, 1,                PAD_NUM},
  {PAD_COL2_LEFT_HD, PAD_COL2_RIGHT_HD, PAD_LINE1_TOP_HD, PAD_LINE1_BOTTOM_HD, 2,                PAD_NUM},
  {PAD_COL3_LEFT_HD, PAD_COL3_RIGHT_HD, PAD_LINE1_TOP_HD, PAD_LINE1_BOTTOM_HD, 3,                PAD_NUM},
  {PAD_COL1_LEFT_HD, PAD_COL1_RIGHT_HD, PAD_LINE2_TOP_HD, PAD_LINE2_BOTTOM_HD, 4,                PAD_NUM},
  {PAD_COL2_LEFT_HD, PAD_COL2_RIGHT_HD, PAD_LINE2_TOP_HD, PAD_LINE2_BOTTOM_HD, 5,                PAD_NUM},
  {PAD_COL3_LEFT_HD, PAD_COL3_RIGHT_HD, PAD_LINE2_TOP_HD, PAD_LINE2_BOTTOM_HD, 6,                PAD_NUM},
  {PAD_COL1_LEFT_HD, PAD_COL1_RIGHT_HD, PAD_LINE3_TOP_HD, PAD_LINE3_BOTTOM_HD, 7,                PAD_NUM},
  {PAD_COL2_LEFT_HD, PAD_COL2_RIGHT_HD, PAD_LINE3_TOP_HD, PAD_LINE3_BOTTOM_HD, 8,                PAD_NUM},
  {PAD_COL3_LEFT_HD, PAD_COL3_RIGHT_HD, PAD_LINE3_TOP_HD, PAD_LINE3_BOTTOM_HD, 9,                PAD_NUM},
  {PAD_COL2_LEFT_HD, PAD_COL2_RIGHT_HD, PAD_LINE4_TOP_HD, PAD_LINE4_BOTTOM_HD, 0,                PAD_NUM},
  {PAD_COL3_LEFT_HD, PAD_COL3_RIGHT_HD, PAD_LINE4_TOP_HD, PAD_LINE4_BOTTOM_HD, PAD_CORRECT_VAL,  PAD_CORRECT},
  {CANCEL_LEFT_HD,   CANCEL_RIGHT_HD,   CANCEL_TOP_HD,    CANCEL_BOTTOM_HD,    PAD_CANCEL_VAL,   PAD_CANCEL},
  {VALID_LEFT_HD,    VALID_RIGHT_HD,    VALID_TOP_HD,     VALID_BOTTOM_HD,     PAD_VALIDATE_VAL, PAD_VALIDATE}
    },
    /* sTextAreaLayout_t textarea */ {
    TEXTAREA_LEFT_HD,
    TEXTAREA_RIGHT_HD,
    TEXTAREA_TOP_HD,
    TEXTAREA_BOTTOM_HD,
    TEXTAREA_MARGIN_HD,
    TEXTAREA_CHARSIZE_HD,
    }
};

#endif /* #ifndef __KEY_LAYOUT_HD_H__ */

