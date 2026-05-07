#pragma once

#include <glib-object.h>

#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_PARAM_INFO (pwg_param_info_get_type())

/**
 * PwgParamInfo:
 *
 * Immutable descriptor for a PipeWire node parameter advertised by node info.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgParamInfo, pwg_param_info, PWG, PARAM_INFO, GObject)

/**
 * pwg_param_info_get_id:
 * @self: a parameter info descriptor.
 *
 * Gets the PipeWire SPA parameter id.
 *
 * Returns: the SPA parameter id.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
unsigned int pwg_param_info_get_id(PwgParamInfo *self);

/**
 * pwg_param_info_dup_name:
 * @self: a parameter info descriptor.
 *
 * Copies the short SPA parameter name when known.
 *
 * Returns: (nullable) (transfer full): the short parameter name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_param_info_dup_name(PwgParamInfo *self);

/**
 * pwg_param_info_get_flags:
 * @self: a parameter info descriptor.
 *
 * Gets the raw `spa_param_info` flags.
 *
 * Returns: the raw `spa_param_info` flags.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
unsigned int pwg_param_info_get_flags(PwgParamInfo *self);

/**
 * pwg_param_info_get_readable:
 * @self: a parameter info descriptor.
 *
 * Gets whether the parameter is advertised as readable.
 *
 * Returns: whether the parameter is advertised as readable.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_param_info_get_readable(PwgParamInfo *self);

/**
 * pwg_param_info_get_writable:
 * @self: a parameter info descriptor.
 *
 * Gets whether the parameter is advertised as writable.
 *
 * Returns: whether the parameter is advertised as writable.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_param_info_get_writable(PwgParamInfo *self);

/**
 * pwg_param_info_get_serial:
 * @self: a parameter info descriptor.
 *
 * Gets whether the parameter info has the SPA serial flag.
 *
 * Returns: whether the parameter info has the serial flag.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_param_info_get_serial(PwgParamInfo *self);

G_END_DECLS
