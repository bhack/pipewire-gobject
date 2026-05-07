#include "pwg-link-info.h"

#include <pipewire/keys.h>

struct _PwgLinkInfo {
  GObject parent_instance;
  PwgGlobal *global;
};

G_DEFINE_TYPE(PwgLinkInfo, pwg_link_info, G_TYPE_OBJECT)

static gboolean
pwg_link_info_property_is_true(PwgLinkInfo *self, const char *key)
{
  g_autofree char *value = pwg_global_dup_property(self->global, key);

  return value != NULL &&
         (g_strcmp0(value, "1") == 0 ||
          g_ascii_strcasecmp(value, "true") == 0 ||
          g_ascii_strcasecmp(value, "yes") == 0);
}

static void
pwg_link_info_dispose(GObject *object)
{
  PwgLinkInfo *self = PWG_LINK_INFO(object);

  g_clear_object(&self->global);

  G_OBJECT_CLASS(pwg_link_info_parent_class)->dispose(object);
}

static void
pwg_link_info_class_init(PwgLinkInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = pwg_link_info_dispose;
}

static void
pwg_link_info_init(PwgLinkInfo *self)
{
  (void) self;
}

PwgLinkInfo *
pwg_link_info_new_from_global(PwgGlobal *global)
{
  PwgLinkInfo *self;

  g_return_val_if_fail(PWG_IS_GLOBAL(global), NULL);

  if (!pwg_global_is_link(global))
    return NULL;

  self = g_object_new(PWG_TYPE_LINK_INFO, NULL);
  self->global = g_object_ref(global);
  return self;
}

PwgGlobal *
pwg_link_info_get_global(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), NULL);

  return self->global;
}

unsigned int
pwg_link_info_get_id(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), 0);

  return pwg_global_get_id(self->global);
}

char *
pwg_link_info_dup_link_id(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_LINK_ID);
}

char *
pwg_link_info_dup_output_node_id(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_LINK_OUTPUT_NODE);
}

char *
pwg_link_info_dup_output_port_id(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_LINK_OUTPUT_PORT);
}

char *
pwg_link_info_dup_input_node_id(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_LINK_INPUT_NODE);
}

char *
pwg_link_info_dup_input_port_id(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_LINK_INPUT_PORT);
}

bool
pwg_link_info_get_passive(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), FALSE);

  return pwg_link_info_property_is_true(self, PW_KEY_LINK_PASSIVE);
}

bool
pwg_link_info_get_feedback(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), FALSE);

  return pwg_link_info_property_is_true(self, PW_KEY_LINK_FEEDBACK);
}

char *
pwg_link_info_dup_object_serial(PwgLinkInfo *self)
{
  g_return_val_if_fail(PWG_IS_LINK_INFO(self), NULL);

  return pwg_global_dup_object_serial(self->global);
}
