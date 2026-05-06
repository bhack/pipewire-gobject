#include "pwg-node-info.h"

#include <pipewire/keys.h>

struct _PwgNodeInfo {
  GObject parent_instance;
  PwgGlobal *global;
};

G_DEFINE_TYPE(PwgNodeInfo, pwg_node_info, G_TYPE_OBJECT)

static void
pwg_node_info_dispose(GObject *object)
{
  PwgNodeInfo *self = PWG_NODE_INFO(object);

  g_clear_object(&self->global);

  G_OBJECT_CLASS(pwg_node_info_parent_class)->dispose(object);
}

static void
pwg_node_info_class_init(PwgNodeInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = pwg_node_info_dispose;
}

static void
pwg_node_info_init(PwgNodeInfo *self)
{
  (void) self;
}

PwgNodeInfo *
pwg_node_info_new_from_global(PwgGlobal *global)
{
  PwgNodeInfo *self;

  g_return_val_if_fail(PWG_IS_GLOBAL(global), NULL);

  if (!pwg_global_is_node(global))
    return NULL;

  self = g_object_new(PWG_TYPE_NODE_INFO, NULL);
  self->global = g_object_ref(global);
  return self;
}

PwgGlobal *
pwg_node_info_get_global(PwgNodeInfo *self)
{
  g_return_val_if_fail(PWG_IS_NODE_INFO(self), NULL);

  return self->global;
}

guint
pwg_node_info_get_id(PwgNodeInfo *self)
{
  g_return_val_if_fail(PWG_IS_NODE_INFO(self), 0);

  return pwg_global_get_id(self->global);
}

char *
pwg_node_info_dup_name(PwgNodeInfo *self)
{
  g_return_val_if_fail(PWG_IS_NODE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_NODE_NAME);
}

char *
pwg_node_info_dup_description(PwgNodeInfo *self)
{
  g_return_val_if_fail(PWG_IS_NODE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_NODE_DESCRIPTION);
}

char *
pwg_node_info_dup_media_class(PwgNodeInfo *self)
{
  g_return_val_if_fail(PWG_IS_NODE_INFO(self), NULL);

  return pwg_global_dup_media_class(self->global);
}

char *
pwg_node_info_dup_object_serial(PwgNodeInfo *self)
{
  g_return_val_if_fail(PWG_IS_NODE_INFO(self), NULL);

  return pwg_global_dup_object_serial(self->global);
}
