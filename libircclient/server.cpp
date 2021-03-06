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

using namespace libircclient;

Server::Server(const QHash<QString, QVariant> &hash) : libirc::Server(hash)
{

}

void Server::LoadHash(const QHash<QString, QVariant> &hash)
{
    libirc::Server::LoadHash(hash);
}

QHash<QString, QVariant> Server::ToHash()
{
    QHash<QString, QVariant> hash = libirc::Server::ToHash();
    return hash;
}

