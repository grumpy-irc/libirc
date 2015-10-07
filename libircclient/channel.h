//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef CHANNEL_H
#define CHANNEL_H

#include "libircclient_global.h"
#include <QString>
#include <QList>
#include "mode.h"
#include "../libirc/channel.h"

namespace libircclient
{
    class User;
    class Network;
    class LIBIRCCLIENTSHARED_EXPORT Channel : public libirc::Channel
    {
        public:
            Channel(QString name, Network *network = NULL);
            ~Channel();
            User *InsertUser(User *user);
            void RemoveUser(QString user);
            void ChangeNick(QString old_nick, QString new_nick);
            bool ContainsUser(QString user);
            void SendMessage(QString text);
            User *GetUser(QString user);
            CMode GetMode();
            void SetMode(QString mode);
            void Part();
        protected:
            CMode localMode;
            QList<User*> users;
            Network *_net;
    };
}

#endif // CHANNEL_H
