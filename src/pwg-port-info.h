#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_PORT_INFO (pwg_port_info_get_type())

/**
 * PwgPortInfo:
 *
 * Convenience wrapper for port-specific properties from a [class@Pwg.Global].
 *
 * Since: 0.1
 * Stability: Unstable
 */
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
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgPortInfo *pwg_port_info_new_from_global(PwgGlobal *global);

/**
 * pwg_port_info_get_global:
 * @self: a port info wrapper.
 *
 * Gets the wrapped global descriptor.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_port_info_get_global(PwgPortInfo *self);

/**
 * pwg_port_info_get_id:
 * @self: a port info wrapper.
 *
 * Gets the PipeWire global id for the wrapped port.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
unsigned int pwg_port_info_get_id(PwgPortInfo *self);

/**
 * pwg_port_info_dup_name:
 * @self: a port info wrapper.
 *
 * Copies the PipeWire `port.name` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `port.name`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_name(PwgPortInfo *self);

/**
 * pwg_port_info_dup_description:
 * @self: a port info wrapper.
 *
 * Copies the best available human-readable port description.
 *
 * Returns: (nullable) (transfer full): a human-readable description, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_description(PwgPortInfo *self);

/**
 * pwg_port_info_dup_alias:
 * @self: a port info wrapper.
 *
 * Copies the PipeWire `port.alias` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `port.alias`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_alias(PwgPortInfo *self);

/**
 * pwg_port_info_dup_direction:
 * @self: a port info wrapper.
 *
 * Copies the PipeWire `port.direction` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `port.direction`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_direction(PwgPortInfo *self);

/**
 * pwg_port_info_get_physical:
 * @self: a port info wrapper.
 *
 * Gets whether the PipeWire `port.physical` property is true.
 *
 * Returns: whether the wrapped port is marked as physical.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_port_info_get_physical(PwgPortInfo *self);

/**
 * pwg_port_info_dup_audio_channel:
 * @self: a port info wrapper.
 *
 * Copies the PipeWire `audio.channel` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `audio.channel`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_audio_channel(PwgPortInfo *self);

/**
 * pwg_port_info_dup_media_class:
 * @self: a port info wrapper.
 *
 * Copies the PipeWire `media.class` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `media.class`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_media_class(PwgPortInfo *self);

/**
 * pwg_port_info_dup_object_serial:
 * @self: a port info wrapper.
 *
 * Copies the PipeWire `object.serial` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_port_info_dup_object_serial(PwgPortInfo *self);

G_END_DECLS
