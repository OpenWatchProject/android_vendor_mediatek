/*
 * MTK HIDL related utils
 */
#ifndef MTK_HIDL_UTILS_H
#define MTK_HIDL_UTILS_H

#ifdef CONFIG_CTRL_IFACE_MTK_HIDL
#include <vendor/mediatek/hardware/wifi/supplicant/2.0/ISupplicantStaNetwork.h>
#define MTK_ALLOWED_KEY_MGMT                                           \
    static_cast<uint32_t>(vendor::mediatek::hardware::wifi::supplicant \
    ::V2_0::ISupplicantStaNetwork::KeyMgmtMask::WPA_EAP_SHA256) |      \
    static_cast<uint32_t>(vendor::mediatek::hardware::wifi::supplicant \
    ::V2_0::ISupplicantStaNetwork::KeyMgmtMask::WAPI_PSK) |            \
    static_cast<uint32_t>(vendor::mediatek::hardware::wifi::supplicant \
    ::V2_0::ISupplicantStaNetwork::KeyMgmtMask::WAPI_CERT)

#define MTK_ALLOWED_PROTO_MASK                                         \
    static_cast<uint32_t>(vendor::mediatek::hardware::wifi::supplicant \
    ::V2_0::ISupplicantStaNetwork::ProtoMask::WAPI)

#define MTK_ALLOWED_GROUP_CIPHER_MASK                                  \
    static_cast<uint32_t>(vendor::mediatek::hardware::wifi::supplicant \
    ::V2_0::ISupplicantStaNetwork::GroupCipherMask::SM4)

#define MTK_ALLOWED_PAIRWISE_CIPHER_MASK                               \
    static_cast<uint32_t>(vendor::mediatek::hardware::wifi::supplicant \
    ::V2_0::ISupplicantStaNetwork::PairwiseCipherMask::SM4)
#else
#define MTK_ALLOWED_KEY_MGMT
#define MTK_ALLOWED_PROTO_MASK
#define MTK_ALLOWED_GROUP_CIPHER_MASK
#define MTK_ALLOWED_PAIRWISE_CIPHER_MASK
#endif

#endif /* MTK_HIDL_UTILS_H */