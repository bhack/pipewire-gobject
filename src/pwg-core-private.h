#pragma once

#include <pipewire/pipewire.h>

#include "pwg-core.h"

G_BEGIN_DECLS

struct pw_core *pwg_core_get_pw_core_internal(PwgCore *self);
struct pw_thread_loop *pwg_core_get_thread_loop_internal(PwgCore *self);

G_END_DECLS
