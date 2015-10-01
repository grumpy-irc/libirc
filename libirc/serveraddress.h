//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SERVERADDRESS_H
#define SERVERADDRESS_H

#include "libirc_global.h"
#include <QString>

namespace libirc
{
    class LIBIRCSHARED_EXPORT ServerAddress
    {
        public:
            ServerAddress(QString url);
            ServerAddress(QString Host, bool SSL, unsigned int Port, QString Nick = "", QString Password = "");
            virtual ~ServerAddress();
            bool IsValid();
            bool UsingSSL();
            QString GetHost();
            QString GetOriginal();
            void SetNick(QString Nick);
            QString GetNick();
            QString GetPassword();
            //! Usually a list of channels to join after connect
            QString GetSuffix();
            bool IsIPv6();
            unsigned int GetPort();
        private:
            unsigned int port;
            QString password;
            QString nick;
            QString host;
            QString suffix;
            bool ssl;
            bool valid;
            bool ipv6;
            QString original;
    };
}

#endif // SERVERADDRESS_H
