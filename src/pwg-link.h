#pragma once

#include <glib-object.h>

#include "pwg-core.h"
#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_LINK (pwg_link_get_type())

/**
 * PwgLink:
 *
 * Live PipeWire link proxy for observing link state.
 *
 * Link objects bind a discovered link global and expose copied state updates.
 * They do not expose raw PipeWire proxy ownership to language bindings.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgLink, pwg_link, PWG, LINK, GObject)

/**
 * pwg_link_new:
 * @core: a PipeWire core wrapper.
 * @global: a PipeWire link global descriptor.
 *
 * Creates a live link proxy wrapper for link state inspection.
 *
 * Returns: (nullable) (transfer full): a new link wrapper, or %NULL if @global
 *   is not a PipeWire link.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
PwgLink *pwg_link_new(PwgCore *core, PwgGlobal *global);

/**
 * pwg_link_start:
 * @self: a link wrapper.
 * @error: return location for a #GError.
 *
 * Binds the link proxy. If the core is not connected, this method connects it
 * first.
 *
 * Returns: true when the link proxy started or was already running.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
bool pwg_link_start(PwgLink *self, GError **error);

/**
 * pwg_link_sync:
 * @self: a link wrapper.
 * @timeout_msec: timeout in milliseconds, or 0 to wait indefinitely.
 * @error: return location for a #GError.
 *
 * Starts the link proxy if needed, performs a PipeWire core roundtrip, and
 * dispatches link state updates queued before the matching roundtrip
 * completion on this object's main context.
 *
 * Returns: true when the link proxy is synchronized.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
bool pwg_link_sync(PwgLink *self, unsigned int timeout_msec, GError **error);

/**
 * pwg_link_stop:
 * @self: a link wrapper.
 *
 * Stops the link proxy and clears copied state.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
void pwg_link_stop(PwgLink *self);

/**
 * pwg_link_get_core:
 * @self: a link wrapper.
 *
 * Gets the core used by this link wrapper.
 *
 * Returns: (transfer none): the core used by this link wrapper.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
PwgCore *pwg_link_get_core(PwgLink *self);

/**
 * pwg_link_get_global:
 * @self: a link wrapper.
 *
 * Gets the global descriptor used to bind this link.
 *
 * Returns: (transfer none): the global descriptor used to bind this link.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_link_get_global(PwgLink *self);

/**
 * pwg_link_get_running:
 * @self: a link wrapper.
 *
 * Gets whether the link wrapper is running.
 *
 * Returns: whether the link wrapper is running.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
bool pwg_link_get_running(PwgLink *self);

/**
 * pwg_link_get_bound:
 * @self: a link wrapper.
 *
 * Gets whether the link proxy is currently bound.
 *
 * Returns: whether the link proxy is currently bound.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
bool pwg_link_get_bound(PwgLink *self);

/**
 * pwg_link_get_state:
 * @self: a link wrapper.
 *
 * Gets the latest copied PipeWire link state string.
 *
 * Returns: (nullable) (transfer none): the current link state, or %NULL before
 *   the first state update.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
const char *pwg_link_get_state(PwgLink *self);

/**
 * pwg_link_dup_error:
 * @self: a link wrapper.
 *
 * Copies the latest PipeWire link state error message.
 *
 * Returns: (nullable) (transfer full): the latest link error, or %NULL.
 *
 * Since: 0.3.7
 * Stability: Unstable
 */
PWG_API
char *pwg_link_dup_error(PwgLink *self);

G_END_DECLS
