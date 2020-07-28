/*
 * Copyright (c) 2015-2018 TRUSTONIC LIMITED
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

#include "test_km_util.h"

#include "log.h"

uint32_t block_length(
    keymaster_digest_t digest)
{
    switch (digest) {
        case KM_DIGEST_MD5:
            return 128;
        case KM_DIGEST_SHA1:
            return 160;
        case KM_DIGEST_SHA_2_224:
            return 224;
        case KM_DIGEST_SHA_2_256:
            return 256;
        case KM_DIGEST_SHA_2_384:
            return 384;
        case KM_DIGEST_SHA_2_512:
            return 512;
        default:
            return 0;
    }
}

void km_free_blob(
    keymaster_blob_t *blob)
{
    if (blob != NULL) {
        free((void*)blob->data);
        blob->data = NULL;
        blob->data_length = 0;
    }
}

void km_free_key_blob(
    keymaster_key_blob_t *key_blob)
{
    if (key_blob != NULL) {
        free((void*)key_blob->key_material);
        key_blob->key_material = NULL;
        key_blob->key_material_size = 0;
    }
}

keymaster_error_t save_key_blob(
    const char *name, const keymaster_key_blob_t *blob)
{
    int rc;
    FILE *fp = fopen(name, "wb");

    if (!fp ||
        fwrite(blob->key_material,
            1, blob->key_material_size, fp) < blob->key_material_size ||
        fflush(fp) || ferror(fp) || (rc = fclose(fp), fp = 0, rc))
        goto bad;
    return KM_ERROR_OK;
bad:
    if (fp) fclose(fp);
    LOG_E("failed to write blob file `%s': %s\n", name, strerror(errno));
    return KM_ERROR_UNKNOWN_ERROR;
}

bool equal_sglists(const struct sgentry *a, size_t na,
    const struct sgentry *b, size_t nb)
{
    const unsigned char *ap = NULL, *bp = NULL;
    size_t asz = 0, bsz = 0;
    size_t n;

    /* Work through the chunks in the scatter-gather lists. */
    for (;;) {

        /* Refill our working pointers from the scatter-gather lists if we've
         * run out of current buffer.  (Also works for initial setup.)  Once
         * this is done, ASZ is zero if and only if we've used up all of input
         * A, and similarly for BSZ.
         */
        while (!asz && na) {
            ap = static_cast<const unsigned char *>(a->p);
            asz = a->sz;
            a++;
            na--;
        }
        while (!bsz && nb) {
            bp = static_cast<const unsigned char *>(b->p);
            bsz = b->sz;
            b++;
            nb--;
        }

        /* Compare the longest possible prefixes of our two working buffers.
         * We'll exhaust at least one of them.
         */
        n = asz < bsz ? asz : bsz;

        /* From the property above, N will be zero only if we've exhausted one
         * of the inputs.
         */
        if (!n) break;

        /* Actually do the comparison, and update our state. */
        if (memcmp(ap, bp, n) != 0) return false;
        ap += n; asz -= n;
        bp += n; bsz -= n;
    }

    /* If ASZ and BSZ are both zero, then we must have finished both, and there
     * is nothing left to compare.  In that case, the two lists contained equal
     * data.  Otherwise, there's something left over: one of the lists contained
     * a prefix of the other, and they're not quite equal.
     */
    return !asz && !bsz;
}

void add_to_sglist(struct sgentry *sg, size_t *n, size_t max,
    const void *p, size_t sz)
{
    (void)max; /* only used in assertion */
    assert(*n < max);
    if (sz) {
        sg[*n].p = p;
        sg[*n].sz = sz;
        (*n)++;
    }
}
