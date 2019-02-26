#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(ISOSURFACERENDER_LIB)
#  define ISOSURFACERENDER_EXPORT Q_DECL_EXPORT
# else
#  define ISOSURFACERENDER_EXPORT Q_DECL_IMPORT
# endif
#else
# define ISOSURFACERENDER_EXPORT
#endif
