/*
 * Copyright (C) 2014-2015 The Android Open Source Project
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

#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>

#define LOG_TAG "ipc-unittest-main"

#include <app/ipc_unittest/common.h>
#include <app/ipc_unittest/uuids.h>

#include <trace.h>

/*  */
static uint _tests_total  = 0; /* Number of conditions checked */
static uint _tests_failed = 0; /* Number of conditions failed  */

static const uuid_t srv_app_uuid = IPC_UNITTEST_SRV_APP_UUID;

/*
 *   Begin and end test macro
 */
#define TEST_BEGIN(name)                                        \
	bool _all_ok = true;                                    \
	const char *_test = name;                               \
	TLOGI("%s:\n", _test);


#define TEST_END                                                \
{                                                               \
	if (_all_ok)                                            \
		TLOGI("%s: PASSED\n", _test);                   \
	else                                                    \
		TLOGI("%s: FAILED\n", _test);                   \
}

/*
 * EXPECT_* macros to check test results.
 */
#define EXPECT_EQ(expected, actual, msg)                        \
{                                                               \
	typeof(actual) _e = expected;                           \
	typeof(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_e != _a) {                                         \
		TLOGI("%s: expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",               \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}

#define EXPECT_GT(expected, actual, msg)                        \
{                                                               \
	typeof(actual) _e = expected;                           \
	typeof(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_e <= _a) {                                         \
		TLOGI("%s: expected " #expected " (%d), "      \
		    "actual " #actual " (%d)\n",                \
		    msg, (int)_e, (int)_a);                     \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}

#define EXPECT_GE_ZERO(actual, msg)                             \
{                                                               \
	typeof(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_a < 0) {                                           \
		TLOGI("%s: expected >= 0 "                     \
		    "actual " #actual " (%d)\n", msg, (int)_a); \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}


#define EXPECT_GT_ZERO(actual, msg)                             \
{                                                               \
	typeof(actual) _a = actual;                             \
	_tests_total++;                                         \
	if (_a <= 0) {                                          \
		TLOGI("%s: expected > 0 "                      \
		    "actual " #actual " (%d)\n", msg, (int)_a); \
		_tests_failed++;                                \
		_all_ok = false;                                \
	}                                                       \
}

/****************************************************************************/

/*
 *  Fill specified buffer with incremental pattern
 */
static void fill_test_buf(uint8_t *buf, size_t cnt, uint8_t seed)
{
	if (!buf || !cnt)
		return;

	while (cnt--) {
		*buf++ = seed++;
	}
}

/*
 *  Local wrapper on top of async connect that provides
 *  synchronos connect with timeout.
 */
int sync_connect(const char *path, uint timeout)
{
	int rc;
	uevent_t evt;
	handle_t chan;

	rc = connect(path, IPC_CONNECT_ASYNC | IPC_CONNECT_WAIT_FOR_PORT);
	if (rc >= 0) {
		chan = (handle_t) rc;
		rc = wait(chan, &evt, timeout);
		if (rc == 0) {
			rc = ERR_BAD_STATE;
			if (evt.handle == chan) {
				if (evt.event & IPC_HANDLE_POLL_READY)
					return chan;

				if (evt.event & IPC_HANDLE_POLL_HUP)
					rc = ERR_CHANNEL_CLOSED;
			}
		}
		close(chan);
	}
	return rc;
}


/****************************************************************************/

/*
 *  wait on handle negative test
 */
static void run_wait_negative_test(void)
{
	int rc;
	uevent_t event;
	lk_time_t timeout = 1000;  // 1 sec

	TEST_BEGIN(__func__);

	/* waiting on invalid (negative value) handle. */
	rc = wait(INVALID_IPC_HANDLE, &event, timeout);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "wait on invalid handle");

	/* waiting on an invalid (out of range (big positive)) handle. */
	rc = wait(MAX_USER_HANDLES, &event, timeout);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "wait on invalid handle");

	/* waiting on non-existing handle that is in valid range. */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		rc = wait(i, &event, timeout);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "wait on invalid handle");
	}

	TEST_END
}

/*
 *  Close handle unittest
 */
static void run_close_handle_negative_test(void)
{
	int rc;

	TEST_BEGIN(__func__);

	/* closing an invalid (negative value) handle. */
	rc = close(INVALID_IPC_HANDLE);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "closing invalid handle");

	/* closing an invalid (out of range (big positive)) handle. */
	rc = close(MAX_USER_HANDLES);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "closing invalid handle");

	/* closing non-existing handle that is in valid range. */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		rc = close(i);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "closing invalid handle");
	}

	TEST_END
}

/*
 *  Set cookie negative unittest
 */
static void  run_set_cookie_negative_test(void)
{
	int rc;

	TEST_BEGIN(__func__);

	/* set cookie for invalid (negative value) handle. */
	rc = set_cookie(INVALID_IPC_HANDLE, (void *) 0x1BEEF);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "set cookie for invalid handle");

	/* set cookie for an invalid (out of range (big positive)) handle. */
	rc = set_cookie(MAX_USER_HANDLES, (void *) 0x2BEEF);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "set cookie for invalid handle");

	/* set cookie for non-existing handle that is in valid range. */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		rc = set_cookie(i, (void *) 0x3BEEF);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "set cookie for invalid handle");
	}

	TEST_END
}


/****************************************************************************/

/*
 *  Port create unittest
 */
static void run_port_create_negative_test(void)
{
	int  rc;
	char path[MAX_PORT_PATH_LEN + 16];

	TEST_BEGIN(__func__);

	/* create port with empty path */
	path[0] = '\0';
	rc = port_create(path, 2, 64, 0);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "empty path srv");

	/* create port with zero buffers */
        sprintf(path, "%s.port", SRV_PATH_BASE);
	rc = port_create(path, 0, 64, 0);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "no buffers");

	/* create port with zero buffer size */
        sprintf(path, "%s.port", SRV_PATH_BASE);
	rc = port_create(path, 2, 0, 0);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "zero buf size");

	/* create port with large number of buffers */
        sprintf(path, "%s.port", SRV_PATH_BASE);
	rc = port_create(path, MAX_PORT_BUF_NUM * 100, 64, 0);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "large buf num");

	/* create port with large buffer size */
        sprintf(path, "%s.port", SRV_PATH_BASE);
	rc = port_create(path,  2, MAX_PORT_BUF_SIZE * 100, 0);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "large buf size");

	/* create port with path oversized name */
	int len = sprintf(path, "%s.port", SRV_PATH_BASE);
	for (uint i = len; i < sizeof(path); i++) path[i] = 'a';
	path[sizeof(path)-1] = '\0';
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE, 0);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "path is too long");
	rc = close (rc);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "close port");

	TEST_END
}

static void run_port_create_test (void)
{
	int  rc;
	uint i;
	char path[MAX_PORT_PATH_LEN];
	handle_t ports[MAX_USER_HANDLES];

	TEST_BEGIN(__func__);

	/* create maximum number of ports */
	for (i = 2; i < MAX_USER_HANDLES-1; i++) {
		sprintf(path, "%s.port.%s%d", SRV_PATH_BASE, "test", i);
		rc = port_create(path, 2, MAX_PORT_BUF_SIZE, 0);
		EXPECT_GE_ZERO (rc, "create ports");
		ports[i] = (handle_t) rc;

		/* create a new port that collide with an existing port */
		rc = port_create(path, 2, MAX_PORT_BUF_SIZE, 0);
		EXPECT_EQ (ERR_ALREADY_EXISTS, rc, "create existing port");
	}

	/* create one more that should succeed */
	sprintf(path, "%s.port.%s%d", SRV_PATH_BASE, "test", i);
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE, 0);
	EXPECT_GE_ZERO (rc, "create ports");
	ports[i] = (handle_t) rc;

	/* but creating colliding port should fail with different
	   error code because we actually exceeded max number of
	   handles instead of colliding with an existing path */
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE, 0);
	EXPECT_EQ (ERR_NO_RESOURCES, rc, "create existing port");

	sprintf(path, "%s.port.%s%d", SRV_PATH_BASE, "test", MAX_USER_HANDLES);
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE, 0);
	EXPECT_EQ (ERR_NO_RESOURCES, rc, "max ports");

	/* close them all  */
	for (i = 2; i < MAX_USER_HANDLES; i++) {
		/* close a valid port  */
		rc = close(ports[i]);
		EXPECT_EQ (NO_ERROR, rc, "closing port");

		/* close previously closed port. It should fail! */
		rc = close(ports[i]);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "closing closed port");

		ports[i] = INVALID_IPC_HANDLE;
	}

	TEST_END
}

/*
 *
 */
static void run_wait_on_port_test (void)
{
	int rc;
	uevent_t event;
	char path[MAX_PORT_PATH_LEN];
	handle_t ports[MAX_USER_HANDLES];

	TEST_BEGIN(__func__);

	#define COOKIE_BASE 100

	/* create maximum number of ports */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		sprintf(path, "%s.port.%s%d", SRV_PATH_BASE, "test", i);
		rc = port_create(path, 2, MAX_PORT_BUF_SIZE, 0);
		EXPECT_GE_ZERO (rc, "max ports");
		ports[i] = (handle_t) rc;

		rc = set_cookie(ports[i], (void *) (COOKIE_BASE + i));
		EXPECT_EQ (NO_ERROR, rc, "set cookie on port");
	}

	/* wait on each individual port */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		/* wait with zero timeout */
		rc = wait(ports[i], &event, 0);
		EXPECT_EQ (ERR_TIMED_OUT, rc, "zero timeout");

		/* wait with non-zero timeout */
		rc = wait(ports[i], &event, 100);
		EXPECT_EQ (ERR_TIMED_OUT, rc, "non-zero timeout");
	}

	/* wait on all ports with zero timeout */
	rc = wait_any(&event, 0);
	EXPECT_EQ (ERR_TIMED_OUT, rc, "zero timeout");

	/* wait on all ports with non-zero timeout*/
	rc = wait_any(&event, 100);
	EXPECT_EQ (ERR_TIMED_OUT, rc, "non-zero timeout");

	/* close them all */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		/* close a valid port  */
		rc = close(ports[i]);
		EXPECT_EQ (NO_ERROR, rc, "closing closed port");
		ports[i] = INVALID_IPC_HANDLE;
	}

	TEST_END
}

/****************************************************************************/

/*
 *  Connect unittests
 */
static void run_connect_negative_test (void)
{
	int  rc;
	char path[MAX_PORT_PATH_LEN + 16];
	lk_time_t connect_timeout = 1000;  // 1 sec

	TEST_BEGIN(__func__);

	/* try to connect to port with an empty name */
	rc = sync_connect("", connect_timeout);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "empty path");

	/* try to connect to non-existing port  */
	sprintf(path, "%s.conn.%s", SRV_PATH_BASE, "blah-blah");
	rc = connect(path, 0);
	EXPECT_EQ (ERR_NOT_FOUND, rc, "non-existing path");

	/* try to connect to non-existing port  */
	sprintf(path, "%s.conn.%s", SRV_PATH_BASE, "blah-blah");
	rc = connect(path, IPC_CONNECT_ASYNC);
	EXPECT_EQ (ERR_NOT_FOUND, rc, "non-existing path");

	/* try to connect to port with very long name */
	int len = sprintf(path, "%s.conn.", SRV_PATH_BASE);
	for (uint i = len; i < sizeof(path); i++) path[i] = 'a';
	path[sizeof(path)-1] = '\0';
	rc = sync_connect (path, connect_timeout);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "long path");

	rc = close (rc);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "close channel");

	TEST_END
}


static void run_connect_close_test(void)
{
	int  rc;
	char path[MAX_PORT_PATH_LEN];
	handle_t chans[16];

	TEST_BEGIN(__func__);

	sprintf(path, "%s.srv.%s", SRV_PATH_BASE, "datasink");

	for (uint j = 2; j < MAX_USER_HANDLES; j++) {
		/* do several iterations to make sure we are not
		   not loosing handles */
		for (uint i = 0; i < countof(chans); i++) {
			rc = sync_connect(path, 1000);
			EXPECT_GE_ZERO (rc, "connect/close");
			chans[i] = (handle_t) rc;
		}

		for (uint i = 0; i < countof(chans); i++) {
			rc = close(chans[i]);
			EXPECT_EQ (NO_ERROR, rc, "connect/close");
		}
	}

	TEST_END
}

static void run_connect_close_by_peer_test(const char *test)
{
	int rc;
	char path[MAX_PORT_PATH_LEN];
	uevent_t event;
	handle_t chans[16];
	uint chan_cnt = 0;

	TEST_BEGIN(__func__);

	#define COOKIE_BASE 100

	/*
	 * open up to 16 connection to specified test port which would
	 * close them all in a different way:
	 */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE, test);
	for (uint i = 0; i  < countof(chans); i++) {
		/* open new connection */
		uint retry_cnt = 10;
		while (retry_cnt) {
			rc = sync_connect(path, 2000);
			if (rc == ERR_NOT_FOUND) {
				/* wait a bit and retry */
				--retry_cnt;
				nanosleep (0, 0, 100 * MSEC);
			} else {
				break;
			}
		}
		EXPECT_GT_ZERO (retry_cnt, test);

		/*
		 * depending on task scheduling connect might return real
		 * handle that will be closed later or it might return
		 * ERR_CHANNEL_CLOSED error if the channel has already been
		 * closed. Both cases are correct and must be handled.
		 */
		if (rc >= 0) {
			/* got real handle */
			chans[i] = (handle_t) rc;

			/* attach cookie for returned channel */
			rc = set_cookie((handle_t) rc,
			                (void*)(COOKIE_BASE + i));
			EXPECT_EQ (NO_ERROR, rc, test);

			chan_cnt++;
		} else {
			/* could be already closed channel */
			EXPECT_EQ (ERR_CHANNEL_CLOSED, rc, test);
		}

		/* check if any channels are closed */
		while ((rc = wait_any(&event, 0)) == NO_ERROR) {
			EXPECT_EQ (IPC_HANDLE_POLL_HUP, event.event, test);
			uint idx = (uint) event.cookie - COOKIE_BASE;
			EXPECT_EQ (chans[idx], event.handle, test);
			EXPECT_GT (countof(chans), idx, test);
			if (idx < countof(chans)) {
				rc = close(chans[idx]);
				EXPECT_EQ (NO_ERROR, rc, test);
				chans[idx] = INVALID_IPC_HANDLE;
			}
			chan_cnt--;
		}
	}

	/* wait until all channels are closed */
	while (chan_cnt) {
		rc = wait_any(&event, 10000);
		EXPECT_EQ (NO_ERROR, rc, test);
		EXPECT_EQ (IPC_HANDLE_POLL_HUP, event.event, test);

		uint idx = (uint) event.cookie - COOKIE_BASE;
		EXPECT_GT (countof(chans), idx, test);
		EXPECT_EQ (chans[idx], event.handle, test);
		if (idx < countof(chans)) {
			rc = close(chans[idx]);
			EXPECT_EQ (NO_ERROR, rc, test);
			chans[idx] = INVALID_IPC_HANDLE;
		}
		chan_cnt--;
	}

	EXPECT_EQ (0, chan_cnt, test);

	TEST_END
}

static void run_async_connect_test (void)
{
	int rc;
	handle_t chan;
	uevent_t event;
	uuid_t peer_uuid = {0};
	char path[MAX_PORT_PATH_LEN];

	TEST_BEGIN(__func__);

	sprintf(path, "%s.main.%s", SRV_PATH_BASE, "async");

	/* connect to non existing port synchronously without wait_for_port */
	rc = connect (path, 0);
	EXPECT_EQ (ERR_NOT_FOUND, rc, "async");
	rc = close ((handle_t)rc);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "async");

	/* connect to non existing port asynchronously without wait_for_port */
	rc = connect (path, IPC_CONNECT_ASYNC);
	EXPECT_EQ (ERR_NOT_FOUND, rc, "async");
	rc = close ((handle_t)rc);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "async");

	/* connect to non existing port asynchronously with wait_for_port */
	rc = connect (path,  IPC_CONNECT_ASYNC | IPC_CONNECT_WAIT_FOR_PORT);
	EXPECT_GE_ZERO(rc, "async");
	if (rc >= 0) {
		chan = (handle_t) rc;

		/* wait on channel */
		rc = wait(chan, &event, 1000);
		EXPECT_EQ (ERR_TIMED_OUT, rc, "async");

		/* and close it */
		rc = close (chan);
		EXPECT_EQ (NO_ERROR, rc, "async");
	}

	/* connect to non-existing port asyncronously with wait_for_port */
	rc = connect (path,  IPC_CONNECT_ASYNC | IPC_CONNECT_WAIT_FOR_PORT);
	EXPECT_GE_ZERO (rc, "async");
	chan = (handle_t) rc;

	if (rc >= 0) {
		handle_t port;
		uint32_t exp_event;

		/* wait on channel for connect */
		rc = wait(chan, &event, 100);
		EXPECT_EQ (ERR_TIMED_OUT, rc, "async");

		/* now create port */
		rc = port_create(path, 1, 64, IPC_PORT_ALLOW_TA_CONNECT);
		EXPECT_GE_ZERO (rc, "async");
		if (rc >= 0) {
			port = (handle_t) rc;

			/* and wait for incomming connections */
			exp_event = IPC_HANDLE_POLL_READY;
			rc = wait(port, &event, 1000);
			EXPECT_EQ (NO_ERROR, rc, "async");
			EXPECT_EQ (exp_event, event.event, "async");

			if (rc == NO_ERROR) {
				handle_t srv_chan;

				/* got one, accept it */
				rc = accept(port, &peer_uuid);
				EXPECT_GE_ZERO (rc, "async");
				srv_chan = (handle_t) rc;

				/* and close it */
				close (srv_chan);

				/* now wait on original chan:
				 * there should be READY and HUP events
				 */
				exp_event = IPC_HANDLE_POLL_READY |
					    IPC_HANDLE_POLL_HUP;
				rc = wait(chan, &event, 1000);
				EXPECT_EQ (NO_ERROR, rc, "async");
				EXPECT_EQ (exp_event, event.event, "async");
			}
			close(port);
		}
		close(chan);
	}

	TEST_END
}


static void run_connect_selfie_test (void)
{
	int rc, rc1;
	uuid_t peer_uuid = {0};
	uuid_t zero_uuid = {0};
	char path[MAX_PORT_PATH_LEN];
	lk_time_t connect_timeout = 1000;  // 1 sec

	TEST_BEGIN(__func__);

	/* Try to connect to port that we register ourself.
	   It is not very usefull scenario, just to make sure that
	   nothing bad is happening */
	sprintf(path, "%s.main.%s", SRV_PATH_BASE, "selfie");
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE,
	                 IPC_PORT_ALLOW_TA_CONNECT);
	EXPECT_GE_ZERO (rc, "selfie");

	if (rc >= 0) {
		handle_t test_port = rc;

		/* Since we are single threaded and cannot accept connection
		 * we will always timeout.
		 */

		/* with non-zero timeout  */
		rc = sync_connect (path, connect_timeout);
		EXPECT_EQ (ERR_TIMED_OUT, rc, "selfie");

		/* with zero timeout */
		rc = sync_connect (path, 0);
		EXPECT_EQ (ERR_TIMED_OUT, rc, "selfie");

		/* since we did not call wait on port yet we have
		 * 2 connection requests pending (attached to port)
		 * teared down by peer (us).
		 */
		uevent_t event;
		uint exp_event = IPC_HANDLE_POLL_READY;

		int rc = wait_any(&event, -1);
		EXPECT_EQ (NO_ERROR, rc, "wait on port");
		EXPECT_EQ (test_port, event.handle, "wait on port");
		EXPECT_EQ (exp_event, event.event,  "wait on port");

		if (rc == NO_ERROR && (event.event & IPC_HANDLE_POLL_READY)) {

			/* we have pending connection, but it is already closed */
			rc = accept (test_port, &peer_uuid);
			EXPECT_EQ (ERR_CHANNEL_CLOSED, rc, "accept");

			rc1 = memcmp(&peer_uuid, &zero_uuid, sizeof(zero_uuid));
			EXPECT_EQ (0, rc1, "accept")

			/* the second one is closed too */
			rc = accept (test_port, &peer_uuid);
			EXPECT_EQ (ERR_CHANNEL_CLOSED, rc, "accept");

			rc1 = memcmp(&peer_uuid, &zero_uuid, sizeof(zero_uuid));
			EXPECT_EQ (0, rc1, "accept")

			/* There should be no more pending connections so next
			   accept should return ERR_NO_MSG */
			rc = accept (test_port, &peer_uuid);
			EXPECT_EQ (ERR_NO_MSG, rc, "accept");

			rc1 = memcmp(&peer_uuid, &zero_uuid, sizeof(zero_uuid));
			EXPECT_EQ (0, rc1, "accept")
		}

		/* add couple connections back and destroy them along with port */
		rc = sync_connect (path, 0);
		EXPECT_EQ (ERR_TIMED_OUT, rc, "selfie");

		rc = sync_connect (path, 0);
		EXPECT_EQ (ERR_TIMED_OUT, rc, "selfie");

		/* close selfie port  */
		rc = close (test_port);
		EXPECT_EQ (NO_ERROR, rc, "close selfie");
	}

	TEST_END
}

static void run_connect_access_test(void)
{
	int rc;
	char path[MAX_PORT_PATH_LEN];

	TEST_BEGIN(__func__);

	/* open connection to NS only accessible service */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE,  "ns_only");
	rc = sync_connect(path, 1000);

	/* It is expected to fail */
	EXPECT_EQ(ERR_ACCESS_DENIED, rc, "connect to ns_only");

	if (rc >= 0)
		close((handle_t)rc);

	/* open connection to TA only accessible service */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE,  "ta_only");
	rc = sync_connect(path, 1000);

	/* it is expected to succeed */
	EXPECT_GE_ZERO(rc, "connect to ta_only");

	if (rc >= 0)
		close((handle_t)rc);

	TEST_END
}

/****************************************************************************/

/*
 *  Accept negative test
 */
static void run_accept_negative_test(void)
{
	int  rc, rc1;
	char path[MAX_PORT_PATH_LEN];
	handle_t chan;
	uuid_t peer_uuid = {0};
	uuid_t zero_uuid = {0};

	TEST_BEGIN(__func__);

	/* accept on invalid (negative value) handle */
	rc = accept(INVALID_IPC_HANDLE, &peer_uuid);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "accept on invalid handle");

	rc1 = memcmp(&peer_uuid, &zero_uuid, sizeof(zero_uuid));
	EXPECT_EQ (0, rc1, "accept")

	/* accept on an invalid (out of range (big positive)) handle */
	rc = accept(MAX_USER_HANDLES, &peer_uuid);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "accept on invalid handle");

	rc1 = memcmp(&peer_uuid, &zero_uuid, sizeof(zero_uuid));
	EXPECT_EQ (0, rc1, "accept")

	/* accept on non-existing handle that is in valid range */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		rc = accept(i, &peer_uuid);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "accept on invalid handle");

		rc1 = memcmp(&peer_uuid, &zero_uuid, sizeof(zero_uuid));
		EXPECT_EQ (0, rc1, "accept")
	}

	/* connect to datasink service */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE, "datasink");
	rc = sync_connect(path, 1000);
	EXPECT_GE_ZERO (rc, "connect to datasink");
	chan = (handle_t) rc;

	/* call accept on channel handle which is an invalid operation */
	rc = accept(chan, &peer_uuid);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "accept on channel");

	rc1 = memcmp(&peer_uuid, &zero_uuid, sizeof(zero_uuid));
	EXPECT_EQ (0, rc1, "accept")

	rc = close(chan);
	EXPECT_EQ (NO_ERROR, rc, "close channnel")

	TEST_END
}

static void run_accept_test (void)
{
	int  rc, rc1;
	uevent_t event;
	char path[MAX_PORT_PATH_LEN];
	handle_t ports[MAX_USER_HANDLES];
	uuid_t peer_uuid = {0};
	uuid_t zero_uuid = {0};

	TEST_BEGIN(__func__);

	#define COOKIE_BASE 100

	/* create maximum number of ports */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		sprintf(path, "%s.port.accept%d", SRV_PATH_BASE, i);
		rc = port_create(path, 2, MAX_PORT_BUF_SIZE,
		                 IPC_PORT_ALLOW_TA_CONNECT);
		EXPECT_GE_ZERO (rc, "max ports");
		ports[i] = (handle_t) rc;

		rc = set_cookie(ports[i], (void *) (COOKIE_BASE + ports[i]));
		EXPECT_EQ (NO_ERROR, rc, "set cookie on port");
	}

	/* poke connect service to initiate connections to us */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE, "connect");
	rc = sync_connect (path, 1000);
	close(rc);

	/* handle incoming connections */
	for (uint i = 2; i < MAX_USER_HANDLES; i++ ) {

		rc = wait_any(&event, 1000);
		EXPECT_EQ (NO_ERROR, rc, "accept test");
		EXPECT_EQ (IPC_HANDLE_POLL_READY, event.event, "accept test");

		/* check port cookie */
		void *exp_cookie = (void *)(COOKIE_BASE + event.handle);
		EXPECT_EQ (exp_cookie, event.cookie, "accept test");

		/* accept connection - should fail because we do not
		   have any room for handles */
		rc = accept (event.handle, &peer_uuid);
		EXPECT_EQ (ERR_NO_RESOURCES, rc, "accept test");

		/* check peer uuid */
		rc1 = memcmp(&peer_uuid, &zero_uuid, sizeof(zero_uuid));
		EXPECT_EQ (0, rc1, "accept test")
	}

	/* free 1 handle  so we have room and repeat test */
	rc = close(ports[2]);
	EXPECT_EQ(NO_ERROR, 0, "close accept test");
	ports[2] = INVALID_IPC_HANDLE;


	/* poke connect service to initiate connections to us */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE, "connect");
	rc = sync_connect (path, 1000);
	close(rc);

	/* handle incoming connections */
	for (uint i = 2; i < MAX_USER_HANDLES-1; i++ ) {

		rc = wait_any(&event, 3000);
		EXPECT_EQ (NO_ERROR, rc, "accept test");
		EXPECT_EQ (IPC_HANDLE_POLL_READY, event.event, "accept test");

		/* check port cookie */
		void *exp_cookie = (void *)(COOKIE_BASE + event.handle);
		EXPECT_EQ (exp_cookie, event.cookie, "accept test");

		rc = accept (event.handle, &peer_uuid);
		EXPECT_EQ (2, rc, "accept test");

		/* check peer uuid */
		rc1 = memcmp(&peer_uuid, &srv_app_uuid, sizeof(srv_app_uuid));
		EXPECT_EQ (0, rc1, "accept test")

		rc = close (rc);
		EXPECT_EQ (NO_ERROR, rc, "accept test");
	}

	/* close them all */
	for (uint i = 3; i < MAX_USER_HANDLES; i++) {
		/* close a valid port  */
		rc = close(ports[i]);
		EXPECT_EQ (NO_ERROR, rc, "close port");
		ports[i] = INVALID_IPC_HANDLE;
	}

	TEST_END
}

/****************************************************************************/

static void run_get_msg_negative_test(void)
{
	int rc;
	ipc_msg_info_t inf;
	handle_t port;
	handle_t chan;
	char path[MAX_PORT_PATH_LEN];

	TEST_BEGIN(__func__);

	/* get_msg on invalid (negative value) handle. */
	rc = get_msg(INVALID_IPC_HANDLE, &inf);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "get_msg on invalid handle");

	/* get_msg on an invalid (out of range (big positive)) handle. */
	rc = get_msg(MAX_USER_HANDLES, &inf);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "get_msg on invalid handle");

	/* get_msg on non-existing handle that is in valid range. */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		rc = get_msg(i, &inf);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "get_msg on invalid handle");
	}

	/* calling get_msg on port handle should fail
	   because get_msg is only applicable to channels */
	sprintf(path, "%s.main.%s", SRV_PATH_BASE,  "datasink");
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE,
	                 IPC_PORT_ALLOW_TA_CONNECT);
	EXPECT_GE_ZERO (rc, "create datasink port");
	port = (handle_t) rc;

	rc = get_msg(port, &inf);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "get_msg on port");
	close(port);

	/* call get_msg on channel that do not have any pending messages */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE,  "datasink");
	rc = sync_connect(path, 1000);
	EXPECT_GE_ZERO (rc, "connect to datasink");
	chan = (handle_t) rc;

	rc = get_msg(chan, &inf);
	EXPECT_EQ (ERR_NO_MSG, rc, "get_msg on empty channel");

	rc = close(chan);
	EXPECT_EQ (NO_ERROR, rc, "close channnel");

	TEST_END
}


static void run_put_msg_negative_test(void)
{
	int rc;
	handle_t port;
	handle_t chan;
	char path[MAX_PORT_PATH_LEN];

	TEST_BEGIN(__func__);

	/* put_msg on invalid (negative value) handle */
	rc = put_msg(INVALID_IPC_HANDLE, 0);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "put_msg on invalid handle");

	/* put_msg on an invalid (out of range (big positive)) handle */
	rc = put_msg(MAX_USER_HANDLES, 0);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "put_msg on invalid handle");

	/* put_msg on non-existing handle that is in valid range */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		rc = put_msg (i, 0);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "put_msg on invalid handle");
	}

	/* calling put_msg on port handle should fail
	   because put_msg is only applicable to channels */
	sprintf(path, "%s.main.%s", SRV_PATH_BASE,  "datasink");
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE,
	                 IPC_PORT_ALLOW_TA_CONNECT);
	EXPECT_GE_ZERO (rc, "create datasink port");
	port = (handle_t) rc;

	rc = put_msg(port, 0);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "put_msg on port");
	rc = close(port);
	EXPECT_EQ (NO_ERROR, rc, "close port");

	/* call put_msg on channel that do not have any pending messages */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE,  "datasink");
	rc = sync_connect(path, 1000);
	EXPECT_GE_ZERO (rc, "connect to datasink");
	chan = (handle_t) rc;

	rc = put_msg(chan, 0);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "put_msg on empty channel");
	rc = close(chan);
	EXPECT_EQ (NO_ERROR, rc, "close channel");

	TEST_END
}


/*
 *  Send 10000 messages to datasink service
 */
static void run_send_msg_test(void)
{
	int rc;
	handle_t chan;
	char path[MAX_PORT_PATH_LEN];
	uint8_t   buf0[64];
	uint8_t   buf1[64];
	iovec_t   iov[2];
	ipc_msg_t msg;

	TEST_BEGIN(__func__);

	/* prepare test buffer */
	fill_test_buf(buf0, sizeof(buf0), 0x55);
	fill_test_buf(buf1, sizeof(buf1), 0x44);

	iov[0].base = buf0;
	iov[0].len  = sizeof(buf0);
	iov[1].base = buf1;
	iov[1].len =  sizeof(buf1);
	msg.num_handles = 0;
	msg.handles = NULL;
	msg.num_iov = 2;
	msg.iov   = iov;

	/* open connection to datasink service */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE,  "datasink");
	rc = sync_connect(path, 1000);
	EXPECT_GE_ZERO (rc, "connect to datasink");

	if (rc >= 0) {
		chan = (handle_t) rc;
		for (uint i = 0; i < 10000; i++) {
			rc = send_msg(chan, &msg);
			if (rc == ERR_NOT_ENOUGH_BUFFER) { /* wait for room */
				uevent_t  uevt;
				uint exp_event = IPC_HANDLE_POLL_SEND_UNBLOCKED;
				rc = wait(chan, &uevt, 1000);
				EXPECT_EQ (NO_ERROR, rc, "waiting for space");
				EXPECT_EQ (chan, uevt.handle, "waiting for space");
				EXPECT_EQ (exp_event, uevt.event,  "waiting for space");
			} else {
				EXPECT_EQ (64, rc, "send_msg bulk")
			}
			if (!_all_ok) {
				TLOGI("%s: abort (rc = %d) test\n", __func__, rc);
				break;
			}
		}
		rc = close (chan);
		EXPECT_EQ (NO_ERROR, rc, "close channel");
	}

	TEST_END
}


static void run_send_msg_negative_test(void)
{
	int rc;
	handle_t port;
	handle_t chan;
	char path[MAX_PORT_PATH_LEN];
	uint8_t buf[64];
	iovec_t iov[2];
	ipc_msg_t msg;

	TEST_BEGIN(__func__);

	/* init msg to empty message */
	memset (&msg, 0, sizeof(msg));

	/* send_msg on invalid (negative value) handle */
	rc = send_msg(INVALID_IPC_HANDLE, &msg);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "send_msg on invalid handle");

	/* calling send_msg with NULL msg should fail for any handle */
	rc = send_msg(INVALID_IPC_HANDLE, NULL);
	EXPECT_EQ (ERR_FAULT, rc, "send_msg on NULL msg");

	/* send_msg on an invalid (out of range (big positive)) handle */
	rc = send_msg(MAX_USER_HANDLES, &msg);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "send_msg on invalid handle");

	/* calling send_msg with NULL msg should fail for any handle */
	rc = send_msg(MAX_USER_HANDLES, NULL);
	EXPECT_EQ (ERR_FAULT, rc, "send_msg on NULL msg");

	/* send_msg on non-existing handle that is in valid range */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		rc = send_msg(i, &msg);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "send on invalid handle");

		/* calling send_msg with NULL msg should fail for any handle */
		rc = send_msg(i, NULL);
		EXPECT_EQ (ERR_FAULT, rc, "send_msg on NULL msg");
	}

	/* calling send_msg on port handle should fail
	   because send_msg is only applicable to channels */
	sprintf(path, "%s.main.%s", SRV_PATH_BASE,  "datasink");
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE,
	                 IPC_PORT_ALLOW_TA_CONNECT);
	EXPECT_GE_ZERO (rc, "create datasink port");
	port = (handle_t) rc;

	rc = send_msg(port, &msg);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "send_msg on port");
	close(port);

	/* open connection to datasink service */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE,  "datasink");
	rc = sync_connect(path, 1000);
	EXPECT_GE_ZERO (rc, "connect to datasink");
	chan = (handle_t) rc;

	/* handles are not supported */
	msg.num_handles = 1;
	rc = send_msg(chan, &msg);
	EXPECT_EQ (ERR_NOT_SUPPORTED, rc, "sending handles");
	msg.num_handles = 0;
	msg.handles  = NULL;

	/* set num_iov to non zero but keep iov ptr NULL */
	msg.num_iov = 1;
	msg.iov  = NULL;
	rc = send_msg(chan, &msg);
	EXPECT_EQ (ERR_FAULT, rc, "sending bad iovec array");

	/*  send msg with iovec with bad base ptr */
	iov[0].len  = sizeof(buf) /  2;
	iov[0].base = NULL;
	iov[1].len  = sizeof(buf) /  2;
	iov[1].base = NULL;
	msg.num_iov = 2;
	msg.iov  = iov;
	rc = send_msg(chan, &msg);
	EXPECT_EQ (ERR_FAULT, rc, "sending bad iovec");

	/*  send msg with iovec with bad base ptr */
	iov[0].len  = sizeof(buf) /  2;
	iov[0].base = buf;
	iov[1].len  = sizeof(buf) /  2;
	iov[1].base = NULL;
	msg.num_iov = 2;
	msg.iov  = iov;
	rc = send_msg(chan, &msg);
	EXPECT_EQ (ERR_FAULT, rc, "sending bad iovec");

	rc = close(chan);
	EXPECT_EQ (NO_ERROR, rc, "close channel");

	TEST_END
}

static void run_read_msg_negative_test(void)
{
	int rc;
	handle_t port;
	handle_t chan;
	uevent_t  uevt;
	char path[MAX_PORT_PATH_LEN];
	uint8_t tx_buf[64];
	uint8_t rx_buf[64];
	ipc_msg_info_t inf;
	ipc_msg_t   tx_msg;
	iovec_t     tx_iov;
	ipc_msg_t   rx_msg;
	iovec_t     rx_iov[2];

	TEST_BEGIN(__func__);

	/* init msg to empty message */
	memset (&rx_msg, 0, sizeof(rx_msg));
	memset (&tx_msg, 0, sizeof(tx_msg));

	/* read_msg on invalid (negative value) handle */
	rc = read_msg(INVALID_IPC_HANDLE, 0, 0, &rx_msg);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "read_msg on invalid handle");

	rc = read_msg(INVALID_IPC_HANDLE, 0, 0, NULL);
	EXPECT_EQ (ERR_FAULT, rc, "read_msg on invalid handle");

	/* calling read_msg with NULL msg should fail for any handle */
	rc = read_msg(MAX_USER_HANDLES, 0, 0, &rx_msg);
	EXPECT_EQ (ERR_BAD_HANDLE, rc, "read_msg on NULL msg");

	rc = read_msg(MAX_USER_HANDLES, 0, 0, NULL);
	EXPECT_EQ (ERR_FAULT, rc, "read_msg on NULL msg");

	/* send_msg on non-existing handle that is in valid range */
	for (uint i = 2; i < MAX_USER_HANDLES; i++) {
		rc = read_msg(i, 0, 0, &rx_msg);
		EXPECT_EQ (ERR_NOT_FOUND, rc, "read_msg on non existing handle");

		/* calling send_msg with NULL msg should fail for any handle */
		rc = read_msg(i, 0, 0, NULL);
		EXPECT_EQ (ERR_FAULT, rc, "read_msg on NULL msg");
	}

	/* calling read_msg on port handle should fail
	   because read_msg is only applicable to channels */
	sprintf(path, "%s.main.%s", SRV_PATH_BASE,  "datasink");
	rc = port_create(path, 2, MAX_PORT_BUF_SIZE,
	                 IPC_PORT_ALLOW_TA_CONNECT);
	EXPECT_GE_ZERO (rc, "create datasink port");
	port = (handle_t) rc;

	rc = read_msg(port, 0, 0, &rx_msg);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "read_msg on port");
	close(port);

	/* open connection to echo service */
	sprintf(path, "%s.srv.%s", SRV_PATH_BASE,  "echo");
	rc = sync_connect(path, 1000);
	EXPECT_GE_ZERO (rc, "connect to datasink");
	chan = (handle_t) rc;

	/* NULL msg on valid channel */
	rc = read_msg(chan, 0, 0, NULL);
	EXPECT_EQ (ERR_FAULT, rc, "read_msg on NULL msg");

	/* read_msg on invalid msg id */
	rc = read_msg(chan, 0, 0, &rx_msg);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "read_msg on invalid msg id");

	rc = read_msg(chan, 1000, 0, &rx_msg);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "read_msg on invalid msg id");

	/* send a message to echo service */
	memset(tx_buf, 0x55, sizeof(tx_buf));
	tx_iov.base = tx_buf;
	tx_iov.len  = sizeof(tx_buf);
	tx_msg.num_iov = 1;
	tx_msg.iov     = &tx_iov;
	tx_msg.num_handles = 0;
	tx_msg.handles = NULL;

	rc = send_msg(chan, &tx_msg);
	EXPECT_EQ (64, rc, "sending msg to echo");

	/* and wait for response */
	rc = wait(chan, &uevt, 1000);
	EXPECT_EQ (NO_ERROR, rc, "waiting on echo response");
	EXPECT_EQ (chan, uevt.handle, "wait on channel");

	rc = get_msg(chan, &inf);
	EXPECT_EQ (NO_ERROR, rc, "getting echo msg");
	EXPECT_EQ (sizeof(tx_buf), inf.len, "echo message reply length");

	/* now we have valid message with valid id */

	rx_iov[0].len  = sizeof(rx_buf) / 2;
	rx_iov[1].len  = sizeof(rx_buf) / 2;

	/* read message with invalid iovec array */
	rx_msg.iov     = NULL;
	rx_msg.num_iov = 2;
	rc = read_msg(chan, inf.id, 0, &rx_msg);
	EXPECT_EQ (ERR_FAULT, rc, "read with invalid iovec array");

	/* read with invalid iovec entry */
	rx_iov[0].base = NULL;
	rx_iov[1].base = NULL;
	rx_msg.iov     = rx_iov;
	rc = read_msg(chan, inf.id, 0, &rx_msg);
	EXPECT_EQ (ERR_FAULT, rc, "read with invalid iovec");

	rx_iov[0].base = rx_buf;
	rx_iov[1].base = NULL;
	rc = read_msg(chan, inf.id, 0, &rx_msg);
	EXPECT_EQ (ERR_FAULT, rc, "read with invalid iovec");

	rx_iov[0].base = rx_buf;
	rx_iov[1].base = rx_buf + sizeof(rx_buf) / 2;

	/* read with invalid offset with valid iovec array */
	rc = read_msg(chan, inf.id, inf.len + 1, &rx_msg);
	EXPECT_EQ (ERR_INVALID_ARGS, rc, "read with invalid offset");

	/* read with handles */
	rx_msg.num_handles = 1;
	rx_msg.handles = NULL;
	rc = read_msg(chan, inf.id, 0, &rx_msg);
	EXPECT_EQ (ERR_NOT_SUPPORTED, rc, "read with handles");
	rx_msg.num_handles = 0;

	/* cleanup */
	rc = put_msg(chan, inf.id);
	EXPECT_EQ (NO_ERROR, rc, "putting echo msg");

	rc = close(chan);
	EXPECT_EQ (NO_ERROR, rc, "close channel");

	TEST_END
}

static void run_end_to_end_msg_test(void)
{
	int rc;
	handle_t chan;
	uevent_t uevt;
	char path[MAX_PORT_PATH_LEN];
	uint8_t tx_buf[64];
	uint8_t rx_buf[64];
	ipc_msg_info_t inf;
	ipc_msg_t   tx_msg;
	iovec_t     tx_iov;
	ipc_msg_t   rx_msg;
	iovec_t     rx_iov;

	TEST_BEGIN(__func__);

	tx_iov.base = tx_buf;
	tx_iov.len  = sizeof(tx_buf);
	tx_msg.num_iov = 1;
	tx_msg.iov     = &tx_iov;
	tx_msg.num_handles = 0;
	tx_msg.handles = NULL;

	rx_iov.base = rx_buf;
	rx_iov.len  = sizeof(rx_buf);
	rx_msg.num_iov = 1;
	rx_msg.iov     = &rx_iov;
	rx_msg.num_handles = 0;
	rx_msg.handles = NULL;

	memset (tx_buf, 0x55, sizeof(tx_buf));
	memset (rx_buf, 0xaa, sizeof(rx_buf));

	sprintf(path, "%s.srv.%s", SRV_PATH_BASE,  "echo");
	rc = sync_connect(path, 1000);
	EXPECT_GE_ZERO (rc, "connect to echo");

	if (rc >= 0) {
		uint tx_cnt = 0;
		uint rx_cnt = 0;

		chan = (handle_t) rc;

		/* send 10000 messages synchronously, waiting for reply
		   for each one
		 */
		tx_cnt = 10000;
		while (tx_cnt) {
			/* send a message */
			rc = send_msg(chan, &tx_msg);
			EXPECT_EQ (64, rc, "sending msg to echo");

			/* wait for response */
			rc = wait(chan, &uevt, 1000);
			EXPECT_EQ (NO_ERROR, rc, "waiting on echo response");
			EXPECT_EQ (chan, uevt.handle, "wait on channel");

			/* get a reply */
			rc = get_msg(chan, &inf);
			EXPECT_EQ (NO_ERROR, rc, "getting echo msg");

			/* read reply data */
			rc = read_msg(chan, inf.id, 0, &rx_msg);
			EXPECT_EQ (64, rc, "reading echo msg");

			/* discard reply */
			rc = put_msg(chan, inf.id);
			EXPECT_EQ (NO_ERROR, rc, "putting echo msg");

			tx_cnt--;
		}

		/* send/receive 10000 messages asynchronously. */
		rx_cnt = tx_cnt = 10000;
		while (tx_cnt || rx_cnt) {

			/* send messages until all buffers are full */
			while (tx_cnt) {
				rc = send_msg(chan, &tx_msg);
				if (rc == ERR_NOT_ENOUGH_BUFFER)
					break;  /* no more space */
				EXPECT_EQ(64, rc, "sending msg to echo");
				if (rc != 64)
					goto abort_test;
				tx_cnt--;
			}

			/* wait for reply msg or room */
			rc = wait(chan, &uevt, 1000);
			EXPECT_EQ (NO_ERROR, rc, "waiting for reply");
			EXPECT_EQ (chan, uevt.handle, "wait on channel");

			/* drain all messages */
			while (rx_cnt) {
				/* get a reply */
				rc = get_msg(chan, &inf);
				if (rc == ERR_NO_MSG)
					break;  /* no more messages  */

				EXPECT_EQ (NO_ERROR, rc, "getting echo msg");

				/* read reply data */
				rc = read_msg(chan, inf.id, 0, &rx_msg);
				EXPECT_EQ (64, rc, "reading echo msg");

				/* discard reply */
				rc = put_msg(chan, inf.id);
				EXPECT_EQ (NO_ERROR, rc, "putting echo msg");

				rx_cnt--;
			}

			if (!_all_ok)
				break;
		}

abort_test:
		EXPECT_EQ (0, tx_cnt, "tx_cnt");
		EXPECT_EQ (0, rx_cnt, "rx_cnt");

		rc = close(chan);
		EXPECT_EQ (NO_ERROR, rc, "close channel");
	}

	TEST_END
}


/****************************************************************************/

/*
 *
 */
static void run_all_tests (void)
{
	TLOGI ("Run all unittest\n");

	/* reset test state */
	_tests_total  = 0;
	_tests_failed = 0;

	/* positive tests */
	run_port_create_test();
	run_wait_on_port_test();
	run_async_connect_test();
	run_connect_close_test();
	run_accept_test();
	run_send_msg_test();
	run_end_to_end_msg_test();

	run_connect_close_by_peer_test("closer1");
	run_connect_close_by_peer_test("closer2");
	run_connect_close_by_peer_test("closer3");
	run_connect_selfie_test();
	run_connect_access_test();

	/* negative tests */
	run_wait_negative_test();
	run_close_handle_negative_test();
	run_set_cookie_negative_test();
	run_port_create_negative_test();
	run_connect_negative_test();
	run_accept_negative_test();
	run_get_msg_negative_test();
	run_put_msg_negative_test();
	run_send_msg_negative_test();
	run_read_msg_negative_test();

	TLOGI("Conditions checked: %d\n", _tests_total);
	TLOGI("Conditions failed:  %d\n", _tests_failed);
	if (_tests_failed == 0)
		TLOGI("All tests PASSED\n");
	else
		TLOGI("Some tests FAILED\n");
}

/*
 *  Application entry point
 */
int main(void)
{
	int rc;
	char path[MAX_PORT_PATH_LEN];
	uuid_t peer_uuid;

	TLOGI ("Welcome to IPC unittest!!!\n");

	/* create control port and just wait on it */
        sprintf(path, "%s.%s", SRV_PATH_BASE, "ctrl");
	rc = port_create(path,  1, MAX_PORT_BUF_SIZE,
	                 IPC_PORT_ALLOW_NS_CONNECT);
	if (rc < 0) {
		TLOGI("failed (%d) to create ctrl port\n", rc );
		return rc;
	}

	/* and just wait forever for now */
	TLOGI("waiting forever\n");
	for (;;) {
		uevent_t uevt;
		int rc = wait_any(&uevt, -1);
		TLOGI("got event (rc=%d): ev=%x handle=%d\n",
		       rc, uevt.event, uevt.handle);
		if (rc == NO_ERROR) {
			if (uevt.event & IPC_HANDLE_POLL_READY) {
				/* get connection request */
				rc = accept(uevt.handle, &peer_uuid);
				if (rc >= 0) {
					/* then run unittest test */
					run_all_tests();

					/* and close it */
					close(rc);
				}
			}
		}
		if (rc < 0)
			break;
	}

	return rc;
}


