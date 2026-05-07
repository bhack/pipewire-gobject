#include "pwg-device-info.h"

#include <pipewire/keys.h>

struct _PwgDeviceInfo {
  GObject parent_instance;
  PwgGlobal *global;
};

G_DEFINE_TYPE(PwgDeviceInfo, pwg_device_info, G_TYPE_OBJECT)

static void
pwg_device_info_dispose(GObject *object)
{
  PwgDeviceInfo *self = PWG_DEVICE_INFO(object);

  g_clear_object(&self->global);

  G_OBJECT_CLASS(pwg_device_info_parent_class)->dispose(object);
}

static void
pwg_device_info_class_init(PwgDeviceInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = pwg_device_info_dispose;
}

static void
pwg_device_info_init(PwgDeviceInfo *self)
{
  (void) self;
}

PwgDeviceInfo *
pwg_device_info_new_from_global(PwgGlobal *global)
{
  PwgDeviceInfo *self;

  g_return_val_if_fail(PWG_IS_GLOBAL(global), NULL);

  if (!pwg_global_is_device(global))
    return NULL;

  self = g_object_new(PWG_TYPE_DEVICE_INFO, NULL);
  self->global = g_object_ref(global);
  return self;
}

PwgGlobal *
pwg_device_info_get_global(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return self->global;
}

unsigned int
pwg_device_info_get_id(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), 0);

  return pwg_global_get_id(self->global);
}

char *
pwg_device_info_dup_name(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_NAME);
}

char *
pwg_device_info_dup_description(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_DESCRIPTION);
}

char *
pwg_device_info_dup_nick(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_NICK);
}

char *
pwg_device_info_dup_api(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_API);
}

char *
pwg_device_info_dup_class(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_CLASS);
}

char *
pwg_device_info_dup_form_factor(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_FORM_FACTOR);
}

char *
pwg_device_info_dup_bus(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_BUS);
}

char *
pwg_device_info_dup_vendor_name(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_VENDOR_NAME);
}

char *
pwg_device_info_dup_product_name(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_DEVICE_PRODUCT_NAME);
}

char *
pwg_device_info_dup_media_class(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_media_class(self->global);
}

char *
pwg_device_info_dup_object_serial(PwgDeviceInfo *self)
{
  g_return_val_if_fail(PWG_IS_DEVICE_INFO(self), NULL);

  return pwg_global_dup_object_serial(self->global);
}
