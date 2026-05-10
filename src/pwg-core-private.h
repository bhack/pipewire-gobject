#pragma once

#include <pipewire/pipewire.h>

#include "pwg-core.h"

G_BEGIN_DECLS

struct pw_core *pwg_core_get_pw_core_internal(PwgCore *self);
struct pw_thread_loop *pwg_core_get_thread_loop_internal(PwgCore *self);
bool pwg_core_sync_main_context_internal(PwgCore *self,
                                         GMainContext *main_context,
                                         unsigned int timeout_msec,
                                         GError **error);

G_END_DECLS
