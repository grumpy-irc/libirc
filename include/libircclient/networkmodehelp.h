//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2017

#ifndef NETWORKMODEHELP_H
#define NETWORKMODEHELP_H

#include <QString>
#include <QHash>
#include "libircclient_global.h"
#include "../libirc/irc_standards.h"

namespace libircclient
{
    /*!
     * \brief The NetworkModeHelp class contains help for various ircd servers
     */
    class LIBIRCCLIENTSHARED_EXPORT NetworkModeHelp
    {
        public:
            static QHash<char, QString> GetUserModeHelp(QString ircd);
            static QHash<char, QString> GetChannelModeHelp(QString ircd);
    };
}

#endif // NETWORK_H
