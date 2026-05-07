#include "pwg-metadata.h"

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <spa/utils/json.h>
#include <string.h>

#include "pwg-core-private.h"
#include "pwg-error.h"

#define PWG_DEFAULT_AUDIO_SINK_KEY "default.audio.sink"
#define PWG_DEFAULT_AUDIO_SOURCE_KEY "default.audio.source"
#define PWG_DEFAULT_CONFIGURED_AUDIO_SINK_KEY "default.configured.audio.sink"
#define PWG_DEFAULT_CONFIGURED_AUDIO_SOURCE_KEY "default.configured.audio.source"

typedef struct {
  unsigned int subject;
  char *key;
  char *type;
  char *value;
} PwgMetadataEntry;

typedef enum {
  PWG_METADATA_EVENT_FOUND,
  PWG_METADATA_EVENT_REMOVED,
  PWG_METADATA_EVENT_CHANGED,
} PwgMetadataEventType;

typedef struct {
  PwgMetadata *metadata;
  unsigned int generation;
  PwgMetadataEventType event_type;
  unsigned int id;
  unsigned int version;
  unsigned int subject;
  char *key;
  char *type;
  char *value;
} PwgMetadataEvent;

struct _PwgMetadata {
  GObject parent_instance;
  PwgCore *core;
  char *name;
  gboolean running;
  gboolean bound;
  GHashTable *entries;
  GMainContext *main_context;
  struct pw_registry *registry;
  struct pw_metadata *metadata;
  unsigned int metadata_id;
  struct spa_hook registry_listener;
  struct spa_hook metadata_listener;
  gboolean has_registry_listener;
  gboolean has_metadata_listener;
  gulong core_connected_notify_id;
  unsigned int generation;
};

G_DEFINE_TYPE(PwgMetadata, pwg_metadata, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_CORE,
  PROP_NAME,
  PROP_RUNNING,
  PROP_BOUND,
  N_PROPS,
};

enum {
  SIGNAL_CHANGED,
  N_SIGNALS,
};

static GParamSpec *properties[N_PROPS];
static unsigned int signals[N_SIGNALS];

static gboolean pwg_metadata_dispatch_event(gpointer userdata);
static int pwg_metadata_on_property(void *userdata,
                                    uint32_t subject,
                                    const char *key,
                                    const char *type,
                                    const char *value);

static const struct pw_metadata_events metadata_events = {
  PW_VERSION_METADATA_EVENTS,
  .property = pwg_metadata_on_property,
};

static void
pwg_metadata_entry_free(PwgMetadataEntry *entry)
{
  g_clear_pointer(&entry->key, g_free);
  g_clear_pointer(&entry->type, g_free);
  g_clear_pointer(&entry->value, g_free);
  g_free(entry);
}

static char *
pwg_metadata_cache_key(unsigned int subject, const char *key)
{
  return g_strdup_printf("%u:%s", subject, key);
}

static PwgMetadataEntry *
pwg_metadata_lookup_entry(PwgMetadata *self, unsigned int subject, const char *key)
{
  g_autofree char *cache_key = NULL;

  cache_key = pwg_metadata_cache_key(subject, key);
  return g_hash_table_lookup(self->entries, cache_key);
}

static const char *
pwg_metadata_lookup_spa_dict(const struct spa_dict *dict, const char *key)
{
  const struct spa_dict_item *item;

  if (dict == NULL)
    return NULL;

  spa_dict_for_each(item, dict) {
    if (g_strcmp0(item->key, key) == 0)
      return item->value;
  }

  return NULL;
}

static gboolean
pwg_metadata_value_type_is_json(const char *type)
{
  return g_strcmp0(type, "Spa:String:JSON") == 0 ||
         g_strcmp0(type, "Spa:JSON") == 0;
}

static gboolean
pwg_metadata_value_looks_like_object(const char *value)
{
  const char *cursor = value;

  while (g_ascii_isspace(*cursor))
    cursor++;

  return *cursor == '{';
}

static char *
pwg_metadata_dup_name_from_json(const char *value)
{
  struct spa_json iter;
  struct spa_json object;
  const char *json_value = NULL;
  int json_value_length;

  spa_json_init(&iter, value, strlen(value));
  if (spa_json_enter_object(&iter, &object) <= 0)
    return NULL;

  while ((json_value_length = spa_json_next(&object, &json_value)) > 0) {
    g_autofree char *key = NULL;
    g_autofree char *name = NULL;

    key = g_malloc((gsize) json_value_length + 1);
    if (spa_json_parse_stringn(
          json_value,
          json_value_length,
          key,
          json_value_length + 1) <= 0)
      return NULL;

    json_value_length = spa_json_next(&object, &json_value);
    if (json_value_length <= 0)
      return NULL;

    if (g_strcmp0(key, "name") != 0)
      continue;

    name = g_malloc((gsize) json_value_length + 1);
    if (spa_json_parse_stringn(
          json_value,
          json_value_length,
          name,
          json_value_length + 1) <= 0)
      return NULL;

    return name[0] != '\0' ? g_steal_pointer(&name) : NULL;
  }

  return NULL;
}

static char *
pwg_metadata_entry_dup_node_name(PwgMetadataEntry *entry)
{
  if (entry == NULL || entry->value == NULL || entry->value[0] == '\0')
    return NULL;

  if (pwg_metadata_value_type_is_json(entry->type) ||
      pwg_metadata_value_looks_like_object(entry->value))
    return pwg_metadata_dup_name_from_json(entry->value);

  return g_strdup(entry->value);
}

static char *
pwg_metadata_dup_default_node_name(PwgMetadata *self, const char *key)
{
  PwgMetadataEntry *entry;

  g_return_val_if_fail(PWG_IS_METADATA(self), NULL);

  entry = pwg_metadata_lookup_entry(self, 0, key);
  return pwg_metadata_entry_dup_node_name(entry);
}

static void
pwg_metadata_event_free(PwgMetadataEvent *event)
{
  g_clear_object(&event->metadata);
  g_clear_pointer(&event->key, g_free);
  g_clear_pointer(&event->type, g_free);
  g_clear_pointer(&event->value, g_free);
  g_free(event);
}

static void
pwg_metadata_clear_subject(PwgMetadata *self, unsigned int subject)
{
  GHashTableIter iter;
  gpointer value;

  g_hash_table_iter_init(&iter, self->entries);
  while (g_hash_table_iter_next(&iter, NULL, &value)) {
    PwgMetadataEntry *entry = value;

    if (entry->subject == subject)
      g_hash_table_iter_remove(&iter);
  }
}

static void
pwg_metadata_cache_change(PwgMetadata *self,
                          unsigned int subject,
                          const char *key,
                          const char *type,
                          const char *value)
{
  g_autofree char *cache_key = NULL;
  PwgMetadataEntry *entry;

  if (key == NULL) {
    pwg_metadata_clear_subject(self, subject);
    return;
  }

  cache_key = pwg_metadata_cache_key(subject, key);
  if (value == NULL) {
    g_hash_table_remove(self->entries, cache_key);
    return;
  }

  entry = g_new0(PwgMetadataEntry, 1);
  entry->subject = subject;
  entry->key = g_strdup(key);
  entry->type = g_strdup(type);
  entry->value = g_strdup(value);
  g_hash_table_replace(self->entries, g_steal_pointer(&cache_key), entry);
}

static void
pwg_metadata_destroy_bound_proxy(PwgMetadata *self)
{
  struct pw_thread_loop *thread_loop;

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop != NULL && self->metadata != NULL) {
    pw_thread_loop_lock(thread_loop);
    if (self->has_metadata_listener) {
      spa_hook_remove(&self->metadata_listener);
      self->has_metadata_listener = FALSE;
    }
    pw_proxy_destroy((struct pw_proxy *) self->metadata);
    self->metadata = NULL;
    pw_thread_loop_unlock(thread_loop);
  } else {
    self->metadata = NULL;
    self->has_metadata_listener = FALSE;
  }

  self->metadata_id = 0;

  if (self->bound) {
    self->bound = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  }

  g_hash_table_remove_all(self->entries);
}

static gboolean
pwg_metadata_bind(PwgMetadata *self, unsigned int id, unsigned int version)
{
  struct pw_thread_loop *thread_loop;
  struct pw_metadata *metadata;
  int result;

  if (self->bound || self->metadata != NULL)
    return TRUE;

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop == NULL || self->registry == NULL)
    return FALSE;

  pw_thread_loop_lock(thread_loop);
  metadata = pw_registry_bind(
    self->registry,
    id,
    PW_TYPE_INTERFACE_Metadata,
    version < PW_VERSION_METADATA ? version : PW_VERSION_METADATA,
    0);
  if (metadata == NULL) {
    pw_thread_loop_unlock(thread_loop);
    return FALSE;
  }

  spa_zero(self->metadata_listener);
  result = pw_metadata_add_listener(metadata, &self->metadata_listener, &metadata_events, self);
  if (result < 0) {
    pw_proxy_destroy((struct pw_proxy *) metadata);
    pw_thread_loop_unlock(thread_loop);
    return FALSE;
  }

  self->metadata = metadata;
  self->metadata_id = id;
  self->has_metadata_listener = TRUE;
  self->bound = TRUE;
  pw_thread_loop_unlock(thread_loop);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  return TRUE;
}

static int
pwg_metadata_on_property(void *userdata,
                         uint32_t subject,
                         const char *key,
                         const char *type,
                         const char *value)
{
  PwgMetadata *self = PWG_METADATA(userdata);
  PwgMetadataEvent *event;

  event = g_new0(PwgMetadataEvent, 1);
  event->event_type = PWG_METADATA_EVENT_CHANGED;
  event->subject = subject;
  event->key = g_strdup(key);
  event->type = g_strdup(type);
  event->value = g_strdup(value);

  event->metadata = g_object_ref(self);
  event->generation = self->generation;
  g_main_context_invoke_full(
    self->main_context,
    G_PRIORITY_DEFAULT,
    pwg_metadata_dispatch_event,
    event,
    (GDestroyNotify) pwg_metadata_event_free);

  return 0;
}

static gboolean
pwg_metadata_dispatch_event(gpointer userdata)
{
  PwgMetadataEvent *event = userdata;
  PwgMetadata *self = event->metadata;

  if (!self->running || event->generation != self->generation)
    return G_SOURCE_REMOVE;

  if (event->event_type == PWG_METADATA_EVENT_FOUND) {
    pwg_metadata_bind(self, event->id, event->version);
  } else if (event->event_type == PWG_METADATA_EVENT_REMOVED) {
    if (self->metadata_id == event->id)
      pwg_metadata_destroy_bound_proxy(self);
  } else {
    pwg_metadata_cache_change(self, event->subject, event->key, event->type, event->value);
    g_signal_emit(
      self,
      signals[SIGNAL_CHANGED],
      0,
      event->subject,
      event->key,
      event->type,
      event->value);
  }

  return G_SOURCE_REMOVE;
}

static void
pwg_metadata_queue_event(PwgMetadata *self, PwgMetadataEvent *event)
{
  event->metadata = g_object_ref(self);
  event->generation = self->generation;

  g_main_context_invoke_full(
    self->main_context,
    G_PRIORITY_DEFAULT,
    pwg_metadata_dispatch_event,
    event,
    (GDestroyNotify) pwg_metadata_event_free);
}

static void
pwg_metadata_on_global(void *userdata,
                       uint32_t id,
                       uint32_t permissions,
                       const char *type,
                       uint32_t version,
                       const struct spa_dict *props)
{
  PwgMetadata *self = PWG_METADATA(userdata);
  const char *metadata_name;
  PwgMetadataEvent *event;

  (void) permissions;

  if (g_strcmp0(type, PW_TYPE_INTERFACE_Metadata) != 0)
    return;

  metadata_name = pwg_metadata_lookup_spa_dict(props, PW_KEY_METADATA_NAME);
  if (g_strcmp0(metadata_name, self->name) != 0)
    return;

  event = g_new0(PwgMetadataEvent, 1);
  event->event_type = PWG_METADATA_EVENT_FOUND;
  event->id = id;
  event->version = version;
  pwg_metadata_queue_event(self, event);
}

static void
pwg_metadata_on_global_remove(void *userdata, uint32_t id)
{
  PwgMetadata *self = PWG_METADATA(userdata);
  PwgMetadataEvent *event;

  event = g_new0(PwgMetadataEvent, 1);
  event->event_type = PWG_METADATA_EVENT_REMOVED;
  event->id = id;
  pwg_metadata_queue_event(self, event);
}

static const struct pw_registry_events registry_events = {
  PW_VERSION_REGISTRY_EVENTS,
  .global = pwg_metadata_on_global,
  .global_remove = pwg_metadata_on_global_remove,
};

static void
pwg_metadata_reset(PwgMetadata *self)
{
  self->registry = NULL;
  self->metadata = NULL;
  self->metadata_id = 0;
  self->has_registry_listener = FALSE;
  self->has_metadata_listener = FALSE;
  self->generation++;

  if (self->bound) {
    self->bound = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  }

  if (self->running) {
    self->running = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  }

  g_hash_table_remove_all(self->entries);
}

static void
pwg_metadata_on_core_connected_notify(GObject *object,
                                      GParamSpec *pspec,
                                      gpointer userdata)
{
  PwgMetadata *self = PWG_METADATA(userdata);

  (void) pspec;

  if (!pwg_core_get_connected(PWG_CORE(object)))
    pwg_metadata_reset(self);
}

static void
pwg_metadata_get_property(GObject *object,
                          unsigned int property_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  PwgMetadata *self = PWG_METADATA(object);

  switch (property_id) {
  case PROP_CORE:
    g_value_set_object(value, self->core);
    break;
  case PROP_NAME:
    g_value_set_string(value, self->name);
    break;
  case PROP_RUNNING:
    g_value_set_boolean(value, self->running);
    break;
  case PROP_BOUND:
    g_value_set_boolean(value, self->bound);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_metadata_set_property(GObject *object,
                          unsigned int property_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  PwgMetadata *self = PWG_METADATA(object);

  switch (property_id) {
  case PROP_CORE:
    self->core = g_value_dup_object(value);
    break;
  case PROP_NAME:
    g_clear_pointer(&self->name, g_free);
    self->name = g_value_dup_string(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_metadata_constructed(GObject *object)
{
  PwgMetadata *self = PWG_METADATA(object);

  G_OBJECT_CLASS(pwg_metadata_parent_class)->constructed(object);

  if (self->core != NULL) {
    self->core_connected_notify_id = g_signal_connect(
      self->core,
      "notify::connected",
      G_CALLBACK(pwg_metadata_on_core_connected_notify),
      self);
  }
}

static void
pwg_metadata_dispose(GObject *object)
{
  PwgMetadata *self = PWG_METADATA(object);

  pwg_metadata_stop(self);

  if (self->core != NULL && self->core_connected_notify_id != 0) {
    g_signal_handler_disconnect(self->core, self->core_connected_notify_id);
    self->core_connected_notify_id = 0;
  }

  g_clear_object(&self->core);
  g_clear_pointer(&self->entries, g_hash_table_unref);

  G_OBJECT_CLASS(pwg_metadata_parent_class)->dispose(object);
}

static void
pwg_metadata_finalize(GObject *object)
{
  PwgMetadata *self = PWG_METADATA(object);

  g_clear_pointer(&self->name, g_free);
  g_clear_pointer(&self->main_context, g_main_context_unref);

  G_OBJECT_CLASS(pwg_metadata_parent_class)->finalize(object);
}

static void
pwg_metadata_class_init(PwgMetadataClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = pwg_metadata_constructed;
  object_class->get_property = pwg_metadata_get_property;
  object_class->set_property = pwg_metadata_set_property;
  object_class->dispose = pwg_metadata_dispose;
  object_class->finalize = pwg_metadata_finalize;

  /**
   * PwgMetadata:core:
   *
   * PipeWire core used by this metadata wrapper.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_CORE] = g_param_spec_object(
    "core",
    "Core",
    "PipeWire core used by this metadata wrapper.",
    PWG_TYPE_CORE,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgMetadata:name:
   *
   * PipeWire metadata global name.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_NAME] = g_param_spec_string(
    "name",
    "Name",
    "PipeWire metadata global name.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgMetadata:running:
   *
   * Whether metadata discovery is running.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_RUNNING] = g_param_spec_boolean(
    "running",
    "Running",
    "Whether metadata discovery is running.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgMetadata:bound:
   *
   * Whether the named metadata object has been discovered and bound.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_BOUND] = g_param_spec_boolean(
    "bound",
    "Bound",
    "Whether the named metadata object has been discovered and bound.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);

  /**
   * PwgMetadata::changed:
   * @self: the metadata object.
   * @subject: the PipeWire global id associated with the metadata.
   * @key: (nullable): the changed metadata key, or %NULL if the subject was cleared.
   * @type: (nullable): the metadata value type, or %NULL.
   * @value: (nullable): the metadata value, or %NULL if the key was cleared.
   *
   * Emitted from the object's construction thread-default main context when
   * the bound metadata object reports a property change.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  signals[SIGNAL_CHANGED] = g_signal_new(
    "changed",
    G_TYPE_FROM_CLASS(klass),
    G_SIGNAL_RUN_LAST,
    0,
    NULL,
    NULL,
    NULL,
    G_TYPE_NONE,
    4,
    G_TYPE_UINT,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING);
}

static void
pwg_metadata_init(PwgMetadata *self)
{
  self->entries = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    g_free,
    (GDestroyNotify) pwg_metadata_entry_free);
  self->main_context = g_main_context_ref_thread_default();
}

PwgMetadata *
pwg_metadata_new(PwgCore *core, const char *name)
{
  g_return_val_if_fail(PWG_IS_CORE(core), NULL);
  g_return_val_if_fail(name != NULL, NULL);

  return g_object_new(PWG_TYPE_METADATA, "core", core, "name", name, NULL);
}

bool
pwg_metadata_start(PwgMetadata *self, GError **error)
{
  struct pw_thread_loop *thread_loop;
  struct pw_core *core;

  g_return_val_if_fail(PWG_IS_METADATA(self), FALSE);

  if (self->running)
    return TRUE;

  if (self->core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Metadata has no PipeWire core");
    return FALSE;
  }

  if (self->name == NULL || self->name[0] == '\0') {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Metadata has no PipeWire name");
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
pwg_metadata_stop(PwgMetadata *self)
{
  struct pw_thread_loop *thread_loop;

  g_return_if_fail(PWG_IS_METADATA(self));

  if (!self->running && self->registry == NULL && self->metadata == NULL)
    return;

  pwg_metadata_destroy_bound_proxy(self);

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
  } else {
    self->registry = NULL;
    self->has_registry_listener = FALSE;
  }

  self->generation++;

  if (self->running) {
    self->running = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  }

  g_hash_table_remove_all(self->entries);
}

PwgCore *
pwg_metadata_get_core(PwgMetadata *self)
{
  g_return_val_if_fail(PWG_IS_METADATA(self), NULL);

  return self->core;
}

const char *
pwg_metadata_get_name(PwgMetadata *self)
{
  g_return_val_if_fail(PWG_IS_METADATA(self), NULL);

  return self->name;
}

bool
pwg_metadata_get_running(PwgMetadata *self)
{
  g_return_val_if_fail(PWG_IS_METADATA(self), FALSE);

  return self->running;
}

bool
pwg_metadata_get_bound(PwgMetadata *self)
{
  g_return_val_if_fail(PWG_IS_METADATA(self), FALSE);

  return self->bound;
}

char *
pwg_metadata_dup_value(PwgMetadata *self, unsigned int subject, const char *key)
{
  PwgMetadataEntry *entry;

  g_return_val_if_fail(PWG_IS_METADATA(self), NULL);
  g_return_val_if_fail(key != NULL, NULL);

  entry = pwg_metadata_lookup_entry(self, subject, key);
  return entry != NULL ? g_strdup(entry->value) : NULL;
}

char *
pwg_metadata_dup_value_type(PwgMetadata *self, unsigned int subject, const char *key)
{
  PwgMetadataEntry *entry;

  g_return_val_if_fail(PWG_IS_METADATA(self), NULL);
  g_return_val_if_fail(key != NULL, NULL);

  entry = pwg_metadata_lookup_entry(self, subject, key);
  return entry != NULL ? g_strdup(entry->type) : NULL;
}

char *
pwg_metadata_dup_default_audio_sink_name(PwgMetadata *self)
{
  return pwg_metadata_dup_default_node_name(self, PWG_DEFAULT_AUDIO_SINK_KEY);
}

char *
pwg_metadata_dup_default_audio_source_name(PwgMetadata *self)
{
  return pwg_metadata_dup_default_node_name(self, PWG_DEFAULT_AUDIO_SOURCE_KEY);
}

char *
pwg_metadata_dup_configured_audio_sink_name(PwgMetadata *self)
{
  return pwg_metadata_dup_default_node_name(self, PWG_DEFAULT_CONFIGURED_AUDIO_SINK_KEY);
}

char *
pwg_metadata_dup_configured_audio_source_name(PwgMetadata *self)
{
  return pwg_metadata_dup_default_node_name(self, PWG_DEFAULT_CONFIGURED_AUDIO_SOURCE_KEY);
}

bool
pwg_metadata_set(PwgMetadata *self,
                 unsigned int subject,
                 const char *key,
                 const char *type,
                 const char *value,
                 GError **error)
{
  struct pw_thread_loop *thread_loop;
  int result;

  g_return_val_if_fail(PWG_IS_METADATA(self), FALSE);
  g_return_val_if_fail(key != NULL, FALSE);

  if (self->metadata == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Metadata object is not bound");
    return FALSE;
  }

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return FALSE;
  }

  pw_thread_loop_lock(thread_loop);
  result = pw_metadata_set_property(self->metadata, subject, key, type, value);
  pw_thread_loop_unlock(thread_loop);

  if (result < 0) {
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not set PipeWire metadata: %s",
      g_strerror(-result));
    return FALSE;
  }

  return TRUE;
}

bool
pwg_metadata_clear(PwgMetadata *self, GError **error)
{
  struct pw_thread_loop *thread_loop;
  int result;

  g_return_val_if_fail(PWG_IS_METADATA(self), FALSE);

  if (self->metadata == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Metadata object is not bound");
    return FALSE;
  }

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return FALSE;
  }

  pw_thread_loop_lock(thread_loop);
  result = pw_metadata_clear(self->metadata);
  pw_thread_loop_unlock(thread_loop);

  if (result < 0) {
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not clear PipeWire metadata: %s",
      g_strerror(-result));
    return FALSE;
  }

  g_hash_table_remove_all(self->entries);
  return TRUE;
}
