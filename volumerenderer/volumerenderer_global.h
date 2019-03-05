#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(VOLUMERENDERER_LIB)
#  define VOLUMERENDERER_EXPORT Q_DECL_EXPORT
# else
#  define VOLUMERENDERER_EXPORT Q_DECL_IMPORT
# endif
#else
# define VOLUMERENDERER_EXPORT
#endif
