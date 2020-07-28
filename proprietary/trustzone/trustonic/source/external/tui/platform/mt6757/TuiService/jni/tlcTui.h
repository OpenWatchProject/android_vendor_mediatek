/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TLCTUI_H__
#define __TLCTUI_H__

#ifdef TUI_TEST
/* TEST ONLY TO SIMULATE TOUCH EVENT*/
/* ------------------------------------------------------------- */
#define PAD_CORRECT_VAL  (11)
#define PAD_CANCEL_VAL   (12)
#define PAD_VALIDATE_VAL (13)

typedef enum {
    NOKEY = 0,
    INPUT,
    PAD_NUM,
    PAD_CORRECT,
    PAD_CANCEL,
    PAD_VALIDATE
}pinPadKeyType_t;

typedef struct {
    uint32_t xLeft;
    uint32_t xRight;
    uint32_t yTop;
    uint32_t yBottom;
    uint8_t val;
    pinPadKeyType_t type;
}sPinPadKey_t;

/* x/y coordinates for our pin pad as in Pinpad_7inch_1024x600.png (determined manually).
 * Could use tools like www.image-maps.com for HTML image maps.
 */
#define PAD_COL1_LEFT     (64)
#define PAD_COL1_RIGHT    (214)
#define PAD_COL2_LEFT     (224)
#define PAD_COL2_RIGHT    (374)
#define PAD_COL3_LEFT     (384)
#define PAD_COL3_RIGHT    (534)

/* /!\ yBottom > yTop */
#define PAD_LINE1_TOP      (500)
#define PAD_LINE1_BOTTOM   (560)
#define PAD_LINE2_TOP      (570)
#define PAD_LINE2_BOTTOM   (630)
#define PAD_LINE3_TOP      (640)
#define PAD_LINE3_BOTTOM   (700)
#define PAD_LINE4_TOP      (710)
#define PAD_LINE4_BOTTOM   (770)

/* TODO: Change this part because the entryfield's shape has changed */
#define TEXTAREA_LEFT      (137)
#define TEXTAREA_RIGHT     (491)
#define TEXTAREA_TOP       (354)
#define TEXTAREA_BOTTOM    (414)
#define TEXTAREA_MARGIN    (3)

#define CANCEL_LEFT       (64)
#define CANCEL_RIGHT      (214)
#define CANCEL_TOP        (780)
#define CANCEL_BOTTOM     (840)
#define VALID_LEFT        (384)
#define VALID_RIGHT       (534)
#define VALID_TOP         (780)
#define VALID_BOTTOM      (840)

static const sPinPadKey_t buttons_array[] = {
    {   0, 0, 0, 0, 0, NOKEY},
    {   PAD_COL1_LEFT, PAD_COL1_RIGHT, PAD_LINE1_TOP, PAD_LINE1_BOTTOM, 1, PAD_NUM},
    {   PAD_COL2_LEFT, PAD_COL2_RIGHT, PAD_LINE1_TOP, PAD_LINE1_BOTTOM, 2, PAD_NUM},
    {   PAD_COL3_LEFT, PAD_COL3_RIGHT, PAD_LINE1_TOP, PAD_LINE1_BOTTOM, 3, PAD_NUM},
    {   PAD_COL1_LEFT, PAD_COL1_RIGHT, PAD_LINE2_TOP, PAD_LINE2_BOTTOM, 4, PAD_NUM},
    {   PAD_COL2_LEFT, PAD_COL2_RIGHT, PAD_LINE2_TOP, PAD_LINE2_BOTTOM, 5, PAD_NUM},
    {   PAD_COL3_LEFT, PAD_COL3_RIGHT, PAD_LINE2_TOP, PAD_LINE2_BOTTOM, 6, PAD_NUM},
    {   PAD_COL1_LEFT, PAD_COL1_RIGHT, PAD_LINE3_TOP, PAD_LINE3_BOTTOM, 7, PAD_NUM},
    {   PAD_COL2_LEFT, PAD_COL2_RIGHT, PAD_LINE3_TOP, PAD_LINE3_BOTTOM, 8, PAD_NUM},
    {   PAD_COL3_LEFT, PAD_COL3_RIGHT, PAD_LINE3_TOP, PAD_LINE3_BOTTOM, 9, PAD_NUM},
    {   PAD_COL3_LEFT, PAD_COL3_RIGHT, PAD_LINE4_TOP, PAD_LINE4_BOTTOM, PAD_CORRECT_VAL, PAD_CORRECT},
    {   PAD_COL2_LEFT, PAD_COL2_RIGHT, PAD_LINE4_TOP, PAD_LINE4_BOTTOM, 0, PAD_NUM},
    {   CANCEL_LEFT, CANCEL_RIGHT, CANCEL_TOP, CANCEL_BOTTOM, PAD_CANCEL_VAL, PAD_CANCEL},
    {   VALID_LEFT, VALID_RIGHT, VALID_TOP, VALID_BOTTOM, PAD_VALIDATE_VAL, PAD_VALIDATE}
};
/* ------------------------------------------------------------- */
#endif /* TUI_TEST */

bool tlcLaunch(void);
bool tlcOpen(void);
bool tlcProcessCmd(uint32_t);
bool tlcWaitCmdFromDriver(uint32_t *);
#ifdef TUI_TEST
uint32_t tlcSimulateTouchEvent(void);
#endif /* TUI_TEST */
bool tlcNotifyEvent(uint32_t eventType);
void tlcClose(void);

#endif /* __TLCTUI_H__ */

