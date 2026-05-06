#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_CLIENT_INFO (pwg_client_info_get_type())

/**
 * PwgClientInfo:
 *
 * Convenience wrapper for client-specific properties from a [class@Pwg.Global].
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgClientInfo, pwg_client_info, PWG, CLIENT_INFO, GObject)

/**
 * pwg_client_info_new_from_global:
 * @global: a PipeWire global descriptor.
 *
 * Creates an immutable client info wrapper for a client global.
 *
 * Returns: (nullable) (transfer full): a new client info object, or %NULL if
 *   @global is not a PipeWire client.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgClientInfo *pwg_client_info_new_from_global(PwgGlobal *global);

/**
 * pwg_client_info_get_global:
 * @self: a client info wrapper.
 *
 * Gets the wrapped global descriptor.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_client_info_get_global(PwgClientInfo *self);

/**
 * pwg_client_info_get_id:
 * @self: a client info wrapper.
 *
 * Gets the PipeWire global id for the wrapped client.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_client_info_get_id(PwgClientInfo *self);

/**
 * pwg_client_info_dup_name:
 * @self: a client info wrapper.
 *
 * Copies the PipeWire `client.name` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `client.name`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_name(PwgClientInfo *self);

/**
 * pwg_client_info_dup_app_name:
 * @self: a client info wrapper.
 *
 * Copies the PipeWire `application.name` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `application.name`, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_app_name(PwgClientInfo *self);

/**
 * pwg_client_info_dup_app_id:
 * @self: a client info wrapper.
 *
 * Copies the PipeWire `application.id` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `application.id`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_app_id(PwgClientInfo *self);

/**
 * pwg_client_info_dup_api:
 * @self: a client info wrapper.
 *
 * Copies the PipeWire `client.api` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `client.api`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_api(PwgClientInfo *self);

/**
 * pwg_client_info_dup_access:
 * @self: a client info wrapper.
 *
 * Copies the PipeWire `pipewire.client.access` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire client access value, or
 *   %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_access(PwgClientInfo *self);

/**
 * pwg_client_info_dup_process_binary:
 * @self: a client info wrapper.
 *
 * Copies the PipeWire `application.process.binary` property.
 *
 * Returns: (nullable) (transfer full): the process binary name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_process_binary(PwgClientInfo *self);

/**
 * pwg_client_info_dup_process_id:
 * @self: a client info wrapper.
 *
 * Copies the PipeWire `application.process.id` property.
 *
 * The process id is returned as a string because registry properties are copied
 * from PipeWire's string property map.
 *
 * Returns: (nullable) (transfer full): the process id string, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_process_id(PwgClientInfo *self);

/**
 * pwg_client_info_dup_object_serial:
 * @self: a client info wrapper.
 *
 * Copies the PipeWire `object.serial` property.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_object_serial(PwgClientInfo *self);

G_END_DECLS
