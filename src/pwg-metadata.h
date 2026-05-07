#pragma once

#include <glib-object.h>

#include "pwg-core.h"
#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_METADATA (pwg_metadata_get_type())

/**
 * PwgMetadata:
 *
 * Wrapper for a named PipeWire metadata object.
 *
 * Metadata objects keep a copied cache of observed key/value entries and allow
 * callers to update the bound metadata object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgMetadata, pwg_metadata, PWG, METADATA, GObject)

/**
 * pwg_metadata_new:
 * @core: a PipeWire core wrapper.
 * @name: the PipeWire metadata global name, such as `default`.
 *
 * Creates a wrapper for a named PipeWire metadata object.
 *
 * Returns: (transfer full): a new metadata wrapper.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgMetadata *pwg_metadata_new(PwgCore *core, const char *name);

/**
 * pwg_metadata_start:
 * @self: a metadata wrapper.
 * @error: return location for a #GError.
 *
 * Starts discovery for the named metadata object. If the core is not connected,
 * this method connects it first.
 *
 * Returns: %TRUE when discovery started or was already running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_metadata_start(PwgMetadata *self, GError **error);

/**
 * pwg_metadata_stop:
 * @self: a metadata wrapper.
 *
 * Stops metadata discovery and clears the current local cache.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_metadata_stop(PwgMetadata *self);

/**
 * pwg_metadata_get_core:
 * @self: a metadata wrapper.
 *
 * Gets the core used by this metadata wrapper.
 *
 * Returns: (transfer none): the core used by this metadata wrapper.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgCore *pwg_metadata_get_core(PwgMetadata *self);

/**
 * pwg_metadata_get_name:
 * @self: a metadata wrapper.
 *
 * Gets the PipeWire metadata global name.
 *
 * Returns: (transfer none): the metadata global name.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_metadata_get_name(PwgMetadata *self);

/**
 * pwg_metadata_get_running:
 * @self: a metadata wrapper.
 *
 * Gets whether metadata discovery is running.
 *
 * Returns: whether metadata discovery is running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_metadata_get_running(PwgMetadata *self);

/**
 * pwg_metadata_get_bound:
 * @self: a metadata wrapper.
 *
 * Gets whether the named metadata object has been discovered and bound.
 *
 * Returns: whether the named metadata object has been discovered and bound.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_metadata_get_bound(PwgMetadata *self);

/**
 * pwg_metadata_dup_value:
 * @self: a metadata wrapper.
 * @subject: the PipeWire global id associated with the metadata.
 * @key: a metadata key.
 *
 * Copies a cached metadata value.
 *
 * Returns: (nullable) (transfer full): the cached metadata value, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_metadata_dup_value(PwgMetadata *self, unsigned int subject, const char *key);

/**
 * pwg_metadata_dup_value_type:
 * @self: a metadata wrapper.
 * @subject: the PipeWire global id associated with the metadata.
 * @key: a metadata key.
 *
 * Copies the type string for a cached metadata value.
 *
 * Returns: (nullable) (transfer full): the cached metadata value type, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_metadata_dup_value_type(PwgMetadata *self, unsigned int subject, const char *key);

/**
 * pwg_metadata_dup_default_audio_sink_name:
 * @self: a metadata wrapper.
 *
 * Reads the standard `default.audio.sink` metadata key and returns the
 * selected PipeWire `node.name`.
 *
 * Returns: (nullable) (transfer full): the default audio sink node name, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_metadata_dup_default_audio_sink_name(PwgMetadata *self);

/**
 * pwg_metadata_dup_default_audio_source_name:
 * @self: a metadata wrapper.
 *
 * Reads the standard `default.audio.source` metadata key and returns the
 * selected PipeWire `node.name`.
 *
 * Returns: (nullable) (transfer full): the default audio source node name, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_metadata_dup_default_audio_source_name(PwgMetadata *self);

/**
 * pwg_metadata_dup_configured_audio_sink_name:
 * @self: a metadata wrapper.
 *
 * Reads the standard `default.configured.audio.sink` metadata key and returns
 * the user-configured PipeWire `node.name`.
 *
 * Returns: (nullable) (transfer full): the configured audio sink node name, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_metadata_dup_configured_audio_sink_name(PwgMetadata *self);

/**
 * pwg_metadata_dup_configured_audio_source_name:
 * @self: a metadata wrapper.
 *
 * Reads the standard `default.configured.audio.source` metadata key and returns
 * the user-configured PipeWire `node.name`.
 *
 * Returns: (nullable) (transfer full): the configured audio source node name,
 *   or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_metadata_dup_configured_audio_source_name(PwgMetadata *self);

/**
 * pwg_metadata_set:
 * @self: a metadata wrapper.
 * @subject: the PipeWire global id associated with the metadata.
 * @key: a metadata key.
 * @type: (nullable): the metadata value type, or %NULL.
 * @value: (nullable): the metadata value, or %NULL to clear @key.
 * @error: return location for a #GError.
 *
 * Sets or clears one metadata key. The named metadata object must be bound.
 *
 * Returns: %TRUE when PipeWire accepted the request.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_metadata_set(PwgMetadata *self,
                      unsigned int subject,
                      const char *key,
                      const char *type,
                      const char *value,
                      GError **error);

/**
 * pwg_metadata_clear:
 * @self: a metadata wrapper.
 * @error: return location for a #GError.
 *
 * Clears all metadata on the bound metadata object.
 *
 * Returns: %TRUE when PipeWire accepted the request.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_metadata_clear(PwgMetadata *self, GError **error);

G_END_DECLS
