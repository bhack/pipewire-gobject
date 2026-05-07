#pragma once

#include <glib-object.h>

#include "pwg-defs.h"

G_BEGIN_DECLS

typedef struct _PwgCore PwgCore;

#define PWG_TYPE_IMPL_MODULE (pwg_impl_module_get_type())

/**
 * PwgImplModule:
 *
 * Handle for a PipeWire implementation module loaded into a [class@Pwg.Core]
 * context.
 *
 * The module is kept loaded while this object is loaded. Unload it explicitly
 * with [method@Pwg.ImplModule.unload] or by dropping the object.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
G_DECLARE_FINAL_TYPE(PwgImplModule, pwg_impl_module, PWG, IMPL_MODULE, GObject)

/**
 * pwg_impl_module_get_core:
 * @self: an implementation module handle.
 *
 * Gets the core that owns this loaded module.
 *
 * Returns: (transfer none): the owning core.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
PwgCore *pwg_impl_module_get_core(PwgImplModule *self);

/**
 * pwg_impl_module_get_name:
 * @self: an implementation module handle.
 *
 * Gets the PipeWire module name that was loaded.
 *
 * Returns: (transfer none): the module name.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_impl_module_get_name(PwgImplModule *self);

/**
 * pwg_impl_module_get_arguments:
 * @self: an implementation module handle.
 *
 * Gets the PipeWire module argument string.
 *
 * Returns: (nullable) (transfer none): the module arguments, or %NULL.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
const char *pwg_impl_module_get_arguments(PwgImplModule *self);

/**
 * pwg_impl_module_get_loaded:
 * @self: an implementation module handle.
 *
 * Gets whether this handle still has a loaded PipeWire implementation module.
 *
 * Returns: whether the module is still loaded.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
bool pwg_impl_module_get_loaded(PwgImplModule *self);

/**
 * pwg_impl_module_unload:
 * @self: an implementation module handle.
 *
 * Unloads the PipeWire implementation module if it is still loaded.
 *
 * Since: 0.1
 * Stability: Unstable
 */
PWG_API
void pwg_impl_module_unload(PwgImplModule *self);

G_END_DECLS
