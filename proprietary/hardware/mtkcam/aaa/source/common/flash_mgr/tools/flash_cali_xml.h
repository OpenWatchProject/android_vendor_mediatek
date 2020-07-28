#pragma once

#ifdef LINUX
#include "flash_cali_tools.h"
#else
#include "kd_camera_feature.h"
#include "flash_cct.h"
#endif


/***********************************************************
 * XML Header
 **********************************************************/
#define XML_HEADER \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"

/***********************************************************
 * XML caliAeTable
 **********************************************************/
#define CALI_AE_XML \
"<caliAeTable>\n" \
"	<caliAe>\n" \
"		<sensorDev>%s</sensorDev>\n" \
"		<part>%d</part>\n" \
"		<quickCaliExp>%d</quickCaliExp>\n" \
"		<quickCaliAfe>%d</quickCaliAfe>\n" \
"		<quickCaliIsp>%d</quickCaliIsp>\n" \
"	</caliAe>\n" \
"</caliAeTable>\n"

int writeCaliAeXml(const char *fname, int sensorDev, int part, int exp, int afe, int isp);

/***********************************************************
 * XML engTabTable
 **********************************************************/
#define ENG_TAB_XML_engTabTable \
"<engTabTable>\n"

#define ENG_TAB_XML_engTab \
"	<engTab>\n" \
"	<sensorDev>%s</sensorDev>\n" \
"	<part>%d</part>\n" \
"	<dutyLt>%d</dutyLt>\n"

#define ENG_TAB_XML_dutyHt \
"		<dutyHt_%d>%d</dutyHt_%d>\n"

#define ENG_TAB_XML_engTab_ \
"	</engTab>\n"

#define ENG_TAB_XML_engTabTable_ \
"</engTabTable>\n"

int writeEngTabXml(const char *fname,
		PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash,
		int sensorDev, int part, int dutyNum, int dutyNumLt);

/***********************************************************
 * XML fwbGainTable
 **********************************************************/
#define FWB_GAIN_XML_fwbGainTable \
"<fwbGainTable>\n"

#define FWB_GAIN_XML_fwbGain \
"	<fwbGain>\n" \
"		<sensor></sensor>\n" \
"		<sensorDev>%s</sensorDev>\n" \
"		<part>%d</part>\n" \
"		<dutyHt>%d</dutyHt>\n" \
"		<dutyLt>%d</dutyLt>\n" \
"		<rGain>%d</rGain>\n" \
"		<gGain>%d</gGain>\n" \
"		<bGain>%d</bGain>\n" \
"	</fwbGain>\n"

#define FWB_GAIN_XML_fwbGainTable_ \
"</fwbGainTable>\n"

int writeFwbGainXml(const char *fname,
		PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash,
		int sensorDev, int part, int dutyNum, int dutyNumLt);

/***********************************************************
 * XML engAllTable
 **********************************************************/
#define ENG_ALL_XML_engAllTable \
"<engAllTable>\n"

#define ENG_ALL_XML_engAll \
"	<engAll>\n" \
"		<sensor></sensor>\n" \
"		<sensorDev>%s</sensorDev>\n" \
"		<part>%d</part>\n" \
"		<dqNum>%d</dqNum>\n" \
"		<time>%d</time>\n" \
"		<exp>%d</exp>\n" \
"		<afe>%d</afe>\n" \
"		<isp>%d</isp>\n" \
"		<dutyHt>%d</dutyHt>\n" \
"		<dutyLt>%d</dutyLt>\n" \
"		<y>%5.2f</y>\n" \
"		<r>%5.2f</r>\n" \
"		<g>%5.2f</g>\n" \
"		<b>%5.2f</b>\n" \
"	</engAll>\n"

#define ENG_ALL_XML_engAllTable_ \
"</engAllTable>\n"

int writeEngAllXml(const char *fname,
		dqCaliData &caliData1, dqCaliData &caliData2, dqCaliData &caliData3,
		int sensorDev, int part);

