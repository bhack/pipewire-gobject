#include "pwg.h"

#include <pipewire/pipewire.h>

static gsize pipewire_initialized = 0;

void
pwg_init(void)
{
  if (g_once_init_enter(&pipewire_initialized)) {
    pw_init(NULL, NULL);
    g_once_init_leave(&pipewire_initialized, 1);
  }
}

const char *
pwg_get_library_version(void)
{
  return PWG_VERSION;
}

const char *
pwg_get_pipewire_library_version(void)
{
  pwg_init();
  return pw_get_library_version();
}
