#pragma once

#include <gio/gio.h>

#include "pwg-core.h"
#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_REGISTRY (pwg_registry_get_type())

/**
 * PwgRegistry:
 *
 * PipeWire registry discovery helper.
 *
 * Registry objects keep a [iface@Gio.ListModel] of copied [class@Pwg.Global]
 * descriptors and emit signals when globals appear or disappear.
 *
 * Since: 0.1
 * Stability: Unstable
 */
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
 * Gets the core used by this registry.
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
 * Gets whether registry discovery is running.
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
 * Gets the live registry global model.
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
 * Looks up a global by PipeWire global id.
 *
 * Returns: (nullable) (transfer full): the matching global, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_registry_lookup_global(PwgRegistry *self, guint id);

/**
 * pwg_registry_lookup_global_by_property:
 * @self: a registry wrapper.
 * @key: a PipeWire property key.
 * @value: a PipeWire property value.
 *
 * Looks up the first global with a matching property.
 *
 * Returns: (nullable) (transfer full): the first matching global, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_registry_lookup_global_by_property(PwgRegistry *self,
                                                  const char *key,
                                                  const char *value);

/**
 * pwg_registry_lookup_global_by_object_serial:
 * @self: a registry wrapper.
 * @object_serial: a PipeWire object serial string.
 *
 * Looks up the first global with a matching PipeWire object serial.
 *
 * Returns: (nullable) (transfer full): the first matching global, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_registry_lookup_global_by_object_serial(PwgRegistry *self,
                                                       const char *object_serial);

/**
 * pwg_registry_dup_globals_by_property:
 * @self: a registry wrapper.
 * @key: a PipeWire property key.
 * @value: a PipeWire property value.
 *
 * Copies a snapshot list of globals with a matching property.
 *
 * Returns: (transfer full): a #GListModel of #PwgGlobal objects.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GListModel *pwg_registry_dup_globals_by_property(PwgRegistry *self,
                                                 const char *key,
                                                 const char *value);

/**
 * pwg_registry_dup_globals_by_interface:
 * @self: a registry wrapper.
 * @interface_type: a PipeWire interface type name.
 *
 * Copies a snapshot list of globals with a matching interface type.
 *
 * Returns: (transfer full): a #GListModel of #PwgGlobal objects.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GListModel *pwg_registry_dup_globals_by_interface(PwgRegistry *self,
                                                  const char *interface_type);

/**
 * pwg_registry_dup_globals_by_media_class:
 * @self: a registry wrapper.
 * @media_class: a PipeWire media class.
 *
 * Copies a snapshot list of globals with a matching media class.
 *
 * Returns: (transfer full): a #GListModel of #PwgGlobal objects.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GListModel *pwg_registry_dup_globals_by_media_class(PwgRegistry *self,
                                                    const char *media_class);

G_END_DECLS
