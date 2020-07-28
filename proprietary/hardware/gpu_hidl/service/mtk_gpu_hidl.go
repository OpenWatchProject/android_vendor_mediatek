package GedSrv

import (
    "android/soong/android"
    "android/soong/cc"
)

func mtkGedSrvDefaults(ctx android.LoadHookContext) {
    type props struct {
        Cflags []string
        Srcs   []string
    }
    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")

    offset_ns := "0"
    if vars.String("VSYNC_EVENT_PHASE_OFFSET_NS") != "" {
        offset_ns = vars.String("VSYNC_EVENT_PHASE_OFFSET_NS")
    }
    p.Cflags = append(p.Cflags, "-DVSYNC_EVENT_PHASE_OFFSET_NS="+offset_ns)

    if vars.Bool("MTK_GED_GAS_SUPPORT") {
        p.Cflags = append(p.Cflags, "-DGED_GAS_SUPPORT")
    }

    ctx.AppendProperties(p)
}

func init() {
    android.RegisterModuleType("mtk_gpu_hidl_defaults", mtkGedSrvDefaultsFactory)
}

func mtkGedSrvDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, mtkGedSrvDefaults)
    return module
}
