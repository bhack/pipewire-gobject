#include "pwg-port-info.h"

#include <pipewire/keys.h>

struct _PwgPortInfo {
  GObject parent_instance;
  PwgGlobal *global;
};

G_DEFINE_TYPE(PwgPortInfo, pwg_port_info, G_TYPE_OBJECT)

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
pwg_port_info_dup_port_id(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_PORT_ID);
}

char *
pwg_port_info_dup_node_id(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_NODE_ID);
}

char *
pwg_port_info_dup_name(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_PORT_NAME);
}

char *
pwg_port_info_dup_direction(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_PORT_DIRECTION);
}

char *
pwg_port_info_dup_alias(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_PORT_ALIAS);
}

char *
pwg_port_info_dup_object_serial(PwgPortInfo *self)
{
  g_return_val_if_fail(PWG_IS_PORT_INFO(self), NULL);

  return pwg_global_dup_object_serial(self->global);
}
