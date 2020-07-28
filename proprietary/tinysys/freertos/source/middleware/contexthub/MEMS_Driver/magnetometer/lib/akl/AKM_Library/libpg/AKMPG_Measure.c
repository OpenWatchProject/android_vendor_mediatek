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

#include "AKMPG_Measure.h"

/*!
 Initialize #AKSCPGPRMS structure. At first, 0 is set to all parameters.
 After that, some parameters, which should not be 0, are set to specific
 value. Some of initial values can be customized by editing the file
 \c "CustomerSpec.h".
 @param[out] prms A pointer to #AKSCPGPRMS structure.
 */
void InitAKSC_PGPRMS(AKSCPGPRMS *prms)
{
    // Set 0 to the AKSCPGPRMS structure.
    memset(prms, 0, sizeof(AKSCPGPRMS));

    // Sensitivity
    prms->m_hs.u.x = AKSC_HSENSE_TARGET;
    prms->m_hs.u.y = AKSC_HSENSE_TARGET;
    prms->m_hs.u.z = AKSC_HSENSE_TARGET;

    prms->m_ho.u.x = 0;
    prms->m_ho.u.y = 0;
    prms->m_ho.u.z = 0;

    prms->m_pgFilter = CSPEC_PG_FILTER;
}

/*!
 Set initial values for SmartCompass library.
 @return If parameters are read successfully, the return value is
 #AKRET_PROC_SUCCEED. Otherwise the return value is #AKRET_PROC_FAIL. No
 error code is reserved to show which operation has failed.
 @param[in,out] prms A pointer to a #AKSCPGPRMS structure.
 */
int16 Init_PG_Measure(AKSCPGPRMS *prms)
{
    // Initialize the decompose parameters
    //AKSC_InitDecomp(prms->m_hdata);

    ALOGE("%s, called", __FUNCTION__);
    AKSC_InitPseudoGyroF(
                        &prms->m_pgcond,
                        &prms->m_pgvar
                        );

    PG_InitConfig(prms);

    return AKRET_PROC_SUCCEED;
}

void PG_InitConfig(AKSCPGPRMS* prms)
{
    int16 m_ocoef   = 307;
    int16 m_th_rdif = 32766;
    int16 m_th_rmax = 32766;
    int16 m_th_rmin = 0;

    prms->m_pgcond.fmode = 1;
    prms->m_pgcond.ihave = 24;
    prms->m_pgcond.iaave = 24;

    ALOGE("%s, called", __FUNCTION__);
    switch(prms->m_pgFilter){
        case 0:
            prms->m_pgcond.ihave = 24;
            prms->m_pgcond.iaave = 24;
            m_ocoef = 103;//0.1;
            break;
        case 1:
            prms->m_pgcond.ihave = 24;
            prms->m_pgcond.iaave = 24;
            m_ocoef = 205;//0.2
            break;
        case 2:
            prms->m_pgcond.ihave = 24;
            prms->m_pgcond.iaave = 24;
            m_ocoef = 307;//0.3
            break;
        case 3:
            prms->m_pgcond.ihave = 32;
            prms->m_pgcond.iaave = 32;
            m_ocoef = 205;//0.2;
            break;
        case 4:
            prms->m_pgcond.ihave = 32;
            prms->m_pgcond.iaave = 32;
            m_ocoef = 307;//0.3;
            break;
        case 5:
            prms->m_pgcond.ihave = 12;
            prms->m_pgcond.iaave = 12;
            m_ocoef = 307;//0.3;
            break;
        case 6:
            prms->m_pgcond.ihave = 12;
            prms->m_pgcond.iaave = 12;
            m_ocoef = 205;//0.2;
            break;
        case 7:
            prms->m_pgcond.ihave = 12;
            prms->m_pgcond.iaave = 12;
            m_ocoef = 103;//0.1;
            break;
        default:
            break;
    }

#if 0
    //prms->m_pgcond.ocoef   = (AKSC_FLOAT)m_ocoef/1024;//90;//103;
    prms->m_pgcond.ocoef   = m_ocoef;
    prms->m_pgcond.th_rdif = (int16)(m_th_rdif/16);
    prms->m_pgcond.th_rmax = (int16)(m_th_rmax/16);
    prms->m_pgcond.th_rmin = (int16)(m_th_rmin/16);
#else
    prms->m_pgcond.ocoef   = (AKSC_FLOAT)m_ocoef/1024;
    prms->m_pgcond.dtave   = 16;
    prms->m_pgcond.th_rdif = (int16)(m_th_rdif/16);
    prms->m_pgcond.th_rmax = (int16)(m_th_rmax/16);
    prms->m_pgcond.th_rmin = (int16)(m_th_rmin/16);

#endif
}

/*!
 Calculate angular speed.
 m_hvec and m_avec should be Android coordination.
 @return
 @param[in,out] prms A pointer to a #AKSCPGPRMS structure.
 */
int16 CalcAngularRate(AKSCPGPRMS* prms)
{
    /* Conversion matrix from Android to SmartCompass coordination */
    const I16MATRIX hlayout = {{
                                 0, 1, 0,
                                -1, 0, 0,
                                 0, 0, 1}};
    const I16MATRIX alayout = {{
                                 0,-1, 0,
                                 1, 0, 0,
                                 0, 0,-1}};


    int16vec tmp_hvec;
    int16 aksc_ret;
    int32        swp;

    // Subtract offset from non-averaged value.
    aksc_ret = AKSC_VNorm(
                          &prms->m_hdata,
                          &prms->m_ho,
                          &prms->m_hs,
                          AKSC_HSENSE_TARGET,
                          &tmp_hvec
                          );
    if (aksc_ret == 0) {
        AKMERROR;
        AKMDEBUG(AKMDBG_LEVEL3,"AKSC_VNorm failed.\n"
                "  have=%6d,%6d,%6d  ho=%6d,%6d,%6d  hs=%6d,%6d,%6d\n",
                prms->m_hdata.u.x, prms->m_hdata.u.y, prms->m_hdata.u.z,
                prms->m_ho.u.x, prms->m_ho.u.y, prms->m_ho.u.z,
                prms->m_hs.u.x, prms->m_hs.u.y, prms->m_hs.u.z);
        return AKRET_PROC_FAIL;
    }
    /*ALOGE("AKMPG AKSC_VNorm debug, have=%d,%d,%d, ho=%d,%d,%d, hs=%d,%d,%d\n",
            prms->m_hdata.u.x, prms->m_hdata.u.y, prms->m_hdata.u.z,
            prms->m_ho.u.x, prms->m_ho.u.y, prms->m_ho.u.z,
            prms->m_hs.u.x, prms->m_hs.u.y, prms->m_hs.u.z);*/

    prms->m_pgRet = AKSC_PseudoGyroF(
                        &prms->m_pgcond,
                        prms->m_pgdt,
                        &tmp_hvec,
                        &prms->m_avec,
                        &hlayout,
                        &alayout,
                        &prms->m_pgvar,
                        &prms->m_pgout,
                        &prms->m_pgquat,
                        &prms->m_pgGravity,
                        &prms->m_pgLinAcc
                    );

    ALOGE("AKMPG AKSC_PseudoGyro: fmode:%d, ihave:%d, iaave:%d, ocoef:%f, th_rmax:%d, th_rmin:%d, th_rdif:%d\n",
            prms->m_pgcond.fmode,
            prms->m_pgcond.ihave,
            prms->m_pgcond.iaave,
            prms->m_pgcond.ocoef,
            prms->m_pgcond.th_rmax,
            prms->m_pgcond.th_rmin,
            prms->m_pgcond.th_rdif);

    ALOGE("AKMPG AKSC_PseudoGyro: ret=%d, dt=%4.2f,"
            "hvec=%4.2f,%4.2f,%4.2f, avec=%4.5f,%4.5f,%4.5f\n",
            prms->m_pgRet,
            prms->m_pgdt/16.0f,
            tmp_hvec.u.x/16.0f, tmp_hvec.u.y/16.0f, tmp_hvec.u.z/16.0f,
            prms->m_avec.u.x/720.0f, prms->m_avec.u.y/720.0f, prms->m_avec.u.z/720.0f);

    ALOGE("AKMPG AKSC_PseudoGyro: dt=%4.2f rate=%4.2f,%4.2f,%4.2f quat=%4.5f,%4.5f,%4.5f,%4.5f\n",
            prms->m_pgdt/16.0f,
            prms->m_pgout.u.x/64.0f,
            prms->m_pgout.u.y/64.0f,
            prms->m_pgout.u.z/64.0f,
            prms->m_pgquat.u.x/16384.0f,
            prms->m_pgquat.u.y/16384.0f,
            prms->m_pgquat.u.z/16384.0f,
            prms->m_pgquat.u.w/16384.0f);

    if(prms->m_pgRet != 1) {
        AKMERROR;
        //AKMDEBUG(AKMDBG_LEVEL3,"AKSC_PseudoGyro failed: dt=%6.2f\n"
        /*ALOGE("AKSC_PseudoGyro failed: ret=%d, dt=%6.2f,"
                "  hvec=%6.2f,%6.2f,%6.2f  avec=%6.5f,%6.5f,%6.5f\n",
                prms->m_pgRet,
                prms->m_pgdt/16.0f,
                tmp_hvec.u.x/16.0f, tmp_hvec.u.y/16.0f, tmp_hvec.u.z/16.0f,
                prms->m_avec.u.x/720.0f, prms->m_avec.u.y/720.0f, prms->m_avec.u.z/720.0f);*/
        return AKRET_PROC_FAIL;
    } else {
        /* Convertion:
         from: AKM coordinate
         to  : Android coordinate
         Unit conversion will be done in HAL. */
        swp = prms->m_pgout.u.x;
        prms->m_pgout.u.x = -(prms->m_pgout.u.y);
        prms->m_pgout.u.y = swp;
        prms->m_pgout.u.z = prms->m_pgout.u.z;

        swp = prms->m_pgquat.u.x;
        prms->m_pgquat.u.x = prms->m_pgquat.u.y;
        prms->m_pgquat.u.y = -(swp);
        prms->m_pgquat.u.z = -(prms->m_pgquat.u.z);

        swp = prms->m_pgGravity.u.x;
        prms->m_pgGravity.u.x = prms->m_pgGravity.u.y;
        prms->m_pgGravity.u.y = -(swp);
        prms->m_pgGravity.u.z = -(prms->m_pgGravity.u.z);

        swp = prms->m_pgLinAcc.u.x;
        prms->m_pgLinAcc.u.x = prms->m_pgLinAcc.u.y;
        prms->m_pgLinAcc.u.y = -(swp);
        prms->m_pgLinAcc.u.z = -(prms->m_pgLinAcc.u.z);

        /*ALOGE("AKSC_PseudoGyro:\n"
            "  dt=%4.2f rate=%4.2f,%4.2f,%4.2f quat=%4.5f,%4.5f,%4.5f,%4.5f\n",
            prms->m_pgdt/16.0f,
            prms->m_pgout.u.x/64.0f,
            prms->m_pgout.u.y/64.0f,
            prms->m_pgout.u.z/64.0f,
            prms->m_pgquat.u.x/16384.0f,
            prms->m_pgquat.u.y/16384.0f,
            prms->m_pgquat.u.z/16384.0f,
            prms->m_pgquat.u.w/16384.0f);*/
    }

    return AKRET_PROC_SUCCEED;
}

void Get_PG_GyroData(
    AKSCPGPRMS *prms,
    float gyro_buf[],
    const int16_t buf_size,
    int8_t *status
) {
    if (gyro_buf != NULL && buf_size >= 3) {
        gyro_buf[0] = (float)prms->m_pgout.u.x * AKM_CONVERT_GYRO * DEG2RAD;
        gyro_buf[1] = (float)prms->m_pgout.u.y * AKM_CONVERT_GYRO * DEG2RAD;
        gyro_buf[2] = (float)prms->m_pgout.u.z * AKM_CONVERT_GYRO * DEG2RAD;
        *status = prms->m_hdst;
    }
}

void Get_PG_RotationVector(
    AKSCPGPRMS* prms,
    float* rv_buf,
    const int16_t buf_size
) {
    int16_t totalHDST = prms->m_hdst;
    float rotationEHA = 0.0f;

    if (rv_buf != NULL && buf_size >= 5) {
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
        rv_buf[0] = prms->m_pgquat.u.x * AKM_CONVERT_GEORV;
        rv_buf[1] = prms->m_pgquat.u.y * AKM_CONVERT_GEORV;
        rv_buf[2] = prms->m_pgquat.u.z * AKM_CONVERT_GEORV;
        rv_buf[3] = prms->m_pgquat.u.w * AKM_CONVERT_GEORV;
        rv_buf[4] = rotationEHA;
    }
}

void Get_PG_Gravity(
    AKSCPGPRMS *prms,
    float *gravity_buf,
    const int16_t buf_size
) {
    if (gravity_buf != NULL && buf_size >= 3) {
        /* Gravity */
        gravity_buf[0] = AKSC2SI(prms->m_pgGravity.u.x);
        gravity_buf[1] = AKSC2SI(prms->m_pgGravity.u.y);
        gravity_buf[2] = AKSC2SI(prms->m_pgGravity.u.z);
    }
}

void Get_PG_LA(
    AKSCPGPRMS *prms,
    float *la_buf,
    const int16_t buf_size
) {
    /* Linear Accelerometer */
    if (la_buf != NULL && buf_size >= 3) {
        la_buf[0] = AKSC2SI(prms->m_pgLinAcc.u.x);
        la_buf[1] = AKSC2SI(prms->m_pgLinAcc.u.y);
        la_buf[2] = AKSC2SI(prms->m_pgLinAcc.u.z);
    }
}
