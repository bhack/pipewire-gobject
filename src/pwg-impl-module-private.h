#pragma once

#include <pipewire/impl-module.h>

#include "pwg-core.h"
#include "pwg-impl-module.h"

G_BEGIN_DECLS

PwgImplModule *_pwg_impl_module_new(PwgCore *core,
                                    const char *name,
                                    const char *arguments,
                                    struct pw_impl_module *module);

G_END_DECLS
