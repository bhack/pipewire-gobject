#include "pwg-route-info.h"

#include <spa/param/param.h>
#include <spa/param/route.h>
#include <spa/pod/iter.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>
#include <spa/utils/defs.h>

#include "pwg-param-private.h"

struct _PwgRouteInfo {
  GObject parent_instance;
  int index;
  int device;
  int profile;
  int priority;
  unsigned int direction_id;
  unsigned int available_id;
  char *name;
  char *description;
  GVariant *info;
};

G_DEFINE_TYPE(PwgRouteInfo, pwg_route_info, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_INDEX,
  PROP_DEVICE,
  PROP_PROFILE,
  PROP_PRIORITY,
  PROP_DIRECTION_ID,
  PROP_DIRECTION,
  PROP_NAME,
  PROP_DESCRIPTION,
  PROP_AVAILABILITY,
  PROP_INFO,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static const char *
pwg_route_info_direction_name(unsigned int direction)
{
  switch (direction) {
  case SPA_DIRECTION_INPUT:
    return "input";
  case SPA_DIRECTION_OUTPUT:
    return "output";
  default:
    return NULL;
  }
}

static const char *
pwg_route_info_availability_name(unsigned int available)
{
  switch (available) {
  case SPA_PARAM_AVAILABILITY_unknown:
    return "unknown";
  case SPA_PARAM_AVAILABILITY_no:
    return "no";
  case SPA_PARAM_AVAILABILITY_yes:
    return "yes";
  default:
    return NULL;
  }
}

static GVariant *
pwg_route_info_empty_info(void)
{
  GVariantBuilder builder;

  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{ss}"));
  return g_variant_ref_sink(g_variant_builder_end(&builder));
}

static GVariant *
pwg_route_info_parse_info(const struct spa_pod *pod)
{
  GVariantBuilder builder;
  struct spa_pod_parser parser;
  struct spa_pod_frame frame;
  int32_t n_items = 0;

  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{ss}"));

  if (pod == NULL || SPA_POD_TYPE(pod) != SPA_TYPE_Struct)
    return g_variant_ref_sink(g_variant_builder_end(&builder));

  spa_pod_parser_pod(&parser, pod);
  if (spa_pod_parser_push_struct(&parser, &frame) < 0 ||
      spa_pod_parser_get_int(&parser, &n_items) < 0 ||
      n_items < 0)
    return g_variant_ref_sink(g_variant_builder_end(&builder));

  for (int32_t i = 0; i < n_items; i++) {
    const char *key = NULL;
    const char *value = NULL;

    if (spa_pod_parser_get_string(&parser, &key) < 0 ||
        spa_pod_parser_get_string(&parser, &value) < 0)
      break;

    if (key != NULL && value != NULL)
      g_variant_builder_add(&builder, "{ss}", key, value);
  }

  spa_pod_parser_pop(&parser, &frame);
  return g_variant_ref_sink(g_variant_builder_end(&builder));
}

static void
pwg_route_info_get_property(GObject *object,
                            unsigned int property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
  PwgRouteInfo *self = PWG_ROUTE_INFO(object);

  switch (property_id) {
  case PROP_INDEX:
    g_value_set_int(value, self->index);
    break;
  case PROP_DEVICE:
    g_value_set_int(value, self->device);
    break;
  case PROP_PROFILE:
    g_value_set_int(value, self->profile);
    break;
  case PROP_PRIORITY:
    g_value_set_int(value, self->priority);
    break;
  case PROP_DIRECTION_ID:
    g_value_set_uint(value, self->direction_id);
    break;
  case PROP_DIRECTION:
    g_value_take_string(value, pwg_route_info_dup_direction(self));
    break;
  case PROP_NAME:
    g_value_set_string(value, self->name);
    break;
  case PROP_DESCRIPTION:
    g_value_set_string(value, self->description);
    break;
  case PROP_AVAILABILITY:
    g_value_take_string(value, pwg_route_info_dup_availability(self));
    break;
  case PROP_INFO:
    g_value_set_variant(value, self->info);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_route_info_finalize(GObject *object)
{
  PwgRouteInfo *self = PWG_ROUTE_INFO(object);

  g_clear_pointer(&self->name, g_free);
  g_clear_pointer(&self->description, g_free);
  g_clear_pointer(&self->info, g_variant_unref);

  G_OBJECT_CLASS(pwg_route_info_parent_class)->finalize(object);
}

static void
pwg_route_info_class_init(PwgRouteInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_route_info_get_property;
  object_class->finalize = pwg_route_info_finalize;

  /**
   * PwgRouteInfo:index:
   *
   * PipeWire route index, or -1 if absent.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_INDEX] = g_param_spec_int(
    "index",
    "Index",
    "Route index.",
    -1,
    G_MAXINT,
    -1,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:device:
   *
   * Route profile-device index, or -1 if absent.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_DEVICE] = g_param_spec_int(
    "device",
    "Device",
    "Route device index.",
    -1,
    G_MAXINT,
    -1,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:profile:
   *
   * Associated active profile index, or -1 if absent.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_PROFILE] = g_param_spec_int(
    "profile",
    "Profile",
    "Associated active profile index.",
    -1,
    G_MAXINT,
    -1,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:priority:
   *
   * Route priority, or -1 if absent.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_PRIORITY] = g_param_spec_int(
    "priority",
    "Priority",
    "Route priority.",
    G_MININT,
    G_MAXINT,
    -1,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:direction-id:
   *
   * Route direction SPA id, or `SPA_ID_INVALID` if absent.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_DIRECTION_ID] = g_param_spec_uint(
    "direction-id",
    "Direction id",
    "SPA direction id.",
    0,
    G_MAXUINT,
    SPA_ID_INVALID,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:direction:
   *
   * Route direction nick, such as `input` or `output`, or %NULL.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_DIRECTION] = g_param_spec_string(
    "direction",
    "Direction",
    "Route direction name.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:name:
   *
   * Route name, or %NULL.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_NAME] = g_param_spec_string(
    "name",
    "Name",
    "Route name.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:description:
   *
   * Route description, or %NULL.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_DESCRIPTION] = g_param_spec_string(
    "description",
    "Description",
    "Route description.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:availability:
   *
   * Route availability nick, such as `yes`, `no`, or `unknown`, or %NULL.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_AVAILABILITY] = g_param_spec_string(
    "availability",
    "Availability",
    "Route availability.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgRouteInfo:info:
   *
   * Route info dictionary as an `a{ss}` variant.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_INFO] = g_param_spec_variant(
    "info",
    "Info",
    "Route info as an a{ss} variant.",
    G_VARIANT_TYPE("a{ss}"),
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_route_info_init(PwgRouteInfo *self)
{
  self->index = -1;
  self->device = -1;
  self->profile = -1;
  self->priority = -1;
  self->direction_id = SPA_ID_INVALID;
  self->available_id = SPA_ID_INVALID;
  self->info = pwg_route_info_empty_info();
}

PwgRouteInfo *
pwg_route_info_new_from_param(PwgParam *param)
{
  const struct spa_pod *pod;
  struct spa_pod *info = NULL;
  int32_t index = -1;
  int32_t device = -1;
  int32_t profile = -1;
  int32_t priority = -1;
  uint32_t direction = SPA_ID_INVALID;
  uint32_t available = SPA_ID_INVALID;
  const char *name = NULL;
  const char *description = NULL;
  g_autoptr(PwgRouteInfo) self = NULL;

  g_return_val_if_fail(PWG_IS_PARAM(param), NULL);

  if (pwg_param_get_id(param) != SPA_PARAM_Route &&
      pwg_param_get_id(param) != SPA_PARAM_EnumRoute)
    return NULL;

  pod = _pwg_param_get_pod(param);
  if (pod == NULL ||
      SPA_POD_TYPE(pod) != SPA_TYPE_Object ||
      SPA_POD_OBJECT_TYPE((const struct spa_pod_object *) pod) != SPA_TYPE_OBJECT_ParamRoute)
    return NULL;

  if (spa_pod_parse_object(
        pod,
        SPA_TYPE_OBJECT_ParamRoute,
        NULL,
        SPA_PARAM_ROUTE_index, SPA_POD_OPT_Int(&index),
        SPA_PARAM_ROUTE_direction, SPA_POD_OPT_Id(&direction),
        SPA_PARAM_ROUTE_device, SPA_POD_OPT_Int(&device),
        SPA_PARAM_ROUTE_name, SPA_POD_OPT_String(&name),
        SPA_PARAM_ROUTE_description, SPA_POD_OPT_String(&description),
        SPA_PARAM_ROUTE_priority, SPA_POD_OPT_Int(&priority),
        SPA_PARAM_ROUTE_available, SPA_POD_OPT_Id(&available),
        SPA_PARAM_ROUTE_info, SPA_POD_OPT_Pod(&info),
        SPA_PARAM_ROUTE_profile, SPA_POD_OPT_Int(&profile)) < 0)
    return NULL;

  self = g_object_new(PWG_TYPE_ROUTE_INFO, NULL);
  self->index = index;
  self->device = device;
  self->profile = profile;
  self->priority = priority;
  self->direction_id = direction;
  self->available_id = available;
  self->name = g_strdup(name);
  self->description = g_strdup(description);
  g_clear_pointer(&self->info, g_variant_unref);
  self->info = pwg_route_info_parse_info(info);

  return g_steal_pointer(&self);
}

int
pwg_route_info_get_index(PwgRouteInfo *self)
{
  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), -1);

  return self->index;
}

int
pwg_route_info_get_device(PwgRouteInfo *self)
{
  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), -1);

  return self->device;
}

int
pwg_route_info_get_profile(PwgRouteInfo *self)
{
  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), -1);

  return self->profile;
}

int
pwg_route_info_get_priority(PwgRouteInfo *self)
{
  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), -1);

  return self->priority;
}

unsigned int
pwg_route_info_get_direction_id(PwgRouteInfo *self)
{
  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), SPA_ID_INVALID);

  return self->direction_id;
}

char *
pwg_route_info_dup_direction(PwgRouteInfo *self)
{
  const char *name;

  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), NULL);

  name = pwg_route_info_direction_name(self->direction_id);
  return name != NULL ? g_strdup(name) : NULL;
}

char *
pwg_route_info_dup_name(PwgRouteInfo *self)
{
  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), NULL);

  return g_strdup(self->name);
}

char *
pwg_route_info_dup_description(PwgRouteInfo *self)
{
  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), NULL);

  return g_strdup(self->description);
}

char *
pwg_route_info_dup_availability(PwgRouteInfo *self)
{
  const char *name;

  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), NULL);

  name = pwg_route_info_availability_name(self->available_id);
  return name != NULL ? g_strdup(name) : NULL;
}

GVariant *
pwg_route_info_get_info(PwgRouteInfo *self)
{
  g_return_val_if_fail(PWG_IS_ROUTE_INFO(self), NULL);

  return g_variant_ref(self->info);
}
