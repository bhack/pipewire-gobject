#pragma once

#include <glib-object.h>

#include "pwg-audio-format.h"
#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_AUDIO_BLOCK (pwg_audio_block_get_type())

/**
 * PwgAudioBlock:
 *
 * Immutable copied interleaved audio sample block.
 *
 * Audio blocks own their sample bytes and never expose realtime PipeWire buffer
 * ownership to language bindings.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgAudioBlock, pwg_audio_block, PWG, AUDIO_BLOCK, GObject)

/**
 * pwg_audio_block_new:
 * @format: audio format for this block.
 * @data: copied interleaved audio sample bytes.
 * @n_frames: number of interleaved frames in @data.
 * @sequence: monotonic stream-local block sequence number.
 * @peak: absolute sample peak in this block.
 *
 * Creates an immutable audio block with owned sample data. The block never
 * exposes PipeWire realtime buffer ownership to language bindings.
 *
 * Returns: (transfer full): a new audio block object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgAudioBlock *pwg_audio_block_new(PwgAudioFormat *format,
                                   GBytes *data,
                                   guint n_frames,
                                   guint64 sequence,
                                   gdouble peak);

/**
 * pwg_audio_block_get_format:
 * @self: an audio block.
 *
 * Gets the audio format descriptor for this block.
 *
 * Returns: (transfer none): the block audio format.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgAudioFormat *pwg_audio_block_get_format(PwgAudioBlock *self);

/**
 * pwg_audio_block_get_data:
 * @self: an audio block.
 *
 * Gets a new reference to the copied interleaved sample bytes.
 *
 * Returns: (transfer full): copied interleaved audio sample bytes.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GBytes *pwg_audio_block_get_data(PwgAudioBlock *self);

/**
 * pwg_audio_block_get_n_frames:
 * @self: an audio block.
 *
 * Gets the number of interleaved frames in this block.
 *
 * Returns: the number of interleaved frames in the block.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint pwg_audio_block_get_n_frames(PwgAudioBlock *self);

/**
 * pwg_audio_block_get_sequence:
 * @self: an audio block.
 *
 * Gets the monotonic stream-local sequence number for this block.
 *
 * Returns: the monotonic stream-local block sequence number.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
guint64 pwg_audio_block_get_sequence(PwgAudioBlock *self);

/**
 * pwg_audio_block_get_peak:
 * @self: an audio block.
 *
 * Gets the absolute sample peak measured while copying this block.
 *
 * Returns: the absolute sample peak in this block.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
gdouble pwg_audio_block_get_peak(PwgAudioBlock *self);

G_END_DECLS
