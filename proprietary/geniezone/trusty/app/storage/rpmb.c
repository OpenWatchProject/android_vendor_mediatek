/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <trusty_std.h>

#include <openssl/mem.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

#include "rpmb.h"

#define RPMB_DEBUG 0

#define MMC_READ_MULTIPLE_BLOCK  18
#define MMC_WRITE_MULTIPLE_BLOCK 25
#define MMC_RELIABLE_WRITE_FLAG (1 << 31)

#define MMC_RSP_PRESENT         (1 << 0)
#define MMC_RSP_CRC             (1 << 2)
#define MMC_RSP_OPCODE          (1 << 4)
#define MMC_CMD_ADTC            (1 << 5)
#define MMC_RSP_SPI_S1          (1 << 7)
#define MMC_RSP_R1              (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_SPI_R1          (MMC_RSP_SPI_S1)

struct rpmb_nonce {
    uint8_t     byte[16];
};

struct rpmb_u16 {
    uint8_t     byte[2];
};

struct rpmb_u32 {
    uint8_t     byte[4];
};

struct rpmb_packet {
    uint8_t              pad[196];
    struct rpmb_key      key_mac;
    uint8_t              data[256];
    struct rpmb_nonce    nonce;
    struct rpmb_u32      write_counter;
    struct rpmb_u16      address;
    struct rpmb_u16      block_count;
    struct rpmb_u16      result;
    struct rpmb_u16      req_resp;
};

enum rpmb_request {
    RPMB_REQ_PROGRAM_KEY                = 0x0001,
    RPMB_REQ_GET_COUNTER                = 0x0002,
    RPMB_REQ_DATA_WRITE                 = 0x0003,
    RPMB_REQ_DATA_READ                  = 0x0004,
    RPMB_REQ_RESULT_READ                = 0x0005,
};

enum rpmb_response {
    RPMB_RESP_PROGRAM_KEY               = 0x0100,
    RPMB_RESP_GET_COUNTER               = 0x0200,
    RPMB_RESP_DATA_WRITE                = 0x0300,
    RPMB_RESP_DATA_READ                 = 0x0400,
};

enum rpmb_result {
    RPMB_RES_OK                         = 0x0000,
    RPMB_RES_GENERAL_FAILURE            = 0x0001,
    RPMB_RES_AUTH_FAILURE               = 0x0002,
    RPMB_RES_COUNT_FAILURE              = 0x0003,
    RPMB_RES_ADDR_FAILURE               = 0x0004,
    RPMB_RES_WRITE_FAILURE              = 0x0005,
    RPMB_RES_READ_FAILURE               = 0x0006,
    RPMB_RES_NO_AUTH_KEY                = 0x0007,

    RPMB_RES_WRITE_COUNTER_EXPIRED      = 0x0080,
};

struct rpmb_state {
    struct rpmb_key     key;
    void                *mmc_handle;
    uint32_t            write_counter;
};

static struct rpmb_u16 rpmb_u16(uint16_t val)
{
    struct rpmb_u16 ret = {{
        val >> 8,
        val >> 0,
    }};
    return ret;
}

static struct rpmb_u32 rpmb_u32(uint32_t val)
{
    struct rpmb_u32 ret = {{
        val >> 24,
        val >> 16,
        val >> 8,
        val >> 0,
    }};
    return ret;
}

static uint16_t rpmb_get_u16(struct rpmb_u16 u16)
{
    size_t i;
    uint16_t val;

    val = 0;
    for (i = 0; i < sizeof(u16.byte); i++)
        val = val << 8 | u16.byte[i];

    return val;
}

static uint32_t rpmb_get_u32(struct rpmb_u32 u32)
{
    size_t i;
    uint32_t val;

    val = 0;
    for (i = 0; i < sizeof(u32.byte); i++)
        val = val << 8 | u32.byte[i];

    return val;
}

#if RPMB_DEBUG
#define rpmb_dprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define rpmb_dprintf(fmt, ...) do { } while (0)
#endif

static void rpmb_dprint_buf(const char *prefix, const uint8_t *buf, size_t size)
{
#if RPMB_DEBUG
    size_t i, j;

    rpmb_dprintf("%s", prefix);
    for (i = 0; i < size; i++) {
        if (i && i % 32 == 0) {
            rpmb_dprintf("\n");
            j = strlen(prefix);
            while (j--)
                rpmb_dprintf(" ");
        }
        rpmb_dprintf(" %02x", buf[i]);
    }
    rpmb_dprintf("\n");
#endif
}

static void rpmb_dprint_u16(const char *prefix, const struct rpmb_u16 u16)
{
    rpmb_dprint_buf(prefix, u16.byte, sizeof(u16.byte));
}

static void rpmb_dprint_u32(const char *prefix, const struct rpmb_u32 u32)
{
    rpmb_dprint_buf(prefix, u32.byte, sizeof(u32.byte));
}

static void rpmb_dprint_key(const char *prefix, const struct rpmb_key key,
                            const char *expected_prefix,
                            const struct rpmb_key expected_key)
{
#if RPMB_DEBUG
    rpmb_dprint_buf(prefix, key.byte, sizeof(key.byte));
    if (CRYPTO_memcmp(key.byte, expected_key.byte, sizeof(key.byte)))
        rpmb_dprint_buf(expected_prefix, expected_key.byte,
                        sizeof(expected_key.byte));
#endif
}

static struct rpmb_nonce rpmb_nonce_init(void)
{
    struct rpmb_nonce rpmb_nonce;

    RAND_bytes(rpmb_nonce.byte, sizeof(rpmb_nonce.byte));

    return rpmb_nonce;
}

static int rpmb_mac(struct rpmb_key key,
                    struct rpmb_packet *packet, size_t packet_count,
                    struct rpmb_key *mac)
{
    size_t i;
    int hmac_ret;
    unsigned int md_len;
    HMAC_CTX hmac_ctx;

    HMAC_CTX_init(&hmac_ctx);
    hmac_ret = HMAC_Init_ex(&hmac_ctx, &key, sizeof(key), EVP_sha256(), NULL);
    if (!hmac_ret) {
        fprintf(stderr, "HMAC_Init_ex failed\n");
        goto err;
    }
    for (i = 0; i < packet_count; i++) {
        STATIC_ASSERT(sizeof(*packet) - offsetof(typeof(*packet), data) == 284);
        hmac_ret = HMAC_Update(&hmac_ctx, packet[i].data, 284);
        if (!hmac_ret) {
            fprintf(stderr, "HMAC_Update failed\n");
            goto err;
        }
    }
    hmac_ret = HMAC_Final(&hmac_ctx, mac->byte, &md_len);
    if (md_len != sizeof(mac->byte)) {
        fprintf(stderr, "bad md_len %d != %zd\n", md_len, sizeof(mac->byte));
        exit(1);
    }
    if (!hmac_ret) {
        fprintf(stderr, "HMAC_Final failed\n");
        goto err;
    }

err:
    HMAC_CTX_cleanup(&hmac_ctx);
    return hmac_ret ? 0 : -1;
}

static int rpmb_check_response(const char *cmd_str, enum rpmb_response response_type,
                               struct rpmb_packet *res, int res_count,
                               struct rpmb_key *mac, struct rpmb_nonce *nonce,
                               uint16_t *addrp)
{
    int i;
    for (i = 0; i < res_count; i++) {
        if (rpmb_get_u16(res[i].req_resp) != response_type) {
            fprintf(stderr, "%s: Bad response type, 0x%x, expected 0x%x\n",
                    cmd_str, rpmb_get_u16(res[i].req_resp), response_type);
            return -1;
        }

        if (rpmb_get_u16(res[i].result) != RPMB_RES_OK) {
            if (rpmb_get_u16(res[i].result) == RPMB_RES_ADDR_FAILURE) {
                fprintf(stderr, "%s: Addr failure, %u\n",
                        cmd_str, rpmb_get_u16(res[i].address));
                return -ENOENT;
            }
            fprintf(stderr, "%s: Bad result, 0x%x\n", cmd_str, rpmb_get_u16(res[i].result));

            if (rpmb_get_u16(res[i].result) == RPMB_RES_GENERAL_FAILURE)
                fprintf(stderr, "%s: RPMB_RES_GENERAL_FAILURE, unknown failure!\n", __func__);
            else if (rpmb_get_u16(res[i].result) == RPMB_RES_AUTH_FAILURE)
                fprintf(stderr, "%s: RPMB_RES_AUTH_FAILURE, key is mismatched!\n", __func__);
            else if (rpmb_get_u16(res[i].result) == RPMB_RES_COUNT_FAILURE)
                fprintf(stderr, "%s: RPMB_RES_COUNT_FAILURE, wrong counter!\n", __func__);
            else if (rpmb_get_u16(res[i].result) == RPMB_RES_WRITE_FAILURE)
                fprintf(stderr, "%s: RPMB_RES_WRITE_FAILURE, wrong counter!\n", __func__);
            else if (rpmb_get_u16(res[i].result) == RPMB_RES_READ_FAILURE)
                fprintf(stderr, "%s: RPMB_RES_READ_FAILURE, wrong counter!\n", __func__);
            else if (rpmb_get_u16(res[i].result) == RPMB_RES_NO_AUTH_KEY)
                fprintf(stderr, "%s: RPMB_RES_NO_AUTH_KEY, key is not programmed!\n", __func__);
            else if (rpmb_get_u16(res[i].result) == RPMB_RES_WRITE_COUNTER_EXPIRED)
                fprintf(stderr, "%s: RPMB_RES_WRITE_COUNTER_EXPIRED, counter is expired!\n", __func__);

            return -1;
        }

        if (i == res_count - 1 && mac && CRYPTO_memcmp(res[i].key_mac.byte, mac->byte, sizeof(mac->byte))) {
            fprintf(stderr, "%s: Bad MAC\n", cmd_str);
            return -1;
        }

        if (nonce && CRYPTO_memcmp(res[i].nonce.byte, nonce->byte, sizeof(nonce->byte))) {
            fprintf(stderr, "%s: Bad nonce\n", cmd_str);
            return -1;
        }

        if (addrp && *addrp != rpmb_get_u16(res[i].address)) {
            fprintf(stderr, "%s: Bad addr, got %u, expected %u\n",
                    cmd_str, rpmb_get_u16(res[i].address), *addrp);
            return -1;
        }
    }

    return 0;
}

static int rpmb_read_counter(struct rpmb_state *state, uint32_t *write_counter)
{
    int ret;
    struct rpmb_key mac;
    struct rpmb_nonce nonce = rpmb_nonce_init();
    struct rpmb_packet cmd = {
        .nonce = nonce,
        .req_resp = rpmb_u16(RPMB_REQ_GET_COUNTER),
    };
    struct rpmb_packet res;

    ret = rpmb_send(state->mmc_handle, NULL, 0, &cmd, sizeof(cmd), &res, sizeof(res), false);
    if (ret < 0)
        return ret;

    ret = rpmb_mac(state->key, &res, 1, &mac);
    if (ret < 0)
        return ret;

    rpmb_dprintf("rpmb: read counter response:\n");
    rpmb_dprint_key("  key/mac       ", res.key_mac, "   expected mac ", mac);
    rpmb_dprint_buf("  nonce         ", res.nonce.byte, sizeof(res.nonce.byte));
    rpmb_dprint_u32("  write_counter ", res.write_counter);
    rpmb_dprint_u16("  result        ", res.result);
    rpmb_dprint_u16("  req/resp      ", res.req_resp);

    if (write_counter)
        *write_counter = rpmb_get_u32(res.write_counter);

    ret = rpmb_check_response("read counter", RPMB_RESP_GET_COUNTER,
                              &res, 1, &mac, &nonce, NULL);

    return ret;
}

int rpmb_read(struct rpmb_state *state, void *buf, uint16_t addr, uint16_t count)
{
    int i;
    int ret;
    struct rpmb_key mac;
    struct rpmb_nonce nonce = rpmb_nonce_init();
    struct rpmb_packet cmd = {
        .nonce = nonce,
        .address = rpmb_u16(addr),
        .req_resp = rpmb_u16(RPMB_REQ_DATA_READ),
    };
    struct rpmb_packet res[count];
    uint8_t *bufp;

    if (!state)
        return -EINVAL;

    ret = rpmb_send(state->mmc_handle, NULL, 0, &cmd, sizeof(cmd), res, sizeof(res), false);
    if (ret < 0)
        return ret;

    ret = rpmb_mac(state->key, res, count, &mac);
    if (ret < 0)
        return ret;

    rpmb_dprintf("rpmb: read data, addr %d, count %d, response:\n", addr, count);
    for (i = 0; i < count; i++) {
        rpmb_dprintf("  block %d\n", i);
        if (i == count - 1)
            rpmb_dprint_key("    key/mac       ", res[i].key_mac, "     expected mac ", mac);
        rpmb_dprint_buf("    data          ", res[i].data, sizeof(res[i].data));
        rpmb_dprint_buf("    nonce         ", res[i].nonce.byte, sizeof(res[i].nonce.byte));
        rpmb_dprint_u16("    address       ", res[i].address);
        rpmb_dprint_u16("    block_count   ", res[i].block_count);
        rpmb_dprint_u16("    result        ", res[i].result);
        rpmb_dprint_u16("    req/resp      ", res[i].req_resp);
    }

    ret = rpmb_check_response("read data", RPMB_RESP_DATA_READ,
                              res, count, &mac, &nonce, &addr);
    if (ret < 0)
        return ret;

    for (bufp = buf, i = 0; i < count; i++, bufp += sizeof(res[i].data))
        memcpy(bufp, res[i].data, sizeof(res[i].data));

    return 0;
}

static int rpmb_write_data(struct rpmb_state *state, const char *buf, uint16_t addr, uint16_t count, bool sync)
{
    int i;
    int ret;
    struct rpmb_key mac;
    struct rpmb_packet cmd[count];
    struct rpmb_packet rescmd = {
        .req_resp = rpmb_u16(RPMB_REQ_RESULT_READ),
    };
    struct rpmb_packet res;

    rpmb_dprintf("rpmb: write data, addr %d, count %d\n", addr, count);
    for (i = 0; i < count; i++) {
        memset(&cmd[i], 0, sizeof(cmd[i]));
        memcpy(cmd[i].data, buf + i * sizeof(cmd[i].data), sizeof(cmd[i].data));
        rpmb_dprint_buf("    data          ", cmd[i].data, sizeof(cmd[i].data));
        cmd[i].write_counter = rpmb_u32(state->write_counter);
        cmd[i].address = rpmb_u16(addr);
        cmd[i].block_count = rpmb_u16(count);
        cmd[i].req_resp = rpmb_u16(RPMB_REQ_DATA_WRITE);
    }
    ret = rpmb_mac(state->key, cmd, count, &cmd[count - 1].key_mac);
    if (ret < 0)
        return ret;

    ret = rpmb_send(state->mmc_handle, cmd, sizeof(cmd), &rescmd, sizeof(rescmd), &res, sizeof(res), sync);
    if (ret < 0)
        return ret;

    ret = rpmb_mac(state->key, &res, 1, &mac);
    if (ret < 0)
        return ret;

    rpmb_dprintf("rpmb: write data, addr %d, count %d, write_counter %d, response\n", addr, count, state->write_counter);
    rpmb_dprint_key("  key/mac       ", res.key_mac, "   expected mac ", mac);
    rpmb_dprint_buf("  nonce         ", res.nonce.byte, sizeof(res.nonce.byte));
    rpmb_dprint_u32("  write_counter ", res.write_counter);
    rpmb_dprint_u16("  address       ", res.address);
    rpmb_dprint_u16("  result        ", res.result);
    rpmb_dprint_u16("  req/resp      ", res.req_resp);

    ret = rpmb_check_response("write data", RPMB_RESP_DATA_WRITE,
                              &res, 1, &mac, NULL, &addr);
    if (ret < 0) {
        if (rpmb_get_u16(res.result) == RPMB_RES_COUNT_FAILURE)
            state->write_counter = 0; /* clear counter to trigger a re-read */
        return ret;
    }

    state->write_counter++;

    return 0;
}

int rpmb_write(struct rpmb_state *state, const void *buf, uint16_t addr, uint16_t count, bool sync)
{
    int ret;
    int retry = 10;

    if (!state)
        return -EINVAL;

_retry:
    /* read counter every write request */
    ret = rpmb_read_counter(state, &state->write_counter);
    if (ret < 0)
        return ret;

    /* other TEE may access rpmb at the same time */
    /* wc may not sync, so retry if failed to write data */
    ret = rpmb_write_data(state, buf, addr, count, sync);
    if (ret < 0) {
        fprintf(stderr, "rpmb: write data failed(%d), retry = %d\n", ret, retry);
        if (--retry)
            goto _retry;
        else
            goto _end;
    }

_end:
    return ret;
}


int rpmb_init(struct rpmb_state **statep,
              void *mmc_handle,
              const struct rpmb_key *key)
{
    struct rpmb_state *state = malloc(sizeof(*state));

    if (!state)
        return -ENOMEM;

    state->mmc_handle = mmc_handle;
    state->key = *key;
    state->write_counter = 0;

    *statep = state;

    return 0;
}

void rpmb_uninit(struct rpmb_state *statep)
{
    free(statep);
}
