//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef LIBIRCCLIENTGENERIC_H
#define LIBIRCCLIENTGENERIC_H

#include <QObject>
#include <QAbstractSocket>
#include "libircclient_global.h"
#include <QString>

namespace libircclient
{
    namespace Generic
    {
        LIBIRCCLIENTSHARED_EXPORT QString ErrorCode2String(QAbstractSocket::SocketError type);
        //! Merge unique items in 2 lists
        LIBIRCCLIENTSHARED_EXPORT QList<QString> UniqueMerge(QList<QString> a, QList<QString> b);
    }
}

#endif // LIBIRCCLIENTGENERIC_H
