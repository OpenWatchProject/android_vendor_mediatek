/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#undef LOG_TAG
#define LOG_TAG "CarrierCfg"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <RfxStatusDefs.h>

#include "carrierconfig.h"

const static CarrierConfigValue carierConfigData[] = {
   { MCC_MNC_SEPARATOR, "00101" },  /* TEST SIM */
   #include <config_data/carrier_config_001_01.h>
   {MCC_MNC_SEPARATOR, "20201" },
   #include <config_data/carrier_config_202_01.h>
   {MCC_MNC_SEPARATOR, "20202" },
   #include <config_data/carrier_config_202_02.h>
   {MCC_MNC_SEPARATOR, "20205" },
   #include <config_data/carrier_config_202_05.h>
   {MCC_MNC_SEPARATOR, "20402" },
   #include <config_data/carrier_config_204_02.h>
   {MCC_MNC_SEPARATOR, "20209" },
   #include <config_data/carrier_config_202_09.h>
   {MCC_MNC_SEPARATOR, "20210" },
   #include <config_data/carrier_config_202_10.h>
   {MCC_MNC_SEPARATOR, "20404" },
   #include <config_data/carrier_config_204_04.h>
   {MCC_MNC_SEPARATOR, "20408" },
   #include <config_data/carrier_config_204_08.h>
   {MCC_MNC_SEPARATOR, "20416" },
   #include <config_data/carrier_config_204_16.h>
   {MCC_MNC_SEPARATOR, "20420" },
   #include <config_data/carrier_config_204_20.h>
   {MCC_MNC_SEPARATOR, "20469" },
   #include <config_data/carrier_config_204_69.h>
   {MCC_MNC_SEPARATOR, "20601" },
   #include <config_data/carrier_config_206_01.h>
   {MCC_MNC_SEPARATOR, "20605" },
   #include <config_data/carrier_config_206_05.h>
   {MCC_MNC_SEPARATOR, "20620" },
   #include <config_data/carrier_config_206_20.h>
   {MCC_MNC_SEPARATOR, "20610" },
   #include <config_data/carrier_config_206_10.h>
   {MCC_MNC_SEPARATOR, "20801" },
   #include <config_data/carrier_config_208_01.h>
   {MCC_MNC_SEPARATOR, "20802" },
   #include <config_data/carrier_config_208_02.h>
   {MCC_MNC_SEPARATOR, "20809" },
   #include <config_data/carrier_config_208_09.h>
   {MCC_MNC_SEPARATOR, "20810" },
   #include <config_data/carrier_config_208_10.h>
   {MCC_MNC_SEPARATOR, "20811" },
   #include <config_data/carrier_config_208_11.h>
   {MCC_MNC_SEPARATOR, "20813" },
   #include <config_data/carrier_config_208_13.h>
   {MCC_MNC_SEPARATOR, "20820" },
   #include <config_data/carrier_config_208_20.h>
   {MCC_MNC_SEPARATOR, "20888" },
   #include <config_data/carrier_config_208_88.h>
   {MCC_MNC_SEPARATOR, "21401" },
   #include <config_data/carrier_config_214_01.h>
   {MCC_MNC_SEPARATOR, "21403" },
   #include <config_data/carrier_config_214_03.h>
   {MCC_MNC_SEPARATOR, "21405" },
   #include <config_data/carrier_config_214_05.h>
   {MCC_MNC_SEPARATOR, "21406" },
   #include <config_data/carrier_config_214_06.h>
   {MCC_MNC_SEPARATOR, "21407" },
   #include <config_data/carrier_config_214_07.h>
   {MCC_MNC_SEPARATOR, "21601" },
   #include <config_data/carrier_config_216_01.h>
   {MCC_MNC_SEPARATOR, "21603" },
   #include <config_data/carrier_config_216_03.h>
   {MCC_MNC_SEPARATOR, "21630" },
   #include <config_data/carrier_config_216_30.h>
   {MCC_MNC_SEPARATOR, "21670" },   /* Vodafone Hungary */
   #include <config_data/carrier_config_216_70.h>
   { MCC_MNC_SEPARATOR, "21901" },  /* TMO Croatia */
   #include <config_data/carrier_config_219_01.h>
   { MCC_MNC_SEPARATOR, "21910" },  /* A1 Croatia */
   #include <config_data/carrier_config_219_10.h>
   {MCC_MNC_SEPARATOR, "22005" },   /* VIP Serbia */
   #include <config_data/carrier_config_220_05.h>
   {MCC_MNC_SEPARATOR, "22201" },
   #include <config_data/carrier_config_222_01.h>
   {MCC_MNC_SEPARATOR, "22206" },
   #include <config_data/carrier_config_222_06.h>
   {MCC_MNC_SEPARATOR, "22210" },
   #include <config_data/carrier_config_222_10.h>
   {MCC_MNC_SEPARATOR, "22288" },
   #include <config_data/carrier_config_222_88.h>
   {MCC_MNC_SEPARATOR, "22601" },  /* Vodafone Romamia */
   #include <config_data/carrier_config_226_01.h>
   {MCC_MNC_SEPARATOR, "22605" },
   #include <config_data/carrier_config_226_05.h>
   {MCC_MNC_SEPARATOR, "22610" },
   #include <config_data/carrier_config_226_10.h>
   {MCC_MNC_SEPARATOR, "22801" },
   #include <config_data/carrier_config_228_01.h>
   {MCC_MNC_SEPARATOR, "22802" },
   #include <config_data/carrier_config_228_02.h>
   {MCC_MNC_SEPARATOR, "22803" }, /* Salt Switzerland */
   #include <config_data/carrier_config_228_03.h>
   {MCC_MNC_SEPARATOR, "23001" },
   #include <config_data/carrier_config_230_01.h>
   {MCC_MNC_SEPARATOR, "23002" },
   #include <config_data/carrier_config_230_02.h>
   {MCC_MNC_SEPARATOR, "23003" },
   #include <config_data/carrier_config_230_03.h>
   {MCC_MNC_SEPARATOR, "23099" },
   #include <config_data/carrier_config_230_99.h>
   { MCC_MNC_SEPARATOR, "23101" },
   #include <config_data/carrier_config_231_01.h>
   {MCC_MNC_SEPARATOR, "23102" },
   #include <config_data/carrier_config_231_02.h>
   {MCC_MNC_SEPARATOR, "23104" },
   #include <config_data/carrier_config_231_04.h>
   {MCC_MNC_SEPARATOR, "23106" },
   #include <config_data/carrier_config_231_06.h>
   {MCC_MNC_SEPARATOR, "23112" },
   #include <config_data/carrier_config_231_12.h>
   {MCC_MNC_SEPARATOR, "23201" },
   #include <config_data/carrier_config_232_01.h>
   {MCC_MNC_SEPARATOR, "23203" },  /* TMOEU Austria */
   #include <config_data/carrier_config_232_03.h>
   {MCC_MNC_SEPARATOR, "23205" },
   #include <config_data/carrier_config_232_05.h>
   {MCC_MNC_SEPARATOR, "23207" },  /* TMOEU Austria */
   #include <config_data/carrier_config_232_07.h>
   {MCC_MNC_SEPARATOR, "23210" },
   #include <config_data/carrier_config_232_10.h>
   {MCC_MNC_SEPARATOR, "23410" },
   #include <config_data/carrier_config_234_10.h>
   {MCC_MNC_SEPARATOR, "23415" },
   #include <config_data/carrier_config_234_15.h>
   {MCC_MNC_SEPARATOR, "23420" },
   #include <config_data/carrier_config_234_20.h>
   {MCC_MNC_SEPARATOR, "23430" },
   #include <config_data/carrier_config_234_30.h>
   {MCC_MNC_SEPARATOR, "23433" },
   #include <config_data/carrier_config_234_33.h>
   {MCC_MNC_SEPARATOR, "23408" },
   #include <config_data/carrier_config_234_08.h>
   {MCC_MNC_SEPARATOR, "23434" },
   #include <config_data/carrier_config_234_34.h>
   {MCC_MNC_SEPARATOR, "23470" },
   #include <config_data/carrier_config_234_70.h>
   {MCC_MNC_SEPARATOR, "23486" },
   #include <config_data/carrier_config_234_86.h>
   {MCC_MNC_SEPARATOR, "23502" },
   #include <config_data/carrier_config_235_02.h>
   {MCC_MNC_SEPARATOR, "23591" },
   #include <config_data/carrier_config_235_91.h>
   {MCC_MNC_SEPARATOR, "23594" },
   #include <config_data/carrier_config_235_94.h>
   {MCC_MNC_SEPARATOR, "23801" }, /* TDC Denmark */
   #include <config_data/carrier_config_238_01.h>
   {MCC_MNC_SEPARATOR, "23802" },
   #include <config_data/carrier_config_238_02.h>
   {MCC_MNC_SEPARATOR, "23806" },
   #include <config_data/carrier_config_238_06.h>
   {MCC_MNC_SEPARATOR, "23810" }, /* TDC Denmark */
   #include <config_data/carrier_config_238_10.h>
   {MCC_MNC_SEPARATOR, "23820" },
   #include <config_data/carrier_config_238_20.h>
   {MCC_MNC_SEPARATOR, "23830" },
   #include <config_data/carrier_config_238_30.h>
   {MCC_MNC_SEPARATOR, "23877" },
   #include <config_data/carrier_config_238_77.h>
   {MCC_MNC_SEPARATOR, "24001" },
   #include <config_data/carrier_config_240_01.h>
   {MCC_MNC_SEPARATOR, "24002" },
   #include <config_data/carrier_config_240_02.h>
   {MCC_MNC_SEPARATOR, "24005" },
   #include <config_data/carrier_config_240_05.h>
   {MCC_MNC_SEPARATOR, "24007" },
   #include <config_data/carrier_config_240_07.h>
   {MCC_MNC_SEPARATOR, "24008" },
   #include <config_data/carrier_config_240_08.h>
   {MCC_MNC_SEPARATOR, "24099" },
   #include <config_data/carrier_config_240_99.h>
   {MCC_MNC_SEPARATOR, "24201" },
   #include <config_data/carrier_config_242_01.h>
   {MCC_MNC_SEPARATOR, "24202" },
   #include <config_data/carrier_config_242_02.h>
   {MCC_MNC_SEPARATOR, "24205" },
   #include <config_data/carrier_config_242_05.h>
   {MCC_MNC_SEPARATOR, "24214" },
   #include <config_data/carrier_config_242_14.h>
   {MCC_MNC_SEPARATOR, "24405" },
   #include <config_data/carrier_config_244_05.h>
   {MCC_MNC_SEPARATOR, "24407" },
   #include <config_data/carrier_config_244_07.h>
   {MCC_MNC_SEPARATOR, "24408" },
   #include <config_data/carrier_config_244_08.h>
   {MCC_MNC_SEPARATOR, "24409" },
   #include <config_data/carrier_config_244_09.h>
   {MCC_MNC_SEPARATOR, "24412" },
   #include <config_data/carrier_config_244_12.h>
   {MCC_MNC_SEPARATOR, "24413" },
   #include <config_data/carrier_config_244_13.h>
   {MCC_MNC_SEPARATOR, "24421" },
   #include <config_data/carrier_config_244_21.h>
   {MCC_MNC_SEPARATOR, "24427" },
   #include <config_data/carrier_config_244_27.h>
   {MCC_MNC_SEPARATOR, "24436" },
   #include <config_data/carrier_config_244_36.h>
   {MCC_MNC_SEPARATOR, "24491" },
   #include <config_data/carrier_config_244_91.h>
   {MCC_MNC_SEPARATOR, "24601" },/* Telia Lithuania */
   #include <config_data/carrier_config_246_01.h>
   {MCC_MNC_SEPARATOR, "24681" },
   #include <config_data/carrier_config_246_81.h>
   {MCC_MNC_SEPARATOR, "24801" },/* Telia Estonia */
   #include <config_data/carrier_config_248_01.h>
   {MCC_MNC_SEPARATOR, "25001" },
   #include <config_data/carrier_config_250_01.h>
   {MCC_MNC_SEPARATOR, "25002" },
   #include <config_data/carrier_config_250_02.h>
   {MCC_MNC_SEPARATOR, "25004" },
   #include <config_data/carrier_config_250_04.h>
   {MCC_MNC_SEPARATOR, "25005" },
   #include <config_data/carrier_config_250_05.h>
   {MCC_MNC_SEPARATOR, "25010" },
   #include <config_data/carrier_config_250_10.h>
   {MCC_MNC_SEPARATOR, "25011" },
   #include <config_data/carrier_config_250_11.h>
   {MCC_MNC_SEPARATOR, "25013" },
   #include <config_data/carrier_config_250_13.h>
   {MCC_MNC_SEPARATOR, "25020" },
   #include <config_data/carrier_config_250_20.h>
   {MCC_MNC_SEPARATOR, "25039" },
   #include <config_data/carrier_config_250_39.h>
   {MCC_MNC_SEPARATOR, "25092" },
   #include <config_data/carrier_config_250_92.h>
   {MCC_MNC_SEPARATOR, "25093" },
   #include <config_data/carrier_config_250_93.h>
   { MCC_MNC_SEPARATOR, "25099" },
   #include <config_data/carrier_config_250_99.h>
   { MCC_MNC_SEPARATOR, "25503" },
   #include <config_data/carrier_config_255_03.h>
   {MCC_MNC_SEPARATOR, "25701" },
   #include <config_data/carrier_config_257_01.h>
   {MCC_MNC_SEPARATOR, "25915" },
   #include <config_data/carrier_config_259_15.h>
   {MCC_MNC_SEPARATOR, "26001" },
   #include <config_data/carrier_config_260_01.h>
   {MCC_MNC_SEPARATOR, "26002" },
   #include <config_data/carrier_config_260_02.h>
   {MCC_MNC_SEPARATOR, "26003" },
   #include <config_data/carrier_config_260_03.h>
   {MCC_MNC_SEPARATOR, "26006" },
   #include <config_data/carrier_config_260_06.h>
   {MCC_MNC_SEPARATOR, "26098" },
   #include <config_data/carrier_config_260_98.h>
   {MCC_MNC_SEPARATOR, "26034" },
   #include <config_data/carrier_config_260_34.h>
   {MCC_MNC_SEPARATOR, "26201" },
   #include <config_data/carrier_config_262_01.h>
   {MCC_MNC_SEPARATOR, "26202" },
   #include <config_data/carrier_config_262_02.h>
   {MCC_MNC_SEPARATOR, "26203" },
   #include <config_data/carrier_config_262_03.h>
   {MCC_MNC_SEPARATOR, "26204" },
   #include <config_data/carrier_config_262_04.h>
   {MCC_MNC_SEPARATOR, "26206" },
   #include <config_data/carrier_config_262_06.h>
   {MCC_MNC_SEPARATOR, "26207" },
   #include <config_data/carrier_config_262_07.h>
   {MCC_MNC_SEPARATOR, "26208" },
   #include <config_data/carrier_config_262_08.h>
   {MCC_MNC_SEPARATOR, "26209" },
   #include <config_data/carrier_config_262_09.h>
   {MCC_MNC_SEPARATOR, "26277" },
   #include <config_data/carrier_config_262_77.h>
   {MCC_MNC_SEPARATOR, "26288" },
   #include <config_data/carrier_config_262_88.h>
   {MCC_MNC_SEPARATOR, "26299" },
   #include <config_data/carrier_config_262_99.h>
   {MCC_MNC_SEPARATOR, "26801" },
   #include <config_data/carrier_config_268_01.h>
   {MCC_MNC_SEPARATOR, "26806" },
   #include <config_data/carrier_config_268_06.h>
   {MCC_MNC_SEPARATOR, "26889" },
   #include <config_data/carrier_config_268_89.h>
   {MCC_MNC_SEPARATOR, "27201" },
   #include <config_data/carrier_config_272_01.h>
   {MCC_MNC_SEPARATOR, "28401" },
   #include <config_data/carrier_config_284_01.h>
   {MCC_MNC_SEPARATOR, "28403" },
   #include <config_data/carrier_config_284_03.h>
   {MCC_MNC_SEPARATOR, "28405" },
   #include <config_data/carrier_config_284_05.h>
   {MCC_MNC_SEPARATOR, "28601" },
   #include <config_data/carrier_config_286_01.h>
   {MCC_MNC_SEPARATOR, "28602" },
   #include <config_data/carrier_config_286_02.h>
   {MCC_MNC_SEPARATOR, "28603" },
   #include <config_data/carrier_config_286_03.h>
   {MCC_MNC_SEPARATOR, "29341" },
   #include <config_data/carrier_config_293_41.h>
   {MCC_MNC_SEPARATOR, "29403" },
   #include <config_data/carrier_config_294_03.h>
   {MCC_MNC_SEPARATOR, "29501" },
   #include <config_data/carrier_config_295_01.h>
   {MCC_MNC_SEPARATOR, "302220" },
   #include <config_data/carrier_config_302_220.h>
   {MCC_MNC_SEPARATOR, "302221" },
   #include <config_data/carrier_config_302_221.h>
   {MCC_MNC_SEPARATOR, "302490" },
   #include <config_data/carrier_config_302_490.h>
   {MCC_MNC_SEPARATOR, "302500" },
   #include <config_data/carrier_config_302_500.h>
   {MCC_MNC_SEPARATOR, "302510" },
   #include <config_data/carrier_config_302_510.h>
   {MCC_MNC_SEPARATOR, "302610" },
   #include <config_data/carrier_config_302_610.h>
   {MCC_MNC_SEPARATOR, "302630" },
   #include <config_data/carrier_config_302_630.h>
   {MCC_MNC_SEPARATOR, "302640" },
   #include <config_data/carrier_config_302_640.h>
   {MCC_MNC_SEPARATOR, "302720" },
   #include <config_data/carrier_config_302_720.h>
   {MCC_MNC_SEPARATOR, "31000" },
   #include <config_data/carrier_config_310_00.h>
   {MCC_MNC_SEPARATOR, "310004" },
   #include <config_data/carrier_config_310_004.h>
   {MCC_MNC_SEPARATOR, "310005" },
   #include <config_data/carrier_config_310_005.h>
   {MCC_MNC_SEPARATOR, "310006" },
   #include <config_data/carrier_config_310_006.h>
   {MCC_MNC_SEPARATOR, "310010" },
   #include <config_data/carrier_config_310_010.h>
   {MCC_MNC_SEPARATOR, "310012" },
   #include <config_data/carrier_config_310_012.h>
   {MCC_MNC_SEPARATOR, "310013" },
   #include <config_data/carrier_config_310_013.h>
   {MCC_MNC_SEPARATOR, "310030" },
   #include <config_data/carrier_config_310_030.h>
   {MCC_MNC_SEPARATOR, "310070" },
   #include <config_data/carrier_config_310_070.h>
   {MCC_MNC_SEPARATOR, "310090" },
   #include <config_data/carrier_config_310_090.h>
   {MCC_MNC_SEPARATOR, "310120" },
   #include <config_data/carrier_config_310_120.h>
   {MCC_MNC_SEPARATOR, "310150" },
   #include <config_data/carrier_config_310_150.h>
   { MCC_MNC_SEPARATOR, "310160" },  /* TMO US */
   #include <config_data/carrier_config_310_160.h>
   {MCC_MNC_SEPARATOR, "310170" },
   #include <config_data/carrier_config_310_170.h>
   { MCC_MNC_SEPARATOR, "310200" },  /* TMO US */
   #include <config_data/carrier_config_310_200.h>
   { MCC_MNC_SEPARATOR, "310210" },  /* TMO US */
   #include <config_data/carrier_config_310_210.h>
   { MCC_MNC_SEPARATOR, "310220" },  /* TMO US */
   #include <config_data/carrier_config_310_220.h>
   { MCC_MNC_SEPARATOR, "310230" },  /* TMO US */
   #include <config_data/carrier_config_310_230.h>
   { MCC_MNC_SEPARATOR, "310240" },  /* TMO US */
   #include <config_data/carrier_config_310_240.h>
   { MCC_MNC_SEPARATOR, "310250" },  /* TMO US */
   #include <config_data/carrier_config_310_250.h>
   { MCC_MNC_SEPARATOR, "310260" },  /* TMO US */
   #include <config_data/carrier_config_310_260.h>
   { MCC_MNC_SEPARATOR, "310270" },  /* TMO US */
   #include <config_data/carrier_config_310_270.h>
   {MCC_MNC_SEPARATOR, "310280" },
   #include <config_data/carrier_config_310_280.h>
   {MCC_MNC_SEPARATOR, "310300" },
   #include <config_data/carrier_config_310_300.h>
   { MCC_MNC_SEPARATOR, "310310" },  /* TMO US */
   #include <config_data/carrier_config_310_310.h>
   { MCC_MNC_SEPARATOR, "310490" },  /* TMO US */
   #include <config_data/carrier_config_310_490.h>
   {MCC_MNC_SEPARATOR, "310350" },
   #include <config_data/carrier_config_310_350.h>
   {MCC_MNC_SEPARATOR, "310380" },
   #include <config_data/carrier_config_310_380.h>
   {MCC_MNC_SEPARATOR, "310410" },
   #include <config_data/carrier_config_310_410.h>
   {MCC_MNC_SEPARATOR, "310450" },
   #include <config_data/carrier_config_310_450.h>
   {MCC_MNC_SEPARATOR, "310530" },
   #include <config_data/carrier_config_310_530.h>
   {MCC_MNC_SEPARATOR, "310560" },
   #include <config_data/carrier_config_310_560.h>
   { MCC_MNC_SEPARATOR, "310580" },  /* TMO US */
   #include <config_data/carrier_config_310_580.h>
   {MCC_MNC_SEPARATOR, "310590" },
   #include <config_data/carrier_config_310_590.h>
   { MCC_MNC_SEPARATOR, "310660" },  /* TMO US */
   #include <config_data/carrier_config_310_660.h>
   {MCC_MNC_SEPARATOR, "310640" },
   #include <config_data/carrier_config_310_640.h>
   {MCC_MNC_SEPARATOR, "310680" },
   #include <config_data/carrier_config_310_680.h>
   {MCC_MNC_SEPARATOR, "310820" },
   #include <config_data/carrier_config_310_820.h>
   {MCC_MNC_SEPARATOR, "310890" },
   #include <config_data/carrier_config_310_890.h>
   {MCC_MNC_SEPARATOR, "310910" },
   #include <config_data/carrier_config_310_910.h>
   { MCC_MNC_SEPARATOR, "310800" },  /* TMO US */
   #include <config_data/carrier_config_310_800.h>
   { MCC_MNC_SEPARATOR, "311480" },
   #include <config_data/carrier_config_311_480.h>
   { MCC_MNC_SEPARATOR, "46001" },   /*CMCC*/
   #include <config_data/carrier_config_460_01.h>
   { MCC_MNC_SEPARATOR, "46002" },   /*CU*/
   #include <config_data/carrier_config_460_02.h>
   { MCC_MNC_SEPARATOR, "46601" },   /*FET*/
   #include <config_data/carrier_config_466_01.h>
   { MCC_MNC_SEPARATOR, "46692" },  /* CHT */
   #include <config_data/carrier_config_466_92.h>
   {MCC_MNC_SEPARATOR, "311012" },
   #include <config_data/carrier_config_311_012.h>
   {MCC_MNC_SEPARATOR, "311180" },
   #include <config_data/carrier_config_311_180.h>
   {MCC_MNC_SEPARATOR, "31100" },
   #include <config_data/carrier_config_311_00.h>
   {MCC_MNC_SEPARATOR, "311220" },
   #include <config_data/carrier_config_311_220.h>
   {MCC_MNC_SEPARATOR, "311221" },
   #include <config_data/carrier_config_311_221.h>
   {MCC_MNC_SEPARATOR, "311222" },
   #include <config_data/carrier_config_311_222.h>
   {MCC_MNC_SEPARATOR, "311223" },
   #include <config_data/carrier_config_311_223.h>
   {MCC_MNC_SEPARATOR, "311224" },
   #include <config_data/carrier_config_311_224.h>
   {MCC_MNC_SEPARATOR, "311225" },
   #include <config_data/carrier_config_311_225.h>
   {MCC_MNC_SEPARATOR, "311226" },
   #include <config_data/carrier_config_311_226.h>
   {MCC_MNC_SEPARATOR, "311227" },
   #include <config_data/carrier_config_311_227.h>
   {MCC_MNC_SEPARATOR, "311228" },
   #include <config_data/carrier_config_311_228.h>
   {MCC_MNC_SEPARATOR, "311229" },
   #include <config_data/carrier_config_311_229.h>
   {MCC_MNC_SEPARATOR, "311481" },
   #include <config_data/carrier_config_311_481.h>
   {MCC_MNC_SEPARATOR, "311482" },
   #include <config_data/carrier_config_311_482.h>
   {MCC_MNC_SEPARATOR, "311483" },
   #include <config_data/carrier_config_311_483.h>
   {MCC_MNC_SEPARATOR, "311484" },
   #include <config_data/carrier_config_311_484.h>
   {MCC_MNC_SEPARATOR, "311485" },
   #include <config_data/carrier_config_311_485.h>
   {MCC_MNC_SEPARATOR, "311486" },
   #include <config_data/carrier_config_311_486.h>
   {MCC_MNC_SEPARATOR, "311487" },
   #include <config_data/carrier_config_311_487.h>
   {MCC_MNC_SEPARATOR, "311488" },
   #include <config_data/carrier_config_311_488.h>
   {MCC_MNC_SEPARATOR, "311489" },
   #include <config_data/carrier_config_311_489.h>
   {MCC_MNC_SEPARATOR, "311580" },
   #include <config_data/carrier_config_311_580.h>
   {MCC_MNC_SEPARATOR, "311581" },
   #include <config_data/carrier_config_311_581.h>
   {MCC_MNC_SEPARATOR, "311582" },
   #include <config_data/carrier_config_311_582.h>
   {MCC_MNC_SEPARATOR, "311583" },
   #include <config_data/carrier_config_311_583.h>
   {MCC_MNC_SEPARATOR, "311584" },
   #include <config_data/carrier_config_311_584.h>
   {MCC_MNC_SEPARATOR, "311585" },
   #include <config_data/carrier_config_311_585.h>
   {MCC_MNC_SEPARATOR, "311586" },
   #include <config_data/carrier_config_311_586.h>
   {MCC_MNC_SEPARATOR, "311587" },
   #include <config_data/carrier_config_311_587.h>
   {MCC_MNC_SEPARATOR, "311588" },
   #include <config_data/carrier_config_311_588.h>
   {MCC_MNC_SEPARATOR, "311589" },
   #include <config_data/carrier_config_311_589.h>
   {MCC_MNC_SEPARATOR, "311870" },
   #include <config_data/carrier_config_311_870.h>
   {MCC_MNC_SEPARATOR, "311880" },
   #include <config_data/carrier_config_311_880.h>
   {MCC_MNC_SEPARATOR, "311940" },
   #include <config_data/carrier_config_311_940.h>
   {MCC_MNC_SEPARATOR, "312010" },
   #include <config_data/carrier_config_312_010.h>
   {MCC_MNC_SEPARATOR, "312190" },
   #include <config_data/carrier_config_312_190.h>
   {MCC_MNC_SEPARATOR, "312530" },
   #include <config_data/carrier_config_312_530.h>
   {MCC_MNC_SEPARATOR, "312670" },
   #include <config_data/carrier_config_312_670.h>
   {MCC_MNC_SEPARATOR, "313070" },
   #include <config_data/carrier_config_313_070.h>
   {MCC_MNC_SEPARATOR, "313100" },
   #include <config_data/carrier_config_313_100.h>
   {MCC_MNC_SEPARATOR, "313390" }, /* Altice US*/
   #include <config_data/carrier_config_313_390.h>
   {MCC_MNC_SEPARATOR, "334020" },
   #include <config_data/carrier_config_334_020.h>
   {MCC_MNC_SEPARATOR, "33403" },
   #include <config_data/carrier_config_334_03.h>
   {MCC_MNC_SEPARATOR, "334030" },
   #include <config_data/carrier_config_334_030.h>
   {MCC_MNC_SEPARATOR, "334050" },
   #include <config_data/carrier_config_334_050.h>
   {MCC_MNC_SEPARATOR, "334090" },
   #include <config_data/carrier_config_334_090.h>
   {MCC_MNC_SEPARATOR, "334140" },
   #include <config_data/carrier_config_334_140.h>
   {MCC_MNC_SEPARATOR, "34001" },
   #include <config_data/carrier_config_340_01.h>
   {MCC_MNC_SEPARATOR, "34420" },
   #include <config_data/carrier_config_344_20.h>
   {MCC_MNC_SEPARATOR, "37001" },
   #include <config_data/carrier_config_370_01.h>
   {MCC_MNC_SEPARATOR, "37002" },
   #include <config_data/carrier_config_370_02.h>
   {MCC_MNC_SEPARATOR, "40401" },
   #include <config_data/carrier_config_404_01.h>
   {MCC_MNC_SEPARATOR, "40402" },
   #include <config_data/carrier_config_404_02.h>
   {MCC_MNC_SEPARATOR, "40403" },
   #include <config_data/carrier_config_404_03.h>
   {MCC_MNC_SEPARATOR, "40404" },
   #include <config_data/carrier_config_404_04.h>
   {MCC_MNC_SEPARATOR, "40405" },
   #include <config_data/carrier_config_404_05.h>
   {MCC_MNC_SEPARATOR, "40407" },
   #include <config_data/carrier_config_404_07.h>
   {MCC_MNC_SEPARATOR, "40410" },
   #include <config_data/carrier_config_404_10.h>
   {MCC_MNC_SEPARATOR, "40411" },
   #include <config_data/carrier_config_404_11.h>
   {MCC_MNC_SEPARATOR, "40412" },
   #include <config_data/carrier_config_404_12.h>
   {MCC_MNC_SEPARATOR, "40413" },
   #include <config_data/carrier_config_404_13.h>
   {MCC_MNC_SEPARATOR, "40414" },
   #include <config_data/carrier_config_404_14.h>
   {MCC_MNC_SEPARATOR, "40415" },
   #include <config_data/carrier_config_404_15.h>
   {MCC_MNC_SEPARATOR, "40416" },
   #include <config_data/carrier_config_404_16.h>
   {MCC_MNC_SEPARATOR, "40419" },
   #include <config_data/carrier_config_404_19.h>
   {MCC_MNC_SEPARATOR, "40420" },
   #include <config_data/carrier_config_404_20.h>
   {MCC_MNC_SEPARATOR, "40422" },
   #include <config_data/carrier_config_404_22.h>
   {MCC_MNC_SEPARATOR, "40424" },
   #include <config_data/carrier_config_404_24.h>
   {MCC_MNC_SEPARATOR, "40427" },
   #include <config_data/carrier_config_404_27.h>
   {MCC_MNC_SEPARATOR, "40430" },
   #include <config_data/carrier_config_404_30.h>
   {MCC_MNC_SEPARATOR, "40431" },
   #include <config_data/carrier_config_404_31.h>
   {MCC_MNC_SEPARATOR, "40434" },
   #include <config_data/carrier_config_404_34.h>
   {MCC_MNC_SEPARATOR, "40438" },
   #include <config_data/carrier_config_404_38.h>
   {MCC_MNC_SEPARATOR, "40451" },
   #include <config_data/carrier_config_404_51.h>
   {MCC_MNC_SEPARATOR, "40453" },
   #include <config_data/carrier_config_404_53.h>
   {MCC_MNC_SEPARATOR, "40454" },
   #include <config_data/carrier_config_404_54.h>
   {MCC_MNC_SEPARATOR, "40455" },
   #include <config_data/carrier_config_404_55.h>
   {MCC_MNC_SEPARATOR, "40457" },
   #include <config_data/carrier_config_404_57.h>
   {MCC_MNC_SEPARATOR, "40458" },
   #include <config_data/carrier_config_404_58.h>
   {MCC_MNC_SEPARATOR, "40459" },
   #include <config_data/carrier_config_404_59.h>
   {MCC_MNC_SEPARATOR, "40462" },
   #include <config_data/carrier_config_404_62.h>
   {MCC_MNC_SEPARATOR, "40464" },
   #include <config_data/carrier_config_404_64.h>
   {MCC_MNC_SEPARATOR, "40466" },
   #include <config_data/carrier_config_404_66.h>
   {MCC_MNC_SEPARATOR, "40471" },
   #include <config_data/carrier_config_404_71.h>
   {MCC_MNC_SEPARATOR, "40472" },
   #include <config_data/carrier_config_404_72.h>
   {MCC_MNC_SEPARATOR, "40473" },
   #include <config_data/carrier_config_404_73.h>
   {MCC_MNC_SEPARATOR, "40474" },
   #include <config_data/carrier_config_404_74.h>
   {MCC_MNC_SEPARATOR, "40475" },
   #include <config_data/carrier_config_404_75.h>
   {MCC_MNC_SEPARATOR, "40476" },
   #include <config_data/carrier_config_404_76.h>
   {MCC_MNC_SEPARATOR, "40477" },
   #include <config_data/carrier_config_404_77.h>
   {MCC_MNC_SEPARATOR, "40479" },
   #include <config_data/carrier_config_404_79.h>
   {MCC_MNC_SEPARATOR, "40480" },
   #include <config_data/carrier_config_404_80.h>
   {MCC_MNC_SEPARATOR, "40481" },
   #include <config_data/carrier_config_404_81.h>
   {MCC_MNC_SEPARATOR, "40440" },
   #include <config_data/carrier_config_404_40.h>
   {MCC_MNC_SEPARATOR, "40443" },
   #include <config_data/carrier_config_404_43.h>
   {MCC_MNC_SEPARATOR, "40444" },
   #include <config_data/carrier_config_404_44.h>
   {MCC_MNC_SEPARATOR, "40445" },
   #include <config_data/carrier_config_404_45.h>
   {MCC_MNC_SEPARATOR, "40446" },
   #include <config_data/carrier_config_404_46.h>
   {MCC_MNC_SEPARATOR, "40449" },
   #include <config_data/carrier_config_404_49.h>
   {MCC_MNC_SEPARATOR, "40456" },
   #include <config_data/carrier_config_404_56.h>
   {MCC_MNC_SEPARATOR, "40460" },
   #include <config_data/carrier_config_404_60.h>
   {MCC_MNC_SEPARATOR, "40470" },
   #include <config_data/carrier_config_404_70.h>
   {MCC_MNC_SEPARATOR, "40478" },
   #include <config_data/carrier_config_404_78.h>
   {MCC_MNC_SEPARATOR, "40482" },
   #include <config_data/carrier_config_404_82.h>
   {MCC_MNC_SEPARATOR, "40484" },
   #include <config_data/carrier_config_404_84.h>
   {MCC_MNC_SEPARATOR, "40486" },
   #include <config_data/carrier_config_404_86.h>
   {MCC_MNC_SEPARATOR, "40487" },
   #include <config_data/carrier_config_404_87.h>
   {MCC_MNC_SEPARATOR, "40488" },
   #include <config_data/carrier_config_404_88.h>
   {MCC_MNC_SEPARATOR, "40489" },
   #include <config_data/carrier_config_404_89.h>
   {MCC_MNC_SEPARATOR, "40490" },
   #include <config_data/carrier_config_404_90.h>
   {MCC_MNC_SEPARATOR, "40492" },
   #include <config_data/carrier_config_404_92.h>
   {MCC_MNC_SEPARATOR, "40493" },
   #include <config_data/carrier_config_404_93.h>
   {MCC_MNC_SEPARATOR, "40494" },
   #include <config_data/carrier_config_404_94.h>
   {MCC_MNC_SEPARATOR, "40495" },
   #include <config_data/carrier_config_404_95.h>
   {MCC_MNC_SEPARATOR, "40496" },
   #include <config_data/carrier_config_404_96.h>
   {MCC_MNC_SEPARATOR, "40497" },
   #include <config_data/carrier_config_404_97.h>
   {MCC_MNC_SEPARATOR, "40498" },
   #include <config_data/carrier_config_404_98.h>
   {MCC_MNC_SEPARATOR, "40525" },
   #include <config_data/carrier_config_405_25.h>
   {MCC_MNC_SEPARATOR, "40527" },
   #include <config_data/carrier_config_405_27.h>
   {MCC_MNC_SEPARATOR, "40530" },
   #include <config_data/carrier_config_405_30.h>
   {MCC_MNC_SEPARATOR, "40531" },
   #include <config_data/carrier_config_405_31.h>
   {MCC_MNC_SEPARATOR, "40532" },
   #include <config_data/carrier_config_405_32.h>
   {MCC_MNC_SEPARATOR, "40534" },
   #include <config_data/carrier_config_405_34.h>
   {MCC_MNC_SEPARATOR, "40535" },
   #include <config_data/carrier_config_405_35.h>
   {MCC_MNC_SEPARATOR, "40536" },
   #include <config_data/carrier_config_405_36.h>
   {MCC_MNC_SEPARATOR, "40537" },
   #include <config_data/carrier_config_405_37.h>
   {MCC_MNC_SEPARATOR, "40538" },
   #include <config_data/carrier_config_405_38.h>
   {MCC_MNC_SEPARATOR, "40539" },
   #include <config_data/carrier_config_405_39.h>
   {MCC_MNC_SEPARATOR, "40541" },
   #include <config_data/carrier_config_405_41.h>
   {MCC_MNC_SEPARATOR, "40542" },
   #include <config_data/carrier_config_405_42.h>
   {MCC_MNC_SEPARATOR, "40543" },
   #include <config_data/carrier_config_405_43.h>
   {MCC_MNC_SEPARATOR, "40544" },
   #include <config_data/carrier_config_405_44.h>
   {MCC_MNC_SEPARATOR, "40545" },
   #include <config_data/carrier_config_405_45.h>
   {MCC_MNC_SEPARATOR, "40546" },
   #include <config_data/carrier_config_405_46.h>
   {MCC_MNC_SEPARATOR, "40547" },
   #include <config_data/carrier_config_405_47.h>
   {MCC_MNC_SEPARATOR, "405030" },
   #include <config_data/carrier_config_405_030.h>
   {MCC_MNC_SEPARATOR, "405035" },
   #include <config_data/carrier_config_405_035.h>
   {MCC_MNC_SEPARATOR, "405036" },
   #include <config_data/carrier_config_405_036.h>
   {MCC_MNC_SEPARATOR, "405037" },
   #include <config_data/carrier_config_405_037.h>
   {MCC_MNC_SEPARATOR, "405038" },
   #include <config_data/carrier_config_405_038.h>
   {MCC_MNC_SEPARATOR, "405039" },
   #include <config_data/carrier_config_405_039.h>
   {MCC_MNC_SEPARATOR, "405044" },
   #include <config_data/carrier_config_405_044.h>
   {MCC_MNC_SEPARATOR, "40551" },
   #include <config_data/carrier_config_405_51.h>
   {MCC_MNC_SEPARATOR, "40552" },
   #include <config_data/carrier_config_405_52.h>
   {MCC_MNC_SEPARATOR, "40553" },
   #include <config_data/carrier_config_405_53.h>
   {MCC_MNC_SEPARATOR, "40554" },
   #include <config_data/carrier_config_405_54.h>
   {MCC_MNC_SEPARATOR, "40555" },
   #include <config_data/carrier_config_405_55.h>
   {MCC_MNC_SEPARATOR, "40556" },
   #include <config_data/carrier_config_405_56.h>
   {MCC_MNC_SEPARATOR, "40566" },
   #include <config_data/carrier_config_405_66.h>
   {MCC_MNC_SEPARATOR, "40567" },
   #include <config_data/carrier_config_405_67.h>
   {MCC_MNC_SEPARATOR, "40570" },
   #include <config_data/carrier_config_405_70.h>
   {MCC_MNC_SEPARATOR, "405750" },
   #include <config_data/carrier_config_405_750.h>
   {MCC_MNC_SEPARATOR, "405751" },
   #include <config_data/carrier_config_405_751.h>
   {MCC_MNC_SEPARATOR, "405752" },
   #include <config_data/carrier_config_405_752.h>
   {MCC_MNC_SEPARATOR, "405753" },
   #include <config_data/carrier_config_405_753.h>
   {MCC_MNC_SEPARATOR, "405754" },
   #include <config_data/carrier_config_405_754.h>
   {MCC_MNC_SEPARATOR, "405755" },
   #include <config_data/carrier_config_405_755.h>
   {MCC_MNC_SEPARATOR, "405756" },
   #include <config_data/carrier_config_405_756.h>
   {MCC_MNC_SEPARATOR, "405799" },
   #include <config_data/carrier_config_405_799.h>
   {MCC_MNC_SEPARATOR, "405818" },
   #include <config_data/carrier_config_405_818.h>
   {MCC_MNC_SEPARATOR, "405819" },
   #include <config_data/carrier_config_405_819.h>
   {MCC_MNC_SEPARATOR, "405840" },
   #include <config_data/carrier_config_405_840.h>
   {MCC_MNC_SEPARATOR, "405845" },
   #include <config_data/carrier_config_405_845.h>
   {MCC_MNC_SEPARATOR, "405846" },
   #include <config_data/carrier_config_405_846.h>
   {MCC_MNC_SEPARATOR, "405848" },
   #include <config_data/carrier_config_405_848.h>
   {MCC_MNC_SEPARATOR, "405849" },
   #include <config_data/carrier_config_405_849.h>
   {MCC_MNC_SEPARATOR, "405850" },
   #include <config_data/carrier_config_405_850.h>
   {MCC_MNC_SEPARATOR, "405852" },
   #include <config_data/carrier_config_405_852.h>
   {MCC_MNC_SEPARATOR, "405853" },
   #include <config_data/carrier_config_405_853.h>
   {MCC_MNC_SEPARATOR, "405854" },
   #include <config_data/carrier_config_405_854.h>
   {MCC_MNC_SEPARATOR, "405855" },
   #include <config_data/carrier_config_405_855.h>
   {MCC_MNC_SEPARATOR, "405856" },
   #include <config_data/carrier_config_405_856.h>
   {MCC_MNC_SEPARATOR, "405857" },
   #include <config_data/carrier_config_405_857.h>
   {MCC_MNC_SEPARATOR, "405858" },
   #include <config_data/carrier_config_405_858.h>
   {MCC_MNC_SEPARATOR, "405859" },
   #include <config_data/carrier_config_405_859.h>
   {MCC_MNC_SEPARATOR, "405860" },
   #include <config_data/carrier_config_405_860.h>
   {MCC_MNC_SEPARATOR, "405861" },
   #include <config_data/carrier_config_405_861.h>
   {MCC_MNC_SEPARATOR, "405862" },
   #include <config_data/carrier_config_405_862.h>
   {MCC_MNC_SEPARATOR, "405863" },
   #include <config_data/carrier_config_405_863.h>
   {MCC_MNC_SEPARATOR, "405864" },
   #include <config_data/carrier_config_405_864.h>
   {MCC_MNC_SEPARATOR, "405865" },
   #include <config_data/carrier_config_405_865.h>
   {MCC_MNC_SEPARATOR, "405866" },
   #include <config_data/carrier_config_405_866.h>
   {MCC_MNC_SEPARATOR, "405867" },
   #include <config_data/carrier_config_405_867.h>
   {MCC_MNC_SEPARATOR, "405868" },
   #include <config_data/carrier_config_405_868.h>
   {MCC_MNC_SEPARATOR, "405869" },
   #include <config_data/carrier_config_405_869.h>
   {MCC_MNC_SEPARATOR, "405870" },
   #include <config_data/carrier_config_405_870.h>
   {MCC_MNC_SEPARATOR, "405871" },
   #include <config_data/carrier_config_405_871.h>
   {MCC_MNC_SEPARATOR, "405872" },
   #include <config_data/carrier_config_405_872.h>
   {MCC_MNC_SEPARATOR, "405873" },
   #include <config_data/carrier_config_405_873.h>
   {MCC_MNC_SEPARATOR, "405874" },
   #include <config_data/carrier_config_405_874.h>
   {MCC_MNC_SEPARATOR, "405876" },
   #include <config_data/carrier_config_405_876.h>
   {MCC_MNC_SEPARATOR, "405879" },
   #include <config_data/carrier_config_405_879.h>
   {MCC_MNC_SEPARATOR, "405927" },
   #include <config_data/carrier_config_405_927.h>
   {MCC_MNC_SEPARATOR, "405929" },
   #include <config_data/carrier_config_405_929.h>
   {MCC_MNC_SEPARATOR, "41004" },
   #include <config_data/carrier_config_410_04.h>
   {MCC_MNC_SEPARATOR, "41006" },
   #include <config_data/carrier_config_410_06.h>
   {MCC_MNC_SEPARATOR, "41301" },
   #include <config_data/carrier_config_413_01.h>
   {MCC_MNC_SEPARATOR, "41302" },
   #include <config_data/carrier_config_413_02.h>
   {MCC_MNC_SEPARATOR, "41406" },
   #include <config_data/carrier_config_414_06.h>
   {MCC_MNC_SEPARATOR, "41409" },
   #include <config_data/carrier_config_414_09.h>
   {MCC_MNC_SEPARATOR, "41866" },
   #include <config_data/carrier_config_418_66.h>
   {MCC_MNC_SEPARATOR, "42001" },
   #include <config_data/carrier_config_420_01.h>
   {MCC_MNC_SEPARATOR, "42003" },
   #include <config_data/carrier_config_420_03.h>
   {MCC_MNC_SEPARATOR, "42004" },
   #include <config_data/carrier_config_420_04.h>
   {MCC_MNC_SEPARATOR, "42402" },
   #include <config_data/carrier_config_424_02.h>
   {MCC_MNC_SEPARATOR, "42403" },
   #include <config_data/carrier_config_424_03.h>
   {MCC_MNC_SEPARATOR, "44007" },
   #include <config_data/carrier_config_440_07.h>
   {MCC_MNC_SEPARATOR, "44008" },
   #include <config_data/carrier_config_440_08.h>
   {MCC_MNC_SEPARATOR, "44010" },
   #include <config_data/carrier_config_440_10.h>
   {MCC_MNC_SEPARATOR, "44020" },
   #include <config_data/carrier_config_440_20.h>
   {MCC_MNC_SEPARATOR, "44050" },
   #include <config_data/carrier_config_440_50.h>
   {MCC_MNC_SEPARATOR, "44051" },
   #include <config_data/carrier_config_440_51.h>
   {MCC_MNC_SEPARATOR, "44052" },
   #include <config_data/carrier_config_440_52.h>
   {MCC_MNC_SEPARATOR, "44053" },
   #include <config_data/carrier_config_440_53.h>
   {MCC_MNC_SEPARATOR, "44054" },
   #include <config_data/carrier_config_440_54.h>
   {MCC_MNC_SEPARATOR, "44055" },
   #include <config_data/carrier_config_440_55.h>
   {MCC_MNC_SEPARATOR, "44056" },
   #include <config_data/carrier_config_440_56.h>
   {MCC_MNC_SEPARATOR, "44070" },
   #include <config_data/carrier_config_440_70.h>
   {MCC_MNC_SEPARATOR, "44071" },
   #include <config_data/carrier_config_440_71.h>
   {MCC_MNC_SEPARATOR, "44072" },
   #include <config_data/carrier_config_440_72.h>
   {MCC_MNC_SEPARATOR, "44073" },
   #include <config_data/carrier_config_440_73.h>
   {MCC_MNC_SEPARATOR, "44074" },
   #include <config_data/carrier_config_440_74.h>
   {MCC_MNC_SEPARATOR, "44075" },
   #include <config_data/carrier_config_440_75.h>
   {MCC_MNC_SEPARATOR, "44076" },
   #include <config_data/carrier_config_440_76.h>
   {MCC_MNC_SEPARATOR, "44077" },
   #include <config_data/carrier_config_440_77.h>
   {MCC_MNC_SEPARATOR, "44078" },
   #include <config_data/carrier_config_440_78.h>
   {MCC_MNC_SEPARATOR, "44079" },
   #include <config_data/carrier_config_440_79.h>
   {MCC_MNC_SEPARATOR, "44088" },
   #include <config_data/carrier_config_440_88.h>
   {MCC_MNC_SEPARATOR, "44089" },
   #include <config_data/carrier_config_440_89.h>
   {MCC_MNC_SEPARATOR, "44170" },
   #include <config_data/carrier_config_441_70.h>
   {MCC_MNC_SEPARATOR, "45005" },
   #include <config_data/carrier_config_450_05.h>
   {MCC_MNC_SEPARATOR, "45006" },
   #include <config_data/carrier_config_450_06.h>
   {MCC_MNC_SEPARATOR, "45008" },
   #include <config_data/carrier_config_450_08.h>
   {MCC_MNC_SEPARATOR, "45202" }, /* Vinaphone Vietnam */
   #include <config_data/carrier_config_452_02.h>
   {MCC_MNC_SEPARATOR, "45204" },
   #include <config_data/carrier_config_452_04.h>
   {MCC_MNC_SEPARATOR, "45205" },
   #include <config_data/carrier_config_452_05.h>
   {MCC_MNC_SEPARATOR, "45400" },
   #include <config_data/carrier_config_454_00.h>
   {MCC_MNC_SEPARATOR, "45402" },
   #include <config_data/carrier_config_454_02.h>
   {MCC_MNC_SEPARATOR, "45403" },
   #include <config_data/carrier_config_454_03.h>
   {MCC_MNC_SEPARATOR, "45404" },
   #include <config_data/carrier_config_454_04.h>
   {MCC_MNC_SEPARATOR, "45406" },
   #include <config_data/carrier_config_454_06.h>
   {MCC_MNC_SEPARATOR, "45410" },
   #include <config_data/carrier_config_454_10.h>
   {MCC_MNC_SEPARATOR, "45412" },
   #include <config_data/carrier_config_454_12.h>
   {MCC_MNC_SEPARATOR, "45413" },
   #include <config_data/carrier_config_454_13.h>
   {MCC_MNC_SEPARATOR, "45415" },
   #include <config_data/carrier_config_454_15.h>
   {MCC_MNC_SEPARATOR, "45416" },
   #include <config_data/carrier_config_454_16.h>
   {MCC_MNC_SEPARATOR, "45417" },
   #include <config_data/carrier_config_454_17.h>
   {MCC_MNC_SEPARATOR, "45418" },
   #include <config_data/carrier_config_454_18.h>
   {MCC_MNC_SEPARATOR, "45419" },
   #include <config_data/carrier_config_454_19.h>
   {MCC_MNC_SEPARATOR, "45420" },
   #include <config_data/carrier_config_454_20.h>
   {MCC_MNC_SEPARATOR, "45501" },    /*CTM*/
   #include <config_data/carrier_config_455_01.h>
   {MCC_MNC_SEPARATOR, "45502" },
   #include <config_data/carrier_config_455_02.h>
   {MCC_MNC_SEPARATOR, "45503" },    /*THREE*/
   #include <config_data/carrier_config_455_03.h>
   {MCC_MNC_SEPARATOR, "45504" },    /*CTM*/
   #include <config_data/carrier_config_455_04.h>
   {MCC_MNC_SEPARATOR, "45505" },    /*THREE*/
   #include <config_data/carrier_config_455_05.h>
   {MCC_MNC_SEPARATOR, "45507" },
   #include <config_data/carrier_config_455_07.h>
   {MCC_MNC_SEPARATOR, "45606" },
   #include <config_data/carrier_config_456_06.h>
   {MCC_MNC_SEPARATOR, "45611" },
   #include <config_data/carrier_config_456_11.h>
   {MCC_MNC_SEPARATOR, "46000" },
   #include <config_data/carrier_config_460_00.h>
   {MCC_MNC_SEPARATOR, "46003" },
   #include <config_data/carrier_config_460_03.h>
   {MCC_MNC_SEPARATOR, "46004" },
   #include <config_data/carrier_config_460_04.h>
   {MCC_MNC_SEPARATOR, "46006" },
   #include <config_data/carrier_config_460_06.h>
   {MCC_MNC_SEPARATOR, "46007" },
   #include <config_data/carrier_config_460_07.h>
   {MCC_MNC_SEPARATOR, "46008" },
   #include <config_data/carrier_config_460_08.h>
   {MCC_MNC_SEPARATOR, "46009" },
   #include <config_data/carrier_config_460_09.h>
   {MCC_MNC_SEPARATOR, "46011" },
   #include <config_data/carrier_config_460_11.h>
   {MCC_MNC_SEPARATOR, "46012" },
   #include <config_data/carrier_config_460_12.h>
   {MCC_MNC_SEPARATOR, "46019" },
   #include <config_data/carrier_config_460_19.h>
   {MCC_MNC_SEPARATOR, "46605" },
   #include <config_data/carrier_config_466_05.h>
   {MCC_MNC_SEPARATOR, "46611" },
   #include <config_data/carrier_config_466_11.h>
   {MCC_MNC_SEPARATOR, "46688" },
   #include <config_data/carrier_config_466_88.h>
   {MCC_MNC_SEPARATOR, "46689" },
   #include <config_data/carrier_config_466_89.h>
   {MCC_MNC_SEPARATOR, "46697" },
   #include <config_data/carrier_config_466_97.h>
   {MCC_MNC_SEPARATOR, "47001" },
   #include <config_data/carrier_config_470_01.h>
   {MCC_MNC_SEPARATOR, "47002" },
   #include <config_data/carrier_config_470_02.h>
   {MCC_MNC_SEPARATOR, "47007" },
   #include <config_data/carrier_config_470_07.h>
   {MCC_MNC_SEPARATOR, "50208" },
   #include <config_data/carrier_config_502_08.h>
   {MCC_MNC_SEPARATOR, "50212" },
   #include <config_data/carrier_config_502_12.h>
   {MCC_MNC_SEPARATOR, "50213" },
   #include <config_data/carrier_config_502_13.h>
   {MCC_MNC_SEPARATOR, "502152" },
   #include <config_data/carrier_config_502_152.h>
   {MCC_MNC_SEPARATOR, "502153" },
   #include <config_data/carrier_config_502_153.h>
   {MCC_MNC_SEPARATOR, "50216" },
   #include <config_data/carrier_config_502_16.h>
   {MCC_MNC_SEPARATOR, "50218" },
   #include <config_data/carrier_config_502_18.h>
   {MCC_MNC_SEPARATOR, "50219" },
   #include <config_data/carrier_config_502_19.h>
   {MCC_MNC_SEPARATOR, "50501" },
   #include <config_data/carrier_config_505_01.h>
   {MCC_MNC_SEPARATOR, "50502" },
   #include <config_data/carrier_config_505_02.h>
   {MCC_MNC_SEPARATOR, "50503" },
   #include <config_data/carrier_config_505_03.h>
   {MCC_MNC_SEPARATOR, "50506" },
   #include <config_data/carrier_config_505_06.h>
   {MCC_MNC_SEPARATOR, "50571" },
   #include <config_data/carrier_config_505_71.h>
   {MCC_MNC_SEPARATOR, "50572" },
   #include <config_data/carrier_config_505_72.h>
   {MCC_MNC_SEPARATOR, "51001" },
   #include <config_data/carrier_config_510_01.h>
   {MCC_MNC_SEPARATOR, "51009" },
   #include <config_data/carrier_config_510_09.h>
   {MCC_MNC_SEPARATOR, "51010" },
   #include <config_data/carrier_config_510_10.h>
   {MCC_MNC_SEPARATOR, "51021" },
   #include <config_data/carrier_config_510_21.h>
   {MCC_MNC_SEPARATOR, "51028" },
   #include <config_data/carrier_config_510_28.h>
   {MCC_MNC_SEPARATOR, "51503" },
   #include <config_data/carrier_config_515_03.h>
   {MCC_MNC_SEPARATOR, "51502" },
   #include <config_data/carrier_config_515_02.h>
   {MCC_MNC_SEPARATOR, "51566" }, /* DITO Philippines */
   #include <config_data/carrier_config_515_66.h>
   {MCC_MNC_SEPARATOR, "52000" },
   #include <config_data/carrier_config_520_00.h>
   {MCC_MNC_SEPARATOR, "52003" },
   #include <config_data/carrier_config_520_03.h>
   {MCC_MNC_SEPARATOR, "52004" },
   #include <config_data/carrier_config_520_04.h>
   {MCC_MNC_SEPARATOR, "52005" },
   #include <config_data/carrier_config_520_05.h>
   {MCC_MNC_SEPARATOR, "52501" },
   #include <config_data/carrier_config_525_01.h>
   {MCC_MNC_SEPARATOR, "52503" },
   #include <config_data/carrier_config_525_03.h>
   {MCC_MNC_SEPARATOR, "52505" },
   #include <config_data/carrier_config_525_05.h>
   {MCC_MNC_SEPARATOR, "52510" },
   #include <config_data/carrier_config_525_10.h>
   {MCC_MNC_SEPARATOR, "53001" },
   #include <config_data/carrier_config_530_01.h>
   {MCC_MNC_SEPARATOR, "53005" },
   #include <config_data/carrier_config_530_05.h>
   {MCC_MNC_SEPARATOR, "53024" },
   #include <config_data/carrier_config_530_24.h>
   {MCC_MNC_SEPARATOR, "530099" },
   #include <config_data/carrier_config_530_099.h>
   {MCC_MNC_SEPARATOR, "60201" },
   #include <config_data/carrier_config_602_01.h>
   {MCC_MNC_SEPARATOR, "60202" },
   #include <config_data/carrier_config_602_02.h>
   {MCC_MNC_SEPARATOR, "60203" },
   #include <config_data/carrier_config_602_03.h>
   {MCC_MNC_SEPARATOR, "60204" },
   #include <config_data/carrier_config_602_04.h>
   {MCC_MNC_SEPARATOR, "62127" },
   #include <config_data/carrier_config_621_27.h>
   {MCC_MNC_SEPARATOR, "62140" },
   #include <config_data/carrier_config_621_40.h>
    {MCC_MNC_SEPARATOR, "63902" },  /*Safaricom*/
   #include <config_data/carrier_config_639_02.h>
   {MCC_MNC_SEPARATOR, "63910" },
   #include <config_data/carrier_config_639_10.h>
   {MCC_MNC_SEPARATOR, "64011" },
   #include <config_data/carrier_config_640_11.h>
   {MCC_MNC_SEPARATOR, "64133" },
   #include <config_data/carrier_config_641_33.h>
   {MCC_MNC_SEPARATOR, "64505" },
   #include <config_data/carrier_config_645_05.h>
   {MCC_MNC_SEPARATOR, "65302" },
   #include <config_data/carrier_config_653_02.h>
   {MCC_MNC_SEPARATOR, "65501" },
   #include <config_data/carrier_config_655_01.h>
   {MCC_MNC_SEPARATOR, "65502" },
   #include <config_data/carrier_config_655_02.h>
   {MCC_MNC_SEPARATOR, "65507" },
   #include <config_data/carrier_config_655_07.h>
   {MCC_MNC_SEPARATOR, "65510" },
   #include <config_data/carrier_config_655_10.h>
   {MCC_MNC_SEPARATOR, "70401" },
   #include <config_data/carrier_config_704_01.h>
   {MCC_MNC_SEPARATOR, "70402" },
   #include <config_data/carrier_config_704_02.h>
   {MCC_MNC_SEPARATOR, "71401" },
   #include <config_data/carrier_config_714_01.h>
   {MCC_MNC_SEPARATOR, "714010" },
   #include <config_data/carrier_config_714_010.h>
   {MCC_MNC_SEPARATOR, "71606" },
   #include <config_data/carrier_config_716_06.h>
   {MCC_MNC_SEPARATOR, "71610" },
   #include <config_data/carrier_config_716_10.h>
   {MCC_MNC_SEPARATOR, "71617" },
   #include <config_data/carrier_config_716_17.h>
   {MCC_MNC_SEPARATOR, "72207" },
   #include <config_data/carrier_config_722_07.h>
   {MCC_MNC_SEPARATOR, "722310" },
   #include <config_data/carrier_config_722_310.h>
   {MCC_MNC_SEPARATOR, "72234" },
   #include <config_data/carrier_config_722_34.h>
   {MCC_MNC_SEPARATOR, "72402" },
   #include <config_data/carrier_config_724_02.h>
   {MCC_MNC_SEPARATOR, "72403" },
   #include <config_data/carrier_config_724_03.h>
   {MCC_MNC_SEPARATOR, "72404" },
   #include <config_data/carrier_config_724_04.h>
   {MCC_MNC_SEPARATOR, "72405" },
   #include <config_data/carrier_config_724_05.h>
   {MCC_MNC_SEPARATOR, "72406" },
   #include <config_data/carrier_config_724_06.h>
   {MCC_MNC_SEPARATOR, "72410" },
   #include <config_data/carrier_config_724_10.h>
   {MCC_MNC_SEPARATOR, "72411" },
   #include <config_data/carrier_config_724_11.h>
   {MCC_MNC_SEPARATOR, "72423" },
   #include <config_data/carrier_config_724_23.h>
   {MCC_MNC_SEPARATOR, "73001" },
   #include <config_data/carrier_config_730_01.h>
   {MCC_MNC_SEPARATOR, "73002" },
   #include <config_data/carrier_config_730_02.h>
   {MCC_MNC_SEPARATOR, "73003" },
   #include <config_data/carrier_config_730_03.h>
   {MCC_MNC_SEPARATOR, "73009" },
   #include <config_data/carrier_config_730_09.h>
   {MCC_MNC_SEPARATOR, "732101" },
   #include <config_data/carrier_config_732_101.h>
   {MCC_MNC_SEPARATOR, "732103" },
   #include <config_data/carrier_config_732_103.h>
   {MCC_MNC_SEPARATOR, "732111" },
   #include <config_data/carrier_config_732_111.h>
   {MCC_MNC_SEPARATOR, "732123" },
   #include <config_data/carrier_config_732_123.h>
   {MCC_MNC_SEPARATOR, "732130" },
   #include <config_data/carrier_config_732_130.h>
   {MCC_MNC_SEPARATOR, "73602" },
   #include <config_data/carrier_config_736_02.h>
   {MCC_MNC_SEPARATOR, "74000" },
   #include <config_data/carrier_config_740_00.h>
   {MCC_MNC_SEPARATOR, "74001" },
   #include <config_data/carrier_config_740_01.h>
   {MCC_MNC_SEPARATOR, "74807" },
   #include <config_data/carrier_config_748_07.h>
   {MCC_MNC_SEPARATOR, "74810" },
   #include <config_data/carrier_config_748_10.h>
   {MCC_MNC_SEPARATOR, "748010" },
   #include <config_data/carrier_config_748_010.h>

   /* End indication */
   { MCC_MNC_END_TAG, NULL },
};

/**
  * Return the start index of CarrierConfigValue array for specific MCC/MNC
  * @param mccmnc  mobile country code and mobile network code string
  * @retrun the start index if found else return -1
  */
int getStartIndex(const char* mccmnc) {
    int i = 0, found = 0;

    while (carierConfigData[i].key != MCC_MNC_END_TAG) {
        if ((carierConfigData[i].key == MCC_MNC_SEPARATOR) &&
             (!strcmp(carierConfigData[i].value, mccmnc))) {
            found = 1;
            break;
        }
        i++;
    }

    return (found == 1) ? i : -1;
}


/**
 * Retrieve the count of keys for specific MCC/MNC
 * @param mccmnc  mobile country code and mobile network code string
 * @retrun the count of keys
 */
unsigned int getKeyCount(const char* mccmnc) {
    int i = 0, count = 0;

    i = getStartIndex(mccmnc);

    if (i >= 0) {
        while ((carierConfigData[++i].key != MCC_MNC_SEPARATOR) &&
                     (carierConfigData[i].key != MCC_MNC_END_TAG)) {
            count++;
        }
    }

    return count;
}

/**
 * Retrieve all the values of corresponding MCC/MNC
 * @param mccmnc  mobile country code and mobile network code string
 * @retrun the array of values and the count of values if MCC/MNC can be found, else return 0
 */
int getValuesByMccMnc(const char* mccmnc, CarrierConfigValue *data) {
    int i = 0, count = 0, len;

    i = getStartIndex(mccmnc);
    if (i >= 0) {
        while ((carierConfigData[++i].key != MCC_MNC_SEPARATOR) &&
                     (carierConfigData[i].key != MCC_MNC_END_TAG)) {
            len = strlen(carierConfigData[i].value);
            data[count].key = carierConfigData[i].key;
            data[count].value = (char*)calloc(len + 1, sizeof(char));
            strncpy(data[count].value, carierConfigData[i].value, len);
            count++;
        }
    }

    return count;
}

