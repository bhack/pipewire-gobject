#include "pwg-metadata-info.h"

#include <pipewire/extensions/metadata.h>
#include <pipewire/keys.h>

struct _PwgMetadataInfo {
  GObject parent_instance;
  PwgGlobal *global;
};

G_DEFINE_TYPE(PwgMetadataInfo, pwg_metadata_info, G_TYPE_OBJECT)

static void
pwg_metadata_info_dispose(GObject *object)
{
  PwgMetadataInfo *self = PWG_METADATA_INFO(object);

  g_clear_object(&self->global);

  G_OBJECT_CLASS(pwg_metadata_info_parent_class)->dispose(object);
}

static void
pwg_metadata_info_class_init(PwgMetadataInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = pwg_metadata_info_dispose;
}

static void
pwg_metadata_info_init(PwgMetadataInfo *self)
{
  (void) self;
}

PwgMetadataInfo *
pwg_metadata_info_new_from_global(PwgGlobal *global)
{
  PwgMetadataInfo *self;

  g_return_val_if_fail(PWG_IS_GLOBAL(global), NULL);

  if (!pwg_global_is_metadata(global))
    return NULL;

  self = g_object_new(PWG_TYPE_METADATA_INFO, NULL);
  self->global = g_object_ref(global);
  return self;
}

PwgGlobal *
pwg_metadata_info_get_global(PwgMetadataInfo *self)
{
  g_return_val_if_fail(PWG_IS_METADATA_INFO(self), NULL);

  return self->global;
}

guint
pwg_metadata_info_get_id(PwgMetadataInfo *self)
{
  g_return_val_if_fail(PWG_IS_METADATA_INFO(self), 0);

  return pwg_global_get_id(self->global);
}

char *
pwg_metadata_info_dup_name(PwgMetadataInfo *self)
{
  g_return_val_if_fail(PWG_IS_METADATA_INFO(self), NULL);

  return pwg_global_dup_property(self->global, PW_KEY_METADATA_NAME);
}

char *
pwg_metadata_info_dup_object_serial(PwgMetadataInfo *self)
{
  g_return_val_if_fail(PWG_IS_METADATA_INFO(self), NULL);

  return pwg_global_dup_object_serial(self->global);
}
