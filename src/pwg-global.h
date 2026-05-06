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

/**
 * pwg_global_dup_name:
 * @self: a PipeWire global descriptor.
 *
 * Returns: (nullable) (transfer full): a common display or stable object name,
 *   or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_global_dup_name(PwgGlobal *self);

/**
 * pwg_global_dup_description:
 * @self: a PipeWire global descriptor.
 *
 * Returns: (nullable) (transfer full): a human-readable description, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_global_dup_description(PwgGlobal *self);

/**
 * pwg_global_dup_media_class:
 * @self: a PipeWire global descriptor.
 *
 * Returns: (nullable) (transfer full): the PipeWire media class, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_global_dup_media_class(PwgGlobal *self);

/**
 * pwg_global_dup_object_serial:
 * @self: a PipeWire global descriptor.
 *
 * Returns: (nullable) (transfer full): the PipeWire object serial, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_global_dup_object_serial(PwgGlobal *self);

/**
 * pwg_global_is_interface:
 * @self: a PipeWire global descriptor.
 * @interface_type: a PipeWire interface type name.
 *
 * Returns: whether this global has @interface_type.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_global_is_interface(PwgGlobal *self, const char *interface_type);

/**
 * pwg_global_is_node:
 * @self: a PipeWire global descriptor.
 *
 * Returns: whether this global is a PipeWire node.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_global_is_node(PwgGlobal *self);

/**
 * pwg_global_is_port:
 * @self: a PipeWire global descriptor.
 *
 * Returns: whether this global is a PipeWire port.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
gboolean pwg_global_is_port(PwgGlobal *self);

/**
 * pwg_global_is_link:
 * @self: a PipeWire global descriptor.
 *
 * Returns: whether this global is a PipeWire link.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
gboolean pwg_global_is_link(PwgGlobal *self);

/**
 * pwg_global_is_client:
 * @self: a PipeWire global descriptor.
 *
 * Returns: whether this global is a PipeWire client.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
gboolean pwg_global_is_client(PwgGlobal *self);

/**
 * pwg_global_is_device:
 * @self: a PipeWire global descriptor.
 *
 * Returns: whether this global is a PipeWire device.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
gboolean pwg_global_is_device(PwgGlobal *self);

/**
 * pwg_global_is_metadata:
 * @self: a PipeWire global descriptor.
 *
 * Returns: whether this global is a PipeWire metadata object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_global_is_metadata(PwgGlobal *self);

G_END_DECLS
