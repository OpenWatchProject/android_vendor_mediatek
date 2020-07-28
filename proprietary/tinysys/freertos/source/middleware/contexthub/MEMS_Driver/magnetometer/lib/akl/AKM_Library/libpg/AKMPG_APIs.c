/******************************************************************************
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
#include "AKMPseudoGyro.h"
#include "AKMPG_APIs.h"
#include "AKMPG_Measure.h"

#define AKM_ACC_FMT             (1)
#define AKM_MAG_FMT             (1)

static AKSCPGPRMS g_prms;

int16 AKMPG_Init(void)
{
    AKSCPGPRMS *prms = (AKSCPGPRMS *)&g_prms;

    // Initialize AKSCPGPRMS structure.
    InitAKSC_PGPRMS(prms);

    // Init SmartCompass library functions.
    if(Init_PG_Measure(prms) != AKRET_PROC_SUCCEED){
        AKMERROR;
        return AKMD_ERROR;
    }

    return AKMD_SUCCESS;
}

void AKMPG_Release(void)
{
}

void AKMPG_Start(void)
{
}

void AKMPG_Stop(void)
{
}

// input
void AKMPG_SetMagData(
    const int16 mx,
    const int16 my,
    const int16 mz,
    const int16 mdt,
    const int16 mag_lv)
{
    AKSCPGPRMS *prms = (AKSCPGPRMS *)&g_prms;
    AKSC_FLOAT cvt;

    cvt = AKM_MAG_FMT;

    prms->m_hdata.u.x = (int16)(mx * cvt);
    prms->m_hdata.u.y = (int16)(my * cvt);
    prms->m_hdata.u.z = (int16)(mz * cvt);
    prms->m_hdst = mag_lv;

    prms->m_pgdt = mdt;

    /* Calculate angular rate */
    if (CalcAngularRate(prms) != AKRET_PROC_SUCCEED) {
        //AKMERROR;
    }
}

// [m/s^2]
void AKMPG_SetAccData(
        const int16 ix,
        const int16 iy,
        const int16 iz)
{
    AKSCPGPRMS *prms = (AKSCPGPRMS *)&g_prms;
    AKSC_FLOAT cvt;

    cvt = AKM_ACC_FMT;

    prms->m_avec.u.x = (int16)(ix * cvt);
    prms->m_avec.u.y = (int16)(iy * cvt);
    prms->m_avec.u.z = (int16)(iz * cvt);
}

void AKMPG_GetGyro(AKSC_FLOAT gyro_buf[3])
{
    AKSCPGPRMS *prms = (AKSCPGPRMS *)&g_prms;

    gyro_buf[0] = prms->m_pgout.u.x;
    gyro_buf[1] = prms->m_pgout.u.y;
    gyro_buf[2] = prms->m_pgout.u.z;

    return;
}

void AKMPG_GetRotationVector(AKSC_FLOAT rv_buf[5])
{
    AKSCPGPRMS *prms  = (AKSCPGPRMS *)&g_prms;
    int16 totalHDST = prms->m_hdst;
    float rotationEHA = 0.0f;

    if (rv_buf != NULL) {
        if (totalHDST == 3) {
            rotationEHA = (5.625f * DEG2RAD);
        } else if (totalHDST == 2) {
            rotationEHA = (11.25f * DEG2RAD);
        } else if (totalHDST == 1) {
            rotationEHA = (22.5f * DEG2RAD);
        } else {
            rotationEHA = (180.0f * DEG2RAD);
        }

        /* Rotation Vector Q14 Format */
        rv_buf[0] = prms->m_pgquat.u.x;
        rv_buf[1] = prms->m_pgquat.u.y;
        rv_buf[2] = prms->m_pgquat.u.z;
        rv_buf[3] = prms->m_pgquat.u.w;
        rv_buf[4] = rotationEHA;
    }

    return;
}

void AKMPG_GetGravity(float gravity_buf[3])
{
    AKSCPGPRMS *prms = (AKSCPGPRMS *)&g_prms;

    gravity_buf[0] = prms->m_pgGravity.u.x;
    gravity_buf[1] = prms->m_pgGravity.u.y;
    gravity_buf[2] = prms->m_pgGravity.u.z;

    return;
}

void AKMPG_GetLA(float la_buf[3])
{
    AKSCPGPRMS *prms = (AKSCPGPRMS *)&g_prms;

    la_buf[0] = prms->m_pgLinAcc.u.x;
    la_buf[1] = prms->m_pgLinAcc.u.y;
    la_buf[2] = prms->m_pgLinAcc.u.z;

    return;
}
