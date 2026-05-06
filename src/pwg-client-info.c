#include "pwg-client-info.h"

#include <pipewire/keys.h>

struct _PwgClientInfo {
  GObject parent_instance;
  PwgGlobal *global;
};

G_DEFINE_TYPE(PwgClientInfo, pwg_client_info, G_TYPE_OBJECT)

static void
pwg_client_info_dispose(GObject *object)
{
  PwgClientInfo *self = PWG_CLIENT_INFO(object);

  g_clear_object(&self->global);

  G_OBJECT_CLASS(pwg_client_info_parent_class)->dispose(object);
}

static void
pwg_client_info_class_init(PwgClientInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = pwg_client_info_dispose;
}

static void
pwg_client_info_init(PwgClientInfo *self)
{
  (void) self;
}

PwgClientInfo *
pwg_client_info_new_from_global(PwgGlobal *global)
{
  PwgClientInfo *self;

  g_return_val_if_fail(PWG_IS_GLOBAL(global), NULL);

  if (!pwg_global_is_client(global))
    return NULL;

  self = g_object_new(PWG_TYPE_CLIENT_INFO, NULL);
  self->global = g_object_ref(global);
  return self;
}

PwgGlobal *
pwg_client_info_get_global(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return self->global;
}

guint
pwg_client_info_get_id(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), 0);

  return pwg_global_get_id(self->global);
}

char *
pwg_client_info_dup_name(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_CLIENT_NAME);
}

char *
pwg_client_info_dup_app_name(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_APP_NAME);
}

char *
pwg_client_info_dup_app_id(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_APP_ID);
}

char *
pwg_client_info_dup_api(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_CLIENT_API);
}

char *
pwg_client_info_dup_access(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_CLIENT_ACCESS);
}

char *
pwg_client_info_dup_process_binary(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_APP_PROCESS_BINARY);
}

char *
pwg_client_info_dup_process_id(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_APP_PROCESS_ID);
}

char *
pwg_client_info_dup_object_serial(PwgClientInfo *self)
{
  g_return_val_if_fail(PWG_IS_CLIENT_INFO(self), NULL);

  return pwg_global_dup_object_serial(self->global);
}
