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

#pragma once

#define HWRNG_PORT "com.android.trusty.hwrng"

/*
 * The hwrng protocol works as follows:
 * 1) Client opens channel to the server, then sends one or more
 *    requests and receives replies.
 *
 *    Client is allowed to keep the channel opened for the duration
 *    of the session.
 *
 *    Client is allowed to open multiple channels, all such channels
 *    should be treated independently.
 *
 *    Client is allowed to issue multiple requests over the same channel.
 *
 * 2) Server queues requests, and may reply with data as it
 *    is retrieved.
 *
 *    The response is comprised of only bytes of random data. No
 *    protocol header is provided.
 *
 *    Server is allowed to combine such requests together and return all
 *    results in a single message.
 *
 *    Server is allowed to return requested RNG data as a series of smaller
 *    chunks at it finds fit.
 */

/**
 * struct hwrng_req - Request structure for communicating with the hardware RNG
 * @len: the length in bytes of random data requested
 *
 */
struct hwrng_req {
    uint32_t len;
};

