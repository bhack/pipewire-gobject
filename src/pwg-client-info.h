#pragma once

#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-global.h"

G_BEGIN_DECLS

#define PWG_TYPE_CLIENT_INFO (pwg_client_info_get_type())

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
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgClientInfo *pwg_client_info_new_from_global(PwgGlobal *global);

/**
 * pwg_client_info_get_global:
 * @self: a client info wrapper.
 *
 * Returns: (transfer none): the wrapped global descriptor.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
PwgGlobal *pwg_client_info_get_global(PwgClientInfo *self);

/**
 * pwg_client_info_get_id:
 * @self: a client info wrapper.
 *
 * Returns: the PipeWire global id.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
guint pwg_client_info_get_id(PwgClientInfo *self);

/**
 * pwg_client_info_dup_client_id:
 * @self: a client info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `client.id`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_client_id(PwgClientInfo *self);

/**
 * pwg_client_info_dup_name:
 * @self: a client info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `client.name`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_name(PwgClientInfo *self);

/**
 * pwg_client_info_dup_api:
 * @self: a client info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `client.api`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_api(PwgClientInfo *self);

/**
 * pwg_client_info_dup_application_name:
 * @self: a client info wrapper.
 *
 * Returns: (nullable) (transfer full): the `application.name`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_application_name(PwgClientInfo *self);

/**
 * pwg_client_info_dup_process_id:
 * @self: a client info wrapper.
 *
 * Returns: (nullable) (transfer full): the `application.process.id`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_process_id(PwgClientInfo *self);

/**
 * pwg_client_info_dup_object_serial:
 * @self: a client info wrapper.
 *
 * Returns: (nullable) (transfer full): the PipeWire `object.serial`, or %NULL.
 *
 * Since: 0.2
 * Stability: Unstable
 */
PWG_API
char *pwg_client_info_dup_object_serial(PwgClientInfo *self);

G_END_DECLS
