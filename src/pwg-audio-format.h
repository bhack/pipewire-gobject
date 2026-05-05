#pragma once

#include <glib-object.h>

#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_AUDIO_FORMAT (pwg_audio_format_get_type())

PWG_API
G_DECLARE_FINAL_TYPE(PwgAudioFormat, pwg_audio_format, PWG, AUDIO_FORMAT, GObject)

/**
 * pwg_audio_format_new:
 * @sample_format: PipeWire/SPA sample format name.
 * @rate: sample rate in Hz.
 * @channels: channel count.
 * @bytes_per_sample: bytes per single-channel sample.
 *
 * Creates an immutable audio format descriptor.
 *
 * Returns: (transfer full): a new audio format object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgAudioFormat *pwg_audio_format_new(const char *sample_format,
                                     guint rate,
                                     guint channels,
                                     guint bytes_per_sample);

/**
 * pwg_audio_format_get_sample_format:
 * @self: an audio format.
 *
 * Returns: (transfer none): the PipeWire/SPA sample format name.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_audio_format_get_sample_format(PwgAudioFormat *self);

/**
 * pwg_audio_format_get_rate:
 * @self: an audio format.
 *
 * Returns: the sample rate in Hz.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_audio_format_get_rate(PwgAudioFormat *self);

/**
 * pwg_audio_format_get_channels:
 * @self: an audio format.
 *
 * Returns: the channel count.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_audio_format_get_channels(PwgAudioFormat *self);

/**
 * pwg_audio_format_get_bytes_per_sample:
 * @self: an audio format.
 *
 * Returns: bytes per single-channel sample.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_audio_format_get_bytes_per_sample(PwgAudioFormat *self);

/**
 * pwg_audio_format_get_bytes_per_frame:
 * @self: an audio format.
 *
 * Returns: bytes per interleaved frame, or 0 if the value overflows.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_audio_format_get_bytes_per_frame(PwgAudioFormat *self);

G_END_DECLS
