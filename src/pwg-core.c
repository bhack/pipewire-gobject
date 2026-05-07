#include "pwg-core.h"

#include <pipewire/impl-module.h>
#include <pipewire/pipewire.h>

#include "pwg-core-private.h"
#include "pwg-error.h"
#include "pwg-impl-module-private.h"
#include "pwg.h"

struct _PwgCore {
  GObject parent_instance;
  struct pw_thread_loop *thread_loop;
  struct pw_context *context;
  struct pw_core *core;
  gboolean connected;
};

G_DEFINE_TYPE(PwgCore, pwg_core, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_CONNECTED,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static void
pwg_core_get_property(GObject *object,
                      unsigned int property_id,
                      GValue *value,
                      GParamSpec *pspec)
{
  PwgCore *self = PWG_CORE(object);

  switch (property_id) {
  case PROP_CONNECTED:
    g_value_set_boolean(value, self->connected);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_core_dispose(GObject *object)
{
  PwgCore *self = PWG_CORE(object);

  pwg_core_disconnect(self);

  G_OBJECT_CLASS(pwg_core_parent_class)->dispose(object);
}

static void
pwg_core_class_init(PwgCoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_core_get_property;
  object_class->dispose = pwg_core_dispose;

  /**
   * PwgCore:connected:
   *
   * Whether this object has an active PipeWire core connection.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_CONNECTED] = g_param_spec_boolean(
    "connected",
    "Connected",
    "Whether this object has an active PipeWire core connection.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_core_init(PwgCore *self)
{
  (void) self;
  pwg_init();
}

PwgCore *
pwg_core_new(void)
{
  return g_object_new(PWG_TYPE_CORE, NULL);
}

bool
pwg_core_connect(PwgCore *self, GError **error)
{
  g_return_val_if_fail(PWG_IS_CORE(self), FALSE);

  if (self->connected)
    return TRUE;

  self->thread_loop = pw_thread_loop_new("pwg-core", NULL);
  if (self->thread_loop == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not create PipeWire thread loop");
    return FALSE;
  }

  self->context = pw_context_new(pw_thread_loop_get_loop(self->thread_loop), NULL, 0);
  if (self->context == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not create PipeWire context");
    pwg_core_disconnect(self);
    return FALSE;
  }

  self->core = pw_context_connect(self->context, NULL, 0);
  if (self->core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not connect to PipeWire core");
    pwg_core_disconnect(self);
    return FALSE;
  }

  if (pw_thread_loop_start(self->thread_loop) < 0) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not start PipeWire thread loop");
    pwg_core_disconnect(self);
    return FALSE;
  }

  self->connected = TRUE;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_CONNECTED]);
  return TRUE;
}

void
pwg_core_disconnect(PwgCore *self)
{
  gboolean was_connected;

  g_return_if_fail(PWG_IS_CORE(self));

  was_connected = self->connected;
  self->connected = FALSE;

  if (self->thread_loop != NULL)
    pw_thread_loop_stop(self->thread_loop);

  if (self->core != NULL) {
    pw_core_disconnect(self->core);
    self->core = NULL;
  }

  if (self->context != NULL) {
    pw_context_destroy(self->context);
    self->context = NULL;
  }

  if (self->thread_loop != NULL) {
    pw_thread_loop_destroy(self->thread_loop);
    self->thread_loop = NULL;
  }

  if (was_connected)
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_CONNECTED]);
}

bool
pwg_core_get_connected(PwgCore *self)
{
  g_return_val_if_fail(PWG_IS_CORE(self), FALSE);

  return self->connected;
}

const char *
pwg_core_get_library_version(PwgCore *self)
{
  g_return_val_if_fail(PWG_IS_CORE(self), NULL);

  return pwg_get_pipewire_library_version();
}

PwgImplModule *
pwg_core_load_module(PwgCore *self,
                     const char *name,
                     const char *arguments,
                     GError **error)
{
  struct pw_impl_module *module;

  g_return_val_if_fail(PWG_IS_CORE(self), NULL);
  g_return_val_if_fail(name != NULL, NULL);

  if (name[0] == '\0') {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "PipeWire module name is empty");
    return NULL;
  }

  if (!self->connected && !pwg_core_connect(self, error))
    return NULL;

  if (self->thread_loop == NULL || self->context == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return NULL;
  }

  pw_thread_loop_lock(self->thread_loop);
  module = pw_context_load_module(self->context, name, arguments, NULL);
  pw_thread_loop_unlock(self->thread_loop);

  if (module == NULL) {
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not load PipeWire module '%s'",
      name);
    return NULL;
  }

  return _pwg_impl_module_new(self, name, arguments, module);
}

struct pw_core *
pwg_core_get_pw_core_internal(PwgCore *self)
{
  g_return_val_if_fail(PWG_IS_CORE(self), NULL);

  return self->core;
}

struct pw_thread_loop *
pwg_core_get_thread_loop_internal(PwgCore *self)
{
  g_return_val_if_fail(PWG_IS_CORE(self), NULL);

  return self->thread_loop;
}
