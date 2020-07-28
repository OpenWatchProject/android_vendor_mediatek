package mtkTelephonyBase

import (
    "android/soong/android"
    "android/soong/java"
    "github.com/google/blueprint/proptools"
)

func init() {
    android.RegisterModuleType("mtk_TelephonyBase_defaults", mtkTelephonyBaseDefaultsFactory)
}

func mtkTelephonyBaseDefaultsFactory() android.Module {
    module := java.DefaultsFactory()
    android.AddLoadHook(module, mtkTelephonyBaseDefaults)
    return module
}

func mtkTelephonyBaseDefaults(ctx android.LoadHookContext) {
    type props struct {
        Enabled *bool
    }
    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")

    if vars.String("MSSI_MTK_TELEPHONY_ADD_ON_POLICY") == "1" {
        p.Enabled = proptools.BoolPtr(false)
    }

    ctx.AppendProperties(p)
}
