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

#include "event2/event_watcher.h"
#include "event-internal.h"

static inline struct event_watcher *
event_watcher_new(struct event_base *base, event_watcher_cb callback, unsigned type)
{
	struct event_watcher *watcher = malloc(sizeof(struct event_watcher));
	watcher->base = base;
	watcher->type = type;
	watcher->callback = callback;
	TAILQ_INSERT_TAIL(&base->event_watchers[type], watcher, next);
	return (watcher);
}

struct event_watcher *
event_watcher_prepare_new(struct event_base *base, event_watcher_cb callback)
{
	return (event_watcher_new(base, callback, EVENT_WATCHER_PREPARE_TYPE));
}

struct event_watcher *
event_watcher_check_new(struct event_base *base, event_watcher_cb callback)
{
	return (event_watcher_new(base, callback, EVENT_WATCHER_CHECK_TYPE));
}

void
event_watcher_free(struct event_watcher *watcher)
{
	TAILQ_REMOVE(&watcher->base->event_watchers[watcher->type], watcher, next);
	free(watcher);
}
