#include "pwg-audio-block.h"

struct _PwgAudioBlock {
  GObject parent_instance;
  PwgAudioFormat *format;
  GBytes *data;
  guint n_frames;
  guint64 sequence;
  gdouble peak;
};

G_DEFINE_TYPE(PwgAudioBlock, pwg_audio_block, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_FORMAT,
  PROP_DATA,
  PROP_N_FRAMES,
  PROP_SEQUENCE,
  PROP_PEAK,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static GBytes *
pwg_audio_block_empty_data(void)
{
  return g_bytes_new_static("", 0);
}

static void
pwg_audio_block_get_property(GObject *object,
                             guint property_id,
                             GValue *value,
                             GParamSpec *pspec)
{
  PwgAudioBlock *self = PWG_AUDIO_BLOCK(object);

  switch (property_id) {
  case PROP_FORMAT:
    g_value_set_object(value, self->format);
    break;
  case PROP_DATA:
    g_value_set_boxed(value, self->data);
    break;
  case PROP_N_FRAMES:
    g_value_set_uint(value, self->n_frames);
    break;
  case PROP_SEQUENCE:
    g_value_set_uint64(value, self->sequence);
    break;
  case PROP_PEAK:
    g_value_set_double(value, self->peak);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_audio_block_set_property(GObject *object,
                             guint property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  PwgAudioBlock *self = PWG_AUDIO_BLOCK(object);

  switch (property_id) {
  case PROP_FORMAT:
    g_set_object(&self->format, g_value_get_object(value));
    break;
  case PROP_DATA:
    g_clear_pointer(&self->data, g_bytes_unref);
    self->data = g_value_dup_boxed(value);
    if (self->data == NULL)
      self->data = pwg_audio_block_empty_data();
    break;
  case PROP_N_FRAMES:
    self->n_frames = g_value_get_uint(value);
    break;
  case PROP_SEQUENCE:
    self->sequence = g_value_get_uint64(value);
    break;
  case PROP_PEAK:
    self->peak = g_value_get_double(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
pwg_audio_block_finalize(GObject *object)
{
  PwgAudioBlock *self = PWG_AUDIO_BLOCK(object);

  g_clear_object(&self->format);
  g_clear_pointer(&self->data, g_bytes_unref);

  G_OBJECT_CLASS(pwg_audio_block_parent_class)->finalize(object);
}

static void
pwg_audio_block_class_init(PwgAudioBlockClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = pwg_audio_block_get_property;
  object_class->set_property = pwg_audio_block_set_property;
  object_class->finalize = pwg_audio_block_finalize;

  /**
   * PwgAudioBlock:format:
   *
   * Audio format descriptor for this block.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_FORMAT] = g_param_spec_object(
    "format",
    "Format",
    "Audio format for this block.",
    PWG_TYPE_AUDIO_FORMAT,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioBlock:data:
   *
   * Copied interleaved audio sample bytes.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_DATA] = g_param_spec_boxed(
    "data",
    "Data",
    "Copied interleaved audio sample bytes.",
    G_TYPE_BYTES,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioBlock:n-frames:
   *
   * Number of interleaved frames in the block.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_N_FRAMES] = g_param_spec_uint(
    "n-frames",
    "N frames",
    "Number of interleaved frames in the block.",
    0,
    G_MAXUINT,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioBlock:sequence:
   *
   * Monotonic stream-local block sequence number.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_SEQUENCE] = g_param_spec_uint64(
    "sequence",
    "Sequence",
    "Monotonic stream-local block sequence number.",
    0,
    G_MAXUINT64,
    0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * PwgAudioBlock:peak:
   *
   * Absolute sample peak in this block.
   *
   * Since: 0.1
   * Stability: Unstable
   */
  properties[PROP_PEAK] = g_param_spec_double(
    "peak",
    "Peak",
    "Absolute sample peak in this block.",
    0.0,
    G_MAXDOUBLE,
    0.0,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void
pwg_audio_block_init(PwgAudioBlock *self)
{
  (void)self;
}

PwgAudioBlock *
pwg_audio_block_new(PwgAudioFormat *format,
                    GBytes *data,
                    guint n_frames,
                    guint64 sequence,
                    gdouble peak)
{
  g_return_val_if_fail(PWG_IS_AUDIO_FORMAT(format), NULL);
  g_return_val_if_fail(data != NULL, NULL);

  return g_object_new(
    PWG_TYPE_AUDIO_BLOCK,
    "format", format,
    "data", data,
    "n-frames", n_frames,
    "sequence", sequence,
    "peak", peak,
    NULL);
}

PwgAudioFormat *
pwg_audio_block_get_format(PwgAudioBlock *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_BLOCK(self), NULL);

  return self->format;
}

GBytes *
pwg_audio_block_get_data(PwgAudioBlock *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_BLOCK(self), NULL);

  return g_bytes_ref(self->data);
}

guint
pwg_audio_block_get_n_frames(PwgAudioBlock *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_BLOCK(self), 0);

  return self->n_frames;
}

guint64
pwg_audio_block_get_sequence(PwgAudioBlock *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_BLOCK(self), 0);

  return self->sequence;
}

gdouble
pwg_audio_block_get_peak(PwgAudioBlock *self)
{
  g_return_val_if_fail(PWG_IS_AUDIO_BLOCK(self), 0.0);

  return self->peak;
}
