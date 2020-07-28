package LibVPU

import (
    "strings"
    "android/soong/android"
    "android/soong/cc"
)

func getVpuVer(ctx android.LoadHookContext) string {
    vars := ctx.Config().VendorConfig("mtkPlugin")
    mtkPlatformDir := strings.ToLower(vars.String("TARGET_BOARD_PLATFORM"))

    if mtkPlatformDir == "mt6779" || mtkPlatformDir == "mt6785" {
        return "3.0"
    }

    if mtkPlatformDir == "mt6885" {
        return "4.0"
    }

    return "2.0"
}

func libVPUDefaults(ctx android.LoadHookContext) {
    type props struct {
        Srcs []string
        Include_dirs []string
        Cflags []string
        Shared_libs []string
    }


    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")
    mtkPlatformDir := strings.ToLower(vars.String("TARGET_BOARD_PLATFORM"))

    if vars.Bool("MTK_VPU_SUPPORT") {

    p.Shared_libs = append(p.Shared_libs, "libion_mtk")

    if mtkPlatformDir == "mt6775" || mtkPlatformDir == "mt6771" {
        p.Srcs = append(p.Srcs,"2.0/vpuImpl.cpp")
        p.Include_dirs = append(p.Include_dirs,
                                "vendor/mediatek/proprietary/hardware/vpu/include")
        p.Include_dirs = append(p.Include_dirs,
                                "vendor/mediatek/proprietary/hardware/m4u/" + mtkPlatformDir)
        p.Include_dirs = append(p.Include_dirs,
                                "vendor/mediatek/proprietary/hardware/vpu/2.0/platform/" + mtkPlatformDir)
        p.Include_dirs = append(p.Include_dirs,
                                "device/mediatek/" + mtkPlatformDir +"/kernel-headers")
        if mtkPlatformDir == "mt6771" {
            p.Cflags = append(p.Cflags, "-DHAVE_EFUSE_BOND_VERSION=71")
        }
    }

        vpuVer := getVpuVer(ctx)

        p.Include_dirs = append(p.Include_dirs, "device/mediatek/common/kernel-headers")
        p.Include_dirs = append(p.Include_dirs, "device/mediatek/" + mtkPlatformDir +"/kernel-headers")

        if mtkPlatformDir == "mt6779" || mtkPlatformDir == "mt6785" {
            p.Srcs = append(p.Srcs, vpuVer + "/vpuImpl.cpp")
            p.Srcs = append(p.Srcs, vpuVer + "/earaUtil.cpp")
            p.Srcs = append(p.Srcs, vpuVer + "/vpuBuffer.cpp")
        }

        if mtkPlatformDir == "mt6885" {
            vpuVer := "4.0"
            p.Srcs = append(p.Srcs, vpuVer + "/vpuImpl.cpp")
            p.Srcs = append(p.Srcs, vpuVer + "/earaUtil.cpp")

            p.Shared_libs = append(p.Shared_libs, "libapusys")
        }

        ctx.AppendProperties(p)
    }
}

func libVPUFactory() android.Module {
    module := cc.LibrarySharedFactory()
    android.AddLoadHook(module, libVPUDefaults)
    return module
}

func libVpuHeaderDefaults(ctx android.LoadHookContext) {
    type props struct {
        Export_include_dirs []string
        Header_libs []string
        Export_header_lib_headers []string
    }

    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")
    mtkPlatformDir := strings.ToLower(vars.String("TARGET_BOARD_PLATFORM"))

    if vars.Bool("MTK_VPU_SUPPORT") {
        vpuVer := getVpuVer(ctx)
        headerPath := vpuVer
        p.Export_include_dirs = append(p.Export_include_dirs, headerPath)
        p.Export_include_dirs = append(p.Export_include_dirs, headerPath + "/platform/" + mtkPlatformDir)

        if mtkPlatformDir == "mt6885" {
            p.Header_libs = append(p.Header_libs, "libapusys_headers")
            p.Export_header_lib_headers = append(p.Export_header_lib_headers, "libapusys_headers")
        }

        ctx.AppendProperties(p)
    }
}

func libVpuHeaderFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, libVpuHeaderDefaults)
    return module
}

func init() {
    android.RegisterModuleType("mtk_libvpu_platform_headers", libVpuHeaderFactory)
    android.RegisterModuleType("vpu_cc_library_shared", libVPUFactory)
}

