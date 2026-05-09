#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "pwg-defs.h"
#include "pwg-param.h"

G_BEGIN_DECLS

#define PWG_TYPE_ROUTE_INFO (pwg_route_info_get_type())

/**
 * PwgRouteInfo:
 *
 * Immutable typed view of a copied PipeWire `ParamRoute` parameter.
 *
 * Route information is copied from [class@Pwg.Param], so it is safe to inspect
 * from language bindings without exposing raw SPA POD ownership.
 *
 * This class describes route data published by PipeWire. It does not select
 * routes or implement session-manager policy.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgRouteInfo, pwg_route_info, PWG, ROUTE_INFO, GObject)

/**
 * pwg_route_info_new_from_param:
 * @param: a copied route parameter.
 *
 * Creates a typed route view from a copied `EnumRoute` or `Route` parameter.
 *
 * Returns: (nullable) (transfer full): a route view, or %NULL if @param is not
 *   a PipeWire route parameter.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgRouteInfo *pwg_route_info_new_from_param(PwgParam *param);

/**
 * pwg_route_info_get_index:
 * @self: a route info wrapper.
 *
 * Gets the PipeWire route index.
 *
 * Returns: the route index, or -1 if absent.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
int pwg_route_info_get_index(PwgRouteInfo *self);

/**
 * pwg_route_info_get_device:
 * @self: a route info wrapper.
 *
 * Gets the route's profile-device index.
 *
 * Returns: the route device index, or -1 if absent.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
int pwg_route_info_get_device(PwgRouteInfo *self);

/**
 * pwg_route_info_get_profile:
 * @self: a route info wrapper.
 *
 * Gets the route's profile index.
 *
 * Returns: the profile index, or -1 if absent.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
int pwg_route_info_get_profile(PwgRouteInfo *self);

/**
 * pwg_route_info_get_priority:
 * @self: a route info wrapper.
 *
 * Gets the route priority.
 *
 * Returns: the route priority, or -1 if absent.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
int pwg_route_info_get_priority(PwgRouteInfo *self);

/**
 * pwg_route_info_get_direction_id:
 * @self: a route info wrapper.
 *
 * Gets the route direction SPA id.
 *
 * Returns: the route direction id, or `SPA_ID_INVALID` if absent.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
unsigned int pwg_route_info_get_direction_id(PwgRouteInfo *self);

/**
 * pwg_route_info_dup_direction:
 * @self: a route info wrapper.
 *
 * Copies the route direction nick, such as `input` or `output`.
 *
 * Returns: (nullable) (transfer full): the route direction nick, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_route_info_dup_direction(PwgRouteInfo *self);

/**
 * pwg_route_info_dup_name:
 * @self: a route info wrapper.
 *
 * Copies the route name.
 *
 * Returns: (nullable) (transfer full): the route name, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_route_info_dup_name(PwgRouteInfo *self);

/**
 * pwg_route_info_dup_description:
 * @self: a route info wrapper.
 *
 * Copies the route description.
 *
 * Returns: (nullable) (transfer full): the route description, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_route_info_dup_description(PwgRouteInfo *self);

/**
 * pwg_route_info_dup_availability:
 * @self: a route info wrapper.
 *
 * Copies the route availability nick, such as `yes`, `no`, or `unknown`.
 *
 * Returns: (nullable) (transfer full): the route availability nick, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
char *pwg_route_info_dup_availability(PwgRouteInfo *self);

/**
 * pwg_route_info_get_info:
 * @self: a route info wrapper.
 *
 * Copies the route info dictionary.
 *
 * Returns: (transfer full): a `a{ss}` dictionary.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GVariant *pwg_route_info_get_info(PwgRouteInfo *self);

G_END_DECLS
