#include "pwg-node.h"

#include <errno.h>
#include <pipewire/node.h>
#include <pipewire/pipewire.h>
#include <spa/utils/result.h>

#include "pwg-core-private.h"
#include "pwg-error.h"
#include "pwg-param-info-private.h"
#include "pwg-param-private.h"

typedef struct {
  unsigned int id;
  unsigned int flags;
} PwgNodeParamInfoCopy;

typedef enum {
  PWG_NODE_EVENT_INFO,
  PWG_NODE_EVENT_PARAM,
  PWG_NODE_EVENT_REMOVED,
} PwgNodeEventType;

typedef struct {
  PwgNode *node;
  unsigned int generation;
  PwgNodeEventType type;
  GArray *param_infos;
  PwgParam *param;
} PwgNodeEvent;

struct _PwgNode {
  GObject parent_instance;
  PwgCore *core;
  PwgGlobal *global;
  gboolean running;
  gboolean bound;
  GListStore *param_infos;
  GListStore *params;
  GMainContext *main_context;
  struct pw_registry *registry;
  struct pw_node *node;
  unsigned int node_id;
  struct spa_hook registry_listener;
  struct spa_hook node_listener;
  gboolean has_registry_listener;
  gboolean has_node_listener;
  gulong core_connected_notify_id;
  unsigned int generation;
  int next_seq;
};

G_DEFINE_TYPE(PwgNode, pwg_node, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_CORE,
  PROP_GLOBAL,
  PROP_RUNNING,
  PROP_BOUND,
  PROP_PARAM_INFOS,
  PROP_PARAMS,
  N_PROPS,
};

enum {
  SIGNAL_PARAM,
  N_SIGNALS,
};

static GParamSpec *properties[N_PROPS];
static unsigned int signals[N_SIGNALS];

static gboolean pwg_node_dispatch_event(gpointer userdata);

static void
pwg_node_event_free(PwgNodeEvent *event)
{
  g_clear_object(&event->node);
  g_clear_pointer(&event->param_infos, g_array_unref);
  g_clear_object(&event->param);
  g_free(event);
}

static void
pwg_node_queue_event(PwgNode *self, PwgNodeEvent *event)
{
  event->node = g_object_ref(self);
  event->generation = self->generation;

  g_main_context_invoke_full(
    self->main_context,
    G_PRIORITY_DEFAULT,
    pwg_node_dispatch_event,
    event,
    (GDestroyNotify) pwg_node_event_free);
}

static void
pwg_node_on_info(void *userdata, const struct pw_node_info *info)
{
  PwgNode *self = PWG_NODE(userdata);
  PwgNodeEvent *event;

  if (info == NULL)
    return;

  if (info->params == NULL && (info->change_mask & PW_NODE_CHANGE_MASK_PARAMS) == 0)
    return;

  event = g_new0(PwgNodeEvent, 1);
  event->type = PWG_NODE_EVENT_INFO;
  event->param_infos = g_array_sized_new(FALSE, FALSE, sizeof(PwgNodeParamInfoCopy), info->n_params);

  if (info->params != NULL) {
    for (uint32_t i = 0; i < info->n_params; i++) {
      PwgNodeParamInfoCopy param_info = {
        .id = info->params[i].id,
        .flags = info->params[i].flags,
      };

      g_array_append_val(event->param_infos, param_info);
    }
  }

  pwg_node_queue_event(self, event);
}

static void
pwg_node_on_param(void *userdata,
                  int seq,
                  uint32_t id,
                  uint32_t index,
                  uint32_t next,
                  const struct spa_pod *param)
{
  PwgNode *self = PWG_NODE(userdata);
  g_autoptr(GBytes) bytes = NULL;
  PwgNodeEvent *event;

  if (param == NULL)
    return;

  bytes = g_bytes_new(param, SPA_POD_SIZE(param));

  event = g_new0(PwgNodeEvent, 1);
  event->type = PWG_NODE_EVENT_PARAM;
  event->param = _pwg_param_new(seq, id, index, next, bytes);
  pwg_node_queue_event(self, event);
}

static const struct pw_node_events node_events = {
  PW_VERSION_NODE_EVENTS,
  .info = pwg_node_on_info,
  .param = pwg_node_on_param,
};

static void
pwg_node_on_global_remove(void *userdata, uint32_t id)
{
  PwgNode *self = PWG_NODE(userdata);
  PwgNodeEvent *event;

  if (id != self->node_id)
    return;

  event = g_new0(PwgNodeEvent, 1);
  event->type = PWG_NODE_EVENT_REMOVED;
  pwg_node_queue_event(self, event);
}

static const struct pw_registry_events registry_events = {
  PW_VERSION_REGISTRY_EVENTS,
  .global_remove = pwg_node_on_global_remove,
};

static void
pwg_node_destroy_pipewire_objects(PwgNode *self)
{
  struct pw_thread_loop *thread_loop;

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop != NULL)
    pw_thread_loop_lock(thread_loop);

  if (self->node != NULL) {
    if (self->has_node_listener) {
      spa_hook_remove(&self->node_listener);
      self->has_node_listener = FALSE;
    }
    pw_proxy_destroy((struct pw_proxy *) self->node);
    self->node = NULL;
  } else {
    self->has_node_listener = FALSE;
  }

  if (self->registry != NULL) {
    if (self->has_registry_listener) {
      spa_hook_remove(&self->registry_listener);
      self->has_registry_listener = FALSE;
    }
    pw_proxy_destroy((struct pw_proxy *) self->registry);
    self->registry = NULL;
  } else {
    self->has_registry_listener = FALSE;
  }

  if (thread_loop != NULL)
    pw_thread_loop_unlock(thread_loop);
}

static void
pwg_node_reset(PwgNode *self)
{
  self->registry = NULL;
  self->node = NULL;
  self->has_registry_listener = FALSE;
  self->has_node_listener = FALSE;
  self->generation++;

  if (self->bound) {
    self->bound = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  }

  if (self->running) {
    self->running = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  }

  g_list_store_remove_all(self->param_infos);
  g_list_store_remove_all(self->params);
}

static void
pwg_node_update_param_infos(PwgNode *self, GArray *param_infos)
{
  g_list_store_remove_all(self->param_infos);

  if (param_infos != NULL) {
    for (unsigned int i = 0; i < param_infos->len; i++) {
      PwgNodeParamInfoCopy *copy = &g_array_index(param_infos, PwgNodeParamInfoCopy, i);
      g_autoptr(PwgParamInfo) param_info = _pwg_param_info_new(copy->id, copy->flags);

      g_list_store_append(self->param_infos, param_info);
    }
  }

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_PARAM_INFOS]);
}

static gboolean
pwg_node_dispatch_event(gpointer userdata)
{
  PwgNodeEvent *event = userdata;
  PwgNode *self = event->node;

  if (!self->running || event->generation != self->generation)
    return G_SOURCE_REMOVE;

  if (event->type == PWG_NODE_EVENT_INFO) {
    pwg_node_update_param_infos(self, event->param_infos);
  } else if (event->type == PWG_NODE_EVENT_PARAM) {
    if (event->param != NULL) {
      g_list_store_append(self->params, event->param);
      g_signal_emit(self, signals[SIGNAL_PARAM], 0, event->param);
    }
  } else {
    pwg_node_stop(self);
  }

  return G_SOURCE_REMOVE;
}

static void
pwg_node_on_core_connected_notify(GObject *object,
                                  GParamSpec *pspec,
                                  gpointer userdata)
{
  PwgNode *self = PWG_NODE(userdata);

  (void) pspec;

  if (!pwg_core_get_connected(PWG_CORE(object)))
    pwg_node_reset(self);
}

static void
pwg_node_get_property(GObject *object,
                      unsigned int property_id,
                      GValue *value,
                      GParamSpec *pspec)
{
  PwgNode *self = PWG_NODE(object);

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
  case PROP_PARAM_INFOS:
    g_value_set_object(value, self->param_infos);
    break;
  case PROP_PARAMS:
    g_value_set_object(value, self->params);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_node_set_property(GObject *object,
                      unsigned int property_id,
                      const GValue *value,
                      GParamSpec *pspec)
{
  PwgNode *self = PWG_NODE(object);

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
pwg_node_constructed(GObject *object)
{
  PwgNode *self = PWG_NODE(object);

  G_OBJECT_CLASS(pwg_node_parent_class)->constructed(object);

  if (self->core != NULL) {
    self->core_connected_notify_id = g_signal_connect(
      self->core,
      "notify::connected",
      G_CALLBACK(pwg_node_on_core_connected_notify),
      self);
  }

  if (self->global != NULL)
    self->node_id = pwg_global_get_id(self->global);
}

static void
pwg_node_dispose(GObject *object)
{
  PwgNode *self = PWG_NODE(object);

  pwg_node_stop(self);

  if (self->core != NULL && self->core_connected_notify_id != 0) {
    g_signal_handler_disconnect(self->core, self->core_connected_notify_id);
    self->core_connected_notify_id = 0;
  }

  g_clear_object(&self->core);
  g_clear_object(&self->global);
  g_clear_object(&self->param_infos);
  g_clear_object(&self->params);

  G_OBJECT_CLASS(pwg_node_parent_class)->dispose(object);
}

static void
pwg_node_finalize(GObject *object)
{
  PwgNode *self = PWG_NODE(object);

  g_clear_pointer(&self->main_context, g_main_context_unref);

  G_OBJECT_CLASS(pwg_node_parent_class)->finalize(object);
}

static void
pwg_node_class_init(PwgNodeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = pwg_node_constructed;
  object_class->get_property = pwg_node_get_property;
  object_class->set_property = pwg_node_set_property;
  object_class->dispose = pwg_node_dispose;
  object_class->finalize = pwg_node_finalize;

  /**
   * PwgNode:core:
   *
   * PipeWire core used by this node wrapper.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_CORE] = g_param_spec_object(
    "core",
    "Core",
    "PipeWire core used by this node wrapper.",
    PWG_TYPE_CORE,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgNode:global:
   *
   * PipeWire node global used by this node wrapper.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_GLOBAL] = g_param_spec_object(
    "global",
    "Global",
    "PipeWire node global used by this node wrapper.",
    PWG_TYPE_GLOBAL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgNode:running:
   *
   * Whether the node wrapper is running.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_RUNNING] = g_param_spec_boolean(
    "running",
    "Running",
    "Whether the node wrapper is running.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgNode:bound:
   *
   * Whether the node proxy is currently bound.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_BOUND] = g_param_spec_boolean(
    "bound",
    "Bound",
    "Whether the node proxy is currently bound.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgNode:param-infos:
   *
   * A [iface@Gio.ListModel] of [class@Pwg.ParamInfo] objects.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_PARAM_INFOS] = g_param_spec_object(
    "param-infos",
    "Param infos",
    "A GListModel of PwgParamInfo objects.",
    G_TYPE_LIST_MODEL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgNode:params:
   *
   * A [iface@Gio.ListModel] of [class@Pwg.Param] objects from the most recent
   * enumeration request.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_PARAMS] = g_param_spec_object(
    "params",
    "Params",
    "A GListModel of PwgParam objects from the most recent enumeration request.",
    G_TYPE_LIST_MODEL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);

  /**
   * PwgNode::param:
   * @self: the node wrapper.
   * @param: the copied parameter returned by enumeration or subscription.
   *
   * Emitted from the object's construction thread-default main context when a
   * node parameter enumeration result or subscribed update arrives.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  signals[SIGNAL_PARAM] = g_signal_new(
    "param",
    G_TYPE_FROM_CLASS(klass),
    G_SIGNAL_RUN_LAST,
    0,
    NULL,
    NULL,
    NULL,
    G_TYPE_NONE,
    1,
    PWG_TYPE_PARAM);
}

static void
pwg_node_init(PwgNode *self)
{
  self->param_infos = g_list_store_new(PWG_TYPE_PARAM_INFO);
  self->params = g_list_store_new(PWG_TYPE_PARAM);
  self->main_context = g_main_context_ref_thread_default();
  self->next_seq = 1;
}

PwgNode *
pwg_node_new(PwgCore *core, PwgGlobal *global)
{
  g_return_val_if_fail(PWG_IS_CORE(core), NULL);
  g_return_val_if_fail(PWG_IS_GLOBAL(global), NULL);

  if (!pwg_global_is_node(global))
    return NULL;

  return g_object_new(PWG_TYPE_NODE, "core", core, "global", global, NULL);
}

bool
pwg_node_start(PwgNode *self, GError **error)
{
  struct pw_thread_loop *thread_loop;
  struct pw_core *core;
  unsigned int version;
  int result;

  g_return_val_if_fail(PWG_IS_NODE(self), FALSE);

  if (self->running)
    return TRUE;

  if (self->core == NULL || self->global == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Node has no PipeWire core or global");
    return FALSE;
  }

  if (!pwg_global_is_node(self->global)) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Global is not a PipeWire node");
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

  version = pwg_global_get_version(self->global);
  self->node = pw_registry_bind(
    self->registry,
    pwg_global_get_id(self->global),
    PW_TYPE_INTERFACE_Node,
    version < PW_VERSION_NODE ? version : PW_VERSION_NODE,
    0);
  if (self->node == NULL) {
    pw_thread_loop_unlock(thread_loop);
    pwg_node_destroy_pipewire_objects(self);
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not bind PipeWire node");
    return FALSE;
  }

  spa_zero(self->node_listener);
  result = pw_node_add_listener(self->node, &self->node_listener, &node_events, self);
  if (result < 0) {
    pw_thread_loop_unlock(thread_loop);
    pwg_node_destroy_pipewire_objects(self);
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not listen to PipeWire node: %s",
      g_strerror(-result));
    return FALSE;
  }

  self->has_node_listener = TRUE;
  self->generation++;
  self->running = TRUE;
  self->bound = TRUE;
  pw_thread_loop_unlock(thread_loop);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  return TRUE;
}

bool
pwg_node_sync(PwgNode *self, unsigned int timeout_msec, GError **error)
{
  g_return_val_if_fail(PWG_IS_NODE(self), false);

  if (!self->running && !pwg_node_start(self, error))
    return false;

  if (self->core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Node has no PipeWire core");
    return false;
  }

  return pwg_core_sync_main_context_internal(self->core, self->main_context, timeout_msec, error);
}

void
pwg_node_stop(PwgNode *self)
{
  g_return_if_fail(PWG_IS_NODE(self));

  if (!self->running && self->registry == NULL && self->node == NULL)
    return;

  self->generation++;
  pwg_node_destroy_pipewire_objects(self);

  if (self->bound) {
    self->bound = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BOUND]);
  }

  if (self->running) {
    self->running = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  }

  g_list_store_remove_all(self->param_infos);
  g_list_store_remove_all(self->params);
}

PwgCore *
pwg_node_get_core(PwgNode *self)
{
  g_return_val_if_fail(PWG_IS_NODE(self), NULL);

  return self->core;
}

PwgGlobal *
pwg_node_get_global(PwgNode *self)
{
  g_return_val_if_fail(PWG_IS_NODE(self), NULL);

  return self->global;
}

bool
pwg_node_get_running(PwgNode *self)
{
  g_return_val_if_fail(PWG_IS_NODE(self), FALSE);

  return self->running;
}

bool
pwg_node_get_bound(PwgNode *self)
{
  g_return_val_if_fail(PWG_IS_NODE(self), FALSE);

  return self->bound;
}

GListModel *
pwg_node_get_param_infos(PwgNode *self)
{
  g_return_val_if_fail(PWG_IS_NODE(self), NULL);

  return G_LIST_MODEL(self->param_infos);
}

GListModel *
pwg_node_get_params(PwgNode *self)
{
  g_return_val_if_fail(PWG_IS_NODE(self), NULL);

  return G_LIST_MODEL(self->params);
}

bool
pwg_node_subscribe_params(PwgNode *self, GVariant *ids, GError **error)
{
  struct pw_thread_loop *thread_loop;
  g_autofree uint32_t *param_ids = NULL;
  gsize n_ids;
  int result;

  g_return_val_if_fail(PWG_IS_NODE(self), FALSE);
  g_return_val_if_fail(ids != NULL, FALSE);

  if (!g_variant_is_of_type(ids, G_VARIANT_TYPE("au"))) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Parameter ids must be a uint32 array variant");
    return FALSE;
  }

  if (self->node == NULL || !self->bound) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Node object is not bound");
    return FALSE;
  }

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return FALSE;
  }

  n_ids = g_variant_n_children(ids);
  if (n_ids > G_MAXUINT32) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Too many parameter ids");
    return FALSE;
  }

  if (n_ids > 0) {
    param_ids = g_new(uint32_t, n_ids);
    for (gsize i = 0; i < n_ids; i++)
      g_variant_get_child(ids, i, "u", &param_ids[i]);
  }

  pw_thread_loop_lock(thread_loop);
  result = pw_node_subscribe_params(self->node, param_ids, (uint32_t) n_ids);
  pw_thread_loop_unlock(thread_loop);

  if (result < 0) {
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not subscribe to PipeWire node params: %s",
      g_strerror(-result));
    return FALSE;
  }

  return TRUE;
}

int
pwg_node_enum_params(PwgNode *self,
                     unsigned int id,
                     unsigned int start,
                     unsigned int num,
                     GError **error)
{
  struct pw_thread_loop *thread_loop;
  int seq;
  int result;

  g_return_val_if_fail(PWG_IS_NODE(self), -1);

  if (self->node == NULL || !self->bound) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Node object is not bound");
    return -1;
  }

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return -1;
  }

  g_list_store_remove_all(self->params);

  seq = self->next_seq++;
  pw_thread_loop_lock(thread_loop);
  result = pw_node_enum_params(self->node, seq, id, start, num == 0 ? UINT32_MAX : num, NULL);
  pw_thread_loop_unlock(thread_loop);

  if (result < 0) {
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not enumerate PipeWire node params: %s",
      g_strerror(-result));
    return -1;
  }

  return SPA_RESULT_IS_ASYNC(result) ? result : seq;
}

static GListModel *
pwg_node_dup_params_for_seq(PwgNode *self, int seq)
{
  g_autoptr(GListStore) params = NULL;
  unsigned int n_items;

  params = g_list_store_new(PWG_TYPE_PARAM);
  n_items = g_list_model_get_n_items(G_LIST_MODEL(self->params));
  for (unsigned int i = 0; i < n_items; i++) {
    g_autoptr(PwgParam) param = g_list_model_get_item(G_LIST_MODEL(self->params), i);

    if (pwg_param_get_seq(param) == seq)
      g_list_store_append(params, param);
  }

  return G_LIST_MODEL(g_steal_pointer(&params));
}

GListModel *
pwg_node_enum_params_sync(PwgNode *self,
                          unsigned int id,
                          unsigned int start,
                          unsigned int num,
                          unsigned int timeout_msec,
                          GError **error)
{
  int seq;

  g_return_val_if_fail(PWG_IS_NODE(self), NULL);

  if (!self->running && !pwg_node_start(self, error))
    return NULL;

  seq = pwg_node_enum_params(self, id, start, num, error);
  if (seq < 0)
    return NULL;

  if (!pwg_node_sync(self, timeout_msec, error))
    return NULL;

  return pwg_node_dup_params_for_seq(self, seq);
}

int
pwg_node_enum_all_params(PwgNode *self, GError **error)
{
  return pwg_node_enum_params(self, PW_ID_ANY, 0, 0, error);
}

bool
pwg_node_set_param(PwgNode *self, PwgParam *param, GError **error)
{
  struct pw_thread_loop *thread_loop;
  const struct spa_pod *pod;
  int result;

  g_return_val_if_fail(PWG_IS_NODE(self), FALSE);
  g_return_val_if_fail(PWG_IS_PARAM(param), FALSE);

  if (self->node == NULL || !self->bound) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Node object is not bound");
    return FALSE;
  }

  thread_loop = self->core != NULL ? pwg_core_get_thread_loop_internal(self->core) : NULL;
  if (thread_loop == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "PipeWire core is not connected");
    return FALSE;
  }

  pod = _pwg_param_get_pod(param);
  if (pod == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_FAILED, "Parameter has no valid SPA POD");
    return FALSE;
  }

  pw_thread_loop_lock(thread_loop);
  result = pw_node_set_param(self->node, pwg_param_get_id(param), 0, pod);
  pw_thread_loop_unlock(thread_loop);

  if (result < 0) {
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_PIPEWIRE,
      "Could not set PipeWire node param: %s",
      g_strerror(-result));
    return FALSE;
  }

  return TRUE;
}
