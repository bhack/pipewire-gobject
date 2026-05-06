#pragma once

#include "pwg-param.h"

G_BEGIN_DECLS

PwgParam *_pwg_param_new(gint seq,
                         guint id,
                         guint index,
                         guint next,
                         GBytes *bytes);

G_END_DECLS
