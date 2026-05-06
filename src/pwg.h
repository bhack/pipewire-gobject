#pragma once

#include <glib.h>

#include "pwg-defs.h"

G_BEGIN_DECLS

/**
 * pwg_init:
 *
 * Initializes PipeWire for this process. Calling this function more than once
 * is harmless.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_init(void);

/**
 * pwg_get_library_version:
 *
 * Gets the Pwg library version string.
 *
 * Returns: (transfer none): the Pwg library version.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_get_library_version(void);

/**
 * pwg_get_pipewire_library_version:
 *
 * Gets the linked PipeWire library version string.
 *
 * Returns: (transfer none): the linked PipeWire library version.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_get_pipewire_library_version(void);

G_END_DECLS

#include "pwg-audio-block.h"
#include "pwg-audio-capture.h"
#include "pwg-audio-format.h"
#include "pwg-client-info.h"
#include "pwg-core.h"
#include "pwg-device-info.h"
#include "pwg-error.h"
#include "pwg-global.h"
#include "pwg-link-info.h"
#include "pwg-metadata.h"
#include "pwg-node.h"
#include "pwg-node-info.h"
#include "pwg-param.h"
#include "pwg-param-info.h"
#include "pwg-port-info.h"
#include "pwg-registry.h"
#include "pwg-stream.h"
