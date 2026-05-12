#include "pwg-link.h"

#include <errno.h>
#include <pipewire/link.h>
#include <pipewire/pipewire.h>

#include "pwg-core-private.h"
#include "pwg-error.h"

typedef enum {
  PWG_LINK_EVENT_INFO,
  PWG_LINK_EVENT_REMOVED,
} PwgLinkEventType;

typedef struct {
  PwgLink *link;
  unsigned int generation;
  PwgLinkEventType type;
  char *state;
  char *error;
} PwgLinkEvent;

struct _PwgLink {
  GObject parent_instance;
  PwgCore *core;
  PwgGlobal *global;
  bool running;
  bool bound;
  char *state;
  char *error;
  GMainContext *main_context;
  struct pw_registry *registry;
  struct pw_link *link;
  unsigned int link_id;
  struct spa_hook registry_listener;
  struct spa_hook link_listener;
  bool has_registry_listener;
  bool has_link_listener;
  gulong core_connected_notify_id;
  unsigned int generation;
};

G_DEFINE_TYPE(PwgLink, pwg_link, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_CORE,
  PROP_GLOBAL,
  PROP_RUNNING,
  PROP_BOUND,
  PROP_STATE,
  PROP_ERROR,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static gboolean pwg_link_dispatch_event(gpointer userdata);

static void
pwg_link_event_free(PwgLinkEvent *event)
{
  g_clear_object(&event->link);
  g_free(event->state);
  g_free(event->error);
  g_free(event);
}

static void
pwg_link_queue_event(PwgLink *self, PwgLinkEvent *event)
{
  event->link = g_object_ref(self);
  event->generation = self->generation;

  g_main_context_invoke_full(
    self->main_context,
    G_PRIORITY_DEFAULT,
    pwg_link_dispatch_event,
    event,
    (GDestroyNotify) pwg_link_event_free);
}

static void
pwg_link_on_info(void *userdata, const struct pw_link_info *info)
{
  PwgLink *self = PWG_LINK(userdata);
  PwgLinkEvent *event;

  if (info == NULL || (info->change_mask & PW_LINK_CHANGE_MASK_STATE) == 0)
    return;

  event = g_new0(PwgLinkEvent, 1);
  event->type = PWG_LINK_EVENT_INFO;
  event->state = g_strdup(pw_link_state_as_string(info->state));
  event->error = g_strdup(info->error);
  pwg_link_queue_event(self, event);
}

static const struct pw_link_events link_events = {
  PW_VERSION_LINK_EVENTS,
  .info = pwg_link_on_info,
};

static void
pwg_link_on_global_remove(void *userdata, uint32_t id)
{
  PwgLink *self = PWG_LINK(userdata);
  PwgLinkEvent *event;

  if (id != self->link_id)
    return;

  event = g_new0(PwgLinkEvent, 1);
  event->type = PWG_LINK_EVENT_REMOVED;
  pwg_link_queue_event(self, event);
}

static const struct pw_registry_events registry_events = {
  PW_VERSION_REGISTRY_EVENTS,
  .global_remove = pwg_link_on_global_remove,
};

static void
pwg_link_destroy_pipewire_objects(PwgLink *self)
{
  struct pw_thread_loop *thread_loop;

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop != NULL)
    pw_thread_loop_lock(thread_loop);

  if (self->link != NULL) {
    if (self->has_link_listener) {
      spa_hook_remove(&self->link_listener);
      self->has_link_listener = false;
    }
    pw_proxy_destroy((struct pw_proxy *) self->link);
    self->link = NULL;
  } else {
    self->has_link_listener = false;
  }

  if (self->registry != NULL) {
    if (self->has_registry_listener) {
      spa_hook_remove(&self->registry_listener);
      self->has_registry_listener = false;
    }
    pw_proxy_destroy((struct pw_proxy *) self->registry);
    self->registry = NULL;
  } else {
    self->has_registry_listener = false;
  }

  if (thread_loop != NULL)
    pw_thread_loop_unlock(thread_loop);
}

static void
pwg_link_clear_state(PwgLink *self)
{
  if (self->state != NULL) {
    g_clear_pointer(&self->state, g_free);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_STATE]);
  }

  if (self->error != NULL) {
    g_clear_pointer(&self->error, g_free);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_ERROR]);
  }
}

static void
pwg_link_reset(PwgLink *self)
{
  self->registry = NULL;
  self->link = NULL;
  self->has_registry_listener = false;
  self->has_link_listener = false;
  self->generation++;

  if (self->bound) {
    self->bound = false;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  }

  if (self->running) {
    self->running = false;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  }

  pwg_link_clear_state(self);
}

static void
pwg_link_update_state(PwgLink *self, const char *state, const char *error)
{
  if (g_strcmp0(self->state, state) != 0) {
    g_free(self->state);
    self->state = g_strdup(state);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_STATE]);
  }

  if (g_strcmp0(self->error, error) != 0) {
    g_free(self->error);
    self->error = g_strdup(error);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_ERROR]);
  }
}

static gboolean
pwg_link_dispatch_event(gpointer userdata)
{
  PwgLinkEvent *event = userdata;
  PwgLink *self = event->link;

  if (!self->running || event->generation != self->generation)
    return G_SOURCE_REMOVE;

  if (event->type == PWG_LINK_EVENT_INFO)
    pwg_link_update_state(self, event->state, event->error);
  else
    pwg_link_stop(self);

  return G_SOURCE_REMOVE;
}

static void
pwg_link_on_core_connected_notify(GObject *object,
                                  GParamSpec *pspec,
                                  gpointer userdata)
{
  PwgLink *self = PWG_LINK(userdata);

  (void) pspec;

  if (!pwg_core_get_connected(PWG_CORE(object)))
    pwg_link_reset(self);
}

static void
pwg_link_get_property(GObject *object,
                      unsigned int property_id,
                      GValue *value,
                      GParamSpec *pspec)
{
  PwgLink *self = PWG_LINK(object);

  switch (property_id) {
  case PROP_CORE:
    g_value_set_object(value, self->core);
    break;
  case PROP_GLOBAL:
    g_value_set_object(value, self->global);
    break;
  case PROP_RUNNING:
    g_value_set_boolean(value, self->running);
    break;
  case PROP_BOUND:
    g_value_set_boolean(value, self->bound);
    break;
  case PROP_STATE:
    g_value_set_string(value, self->state);
    break;
  case PROP_ERROR:
    g_value_set_string(value, self->error);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_link_set_property(GObject *object,
                      unsigned int property_id,
                      const GValue *value,
                      GParamSpec *pspec)
{
  PwgLink *self = PWG_LINK(object);

  switch (property_id) {
  case PROP_CORE:
    self->core = g_value_dup_object(value);
    break;
  case PROP_GLOBAL:
    self->global = g_value_dup_object(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_link_constructed(GObject *object)
{
  PwgLink *self = PWG_LINK(object);

  G_OBJECT_CLASS(pwg_link_parent_class)->constructed(object);

  if (self->core != NULL) {
    self->core_connected_notify_id = g_signal_connect(
      self->core,
      "notify::connected",
      G_CALLBACK(pwg_link_on_core_connected_notify),
      self);
  }

  if (self->global != NULL)
    self->link_id = pwg_global_get_id(self->global);
}

static void
pwg_link_dispose(GObject *object)
{
  PwgLink *self = PWG_LINK(object);

  pwg_link_stop(self);

  if (self->core != NULL && self->core_connected_notify_id != 0) {
    g_signal_handler_disconnect(self->core, self->core_connected_notify_id);
    self->core_connected_notify_id = 0;
  }

  g_clear_object(&self->core);
  g_clear_object(&self->global);

  G_OBJECT_CLASS(pwg_link_parent_class)->dispose(object);
}

static void
pwg_link_finalize(GObject *object)
{
  PwgLink *self = PWG_LINK(object);

  g_clear_pointer(&self->state, g_free);
  g_clear_pointer(&self->error, g_free);
  g_clear_pointer(&self->main_context, g_main_context_unref);

  G_OBJECT_CLASS(pwg_link_parent_class)->finalize(object);
}

static void
pwg_link_class_init(PwgLinkClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = pwg_link_constructed;
  object_class->get_property = pwg_link_get_property;
  object_class->set_property = pwg_link_set_property;
  object_class->dispose = pwg_link_dispose;
  object_class->finalize = pwg_link_finalize;

  /**
   * PwgLink:core:
   *
   * PipeWire core used by this link wrapper.
   *
   * Since: 0.3.7
   * Stability: Unstable
   */
  properties[PROP_CORE] = g_param_spec_object(
    "core",
    "Core",
    "PipeWire core used by this link wrapper.",
    PWG_TYPE_CORE,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgLink:global:
   *
   * PipeWire link global used by this link wrapper.
   *
   * Since: 0.3.7
   * Stability: Unstable
   */
  properties[PROP_GLOBAL] = g_param_spec_object(
    "global",
    "Global",
    "PipeWire link global used by this link wrapper.",
    PWG_TYPE_GLOBAL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgLink:running:
   *
   * Whether the link wrapper is running.
   *
   * Since: 0.3.7
   * Stability: Unstable
   */
  properties[PROP_RUNNING] = g_param_spec_boolean(
    "running",
    "Running",
    "Whether the link wrapper is running.",
    false,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgLink:bound:
   *
   * Whether the link proxy is currently bound.
   *
   * Since: 0.3.7
   * Stability: Unstable
   */
  properties[PROP_BOUND] = g_param_spec_boolean(
    "bound",
    "Bound",
    "Whether the link proxy is currently bound.",
    false,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgLink:state:
   *
   * The latest copied PipeWire link state string.
   *
   * Since: 0.3.7
   * Stability: Unstable
   */
  properties[PROP_STATE] = g_param_spec_string(
    "state",
    "State",
    "The latest copied PipeWire link state string.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgLink:error:
   *
   * The latest copied PipeWire link state error message.
   *
   * Since: 0.3.7
   * Stability: Unstable
   */
  properties[PROP_ERROR] = g_param_spec_string(
    "error",
    "Error",
    "The latest copied PipeWire link state error message.",
    NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_link_init(PwgLink *self)
{
  self->main_context = g_main_context_ref_thread_default();
}

PwgLink *
pwg_link_new(PwgCore *core, PwgGlobal *global)
{
  g_return_val_if_fail(PWG_IS_CORE(core), NULL);
  g_return_val_if_fail(PWG_IS_GLOBAL(global), NULL);

  if (!pwg_global_is_link(global))
    return NULL;

  return g_object_new(PWG_TYPE_LINK, "core", core, "global", global, NULL);
}

bool
pwg_link_start(PwgLink *self, GError **error)
{
  struct pw_thread_loop *thread_loop;
  struct pw_core *core;
  unsigned int version;
  int result;

  g_return_val_if_fail(PWG_IS_LINK(self), false);

  if (self->running)
    return true;

  if (self->core == NULL || self->global == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Link has no PipeWire core or global");
    return false;
  }

  if (!pwg_global_is_link(self->global)) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Global is not a PipeWire link");
    return false;
  }

  if (!pwg_core_get_connected(self->core) && !pwg_core_connect(self->core, error))
    return false;

  thread_loop = pwg_core_get_thread_loop_internal(self->core);
  core = pwg_core_get_pw_core_internal(self->core);
  if (thread_loop == NULL || core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return false;
  }

  pw_thread_loop_lock(thread_loop);

  self->registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);
  if (self->registry == NULL) {
    pw_thread_loop_unlock(thread_loop);
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not create PipeWire registry");
    return false;
  }

  spa_zero(self->registry_listener);
  pw_registry_add_listener(self->registry, &self->registry_listener, &registry_events, self);
  self->has_registry_listener = true;

  version = pwg_global_get_version(self->global);
  self->link = pw_registry_bind(
    self->registry,
    pwg_global_get_id(self->global),
    PW_TYPE_INTERFACE_Link,
    version < PW_VERSION_LINK ? version : PW_VERSION_LINK,
    0);
  if (self->link == NULL) {
    pw_thread_loop_unlock(thread_loop);
    pwg_link_destroy_pipewire_objects(self);
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not bind PipeWire link");
    return false;
  }

  spa_zero(self->link_listener);
  result = pw_link_add_listener(self->link, &self->link_listener, &link_events, self);
  if (result < 0) {
    pw_thread_loop_unlock(thread_loop);
    pwg_link_destroy_pipewire_objects(self);
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not listen to PipeWire link: %s",
      g_strerror(-result));
    return false;
  }

  self->has_link_listener = true;
  self->generation++;
  self->running = true;
  self->bound = true;
  pw_thread_loop_unlock(thread_loop);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  return true;
}

bool
pwg_link_sync(PwgLink *self, unsigned int timeout_msec, GError **error)
{
  g_return_val_if_fail(PWG_IS_LINK(self), false);

  if (!self->running && !pwg_link_start(self, error))
    return false;

  if (self->core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Link has no PipeWire core");
    return false;
  }

  return pwg_core_sync_main_context_internal(self->core, self->main_context, timeout_msec, error);
}

void
pwg_link_stop(PwgLink *self)
{
  g_return_if_fail(PWG_IS_LINK(self));

  if (!self->running && self->registry == NULL && self->link == NULL)
    return;

  self->generation++;
  pwg_link_destroy_pipewire_objects(self);

  if (self->bound) {
    self->bound = false;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  }

  if (self->running) {
    self->running = false;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  }

  pwg_link_clear_state(self);
}

PwgCore *
pwg_link_get_core(PwgLink *self)
{
  g_return_val_if_fail(PWG_IS_LINK(self), NULL);

  return self->core;
}

PwgGlobal *
pwg_link_get_global(PwgLink *self)
{
  g_return_val_if_fail(PWG_IS_LINK(self), NULL);

  return self->global;
}

bool
pwg_link_get_running(PwgLink *self)
{
  g_return_val_if_fail(PWG_IS_LINK(self), false);

  return self->running;
}

bool
pwg_link_get_bound(PwgLink *self)
{
  g_return_val_if_fail(PWG_IS_LINK(self), false);

  return self->bound;
}

const char *
pwg_link_get_state(PwgLink *self)
{
  g_return_val_if_fail(PWG_IS_LINK(self), NULL);

  return self->state;
}

char *
pwg_link_dup_error(PwgLink *self)
{
  g_return_val_if_fail(PWG_IS_LINK(self), NULL);

  return g_strdup(self->error);
}
