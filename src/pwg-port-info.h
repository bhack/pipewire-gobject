#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_PORT_INFO (pwg_port_info_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgPortInfo, pwg_port_info, PWG, PORT_INFO, GObject)

/**
 * pwg_port_info_new_from_global:
 * @global: a PipeWire global descriptor.
 *
 * Creates an immutable port info wrapper for a port global.
 *
 * Returns: (nullable) (transfer full): a new port info object, or %NULL if
 *   @global is not a PipeWire port.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgPortInfo *pwg_port_info_new_from_global(PwgGlobal *global);

/**
 * pwg_port_info_get_global:
 * @self: a port info wrapper.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_port_info_get_global(PwgPortInfo *self);

/**
 * pwg_port_info_get_id:
 * @self: a port info wrapper.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
guint pwg_port_info_get_id(PwgPortInfo *self);

/**
 * pwg_port_info_dup_port_id:
 * @self: a port info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `port.id`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_port_id(PwgPortInfo *self);

/**
 * pwg_port_info_dup_node_id:
 * @self: a port info wrapper.
 *
 * Returns: (nullable) (transfer full): the owning PipeWire `node.id`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_node_id(PwgPortInfo *self);

/**
 * pwg_port_info_dup_name:
 * @self: a port info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `port.name`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_name(PwgPortInfo *self);

/**
 * pwg_port_info_dup_direction:
 * @self: a port info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `port.direction`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_direction(PwgPortInfo *self);

/**
 * pwg_port_info_dup_alias:
 * @self: a port info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `port.alias`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_alias(PwgPortInfo *self);

/**
 * pwg_port_info_dup_object_serial:
 * @self: a port info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_object_serial(PwgPortInfo *self);

G_END_DECLS
