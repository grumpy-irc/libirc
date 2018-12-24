//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "network.h"

using namespace libirc;

Network::Network(const QString &Name)
{
    this->networkName = Name;
}

void Network::LoadHash(const QHash<QString, QVariant> &hash)
{
    UNSERIALIZE_STRING(networkName);
}

QHash<QString, QVariant> Network::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(networkName);
    return hash;
}

QString Network::GetNetworkName()
{
    return this->networkName;
}
