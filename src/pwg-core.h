#pragma once

#include <glib-object.h>

#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_CORE (pwg_core_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgCore, pwg_core, PWG, CORE, GObject)

/**
 * pwg_core_new:
 *
 * Creates a wrapper for a PipeWire core connection.
 *
 * Returns: (transfer full): a new PipeWire core wrapper.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgCore *pwg_core_new(void);

/**
 * pwg_core_connect:
 * @self: a core wrapper.
 * @error: return location for a #GError.
 *
 * Returns: %TRUE when the core connected or was already connected.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_core_connect(PwgCore *self, GError **error);

/**
 * pwg_core_disconnect:
 * @self: a core wrapper.
 *
 * Disconnects from PipeWire if currently connected.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_core_disconnect(PwgCore *self);

/**
 * pwg_core_get_connected:
 * @self: a core wrapper.
 *
 * Returns: whether this wrapper currently has an active PipeWire connection.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_core_get_connected(PwgCore *self);

/**
 * pwg_core_get_library_version:
 * @self: a core wrapper.
 *
 * Returns: (transfer none): the PipeWire library version string.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_core_get_library_version(PwgCore *self);

G_END_DECLS
