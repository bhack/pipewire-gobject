#pragma once

#include <glib-object.h>

#include "pwg-audio-block.h"
#include "pwg-audio-format.h"
#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_STREAM (pwg_stream_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgStream, pwg_stream, PWG, STREAM, GObject)

/**
 * pwg_stream_new_audio_capture:
 * @target_object: (nullable): PipeWire node name or object serial to capture from.
 * @monitor: whether to capture sink monitor audio.
 *
 * Creates an audio capture stream that reports copied/reduced level data to
 * high-level language bindings.
 *
 * Returns: (transfer full): a new stream object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgStream *pwg_stream_new_audio_capture(const char *target_object, gboolean monitor);

/**
 * pwg_stream_start:
 * @self: a stream object.
 * @error: return location for a #GError.
 *
 * Returns: %TRUE when the stream started or was already running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_stream_start(PwgStream *self, GError **error);

/**
 * pwg_stream_stop:
 * @self: a stream object.
 *
 * Stops the stream if it is running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_stream_stop(PwgStream *self);

/**
 * pwg_stream_get_running:
 * @self: a stream object.
 *
 * Returns: whether the stream is running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_stream_get_running(PwgStream *self);

/**
 * pwg_stream_get_target_object:
 * @self: a stream object.
 *
 * Returns: (nullable) (transfer none): the requested PipeWire target.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_stream_get_target_object(PwgStream *self);

/**
 * pwg_stream_get_monitor:
 * @self: a stream object.
 *
 * Returns: whether sink monitor capture is requested.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_stream_get_monitor(PwgStream *self);

/**
 * pwg_stream_get_rate:
 * @self: a stream object.
 *
 * Returns: the negotiated sample rate, or 0 before format negotiation.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_stream_get_rate(PwgStream *self);

/**
 * pwg_stream_get_channels:
 * @self: a stream object.
 *
 * Returns: the negotiated channel count, or 0 before format negotiation.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_stream_get_channels(PwgStream *self);

/**
 * pwg_stream_get_peak:
 * @self: a stream object.
 *
 * Returns: the latest absolute sample peak.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gdouble pwg_stream_get_peak(PwgStream *self);

/**
 * pwg_stream_get_audio_format:
 * @self: a stream object.
 *
 * Returns: (nullable) (transfer none): the current negotiated audio format, or
 *   %NULL before format negotiation.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgAudioFormat *pwg_stream_get_audio_format(PwgStream *self);

/**
 * pwg_stream_get_deliver_audio_blocks:
 * @self: a stream object.
 *
 * Returns: whether copied audio block delivery is enabled.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gboolean pwg_stream_get_deliver_audio_blocks(PwgStream *self);

/**
 * pwg_stream_set_deliver_audio_blocks:
 * @self: a stream object.
 * @deliver_audio_blocks: whether to emit copied audio blocks.
 *
 * Enables or disables the `PwgStream::audio-block` signal. Audio block delivery
 * copies realtime PipeWire buffers and should be enabled only by consumers that
 * need sample data.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_stream_set_deliver_audio_blocks(PwgStream *self, gboolean deliver_audio_blocks);

G_END_DECLS
