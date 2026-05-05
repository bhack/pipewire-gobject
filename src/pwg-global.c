#include "pwg-global.h"

#include "pwg-global-private.h"

struct _PwgGlobal {
  GObject parent_instance;
  guint id;
  guint permissions;
  char *interface_type;
  guint version;
  GVariant *properties;
};

G_DEFINE_TYPE(PwgGlobal, pwg_global, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_ID,
  PROP_PERMISSIONS,
  PROP_INTERFACE_TYPE,
  PROP_VERSION,
  PROP_PROPERTIES,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static GVariant *
pwg_global_empty_properties(void)
{
  GVariantBuilder builder;

  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{ss}"));
  return g_variant_ref_sink(g_variant_builder_end(&builder));
}

static GVariant *
pwg_global_variant_from_spa_dict(const struct spa_dict *dict)
{
  GVariantBuilder builder;
  const struct spa_dict_item *item;

  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{ss}"));
  if (dict != NULL) {
    spa_dict_for_each(item, dict) {
      if (item->key != NULL && item->value != NULL)
        g_variant_builder_add(&builder, "{ss}", item->key, item->value);
    }
  }

  return g_variant_ref_sink(g_variant_builder_end(&builder));
}

static void
pwg_global_get_property(GObject *object,
                        guint property_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  PwgGlobal *self = PWG_GLOBAL(object);

  switch (property_id) {
  case PROP_ID:
    g_value_set_uint(value, self->id);
    break;
  case PROP_PERMISSIONS:
    g_value_set_uint(value, self->permissions);
    break;
  case PROP_INTERFACE_TYPE:
    g_value_set_string(value, self->interface_type);
    break;
  case PROP_VERSION:
    g_value_set_uint(value, self->version);
    break;
  case PROP_PROPERTIES:
    g_value_set_variant(value, self->properties);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_global_set_property(GObject *object,
                        guint property_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  PwgGlobal *self = PWG_GLOBAL(object);

  switch (property_id) {
  case PROP_ID:
    self->id = g_value_get_uint(value);
    break;
  case PROP_PERMISSIONS:
    self->permissions = g_value_get_uint(value);
    break;
  case PROP_INTERFACE_TYPE:
    g_clear_pointer(&self->interface_type, g_free);
    self->interface_type = g_value_dup_string(value);
    break;
  case PROP_VERSION:
    self->version = g_value_get_uint(value);
    break;
  case PROP_PROPERTIES:
    g_clear_pointer(&self->properties, g_variant_unref);
    self->properties = g_value_dup_variant(value);
    if (self->properties == NULL)
      self->properties = pwg_global_empty_properties();
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_global_finalize(GObject *object)
{
  PwgGlobal *self = PWG_GLOBAL(object);

  g_clear_pointer(&self->interface_type, g_free);
  g_clear_pointer(&self->properties, g_variant_unref);

  G_OBJECT_CLASS(pwg_global_parent_class)->finalize(object);
}

static void
pwg_global_class_init(PwgGlobalClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_global_get_property;
  object_class->set_property = pwg_global_set_property;
  object_class->finalize = pwg_global_finalize;

  properties[PROP_ID] = g_param_spec_uint(
    "id",
    "Id",
    "PipeWire global id.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  properties[PROP_PERMISSIONS] = g_param_spec_uint(
    "permissions",
    "Permissions",
    "PipeWire permissions bitmask visible to this client.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  properties[PROP_INTERFACE_TYPE] = g_param_spec_string(
    "interface-type",
    "Interface type",
    "PipeWire interface type name.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  properties[PROP_VERSION] = g_param_spec_uint(
    "version",
    "Version",
    "PipeWire interface version.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  properties[PROP_PROPERTIES] = g_param_spec_variant(
    "properties",
    "Properties",
    "PipeWire global properties as an a{ss} variant.",
    G_VARIANT_TYPE("a{ss}"),
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_global_init(PwgGlobal *self)
{
  self->properties = pwg_global_empty_properties();
}

PwgGlobal *
_pwg_global_new_from_spa_dict(guint id,
                              guint permissions,
                              const char *interface_type,
                              guint version,
                              const struct spa_dict *properties)
{
  g_autoptr(GVariant) variant = pwg_global_variant_from_spa_dict(properties);

  return g_object_new(
    PWG_TYPE_GLOBAL,
    "id", id,
    "permissions", permissions,
    "interface-type", interface_type,
    "version", version,
    "properties", variant,
    NULL);
}

guint
pwg_global_get_id(PwgGlobal *self)
{
  g_return_val_if_fail(PWG_IS_GLOBAL(self), 0);

  return self->id;
}

guint
pwg_global_get_permissions(PwgGlobal *self)
{
  g_return_val_if_fail(PWG_IS_GLOBAL(self), 0);

  return self->permissions;
}

const char *
pwg_global_get_interface_type(PwgGlobal *self)
{
  g_return_val_if_fail(PWG_IS_GLOBAL(self), NULL);

  return self->interface_type;
}

guint
pwg_global_get_version(PwgGlobal *self)
{
  g_return_val_if_fail(PWG_IS_GLOBAL(self), 0);

  return self->version;
}

GVariant *
pwg_global_get_properties(PwgGlobal *self)
{
  g_return_val_if_fail(PWG_IS_GLOBAL(self), NULL);

  return g_variant_ref(self->properties);
}

char *
pwg_global_dup_property(PwgGlobal *self, const char *key)
{
  const char *value = NULL;

  g_return_val_if_fail(PWG_IS_GLOBAL(self), NULL);
  g_return_val_if_fail(key != NULL, NULL);

  if (g_variant_lookup(self->properties, key, "&s", &value))
    return g_strdup(value);

  return NULL;
}
