//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#ifndef __BOOTS_CFG_H__
#define __BOOTS_CFG_H__

/** In MTK Android base platform need enable this to get CHIP ID */
//#define MTK_ANDROID_PLATFORM

/**
 * boots would crete socket for communication, assign writable folder for it.
 * If ignore this define socket would be created in the same directory with boots
 */
//#define BOOTS_SOCKET_PATH   "/tmp/"

/**
 * For mt66xx series chip ID, please make sure it be defined
 *      - MTK Android environment default is "persist.mtk.wcn.combo.chipid" in property
 *      - Pure Linux please set it in environment variable
 */
#define WCN_COMBO_LOADER_CHIP_ID_PROP   "persist.vendor.connsys.chipid"

/** Debug log level */
#define BOOTS_MSG_LVL_DEFAULT           BOOTS_MSG_LVL_INFO
//#define BOOTS_VERBOSE_MSG /* VERBOSE Log */

//---------------------------------------------------------------------------
#endif // __BOOTS_CFG_H__
