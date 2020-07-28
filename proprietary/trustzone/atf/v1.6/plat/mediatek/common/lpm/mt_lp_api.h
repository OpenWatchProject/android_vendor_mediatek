#ifndef __MT_LP_API_H__
#define __MT_LP_API_H__

#include <mt_lp_rm.h>

/* MTK Low Power API Types for CCCI */
enum MT_LP_API_CCCI_TYPE {
	CCCI_AP_MDSRC_REQUEST,
	CCCI_AP_MDSRC_RELEASE,
	CCCI_AP_MDSRC_ACK,
};

/* MTK Low Power API for CCCI Hardware Control
 *
 * Args:
 *	param1 (int): control type
 *
 * Returns:
 *	int: The return value, zero for success, non-zero otherwise.
 */
static inline int mt_ccci_hwctrl(int type)
{
	int ret = 0, val = 0;

	switch (type) {
	case CCCI_AP_MDSRC_REQUEST:
	case CCCI_AP_MDSRC_RELEASE:
		val = (type == CCCI_AP_MDSRC_REQUEST) ? 1 : 0;
		ret = mt_lp_rm_do_hwctrl(PLAT_AP_MDSRC_REQ, val, NULL);
		break;
	case CCCI_AP_MDSRC_ACK:
		ret = mt_lp_rm_do_hwctrl(PLAT_AP_MDSRC_ACK, 0, NULL);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

/* MTK Low Power API Types for Audio */
enum MT_LP_API_AUDIO_TYPE {
	FM_AUDIO_ENTER,
	FM_AUDIO_LEAVE,
	AUDIO_DSP_ENTER,
	AUDIO_DSP_LEAVE,
};

/* MTK Low Power API for Audio Update Status
 *
 * Args:
 *	param1 (int): status type
 *
 * Returns:
 *	int: The return value, zero for success, non-zero otherwise.
 */
static inline int mt_audio_update(int type)
{
	int ret = 0, val = 0;

	switch (type) {
	case FM_AUDIO_ENTER:
	case FM_AUDIO_LEAVE:
		val = (type == FM_AUDIO_ENTER) ? 1 : 0;
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_FMAUDIO, &val);
		break;
	case AUDIO_DSP_ENTER:
	case AUDIO_DSP_LEAVE:
		val = (type == AUDIO_DSP_ENTER) ? 1 : 0;
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_ADSP, &val);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

#endif
