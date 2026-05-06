#include "pwg-port-info.h"

#include <pipewire/keys.h>

struct _PwgPortInfo {
  GObject parent_instance;
  PwgGlobal *global;
};

G_DEFINE_TYPE(PwgPortInfo, pwg_port_info, G_TYPE_OBJECT)

static gboolean
pwg_port_info_property_is_true(PwgPortInfo *self, const char *key)
{
  g_autofree char *value = pwg_global_dup_property(self->global, key);

  return value != NULL &&
         (g_strcmp0(value, "1") == 0 ||
          g_ascii_strcasecmp(value, "true") == 0 ||
          g_ascii_strcasecmp(value, "yes") == 0);
}

static void
pwg_port_info_dispose(GObject *object)
{
  PwgPortInfo *self = PWG_PORT_INFO(object);

  g_clear_object(&self->global);

  G_OBJECT_CLASS(pwg_port_info_parent_class)->dispose(object);
}

static void
pwg_port_info_class_init(PwgPortInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = pwg_port_info_dispose;
}

static void
pwg_port_info_init(PwgPortInfo *self)
{
  (void) self;
}

PwgPortInfo *
pwg_port_info_new_from_global(PwgGlobal *global)
{
  PwgPortInfo *self;

  g_return_val_if_fail(PWG_IS_GLOBAL(global), NULL);

  if (!pwg_global_is_port(global))
    return NULL;

  self = g_object_new(PWG_TYPE_PORT_INFO, NULL);
  self->global = g_object_ref(global);
  return self;
}

PwgGlobal *
pwg_port_info_get_global(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return self->global;
}

guint
pwg_port_info_get_id(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), 0);

  return pwg_global_get_id(self->global);
}

char *
pwg_port_info_dup_name(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_PORT_NAME);
}

char *
pwg_port_info_dup_description(PwgPortInfo *self)
{
  g_autofree char *alias = NULL;

  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  alias = pwg_port_info_dup_alias(self);
  if (alias != NULL && alias[0] != '\0')
    return g_steal_pointer(&alias);

  return pwg_global_dup_description(self->global);
}

char *
pwg_port_info_dup_alias(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_PORT_ALIAS);
}

char *
pwg_port_info_dup_direction(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_PORT_DIRECTION);
}

gboolean
pwg_port_info_get_physical(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), FALSE);

  return pwg_port_info_property_is_true(self, PW_KEY_PORT_PHYSICAL);
}

char *
pwg_port_info_dup_audio_channel(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_AUDIO_CHANNEL);
}

char *
pwg_port_info_dup_media_class(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_media_class(self->global);
}

char *
pwg_port_info_dup_object_serial(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_object_serial(self->global);
}
