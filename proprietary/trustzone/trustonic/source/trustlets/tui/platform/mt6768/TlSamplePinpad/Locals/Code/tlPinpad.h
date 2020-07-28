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

/** State of the pinpad */
typedef enum {
    ST_NONE = 0,    /**< No images were set */
    ST_INITIALIZED, /**< TUI session opened */
    ST_IMAGE_SET,   /**< The initial full screen image is set */
    ST_VALIDATED,   /**< Four characters entered and 'validate' pressed */
    ST_CANCELLED    /**< 'Cancel' pressed */
} transactionState_t, *transactionState_ptr;

/** Accepted number of characters in input field */
#define MAX_PIN_LENGTH (4)

/** Number of characters of PIN */
#define MIN_PIN_LENGTH (4)

