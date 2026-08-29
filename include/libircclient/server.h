//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SERVER_H
#define SERVER_H

#include "libircclient_global.h"
#include <QString>
#include "../libirc/server.h"

namespace libircclient
{
	class LIBIRCCLIENTSHARED_EXPORT Server : public libirc::Server
    {
        public:
            Server()=default;
            Server(const QHash<QString, QVariant> &hash);
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;
    };
}

#endif // SERVER_H
