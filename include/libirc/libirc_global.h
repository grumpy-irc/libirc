#ifndef LIBIRC_GLOBAL_H
#define LIBIRC_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBIRC_LIBRARY)
#  define LIBIRCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBIRCSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBIRC_GLOBAL_H
