#pragma once

#include <glib.h>

#include "pwg-defs.h"

G_BEGIN_DECLS

#define PWG_ERROR (pwg_error_quark())

/**
 * PwgError:
 * @PWG_ERROR_FAILED: generic failure.
 * @PWG_ERROR_NO_MEMORY: allocation failed.
 * @PWG_ERROR_PIPEWIRE: PipeWire reported or caused the failure.
 *
 * Error codes for the %PWG_ERROR domain.
 *
 * Since: 0.1
 * Stability: Unstable
 */
typedef enum {
  PWG_ERROR_FAILED,
  PWG_ERROR_NO_MEMORY,
  PWG_ERROR_PIPEWIRE,
} PwgError;

/**
 * pwg_error_quark:
 *
 * Gets the #GQuark used for Pwg errors.
 *
 * Returns: the #GQuark for the %PWG_ERROR domain.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
GQuark pwg_error_quark(void);

G_END_DECLS
