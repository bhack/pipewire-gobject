#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "pwg-core.h"
#include "pwg-defs.h"
#include "pwg-global.h"
#include "pwg-param.h"

G_BEGIN_DECLS

#define PWG_TYPE_DEVICE (pwg_device_get_type())

/**
 * PwgDevice:
 *
 * Live PipeWire device proxy for parameter inspection and limited parameter
 * updates.
 *
 * Device objects bind a discovered device global and expose copied parameter
 * info and enumeration results. They can queue copied parameter updates built
 * by this library, but they do not expose raw PipeWire proxy ownership to
 * language bindings.
 *
 * Device route parameters can be inspected and observed with this class, but
 * route policy and default-device decisions remain the responsibility of the
 * host session manager.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgDevice, pwg_device, PWG, DEVICE, GObject)

/**
 * pwg_device_new:
 * @core: a PipeWire core wrapper.
 * @global: a PipeWire device global descriptor.
 *
 * Creates a live device proxy wrapper for parameter inspection and limited
 * copied parameter updates.
 *
 * Returns: (nullable) (transfer full): a new device wrapper, or %NULL if
 *   @global is not a PipeWire device.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgDevice *pwg_device_new(PwgCore *core, PwgGlobal *global);

/**
 * pwg_device_start:
 * @self: a device wrapper.
 * @error: return location for a #GError.
 *
 * Binds the device proxy. If the core is not connected, this method connects it
 * first.
 *
 * Returns: %TRUE when the device proxy started or was already running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_device_start(PwgDevice *self, GError **error);

/**
 * pwg_device_stop:
 * @self: a device wrapper.
 *
 * Stops the device proxy and clears copied parameter state.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_device_stop(PwgDevice *self);

/**
 * pwg_device_get_core:
 * @self: a device wrapper.
 *
 * Gets the core used by this device wrapper.
 *
 * Returns: (transfer none): the core used by this device wrapper.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgCore *pwg_device_get_core(PwgDevice *self);

/**
 * pwg_device_get_global:
 * @self: a device wrapper.
 *
 * Gets the global descriptor used to bind this device.
 *
 * Returns: (transfer none): the global descriptor used to bind this device.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_device_get_global(PwgDevice *self);

/**
 * pwg_device_get_running:
 * @self: a device wrapper.
 *
 * Gets whether the device wrapper is running.
 *
 * Returns: whether the device wrapper is running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_device_get_running(PwgDevice *self);

/**
 * pwg_device_get_bound:
 * @self: a device wrapper.
 *
 * Gets whether the device proxy is currently bound.
 *
 * Returns: whether the device proxy is currently bound.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_device_get_bound(PwgDevice *self);

/**
 * pwg_device_get_param_infos:
 * @self: a device wrapper.
 *
 * Gets the live model of [class@Pwg.ParamInfo] descriptors advertised by
 * device info events.
 *
 * Returns: (transfer none): a #GListModel of #PwgParamInfo objects.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GListModel *pwg_device_get_param_infos(PwgDevice *self);

/**
 * pwg_device_get_params:
 * @self: a device wrapper.
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
GListModel *pwg_device_get_params(PwgDevice *self);

/**
 * pwg_device_subscribe_params:
 * @self: a device wrapper.
 * @ids: a `au` variant containing SPA parameter ids to subscribe to.
 * @error: return location for a #GError.
 *
 * Replaces the device's active parameter subscription set with the ids in @ids.
 * PipeWire emits copied [class@Pwg.Param] values through
 * [signal@Pwg.Device::param] for current subscribed values and later updates.
 * Passing an empty `au` variant clears the subscription.
 *
 * Applications should not use [method@Pwg.Param.get_seq] to distinguish the
 * first subscription delivery from later change notifications.
 *
 * Returns: %TRUE when the subscription request was queued.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_device_subscribe_params(PwgDevice *self, GVariant *ids, GError **error);

/**
 * pwg_device_enum_params:
 * @self: a device wrapper.
 * @id: the SPA parameter id to enumerate.
 * @start: the starting enumeration index.
 * @num: maximum number of params to request, or 0 for all.
 * @error: return location for a #GError.
 *
 * Requests read-only enumeration of device parameters for @id. Results are
 * appended to [method@Pwg.Device.get_params] and emitted through the
 * [signal@Pwg.Device::param] signal.
 *
 * Returns: the PipeWire request sequence number, or -1 on failure.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
int pwg_device_enum_params(PwgDevice *self,
                           unsigned int id,
                           unsigned int start,
                           unsigned int num,
                           GError **error);

/**
 * pwg_device_enum_all_params:
 * @self: a device wrapper.
 * @error: return location for a #GError.
 *
 * Requests read-only enumeration of all available device parameter ids.
 *
 * Returns: the PipeWire request sequence number, or -1 on failure.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
int pwg_device_enum_all_params(PwgDevice *self, GError **error);

/**
 * pwg_device_set_param:
 * @self: a device wrapper.
 * @param: a copied parameter built or returned by this library.
 * @error: return location for a #GError.
 *
 * Queues a parameter update on the bound device. A successful return means
 * PipeWire accepted the request for dispatch; it does not confirm that the
 * target applied the new value. Observe later device state or parameter events
 * when application logic needs confirmation.
 *
 * Returns: %TRUE when the update request was queued.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_device_set_param(PwgDevice *self, PwgParam *param, GError **error);

G_END_DECLS
