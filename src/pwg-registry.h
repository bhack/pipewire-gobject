#pragma once

#include <gio/gio.h>

#include "pwg-core.h"
#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_REGISTRY (pwg_registry_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgRegistry, pwg_registry, PWG, REGISTRY, GObject)

/**
 * pwg_registry_new:
 * @core: a PipeWire core wrapper.
 *
 * Creates a registry wrapper for discovering PipeWire globals.
 *
 * Returns: (transfer full): a new registry object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgRegistry *pwg_registry_new(PwgCore *core);

/**
 * pwg_registry_start:
 * @self: a registry wrapper.
 * @error: return location for a #GError.
 *
 * Starts registry discovery. If the core is not connected, this method
 * connects it first.
 *
 * Returns: %TRUE when discovery started or was already running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_registry_start(PwgRegistry *self, GError **error);

/**
 * pwg_registry_stop:
 * @self: a registry wrapper.
 *
 * Stops registry discovery and clears the current global list.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_registry_stop(PwgRegistry *self);

/**
 * pwg_registry_get_core:
 * @self: a registry wrapper.
 *
 * Returns: (transfer none): the core used by this registry.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgCore *pwg_registry_get_core(PwgRegistry *self);

/**
 * pwg_registry_get_running:
 * @self: a registry wrapper.
 *
 * Returns: whether registry discovery is running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_registry_get_running(PwgRegistry *self);

/**
 * pwg_registry_get_globals:
 * @self: a registry wrapper.
 *
 * Returns: (transfer none): a #GListModel of #PwgGlobal objects.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GListModel *pwg_registry_get_globals(PwgRegistry *self);

/**
 * pwg_registry_lookup_global:
 * @self: a registry wrapper.
 * @id: a PipeWire global id.
 *
 * Returns: (nullable) (transfer full): the matching global, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_registry_lookup_global(PwgRegistry *self, guint id);

G_END_DECLS
