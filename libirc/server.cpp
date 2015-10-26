//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "server.h"

using namespace libirc;

Server::Server()
{
    this->_ssl = false;
    this->_version = "unknown version";
}

QString Server::GetVersion() const
{
    return this->_version;
}

void Server::LoadHash(QHash<QString, QVariant> hash)
{
    UNSERIALIZE_BOOL(_ssl);
    UNSERIALIZE_STRING(_version);
    UNSERIALIZE_STRING(_host);
    UNSERIALIZE_STRING(_name);
}

QHash<QString, QVariant> Server::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(_ssl);
    SERIALIZE(_version);
    SERIALIZE(_host);
    SERIALIZE(_name);
    return hash;
}

QString Server::GetName() const
{
    return this->_name;
}

void Server::SetName(QString name)
{
    this->_name = name;
}

void Server::SetVersion(QString version)
{
    this->_version = version;
}

libirc::Server::Server(QHash<QString, QVariant> hash)
{
    this->_ssl = false;
    this->_version = "unknown version";
    this->LoadHash(hash);
}
