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
#ifndef __TESTLIMIT_H__
#define __TESTLIMIT_H__

// Limit of factory shipment test
#define TLIMIT_TN_REVISION              ""
#define TLIMIT_NO_RST_WIA1              "1-3"
#define TLIMIT_TN_RST_WIA1              "RST_WIA1"
#define TLIMIT_LO_RST_WIA1              0x48
#define TLIMIT_HI_RST_WIA1              0x48
#define TLIMIT_NO_RST_WIA2              "1-4"
#define TLIMIT_TN_RST_WIA2              "RST_WIA2"
#define TLIMIT_LO_RST_WIA2              0x05
#define TLIMIT_HI_RST_WIA2              0x05

#define TLIMIT_NO_ASAX                  "1-7"
#define TLIMIT_TN_ASAX                  "ASAX"
#define TLIMIT_LO_ASAX                  1
#define TLIMIT_HI_ASAX                  254
#define TLIMIT_NO_ASAY                  "1-8"
#define TLIMIT_TN_ASAY                  "ASAY"
#define TLIMIT_LO_ASAY                  1
#define TLIMIT_HI_ASAY                  254
#define TLIMIT_NO_ASAZ                  "1-9"
#define TLIMIT_TN_ASAZ                  "ASAZ"
#define TLIMIT_LO_ASAZ                  1
#define TLIMIT_HI_ASAZ                  254

#define TLIMIT_NO_SNG_ST1               "2-3"
#define TLIMIT_TN_SNG_ST1               "SNG_ST1"
#define TLIMIT_LO_SNG_ST1               1
#define TLIMIT_HI_SNG_ST1               1

#define TLIMIT_NO_SNG_HX                "2-4"
#define TLIMIT_TN_SNG_HX                "SNG_HX"
#define TLIMIT_LO_SNG_HX                -8189
#define TLIMIT_HI_SNG_HX                8189

#define TLIMIT_NO_SNG_HY                "2-6"
#define TLIMIT_TN_SNG_HY                "SNG_HY"
#define TLIMIT_LO_SNG_HY                -8189
#define TLIMIT_HI_SNG_HY                8189

#define TLIMIT_NO_SNG_HZ                "2-8"
#define TLIMIT_TN_SNG_HZ                "SNG_HZ"
#define TLIMIT_LO_SNG_HZ                -8189
#define TLIMIT_HI_SNG_HZ                8189

#define TLIMIT_NO_SNG_ST2               "2-10"
#define TLIMIT_TN_SNG_ST2               "SNG_ST2"
#define TLIMIT_LO_SNG_ST2               0
#define TLIMIT_HI_SNG_ST2               0

#define TLIMIT_NO_SLF_ST1               "2-13"
#define TLIMIT_TN_SLF_ST1               "SLF_ST1"
#define TLIMIT_LO_SLF_ST1               1
#define TLIMIT_HI_SLF_ST1               1

#define TLIMIT_NO_SLF_RVHX              "2-14"
#define TLIMIT_TN_SLF_RVHX              "SLF_REVSHX"
#define TLIMIT_LO_SLF_RVHX              -30
#define TLIMIT_HI_SLF_RVHX              30

#define TLIMIT_NO_SLF_RVHY              "2-16"
#define TLIMIT_TN_SLF_RVHY              "SLF_REVSHY"
#define TLIMIT_LO_SLF_RVHY              -30
#define TLIMIT_HI_SLF_RVHY              30

#define TLIMIT_NO_SLF_RVHZ              "2-18"
#define TLIMIT_TN_SLF_RVHZ              "SLF_REVSHZ"
#define TLIMIT_LO_SLF_RVHZ              -400
#define TLIMIT_HI_SLF_RVHZ              -50

#define TLIMIT_NO_SLF_ST2               "2-20"
#define TLIMIT_TN_SLF_ST2               "SLF_ST2"
#define TLIMIT_LO_SLF_ST2               0
#define TLIMIT_HI_SLF_ST2               0

#endif /* __TESTLIMIT_H__ */
