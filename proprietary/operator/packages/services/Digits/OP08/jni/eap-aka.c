#include <jni.h>
#include <nativehelper/JNIHelp.h>
#include <utils/Log.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "eap-aka.h"
#include "utils/common.h"
#include "crypto/crypto.h"

#define UNUSED __attribute__((__unused__))

/*
 * Class:     com_mediatek_digits_utils_EapAka
 * Method:    getEapAkaAuthKey
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jbyteArray JNICALL Java_com_mediatek_digits_utils_EapAka_getEapAkaAuthKey
        (JNIEnv *env, jobject clazz UNUSED, jbyteArray masterKey)
{
    const u8 *seed = (const u8*)(*env)->GetByteArrayElements(env, masterKey, 0);
    int seed_len = (int)(*env)->GetArrayLength(env, masterKey);

    u8 buffer[40];
    jbyte k_aut[16];

    // 1. use the seed (MK = identity | CK | IK) to generate PRN
    fips186_2_prf(seed, seed_len, buffer, 40);

    // 2. RFC 4187: K_aut is from bit #128 to #255, i.e. byte #16 to #31, of the fips186_2_prf output
    memcpy(k_aut, &buffer[16], 16);
    jbyteArray k_aut_bytes = (*env)->NewByteArray(env, 16);
    if (!k_aut_bytes) {
        ALOGE("Failed to new jbyteArray");
        return NULL;
    }

    (*env)->SetByteArrayRegion(env, k_aut_bytes, 0, 16, k_aut);
    return k_aut_bytes;
}
