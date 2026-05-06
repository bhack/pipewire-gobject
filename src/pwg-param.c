#include "pwg-param.h"

#include <spa/debug/types.h>
#include <spa/param/param-types.h>
#include <spa/pod/iter.h>
#include <spa/pod/pod.h>
#include <spa/utils/type.h>

#include "pwg-param-private.h"

struct _PwgParam {
  GObject parent_instance;
  gint seq;
  guint id;
  guint index;
  guint next;
  GBytes *bytes;
};

G_DEFINE_TYPE(PwgParam, pwg_param, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_SEQ,
  PROP_ID,
  PROP_NAME,
  PROP_INDEX,
  PROP_NEXT,
  PROP_POD_TYPE,
  PROP_POD_TYPE_NAME,
  PROP_OBJECT_TYPE,
  PROP_OBJECT_TYPE_NAME,
  PROP_OBJECT_ID,
  PROP_OBJECT_ID_NAME,
  PROP_BYTES,
  PROP_SUMMARY,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static GBytes *
pwg_param_empty_bytes(void)
{
  return g_bytes_new_static("", 0);
}

static const struct spa_pod *
pwg_param_get_pod(PwgParam *self)
{
  gsize size = 0;
  const struct spa_pod *pod;

  pod = g_bytes_get_data(self->bytes, &size);
  if (pod == NULL || size < sizeof(struct spa_pod))
    return NULL;
  if (SPA_POD_SIZE(pod) > size)
    return NULL;

  return pod;
}

static const char *
pwg_param_id_name(guint id)
{
  return spa_debug_type_find_short_name(spa_type_param, id);
}

static const char *
pwg_param_pod_type_name(guint type)
{
  return spa_debug_type_find_short_name(SPA_TYPE_ROOT, type);
}

static const char *
pwg_param_object_type_name(guint type)
{
  return spa_debug_type_find_short_name(SPA_TYPE_ROOT, type);
}

static const char *
pwg_param_object_id_name(guint object_type, guint object_id)
{
  const struct spa_type_info *type_info;
  const struct spa_type_info *id_root;

  type_info = spa_debug_type_find(SPA_TYPE_ROOT, object_type);
  if (type_info == NULL || type_info->values == NULL)
    return NULL;

  id_root = spa_debug_type_find(type_info->values, 0);
  if (id_root == NULL || id_root->values == NULL)
    return NULL;

  return spa_debug_type_find_short_name(id_root->values, object_id);
}

static guint
pwg_param_count_object_properties(const struct spa_pod *pod)
{
  const struct spa_pod_object *object = (const struct spa_pod_object *) pod;
  struct spa_pod_prop *prop;
  guint count = 0;

  if (pod == NULL || SPA_POD_TYPE(pod) != SPA_TYPE_Object)
    return 0;

  SPA_POD_OBJECT_FOREACH(object, prop)
    count++;

  return count;
}

static char *
pwg_param_dup_basic_value_summary(const struct spa_pod *pod)
{
  if (pod == NULL)
    return g_strdup("invalid");

  switch (SPA_POD_TYPE(pod)) {
  case SPA_TYPE_Bool:
    return g_strdup(((const struct spa_pod_bool *) pod)->value ? "true" : "false");
  case SPA_TYPE_Id: {
    guint value = ((const struct spa_pod_id *) pod)->value;
    const char *name = spa_debug_type_find_short_name(SPA_TYPE_ROOT, value);

    return name != NULL ? g_strdup_printf("%s (%u)", name, value) : g_strdup_printf("%u", value);
  }
  case SPA_TYPE_Int:
    return g_strdup_printf("%d", ((const struct spa_pod_int *) pod)->value);
  case SPA_TYPE_Long:
    return g_strdup_printf("%" G_GINT64_FORMAT, ((const struct spa_pod_long *) pod)->value);
  case SPA_TYPE_Float:
    return g_strdup_printf("%g", (double) ((const struct spa_pod_float *) pod)->value);
  case SPA_TYPE_Double:
    return g_strdup_printf("%g", ((const struct spa_pod_double *) pod)->value);
  case SPA_TYPE_String: {
    const char *value = SPA_POD_BODY_CONST(pod);

    return g_strdup_printf("\"%.*s\"", (int) SPA_POD_BODY_SIZE(pod), value);
  }
  case SPA_TYPE_Rectangle: {
    const struct spa_rectangle *value = &((const struct spa_pod_rectangle *) pod)->value;

    return g_strdup_printf("%ux%u", value->width, value->height);
  }
  case SPA_TYPE_Fraction: {
    const struct spa_fraction *value = &((const struct spa_pod_fraction *) pod)->value;

    return g_strdup_printf("%u/%u", value->num, value->denom);
  }
  default:
    return NULL;
  }
}

static void
pwg_param_get_property(GObject *object,
                       guint property_id,
                       GValue *value,
                       GParamSpec *pspec)
{
  PwgParam *self = PWG_PARAM(object);

  switch (property_id) {
  case PROP_SEQ:
    g_value_set_int(value, self->seq);
    break;
  case PROP_ID:
    g_value_set_uint(value, self->id);
    break;
  case PROP_NAME:
    g_value_set_string(value, pwg_param_id_name(self->id));
    break;
  case PROP_INDEX:
    g_value_set_uint(value, self->index);
    break;
  case PROP_NEXT:
    g_value_set_uint(value, self->next);
    break;
  case PROP_POD_TYPE:
    g_value_set_uint(value, pwg_param_get_pod_type(self));
    break;
  case PROP_POD_TYPE_NAME:
    g_value_take_string(value, pwg_param_dup_pod_type_name(self));
    break;
  case PROP_OBJECT_TYPE:
    g_value_set_uint(value, pwg_param_get_object_type(self));
    break;
  case PROP_OBJECT_TYPE_NAME:
    g_value_take_string(value, pwg_param_dup_object_type_name(self));
    break;
  case PROP_OBJECT_ID:
    g_value_set_uint(value, pwg_param_get_object_id(self));
    break;
  case PROP_OBJECT_ID_NAME:
    g_value_take_string(value, pwg_param_dup_object_id_name(self));
    break;
  case PROP_BYTES:
    g_value_set_boxed(value, self->bytes);
    break;
  case PROP_SUMMARY:
    g_value_take_string(value, pwg_param_dup_summary(self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_param_set_property(GObject *object,
                       guint property_id,
                       const GValue *value,
                       GParamSpec *pspec)
{
  PwgParam *self = PWG_PARAM(object);

  switch (property_id) {
  case PROP_SEQ:
    self->seq = g_value_get_int(value);
    break;
  case PROP_ID:
    self->id = g_value_get_uint(value);
    break;
  case PROP_INDEX:
    self->index = g_value_get_uint(value);
    break;
  case PROP_NEXT:
    self->next = g_value_get_uint(value);
    break;
  case PROP_BYTES:
    g_clear_pointer(&self->bytes, g_bytes_unref);
    self->bytes = g_value_dup_boxed(value);
    if (self->bytes == NULL)
      self->bytes = pwg_param_empty_bytes();
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_param_finalize(GObject *object)
{
  PwgParam *self = PWG_PARAM(object);

  g_clear_pointer(&self->bytes, g_bytes_unref);

  G_OBJECT_CLASS(pwg_param_parent_class)->finalize(object);
}

static void
pwg_param_class_init(PwgParamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_param_get_property;
  object_class->set_property = pwg_param_set_property;
  object_class->finalize = pwg_param_finalize;

  /**
   * PwgParam:seq:
   *
   * Enumeration request sequence number.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_SEQ] = g_param_spec_int(
    "seq",
    "Seq",
    "Enumeration request sequence number.",
    G_MININT,
    G_MAXINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:id:
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
   * PwgParam:name:
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
   * PwgParam:index:
   *
   * Enumeration index for this parameter.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_INDEX] = g_param_spec_uint(
    "index",
    "Index",
    "Enumeration index for this parameter.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:next:
   *
   * PipeWire next enumeration index.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_NEXT] = g_param_spec_uint(
    "next",
    "Next",
    "PipeWire next enumeration index.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:pod-type:
   *
   * Top-level SPA POD type id.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_POD_TYPE] = g_param_spec_uint(
    "pod-type",
    "POD type",
    "Top-level SPA POD type id.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:pod-type-name:
   *
   * Short top-level SPA POD type name when known.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_POD_TYPE_NAME] = g_param_spec_string(
    "pod-type-name",
    "POD type name",
    "Short top-level SPA POD type name when known.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:object-type:
   *
   * SPA object type id when the copied POD is an object.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_OBJECT_TYPE] = g_param_spec_uint(
    "object-type",
    "Object type",
    "SPA object type id when the copied POD is an object.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:object-type-name:
   *
   * Short object type name when the copied POD is an object.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_OBJECT_TYPE_NAME] = g_param_spec_string(
    "object-type-name",
    "Object type name",
    "Short object type name when the copied POD is an object.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:object-id:
   *
   * SPA object id when the copied POD is an object.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_OBJECT_ID] = g_param_spec_uint(
    "object-id",
    "Object id",
    "SPA object id when the copied POD is an object.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:object-id-name:
   *
   * Short object id name when the copied POD is an object.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_OBJECT_ID_NAME] = g_param_spec_string(
    "object-id-name",
    "Object id name",
    "Short object id name when the copied POD is an object.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:bytes:
   *
   * Copied SPA POD bytes.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_BYTES] = g_param_spec_boxed(
    "bytes",
    "Bytes",
    "Copied SPA POD bytes.",
    G_TYPE_BYTES,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgParam:summary:
   *
   * Short diagnostic summary of the copied parameter.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_SUMMARY] = g_param_spec_string(
    "summary",
    "Summary",
    "Short diagnostic summary of the copied parameter.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_param_init(PwgParam *self)
{
  self->bytes = pwg_param_empty_bytes();
}

PwgParam *
_pwg_param_new(gint seq,
               guint id,
               guint index,
               guint next,
               GBytes *bytes)
{
  g_return_val_if_fail(bytes != NULL, NULL);

  return g_object_new(
    PWG_TYPE_PARAM,
    "seq", seq,
    "id", id,
    "index", index,
    "next", next,
    "bytes", bytes,
    NULL);
}

gint
pwg_param_get_seq(PwgParam *self)
{
  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  return self->seq;
}

guint
pwg_param_get_id(PwgParam *self)
{
  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  return self->id;
}

char *
pwg_param_dup_name(PwgParam *self)
{
  const char *name;

  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  name = pwg_param_id_name(self->id);
  return name != NULL ? g_strdup(name) : NULL;
}

guint
pwg_param_get_index(PwgParam *self)
{
  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  return self->index;
}

guint
pwg_param_get_next(PwgParam *self)
{
  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  return self->next;
}

guint
pwg_param_get_pod_type(PwgParam *self)
{
  const struct spa_pod *pod;

  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  pod = pwg_param_get_pod(self);
  return pod != NULL ? SPA_POD_TYPE(pod) : 0;
}

char *
pwg_param_dup_pod_type_name(PwgParam *self)
{
  const char *name;
  guint type;

  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  type = pwg_param_get_pod_type(self);
  name = pwg_param_pod_type_name(type);
  return name != NULL ? g_strdup(name) : NULL;
}

guint
pwg_param_get_object_type(PwgParam *self)
{
  const struct spa_pod *pod;

  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  pod = pwg_param_get_pod(self);
  if (pod == NULL || SPA_POD_TYPE(pod) != SPA_TYPE_Object)
    return 0;

  return SPA_POD_OBJECT_TYPE((const struct spa_pod_object *) pod);
}

char *
pwg_param_dup_object_type_name(PwgParam *self)
{
  const char *name;
  guint type;

  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  type = pwg_param_get_object_type(self);
  name = pwg_param_object_type_name(type);
  return name != NULL ? g_strdup(name) : NULL;
}

guint
pwg_param_get_object_id(PwgParam *self)
{
  const struct spa_pod *pod;

  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  pod = pwg_param_get_pod(self);
  if (pod == NULL || SPA_POD_TYPE(pod) != SPA_TYPE_Object)
    return 0;

  return SPA_POD_OBJECT_ID((const struct spa_pod_object *) pod);
}

char *
pwg_param_dup_object_id_name(PwgParam *self)
{
  const char *name;
  guint object_type;
  guint object_id;

  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  object_type = pwg_param_get_object_type(self);
  object_id = pwg_param_get_object_id(self);
  name = pwg_param_object_id_name(object_type, object_id);
  return name != NULL ? g_strdup(name) : NULL;
}

GBytes *
pwg_param_get_bytes(PwgParam *self)
{
  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  return g_bytes_ref(self->bytes);
}

char *
pwg_param_dup_summary(PwgParam *self)
{
  const struct spa_pod *pod;
  g_autofree char *value_summary = NULL;
  const char *pod_type_name;
  guint pod_type;

  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  pod = pwg_param_get_pod(self);
  if (pod == NULL)
    return g_strdup("invalid POD");

  pod_type = SPA_POD_TYPE(pod);
  pod_type_name = pwg_param_pod_type_name(pod_type);
  if (pod_type == SPA_TYPE_Object) {
    g_autofree char *object_type_name = pwg_param_dup_object_type_name(self);
    g_autofree char *object_id_name = pwg_param_dup_object_id_name(self);
    guint props = pwg_param_count_object_properties(pod);

    return g_strdup_printf(
      "%s %s (%u), id %s (%u), %u properties",
      pod_type_name != NULL ? pod_type_name : "Object",
      object_type_name != NULL ? object_type_name : "unknown",
      pwg_param_get_object_type(self),
      object_id_name != NULL ? object_id_name : "unknown",
      pwg_param_get_object_id(self),
      props);
  }

  value_summary = pwg_param_dup_basic_value_summary(pod);
  if (value_summary != NULL) {
    return g_strdup_printf(
      "%s %s",
      pod_type_name != NULL ? pod_type_name : "POD",
      value_summary);
  }

  return g_strdup_printf(
    "%s (%u), %u bytes",
    pod_type_name != NULL ? pod_type_name : "POD",
    pod_type,
    SPA_POD_BODY_SIZE(pod));
}
