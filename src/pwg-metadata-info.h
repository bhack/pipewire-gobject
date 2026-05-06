#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_METADATA_INFO (pwg_metadata_info_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgMetadataInfo, pwg_metadata_info, PWG, METADATA_INFO, GObject)

/**
 * pwg_metadata_info_new_from_global:
 * @global: a PipeWire global descriptor.
 *
 * Creates an immutable metadata info wrapper for a metadata global.
 *
 * Returns: (nullable) (transfer full): a new metadata info object, or %NULL if
 *   @global is not a PipeWire metadata object.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgMetadataInfo *pwg_metadata_info_new_from_global(PwgGlobal *global);

/**
 * pwg_metadata_info_get_global:
 * @self: a metadata info wrapper.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_metadata_info_get_global(PwgMetadataInfo *self);

/**
 * pwg_metadata_info_get_id:
 * @self: a metadata info wrapper.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
guint pwg_metadata_info_get_id(PwgMetadataInfo *self);

/**
 * pwg_metadata_info_dup_name:
 * @self: a metadata info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `metadata.name`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_metadata_info_dup_name(PwgMetadataInfo *self);

/**
 * pwg_metadata_info_dup_object_serial:
 * @self: a metadata info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_metadata_info_dup_object_serial(PwgMetadataInfo *self);

G_END_DECLS
