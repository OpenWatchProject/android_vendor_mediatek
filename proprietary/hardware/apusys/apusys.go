package LibAPUSYS

import (
    "strings"

    "android/soong/android"
    "android/soong/cc"
)

func libApusysDefaults(ctx android.LoadHookContext) {
    type props struct {
        Srcs []string
        Include_dirs []string
        Cflags []string
    }

    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")
    mtkPlatformDir := strings.ToLower(vars.String("TARGET_BOARD_PLATFORM"))

    if vars.Bool("MTK_APUSYS_SUPPORT") {
        if mtkPlatformDir == "mt6885" {
            apusysVersion := "1.0"

            //Use ION or DMA memory interface
            p.Cflags = append(p.Cflags, "-DMTK_APSYS_ION_ENABLE")

            p.Srcs = append(p.Srcs,apusysVersion + "/apusysCmd.cpp")
            p.Srcs = append(p.Srcs,apusysVersion + "/apusysEngine.cpp")
            p.Srcs = append(p.Srcs,apusysVersion + "/ionAllocator.cpp")
            p.Srcs = append(p.Srcs,apusysVersion + "/dmaAllocator.cpp")
        }

        ctx.AppendProperties(p)
    }
}

func libApusysHeaderDefaults(ctx android.LoadHookContext) {
    type props struct {
        Export_include_dirs []string
    }

    p := &props{}

    vars := ctx.Config().VendorConfig("mtkPlugin")
    mtkPlatformDir := strings.ToLower(vars.String("TARGET_BOARD_PLATFORM"))

    if mtkPlatformDir == "mt6885" {
        apusysVersion := "1.0"
        headerPath := apusysVersion + "/include"
        p.Export_include_dirs = append(p.Export_include_dirs, headerPath)
    }

    ctx.AppendProperties(p)
}

func libApusysFactory() android.Module {
    module := cc.LibrarySharedFactory()
    android.AddLoadHook(module, libApusysDefaults)
    return module
}

func libApusysHeaderFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, libApusysHeaderDefaults)
    return module
}

func init() {
    android.RegisterModuleType("mtk_libapusys_platform_headers", libApusysHeaderFactory)
    android.RegisterModuleType("apusys_cc_library_shared", libApusysFactory)
}

