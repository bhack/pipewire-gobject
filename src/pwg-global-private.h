#pragma once

#include <spa/utils/dict.h>

#include "pwg-global.h"

G_BEGIN_DECLS

PwgGlobal *_pwg_global_new_from_spa_dict(guint id,
                                         guint permissions,
                                         const char *interface_type,
                                         guint version,
                                         const struct spa_dict *properties);

G_END_DECLS
