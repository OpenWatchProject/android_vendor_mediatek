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
#include "AKCommon.h"
#include "AKCompass.h"
#include "ParameterIO.h"
#include "Measure.h"
#include "AK09911Register.h"
#include "AkmApi.h"
#include <string.h>


#define SNG2DBLBYTE(high, low)      ((int16)((((uint16)(high))<<8)+(uint16)(low)))
#define DISP_CONV_AKSCF(val)    ((val) * 0.06f)
#define DISP_CONV_Q6F(val)      ((val) * 0.015625f)
#define AKM_SENSOR_INFO_SIZE        2
#define AKM_SENSOR_CONF_SIZE        3

typedef unsigned char BYTE;

//#define AKMD_AK099XX
#ifdef AKMD_AK099XX
#define AKMD_ST2_POS 8
#else
#define AKMD_ST2_POS 7
#endif

AKM_DEVICE_TYPE AKM_GetDeviceType(uint8 wia_2);

extern uint8_t akm_wia[2];
/*!
 Initialize #AKSCPRMS structure. At first, 0 is set to all parameters.
 After that, some parameters, which should not be 0, are set to specific
 value. Some of initial values can be customized by editing the file
 \c "CustomerSpec.h".
 @param[out] prms A pointer to #AKSCPRMS structure.
 */
void InitAKSCPRMS(AKSCPRMS *prms)
{
    // Set 0 to the AKSCPRMS structure.
    memset(prms, 0, sizeof(AKSCPRMS));

    // Sensitivity
    prms->m_hs.u.x = AKSC_HSENSE_TARGET;
    prms->m_hs.u.y = AKSC_HSENSE_TARGET;
    prms->m_hs.u.z = AKSC_HSENSE_TARGET;

    // HDOE
    prms->m_hdst = AKSC_HDST_UNSOLVED;

    // (m_hdata is initialized with AKSC_InitDecomp)
    prms->m_hnave = CSPEC_HNAVE;
    prms->m_dvec.u.x = CSPEC_DVEC_X;
    prms->m_dvec.u.y = CSPEC_DVEC_Y;
    prms->m_dvec.u.z = CSPEC_DVEC_Z;

    prms->m_hlayout.m[0][0] = 0;
    prms->m_hlayout.m[0][1] = 1;
    prms->m_hlayout.m[0][2] = 0;
    prms->m_hlayout.m[1][0] = -1;
    prms->m_hlayout.m[1][1] = 0;
    prms->m_hlayout.m[1][2] = 0;
    prms->m_hlayout.m[2][0] = 0;
    prms->m_hlayout.m[2][1] = 0;
    prms->m_hlayout.m[2][2] = 1;

    prms->m_alayout.m[0][0] = 0;
    prms->m_alayout.m[0][1] = -1;
    prms->m_alayout.m[0][2] = 0;
    prms->m_alayout.m[1][0] = 1;
    prms->m_alayout.m[1][1] = 0;
    prms->m_alayout.m[1][2] = 0;
    prms->m_alayout.m[2][0] = 0;
    prms->m_alayout.m[2][1] = 0;
    prms->m_alayout.m[2][2] = -1;

    prms->m_glayout.m[0][0] = 0;
    prms->m_glayout.m[0][1] = 1;
    prms->m_glayout.m[0][2] = 0;
    prms->m_glayout.m[1][0] = -1;
    prms->m_glayout.m[1][1] = 0;
    prms->m_glayout.m[1][2] = 0;
    prms->m_glayout.m[2][0] = 0;
    prms->m_glayout.m[2][1] = 0;
    prms->m_glayout.m[2][2] = 1;
}

/*!
 Fill #AKSCPRMS structure with default value.
 @param[out] prms A pointer to #AKSCPRMS structure.
 */
void SetDefaultPRMS(AKSCPRMS *prms)
{
    int16 i;
#ifndef AKMD_DISABLE_DOEPLUS
    int16 j;
#endif
    // Set parameter to HDST, HO, HREF
    for (i = 0; i < CSPEC_NUM_FORMATION; i++) {
        prms->HSUC_HDST[i] = AKSC_HDST_UNSOLVED;
        prms->HSUC_HO[i].u.x = 0;
        prms->HSUC_HO[i].u.y = 0;
        prms->HSUC_HO[i].u.z = 0;
        prms->HFLUCV_HREF[i].u.x = 0;
        prms->HFLUCV_HREF[i].u.y = 0;
        prms->HFLUCV_HREF[i].u.z = 0;
        prms->HSUC_HBASE[i].u.x = 0;
        prms->HSUC_HBASE[i].u.y = 0;
        prms->HSUC_HBASE[i].u.z = 0;
#ifndef AKMD_DISABLE_DOEPLUS
        for (j = 0; j < AKSC_DOEP_SIZE; j++) {
            prms->DOEP_PRMS[i][j] = (AKSC_FLOAT)(0.);
        }
#endif
    }
}

/*!
 Read hard coded value (Fuse ROM) from AKM E-Compass. Then set the read value
 to calculation parameter.
 @return If parameters are read successfully, the return value is
 #AKRET_PROC_SUCCEED. Otherwise the return value is #AKRET_PROC_FAIL. No
 error code is reserved to show which operation has failed.
 @param[out] prms A pointer to #AKSCPRMS structure.
 */
int16 ReadFUSEROM(AKSCPRMS *prms)
{
    int16 regs[4];

    // Put temporary value
    regs[0] = akm_wia[0];
    /* Don't adjust ASA since it is done in the ddf driver */
    regs[1] = AKMD_ASA;
    regs[2] = AKMD_ASA;
    regs[3] = AKMD_ASA;

    // Additional settings
    prms->m_asa.u.x = regs[1];
    prms->m_asa.u.y = regs[2];
    prms->m_asa.u.z = regs[3];

    // Set keywords for SmartCompassLibrary certification
    prms->m_key[0] = 0;  // AKSC_GetVersion_Device();
    prms->m_key[1] = regs[0];
    prms->m_key[2] = regs[1];
    prms->m_key[3] = regs[2];
    prms->m_key[4] = regs[3];
    // printf("akm ReadFUSEROM m_key[1] = %x\n",prms->m_key[1]);


    strncpy( (char*)prms->m_licenser, CSPEC_CI_LICENSER, AKSC_CI_MAX_CHARSIZE);
    strncpy( (char*)prms->m_licensee, CSPEC_CI_LICENSEE, AKSC_CI_MAX_CHARSIZE);

    return AKRET_PROC_SUCCEED;
}


/*!
 Set initial values for SmartCompass library.
 @return If parameters are read successfully, the return value is
 #AKRET_PROC_SUCCEED. Otherwise the return value is #AKRET_PROC_FAIL. No
 error code is reserved to show which operation has failed.
 @param[in,out] prms A pointer to a #AKSCPRMS structure.
 */
int16 Init_Measure(AKSCPRMS *prms)
{
#if 0

    uint8 i2cData[AK09911_BDATA_SIZE];

    // Soft Reset

    i2cData[0] = AK09911_CNTL3_SOFT_RESET;
    AKD_TxData(AK09911_REG_CNTL3, i2cData, 1);

    // Wait over 100 us
    AKD_Sleep(1);
#endif

    prms->m_form = 0;

    // Restore the value when succeeding in estimating of HOffset.
    prms->m_ho   = prms->HSUC_HO[prms->m_form];
    prms->m_ho32.u.x = (int32)prms->HSUC_HO[prms->m_form].u.x;
    prms->m_ho32.u.y = (int32)prms->HSUC_HO[prms->m_form].u.y;
    prms->m_ho32.u.z = (int32)prms->HSUC_HO[prms->m_form].u.z;

    prms->m_hdst = prms->HSUC_HDST[prms->m_form];
    prms->m_hbase = prms->HSUC_HBASE[prms->m_form];

    // Initialize the decompose parameters
    AKSC_InitDecomp(prms->m_hdata);

#ifndef AKMD_DISABLE_DOEPLUS
    // Initialize DOEPlus parameters
    AKSC_InitDOEPlus(prms->m_doep_var);
    prms->m_doep_lv = AKSC_LoadDOEPlus(
                          prms->DOEP_PRMS[prms->m_form],
                          prms->m_doep_var);
    AKSC_InitDecomp(prms->m_hdata_plus);
#endif

    // Initialize HDOEaG parameters
    AKSC_InitHDOEaG(
        prms->m_doeag_var,
        1,
        &prms->m_hlayout,
        &prms->m_ho,
        prms->m_hdst
    );

    AKSC_InitHFlucCheck(
        &(prms->m_hflucv),
        &(prms->HFLUCV_HREF[prms->m_form]),
        HFLUCV_TH
    );

    // Initialize for Direction9D
    //AKSC_InitDirection9D(prms->m_hD9D);

    // Reset counter
    prms->m_cntSuspend = 0;

    prms->m_hdt_ag = -1;
    prms->m_gdt_ag = -1;

    // set DOEaG mode
    //prms->m_mode = CSPEC_DOEAG_MODE;
    prms->m_mode = CSPEC_DOEONLY_MODE;
    prms->m_oedt = CSPEC_DOEAG_OEDT;

    return AKRET_PROC_SUCCEED;
}

/*!
 SmartCompass main calculation routine. This function will be processed
 when INT pin event is occurred.
 @retval AKRET_
 @param[in] bData An array of register values which holds,
 ST1, HXL, HXH, HYL, HYH, HZL, HZH and ST2 value respectively.
 @param[in,out] prms A pointer to a #AKSCPRMS structure.
 @param[in] curForm The index of hardware position which represents the
 index when this function is called.
 */
int16 GetMagneticVectorDecomp(
    const int16 bData[],
    AKSCPRMS    *prms,
    const int16 curForm)
{
    const int16vec hrefZero = {{0, 0, 0}};
    int16          temperature, dor, derr, hofl, cb, dc;
    int32vec       preHbase;
    int16          overflow;
    int16          aksc_ret;
    int16          ret;

    temperature = 0;
    dor = 0;
    derr = 0;
    hofl = 0;
    cb = 0;
    dc = 0;

    preHbase = prms->m_hbase;
    overflow = 0;
    ret = AKRET_PROC_SUCCEED;

    // Subtract the formation suspend counter
    if (prms->m_cntSuspend > 0) {
        prms->m_cntSuspend--;

        // Check the suspend counter
        if (prms->m_cntSuspend == 0) {
            // Restore the value when succeeding in estimating of HOffset.
            prms->m_ho   = prms->HSUC_HO[prms->m_form];
            prms->m_ho32.u.x = (int32)prms->HSUC_HO[prms->m_form].u.x;
            prms->m_ho32.u.y = (int32)prms->HSUC_HO[prms->m_form].u.y;
            prms->m_ho32.u.z = (int32)prms->HSUC_HO[prms->m_form].u.z;

            prms->m_hdst = prms->HSUC_HDST[prms->m_form];
            prms->m_hbase = prms->HSUC_HBASE[prms->m_form];

            // Initialize the decompose parameters
            AKSC_InitDecomp(prms->m_hdata);

#ifndef AKMD_DISABLE_DOEPLUS
            // Initialize DOEPlus parameters
            AKSC_InitDOEPlus(prms->m_doep_var);
            prms->m_doep_lv = AKSC_LoadDOEPlus(
                                  prms->DOEP_PRMS[prms->m_form],
                                  prms->m_doep_var);
            AKSC_InitDecomp(prms->m_hdata_plus);
#endif

            // Initialize HDOE parameters
            AKSC_InitHDOEaG(
                prms->m_doeag_var,
                1,
                &prms->m_hlayout,
                &prms->m_ho,
                prms->m_hdst
            );

            // Initialize HFlucCheck parameters
            AKSC_InitHFlucCheck(
                &(prms->m_hflucv),
                &(prms->HFLUCV_HREF[prms->m_form]),
                HFLUCV_TH
            );
        }
    }

    // Decompose one block data into each Magnetic sensor's data
    // printf("akm decomps3 wia:%x\n",akm_wia[1]);
    aksc_ret = AKSC_DecompS3(
                   AKM_GetDeviceType(akm_wia[1]),
                   bData,
                   prms->m_hnave,
                   &prms->m_asa,
                   prms->m_pdcptr,
                   prms->m_hdata,
                   &prms->m_hbase,
                   &prms->m_hn,
                   &prms->m_have,
                   &temperature,
                   &dor,
                   &derr,
                   &hofl,
                   &cb,
                   &dc
               );
#if 0
    Logger_log("akm %s: ST1, HXH&HXL, HYH&HYL, HZH&HZL, ST2,"
               " hdata[0].u.x, hdata[0].u.y, hdata[0].u.z,"
               " asax, asay, asaz ="
               " %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
               __FUNCTION__,
               bData[0],
               (int16)(((uint16)bData[2]) << 8 | bData[1]),
               (int16)(((uint16)bData[4]) << 8 | bData[3]),
               (int16)(((uint16)bData[6]) << 8 | bData[5]), bData[AKMD_ST2_POS],
               prms->m_hdata[0].u.x, prms->m_hdata[0].u.y, prms->m_hdata[0].u.z,
               prms->m_asa.u.x, prms->m_asa.u.y, prms->m_asa.u.z);
#endif
    if (aksc_ret == 0) {
        AKMERROR;
        AKMDEBUG(AKMDBG_DUMP,
                 "AKSC_DecompS3 failed.\n"
                 "  ST1=0x%02X, ST2=0x%02X\n"
                 "  XYZ(HEX)=%02X,%02X,%02X,%02X,%02X,%02X\n"
                 "  asa(dec)=%d,%d,%d\n"
                 "  pdc(addr)=0x%p\n"
                 "  hbase(dec)=%ld,%ld,%ld\n",
                 bData[0], bData[AKMD_ST2_POS],
                 bData[1], bData[2], bData[3], bData[4], bData[5], bData[6],
                 prms->m_asa.u.x, prms->m_asa.u.y, prms->m_asa.u.z,
                 prms->m_pdcptr,
                 prms->m_hbase.u.x, prms->m_hbase.u.y, prms->m_hbase.u.z);
        return AKRET_PROC_FAIL;
    }

    // Check the formation change
    if (prms->m_form != curForm) {
        prms->m_form = curForm;
        prms->m_cntSuspend = CSPEC_CNTSUSPEND_SNG;
        ret |= AKRET_FORMATION_CHANGED;
        return ret;
    }

    // Check derr
    if (derr == 1) {
        ret |= AKRET_DATA_READERROR;
        return ret;
    }

    // Check hofl
    if (hofl == 1) {
        if (prms->m_cntSuspend <= 0) {
            // Set a HDOEaG level as "HDST_UNSOLVED"
            AKSC_SetHDOEaGLevel(
                prms->m_doeag_var,
                &prms->m_hlayout,
                &prms->m_ho,
                AKSC_HDST_UNSOLVED,
                1
            );
            prms->m_hdst = AKSC_HDST_UNSOLVED;
        }
        ret |= AKRET_DATA_OVERFLOW;
        return ret;
    }

    // Check hbase
    if (cb == 1) {
        // Translate HOffset
        AKSC_TransByHbase(
            &(preHbase),
            &(prms->m_hbase),
            &(prms->m_ho),
            &(prms->m_ho32),
            &overflow
        );
        if (overflow == 1) {
            ret |= AKRET_OFFSET_OVERFLOW;
        }

        // Set hflucv.href to 0
        AKSC_InitHFlucCheck(
            &(prms->m_hflucv),
            &(hrefZero),
            HFLUCV_TH
        );

        if (prms->m_cntSuspend <= 0) {
            // Set a HDOEaG level as "HDST_UNSOLVED"
            AKSC_SetHDOEaGLevel(
                prms->m_doeag_var,
                &prms->m_hlayout,
                &prms->m_ho,
                AKSC_HDST_UNSOLVED,
                1
            );
            prms->m_hdst = AKSC_HDST_UNSOLVED;
        }

        ret |= AKRET_HBASE_CHANGED;
        return ret;
    }

    return AKRET_PROC_SUCCEED;
}

int16 GetMagneticVectorOffset(
    AKSCPRMS    *prms)
{
    int16    hdSucc, hfluc, aksc_ret;
    int16vec hvec;
    int16    ret = AKRET_PROC_SUCCEED;
#ifndef AKMD_DISABLE_DOEPLUS
    int      i;
    int16    doep_ret;
#endif

    if (prms->m_cntSuspend <= 0) {
        // Detect a fluctuation of magnetic field.
        hfluc = AKSC_HFlucCheck(&(prms->m_hflucv), &(prms->m_hdata[0]));

        if (hfluc == 1) {
            // Set a HDOEaG level as "HDST_UNSOLVED"
            AKSC_SetHDOEaGLevel(
                prms->m_doeag_var,
                &prms->m_hlayout,
                &prms->m_ho,
                AKSC_HDST_UNSOLVED,
                1
            );
            prms->m_hdst = AKSC_HDST_UNSOLVED;
            ret |= AKRET_HFLUC_OCCURRED;
            return ret;
        } else {
#ifndef AKMD_DISABLE_DOEPLUS
            if(prms->m_hdt_ag >= 0) { // only the case when valid mag data is updated
                // Compensate Magnetic Distortion by DOEPlus
                doep_ret = AKSC_DOEPlus(&prms->m_hdata[0], prms->m_doep_var, &prms->m_doep_lv);

                // Save DOEPlus parameters
                if ((doep_ret == 1) && (prms->m_doep_lv == 3)) {
                    AKSC_SaveDOEPlus(prms->m_doep_var, prms->DOEP_PRMS[prms->m_form]);
                }

                // Calculate compensated vector for DOE
                for (i = 0; i < prms->m_hn; i++) {
                    AKSC_DOEPlus_DistCompen(&prms->m_hdata[i], prms->m_doep_var, &prms->m_hdata_plus[i]);
                }
            }

            AKMDEBUG(AKMDBG_DOEPLUS, "DOEP: %7d, %7d, %7d ",
                     prms->m_hdata[0].u.x,
                     prms->m_hdata[0].u.y,
                     prms->m_hdata[0].u.z);
            AKMDEBUG(AKMDBG_DOEPLUS, "|%7d, %7d, %7d \n",
                     prms->m_hdata_plus[0].u.x,
                     prms->m_hdata_plus[0].u.y,
                     prms->m_hdata_plus[0].u.z);

            //Calculate Magnetic sensor's offset by DOE
            hdSucc = AKSC_HDOEaG(
                         prms->m_licenser,
                         prms->m_licensee,
                         prms->m_key,
                         prms->m_mode,
                         prms->m_oedt,
                         prms->m_hdt_ag,
                         prms->m_gdt_ag,
                         &prms->m_hdata_plus[0],
                         &prms->m_gvec,
                         &prms->m_hlayout,
                         &prms->m_glayout,
                         prms->m_doeag_var,
                         &prms->m_ho,
                         &prms->m_hdst
                     );
#else
            //Calculate Magnetic sensor's offset by DOEaG
            hdSucc = AKSC_HDOEaG(
                         prms->m_licenser,
                         prms->m_licensee,
                         prms->m_key,
                         prms->m_mode,
                         prms->m_oedt,
                         prms->m_hdt_ag,
                         prms->m_gdt_ag,
                         &prms->m_hdata[0],
                         &prms->m_gvec,
                         &prms->m_hlayout,
                         &prms->m_glayout,
                         prms->m_doeag_var,
                         &prms->m_ho,
                         &prms->m_hdst
                     );
#if 0
            // Logger_log("akm", 1,
            printf("akm mode  =%6d "
                       "hdt_ag=%6d "
                       "gdt_ag=%6d "
                       "hdata =%6d,%6d,%6d "
                       "gvec  =%6d,%6d,%6d "
                       "ho    =%6d,%6d,%6d "
                       "hdst  =%6d\n",
                       prms->m_mode,
                       prms->m_hdt_ag,
                       prms->m_gdt_ag,
                       prms->m_hdata[0].u.x, prms->m_hdata[0].u.y, prms->m_hdata[0].u.z,
                       prms->m_gvec.u.x, prms->m_gvec.u.y, prms->m_gvec.u.z,
                       prms->m_ho.u.x, prms->m_ho.u.y, prms->m_ho.u.z,
                       prms->m_hdst);
#endif
#endif
            if (hdSucc == AKSC_CERTIFICATION_DENIED) {
                AKMERROR;
                return AKRET_PROC_FAIL;
            }
            if (hdSucc > 0) {
                prms->HSUC_HO[prms->m_form] = prms->m_ho;
                prms->m_ho32.u.x = (int32)prms->m_ho.u.x;
                prms->m_ho32.u.y = (int32)prms->m_ho.u.y;
                prms->m_ho32.u.z = (int32)prms->m_ho.u.z;

                prms->HSUC_HDST[prms->m_form] = prms->m_hdst;
                prms->HFLUCV_HREF[prms->m_form] = prms->m_hflucv.href;
                prms->HSUC_HBASE[prms->m_form] = prms->m_hbase;
            }
        }
    }

#ifndef AKMD_DISABLE_DOEPLUS
    if(prms->m_hdt_ag >= 0) { // only the case when valid mag data is updated
        // Calculate compensated vector
        AKSC_DOEPlus_DistCompen(&prms->m_have, prms->m_doep_var, &prms->m_have);
    }
#endif

    // Subtract offset and normalize magnetic field vector.
    aksc_ret = AKSC_VNorm(
                   &prms->m_have,
                   &prms->m_ho,
                   &prms->m_hs,
                   AKSC_HSENSE_TARGET,
                   &hvec
               );
    if (aksc_ret == 0) {
        AKMERROR;
        AKMDEBUG(AKMDBG_DUMP,
                 "AKSC_VNorm failed.\n"
                 "  have=%6d,%6d,%6d  ho=%6d,%6d,%6d  hs=%6d,%6d,%6d gdata=%6d,%6d,%6d hdt_ag=%6d gdt_ag=%6d\n",
                 prms->m_have.u.x, prms->m_have.u.y, prms->m_have.u.z,
                 prms->m_ho.u.x, prms->m_ho.u.y, prms->m_ho.u.z,
                 prms->m_hs.u.x, prms->m_hs.u.y, prms->m_hs.u.z,
                 prms->m_gvec.u.x, prms->m_gvec.u.y, prms->m_gvec.u.z,
                 prms->m_hdt_ag, prms->m_gdt_ag
                );
        ret |= AKRET_VNORM_ERROR;
        return ret;
    } else {
        AKMDEBUG(AKMDBG_DUMP,
                 "AKSC_VNorm success.\n"
                 "  have=%6d,%6d,%6d  ho=%6d,%6d,%6d  hs=%6d,%6d,%6d gdata=%6d,%6d,%6d hdt_ag=%6d gdt_ag=%6d\n",
                 prms->m_have.u.x, prms->m_have.u.y, prms->m_have.u.z,
                 prms->m_ho.u.x, prms->m_ho.u.y, prms->m_ho.u.z,
                 prms->m_hs.u.x, prms->m_hs.u.y, prms->m_hs.u.z,
                 prms->m_gvec.u.x, prms->m_gvec.u.y, prms->m_gvec.u.z,
                 prms->m_hdt_ag, prms->m_gdt_ag
                );
    }

    // hvec is updated only when VNorm function is succeeded.
    prms->m_hvec = hvec;

    // Bias of Uncalibrated Magnetic Field
    prms->m_bias.u.x = (int32)(prms->m_ho.u.x) + prms->m_hbase.u.x;
    prms->m_bias.u.y = (int32)(prms->m_ho.u.y) + prms->m_hbase.u.y;
    prms->m_bias.u.z = (int32)(prms->m_ho.u.z) + prms->m_hbase.u.z;

    //Convert layout from sensor to Android by using PAT number.
    // Magnetometer
    //ConvertCoordinate(prms->m_hlayout_patno, &prms->m_hvec);
    // Bias of Uncalibrated Magnetic Field
    //ConvertCoordinate32(prms->m_hlayout_patno, &prms->m_bias);

    // Magnetic Field
    prms->m_calib.u.x = prms->m_hvec.u.x;
    prms->m_calib.u.y = prms->m_hvec.u.y;
    prms->m_calib.u.z = prms->m_hvec.u.z;

    // Uncalibrated Magnetic Field
    prms->m_uncalib.u.x = (int32)(prms->m_calib.u.x) + prms->m_bias.u.x;
    prms->m_uncalib.u.y = (int32)(prms->m_calib.u.y) + prms->m_bias.u.y;
    prms->m_uncalib.u.z = (int32)(prms->m_calib.u.z) + prms->m_bias.u.z;

    AKMDEBUG(AKMDBG_VECTOR,
             "mag(dec)=%6d,%6d,%6d\n"
             "maguc(dec),bias(dec)=%7ld,%7ld,%7ld,%7ld,%7ld,%7ld\n",
             prms->m_calib.u.x, prms->m_calib.u.y, prms->m_calib.u.z,
             prms->m_uncalib.u.x, prms->m_uncalib.u.y, prms->m_uncalib.u.z,
             prms->m_bias.u.x, prms->m_bias.u.y, prms->m_bias.u.z);

    return AKRET_PROC_SUCCEED;
}

#if 0
/*!
 Calculate Yaw, Pitch, Roll angle.
 m_hvec, m_avec and m_gvec should be Android coordination.
 @return Always return #AKRET_PROC_SUCCEED.
 @param[in,out] prms A pointer to a #AKSCPRMS structure.
 */
int16 CalcDirection(AKSCPRMS *prms)
{
    /* Conversion matrix from Android to SmartCompass coordination */
    int16 preThe;

    preThe = prms->m_theta;

    AKMDEBUG(AKMDBG_D9D, "D9D dt:%4d, %4d, %4d\n",
             prms->m_hdt, prms->m_adt, prms->m_gdt);

#if D9D_DISABLE
    prms->m_d9dRet = AKSC_DirectionS3(
                         prms->m_licenser,
                         prms->m_licensee,
                         prms->m_key,
                         &prms->m_hvec,
                         &prms->m_avec,
                         &prms->m_dvec,
                         &prms->m_hlayout,
                         &prms->m_alayout,
                         &prms->m_theta,
                         &prms->m_delta,
                         &prms->m_hr,
                         &prms->m_hrhoriz,
                         &prms->m_ar,
                         &prms->m_phi180,
                         &prms->m_phi90,
                         &prms->m_eta180,
                         &prms->m_eta90,
                         &prms->m_mat,
                         &prms->m_quat);
#else
    prms->m_d9dRet = AKSC_Direction9D(
                         prms->m_hD9D,
                         prms->m_licenser,
                         prms->m_licensee,
                         prms->m_key,
                         &prms->m_hvec,
                         &prms->m_avec,
                         &prms->m_gvec,
                         prms->m_hdt,
                         prms->m_adt,
                         prms->m_gdt,
                         &prms->m_dvec,
                         &prms->m_hlayout,
                         &prms->m_alayout,
                         &prms->m_glayout,
                         &prms->m_theta,
                         &prms->m_delta,
                         &prms->m_hr,
                         &prms->m_hrhoriz,
                         &prms->m_ar,
                         &prms->m_phi180,
                         &prms->m_phi90,
                         &prms->m_eta180,
                         &prms->m_eta90,
                         &prms->m_mat,
                         &prms->m_quat,
                         &prms->m_gravity,
                         &prms->m_linacc);
#endif
    prms->m_theta = AKSC_ThetaFilter(
                        prms->m_theta,
                        preThe,
                        THETAFILTER_SCALE);

    if (prms->m_d9dRet == AKSC_CERTIFICATION_DENIED) {
        AKMERROR;
        return AKRET_PROC_FAIL;
    }

    /* Convert Yaw, Pitch, Roll angle to Android coordinate system */
    if (prms->m_d9dRet & 0x02) {
        prms->m_eta180 = -prms->m_eta180;
        prms->m_eta90  = -prms->m_eta90;

        AKMDEBUG(AKMDBG_D9D, "AKSC_Direction9D (0x%02x):\n"
                 "  Yaw, Pitch, Roll=%6.1f,%6.1f,%6.1f\n",
                 prms->m_d9dRet,
                 DISP_CONV_Q6F(prms->m_theta),
                 DISP_CONV_Q6F(prms->m_phi180),
                 DISP_CONV_Q6F(prms->m_eta90));
    }

    return AKRET_PROC_SUCCEED;
}
#endif

// add for multi lib
AKM_DEVICE_TYPE AKM_GetDeviceType(uint8 wia_2) {
    AKM_DEVICE_TYPE device = NO_AKM_DEVICE;
    switch (wia_2) {
        case 0x05:
            device = AK09911;
            break;
        case 0x04:
            device = AK09912;
            break;
        case 0x08:
            device = AK09913;
            break;
        case 0x10:
            device = AK09915;
            break;
        case 0x09:
            device = AK09916C;
            break;
        case 0x0b:
            device = AK09916D;
            break;
        case 0x0c:
            device = AK09918;
            break;
        default:
            device = NO_AKM_DEVICE;
            break;
    }
    // printf("AKM_GetDeviceType device:%d\n",device);
    return device;
}

