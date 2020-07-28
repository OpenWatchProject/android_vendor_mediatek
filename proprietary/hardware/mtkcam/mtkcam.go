package mtkcam

import (
    "android/soong/android"
    "android/soong/cc"
    "os"
    "path/filepath"
    "strings"
)

func init() {
    android.RegisterModuleType("mtk_mtkcam_algorithm_platform_headers", algoFactory)
}

func algoFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, algoHook)
    return module
}

func exists(local_path string, mod string) bool {
    _, err := os.Stat(filepath.Join(local_path, mod))
    if err == nil { return true }
    if os.IsNotExist(err) { return false }
    return true
}

func algoHook(ctx android.LoadHookContext) {
    type props struct {
        export_include_dirs []string
    }
    local_path := ctx.ModuleDir()
    vars := ctx.Config().VendorConfig("mtkPlugin")
    target_board_platform := strings.ToLower(vars.String("TARGET_BOARD_PLATFORM"))
    algo_path := filepath.Join("include/algorithm/", target_board_platform)

    p := &props{}
    if exists(local_path, algo_path) {
        p.export_include_dirs = append(p.export_include_dirs, algo_path)
    }

    ctx.AppendProperties(p)
}
