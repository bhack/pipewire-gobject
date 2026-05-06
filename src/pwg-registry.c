#include "pwg-registry.h"

#include <pipewire/pipewire.h>

#include "pwg-core-private.h"
#include "pwg-error.h"
#include "pwg-global-private.h"

struct _PwgRegistry {
  GObject parent_instance;
  PwgCore *core;
  gboolean running;
  GListStore *globals;
  GMainContext *main_context;
  struct pw_registry *registry;
  struct spa_hook registry_listener;
  gboolean has_registry_listener;
  gulong core_connected_notify_id;
  guint generation;
};

G_DEFINE_TYPE(PwgRegistry, pwg_registry, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_CORE,
  PROP_RUNNING,
  PROP_GLOBALS,
  N_PROPS,
};

enum {
  SIGNAL_GLOBAL_ADDED,
  SIGNAL_GLOBAL_REMOVED,
  N_SIGNALS,
};

typedef enum {
  PWG_REGISTRY_EVENT_GLOBAL_ADDED,
  PWG_REGISTRY_EVENT_GLOBAL_REMOVED,
} PwgRegistryEventType;

typedef struct {
  PwgRegistry *registry;
  guint generation;
  PwgRegistryEventType type;
  PwgGlobal *global;
  guint id;
} PwgRegistryEvent;

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static void
pwg_registry_event_free(PwgRegistryEvent *event)
{
  g_clear_object(&event->global);
  g_clear_object(&event->registry);
  g_free(event);
}

static PwgGlobal *
pwg_registry_find_global(PwgRegistry *self, guint id, guint *position)
{
  guint n_items;

  n_items = g_list_model_get_n_items(G_LIST_MODEL(self->globals));
  for (guint i = 0; i < n_items; i++) {
    g_autoptr(PwgGlobal) global = g_list_model_get_item(G_LIST_MODEL(self->globals), i);

    if (pwg_global_get_id(global) == id) {
      if (position != NULL)
        *position = i;
      return g_object_ref(global);
    }
  }

  return NULL;
}

static gboolean
pwg_registry_global_property_equals(PwgGlobal *global,
                                    const char *key,
                                    const char *value)
{
  g_autofree char *global_value = pwg_global_dup_property(global, key);

  return g_strcmp0(global_value, value) == 0;
}

static GListModel *
pwg_registry_dup_matching_globals(PwgRegistry *self,
                                  gboolean (*match_func)(PwgGlobal *global, gpointer userdata),
                                  gpointer userdata)
{
  g_autoptr(GListStore) matches = NULL;
  guint n_items;

  matches = g_list_store_new(PWG_TYPE_GLOBAL);
  n_items = g_list_model_get_n_items(G_LIST_MODEL(self->globals));
  for (guint i = 0; i < n_items; i++) {
    g_autoptr(PwgGlobal) global = g_list_model_get_item(G_LIST_MODEL(self->globals), i);

    if (match_func(global, userdata))
      g_list_store_append(matches, global);
  }

  return G_LIST_MODEL(g_steal_pointer(&matches));
}

static PwgGlobal *
pwg_registry_lookup_matching_global(PwgRegistry *self,
                                    gboolean (*match_func)(PwgGlobal *global, gpointer userdata),
                                    gpointer userdata)
{
  guint n_items;

  n_items = g_list_model_get_n_items(G_LIST_MODEL(self->globals));
  for (guint i = 0; i < n_items; i++) {
    g_autoptr(PwgGlobal) global = g_list_model_get_item(G_LIST_MODEL(self->globals), i);

    if (match_func(global, userdata))
      return g_object_ref(global);
  }

  return NULL;
}

typedef struct {
  const char *key;
  const char *value;
} PwgRegistryPropertyMatch;

static gboolean
pwg_registry_match_property(PwgGlobal *global, gpointer userdata)
{
  PwgRegistryPropertyMatch *match = userdata;

  return pwg_registry_global_property_equals(global, match->key, match->value);
}

static gboolean
pwg_registry_match_interface(PwgGlobal *global, gpointer userdata)
{
  const char *interface_type = userdata;

  return pwg_global_is_interface(global, interface_type);
}

static gboolean
pwg_registry_match_media_class(PwgGlobal *global, gpointer userdata)
{
  const char *media_class = userdata;
  g_autofree char *global_media_class = pwg_global_dup_media_class(global);

  return g_strcmp0(global_media_class, media_class) == 0;
}

static gboolean
pwg_registry_dispatch_event(gpointer userdata)
{
  PwgRegistryEvent *event = userdata;
  PwgRegistry *self = event->registry;

  if (!self->running || event->generation != self->generation)
    return G_SOURCE_REMOVE;

  if (event->type == PWG_REGISTRY_EVENT_GLOBAL_ADDED) {
    guint position = 0;
    g_autoptr(PwgGlobal) existing = NULL;

    existing = pwg_registry_find_global(self, pwg_global_get_id(event->global), &position);
    if (existing != NULL)
      g_list_store_remove(self->globals, position);

    g_list_store_append(self->globals, event->global);
    g_signal_emit(self, signals[SIGNAL_GLOBAL_ADDED], 0, event->global);
  } else {
    guint position = 0;
    g_autoptr(PwgGlobal) existing = NULL;

    existing = pwg_registry_find_global(self, event->id, &position);
    if (existing != NULL) {
      g_list_store_remove(self->globals, position);
      g_signal_emit(self, signals[SIGNAL_GLOBAL_REMOVED], 0, existing);
    }
  }

  return G_SOURCE_REMOVE;
}

static void
pwg_registry_queue_event(PwgRegistry *self, PwgRegistryEvent *event)
{
  event->registry = g_object_ref(self);
  event->generation = self->generation;

  g_main_context_invoke_full(
    self->main_context,
    G_PRIORITY_DEFAULT,
    pwg_registry_dispatch_event,
    event,
    (GDestroyNotify) pwg_registry_event_free);
}

static void
pwg_registry_on_global(void *userdata,
                       uint32_t id,
                       uint32_t permissions,
                       const char *type,
                       uint32_t version,
                       const struct spa_dict *props)
{
  PwgRegistry *self = PWG_REGISTRY(userdata);
  PwgRegistryEvent *event;

  event = g_new0(PwgRegistryEvent, 1);
  event->type = PWG_REGISTRY_EVENT_GLOBAL_ADDED;
  event->global = _pwg_global_new_from_spa_dict(id, permissions, type, version, props);
  pwg_registry_queue_event(self, event);
}

static void
pwg_registry_on_global_remove(void *userdata, uint32_t id)
{
  PwgRegistry *self = PWG_REGISTRY(userdata);
  PwgRegistryEvent *event;

  event = g_new0(PwgRegistryEvent, 1);
  event->type = PWG_REGISTRY_EVENT_GLOBAL_REMOVED;
  event->id = id;
  pwg_registry_queue_event(self, event);
}

static const struct pw_registry_events registry_events = {
  PW_VERSION_REGISTRY_EVENTS,
  .global = pwg_registry_on_global,
  .global_remove = pwg_registry_on_global_remove,
};

static void
pwg_registry_reset(PwgRegistry *self)
{
  self->registry = NULL;
  self->has_registry_listener = FALSE;
  self->generation++;

  if (self->running) {
    self->running = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  }

  g_list_store_remove_all(self->globals);
}

static void
pwg_registry_on_core_connected_notify(GObject *object,
                                      GParamSpec *pspec,
                                      gpointer userdata)
{
  PwgRegistry *self = PWG_REGISTRY(userdata);

  (void) pspec;

  if (!pwg_core_get_connected(PWG_CORE(object)))
    pwg_registry_reset(self);
}

static void
pwg_registry_get_property(GObject *object,
                          guint property_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  PwgRegistry *self = PWG_REGISTRY(object);

  switch (property_id) {
  case PROP_CORE:
    g_value_set_object(value, self->core);
    break;
  case PROP_RUNNING:
    g_value_set_boolean(value, self->running);
    break;
  case PROP_GLOBALS:
    g_value_set_object(value, self->globals);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_registry_set_property(GObject *object,
                          guint property_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  PwgRegistry *self = PWG_REGISTRY(object);

  switch (property_id) {
  case PROP_CORE:
    self->core = g_value_dup_object(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_registry_constructed(GObject *object)
{
  PwgRegistry *self = PWG_REGISTRY(object);

  G_OBJECT_CLASS(pwg_registry_parent_class)->constructed(object);

  if (self->core != NULL) {
    self->core_connected_notify_id = g_signal_connect(
      self->core,
      "notify::connected",
      G_CALLBACK(pwg_registry_on_core_connected_notify),
      self);
  }
}

static void
pwg_registry_dispose(GObject *object)
{
  PwgRegistry *self = PWG_REGISTRY(object);

  pwg_registry_stop(self);

  if (self->core != NULL && self->core_connected_notify_id != 0) {
    g_signal_handler_disconnect(self->core, self->core_connected_notify_id);
    self->core_connected_notify_id = 0;
  }

  g_clear_object(&self->core);
  g_clear_object(&self->globals);

  G_OBJECT_CLASS(pwg_registry_parent_class)->dispose(object);
}

static void
pwg_registry_finalize(GObject *object)
{
  PwgRegistry *self = PWG_REGISTRY(object);

  g_clear_pointer(&self->main_context, g_main_context_unref);

  G_OBJECT_CLASS(pwg_registry_parent_class)->finalize(object);
}

static void
pwg_registry_class_init(PwgRegistryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = pwg_registry_constructed;
  object_class->get_property = pwg_registry_get_property;
  object_class->set_property = pwg_registry_set_property;
  object_class->dispose = pwg_registry_dispose;
  object_class->finalize = pwg_registry_finalize;

  properties[PROP_CORE] = g_param_spec_object(
    "core",
    "Core",
    "PipeWire core used by this registry.",
    PWG_TYPE_CORE,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  properties[PROP_RUNNING] = g_param_spec_boolean(
    "running",
    "Running",
    "Whether registry discovery is running.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  properties[PROP_GLOBALS] = g_param_spec_object(
    "globals",
    "Globals",
    "A GListModel of PwgGlobal objects.",
    G_TYPE_LIST_MODEL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);

  /**
   * PwgRegistry::global-added:
   * @self: the registry object.
   * @global: the global descriptor that was added.
   *
   * Emitted from the object's construction thread-default main context when a
   * PipeWire global is discovered.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  signals[SIGNAL_GLOBAL_ADDED] = g_signal_new(
    "global-added",
    G_TYPE_FROM_CLASS(klass),
    G_SIGNAL_RUN_LAST,
    0,
    NULL,
    NULL,
    NULL,
    G_TYPE_NONE,
    1,
    PWG_TYPE_GLOBAL);

  /**
   * PwgRegistry::global-removed:
   * @self: the registry object.
   * @global: the global descriptor that was removed.
   *
   * Emitted from the object's construction thread-default main context when a
   * PipeWire global disappears.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  signals[SIGNAL_GLOBAL_REMOVED] = g_signal_new(
    "global-removed",
    G_TYPE_FROM_CLASS(klass),
    G_SIGNAL_RUN_LAST,
    0,
    NULL,
    NULL,
    NULL,
    G_TYPE_NONE,
    1,
    PWG_TYPE_GLOBAL);
}

static void
pwg_registry_init(PwgRegistry *self)
{
  self->globals = g_list_store_new(PWG_TYPE_GLOBAL);
  self->main_context = g_main_context_ref_thread_default();
}

PwgRegistry *
pwg_registry_new(PwgCore *core)
{
  g_return_val_if_fail(PWG_IS_CORE(core), NULL);

  return g_object_new(PWG_TYPE_REGISTRY, "core", core, NULL);
}

gboolean
pwg_registry_start(PwgRegistry *self, GError **error)
{
  struct pw_thread_loop *thread_loop;
  struct pw_core *core;

  g_return_val_if_fail(PWG_IS_REGISTRY(self), FALSE);

  if (self->running)
    return TRUE;

  if (self->core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Registry has no PipeWire core");
    return FALSE;
  }

  if (!pwg_core_get_connected(self->core) && !pwg_core_connect(self->core, error))
    return FALSE;

  thread_loop = pwg_core_get_thread_loop_internal(self->core);
  core = pwg_core_get_pw_core_internal(self->core);
  if (thread_loop == NULL || core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return FALSE;
  }

  pw_thread_loop_lock(thread_loop);
  self->registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);
  if (self->registry == NULL) {
    pw_thread_loop_unlock(thread_loop);
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not create PipeWire registry");
    return FALSE;
  }

  spa_zero(self->registry_listener);
  pw_registry_add_listener(self->registry, &self->registry_listener, &registry_events, self);
  self->has_registry_listener = TRUE;
  self->generation++;
  self->running = TRUE;
  pw_thread_loop_unlock(thread_loop);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  return TRUE;
}

void
pwg_registry_stop(PwgRegistry *self)
{
  struct pw_thread_loop *thread_loop;

  g_return_if_fail(PWG_IS_REGISTRY(self));

  if (!self->running && self->registry == NULL)
    return;

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop != NULL && self->registry != NULL) {
    pw_thread_loop_lock(thread_loop);
    if (self->has_registry_listener) {
      spa_hook_remove(&self->registry_listener);
      self->has_registry_listener = FALSE;
    }
    pw_proxy_destroy((struct pw_proxy *) self->registry);
    self->registry = NULL;
    pw_thread_loop_unlock(thread_loop);
  }

  self->generation++;

  if (self->running) {
    self->running = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  }

  g_list_store_remove_all(self->globals);
}

PwgCore *
pwg_registry_get_core(PwgRegistry *self)
{
  g_return_val_if_fail(PWG_IS_REGISTRY(self), NULL);

  return self->core;
}

gboolean
pwg_registry_get_running(PwgRegistry *self)
{
  g_return_val_if_fail(PWG_IS_REGISTRY(self), FALSE);

  return self->running;
}

GListModel *
pwg_registry_get_globals(PwgRegistry *self)
{
  g_return_val_if_fail(PWG_IS_REGISTRY(self), NULL);

  return G_LIST_MODEL(self->globals);
}

PwgGlobal *
pwg_registry_lookup_global(PwgRegistry *self, guint id)
{
  g_return_val_if_fail(PWG_IS_REGISTRY(self), NULL);

  return pwg_registry_find_global(self, id, NULL);
}

PwgGlobal *
pwg_registry_lookup_global_by_property(PwgRegistry *self,
                                       const char *key,
                                       const char *value)
{
  PwgRegistryPropertyMatch match = {key, value};

  g_return_val_if_fail(PWG_IS_REGISTRY(self), NULL);
  g_return_val_if_fail(key != NULL, NULL);
  g_return_val_if_fail(value != NULL, NULL);

  return pwg_registry_lookup_matching_global(self, pwg_registry_match_property, &match);
}

PwgGlobal *
pwg_registry_lookup_global_by_object_serial(PwgRegistry *self,
                                            const char *object_serial)
{
  g_return_val_if_fail(PWG_IS_REGISTRY(self), NULL);
  g_return_val_if_fail(object_serial != NULL, NULL);

  return pwg_registry_lookup_global_by_property(self, PW_KEY_OBJECT_SERIAL, object_serial);
}

GListModel *
pwg_registry_dup_globals_by_property(PwgRegistry *self,
                                     const char *key,
                                     const char *value)
{
  PwgRegistryPropertyMatch match = {key, value};

  g_return_val_if_fail(PWG_IS_REGISTRY(self), NULL);
  g_return_val_if_fail(key != NULL, NULL);
  g_return_val_if_fail(value != NULL, NULL);

  return pwg_registry_dup_matching_globals(self, pwg_registry_match_property, &match);
}

GListModel *
pwg_registry_dup_globals_by_interface(PwgRegistry *self,
                                      const char *interface_type)
{
  g_return_val_if_fail(PWG_IS_REGISTRY(self), NULL);
  g_return_val_if_fail(interface_type != NULL, NULL);

  return pwg_registry_dup_matching_globals(self, pwg_registry_match_interface, (gpointer) interface_type);
}

GListModel *
pwg_registry_dup_globals_by_media_class(PwgRegistry *self,
                                        const char *media_class)
{
  g_return_val_if_fail(PWG_IS_REGISTRY(self), NULL);
  g_return_val_if_fail(media_class != NULL, NULL);

  return pwg_registry_dup_matching_globals(self, pwg_registry_match_media_class, (gpointer) media_class);
}
