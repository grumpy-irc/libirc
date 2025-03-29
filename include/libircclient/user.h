//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef USER_H
#define USER_H

#include "../libirc/user.h"
#include "libircclient_global.h"

namespace libircclient
{
    class LIBIRCCLIENTSHARED_EXPORT User : public libirc::User
    {
        public:
            User();
            User(const QHash<QString, QVariant> &hash);
            User(const QString &user);
            User(User *user);
            QString GetPrefixedNick();
            char GetHighestCUMode();
            QList<char> ChannelPrefixes;
            QString ServerName;
            QList<char> CUModes;
            QString AwayMs;
            bool IsAway;
            int Hops;
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;


        private:

    };
}

#endif // USER_H
