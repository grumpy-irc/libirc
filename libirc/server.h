//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef LIBIRCSERVER_H
#define LIBIRCSERVER_H

#include "serializableitem.h"

namespace libirc
{
    class LIBIRCSHARED_EXPORT Server : public SerializableItem
    {
        public:
            Server(QHash<QString, QVariant> hash);
            // Default ct
            Server();
            virtual ~Server() {}
            QString GetName() const;
            void SetVersion(QString version);
            void SetName(QString name);
            QString GetVersion() const;
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();

        protected:
            QString _name;
            QString _host;
            QString _version;
            unsigned int _port;
            bool _ssl;
    };
}

#endif // SERVER_H
