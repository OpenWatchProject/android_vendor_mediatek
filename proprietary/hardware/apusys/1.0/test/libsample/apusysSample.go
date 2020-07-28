package LibAPUSYSSAMPLE

import (
    "strings"

    "android/soong/android"
    "android/soong/cc"
)

func libApusysSampleDefaults(ctx android.LoadHookContext) {
    type props struct {
        Srcs []string
        Include_dirs []string
        Cflags []string
    }

    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")
    mtkPlatformDir := strings.ToLower(vars.String("TARGET_BOARD_PLATFORM"))

    //apusysVersion := "1.0"

    if mtkPlatformDir == "mt6779" {
        p.Srcs = append(p.Srcs, "sampleEngine.cpp")
        //p.Include_dirs = append(p.Include_dirs,
        //                        "vendor/mediatek/proprietary/hardware/apusys/" + apusysVersion)
    }

    ctx.AppendProperties(p)
}

func libApusysSampleFactory() android.Module {
    module := cc.LibraryStaticFactory()
    android.AddLoadHook(module, libApusysSampleDefaults)
    return module
}

func init() {
    android.RegisterModuleType("apusys_sample_cc_library_static", libApusysSampleFactory)
}

