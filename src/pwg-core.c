#include "pwg-core.h"

#include <errno.h>
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
  GHashTable *pipewire_properties;
  gboolean connected;
};

G_DEFINE_TYPE(PwgCore, pwg_core, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_CONNECTED,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

typedef struct {
  struct pw_thread_loop *thread_loop;
  GMainContext *main_context;
  int seq;
  bool done;
  int main_context_done;
  int error_result;
  char *error_message;
} PwgCoreSyncData;

static gboolean
pwg_core_sync_on_main_context_done(gpointer userdata)
{
  PwgCoreSyncData *data = userdata;

  g_atomic_int_set(&data->main_context_done, 1);
  return false;
}

static void
pwg_core_sync_queue_main_context_done(PwgCoreSyncData *data)
{
  GSource *source;

  if (data->main_context == NULL)
    return;

  source = g_idle_source_new();
  g_source_set_priority(source, G_PRIORITY_DEFAULT);
  g_source_set_callback(source, pwg_core_sync_on_main_context_done, data, NULL);
  g_source_attach(source, data->main_context);
  g_source_unref(source);
}

static void
pwg_core_sync_on_done(void *userdata, uint32_t id, int seq)
{
  PwgCoreSyncData *data = userdata;

  if (id != PW_ID_CORE || seq != data->seq)
    return;

  data->done = true;
  pwg_core_sync_queue_main_context_done(data);
  pw_thread_loop_signal(data->thread_loop, false);
}

static void
pwg_core_sync_on_error(void *userdata, uint32_t id, int seq, int res, const char *message)
{
  PwgCoreSyncData *data = userdata;

  if (id != PW_ID_CORE || seq != data->seq)
    return;

  data->error_result = res < 0 ? res : -EIO;
  g_free(data->error_message);
  data->error_message = g_strdup(message);
  pw_thread_loop_signal(data->thread_loop, false);
}

static const struct pw_core_events pwg_core_sync_events = {
  PW_VERSION_CORE_EVENTS,
  .done = pwg_core_sync_on_done,
  .error = pwg_core_sync_on_error,
};

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
pwg_core_finalize(GObject *object)
{
  PwgCore *self = PWG_CORE(object);

  g_clear_pointer(&self->pipewire_properties, g_hash_table_unref);

  G_OBJECT_CLASS(pwg_core_parent_class)->finalize(object);
}

static void
pwg_core_class_init(PwgCoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_core_get_property;
  object_class->dispose = pwg_core_dispose;
  object_class->finalize = pwg_core_finalize;

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
  pwg_init();
  self->pipewire_properties = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}

PwgCore *
pwg_core_new(void)
{
  return g_object_new(PWG_TYPE_CORE, NULL);
}

bool
pwg_core_connect(PwgCore *self, GError **error)
{
  struct pw_properties *connect_properties = NULL;
  GHashTableIter iter;
  void *key;
  void *value;

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

  if (g_hash_table_size(self->pipewire_properties) > 0) {
    connect_properties = pw_properties_new(NULL, NULL);
    if (connect_properties == NULL) {
      g_set_error_literal(error, PWG_ERROR, PWG_ERROR_NO_MEMORY, "Could not allocate PipeWire properties");
      pwg_core_disconnect(self);
      return FALSE;
    }

    g_hash_table_iter_init(&iter, self->pipewire_properties);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
      if (pw_properties_set(connect_properties, key, value) < 0) {
        pw_properties_free(connect_properties);
        g_set_error_literal(error, PWG_ERROR, PWG_ERROR_NO_MEMORY, "Could not copy PipeWire properties");
        pwg_core_disconnect(self);
        return FALSE;
      }
    }
  }

  self->core = pw_context_connect(self->context, connect_properties, 0);
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

bool
pwg_core_set_pipewire_property(PwgCore *self,
                               const char *key,
                               const char *value,
                               GError **error)
{
  g_return_val_if_fail(PWG_IS_CORE(self), FALSE);

  if (key == NULL || key[0] == '\0') {
    g_set_error_literal(
      error,
      PWG_ERROR,
      PWG_ERROR_FAILED,
      "PipeWire core property key must not be empty");
    return FALSE;
  }

  if (self->connected) {
    g_set_error_literal(
      error,
      PWG_ERROR,
      PWG_ERROR_FAILED,
      "PipeWire core properties cannot be changed after connecting");
    return FALSE;
  }

  if (value == NULL) {
    g_hash_table_remove(self->pipewire_properties, key);
    return TRUE;
  }

  g_hash_table_replace(self->pipewire_properties, g_strdup(key), g_strdup(value));
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

bool
pwg_core_sync(PwgCore *self, unsigned int timeout_msec, GError **error)
{
  return pwg_core_sync_main_context_internal(self, NULL, timeout_msec, error);
}

bool
pwg_core_sync_main_context_internal(PwgCore *self,
                                    GMainContext *main_context,
                                    unsigned int timeout_msec,
                                    GError **error)
{
  PwgCoreSyncData data = { 0 };
  struct spa_hook listener;
  struct timespec abstime = { 0 };
  int result;
  int wait_result = 0;

  g_return_val_if_fail(PWG_IS_CORE(self), false);

  if (!self->connected && !pwg_core_connect(self, error))
    return false;

  if (self->thread_loop == NULL || self->core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return false;
  }

  if (pw_thread_loop_in_thread(self->thread_loop)) {
    g_set_error_literal(
      error,
      PWG_ERROR,
      PWG_ERROR_FAILED,
      "PipeWire core sync cannot run from the PipeWire thread loop");
    return false;
  }

  if (timeout_msec > 0) {
    result = pw_thread_loop_get_time(
      self->thread_loop,
      &abstime,
      (int64_t) timeout_msec * SPA_NSEC_PER_MSEC);
    if (result < 0) {
      g_set_error(
        error,
        PWG_ERROR,
        PWG_ERROR_PIPEWIRE,
        "Could not calculate PipeWire core sync timeout: %s",
        g_strerror(-result));
      return false;
    }
  }

  data.thread_loop = self->thread_loop;
  if (main_context != NULL)
    data.main_context = g_main_context_ref(main_context);

  pw_thread_loop_lock(self->thread_loop);

  spa_zero(listener);
  result = pw_core_add_listener(self->core, &listener, &pwg_core_sync_events, &data);
  if (result < 0) {
    pw_thread_loop_unlock(self->thread_loop);
    g_clear_pointer(&data.main_context, g_main_context_unref);
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not listen for PipeWire core sync completion: %s",
      g_strerror(-result));
    return false;
  }

  data.seq = pw_core_sync(self->core, PW_ID_CORE, 0);
  if (data.seq < 0) {
    spa_hook_remove(&listener);
    pw_thread_loop_unlock(self->thread_loop);
    g_clear_pointer(&data.main_context, g_main_context_unref);
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not send PipeWire core sync request: %s",
      g_strerror(-data.seq));
    return false;
  }

  while (!data.done && data.error_result == 0) {
    if (timeout_msec == 0) {
      pw_thread_loop_wait(self->thread_loop);
    } else {
      wait_result = pw_thread_loop_timed_wait_full(self->thread_loop, &abstime);
      if (wait_result < 0)
        break;
    }
  }

  spa_hook_remove(&listener);
  pw_thread_loop_unlock(self->thread_loop);

  if (data.error_result < 0) {
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "PipeWire core sync failed: %s%s%s",
      g_strerror(-data.error_result),
      data.error_message != NULL ? ": " : "",
      data.error_message != NULL ? data.error_message : "");
    g_free(data.error_message);
    g_clear_pointer(&data.main_context, g_main_context_unref);
    return false;
  }

  g_free(data.error_message);

  if (!data.done) {
    if (wait_result == -ETIMEDOUT) {
      g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core sync timed out");
    } else if (wait_result < 0) {
      g_set_error(
        error,
        PWG_ERROR,
        PWG_ERROR_PIPEWIRE,
        "PipeWire core sync wait failed: %s",
        g_strerror(-wait_result));
    } else {
      g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core sync did not complete");
    }
    g_clear_pointer(&data.main_context, g_main_context_unref);
    return false;
  }

  while (data.main_context != NULL && !g_atomic_int_get(&data.main_context_done))
    g_main_context_iteration(data.main_context, true);

  g_clear_pointer(&data.main_context, g_main_context_unref);
  return true;
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
