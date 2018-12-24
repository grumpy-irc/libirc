//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SERVERADDRESS_H
#define SERVERADDRESS_H

#include "serializableitem.h"
#include "libirc_global.h"
#include <QString>

namespace libirc
{
    class LIBIRCSHARED_EXPORT ServerAddress : public SerializableItem
    {
        public:
            ServerAddress(const QString &url);
            ServerAddress(const QString &Host, bool SSL, unsigned int Port, const QString &Nick = "", const QString &Password = "");
            ServerAddress(const QHash<QString, QVariant> &hash);
            ~ServerAddress() override=default;
            bool IsValid();
            bool UsingSSL();
            QString GetHost();
            QString GetOriginal();
            void SetNick(const QString &nick);
            QString GetNick();
            QString GetPassword();
            //! Usually a list of channels to join after connect
            QString GetSuffix();
            void SetSuffix(const QString &suffix);
            unsigned int GetPort();
            void SetHost(const QString &host);
            void SetPassword(const QString &pw);
            void SetPort(unsigned int port);
            void SetSSL(bool ssl);
            void SetRealname(const QString &name);
            QString GetRealname();
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;
        private:
            bool IsIPv6();
            unsigned int _port;
            QString _password;
            QString _nick;
            QString _host;
            QString _realname;
            QString _suffix;
            bool _ssl;
            bool _valid;
            bool _ipv6;
            QString _original;
    };
}

#endif // SERVERADDRESS_H
