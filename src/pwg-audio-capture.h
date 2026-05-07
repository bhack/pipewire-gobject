#pragma once

#include <glib-object.h>

#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_TYPE_AUDIO_CAPTURE (pwg_audio_capture_get_type())

/**
 * PwgAudioCapture:
 *
 * PipeWire audio capture wrapper.
 *
 * This class is kept for compatibility with the initial stream API. New code
 * should use [class@Pwg.Stream] for app-owned audio streams.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgAudioCapture, pwg_audio_capture, PWG, AUDIO_CAPTURE, GObject)

/**
 * pwg_audio_capture_new:
 * @target_object: (nullable): PipeWire node name or object serial to capture from.
 * @monitor: whether to capture sink monitor audio.
 *
 * Creates a PipeWire audio capture stream wrapper. Passing %NULL for
 * @target_object lets PipeWire choose the target.
 *
 * Returns: (transfer full): a new audio capture object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgAudioCapture *pwg_audio_capture_new(const char *target_object, bool monitor);

/**
 * pwg_audio_capture_start:
 * @self: an audio capture object.
 * @error: return location for a #GError.
 *
 * Starts the PipeWire capture stream.
 *
 * Returns: %TRUE when capture started or was already running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_audio_capture_start(PwgAudioCapture *self, GError **error);

/**
 * pwg_audio_capture_stop:
 * @self: an audio capture object.
 *
 * Stops the capture stream if it is running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_audio_capture_stop(PwgAudioCapture *self);

/**
 * pwg_audio_capture_get_running:
 * @self: an audio capture object.
 *
 * Gets whether the capture stream is running.
 *
 * Returns: whether the capture stream is running.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_audio_capture_get_running(PwgAudioCapture *self);

/**
 * pwg_audio_capture_get_target_object:
 * @self: an audio capture object.
 *
 * Gets the requested PipeWire target object.
 *
 * Returns: (nullable) (transfer none): the requested PipeWire target.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_audio_capture_get_target_object(PwgAudioCapture *self);

/**
 * pwg_audio_capture_get_monitor:
 * @self: an audio capture object.
 *
 * Gets whether sink monitor capture was requested.
 *
 * Returns: whether sink monitor capture is requested.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_audio_capture_get_monitor(PwgAudioCapture *self);

/**
 * pwg_audio_capture_get_rate:
 * @self: an audio capture object.
 *
 * Gets the negotiated sample rate.
 *
 * Returns: the negotiated sample rate, or 0 before format negotiation.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
unsigned int pwg_audio_capture_get_rate(PwgAudioCapture *self);

/**
 * pwg_audio_capture_get_channels:
 * @self: an audio capture object.
 *
 * Gets the negotiated channel count.
 *
 * Returns: the negotiated channel count, or 0 before format negotiation.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
unsigned int pwg_audio_capture_get_channels(PwgAudioCapture *self);

/**
 * pwg_audio_capture_get_peak:
 * @self: an audio capture object.
 *
 * Gets the latest absolute sample peak.
 *
 * Returns: the latest absolute sample peak.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
double pwg_audio_capture_get_peak(PwgAudioCapture *self);

G_END_DECLS
