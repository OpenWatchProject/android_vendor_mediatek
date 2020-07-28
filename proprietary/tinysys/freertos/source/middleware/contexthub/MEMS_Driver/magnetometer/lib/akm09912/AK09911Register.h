/******************************************************************************
 *
 *  $Id: $
 *
 * -- Copyright Notice --
 *
 * Copyright (c) 2004 Asahi Kasei Microdevices Corporation, Japan
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
 * -- End Asahi Kasei Microdevices Copyright Notice --
 *
 ******************************************************************************/
#ifndef __AK09911REGSTER_H__
#define __AK09911REGSTER_H__

// AK09911 register address
#define AK09911_REG_WIA1                    0x00
#define AK09911_REG_WIA2                    0x01
#define AK09911_REG_INFO1                   0x02
#define AK09911_REG_INFO2                   0x03

#define AK09911_REG_ST1                     0x10
#define AK09911_REG_HXL                     0x11
#define AK09911_REG_HXH                     0x12
#define AK09911_REG_HYL                     0x13
#define AK09911_REG_HYH                     0x14
#define AK09911_REG_HZL                     0x15
#define AK09911_REG_HZH                     0x16
#define AK09911_REG_TMPS                    0x17
#define AK09911_REG_ST2                     0x18

#define AK09911_REG_CNTL1                   0x30
#define AK09911_REG_CNTL2                   0x31
#define AK09911_REG_CNTL3                   0x32

#define AK09911_FUSE_ASAX                   0x60
#define AK09911_FUSE_ASAY                   0x61
#define AK09911_FUSE_ASAZ                   0x62

#define AK09911_CNTL2_SNG_MEASURE           0x01
#define AK09911_CNTL2_CONT_MEASURE_MODE1    0x02
#define AK09911_CNTL2_CONT_MEASURE_MODE2    0x04
#define AK09911_CNTL2_CONT_MEASURE_MODE3    0x06
#define AK09911_CNTL2_CONT_MEASURE_MODE4    0x08
#define AK09911_CNTL2_SELF_TEST             0x10
#define AK09911_CNTL2_FUSE_ACCESS           0x1F
#define AK09911_CNTL2_POWER_DOWN            0x00

#define AK09911_CNTL3_SOFT_RESET            0x01

#endif

