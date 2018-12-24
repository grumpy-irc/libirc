//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "user.h"

using namespace libircclient;

User::User()
{
    this->Hops = 0;
    this->IsAway = false;
}

User::User(const QHash<QString, QVariant> &hash)
{
    this->Hops = 0;
    this->IsAway = false;
    this->LoadHash(hash);
}

User::User(const QString &user) : libirc::User(user)
{
    this->Hops = 0;
    this->IsAway = false;
}

User::User(User *user) : libirc::User(user)
{
    this->IsAway = user->IsAway;
    this->Hops = user->Hops;
    this->ChannelPrefixes = user->ChannelPrefixes;
    this->CUModes = user->CUModes;
}

QString User::GetPrefixedNick()
{
    QString nick = this->GetNick();
    if (!this->ChannelPrefixes.isEmpty())
        nick = QChar(this->ChannelPrefixes[0]) + nick;

    return nick;
}

char User::GetHighestCUMode()
{
    if (this->CUModes.isEmpty())
        return 0;

    // Get a mode with index 0 as that is meant to be highest
    return this->CUModes[0];
}

void User::LoadHash(const QHash<QString, QVariant> &hash)
{
    libirc::User::LoadHash(hash);
    UNSERIALIZE_CHARLIST(ChannelPrefixes);
    UNSERIALIZE_STRING(AwayMs);
    //UNSERIALIZE_INT(Hops);
    UNSERIALIZE_BOOL(IsAway);
    UNSERIALIZE_STRING(ServerName);
    UNSERIALIZE_CHARLIST(CUModes);
}

QHash<QString, QVariant> User::ToHash()
{
    QHash<QString, QVariant> hash = libirc::User::ToHash();
    SERIALIZE_CHARLIST(ChannelPrefixes);
    SERIALIZE_CHARLIST(CUModes);
    SERIALIZE(ServerName);
    SERIALIZE(IsAway);
    SERIALIZE(AwayMs);
    return hash;
}


