#pragma once

#include <spa/pod/pod.h>

#include "pwg-param.h"

G_BEGIN_DECLS

PwgParam *_pwg_param_new(gint seq,
                         guint id,
                         guint index,
                         guint next,
                         GBytes *bytes);

const struct spa_pod *_pwg_param_get_pod(PwgParam *self);

G_END_DECLS
