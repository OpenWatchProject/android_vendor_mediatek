/*
* Copyright (c) 2015 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _HDMI_TA_H_
#define _HDMI_TA_H_

#define TZ_TA_HDMI_NAME "HDMI TA"
#define TZ_TA_HDMI_UUID "eaf800b0-da1b-11e2-a28f-0800200c9a66"

typedef enum
{
    HDMI_TA_WRITE_REG = 0,
    HDMI_TA_DPI1_WRITE_REG,
    HDMI_TA_INSTALL_HDCP_KEY,
    HDMI_TA_LOAD_HDCP_KEY,
    HDMI_TA_GET_HDCP_AKSV,
    HDMI_TA_HDCP_ENC_EN,
    HDMI_TA_HDCP_RST,
    HDMI_TA_VID_UNMUTE,
    HDMI_TA_AUD_UNMUTE,
    HDMI_TA_PROTECT_HDMIREG,
    HDMI_TA_LOAD_ROM,
    HDMI_TA_HDCP_FAIL,
} HDMI_TA_SERVICE_CMD_T;

typedef enum {
    TA_RETURN_HDCP_STATE_ENC_EN = 0,
    TA_RETURN_HDCP_STATE_ENC_FAIL,
    TA_RETURN_HDCP_STATE_ENC_UNKNOWN
} TA_RETURN_HDMI_HDCP_STATE;

extern unsigned char vTaGetHdcpStatus(void);
extern unsigned char vTaGetHdmiStatus(void);

#endif
