/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <string>

#include <dlfcn.h>

#include <api_keymaster1.h>
#include <api_keymaster2.h>

#include "test_km_compat.h"
#include "test_km_util.h"

#include "log.h"

struct blob {
    blob() : p {}, sz {} { }
    explicit blob(size_t sz) : p {sz ? new uint8_t[sz] : nullptr}, sz {sz} { }
    blob(const uint8_t *q, size_t sz) : blob {sz}
        { if (sz) memcpy(p.get(), q, sz); }
    blob(const blob &b) = delete;
    blob(blob &&b) noexcept : p {std::move(b.p)}, sz {b.sz} { }
    explicit blob(const keymaster_key_blob_t &k) :
        blob {k.key_material, k.key_material_size} { }
    explicit blob(const keymaster_blob_t &b) :
        blob {b.data, b.data_length} { }
    struct steal_blob {
        steal_blob(keymaster_key_blob_t &&k) :
            p {const_cast<uint8_t *>(k.key_material)}, sz {k.key_material_size}
            { k.key_material = nullptr; }
        steal_blob(keymaster_blob_t &&b) :
            p {const_cast<uint8_t *>(b.data)}, sz {b.data_length}
            { b.data = nullptr; }
        ~steal_blob() { free(p); }
        uint8_t *p;
        size_t sz;
    };
    explicit blob(keymaster_key_blob_t &&k) : blob {} {
        steal_blob kk {std::move(k)};
        *this = keymaster_key_blob_t {kk.p, kk.sz};
    }
    explicit blob(keymaster_blob_t &&b) : blob {} {
        steal_blob bb {std::move(b)};
        *this = keymaster_key_blob_t {bb.p, bb.sz};
    }
    blob &operator=(blob &&b) noexcept
        { p = std::move(b.p); sz = b.sz; return *this; }
    blob &operator=(const keymaster_key_blob_t &k) { return *this = blob {k}; }
    blob &operator=(keymaster_key_blob_t &&k) { return *this = blob {k}; }
    operator keymaster_key_blob_t() const { return {p.get(), sz}; }
    operator keymaster_blob_t() const { return {p.get(), sz}; }
    blob &replace(size_t sz) { blob t {sz}; std::swap(*this, t); return *this; }
    blob &replace(uint8_t *q, size_t sz)
        { blob t {q, sz}; std::swap(*this, t); return *this; }

    std::unique_ptr<uint8_t> p;
    size_t sz;
};

struct param : keymaster_key_param_t {
    param(keymaster_tag_t tag, bool b)
        { this->tag = tag; this->boolean = b; }
    param(keymaster_tag_t tag, uint32_t i)
        { this->tag = tag; this->integer = i; }
    param(keymaster_tag_t tag, uint64_t i)
        { this->tag = tag; this->long_integer = i; }
    param(keymaster_tag_t tag, const ::blob &b)
        { this->tag = tag; this->blob = b; }
    param(keymaster_tag_t tag, const keymaster_blob_t &b)
        { this->tag = tag; this->blob = b; }
    param(keymaster_tag_t tag, const uint8_t *p, size_t sz)
        { this->tag = tag; this->blob = {p, sz}; }

    param(keymaster_tag_t tag, int i) : param {tag, uint32_t(i)} { }
};

[[noreturn]] static void bail(const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    fputs("*** UNEXPECTED SYSTEM ERROR: ", stderr);
    vfprintf(stderr, msg, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(126);
}

static void load_key_blob(blob &b, const char *file)
{
    FILE *fp = fopen(file, "rb");
    unsigned char *p = NULL, *q;
    size_t sz = 0, o = 0, n;

    if (!fp) bail("fopen(%s): %s", file, strerror(errno));
    for (;;) {
        sz = sz ? 3*sz/2 : 4096;
        q = static_cast<unsigned char *>(realloc(p, sz));
        if (!q) bail("out of memory");
        p = q;
        n = fread(p + o, 1, sz - o, fp);
        o += n;
        if (o < sz) break;
    }
    if (ferror(fp)) bail("read(%s): %s", file, strerror(errno));
    fclose(fp);
    b.replace(p, o);
    free(p);
}

template<typename KM>
struct kmimpl_base {
    kmimpl_base(const char *file);
    virtual ~kmimpl_base() noexcept;
    virtual keymaster_error_t makeblob(blob &);
    virtual keymaster_error_t checkblob(const blob &,
        const keymaster_key_param_set_t *kps = NULL);

    const char *file;
    void *dl {};
    KM *km {};
};

template<typename KM>
kmimpl_base<KM>::kmimpl_base(const char *file) : file {file}
{
    dl = dlopen(file, RTLD_LAZY | RTLD_LOCAL);
    if (!dl) bail("dlopen(`%s'): %s", file, strerror(errno));
    hw_module_t *mod =
        static_cast<hw_module_t *>(dlsym(dl, HAL_MODULE_INFO_SYM_AS_STR));
    if (!mod) bail("dlsym(`%s', `hmi')", file);
    hw_device_t *dev {};
    int rc = mod->methods->open(mod, KEYSTORE_KEYMASTER, &dev);
    if (rc < 0) bail("open(`%s'): %s", file, strerror(-rc));
    km = reinterpret_cast<KM *>(dev);
}

template<typename KM>
kmimpl_base<KM>::~kmimpl_base()
{
    if (km) km->common.close(&km->common);
    if (dl) dlclose(dl);
}

template<typename KM>
keymaster_error_t kmimpl_base<KM>::makeblob(blob &b)
{
    keymaster_error_t res = KM_ERROR_OK;
    param kp[] {
        { KM_TAG_ALGORITHM,             KM_ALGORITHM_EC },
        { KM_TAG_KEY_SIZE,              256 },
        { KM_TAG_PURPOSE,               KM_PURPOSE_SIGN },
        { KM_TAG_DIGEST,                KM_DIGEST_NONE },
        { KM_TAG_NO_AUTH_REQUIRED,      true }
    };
    keymaster_key_param_set_t kps {kp, N_ARRAY_ELTS(kp)};
    keymaster_key_blob_t kb;

    CHECK_RESULT_OK(km->generate_key(km, &kps, &kb, NULL));
    b = std::move(kb);
end:
    return res;
}

template<typename KM>
keymaster_error_t kmimpl_base<KM>::checkblob(const blob &b,
    const keymaster_key_param_set_t *kps)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_operation_handle_t op;
    const keymaster_key_blob_t kb = b;
    keymaster_key_param_set_t kps_empty {NULL, 0};

    keymaster_error_t r = km->begin(km, KM_PURPOSE_SIGN,
        &kb, kps ? kps : &kps_empty, NULL, &op);
    if (r == KM_ERROR_OK)
        CHECK_RESULT_OK(km->abort(km, op));
    res = r;
end:
    return res;
}

struct keymaster_m : kmimpl_base<keymaster1_device_t>
{
    keymaster_m(const char *file) : kmimpl_base(file) { }
};

struct keymaster_n : kmimpl_base<keymaster2_device_t>
{
    keymaster_n(const char *file) : kmimpl_base {file} { }

    keymaster_error_t upgrade(blob &bb, const blob &b,
        const keymaster_key_param_set_t *kps = NULL);
};

keymaster_error_t keymaster_n::upgrade(blob &bb, const blob &b,
    const keymaster_key_param_set_t *kps)
{
    keymaster_key_blob_t ikb = b, okb;
    keymaster_key_param_set_t kps_empty {NULL, 0};
    keymaster_error_t res = KM_ERROR_OK;

    CHECK_RESULT_OK(km->upgrade_key(km, &ikb, kps ? kps : &kps_empty, &okb));
    bb = std::move(okb);

end:
    return res;
}

keymaster_error_t test_km_compat()
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_m km_m {"libMcTeeKeymasterM.so"};
    keymaster_n km_n {"libMcTeeKeymasterN.so"};
    blob b, bb;
    bool test_loadblob;

    { const char *p = getenv("KM_TEST_LOADBLOB");
        test_loadblob = p && (*p == '1' || *p == 'y' || *p == 't'); }

    // Make a KM M blob.  M should be able to read it.
    CHECK_RESULT_OK(km_m.makeblob(b));
    CHECK_RESULT_OK(km_m.checkblob(b));

    // N won't like this blob until it's upgraded.  Then it'll work OK.
    CHECK_RESULT(KM_ERROR_KEY_REQUIRES_UPGRADE, km_n.checkblob(b));
    CHECK_RESULT_OK(km_n.upgrade(bb, b));
    CHECK_RESULT_OK(km_n.checkblob(bb));

    // Make an N blob.  N will like it fine; M won't accept it whatever we do.
    CHECK_RESULT_OK(km_n.makeblob(b));
    CHECK_RESULT_OK(km_n.checkblob(b));
    CHECK_RESULT(KM_ERROR_INVALID_KEY_BLOB, km_m.checkblob(b));

    // Load a blob from the real M TA.
    if (test_loadblob) {
        param kp[] {
            { KM_TAG_PADDING,           KM_PAD_RSA_PSS },
            { KM_TAG_DIGEST,            KM_DIGEST_SHA1 }
        };
        keymaster_key_param_set_t kps {kp, N_ARRAY_ELTS(kp)};
        load_key_blob(b, "rsa-v1.blob");
        CHECK_RESULT_OK(km_m.checkblob(b, &kps));
        CHECK_RESULT(KM_ERROR_KEY_REQUIRES_UPGRADE, km_n.checkblob(b, &kps));
        CHECK_RESULT_OK(km_n.upgrade(bb, b));
        CHECK_RESULT_OK(km_n.checkblob(bb, &kps));
    }

    // Load a more ambitious blob...
    if (test_loadblob) {
        static const uint8_t appid[] = { 1, 2, 3 };
        static const uint8_t appdata[] = { 4, 5, 6, 7 };
        param kp[] {
            { KM_TAG_DIGEST,            KM_DIGEST_SHA_2_256 },
            { KM_TAG_APPLICATION_ID,    appid, sizeof(appid) },
            { KM_TAG_APPLICATION_DATA,  appdata, sizeof(appdata) }
        };
        keymaster_key_param_set_t kps {kp, N_ARRAY_ELTS(kp)};
        load_key_blob(b, "ec-v1+id+data.blob");
        CHECK_RESULT_OK(km_m.checkblob(b, &kps));
        CHECK_RESULT(KM_ERROR_KEY_REQUIRES_UPGRADE, km_n.checkblob(b, &kps));
        CHECK_RESULT_OK(km_n.upgrade(bb, b, &kps));
        CHECK_RESULT_OK(km_n.checkblob(bb, &kps));
    }

end:
    return res;
}
