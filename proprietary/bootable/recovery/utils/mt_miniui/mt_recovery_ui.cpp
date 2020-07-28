#include <roots.h>
#include <android-base/logging.h>
#include <mt_recovery_ui.h>

static constexpr const char* NVDATA_ROOT = "/mnt/vendor/nvdata";

bool MTK_Device::PostWipeData() {
  int result = -1;

  load_volume_table();
  has_nvdata = volume_for_mount_point(NVDATA_ROOT) != nullptr;

  LOG(INFO) << "PostWipeData";

  if (has_nvdata) {
    LOG(INFO) << "has nvdata ";
    if (ensure_path_unmounted(NVDATA_ROOT) != 0) {
      LOG(ERROR) << "Failed to unmount " << NVDATA_ROOT;
      return false;
    }
    LOG(INFO) << "Wiping " << NVDATA_ROOT;
    result = format_volume(NVDATA_ROOT);
  } else {
    LOG(INFO) << "no nvdata ";
    result = 0;
  }

  if (result == 0)
    return true;

  return false;
}
