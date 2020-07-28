/******************************************************************************
 *
 * Copyright (c) 2015 Asahi Kasei Microdevices Corporation, Japan
 * All Rights Reserved.
 *
 * This software program is the proprietary program of Asahi Kasei Microdevices
 * Corporation("AKM") licensed to authorized Licensee under the respective
 * agreement between the Licensee and AKM only for use with AKM's electronic
 * compass IC.
 *
 * THIS SOFTWARE IS PROVIDED TO YOU "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABLITY, FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT OF
 * THIRD PARTY RIGHTS, AND WE SHALL NOT BE LIABLE FOR ANY LOSSES AND DAMAGES
 * WHICH MAY OCCUR THROUGH USE OF THIS SOFTWARE.
 *
 ******************************************************************************/
#ifndef INC_AKL_HELPER_H
#define INC_AKL_HELPER_H

#include "AKL_APIs.h"

/**
 * Dump specified AKL_SCL_PRMS object.
 *
 * When one of the arguments is #AKL_NULL, this function do nothing.
 *
 * @param[in] AKL_SCL_PRMS A AKL_SCL_PRMS object
 * @param[in] dumpS        A pointer of callback function which dump a string.
 * @param[in] dumpP        A pointer of callback function which dump a pointer
 *                         address.
 * @param[in] dumpI        A pointer of callback function which dump a int32 value.
 * @param[in] dumpF        A pointer of callback function which dump a float value.
 *
 * @return #AKMGR_SUCCESS is returned when the function success.
 * Otherwise #AKMGR_ERROR or other error code will return.
 */
#ifdef AKL_DEBUG
#define DUMP_AKL_OBJ(handle) \
    AKL_dumpObj((handle), &dispString, &dispPointer, &dispInt, \
                &dispFloat)
#else
#define DUMP_AKL_OBJ(handle)  NULL
#endif


int32_t AKL_dumpObj(
    struct AKL_SCL_PRMS *obj,
    void (*dumpS)(const char *str),
    void (*dumpP)(const char *desc, const void *addr),
    void (*dumpI)(const char *desc, const int16_t val),
    void (*dumpF)(const char *desc, const float val)
);

#endif /* INC_AKL_HELPER_H */
