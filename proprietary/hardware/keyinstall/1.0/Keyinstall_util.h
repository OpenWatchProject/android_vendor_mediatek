#include <vendor/mediatek/hardware/keyinstall/1.0/IKeyinstall.h>
using namespace vendor::mediatek::hardware::keyinstall::V1_0;

void convertCNF2HIDL(const FT_DRMKEY_INSTALL_CNF *s, HIDL_FT_DRMKEY_INSTALL_CNF *t);
void convertCNF2nonHIDL(const HIDL_FT_DRMKEY_INSTALL_CNF *s, FT_DRMKEY_INSTALL_CNF *t);
void convertREQ2nonHIDL(const HIDL_FT_DRMKEY_INSTALL_REQ *s, FT_DRMKEY_INSTALL_REQ *t);
void convertVector2Array(std::vector<uint8_t> in, char *out);
void convertREQ2HIDL(const FT_DRMKEY_INSTALL_REQ *s, HIDL_FT_DRMKEY_INSTALL_REQ *t);
