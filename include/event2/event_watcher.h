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
#ifndef EVENT2_EVENT_WATCHER_H_INCLUDED_
#define EVENT2_EVENT_WATCHER_H_INCLUDED_

/** @file event2/event_watcher.h

	Functions for registering and deregistering event watchers.

 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EVENT__HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>

#include <event2/visibility.h>

struct event_base;
struct event_watcher;

/**
	Contextual information passed from event_base_loop to the watcher callbacks. We define this as a
	struct rather than individual parameters to the callback function for the sake of extensibility.
 */
struct event_watcher_cb_info {
	/**
		The current time (may be cached). This value is scoped to this callback; if you need to retain it
		outside the scope of the callback (e.g. to measure the duration between callbacks), make a copy.
	 */
	struct timeval now;

	/**
		The timeout duration passed to the underlying implementation's `dispatch`. This will be NULL if
		there are no pending EV_TIMEOUT events. It may be a useful performance statistic to compare this
		value -- essentially the expected polling duration -- against the actual polling duration
		measured by comparing the "now" times reported in prepare and check.
	 */
	const struct timeval *timeout;
};

/**
	Watcher callback, invoked by event_base_loop.
	@param watcher the event watcher that invoked this callback.
	@param info contextual information passed from event_base_loop.
 */
typedef void (*event_watcher_cb)(struct event_watcher *, const struct event_watcher_cb_info *);

/**
	Register a new "prepare" watcher, to be called in the event loop prior to polling for events.
	Watchers will be called in the order they were registered.
	@param base the event_base to operate on.
	@param callback the callback function to invoke.
	@return a pointer to the newly allocated event watcher.
 */
EVENT2_EXPORT_SYMBOL
struct event_watcher *event_watcher_prepare_new(struct event_base *base, event_watcher_cb callback);

/**
	Register a new "check" watcher, to be called in the event loop after polling for events and before
	handling them. Watchers will be called in the order they were registered.
	@param base the event_base to operate on.
	@param callback the callback function to invoke.
	@return a pointer to the newly allocated event watcher.
 */
EVENT2_EXPORT_SYMBOL
struct event_watcher *event_watcher_check_new(struct event_base *base, event_watcher_cb callback);

/**
	Get the event_base that a given event_watcher is registered with.
	@param watcher the watcher to get the event_base for.
	@return the event_base for the given watcher.
 */
EVENT2_EXPORT_SYMBOL
struct event_base *event_watcher_base(struct event_watcher *watcher);

/**
	Deregister and deallocate a watcher. Any watchers not freed using event_watcher_free will
	eventually be deallocated in event_base_free (calling event_watcher_free on a watcher after
	event_base_free has been called on its corresponding event_base is an error).
	@param watcher the watcher to deregister and deallocate.
 */
EVENT2_EXPORT_SYMBOL
void event_watcher_free(struct event_watcher *watcher);

#ifdef __cplusplus
}
#endif

#endif /* EVENT2_EVENT_WATCHER_H_INCLUDED_ */
