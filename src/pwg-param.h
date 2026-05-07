#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "pwg-audio-format.h"
#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_PARAM (pwg_param_get_type())

/**
 * PwgParam:
 *
 * Immutable copied PipeWire parameter returned by node parameter enumeration or
 * built by library constructors.
 *
 * Parameter objects own copied POD bytes and do not expose PipeWire or SPA
 * memory ownership to language bindings.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgParam, pwg_param, PWG, PARAM, GObject)

/**
 * pwg_param_new_props_volume:
 * @volume: linear volume, where 0.0 is silence and 1.0 is unity gain.
 *
 * Builds a copied SPA `Props` parameter containing a `volume` property.
 *
 * Returns: (nullable) (transfer full): a copied parameter, or %NULL on invalid
 *   input.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgParam *pwg_param_new_props_volume(gdouble volume);

/**
 * pwg_param_new_props_mute:
 * @mute: whether the target should be muted.
 *
 * Builds a copied SPA `Props` parameter containing a `mute` property.
 *
 * Returns: (nullable) (transfer full): a copied parameter, or %NULL if it could
 *   not be built.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgParam *pwg_param_new_props_mute(gboolean mute);

/**
 * pwg_param_new_props_controls:
 * @controls: (transfer none): `a{sd}` mapping of control names to values.
 *
 * Builds a copied SPA `Props` parameter containing named float controls in the
 * `params` property. This is useful for PipeWire modules that expose controls
 * by name, such as filter-chain biquad coefficients.
 *
 * Returns: (nullable) (transfer full): a copied parameter, or %NULL on invalid
 *   input.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgParam *pwg_param_new_props_controls(GVariant *controls);

/**
 * pwg_param_get_seq:
 * @self: a parameter object.
 *
 * Gets the sequence number of the enumeration request that produced this
 * parameter.
 *
 * Returns: the enumeration sequence number.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gint pwg_param_get_seq(PwgParam *self);

/**
 * pwg_param_get_id:
 * @self: a parameter object.
 *
 * Gets the SPA parameter id.
 *
 * Returns: the SPA parameter id.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_param_get_id(PwgParam *self);

/**
 * pwg_param_dup_name:
 * @self: a parameter object.
 *
 * Copies the short SPA parameter name when known.
 *
 * Returns: (nullable) (transfer full): the short parameter name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_param_dup_name(PwgParam *self);

/**
 * pwg_param_get_index:
 * @self: a parameter object.
 *
 * Gets the enumeration index for this parameter.
 *
 * Returns: the enumeration index.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_param_get_index(PwgParam *self);

/**
 * pwg_param_get_next:
 * @self: a parameter object.
 *
 * Gets PipeWire's next enumeration index for this parameter.
 *
 * Returns: the next enumeration index reported by PipeWire.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_param_get_next(PwgParam *self);

/**
 * pwg_param_get_pod_type:
 * @self: a parameter object.
 *
 * Gets the top-level SPA POD type id.
 *
 * Returns: the top-level SPA POD type id.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_param_get_pod_type(PwgParam *self);

/**
 * pwg_param_dup_pod_type_name:
 * @self: a parameter object.
 *
 * Copies the short top-level SPA POD type name when known.
 *
 * Returns: (nullable) (transfer full): the short POD type name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_param_dup_pod_type_name(PwgParam *self);

/**
 * pwg_param_get_object_type:
 * @self: a parameter object.
 *
 * Gets the object type id when the copied POD is a SPA object.
 *
 * Returns: the object type id, or 0 when the POD is not an object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_param_get_object_type(PwgParam *self);

/**
 * pwg_param_dup_object_type_name:
 * @self: a parameter object.
 *
 * Copies the short object type name when the copied POD is a SPA object.
 *
 * Returns: (nullable) (transfer full): the short object type name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_param_dup_object_type_name(PwgParam *self);

/**
 * pwg_param_get_object_id:
 * @self: a parameter object.
 *
 * Gets the object id when the copied POD is a SPA object.
 *
 * Returns: the object id, or 0 when the POD is not an object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_param_get_object_id(PwgParam *self);

/**
 * pwg_param_dup_object_id_name:
 * @self: a parameter object.
 *
 * Copies the short object id name when the copied POD is a SPA object.
 *
 * Returns: (nullable) (transfer full): the short object id name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_param_dup_object_id_name(PwgParam *self);

/**
 * pwg_param_get_format_media_type:
 * @self: a parameter object.
 *
 * Gets the SPA media type id when this parameter contains a SPA Format object.
 *
 * Returns: the SPA media type id, or 0 when this is not a Format parameter.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_param_get_format_media_type(PwgParam *self);

/**
 * pwg_param_dup_format_media_type_name:
 * @self: a parameter object.
 *
 * Copies the short SPA media type name when this parameter contains a SPA
 * Format object.
 *
 * Returns: (nullable) (transfer full): the short media type name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_param_dup_format_media_type_name(PwgParam *self);

/**
 * pwg_param_get_format_media_subtype:
 * @self: a parameter object.
 *
 * Gets the SPA media subtype id when this parameter contains a SPA Format
 * object.
 *
 * Returns: the SPA media subtype id, or 0 when this is not a Format parameter.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_param_get_format_media_subtype(PwgParam *self);

/**
 * pwg_param_dup_format_media_subtype_name:
 * @self: a parameter object.
 *
 * Copies the short SPA media subtype name when this parameter contains a SPA
 * Format object.
 *
 * Returns: (nullable) (transfer full): the short media subtype name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_param_dup_format_media_subtype_name(PwgParam *self);

/**
 * pwg_param_dup_audio_format:
 * @self: a parameter object.
 *
 * Parses this parameter as a raw audio SPA Format object.
 *
 * Returns: (nullable) (transfer full): a parsed audio format descriptor, or
 *   %NULL if this parameter is not a raw audio format.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgAudioFormat *pwg_param_dup_audio_format(PwgParam *self);

/**
 * pwg_param_get_bytes:
 * @self: a parameter object.
 *
 * Gets a new reference to the copied SPA POD bytes.
 *
 * Returns: (transfer full): copied SPA POD bytes.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GBytes *pwg_param_get_bytes(PwgParam *self);

/**
 * pwg_param_dup_summary:
 * @self: a parameter object.
 *
 * Copies a short diagnostic summary of the copied parameter.
 *
 * Returns: (transfer full): a short diagnostic summary.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_param_dup_summary(PwgParam *self);

G_END_DECLS
