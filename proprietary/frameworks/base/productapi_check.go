package mtkProductapi

import (
    "android/soong/android"
    "android/soong/java"
    "github.com/google/blueprint/proptools"
)

func mtkProductapiDefaults(ctx android.LoadHookContext) {
    type props struct {
        Check_api struct {
          Current struct {
            Api_file *string
            Removed_api_file *string
        }
        }
    }
    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")
    if vars.Bool("MTK_PRODUCT_API_CHECK") {
          p.Check_api.Current.Api_file = proptools.StringPtr("api/product-current.txt")
          p.Check_api.Current.Removed_api_file = proptools.StringPtr("api/product-removed.txt")
    }
    ctx.AppendProperties(p)
}

func init() {
    android.RegisterModuleType("mtk_productapi_check_defaults", mtkProductapiDefaultsFactory)
}

func mtkProductapiDefaultsFactory() android.Module {
    module := java.StubsDefaultsFactory()
    android.AddLoadHook(module, mtkProductapiDefaults)
    return module
}
