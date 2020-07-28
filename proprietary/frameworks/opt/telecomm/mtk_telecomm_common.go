package mtkTelecommCommonFeature

import (
    "android/soong/android"
    "android/soong/java"
    "github.com/google/blueprint/proptools"
)

func mtkTelecommCommonDefaults(ctx android.LoadHookContext) {
    type props struct {
        Libs []string
        Enabled *bool
    }
    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")
    if vars.String("MSSI_MTK_TELEPHONY_ADD_ON_POLICY") == "0" {
        p.Libs = append(p.Libs, "mediatek-telephony-base")
    }
    if vars.String("MSSI_MTK_TELEPHONY_ADD_ON_POLICY") == "1" {
        p.Enabled = proptools.BoolPtr(false)
    }
    ctx.AppendProperties(p)
}

func init() {
    android.RegisterModuleType("mtk_telecomm_common_defaults", mtkTelecommCommonDefaultsFactory)
}

func mtkTelecommCommonDefaultsFactory() android.Module {
    module := java.DefaultsFactory()
    android.AddLoadHook(module, mtkTelecommCommonDefaults)
    return module
}

