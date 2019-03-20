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

#include <event2/visibility.h>

struct event_base;
struct event_watcher;

/**
  TODO(mergeconflict): What parameters would be interesting to pass from the event loop to these
  callbacks? For comparison, libev passes a pointer to the event loop and the watcher, which might
  be handy. It also passes "revents" which is a bitfield representing the type of event (e.g. read,
  write), which seems less useful, since these aren't events at all. In contrast, libuv just passes 
  a pointer to the watcher.

    https://metacpan.org/pod/distribution/EV/libev/ev.pod#ev_prepare-and-ev_check-customise-your-event-loop!
    http://docs.libuv.org/en/v1.x/prepare.html, http://docs.libuv.org/en/v1.x/check.html

  Note that we could get away with just passing the watcher, not the base, since event_watcher
  holds a pointer to its base (see definition in event-internal.h). We'd just need to provide a
  function like:

    struct event_base *event_watcher_base(struct event_watcher *)

  which is either a convenience or an inconvenience depending how you look at it...
 */
typedef void (*event_watcher_cb)(struct event_base *, struct event_watcher *);

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
