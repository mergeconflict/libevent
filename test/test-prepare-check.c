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

static struct event_watcher *prepare_callback_1_watcher;
static struct event_watcher *prepare_callback_2_watcher;
static struct event_watcher *check_callback_1_watcher;
static struct event_watcher *check_callback_2_watcher;

static int prepare_callback_1_count = 0;
static int prepare_callback_2_count = 0;
static int check_callback_1_count = 0;
static int check_callback_2_count = 0;
static int timeout_callback_count = 0;

static struct event_base *base;

void
prepare_callback_1(struct event_base *base, struct event_watcher *watcher)
{
  printf("prepare_callback_1\n");
  switch (prepare_callback_1_count) {
  case 0:
    /* first iteration: no callbacks should have fired */
    assert(prepare_callback_2_count == 0);
    assert(check_callback_1_count == 0);
    assert(check_callback_2_count == 0);
    assert(timeout_callback_count == 0);
    break;
  case 1:
    /* second iteration: all callbacks should have fired once */
    assert(prepare_callback_2_count == 1);
    assert(check_callback_1_count == 1);
    assert(check_callback_2_count == 1);
    assert(timeout_callback_count == 1);
    break;
  default:
    /* there should be no third iteration */
    assert(0);
  }
  ++prepare_callback_1_count;
}

void
prepare_callback_2(struct event_base *base, struct event_watcher *watcher)
{
  printf("prepare_callback_2\n");
  /* only the first prepare callback should have fired previously, and there should be no second iteration */
  assert(prepare_callback_1_count == 1);
  assert(prepare_callback_2_count == 0);
  assert(check_callback_1_count == 0);
  assert(check_callback_2_count == 0);
  assert(timeout_callback_count == 0);
  ++prepare_callback_2_count;
}

void
check_callback_1(struct event_base *base, struct event_watcher *watcher)
{
  printf("check_callback_1\n");
  switch (check_callback_1_count) {
  case 0:
    /* first iteration: prepare callbacks should fire before check callbacks */
    assert(prepare_callback_1_count == 1);
    assert(prepare_callback_2_count == 1);
    assert(check_callback_2_count == 0);
    assert(timeout_callback_count == 0);
    break;
  case 1:
    /* second iteration: first prepare callback should have fired twice */
    assert(prepare_callback_1_count == 2);
    assert(prepare_callback_2_count == 1);
    assert(check_callback_2_count == 1);
    assert(timeout_callback_count == 1);
    break;
  default:
    /* there should be no third iteration */
    assert(0);
  }
  ++check_callback_1_count;
}

void
check_callback_2(struct event_base *base, struct event_watcher *watcher)
{
  printf("check_callback_2\n");
  /* first check callback should fire before second check callback, and there should be no second iteration */
  assert(prepare_callback_1_count == 1);
  assert(prepare_callback_2_count == 1);
  assert(check_callback_1_count == 1);
  assert(check_callback_2_count == 0);
  assert(timeout_callback_count == 0);
  ++check_callback_2_count;
}

void
timeout_callback(evutil_socket_t fd, short events, void *arg)
{
  printf("timeout_callback\n");
  /* the prepare and check callbacks should fire before the timeout event, which should only fire once */
  assert(prepare_callback_1_count == 1);
  assert(prepare_callback_2_count == 1);
  assert(check_callback_1_count == 1);
  assert(check_callback_2_count == 1);
  assert(timeout_callback_count == 0);

  /* free the second prepare and check watchers to verify they aren't invoked a second time */
  event_watcher_free(prepare_callback_2_watcher);
  event_watcher_free(check_callback_2_watcher);
  ++timeout_callback_count;
}

int
main(int argc, char **argv)
{
	base = event_base_new();

  /* install prepare and check watchers, and schedule an immediate timeout event */
  prepare_callback_1_watcher = event_watcher_prepare_new(base, &prepare_callback_1);
  prepare_callback_2_watcher = event_watcher_prepare_new(base, &prepare_callback_2);
  check_callback_1_watcher = event_watcher_prepare_new(base, &check_callback_1);
  check_callback_2_watcher = event_watcher_prepare_new(base, &check_callback_2);
  event_base_once(base, -1, EV_TIMEOUT, &timeout_callback, 0, 0);

  /* dispatch and verify that callbacks fire in the correct order */
  event_base_dispatch(base);
  assert(prepare_callback_1_count == 2);
  assert(prepare_callback_2_count == 1);
  assert(check_callback_1_count == 2);
  assert(check_callback_2_count == 1);
  assert(timeout_callback_count == 1);

  /* this will free the watchers that weren't already freed in timeout_callback */
  event_base_free(base);
	return (0);
}
