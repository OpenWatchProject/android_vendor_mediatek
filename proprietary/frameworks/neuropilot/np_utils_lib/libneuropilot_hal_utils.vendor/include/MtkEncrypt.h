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


#ifndef ANDROID_ML_MTK_ENCRYPT_H
#define ANDROID_ML_MTK_ENCRYPT_H

namespace android {
namespace nn {

std::string aes_encrypt(std::string str, int rand_index);

int define_len(std::string str);

std::string mtk_encrypt_print(const char* fmt, ...);

class MTK_ENCRYPT {
    public:
        std::stringstream *input_str;

        MTK_ENCRYPT(std::stringstream *ss) {
            input_str = ss;
        }

        template <typename T>
        MTK_ENCRYPT& operator<<(T a) {
            *input_str << a;
            return *this;
        }
};

/**
 * Encrypt printf like Log Method.
 * Choose Example 1 for more convenient usage.
 * But if you encounter build error with Example 1, try to use Example 2.
 * Example1 usage: TFLITE_MTK_LOG_INFO(MTK_ENCRYPT_PRINT("I am test %s %d", "ttt", 123));
 * Example2 usage: printf("%s\n", MTK_ENCRYPT_PRINT("I am test %s %d", "ttt", 123));
 */
#define MTK_ENCRYPT_PRINT(fmt, ...) android::nn::mtk_encrypt_print(fmt, ##__VA_ARGS__).c_str()

/**
 * Encrypt cout like Log Method.
 * Example usage: std::cout << MTK_ENCRYPT_COUT("abc" << 123 << 5.5);
 */
#define MTK_ENCRYPT_COUT(input) ({ \
    std::stringstream encrypt_cout; \
    android::nn::MTK_ENCRYPT(&encrypt_cout) << input; \
    android::nn::aes_encrypt(encrypt_cout.str(), rand()%10); \
})

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_MTK_ENCRYPT_H
