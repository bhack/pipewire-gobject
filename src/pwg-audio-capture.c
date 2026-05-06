#include "pwg-audio-capture.h"

#include "pwg-stream.h"
#include "pwg.h"

struct _PwgAudioCapture {
  GObject parent_instance;
  char *target_object;
  gboolean monitor;
  PwgStream *stream;
  gulong level_handler_id;
  gulong running_notify_handler_id;
  gulong rate_notify_handler_id;
  gulong channels_notify_handler_id;
  gulong peak_notify_handler_id;
};

G_DEFINE_TYPE(PwgAudioCapture, pwg_audio_capture, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_TARGET_OBJECT,
  PROP_MONITOR,
  PROP_RUNNING,
  PROP_RATE,
  PROP_CHANNELS,
  PROP_PEAK,
  N_PROPS,
};

enum {
  SIGNAL_LEVEL,
  N_SIGNALS,
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static void
pwg_audio_capture_on_stream_level(PwgStream *stream, gdouble peak, gpointer userdata)
{
  PwgAudioCapture *self = PWG_AUDIO_CAPTURE(userdata);

  (void)stream;

  g_signal_emit(self, signals[SIGNAL_LEVEL], 0, peak);
}

static void
pwg_audio_capture_on_stream_notify(GObject *object, GParamSpec *pspec, gpointer userdata)
{
  PwgAudioCapture *self = PWG_AUDIO_CAPTURE(userdata);
  const char *name = g_param_spec_get_name(pspec);

  (void)object;

  if (g_str_equal(name, "running"))
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  else if (g_str_equal(name, "rate"))
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RATE]);
  else if (g_str_equal(name, "channels"))
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_CHANNELS]);
  else if (g_str_equal(name, "peak"))
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_PEAK]);
}

static gulong
pwg_audio_capture_connect_notify(PwgAudioCapture *self, const char *name)
{
  char *detailed_signal = g_strdup_printf("notify::%s", name);
  gulong handler_id;

  handler_id = g_signal_connect(
    self->stream,
    detailed_signal,
    G_CALLBACK(pwg_audio_capture_on_stream_notify),
    self);
  g_free(detailed_signal);

  return handler_id;
}

static void
pwg_audio_capture_get_property(GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec)
{
  PwgAudioCapture *self = PWG_AUDIO_CAPTURE(object);

  switch (property_id) {
  case PROP_TARGET_OBJECT:
    g_value_set_string(value, self->target_object);
    break;
  case PROP_MONITOR:
    g_value_set_boolean(value, self->monitor);
    break;
  case PROP_RUNNING:
    g_value_set_boolean(value, pwg_audio_capture_get_running(self));
    break;
  case PROP_RATE:
    g_value_set_uint(value, pwg_audio_capture_get_rate(self));
    break;
  case PROP_CHANNELS:
    g_value_set_uint(value, pwg_audio_capture_get_channels(self));
    break;
  case PROP_PEAK:
    g_value_set_double(value, pwg_audio_capture_get_peak(self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_audio_capture_set_property(GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
  PwgAudioCapture *self = PWG_AUDIO_CAPTURE(object);

  switch (property_id) {
  case PROP_TARGET_OBJECT:
    g_free(self->target_object);
    self->target_object = g_value_dup_string(value);
    break;
  case PROP_MONITOR:
    self->monitor = g_value_get_boolean(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_audio_capture_constructed(GObject *object)
{
  PwgAudioCapture *self = PWG_AUDIO_CAPTURE(object);

  G_OBJECT_CLASS(pwg_audio_capture_parent_class)->constructed(object);

  self->stream = pwg_stream_new_audio_capture(self->target_object, self->monitor);
  self->level_handler_id = g_signal_connect(
    self->stream,
    "level",
    G_CALLBACK(pwg_audio_capture_on_stream_level),
    self);
  self->running_notify_handler_id =
    pwg_audio_capture_connect_notify(self, "running");
  self->rate_notify_handler_id =
    pwg_audio_capture_connect_notify(self, "rate");
  self->channels_notify_handler_id =
    pwg_audio_capture_connect_notify(self, "channels");
  self->peak_notify_handler_id =
    pwg_audio_capture_connect_notify(self, "peak");
}

static void
pwg_audio_capture_disconnect_stream(PwgAudioCapture *self)
{
  if (self->stream == NULL)
    return;

  if (self->level_handler_id != 0)
    g_signal_handler_disconnect(self->stream, self->level_handler_id);
  self->level_handler_id = 0;

  if (self->running_notify_handler_id != 0)
    g_signal_handler_disconnect(self->stream, self->running_notify_handler_id);
  self->running_notify_handler_id = 0;

  if (self->rate_notify_handler_id != 0)
    g_signal_handler_disconnect(self->stream, self->rate_notify_handler_id);
  self->rate_notify_handler_id = 0;

  if (self->channels_notify_handler_id != 0)
    g_signal_handler_disconnect(self->stream, self->channels_notify_handler_id);
  self->channels_notify_handler_id = 0;

  if (self->peak_notify_handler_id != 0)
    g_signal_handler_disconnect(self->stream, self->peak_notify_handler_id);
  self->peak_notify_handler_id = 0;
}

static void
pwg_audio_capture_dispose(GObject *object)
{
  PwgAudioCapture *self = PWG_AUDIO_CAPTURE(object);

  pwg_audio_capture_stop(self);
  pwg_audio_capture_disconnect_stream(self);
  g_clear_object(&self->stream);

  G_OBJECT_CLASS(pwg_audio_capture_parent_class)->dispose(object);
}

static void
pwg_audio_capture_finalize(GObject *object)
{
  PwgAudioCapture *self = PWG_AUDIO_CAPTURE(object);

  g_clear_pointer(&self->target_object, g_free);

  G_OBJECT_CLASS(pwg_audio_capture_parent_class)->finalize(object);
}

static void
pwg_audio_capture_class_init(PwgAudioCaptureClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_audio_capture_get_property;
  object_class->set_property = pwg_audio_capture_set_property;
  object_class->constructed = pwg_audio_capture_constructed;
  object_class->dispose = pwg_audio_capture_dispose;
  object_class->finalize = pwg_audio_capture_finalize;

  /**
   * PwgAudioCapture:target-object:
   *
   * PipeWire `node.name` or object serial to capture from.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_TARGET_OBJECT] = g_param_spec_string(
    "target-object",
    "Target object",
    "PipeWire node.name or object serial to capture from.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioCapture:monitor:
   *
   * Whether to capture sink monitor audio instead of source audio.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_MONITOR] = g_param_spec_boolean(
    "monitor",
    "Monitor",
    "Whether to capture sink monitor audio instead of source audio.",
    FALSE,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioCapture:running:
   *
   * Whether the capture stream is running.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_RUNNING] = g_param_spec_boolean(
    "running",
    "Running",
    "Whether the capture stream is running.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioCapture:rate:
   *
   * Negotiated sample rate, or 0 before format negotiation.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_RATE] = g_param_spec_uint(
    "rate",
    "Rate",
    "Negotiated sample rate.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioCapture:channels:
   *
   * Negotiated channel count, or 0 before format negotiation.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_CHANNELS] = g_param_spec_uint(
    "channels",
    "Channels",
    "Negotiated channel count.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioCapture:peak:
   *
   * Most recent absolute sample peak.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_PEAK] = g_param_spec_double(
    "peak",
    "Peak",
    "Most recent absolute sample peak.",
    0.0,
    1.0,
    0.0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);

  /**
   * PwgAudioCapture::level:
   * @self: the capture object.
   * @peak: the latest absolute sample peak.
   *
   * Emitted from the object's construction thread-default main context when a
   * new peak value is available.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  signals[SIGNAL_LEVEL] = g_signal_new(
    "level",
    G_TYPE_FROM_CLASS(klass),
    G_SIGNAL_RUN_LAST,
    0,
    NULL,
    NULL,
    NULL,
    G_TYPE_NONE,
    1,
    G_TYPE_DOUBLE);
}

static void
pwg_audio_capture_init(PwgAudioCapture *self)
{
  (void)self;

  pwg_init();
}

PwgAudioCapture *
pwg_audio_capture_new(const char *target_object, gboolean monitor)
{
  return g_object_new(
    PWG_TYPE_AUDIO_CAPTURE,
    "target-object", target_object,
    "monitor", monitor,
    NULL);
}

gboolean
pwg_audio_capture_start(PwgAudioCapture *self, GError **error)
{
  g_return_val_if_fail(PWG_IS_AUDIO_CAPTURE(self), FALSE);

  return pwg_stream_start(self->stream, error);
}

void
pwg_audio_capture_stop(PwgAudioCapture *self)
{
  g_return_if_fail(PWG_IS_AUDIO_CAPTURE(self));

  if (self->stream != NULL)
    pwg_stream_stop(self->stream);
}

gboolean
pwg_audio_capture_get_running(PwgAudioCapture *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_CAPTURE(self), FALSE);

  return self->stream != NULL && pwg_stream_get_running(self->stream);
}

const char *
pwg_audio_capture_get_target_object(PwgAudioCapture *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_CAPTURE(self), NULL);

  return self->target_object;
}

gboolean
pwg_audio_capture_get_monitor(PwgAudioCapture *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_CAPTURE(self), FALSE);

  return self->monitor;
}

guint
pwg_audio_capture_get_rate(PwgAudioCapture *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_CAPTURE(self), 0);

  return self->stream != NULL ? pwg_stream_get_rate(self->stream) : 0;
}

guint
pwg_audio_capture_get_channels(PwgAudioCapture *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_CAPTURE(self), 0);

  return self->stream != NULL ? pwg_stream_get_channels(self->stream) : 0;
}

gdouble
pwg_audio_capture_get_peak(PwgAudioCapture *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_CAPTURE(self), 0.0);

  return self->stream != NULL ? pwg_stream_get_peak(self->stream) : 0.0;
}
