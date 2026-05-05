#pragma once

#include <gio/gio.h>

#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_GLOBAL (pwg_global_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgGlobal, pwg_global, PWG, GLOBAL, GObject)

/**
 * pwg_global_get_id:
 * @self: a PipeWire global descriptor.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_global_get_id(PwgGlobal *self);

/**
 * pwg_global_get_permissions:
 * @self: a PipeWire global descriptor.
 *
 * Returns: the PipeWire permissions bitmask visible to this client.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_global_get_permissions(PwgGlobal *self);

/**
 * pwg_global_get_interface_type:
 * @self: a PipeWire global descriptor.
 *
 * Returns: (transfer none): the PipeWire interface type name.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_global_get_interface_type(PwgGlobal *self);

/**
 * pwg_global_get_version:
 * @self: a PipeWire global descriptor.
 *
 * Returns: the PipeWire interface version.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_global_get_version(PwgGlobal *self);

/**
 * pwg_global_get_properties:
 * @self: a PipeWire global descriptor.
 *
 * Returns: (transfer full): the global properties as an `a{ss}` variant.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GVariant *pwg_global_get_properties(PwgGlobal *self);

/**
 * pwg_global_dup_property:
 * @self: a PipeWire global descriptor.
 * @key: a PipeWire property key.
 *
 * Returns: (nullable) (transfer full): the property value, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_global_dup_property(PwgGlobal *self, const char *key);

G_END_DECLS
