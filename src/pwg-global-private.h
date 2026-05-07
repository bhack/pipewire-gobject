#pragma once

#include <spa/utils/dict.h>

#include "pwg-global.h"

G_BEGIN_DECLS

PwgGlobal *_pwg_global_new_from_spa_dict(unsigned int id,
                                         unsigned int permissions,
                                         const char *interface_type,
                                         unsigned int version,
                                         const struct spa_dict *properties);

G_END_DECLS
