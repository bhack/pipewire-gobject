#pragma once

#include <glib.h>

G_BEGIN_DECLS

#if defined(_WIN32) && !defined(__CYGWIN__)
# ifdef BUILDING_PWG
#  define PWG_API __declspec(dllexport)
# else
#  define PWG_API __declspec(dllimport)
# endif
#elif defined(__GNUC__)
# define PWG_API extern __attribute__((visibility("default")))
#else
# define PWG_API extern
#endif

G_END_DECLS
