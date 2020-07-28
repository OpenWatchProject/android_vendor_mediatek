#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/system_properties.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "UDFWRAPPER"
#include <log/log.h>

void usage(const char* me) {
  static const char* usage_s = "Usage:\n"
    "  %s /system/bin/app_process <args>\n"
    "or, better:\n"
    "  setprop wrap.<nicename> %s\n";
  fprintf(stderr, usage_s, me, me);
  exit(1);
}

void env_prepend(const char* name, const char* value, const char* delim) {
  const char* value_old = getenv(name);
  std::string value_new = value;
  if (value_old) {
    value_new += delim;
    value_new += value_old;
  }
  setenv(name, value_new.c_str(), 1);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    usage(argv[0]);
  }
  char** args = new char*[argc];
  
#if defined(__LP64__)
  if (strcmp(argv[1], "/system/bin/app_process64") == 0) {
    args[0] = (char*)"/system/bin/udf/app_process64";
  }
  else if (strcmp(argv[1], "/system/bin/app_process") == 0) {
    args[0] = (char*)"/system/bin/udf/app_process64";
  }
  else {
    args[0] = argv[1];
  }
#else
#ifdef UDF_64BIT_PLATFORM_32BIT_PROC
  if (strcmp(argv[1], "/system/bin/app_process32") == 0) {
    args[0] = (char*)"/system/bin/udf/app_process32";
  }
  else if (strcmp(argv[1], "/system/bin/app_process") == 0) {
    args[0] = (char*)"/system/bin/udf/app_process32";
  }
#else
  if (strcmp(argv[1], "/system/bin/app_process32") == 0) {
    args[0] = (char*)"/system/bin/udf/app_process32";
  }
  else if (strcmp(argv[1], "/system/bin/app_process") == 0) {
    args[0] = (char*)"/system/bin/udf/app_process";
  }
#endif
  else {
    args[0] = argv[1];
  }
#endif
  ALOGI("%s\n", args[0]);

  for (int i = 1; i < argc - 1; ++i)
    args[i] = argv[i + 1];
  args[argc - 1] = 0;

  execv(args[0], args);
}
