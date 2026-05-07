#include "pwg-impl-module.h"

#include <pipewire/impl.h>

#include "pwg-core-private.h"
#include "pwg-impl-module-private.h"

struct _PwgImplModule {
  GObject parent_instance;
  PwgCore *core;
  char *name;
  char *arguments;
  gboolean loaded;
  struct pw_impl_module *module;
  struct spa_hook module_listener;
  gboolean has_module_listener;
};

G_DEFINE_TYPE(PwgImplModule, pwg_impl_module, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_CORE,
  PROP_NAME,
  PROP_ARGUMENTS,
  PROP_LOADED,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static void
pwg_impl_module_on_free(void *userdata)
{
  PwgImplModule *self = PWG_IMPL_MODULE(userdata);

  self->module = NULL;
  self->has_module_listener = FALSE;
  if (self->loaded) {
    self->loaded = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_LOADED]);
  }
}

static const struct pw_impl_module_events module_events = {
  PW_VERSION_IMPL_MODULE_EVENTS,
  .free = pwg_impl_module_on_free,
};

static void
pwg_impl_module_get_property(GObject *object,
                             unsigned int property_id,
                             GValue *value,
                             GParamSpec *pspec)
{
  PwgImplModule *self = PWG_IMPL_MODULE(object);

  switch (property_id) {
  case PROP_CORE:
    g_value_set_object(value, self->core);
    break;
  case PROP_NAME:
    g_value_set_string(value, self->name);
    break;
  case PROP_ARGUMENTS:
    g_value_set_string(value, self->arguments);
    break;
  case PROP_LOADED:
    g_value_set_boolean(value, self->loaded);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_impl_module_set_property(GObject *object,
                             unsigned int property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  PwgImplModule *self = PWG_IMPL_MODULE(object);

  switch (property_id) {
  case PROP_CORE:
    g_clear_object(&self->core);
    self->core = g_value_dup_object(value);
    break;
  case PROP_NAME:
    g_free(self->name);
    self->name = g_value_dup_string(value);
    break;
  case PROP_ARGUMENTS:
    g_free(self->arguments);
    self->arguments = g_value_dup_string(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_impl_module_dispose(GObject *object)
{
  PwgImplModule *self = PWG_IMPL_MODULE(object);

  pwg_impl_module_unload(self);
  g_clear_object(&self->core);

  G_OBJECT_CLASS(pwg_impl_module_parent_class)->dispose(object);
}

static void
pwg_impl_module_finalize(GObject *object)
{
  PwgImplModule *self = PWG_IMPL_MODULE(object);

  g_free(self->name);
  g_free(self->arguments);

  G_OBJECT_CLASS(pwg_impl_module_parent_class)->finalize(object);
}

static void
pwg_impl_module_class_init(PwgImplModuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_impl_module_get_property;
  object_class->set_property = pwg_impl_module_set_property;
  object_class->dispose = pwg_impl_module_dispose;
  object_class->finalize = pwg_impl_module_finalize;

  /**
   * PwgImplModule:core:
   *
   * Core that owns this loaded PipeWire implementation module.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_CORE] = g_param_spec_object(
    "core",
    "Core",
    "Core that owns this loaded PipeWire implementation module.",
    PWG_TYPE_CORE,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgImplModule:name:
   *
   * PipeWire module name.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_NAME] = g_param_spec_string(
    "name",
    "Name",
    "PipeWire module name.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgImplModule:arguments:
   *
   * PipeWire module argument string.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_ARGUMENTS] = g_param_spec_string(
    "arguments",
    "Arguments",
    "PipeWire module argument string.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgImplModule:loaded:
   *
   * Whether this handle still has a loaded PipeWire implementation module.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_LOADED] = g_param_spec_boolean(
    "loaded",
    "Loaded",
    "Whether this handle still has a loaded PipeWire implementation module.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_impl_module_init(PwgImplModule *self)
{
  (void) self;
}

PwgImplModule *
_pwg_impl_module_new(PwgCore *core,
                     const char *name,
                     const char *arguments,
                     struct pw_impl_module *module)
{
  PwgImplModule *self;

  g_return_val_if_fail(PWG_IS_CORE(core), NULL);
  g_return_val_if_fail(name != NULL, NULL);
  g_return_val_if_fail(module != NULL, NULL);

  self = g_object_new(
    PWG_TYPE_IMPL_MODULE,
    "core", core,
    "name", name,
    "arguments", arguments,
    NULL);
  self->module = module;
  self->loaded = TRUE;

  spa_zero(self->module_listener);
  pw_impl_module_add_listener(self->module, &self->module_listener, &module_events, self);
  self->has_module_listener = TRUE;

  return self;
}

PwgCore *
pwg_impl_module_get_core(PwgImplModule *self)
{
  g_return_val_if_fail(PWG_IS_IMPL_MODULE(self), NULL);

  return self->core;
}

const char *
pwg_impl_module_get_name(PwgImplModule *self)
{
  g_return_val_if_fail(PWG_IS_IMPL_MODULE(self), NULL);

  return self->name;
}

const char *
pwg_impl_module_get_arguments(PwgImplModule *self)
{
  g_return_val_if_fail(PWG_IS_IMPL_MODULE(self), NULL);

  return self->arguments;
}

bool
pwg_impl_module_get_loaded(PwgImplModule *self)
{
  g_return_val_if_fail(PWG_IS_IMPL_MODULE(self), FALSE);

  return self->loaded;
}

void
pwg_impl_module_unload(PwgImplModule *self)
{
  struct pw_impl_module *module;
  struct pw_thread_loop *thread_loop;
  gboolean was_loaded;

  g_return_if_fail(PWG_IS_IMPL_MODULE(self));

  if (self->module == NULL)
    return;

  module = self->module;
  self->module = NULL;
  was_loaded = self->loaded;
  self->loaded = FALSE;

  if (self->has_module_listener) {
    spa_hook_remove(&self->module_listener);
    self->has_module_listener = FALSE;
  }

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop != NULL)
    pw_thread_loop_lock(thread_loop);
  pw_impl_module_destroy(module);
  if (thread_loop != NULL)
    pw_thread_loop_unlock(thread_loop);

  if (was_loaded)
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_LOADED]);
}
