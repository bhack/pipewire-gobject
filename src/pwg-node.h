#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "pwg-core.h"
#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_NODE (pwg_node_get_type())

/**
 * PwgNode:
 *
 * Live read-only PipeWire node proxy for parameter inspection.
 *
 * Node objects bind a discovered node global and expose copied parameter info
 * and enumeration results. They do not expose raw PipeWire proxy ownership to
 * language bindings.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgNode, pwg_node, PWG, NODE, GObject)

/**
 * pwg_node_new:
 * @core: a PipeWire core wrapper.
 * @global: a PipeWire node global descriptor.
 *
 * Creates a live node proxy wrapper for read-only parameter inspection.
 *
 * Returns: (nullable) (transfer full): a new node wrapper, or %NULL if @global
 *   is not a PipeWire node.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgNode *pwg_node_new(PwgCore *core, PwgGlobal *global);

/**
 * pwg_node_start:
 * @self: a node wrapper.
 * @error: return location for a #GError.
 *
 * Binds the node proxy. If the core is not connected, this method connects it
 * first.
 *
 * Returns: %TRUE when the node proxy started or was already running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_node_start(PwgNode *self, GError **error);

/**
 * pwg_node_stop:
 * @self: a node wrapper.
 *
 * Stops the node proxy and clears copied parameter state.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_node_stop(PwgNode *self);

/**
 * pwg_node_get_core:
 * @self: a node wrapper.
 *
 * Gets the core used by this node wrapper.
 *
 * Returns: (transfer none): the core used by this node wrapper.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgCore *pwg_node_get_core(PwgNode *self);

/**
 * pwg_node_get_global:
 * @self: a node wrapper.
 *
 * Gets the global descriptor used to bind this node.
 *
 * Returns: (transfer none): the global descriptor used to bind this node.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_node_get_global(PwgNode *self);

/**
 * pwg_node_get_running:
 * @self: a node wrapper.
 *
 * Gets whether the node wrapper is running.
 *
 * Returns: whether the node wrapper is running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_node_get_running(PwgNode *self);

/**
 * pwg_node_get_bound:
 * @self: a node wrapper.
 *
 * Gets whether the node proxy is currently bound.
 *
 * Returns: whether the node proxy is currently bound.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_node_get_bound(PwgNode *self);

/**
 * pwg_node_get_param_infos:
 * @self: a node wrapper.
 *
 * Gets the live model of [class@Pwg.ParamInfo] descriptors advertised by node
 * info events.
 *
 * Returns: (transfer none): a #GListModel of #PwgParamInfo objects.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GListModel *pwg_node_get_param_infos(PwgNode *self);

/**
 * pwg_node_get_params:
 * @self: a node wrapper.
 *
 * Gets the live model of [class@Pwg.Param] values from the most recent
 * enumeration request.
 *
 * Returns: (transfer none): a #GListModel of #PwgParam objects.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GListModel *pwg_node_get_params(PwgNode *self);

/**
 * pwg_node_enum_params:
 * @self: a node wrapper.
 * @id: the SPA parameter id to enumerate.
 * @start: the starting enumeration index.
 * @num: maximum number of params to request, or 0 for all.
 * @error: return location for a #GError.
 *
 * Requests read-only enumeration of node parameters for @id. Results are
 * appended to [method@Pwg.Node.get_params] and emitted through the
 * [signal@Pwg.Node::param] signal.
 *
 * Returns: the PipeWire request sequence number, or -1 on failure.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gint pwg_node_enum_params(PwgNode *self,
                          guint id,
                          guint start,
                          guint num,
                          GError **error);

/**
 * pwg_node_enum_all_params:
 * @self: a node wrapper.
 * @error: return location for a #GError.
 *
 * Requests read-only enumeration of all available node parameter ids.
 *
 * Returns: the PipeWire request sequence number, or -1 on failure.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gint pwg_node_enum_all_params(PwgNode *self, GError **error);

G_END_DECLS
