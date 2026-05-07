#include "pwg-param.h"

#include <math.h>
#include <spa/param/audio/raw-utils.h>
#include <spa/param/audio/raw-types.h>
#include <spa/param/format-types.h>
#include <spa/param/format-utils.h>
#include <spa/debug/types.h>
#include <spa/param/param-types.h>
#include <spa/param/props.h>
#include <spa/pod/builder.h>
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

#define PWG_PARAM_SIMPLE_PROPS_BUFFER_SIZE 256
#define PWG_PARAM_CONTROL_PROPS_BUFFER_SIZE (64 * 1024)

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

const struct spa_pod *
_pwg_param_get_pod(PwgParam *self)
{
  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  return pwg_param_get_pod(self);
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

static gboolean
pwg_param_parse_format(PwgParam *self, guint *media_type, guint *media_subtype)
{
  const struct spa_pod *pod;
  uint32_t parsed_media_type = 0;
  uint32_t parsed_media_subtype = 0;

  pod = pwg_param_get_pod(self);
  if (pod == NULL || SPA_POD_TYPE(pod) != SPA_TYPE_Object)
    return FALSE;
  if (SPA_POD_OBJECT_TYPE((const struct spa_pod_object *) pod) != SPA_TYPE_OBJECT_Format)
    return FALSE;
  if (spa_format_parse(pod, &parsed_media_type, &parsed_media_subtype) < 0)
    return FALSE;

  if (media_type != NULL)
    *media_type = parsed_media_type;
  if (media_subtype != NULL)
    *media_subtype = parsed_media_subtype;

  return TRUE;
}

static const char *
pwg_param_audio_format_name(enum spa_audio_format format)
{
  return spa_debug_type_find_short_name(spa_type_audio_format, format);
}

static guint
pwg_param_audio_format_bytes_per_sample(enum spa_audio_format format)
{
  switch (format) {
  case SPA_AUDIO_FORMAT_U8:
  case SPA_AUDIO_FORMAT_S8:
  case SPA_AUDIO_FORMAT_ULAW:
  case SPA_AUDIO_FORMAT_ALAW:
  case SPA_AUDIO_FORMAT_U8P:
  case SPA_AUDIO_FORMAT_S8P:
    return 1;
  case SPA_AUDIO_FORMAT_S16_LE:
  case SPA_AUDIO_FORMAT_S16_BE:
  case SPA_AUDIO_FORMAT_U16_LE:
  case SPA_AUDIO_FORMAT_U16_BE:
  case SPA_AUDIO_FORMAT_S16P:
    return 2;
  case SPA_AUDIO_FORMAT_S24_LE:
  case SPA_AUDIO_FORMAT_S24_BE:
  case SPA_AUDIO_FORMAT_U24_LE:
  case SPA_AUDIO_FORMAT_U24_BE:
  case SPA_AUDIO_FORMAT_S24P:
    return 3;
  case SPA_AUDIO_FORMAT_S24_32_LE:
  case SPA_AUDIO_FORMAT_S24_32_BE:
  case SPA_AUDIO_FORMAT_U24_32_LE:
  case SPA_AUDIO_FORMAT_U24_32_BE:
  case SPA_AUDIO_FORMAT_S32_LE:
  case SPA_AUDIO_FORMAT_S32_BE:
  case SPA_AUDIO_FORMAT_U32_LE:
  case SPA_AUDIO_FORMAT_U32_BE:
  case SPA_AUDIO_FORMAT_F32_LE:
  case SPA_AUDIO_FORMAT_F32_BE:
  case SPA_AUDIO_FORMAT_S24_32P:
  case SPA_AUDIO_FORMAT_S32P:
  case SPA_AUDIO_FORMAT_F32P:
    return 4;
  case SPA_AUDIO_FORMAT_F64_LE:
  case SPA_AUDIO_FORMAT_F64_BE:
  case SPA_AUDIO_FORMAT_F64P:
    return 8;
  default:
    return 0;
  }
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

static PwgParam *
pwg_param_new_props(gboolean has_volume,
                    gdouble volume,
                    gboolean has_mute,
                    gboolean mute)
{
  uint8_t buffer[PWG_PARAM_SIMPLE_PROPS_BUFFER_SIZE];
  struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  struct spa_pod_frame frame;
  struct spa_pod *pod;
  g_autoptr(GBytes) bytes = NULL;

  if (spa_pod_builder_push_object(&builder, &frame, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props) < 0)
    return NULL;

  if (has_volume) {
    if (spa_pod_builder_prop(&builder, SPA_PROP_volume, 0) < 0)
      return NULL;
    if (spa_pod_builder_float(&builder, (float) volume) < 0)
      return NULL;
  }

  if (has_mute) {
    if (spa_pod_builder_prop(&builder, SPA_PROP_mute, 0) < 0)
      return NULL;
    if (spa_pod_builder_bool(&builder, mute) < 0)
      return NULL;
  }

  pod = spa_pod_builder_pop(&builder, &frame);
  if (pod == NULL)
    return NULL;

  bytes = g_bytes_new(pod, SPA_POD_SIZE(pod));
  return _pwg_param_new(0, SPA_PARAM_Props, 0, 0, bytes);
}

PwgParam *
pwg_param_new_props_volume(gdouble volume)
{
  g_return_val_if_fail(volume >= 0.0, NULL);
  g_return_val_if_fail(volume <= G_MAXFLOAT, NULL);

  return pwg_param_new_props(TRUE, volume, FALSE, FALSE);
}

PwgParam *
pwg_param_new_props_mute(gboolean mute)
{
  return pwg_param_new_props(FALSE, 0.0, TRUE, mute);
}

PwgParam *
pwg_param_new_props_controls(GVariant *controls)
{
  g_autofree uint8_t *buffer = NULL;
  struct spa_pod_builder builder;
  struct spa_pod_frame object_frame;
  struct spa_pod_frame params_frame;
  struct spa_pod *pod;
  GVariantIter iter;
  const char *name;
  gdouble value;
  g_autoptr(GBytes) bytes = NULL;

  g_return_val_if_fail(controls != NULL, NULL);

  if (!g_variant_is_of_type(controls, G_VARIANT_TYPE("a{sd}")))
    return NULL;

  buffer = g_malloc0(PWG_PARAM_CONTROL_PROPS_BUFFER_SIZE);
  builder = SPA_POD_BUILDER_INIT(buffer, PWG_PARAM_CONTROL_PROPS_BUFFER_SIZE);

  if (spa_pod_builder_push_object(&builder, &object_frame, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props) < 0)
    return NULL;

  if (spa_pod_builder_prop(&builder, SPA_PROP_params, 0) < 0)
    return NULL;
  if (spa_pod_builder_push_struct(&builder, &params_frame) < 0)
    return NULL;

  g_variant_iter_init(&iter, controls);
  while (g_variant_iter_loop(&iter, "{&sd}", &name, &value)) {
    if (name == NULL || name[0] == '\0')
      return NULL;
    if (!isfinite(value) || value < -G_MAXFLOAT || value > G_MAXFLOAT)
      return NULL;
    if (spa_pod_builder_string(&builder, name) < 0)
      return NULL;
    if (spa_pod_builder_float(&builder, (float) value) < 0)
      return NULL;
  }

  if (spa_pod_builder_pop(&builder, &params_frame) == NULL)
    return NULL;

  pod = spa_pod_builder_pop(&builder, &object_frame);
  if (pod == NULL)
    return NULL;

  bytes = g_bytes_new(pod, SPA_POD_SIZE(pod));
  return _pwg_param_new(0, SPA_PARAM_Props, 0, 0, bytes);
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

guint
pwg_param_get_format_media_type(PwgParam *self)
{
  guint media_type = 0;

  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  if (!pwg_param_parse_format(self, &media_type, NULL))
    return 0;

  return media_type;
}

char *
pwg_param_dup_format_media_type_name(PwgParam *self)
{
  guint media_type = 0;
  const char *name;

  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  if (!pwg_param_parse_format(self, &media_type, NULL))
    return NULL;

  name = spa_debug_type_find_short_name(spa_type_media_type, media_type);
  return name != NULL ? g_strdup(name) : NULL;
}

guint
pwg_param_get_format_media_subtype(PwgParam *self)
{
  guint media_subtype = 0;

  g_return_val_if_fail(PWG_IS_PARAM(self), 0);

  if (!pwg_param_parse_format(self, NULL, &media_subtype))
    return 0;

  return media_subtype;
}

char *
pwg_param_dup_format_media_subtype_name(PwgParam *self)
{
  guint media_subtype = 0;
  const char *name;

  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  if (!pwg_param_parse_format(self, NULL, &media_subtype))
    return NULL;

  name = spa_debug_type_find_short_name(spa_type_media_subtype, media_subtype);
  return name != NULL ? g_strdup(name) : NULL;
}

PwgAudioFormat *
pwg_param_dup_audio_format(PwgParam *self)
{
  const struct spa_pod *pod;
  guint media_type = 0;
  guint media_subtype = 0;
  struct spa_audio_info_raw raw_info = {0};
  const char *sample_format;

  g_return_val_if_fail(PWG_IS_PARAM(self), NULL);

  pod = pwg_param_get_pod(self);
  if (!pwg_param_parse_format(self, &media_type, &media_subtype))
    return NULL;
  if (media_type != SPA_MEDIA_TYPE_audio || media_subtype != SPA_MEDIA_SUBTYPE_raw)
    return NULL;
  if (spa_format_audio_raw_parse(pod, &raw_info) < 0)
    return NULL;

  sample_format = pwg_param_audio_format_name(raw_info.format);
  return pwg_audio_format_new(
    sample_format != NULL ? sample_format : "unknown",
    raw_info.rate,
    raw_info.channels,
    pwg_param_audio_format_bytes_per_sample(raw_info.format));
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
