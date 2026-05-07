#include <math.h>

#include <glib.h>

#include "pwg-audio-block.h"
#include "pwg-audio-format.h"
#include "pwg-error.h"
#include "pwg-stream.h"

void _pwg_stream_test_push_f32_audio(PwgStream *self,
                                     const float *samples,
                                     guint n_samples,
                                     guint rate,
                                     guint channels);

typedef struct {
  GMainLoop *loop;
  PwgStream *stream;
  gboolean got_format;
  gboolean got_level;
  gboolean got_block;
  gboolean timed_out;
  gdouble peak;
} StreamDispatchFixture;

static gboolean
timeout_cb(gpointer userdata)
{
  StreamDispatchFixture *fixture = userdata;

  fixture->timed_out = TRUE;
  g_main_loop_quit(fixture->loop);
  return G_SOURCE_REMOVE;
}

static void
maybe_done(StreamDispatchFixture *fixture)
{
  if (fixture->got_format && fixture->got_level && fixture->got_block)
    g_main_loop_quit(fixture->loop);
}

static void
on_audio_format_notify(GObject *object, GParamSpec *pspec, gpointer userdata)
{
  StreamDispatchFixture *fixture = userdata;
  PwgStream *stream = PWG_STREAM(object);
  PwgAudioFormat *format;

  (void) pspec;

  format = pwg_stream_get_audio_format(stream);
  g_assert_nonnull(format);
  g_assert_cmpstr(pwg_audio_format_get_sample_format(format), ==, "F32");
  g_assert_cmpuint(pwg_audio_format_get_rate(format), ==, 48000);
  g_assert_cmpuint(pwg_audio_format_get_channels(format), ==, 2);
  g_assert_cmpuint(pwg_audio_format_get_bytes_per_sample(format), ==, sizeof(float));
  g_assert_cmpuint(pwg_audio_format_get_bytes_per_frame(format), ==, 2 * sizeof(float));

  fixture->got_format = TRUE;
  maybe_done(fixture);
}

static void
on_level(PwgStream *stream, gdouble peak, gpointer userdata)
{
  StreamDispatchFixture *fixture = userdata;

  g_assert_true(stream == fixture->stream);
  g_assert_cmpfloat(fabs(peak - 0.75), <, 0.000001);
  g_assert_cmpfloat(fabs(pwg_stream_get_peak(stream) - peak), <, 0.000001);

  fixture->got_level = TRUE;
  fixture->peak = peak;
  maybe_done(fixture);
}

static void
on_audio_block(PwgStream *stream, PwgAudioBlock *block, gpointer userdata)
{
  StreamDispatchFixture *fixture = userdata;
  PwgAudioFormat *format;
  GBytes *bytes;
  gsize size = 0;
  const float *samples;

  g_assert_true(stream == fixture->stream);
  g_assert_nonnull(block);
  g_assert_cmpuint(pwg_audio_block_get_sequence(block), ==, 0);
  g_assert_cmpuint(pwg_audio_block_get_n_frames(block), ==, 3);
  g_assert_cmpfloat(fabs(pwg_audio_block_get_peak(block) - 0.75), <, 0.000001);

  format = pwg_audio_block_get_format(block);
  g_assert_nonnull(format);
  g_assert_cmpstr(pwg_audio_format_get_sample_format(format), ==, "F32");
  g_assert_cmpuint(pwg_audio_format_get_rate(format), ==, 48000);
  g_assert_cmpuint(pwg_audio_format_get_channels(format), ==, 2);
  g_assert_cmpuint(pwg_audio_format_get_bytes_per_sample(format), ==, sizeof(float));
  g_assert_cmpuint(pwg_audio_format_get_bytes_per_frame(format), ==, 2 * sizeof(float));

  bytes = pwg_audio_block_get_data(block);
  g_assert_nonnull(bytes);
  samples = g_bytes_get_data(bytes, &size);
  g_assert_cmpuint(size, ==, 6 * sizeof(float));
  g_assert_nonnull(samples);
  g_assert_cmpfloat(fabs(samples[0] - 0.25), <, 0.000001);
  g_assert_cmpfloat(fabs(samples[1] + 0.50), <, 0.000001);
  g_assert_cmpfloat(fabs(samples[2] - 0.75), <, 0.000001);
  g_bytes_unref(bytes);

  fixture->got_block = TRUE;
  maybe_done(fixture);
}

static void
test_stream_dispatch_f32_audio(void)
{
  const float samples[] = {
    0.25f, -0.50f,
    0.75f, -0.10f,
    0.00f, 0.50f,
  };
  StreamDispatchFixture fixture = {0};
  guint timeout_id;

  fixture.loop = g_main_loop_new(NULL, FALSE);
  fixture.stream = pwg_stream_new_audio_capture(NULL, FALSE);
  g_assert_nonnull(fixture.stream);
  pwg_stream_set_deliver_audio_blocks(fixture.stream, TRUE);

  g_signal_connect(
    fixture.stream,
    "notify::audio-format",
    G_CALLBACK(on_audio_format_notify),
    &fixture);
  g_signal_connect(fixture.stream, "level", G_CALLBACK(on_level), &fixture);
  g_signal_connect(fixture.stream, "audio-block", G_CALLBACK(on_audio_block), &fixture);

  timeout_id = g_timeout_add(1000, timeout_cb, &fixture);
  _pwg_stream_test_push_f32_audio(
    fixture.stream,
    samples,
    G_N_ELEMENTS(samples),
    48000,
    2);
  g_main_loop_run(fixture.loop);
  if (!fixture.timed_out)
    g_source_remove(timeout_id);

  g_assert_false(fixture.timed_out);
  g_assert_true(fixture.got_format);
  g_assert_true(fixture.got_level);
  g_assert_true(fixture.got_block);
  g_assert_cmpfloat(fabs(fixture.peak - 0.75), <, 0.000001);

  g_clear_object(&fixture.stream);
  g_main_loop_unref(fixture.loop);
}

static void
test_stream_requested_format(void)
{
  g_autoptr(PwgStream) stream = NULL;
  g_autoptr(GError) error = NULL;

  stream = pwg_stream_new_audio_capture(NULL, FALSE);
  g_assert_nonnull(stream);
  g_assert_cmpstr(pwg_stream_get_requested_sample_format(stream), ==, "F32");
  g_assert_cmpuint(pwg_stream_get_requested_rate(stream), ==, 48000);
  g_assert_cmpuint(pwg_stream_get_requested_channels(stream), ==, 2);

  g_assert_true(pwg_stream_set_requested_format(stream, "F32", 44100, 1, &error));
  g_assert_no_error(error);
  g_assert_cmpstr(pwg_stream_get_requested_sample_format(stream), ==, "F32");
  g_assert_cmpuint(pwg_stream_get_requested_rate(stream), ==, 44100);
  g_assert_cmpuint(pwg_stream_get_requested_channels(stream), ==, 1);

  g_assert_false(pwg_stream_set_requested_format(stream, "S16", 44100, 1, &error));
  g_assert_error(error, PWG_ERROR, PWG_ERROR_FAILED);
  g_clear_error(&error);
  g_assert_cmpstr(pwg_stream_get_requested_sample_format(stream), ==, "F32");
  g_assert_cmpuint(pwg_stream_get_requested_rate(stream), ==, 44100);
  g_assert_cmpuint(pwg_stream_get_requested_channels(stream), ==, 1);

  g_assert_false(pwg_stream_set_requested_format(stream, "F32", 44100, 6, &error));
  g_assert_error(error, PWG_ERROR, PWG_ERROR_FAILED);
}

int
main(int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/pwg/stream/dispatch-f32-audio", test_stream_dispatch_f32_audio);
  g_test_add_func("/pwg/stream/requested-format", test_stream_requested_format);

  return g_test_run();
}
