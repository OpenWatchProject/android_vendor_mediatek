package libjpeg_alpha_vendor

import (
    "path/filepath"
    "strings"

    "android/soong/android"
    "android/soong/cc"
)

func init() {
    android.RegisterModuleType("mtk_libjpeg_alpha_oal_vendor_prebuilt_library_shared", mtkLibJpegAlphaOALVendorPrebuiltFactory)
}

func mtkLibJpegAlphaOALVendorPrebuiltFactory() android.Module {
    m, _ := cc.NewPrebuiltSharedLibrary(android.HostAndDeviceSupported)
    module := m.Init()
    android.AddLoadHook(module, func(ctx android.LoadHookContext) { mtkLibJpegAlphaOALVendorLoadHook(ctx, false) })
    return module
}

func mtkLibJpegAlphaOALVendorLoadHook(ctx android.LoadHookContext, proprietary bool) {
    type props struct {
        Multilib struct {
            Lib32 struct {
                Srcs []string
            }
            Lib64 struct {
                Srcs []string
            }
        }
    }
    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")
    mtkPlatformDir := strings.ToLower(vars.String("TARGET_BOARD_PLATFORM"))
    if (mtkPlatformDir == "mt6779" || mtkPlatformDir == "mt6768" || mtkPlatformDir == "mt6785") {
        p.Multilib.Lib32.Srcs = append(p.Multilib.Lib32.Srcs, filepath.Join(mtkPlatformDir, "lib", "libjpeg-alpha-oal_vendor.so"))
        p.Multilib.Lib64.Srcs = append(p.Multilib.Lib64.Srcs, filepath.Join(mtkPlatformDir, "lib64", "libjpeg-alpha-oal_vendor.so"))
    } else {
        p.Multilib.Lib32.Srcs = append(p.Multilib.Lib32.Srcs, filepath.Join("common", "lib", "libjpeg-alpha-oal_vendor.so"))
        p.Multilib.Lib64.Srcs = append(p.Multilib.Lib64.Srcs, filepath.Join("common", "lib64", "libjpeg-alpha-oal_vendor.so"))
    }
    ctx.AppendProperties(p)
}

