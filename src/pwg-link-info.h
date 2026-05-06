#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_LINK_INFO (pwg_link_info_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgLinkInfo, pwg_link_info, PWG, LINK_INFO, GObject)

/**
 * pwg_link_info_new_from_global:
 * @global: a PipeWire global descriptor.
 *
 * Creates an immutable link info wrapper for a link global.
 *
 * Returns: (nullable) (transfer full): a new link info object, or %NULL if
 *   @global is not a PipeWire link.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgLinkInfo *pwg_link_info_new_from_global(PwgGlobal *global);

/**
 * pwg_link_info_get_global:
 * @self: a link info wrapper.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_link_info_get_global(PwgLinkInfo *self);

/**
 * pwg_link_info_get_id:
 * @self: a link info wrapper.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
guint pwg_link_info_get_id(PwgLinkInfo *self);

/**
 * pwg_link_info_dup_link_id:
 * @self: a link info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `link.id`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_link_info_dup_link_id(PwgLinkInfo *self);

/**
 * pwg_link_info_dup_input_node_id:
 * @self: a link info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `link.input.node`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_link_info_dup_input_node_id(PwgLinkInfo *self);

/**
 * pwg_link_info_dup_input_port_id:
 * @self: a link info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `link.input.port`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_link_info_dup_input_port_id(PwgLinkInfo *self);

/**
 * pwg_link_info_dup_output_node_id:
 * @self: a link info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `link.output.node`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_link_info_dup_output_node_id(PwgLinkInfo *self);

/**
 * pwg_link_info_dup_output_port_id:
 * @self: a link info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `link.output.port`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_link_info_dup_output_port_id(PwgLinkInfo *self);

/**
 * pwg_link_info_dup_object_serial:
 * @self: a link info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_link_info_dup_object_serial(PwgLinkInfo *self);

G_END_DECLS
