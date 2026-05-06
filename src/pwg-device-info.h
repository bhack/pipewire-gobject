#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_DEVICE_INFO (pwg_device_info_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgDeviceInfo, pwg_device_info, PWG, DEVICE_INFO, GObject)

/**
 * pwg_device_info_new_from_global:
 * @global: a PipeWire global descriptor.
 *
 * Creates an immutable device info wrapper for a device global.
 *
 * Returns: (nullable) (transfer full): a new device info object, or %NULL if
 *   @global is not a PipeWire device.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgDeviceInfo *pwg_device_info_new_from_global(PwgGlobal *global);

/**
 * pwg_device_info_get_global:
 * @self: a device info wrapper.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_device_info_get_global(PwgDeviceInfo *self);

/**
 * pwg_device_info_get_id:
 * @self: a device info wrapper.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
guint pwg_device_info_get_id(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_device_id:
 * @self: a device info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.id`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_device_id(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_name:
 * @self: a device info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.name`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_name(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_description:
 * @self: a device info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.description`, or
 *   %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_description(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_api:
 * @self: a device info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.api`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_api(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_device_class:
 * @self: a device info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.class`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_device_class(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_object_serial:
 * @self: a device info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_object_serial(PwgDeviceInfo *self);

G_END_DECLS
