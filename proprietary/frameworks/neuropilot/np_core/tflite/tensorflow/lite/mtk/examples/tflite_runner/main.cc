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

#include "tensorflow/lite/mtk/experimental/addon/easyloggingpp/easylogging++.h"
#include "tensorflow/lite/mtk/examples/tflite_runner/tflite_runner.h"

#include <glob.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include <thread>
#include <gtest/gtest.h>

INITIALIZE_EASYLOGGINGPP

enum OutputType {
  OUTPUT_NONE = 0,
  SHOW_OUTPUT = 1,
  SAVE_OUTPUT = 2,
};

enum ModeType {
  // Glob model/input/golden by the given root path and file name prefix
  DEFAULT_MODE = 0,
  SPECIFIED_MODEL_MODE = 1,
  SPECIFIED_MODEL_WITH_RANDOM_INPUT = 2,
};

struct Settings {
  bool accel = true;
  // Mode
  // 0: default mode
  // 1. specified model with input/output
  // 2. specified model with random input
  int mode = 0;
  int output_type = 0;
  int32_t loop_count = 1;
  std::string model_path;
  std::string input_path;
  std::string golden_path;
  bool relax_float16 = true;
  bool print_interpreter_state = false;
  bool allow_threshold = true;
  bool continuous_input = false;
  bool gtest_mode = false;
  int thread_num = 1;
  int interpreter_thread_num = 8;
};

static Settings g_settings;

std::vector<std::string> globFile(const std::string& prefix) {
  // glob struct resides on the stack
  glob_t glob_result;
  memset(&glob_result, 0, sizeof(glob_result));
  std::string pattern = prefix + "*";
  // do the glob operation
  LOG(INFO) << "glob with pattern: " << pattern;
  int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);

  if (return_value != 0) {
    globfree(&glob_result);
    LOG(ERROR) << "glob() failed with return_value " << return_value;
  }

  // collect all the filenames into a std::list<std::string>
  std::vector<std::string> filenames;

  for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
    filenames.push_back(std::string(glob_result.gl_pathv[i]));
  }

  // cleanup
  globfree(&glob_result);

  // done
  return filenames;
}

Settings processOptions(int argc, char** argv) {
  static const char* short_options = "a:s:c:o:m:x:y:r:i:t:n:g:d:e:";
  static struct option long_options[] = {
    {"accelerated", required_argument, nullptr, 'a'},
    {"specified", required_argument, nullptr, 's'},
    {"loop_count", required_argument, nullptr, 'c'},
    {"show_output", required_argument, nullptr, 'o'},
    {"model", required_argument, nullptr, 'm'},
    {"batch_xs", required_argument, nullptr, 'x'},
    {"ys", required_argument, nullptr, 'y'},
    {"relax_float16", required_argument, nullptr, 'r'},
    {"print_interpreter_state", required_argument, nullptr, 'i'},
    {"threshold", required_argument, nullptr, 't'},
    {"continuous_input", required_argument, nullptr, 'n'},
    {"gtest", required_argument, nullptr, 'g'},
    {"tfliterunner_threads", required_argument, nullptr, 'd'},
    {"interpreter_threads", required_argument, nullptr, 'e'},
    {nullptr, no_argument, nullptr, 0}
  };

  Settings s;
  int c = -1;
  //int option_index = 0;
  s.loop_count = 1;
  s.relax_float16 = true;  // Relax to FP16 by default
  s.allow_threshold = true;
  s.thread_num = 1;

  while (1) {
    /* getopt_long stores the option index here. */
    c = getopt_long(argc, argv,
        short_options,  //  "a:s:c:o:m:x:y:r:i:t:n:g:d:e:",
        long_options,
        nullptr);

    /* Detect the end of the options. */
    if (c == -1) break;

    switch (c) {
      case 'a':
        s.accel = strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 'c':
        s.loop_count = strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 's':
        s.mode = strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 'o':
        s.output_type = strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 'm':
        s.model_path = optarg;
        break;
      case 'x':
        s.input_path = optarg;
        break;
      case 'y':
        s.golden_path = optarg;
        break;
      case 'r':
        s.relax_float16 =
            strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 'i':
        s.print_interpreter_state =
            strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 't':
        s.allow_threshold = strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 'n':
        s.continuous_input = strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 'g':
        s.gtest_mode = strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 'd':
        s.thread_num = strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      case 'e':
        s.interpreter_thread_num =
            strtol(optarg, reinterpret_cast<char**>(NULL), 10);
        break;
      default:
        exit(-1);
    }
  }

  LOG(INFO) << "Loop count: " << s.loop_count;
  LOG(INFO) << "Use NNAPI: " << (s.accel ? "True" : "False");
  LOG(INFO) << "Mode: " << s.mode;
  LOG(INFO) << "Output type: " << s.output_type;
  LOG(INFO) << "Allow FP16 precision computation: " \
    << (s.relax_float16 ? "True" : "False");
  LOG(INFO) << "Allow threshold when comparing outout: " \
    << (s.allow_threshold ? "True" : "False");
  LOG(INFO) << "Num of threads in TFLiteRunner: " << s.thread_num;
  LOG(INFO) << "Num of threads in Interpreter: " << s.interpreter_thread_num;

  return s;
}

void invoke(const std::vector<std::string>& xs_list,
           const std::vector<std::string>& ys_list,
           int& result) {
  TFLiteRunnerHandle* handle = nullptr;

  result = TFLiteRunner_Create(&handle, g_settings.model_path);
  if (result == 0) {
    if (g_settings.print_interpreter_state) {
      TFLiteRunner_PrintState(handle);
    }
    TFLiteRunner_SetUseNnApi(handle, g_settings.accel);
    TFLiteRunner_SetAllowThreshold(handle, g_settings.allow_threshold);
    TFLiteRunner_SetAllowFp16Precision(handle, g_settings.relax_float16);
    TFLiteRunner_SetLoopCount(handle, g_settings.loop_count);
    TFLiteRunner_SetInterpreterNumThreads(handle,
                                          g_settings.interpreter_thread_num);
    if (g_settings.output_type == SHOW_OUTPUT) {
      TFLiteRunner_SetShowOutput(handle, true);
    }
    if (g_settings.output_type == SAVE_OUTPUT) {
      TFLiteRunner_SetSaveOutput(handle, true);
    }
    result = TFLiteRunner_Invoke(handle, xs_list, ys_list);
  }

  if (handle != nullptr) {
    TFLiteRunner_Free(handle);
  }
}

int run() {
  int ret = 0;
  std::vector<std::string> xs_list;
  std::vector<std::string> ys_list;
  if (g_settings.mode != SPECIFIED_MODEL_WITH_RANDOM_INPUT) {
    if (g_settings.continuous_input) {
      xs_list = globFile(g_settings.input_path);
      ys_list = globFile(g_settings.golden_path);

      CHECK_EQ(xs_list.size(), ys_list.size()) \
        << "There is a mismatch between the input and the golden";
      LOG(INFO) << "Found " << xs_list.size() << " input-golden sets";
    } else {
      xs_list.push_back(g_settings.input_path);
      ys_list.push_back(g_settings.golden_path);
    }
  }

  // Create a vector of threads
  std::vector<std::thread> invoke_threads;
  // Create a vector of threads
  std::vector<int> results(g_settings.thread_num, 0);
  for (int i = 0; i < g_settings.thread_num; i++) {
    std::thread th(invoke,
                   std::ref(xs_list),
                   std::ref(ys_list),
                   std::ref(results[i]));
    invoke_threads.push_back(std::move(th));
  }

  // Iterate over the thread vector
  for (std::thread& th : invoke_threads) {
    // If thread object is joinable then join that thread.
    if (th.joinable()) {
        th.join();
    }
  }

  if (std::all_of(results.cbegin(), results.cend(),
          [](int result) {return result != 0;})) {
      ret = 1;
  }

  invoke_threads.clear();
  results.clear();
  return ret;
}

TEST(TFLiteRunner, run) {
  int ret = run();
  EXPECT_EQ(0, ret);
}

int main(int argc, char** argv) {
  el::Loggers::removeFlag(el::LoggingFlag::ColoredTerminalOutput);
  el::Configurations defaultConf;
  defaultConf.setGlobally(el::ConfigurationType::Format, "%msg");
  el::Loggers::reconfigureLogger("default", defaultConf);

  testing::InitGoogleTest(&argc, argv);
  g_settings = processOptions(argc, argv);

  int ret = 0;

  if (g_settings.gtest_mode) {
    ret = RUN_ALL_TESTS();
  } else {
    ret = run();
  }

  // Call exit here to prevent cleanup/finalize. Let kernel do the cleanup.
  _exit(0);
  return ret;
}
