/*
 * Copyright (c) 2000-2007 Niels Provos <provos@citi.umich.edu>
 * Copyright (c) 2007-2012 Niels Provos and Nick Mathewson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <event2/event.h>
#include <event2/event_watcher.h>
#include <assert.h>

static int iteration = 0;
static int prepare_callback_1_count = 0;
static int prepare_callback_2_count = 0;
static int check_callback_1_count = 0;
static int check_callback_2_count = 0;
static struct timeval start_time = { 0, 0 };
static struct timeval end_time = { 0, 0 };

#define log_callback \
	printf("%s:\n", __func__); \
	printf("  now: %ld.%06ld\n", info->now.tv_sec, info->now.tv_usec); \
	printf("  timeout: %ld.%06ld\n", info->timeout->tv_sec, info->timeout->tv_usec);

void
prepare_callback_1(struct event_watcher *watcher, const struct event_watcher_cb_info *info)
{
	log_callback;
	++prepare_callback_1_count;

	/* prepare_callback_1 should always fire before prepare_callback_2, and before both check callbacks */
	assert(prepare_callback_1_count > prepare_callback_2_count);
	assert(prepare_callback_1_count > check_callback_1_count);
	assert(prepare_callback_1_count > check_callback_2_count);

	/* if we've just scheduled the timeout event at the beginning of the iteration, save the current time and assert that the timeout is what we set */
	if (start_time.tv_sec == 0) {
		start_time = info->now;
		assert(info->timeout->tv_sec == 1);
		assert(info->timeout->tv_usec == 0);
	}
}

void
prepare_callback_2(struct event_watcher *watcher, const struct event_watcher_cb_info *info)
{
	log_callback;
	++prepare_callback_2_count;

	/* prepare_callback_2 should only fire on the first iteration, and should fire before both check callbacks */
	assert(iteration == 0);
	assert(prepare_callback_2_count > check_callback_1_count);
	assert(prepare_callback_2_count > check_callback_2_count);
}

void
check_callback_1(struct event_watcher *watcher, const struct event_watcher_cb_info *info)
{
	log_callback;
	++check_callback_1_count;

	/* check_callback_1 should always fire before check_callback_2 */
	assert(check_callback_1_count > check_callback_2_count);

	/* save the end time, in case the timeout fires this time through the event loop */
	end_time = info->now;
}

void
check_callback_2(struct event_watcher *watcher, const struct event_watcher_cb_info *info)
{
	log_callback;
	++check_callback_2_count;

	/* check_callback_2 should only fire on the first iteration */
	assert(iteration == 0);
}

void
timeout_callback(evutil_socket_t fd, short events, void *arg)
{
	printf("timeout_callback\n");

	/* the duration between the start and end times should be at least 1 second */
	assert(end_time.tv_sec >= start_time.tv_sec + 1);
}

int
main(int argc, char **argv)
{
	struct event_base *base = event_base_new();

	/* install prepare and check watchers */
	event_watcher_prepare_new(base, &prepare_callback_1);
	event_watcher_check_new(base, &check_callback_1);
	struct event_watcher *prepare_callback_2_watcher = event_watcher_prepare_new(base, &prepare_callback_2);
	struct event_watcher *check_callback_2_watcher = event_watcher_check_new(base, &check_callback_2);

	/* schedule an 1 second timeout event, and run the event loop until the timeout fires */
	printf("iteration 1\n-----------\n");
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	event_base_once(base, -1, EV_TIMEOUT, &timeout_callback, 0, &tv);
	event_base_dispatch(base);

	/* second iteration: free two of the watchers, schedule a timeout and run the event loop again */
	printf("iteration 2\n-----------\n");
	iteration = 1;
	start_time.tv_sec = 0;
	event_watcher_free(prepare_callback_2_watcher);
	event_watcher_free(check_callback_2_watcher);
	event_base_once(base, -1, EV_TIMEOUT, &timeout_callback, 0, &tv);
	event_base_dispatch(base);

	/* this will free the watchers that weren't already freed in timeout_callback */
	event_base_free(base);
	return (0);
}
