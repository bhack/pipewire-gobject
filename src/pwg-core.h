#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-impl-module.h"

G_BEGIN_DECLS

#define PWG_TYPE_CORE (pwg_core_get_type())

/**
 * PwgCore:
 *
 * Wrapper for a PipeWire core connection.
 *
 * Core objects own the PipeWire loop, context, and core handles used by
 * registry and metadata helper objects.
 *
 * Since: 0.1
 * Stability: Unstable
 */
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
 * Connects this wrapper to PipeWire.
 *
 * Returns: %TRUE when the core connected or was already connected.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_core_connect(PwgCore *self, GError **error);

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
 * Gets whether this wrapper currently has an active PipeWire connection.
 *
 * Returns: whether this wrapper currently has an active PipeWire connection.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_core_get_connected(PwgCore *self);

/**
 * pwg_core_get_library_version:
 * @self: a core wrapper.
 *
 * Gets the linked PipeWire library version string.
 *
 * Returns: (transfer none): the PipeWire library version string.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_core_get_library_version(PwgCore *self);

/**
 * pwg_core_load_module:
 * @self: a core wrapper.
 * @name: the PipeWire implementation module name.
 * @arguments: (nullable): PipeWire module arguments.
 * @error: return location for a #GError.
 *
 * Loads a PipeWire implementation module into this core context.
 *
 * This is for app-owned module lifecycles such as loading a loopback or
 * filter-chain helper. It does not replace WirePlumber or implement
 * session-management policy.
 *
 * Returns: (nullable) (transfer full): a loaded module handle, or %NULL on
 *   failure.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgImplModule *pwg_core_load_module(PwgCore *self,
                                    const char *name,
                                    const char *arguments,
                                    GError **error);

G_END_DECLS
