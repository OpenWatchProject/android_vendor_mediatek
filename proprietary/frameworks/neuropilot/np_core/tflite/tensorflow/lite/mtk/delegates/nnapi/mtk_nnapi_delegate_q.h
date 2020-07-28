/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef MTK_TENSORFLOW_LITE_NNAPI_DELEGATE_Q_H_
#define MTK_TENSORFLOW_LITE_NNAPI_DELEGATE_Q_H_

#include <memory>
#include <string>
#include <vector>

#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/mtk/delegates/nnapi/mtk_nnapi_delegate.h"
typedef struct ANeuralNetworksMemory ANeuralNetworksMemory;

namespace tflite {

// MTK NNAPI delegate for Android Q
class MtkNnApiDelegateQ : public MtkNnApiDelegate {
 public:
  MtkNnApiDelegateQ();
  ~MtkNnApiDelegateQ() override;
  TfLiteDelegate* GetTFLiteDelegate() override { return delegate_.get(); }

 private:
  std::unique_ptr<TfLiteDelegate> delegate_;
};

// TFliteDelegate to interface with NNAPI.
class MtkStatefulNnApiDelegate : public TfLiteDelegate {
 public:
  // Encapsulates all options that are specific to NNAPI delegate.
  struct Options {
    // Preferred Power/perf trade-off. For more details please see
    // ANeuralNetworksCompilation_setPreference documentation in :
    // https://developer.android.com/ndk/reference/group/neural-networks.html
    enum ExecutionPreference {
      kUndefined = -1,
      kLowPower = 0,
      kFastSingleAnswer = 1,
      kSustainedSpeed = 2,
    };

    // Preferred Power/perf trade-off.
    ExecutionPreference execution_preference = kUndefined;

    // Selected NNAPI accelerator with nul-terminated name.
    // Default to nullptr, which implies the NNAPI default behavior: NNAPI
    // runtime is allowed to use all available accelerators. If the selected
    // accelerator cannot be found, NNAPI will not be used.
    // It is the caller's responsibility to ensure the string is valid for the
    // duration of the Options object lifetime.
    const char* accelerator_name = nullptr;

    // The nul-terminated cache dir for NNAPI model.
    // Default to nullptr, which implies the NNAPI will not try caching the
    // compilation.
    const char* cache_dir = nullptr;

    // The unique nul-terminated token string for NNAPI model.
    // Default to nullptr, which implies the NNAPI will not try caching the
    // compilation. It is the caller's responsibility to ensure there is no
    // clash of the tokens.
    // NOTE: when using compilation caching, it is not recommended to use the
    // same delegate instance for multiple models.
    const char* model_token = nullptr;
  };

  // Uses default options.
  MtkStatefulNnApiDelegate();

  // The constructor that accepts options from user.
  explicit MtkStatefulNnApiDelegate(Options options);

  ~MtkStatefulNnApiDelegate() = default;

  // Returns the delegate options.
  static const Options GetOptions(TfLiteDelegate* delegate);

  // Callback function which copies data from ANeuralNetworksMemory to host
  // tensor CPU buffer. It is the users responsibility to implement these
  // callbacks for the specific types of shared memory they intend to use.
  // WARNING: This is an experimental interface that is subject to change.
  typedef TfLiteStatus (*CopyToHostTensorFnPtr)(TfLiteTensor* tensor,
                                                ANeuralNetworksMemory* memory,
                                                size_t memory_offset,
                                                size_t byte_size,
                                                void* callback_context);

  // Encapsulates all fields related to memory registration for internal
  // bookkeeping only.
  struct MemoryRegistration {
    ANeuralNetworksMemory* memory;
    CopyToHostTensorFnPtr callback;
    void* callback_context;
  };

  // Register the ANeuralNetworksMemory handle with the delegate. A
  // TfLiteBufferHandle will be returned to be used with
  // Interpreter::SetBufferHandle. The callback_context will be passed to the
  // callback function when invoked.
  // Note: the returned TfLiteBufferHandle can only be used with a single
  // Interpreter instance. However, the caller can register the same memory
  // multiple times to get different handles to use with difference Interpreter
  // instances
  // WARNING: This is an experimental interface that is subject to change.
  TfLiteBufferHandle RegisterNnapiMemory(ANeuralNetworksMemory* memory,
                                         CopyToHostTensorFnPtr callback,
                                         void* callback_context);

  // Returns the vector of known ANeuralNetworksMemory handles.
  // Note: this function is not intended to be called by developers.
  // WARNING: This is an experimental interface that is subject to change.
  static const std::vector<MemoryRegistration>& GetTensorMemoryMap(
      TfLiteDelegate* delegate);

 private:
  // Encapsulates all delegate data.
  struct Data {
    // Preferred Power/perf trade-off.
    Options::ExecutionPreference execution_preference;
    // Selected NNAPI accelerator name.
    std::string accelerator_name;
    // The cache dir for NNAPI model.
    std::string cache_dir;
    // The unique token string for NNAPI model.
    std::string model_token;
    // Tensor to ANeuralNetworksMemory mapping.
    std::vector<MemoryRegistration> tensor_memory_map;
  };

  // Implements TfLiteDelegate::Prepare. Please refer to TFLiteDelegate
  // documentation for more info.
  static TfLiteStatus DoPrepare(TfLiteContext* context,
                                TfLiteDelegate* delegate);

  // Copy the data from delegate buffer handle into raw memory of the given
  // 'tensor'. The delegate is allowed to allocate the raw
  // bytes as long as it follows the rules for kTfLiteDynamic tensors.
  static TfLiteStatus DoCopyFromBufferHandle(TfLiteContext* context,
                                             TfLiteDelegate* delegate,
                                             TfLiteBufferHandle buffer_handle,
                                             TfLiteTensor* tensor);

  // Copy the data from raw memory of the given 'tensor' to delegate buffer
  // handle. Currently this function is not supported, and calling the function
  // will result in an error.
  static TfLiteStatus DoCopyToBufferHandle(TfLiteContext* context,
                                           TfLiteDelegate* delegate,
                                           TfLiteBufferHandle buffer_handle,
                                           TfLiteTensor* tensor);

  // Free the Delegate Buffer Handle. Note: This only frees the handle, but
  // this doesn't release the underlying resource (e.g. textures). The
  // resources are either owned by application layer or the delegate.
  static void DoFreeBufferHandle(TfLiteContext* context,
                                 TfLiteDelegate* delegate,
                                 TfLiteBufferHandle* handle);

  // Delegate data presented through TfLiteDelegate::data_.
  Data delegate_data_;
};

}  // namespace tflite

#endif  // MTK_TENSORFLOW_LITE_NNAPI_DELEGATE_Q_H_
