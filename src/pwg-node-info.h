#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_NODE_INFO (pwg_node_info_get_type())

/**
 * PwgNodeInfo:
 *
 * Convenience wrapper for node-specific properties from a [class@Pwg.Global].
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgNodeInfo, pwg_node_info, PWG, NODE_INFO, GObject)

/**
 * pwg_node_info_new_from_global:
 * @global: a PipeWire global descriptor.
 *
 * Creates an immutable node info wrapper for a node global.
 *
 * Returns: (nullable) (transfer full): a new node info object, or %NULL if
 *   @global is not a PipeWire node.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgNodeInfo *pwg_node_info_new_from_global(PwgGlobal *global);

/**
 * pwg_node_info_get_global:
 * @self: a node info wrapper.
 *
 * Gets the wrapped global descriptor.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_node_info_get_global(PwgNodeInfo *self);

/**
 * pwg_node_info_get_id:
 * @self: a node info wrapper.
 *
 * Gets the PipeWire global id for the wrapped node.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
unsigned int pwg_node_info_get_id(PwgNodeInfo *self);

/**
 * pwg_node_info_dup_name:
 * @self: a node info wrapper.
 *
 * Copies the PipeWire `node.name` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `node.name`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_node_info_dup_name(PwgNodeInfo *self);

/**
 * pwg_node_info_dup_description:
 * @self: a node info wrapper.
 *
 * Copies the PipeWire `node.description` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `node.description`, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_node_info_dup_description(PwgNodeInfo *self);

/**
 * pwg_node_info_dup_media_class:
 * @self: a node info wrapper.
 *
 * Copies the PipeWire `media.class` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `media.class`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_node_info_dup_media_class(PwgNodeInfo *self);

/**
 * pwg_node_info_dup_object_serial:
 * @self: a node info wrapper.
 *
 * Copies the PipeWire `object.serial` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_node_info_dup_object_serial(PwgNodeInfo *self);

G_END_DECLS
