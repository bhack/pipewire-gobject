#include "pwg-audio-format.h"

struct _PwgAudioFormat {
  GObject parent_instance;
  char *sample_format;
  unsigned int rate;
  unsigned int channels;
  unsigned int bytes_per_sample;
};

G_DEFINE_TYPE(PwgAudioFormat, pwg_audio_format, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_SAMPLE_FORMAT,
  PROP_RATE,
  PROP_CHANNELS,
  PROP_BYTES_PER_SAMPLE,
  PROP_BYTES_PER_FRAME,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static unsigned int
pwg_audio_format_calculate_bytes_per_frame(PwgAudioFormat *self)
{
  if (self->channels != 0 && self->bytes_per_sample > G_MAXUINT / self->channels)
    return 0;

  return self->bytes_per_sample * self->channels;
}

static void
pwg_audio_format_get_property(GObject *object,
                              unsigned int property_id,
                              GValue *value,
                              GParamSpec *pspec)
{
  PwgAudioFormat *self = PWG_AUDIO_FORMAT(object);

  switch (property_id) {
  case PROP_SAMPLE_FORMAT:
    g_value_set_string(value, self->sample_format);
    break;
  case PROP_RATE:
    g_value_set_uint(value, self->rate);
    break;
  case PROP_CHANNELS:
    g_value_set_uint(value, self->channels);
    break;
  case PROP_BYTES_PER_SAMPLE:
    g_value_set_uint(value, self->bytes_per_sample);
    break;
  case PROP_BYTES_PER_FRAME:
    g_value_set_uint(value, pwg_audio_format_calculate_bytes_per_frame(self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_audio_format_set_property(GObject *object,
                              unsigned int property_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
  PwgAudioFormat *self = PWG_AUDIO_FORMAT(object);

  switch (property_id) {
  case PROP_SAMPLE_FORMAT:
    g_free(self->sample_format);
    self->sample_format = g_value_dup_string(value);
    break;
  case PROP_RATE:
    self->rate = g_value_get_uint(value);
    break;
  case PROP_CHANNELS:
    self->channels = g_value_get_uint(value);
    break;
  case PROP_BYTES_PER_SAMPLE:
    self->bytes_per_sample = g_value_get_uint(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_audio_format_finalize(GObject *object)
{
  PwgAudioFormat *self = PWG_AUDIO_FORMAT(object);

  g_clear_pointer(&self->sample_format, g_free);

  G_OBJECT_CLASS(pwg_audio_format_parent_class)->finalize(object);
}

static void
pwg_audio_format_class_init(PwgAudioFormatClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_audio_format_get_property;
  object_class->set_property = pwg_audio_format_set_property;
  object_class->finalize = pwg_audio_format_finalize;

  /**
   * PwgAudioFormat:sample-format:
   *
   * PipeWire/SPA sample format name.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_SAMPLE_FORMAT] = g_param_spec_string(
    "sample-format",
    "Sample format",
    "PipeWire/SPA sample format name.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioFormat:rate:
   *
   * Sample rate in Hz.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_RATE] = g_param_spec_uint(
    "rate",
    "Rate",
    "Sample rate in Hz.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioFormat:channels:
   *
   * Channel count.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_CHANNELS] = g_param_spec_uint(
    "channels",
    "Channels",
    "Channel count.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioFormat:bytes-per-sample:
   *
   * Bytes per single-channel sample.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_BYTES_PER_SAMPLE] = g_param_spec_uint(
    "bytes-per-sample",
    "Bytes per sample",
    "Bytes per single-channel sample.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioFormat:bytes-per-frame:
   *
   * Bytes per interleaved frame.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_BYTES_PER_FRAME] = g_param_spec_uint(
    "bytes-per-frame",
    "Bytes per frame",
    "Bytes per interleaved frame.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_audio_format_init(PwgAudioFormat *self)
{
  (void)self;
}

PwgAudioFormat *
pwg_audio_format_new(const char *sample_format,
                     unsigned int rate,
                     unsigned int channels,
                     unsigned int bytes_per_sample)
{
  g_return_val_if_fail(sample_format != NULL, NULL);

  return g_object_new(
    PWG_TYPE_AUDIO_FORMAT,
    "sample-format", sample_format,
    "rate", rate,
    "channels", channels,
    "bytes-per-sample", bytes_per_sample,
    NULL);
}

const char *
pwg_audio_format_get_sample_format(PwgAudioFormat *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_FORMAT(self), NULL);

  return self->sample_format;
}

unsigned int
pwg_audio_format_get_rate(PwgAudioFormat *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_FORMAT(self), 0);

  return self->rate;
}

unsigned int
pwg_audio_format_get_channels(PwgAudioFormat *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_FORMAT(self), 0);

  return self->channels;
}

unsigned int
pwg_audio_format_get_bytes_per_sample(PwgAudioFormat *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_FORMAT(self), 0);

  return self->bytes_per_sample;
}

unsigned int
pwg_audio_format_get_bytes_per_frame(PwgAudioFormat *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_FORMAT(self), 0);

  return pwg_audio_format_calculate_bytes_per_frame(self);
}
