#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_DEVICE_INFO (pwg_device_info_get_type())

/**
 * PwgDeviceInfo:
 *
 * Convenience wrapper for device-specific properties from a [class@Pwg.Global].
 *
 * Since: 0.1
 * Stability: Unstable
 */
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
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgDeviceInfo *pwg_device_info_new_from_global(PwgGlobal *global);

/**
 * pwg_device_info_get_global:
 * @self: a device info wrapper.
 *
 * Gets the wrapped global descriptor.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_device_info_get_global(PwgDeviceInfo *self);

/**
 * pwg_device_info_get_id:
 * @self: a device info wrapper.
 *
 * Gets the PipeWire global id for the wrapped device.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
unsigned int pwg_device_info_get_id(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_name:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.name` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.name`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_name(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_description:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.description` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.description`, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_description(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_nick:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.nick` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.nick`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_nick(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_api:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.api` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.api`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_api(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_class:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.class` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.class`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_class(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_form_factor:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.form-factor` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.form-factor`, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_form_factor(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_bus:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.bus` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.bus`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_bus(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_vendor_name:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.vendor.name` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.vendor.name`, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_vendor_name(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_product_name:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `device.product.name` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `device.product.name`, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_product_name(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_media_class:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `media.class` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `media.class`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_media_class(PwgDeviceInfo *self);

/**
 * pwg_device_info_dup_object_serial:
 * @self: a device info wrapper.
 *
 * Copies the PipeWire `object.serial` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_device_info_dup_object_serial(PwgDeviceInfo *self);

G_END_DECLS
