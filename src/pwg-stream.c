#include "pwg-stream.h"

#include <math.h>
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

#include "pwg-error.h"
#include "pwg.h"

#define PWG_STREAM_MAX_PENDING_BLOCKS 64
#define PWG_STREAM_DEFAULT_SAMPLE_FORMAT "F32"
#define PWG_STREAM_DEFAULT_RATE 48000
#define PWG_STREAM_DEFAULT_CHANNELS 2

typedef struct {
  GBytes *data;
  char *sample_format;
  guint rate;
  guint channels;
  guint bytes_per_sample;
  guint n_frames;
  guint64 sequence;
  gdouble peak;
} PwgStreamPendingBlock;

struct _PwgStream {
  GObject parent_instance;
  char *target_object;
  gboolean monitor;
  char *requested_sample_format;
  guint requested_rate;
  guint requested_channels;
  gint deliver_audio_blocks;
  gboolean running;
  guint rate;
  guint channels;
  gdouble peak;
  PwgAudioFormat *audio_format;
  struct spa_audio_info format;
  struct pw_thread_loop *thread_loop;
  struct pw_context *context;
  struct pw_core *core;
  struct pw_stream *stream;
  struct spa_hook stream_listener;
  GMainContext *main_context;
  GMutex dispatch_lock;
  gboolean dispatch_pending;
  gboolean has_pending_peak;
  gdouble pending_peak;
  gboolean has_pending_format;
  char *pending_sample_format;
  guint pending_rate;
  guint pending_channels;
  guint pending_bytes_per_sample;
  GQueue pending_blocks;
  guint64 next_sequence;
};

G_DEFINE_TYPE(PwgStream, pwg_stream, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_TARGET_OBJECT,
  PROP_MONITOR,
  PROP_DELIVER_AUDIO_BLOCKS,
  PROP_RUNNING,
  PROP_RATE,
  PROP_CHANNELS,
  PROP_PEAK,
  PROP_AUDIO_FORMAT,
  N_PROPS,
};

enum {
  SIGNAL_LEVEL,
  SIGNAL_AUDIO_BLOCK,
  N_SIGNALS,
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static void
pwg_stream_pending_block_free(PwgStreamPendingBlock *block)
{
  if (block == NULL)
    return;

  g_clear_pointer(&block->data, g_bytes_unref);
  g_clear_pointer(&block->sample_format, g_free);
  g_free(block);
}

static void
pwg_stream_clear_pending_blocks(GQueue *queue)
{
  PwgStreamPendingBlock *block;

  while ((block = g_queue_pop_head(queue)) != NULL)
    pwg_stream_pending_block_free(block);
}

static const char *
pwg_stream_sample_format_name(enum spa_audio_format format)
{
  switch (format) {
  case SPA_AUDIO_FORMAT_F32:
    return "F32";
  default:
    return "unknown";
  }
}

static guint
pwg_stream_sample_format_bytes_per_sample(enum spa_audio_format format)
{
  switch (format) {
  case SPA_AUDIO_FORMAT_F32:
    return sizeof(float);
  default:
    return 0;
  }
}

static gboolean
pwg_stream_sample_format_from_name(const char *sample_format,
                                   enum spa_audio_format *format,
                                   GError **error)
{
  if (sample_format == NULL || sample_format[0] == '\0') {
    g_set_error_literal(
      error,
      PWG_ERROR,
      PWG_ERROR_FAILED,
      "Requested sample format must not be empty");
    return FALSE;
  }

  if (g_strcmp0(sample_format, "F32") == 0) {
    if (format != NULL)
      *format = SPA_AUDIO_FORMAT_F32;
    return TRUE;
  }

  g_set_error(
    error,
    PWG_ERROR,
    PWG_ERROR_FAILED,
    "Unsupported requested sample format '%s'",
    sample_format);
  return FALSE;
}

static gboolean
pwg_stream_validate_requested_format(const char *sample_format,
                                     guint rate,
                                     guint channels,
                                     enum spa_audio_format *format,
                                     GError **error)
{
  if (!pwg_stream_sample_format_from_name(sample_format, format, error))
    return FALSE;

  if (rate == 0) {
    g_set_error_literal(
      error,
      PWG_ERROR,
      PWG_ERROR_FAILED,
      "Requested sample rate must be greater than zero");
    return FALSE;
  }

  if (channels != 1 && channels != 2) {
    g_set_error(
      error,
      PWG_ERROR,
      PWG_ERROR_FAILED,
      "Unsupported requested channel count %u",
      channels);
    return FALSE;
  }

  return TRUE;
}

static void
pwg_stream_set_channel_positions(struct spa_audio_info_raw *format)
{
  if (format->channels == 1) {
    format->position[0] = SPA_AUDIO_CHANNEL_MONO;
  } else if (format->channels == 2) {
    format->position[0] = SPA_AUDIO_CHANNEL_FL;
    format->position[1] = SPA_AUDIO_CHANNEL_FR;
  }
}

static void
pwg_stream_emit_peak(PwgStream *self, gdouble peak)
{
  self->peak = peak;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_PEAK]);
  g_signal_emit(self, signals[SIGNAL_LEVEL], 0, peak);
}

static void
pwg_stream_emit_audio_block(PwgStream *self, PwgStreamPendingBlock *pending)
{
  g_autoptr(PwgAudioFormat) format = NULL;
  g_autoptr(PwgAudioBlock) block = NULL;

  format = pwg_audio_format_new(
    pending->sample_format,
    pending->rate,
    pending->channels,
    pending->bytes_per_sample);
  if (format == NULL)
    return;

  block = pwg_audio_block_new(
    format,
    pending->data,
    pending->n_frames,
    pending->sequence,
    pending->peak);
  if (block == NULL)
    return;

  g_signal_emit(self, signals[SIGNAL_AUDIO_BLOCK], 0, block);
}

static gboolean
pwg_stream_dispatch_pending(gpointer userdata)
{
  PwgStream *self = PWG_STREAM(userdata);
  GQueue blocks = G_QUEUE_INIT;
  PwgStreamPendingBlock *block;
  gboolean has_peak;
  gboolean has_format;
  char *sample_format;
  gdouble peak;
  guint rate;
  guint channels;
  guint bytes_per_sample;

  g_mutex_lock(&self->dispatch_lock);
  has_peak = self->has_pending_peak;
  has_format = self->has_pending_format;
  sample_format = g_steal_pointer(&self->pending_sample_format);
  peak = self->pending_peak;
  rate = self->pending_rate;
  channels = self->pending_channels;
  bytes_per_sample = self->pending_bytes_per_sample;
  while ((block = g_queue_pop_head(&self->pending_blocks)) != NULL)
    g_queue_push_tail(&blocks, block);
  self->dispatch_pending = FALSE;
  self->has_pending_peak = FALSE;
  self->pending_peak = 0.0;
  self->has_pending_format = FALSE;
  g_mutex_unlock(&self->dispatch_lock);

  if (has_format) {
    g_autoptr(PwgAudioFormat) audio_format = NULL;

    self->rate = rate;
    self->channels = channels;
    audio_format = pwg_audio_format_new(
      sample_format != NULL ? sample_format : "unknown",
      rate,
      channels,
      bytes_per_sample);
    g_set_object(&self->audio_format, audio_format);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RATE]);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_CHANNELS]);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_AUDIO_FORMAT]);
  }
  g_free(sample_format);

  if (has_peak && self->running)
    pwg_stream_emit_peak(self, peak);

  while ((block = g_queue_pop_head(&blocks)) != NULL) {
    if (self->running)
      pwg_stream_emit_audio_block(self, block);
    pwg_stream_pending_block_free(block);
  }

  return G_SOURCE_REMOVE;
}

static void
pwg_stream_queue_dispatch_locked(PwgStream *self)
{
  GSource *source;

  if (self->dispatch_pending)
    return;

  self->dispatch_pending = TRUE;
  source = g_idle_source_new();
  g_source_set_priority(source, G_PRIORITY_DEFAULT);
  g_source_set_callback(
    source,
    pwg_stream_dispatch_pending,
    g_object_ref(self),
    g_object_unref);
  g_source_attach(source, self->main_context);
  g_source_unref(source);
}

static void
pwg_stream_queue_peak(PwgStream *self, gdouble peak)
{
  g_mutex_lock(&self->dispatch_lock);
  self->pending_peak = self->has_pending_peak ? MAX(self->pending_peak, peak) : peak;
  self->has_pending_peak = TRUE;
  pwg_stream_queue_dispatch_locked(self);
  g_mutex_unlock(&self->dispatch_lock);
}

static void
pwg_stream_queue_format(PwgStream *self,
                        const char *sample_format,
                        guint rate,
                        guint channels,
                        guint bytes_per_sample)
{
  g_mutex_lock(&self->dispatch_lock);
  g_free(self->pending_sample_format);
  self->pending_sample_format = g_strdup(sample_format);
  self->pending_rate = rate;
  self->pending_channels = channels;
  self->pending_bytes_per_sample = bytes_per_sample;
  self->has_pending_format = TRUE;
  pwg_stream_queue_dispatch_locked(self);
  g_mutex_unlock(&self->dispatch_lock);
}

static void
pwg_stream_queue_audio_block(PwgStream *self,
                             const void *data,
                             gsize size,
                             const char *sample_format,
                             guint rate,
                             guint channels,
                             guint bytes_per_sample,
                             guint n_frames,
                             gdouble peak)
{
  PwgStreamPendingBlock *block;

  if (data == NULL || size == 0)
    return;

  block = g_new0(PwgStreamPendingBlock, 1);
  block->data = g_bytes_new(data, size);
  block->sample_format = g_strdup(sample_format);
  block->rate = rate;
  block->channels = channels;
  block->bytes_per_sample = bytes_per_sample;
  block->n_frames = n_frames;
  block->peak = peak;

  g_mutex_lock(&self->dispatch_lock);
  block->sequence = self->next_sequence++;
  g_queue_push_tail(&self->pending_blocks, block);
  while (g_queue_get_length(&self->pending_blocks) > PWG_STREAM_MAX_PENDING_BLOCKS) {
    PwgStreamPendingBlock *dropped = g_queue_pop_head(&self->pending_blocks);

    pwg_stream_pending_block_free(dropped);
  }
  pwg_stream_queue_dispatch_locked(self);
  g_mutex_unlock(&self->dispatch_lock);
}

static void
pwg_stream_handle_f32_audio(PwgStream *self,
                            const void *data,
                            gsize size,
                            guint rate,
                            guint channels)
{
  const guint bytes_per_sample = sizeof(float);
  const char *sample_format = "F32";
  const float *samples;
  guint n_samples;
  guint n_frames;
  guint n;
  gdouble peak = 0.0;

  if (data == NULL || size < bytes_per_sample || channels == 0)
    return;

  n_samples = size / bytes_per_sample;
  n_frames = n_samples / channels;
  if (n_frames == 0)
    return;

  samples = data;
  for (n = 0; n < n_samples; n++)
    peak = MAX(peak, fabs(samples[n]));

  pwg_stream_queue_peak(self, peak);
  if (g_atomic_int_get(&self->deliver_audio_blocks))
    pwg_stream_queue_audio_block(
      self,
      data,
      n_samples * bytes_per_sample,
      sample_format,
      rate,
      channels,
      bytes_per_sample,
      n_frames,
      peak);
}

static void
pwg_stream_on_process(void *userdata)
{
  PwgStream *self = PWG_STREAM(userdata);
  struct pw_buffer *buffer;
  struct spa_buffer *spa_buffer;
  struct spa_data *spa_data;
  struct spa_chunk *chunk;
  guint8 *audio_data;
  guint n_channels;
  guint rate;

  buffer = pw_stream_dequeue_buffer(self->stream);
  if (buffer == NULL)
    return;

  spa_buffer = buffer->buffer;
  if (spa_buffer == NULL || spa_buffer->n_datas == 0 ||
      spa_buffer->datas[0].data == NULL || spa_buffer->datas[0].chunk == NULL)
    goto done;
  spa_data = &spa_buffer->datas[0];

  if (self->format.info.raw.format != SPA_AUDIO_FORMAT_F32)
    goto done;

  n_channels = self->format.info.raw.channels;
  if (n_channels == 0)
    goto done;

  chunk = spa_data->chunk;
  if (chunk->offset > spa_data->maxsize ||
      chunk->size > spa_data->maxsize - chunk->offset)
    goto done;

  audio_data = (guint8 *)spa_data->data + chunk->offset;
  rate = self->format.info.raw.rate;
  pwg_stream_handle_f32_audio(self, audio_data, chunk->size, rate, n_channels);

done:
  pw_stream_queue_buffer(self->stream, buffer);
}

#ifdef PWG_STREAM_TESTING
void
_pwg_stream_test_push_f32_audio(PwgStream *self,
                                const float *samples,
                                guint n_samples,
                                guint rate,
                                guint channels)
{
  g_return_if_fail(PWG_IS_STREAM(self));

  self->running = TRUE;
  pwg_stream_queue_format(self, "F32", rate, channels, sizeof(float));
  pwg_stream_handle_f32_audio(self, samples, n_samples * sizeof(float), rate, channels);
}
#endif

static void
pwg_stream_on_param_changed(void *userdata, uint32_t id, const struct spa_pod *param)
{
  PwgStream *self = PWG_STREAM(userdata);

  if (param == NULL || id != SPA_PARAM_Format)
    return;

  if (spa_format_parse(param, &self->format.media_type, &self->format.media_subtype) < 0)
    return;

  if (self->format.media_type != SPA_MEDIA_TYPE_audio ||
      self->format.media_subtype != SPA_MEDIA_SUBTYPE_raw)
    return;

  spa_format_audio_raw_parse(param, &self->format.info.raw);
  pwg_stream_queue_format(
    self,
    pwg_stream_sample_format_name(self->format.info.raw.format),
    self->format.info.raw.rate,
    self->format.info.raw.channels,
    pwg_stream_sample_format_bytes_per_sample(self->format.info.raw.format));
}

static const struct pw_stream_events stream_events = {
  PW_VERSION_STREAM_EVENTS,
  .param_changed = pwg_stream_on_param_changed,
  .process = pwg_stream_on_process,
};

static void
pwg_stream_get_property(GObject *object,
                        guint property_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  PwgStream *self = PWG_STREAM(object);

  switch (property_id) {
  case PROP_TARGET_OBJECT:
    g_value_set_string(value, self->target_object);
    break;
  case PROP_MONITOR:
    g_value_set_boolean(value, self->monitor);
    break;
  case PROP_DELIVER_AUDIO_BLOCKS:
    g_value_set_boolean(value, g_atomic_int_get(&self->deliver_audio_blocks));
    break;
  case PROP_RUNNING:
    g_value_set_boolean(value, self->running);
    break;
  case PROP_RATE:
    g_value_set_uint(value, self->rate);
    break;
  case PROP_CHANNELS:
    g_value_set_uint(value, self->channels);
    break;
  case PROP_PEAK:
    g_value_set_double(value, self->peak);
    break;
  case PROP_AUDIO_FORMAT:
    g_value_set_object(value, self->audio_format);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_stream_set_property(GObject *object,
                        guint property_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  PwgStream *self = PWG_STREAM(object);

  switch (property_id) {
  case PROP_TARGET_OBJECT:
    g_free(self->target_object);
    self->target_object = g_value_dup_string(value);
    break;
  case PROP_MONITOR:
    self->monitor = g_value_get_boolean(value);
    break;
  case PROP_DELIVER_AUDIO_BLOCKS:
    g_atomic_int_set(&self->deliver_audio_blocks, g_value_get_boolean(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_stream_dispose(GObject *object)
{
  PwgStream *self = PWG_STREAM(object);

  pwg_stream_stop(self);

  G_OBJECT_CLASS(pwg_stream_parent_class)->dispose(object);
}

static void
pwg_stream_finalize(GObject *object)
{
  PwgStream *self = PWG_STREAM(object);

  g_clear_pointer(&self->target_object, g_free);
  g_clear_pointer(&self->requested_sample_format, g_free);
  g_clear_object(&self->audio_format);
  g_clear_pointer(&self->pending_sample_format, g_free);
  pwg_stream_clear_pending_blocks(&self->pending_blocks);
  g_clear_pointer(&self->main_context, g_main_context_unref);
  g_mutex_clear(&self->dispatch_lock);

  G_OBJECT_CLASS(pwg_stream_parent_class)->finalize(object);
}

static void
pwg_stream_class_init(PwgStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_stream_get_property;
  object_class->set_property = pwg_stream_set_property;
  object_class->dispose = pwg_stream_dispose;
  object_class->finalize = pwg_stream_finalize;

  /**
   * PwgStream:target-object:
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
   * PwgStream:monitor:
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
   * PwgStream:deliver-audio-blocks:
   *
   * Whether to emit copied audio blocks from the stream.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_DELIVER_AUDIO_BLOCKS] = g_param_spec_boolean(
    "deliver-audio-blocks",
    "Deliver audio blocks",
    "Whether to emit copied audio blocks from the stream.",
    FALSE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgStream:running:
   *
   * Whether the stream is running.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_RUNNING] = g_param_spec_boolean(
    "running",
    "Running",
    "Whether the stream is running.",
    FALSE,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgStream:rate:
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
   * PwgStream:channels:
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
   * PwgStream:peak:
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
    G_MAXDOUBLE,
    0.0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * PwgStream:audio-format:
   *
   * Current negotiated audio format, or %NULL before format negotiation.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_AUDIO_FORMAT] = g_param_spec_object(
    "audio-format",
    "Audio format",
    "Current negotiated audio format.",
    PWG_TYPE_AUDIO_FORMAT,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);

  /**
   * PwgStream::level:
   * @self: the stream object.
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

  /**
   * PwgStream::audio-block:
   * @self: the stream object.
   * @block: copied audio sample data and format metadata.
   *
   * Emitted from the object's construction thread-default main context when a
   * copied audio block is available. Enable this signal with
   * pwg_stream_set_deliver_audio_blocks().
   *
   * Since: 0.1
   * Stability: Unstable
   */
  signals[SIGNAL_AUDIO_BLOCK] = g_signal_new(
    "audio-block",
    G_TYPE_FROM_CLASS(klass),
    G_SIGNAL_RUN_LAST,
    0,
    NULL,
    NULL,
    NULL,
    G_TYPE_NONE,
    1,
    PWG_TYPE_AUDIO_BLOCK);
}

static void
pwg_stream_init(PwgStream *self)
{
  pwg_init();
  self->requested_sample_format = g_strdup(PWG_STREAM_DEFAULT_SAMPLE_FORMAT);
  self->requested_rate = PWG_STREAM_DEFAULT_RATE;
  self->requested_channels = PWG_STREAM_DEFAULT_CHANNELS;
  self->main_context = g_main_context_ref_thread_default();
  g_mutex_init(&self->dispatch_lock);
  g_queue_init(&self->pending_blocks);
}

PwgStream *
pwg_stream_new_audio_capture(const char *target_object, gboolean monitor)
{
  return g_object_new(
    PWG_TYPE_STREAM,
    "target-object", target_object,
    "monitor", monitor,
    NULL);
}

gboolean
pwg_stream_set_requested_format(PwgStream *self,
                                const char *sample_format,
                                guint rate,
                                guint channels,
                                GError **error)
{
  enum spa_audio_format format;

  g_return_val_if_fail(PWG_IS_STREAM(self), FALSE);

  if (self->running) {
    g_set_error_literal(
      error,
      PWG_ERROR,
      PWG_ERROR_FAILED,
      "Cannot change requested stream format while the stream is running");
    return FALSE;
  }

  if (!pwg_stream_validate_requested_format(sample_format, rate, channels, &format, error))
    return FALSE;

  g_free(self->requested_sample_format);
  self->requested_sample_format = g_strdup(pwg_stream_sample_format_name(format));
  self->requested_rate = rate;
  self->requested_channels = channels;
  return TRUE;
}

gboolean
pwg_stream_start(PwgStream *self, GError **error)
{
  struct pw_properties *props;
  const struct spa_pod *params[1];
  enum spa_audio_format requested_format;
  struct spa_audio_info_raw capture_format = SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_F32);
  uint8_t pod_buffer[1024];
  struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(pod_buffer, sizeof(pod_buffer));

  g_return_val_if_fail(PWG_IS_STREAM(self), FALSE);

  if (self->running)
    return TRUE;

  if (!pwg_stream_validate_requested_format(
        self->requested_sample_format,
        self->requested_rate,
        self->requested_channels,
        &requested_format,
        error))
    return FALSE;

  capture_format.format = requested_format;
  capture_format.rate = self->requested_rate;
  capture_format.channels = self->requested_channels;
  pwg_stream_set_channel_positions(&capture_format);

  self->thread_loop = pw_thread_loop_new("pwg-stream", NULL);
  if (self->thread_loop == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not create PipeWire thread loop");
    return FALSE;
  }

  self->context = pw_context_new(pw_thread_loop_get_loop(self->thread_loop), NULL, 0);
  if (self->context == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not create PipeWire context");
    pwg_stream_stop(self);
    return FALSE;
  }

  self->core = pw_context_connect(self->context, NULL, 0);
  if (self->core == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not connect to PipeWire core");
    pwg_stream_stop(self);
    return FALSE;
  }

  props = pw_properties_new(
    PW_KEY_MEDIA_TYPE, "Audio",
    PW_KEY_MEDIA_CATEGORY, "Capture",
    PW_KEY_MEDIA_ROLE, "Music",
    PW_KEY_MEDIA_NAME, "Pwg audio stream",
    NULL);
  if (props == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_NO_MEMORY, "Could not create PipeWire properties");
    pwg_stream_stop(self);
    return FALSE;
  }

  if (self->target_object != NULL && self->target_object[0] != '\0')
    pw_properties_set(props, PW_KEY_TARGET_OBJECT, self->target_object);
  if (self->monitor)
    pw_properties_set(props, PW_KEY_STREAM_CAPTURE_SINK, "true");

  self->stream = pw_stream_new(self->core, "pwg-stream", props);
  if (self->stream == NULL) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not create PipeWire stream");
    pwg_stream_stop(self);
    return FALSE;
  }

  pw_stream_add_listener(self->stream, &self->stream_listener, &stream_events, self);

  params[0] = spa_format_audio_raw_build(
    &builder,
    SPA_PARAM_EnumFormat,
    &capture_format);

  if (pw_stream_connect(
        self->stream,
        PW_DIRECTION_INPUT,
        PW_ID_ANY,
        PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS,
        params,
        1) < 0) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not connect PipeWire stream");
    pwg_stream_stop(self);
    return FALSE;
  }

  if (pw_thread_loop_start(self->thread_loop) < 0) {
    g_set_error_literal(error, PWG_ERROR, PWG_ERROR_PIPEWIRE, "Could not start PipeWire thread loop");
    pwg_stream_stop(self);
    return FALSE;
  }

  self->running = TRUE;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
  return TRUE;
}

void
pwg_stream_stop(PwgStream *self)
{
  gboolean was_running;

  g_return_if_fail(PWG_IS_STREAM(self));

  was_running = self->running;
  self->running = FALSE;

  if (self->thread_loop != NULL)
    pw_thread_loop_stop(self->thread_loop);

  if (self->stream != NULL) {
    pw_stream_destroy(self->stream);
    self->stream = NULL;
  }

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

  g_mutex_lock(&self->dispatch_lock);
  self->has_pending_peak = FALSE;
  self->pending_peak = 0.0;
  self->has_pending_format = FALSE;
  g_clear_pointer(&self->pending_sample_format, g_free);
  pwg_stream_clear_pending_blocks(&self->pending_blocks);
  g_mutex_unlock(&self->dispatch_lock);

  if (was_running)
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_RUNNING]);
}

gboolean
pwg_stream_get_running(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), FALSE);

  return self->running;
}

const char *
pwg_stream_get_target_object(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), NULL);

  return self->target_object;
}

gboolean
pwg_stream_get_monitor(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), FALSE);

  return self->monitor;
}

const char *
pwg_stream_get_requested_sample_format(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), NULL);

  return self->requested_sample_format;
}

guint
pwg_stream_get_requested_rate(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), 0);

  return self->requested_rate;
}

guint
pwg_stream_get_requested_channels(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), 0);

  return self->requested_channels;
}

guint
pwg_stream_get_rate(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), 0);

  return self->rate;
}

guint
pwg_stream_get_channels(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), 0);

  return self->channels;
}

gdouble
pwg_stream_get_peak(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), 0.0);

  return self->peak;
}

PwgAudioFormat *
pwg_stream_get_audio_format(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), NULL);

  return self->audio_format;
}

gboolean
pwg_stream_get_deliver_audio_blocks(PwgStream *self)
{
  g_return_val_if_fail(PWG_IS_STREAM(self), FALSE);

  return g_atomic_int_get(&self->deliver_audio_blocks);
}

void
pwg_stream_set_deliver_audio_blocks(PwgStream *self, gboolean deliver_audio_blocks)
{
  g_return_if_fail(PWG_IS_STREAM(self));

  deliver_audio_blocks = !!deliver_audio_blocks;
  if (g_atomic_int_get(&self->deliver_audio_blocks) == deliver_audio_blocks)
    return;

  g_atomic_int_set(&self->deliver_audio_blocks, deliver_audio_blocks);
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_DELIVER_AUDIO_BLOCKS]);
}
