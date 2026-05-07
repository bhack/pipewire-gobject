#include "pwg-param-info.h"

#include <spa/debug/types.h>
#include <spa/param/param-types.h>

#include "pwg-param-info-private.h"

struct _PwgParamInfo {
  GObject parent_instance;
  unsigned int id;
  unsigned int flags;
};

G_DEFINE_TYPE(PwgParamInfo, pwg_param_info, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_FLAGS,
  PROP_READABLE,
  PROP_WRITABLE,
  PROP_SERIAL,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static const char *
pwg_param_info_id_name(unsigned int id)
{
  return spa_debug_type_find_short_name(spa_type_param, id);
}

static void
pwg_param_info_get_property(GObject *object,
                            unsigned int property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
  PwgParamInfo *self = PWG_PARAM_INFO(object);

  switch (property_id) {
  case PROP_ID:
    g_value_set_uint(value, self->id);
    break;
  case PROP_NAME:
    g_value_set_string(value, pwg_param_info_id_name(self->id));
    break;
  case PROP_FLAGS:
    g_value_set_uint(value, self->flags);
    break;
  case PROP_READABLE:
    g_value_set_boolean(value, pwg_param_info_get_readable(self));
    break;
  case PROP_WRITABLE:
    g_value_set_boolean(value, pwg_param_info_get_writable(self));
    break;
  case PROP_SERIAL:
    g_value_set_boolean(value, pwg_param_info_get_serial(self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_param_info_set_property(GObject *object,
                            unsigned int property_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
  PwgParamInfo *self = PWG_PARAM_INFO(object);

  switch (property_id) {
  case PROP_ID:
    self->id = g_value_get_uint(value);
    break;
  case PROP_FLAGS:
    self->flags = g_value_get_uint(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_param_info_class_init(PwgParamInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_param_info_get_property;
  object_class->set_property = pwg_param_info_set_property;

  /**
   * PwgParamInfo:id:
   *
   * SPA parameter id.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_ID] = g_param_spec_uint(
    "id",
    "Id",
    "SPA parameter id.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParamInfo:name:
   *
   * Short SPA parameter name when known.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_NAME] = g_param_spec_string(
    "name",
    "Name",
    "Short SPA parameter name when known.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParamInfo:flags:
   *
   * Raw `spa_param_info` flags.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_FLAGS] = g_param_spec_uint(
    "flags",
    "Flags",
    "Raw spa_param_info flags.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParamInfo:readable:
   *
   * Whether the parameter is advertised as readable.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_READABLE] = g_param_spec_boolean(
    "readable",
    "Readable",
    "Whether the parameter is advertised as readable.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParamInfo:writable:
   *
   * Whether the parameter is advertised as writable.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_WRITABLE] = g_param_spec_boolean(
    "writable",
    "Writable",
    "Whether the parameter is advertised as writable.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParamInfo:serial:
   *
   * Whether the parameter info has the SPA serial flag.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_SERIAL] = g_param_spec_boolean(
    "serial",
    "Serial",
    "Whether the parameter info has the SPA serial flag.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_param_info_init(PwgParamInfo *self)
{
  (void) self;
}

PwgParamInfo *
_pwg_param_info_new(unsigned int id, unsigned int flags)
{
  return g_object_new(PWG_TYPE_PARAM_INFO, "id", id, "flags", flags, NULL);
}

unsigned int
pwg_param_info_get_id(PwgParamInfo *self)
{
  g_return_val_if_fail(PWG_IS_PARAM_INFO(self), 0);

  return self->id;
}

char *
pwg_param_info_dup_name(PwgParamInfo *self)
{
  const char *name;

  g_return_val_if_fail(PWG_IS_PARAM_INFO(self), NULL);

  name = pwg_param_info_id_name(self->id);
  return name != NULL ? g_strdup(name) : NULL;
}

unsigned int
pwg_param_info_get_flags(PwgParamInfo *self)
{
  g_return_val_if_fail(PWG_IS_PARAM_INFO(self), 0);

  return self->flags;
}

bool
pwg_param_info_get_readable(PwgParamInfo *self)
{
  g_return_val_if_fail(PWG_IS_PARAM_INFO(self), FALSE);

  return (self->flags & SPA_PARAM_INFO_READ) != 0;
}

bool
pwg_param_info_get_writable(PwgParamInfo *self)
{
  g_return_val_if_fail(PWG_IS_PARAM_INFO(self), FALSE);

  return (self->flags & SPA_PARAM_INFO_WRITE) != 0;
}

bool
pwg_param_info_get_serial(PwgParamInfo *self)
{
  g_return_val_if_fail(PWG_IS_PARAM_INFO(self), FALSE);

  return (self->flags & SPA_PARAM_INFO_SERIAL) != 0;
}
