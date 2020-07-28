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

#ifndef __TLPINPAD_LAYOUT_H__
#define __TLPINPAD_LAYOUT_H__

/** Layout of pinpad, define size and position of buttons and input area. */

/** Values for control buttons */
#define PAD_CORRECT_VAL  (10)
#define PAD_CANCEL_VAL   (11)
#define PAD_VALIDATE_VAL (12)
#define PAD_NOKEY_VAL    (0xFF)

/** Button types */
typedef enum {
    NOKEY = 0,
    PAD_NUM,        /**< Number */
    PAD_CORRECT,
    PAD_CANCEL,
    PAD_VALIDATE
} pinPadKeyType_t;

/** Button definition: location, type, value */
typedef struct {
    uint32_t        xLeft;
    uint32_t        xRight;
    uint32_t        yTop;
    uint32_t        yBottom;
    uint8_t         val;
    pinPadKeyType_t type;
} sPinPadKey_t;

/** buttons_array - Array of buttons used to find out pressed key
 *
 * Layout:
 * 0: Empty button in the beginning
 * 1-9: Position of buttons for 1-9
 * 10: Position of buttons for 0
 * 11: Position of buttons for PAD_CORRECT_VAL
 * 12: Position of buttons for PAD_CANCEL_VAL
 * 13: Position of buttons for PAD_VALIDATE_VAL
 */
#define PINPAD_SIZE       (13)

/** Text area definition: location, layout
 * The text area displays the stars (and blanks).
 */
typedef struct {
    uint32_t taLeft;
    uint32_t taRight;
    uint32_t taTop;
    uint32_t taBottom;
    uint32_t taMargin;  /**< space between stars */
    uint32_t taCharsize;/**< width of stars */
} sTextAreaLayout_t;

/** Layout definition including buttons and textarea */
typedef struct {
    sPinPadKey_t        buttons_array[PINPAD_SIZE+1];
    sTextAreaLayout_t   textarea;
} sLayout_t;

#endif /* #ifndef __TLPINPAD_LAYOUT_H__ */

