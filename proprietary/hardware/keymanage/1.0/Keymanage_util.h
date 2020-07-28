//#ifndef VENDOR_MEDIATEK_HARDWARE_KEYMANAGE_V1_0_KEYMANAGE_UTIL_H
//#define VENDOR_MEDIATEK_HARDWARE_KEYMANAGE_V1_0_KEYMANAGE_UTIL_H

#include <vendor/mediatek/hardware/keymanage/1.0/IKeymanage.h>
using namespace vendor::mediatek::hardware::keymanage::V1_0;

void convertCNF2HIDL(const FT_DRMKEY_INSTALL_CNF *s, HIDL_FT_DRMKEY_INSTALL_CNF *t);
void convertCNF2nonHIDL(const HIDL_FT_DRMKEY_INSTALL_CNF *s, FT_DRMKEY_INSTALL_CNF *t);
void convertREQ2nonHIDL(const HIDL_FT_DRMKEY_INSTALL_REQ *s, FT_DRMKEY_INSTALL_REQ *t);
void convertVector2Array(std::vector<uint8_t> in, char *out);
void convertVector2Array_U(std::vector<uint8_t> in, unsigned char *out);
void convertArray2Vector(unsigned char *in, int len, std::vector<uint8_t>& out);
void convertREQ2HIDL(const FT_DRMKEY_INSTALL_REQ *s, HIDL_FT_DRMKEY_INSTALL_REQ *t);

//#endif  // VENDOR_MEDIATEK_HARDWARE_KEYMANAGE_V1_0_KEYMANAGE_UTIL_H