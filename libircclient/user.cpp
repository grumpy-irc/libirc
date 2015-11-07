//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "user.h"

using namespace libircclient;

User::User()
{
    this->ChannelPrefix = 0;
    this->CUMode = 0;
}

User::User(QHash<QString, QVariant> hash)
{
    this->ChannelPrefix = 0;
    this->CUMode = 0;
    this->LoadHash(hash);
}

User::User(QString user) : libirc::User(user)
{
    this->ChannelPrefix = 0;
    this->CUMode = 0;
}

User::User(User *user) : libirc::User(user)
{
    this->ChannelPrefix = user->ChannelPrefix;
    this->CUMode = user->CUMode;
}

QString User::GetPrefixedNick()
{
    QString nick = this->GetNick();
    if (this->ChannelPrefix != 0)
        nick = QChar(this->ChannelPrefix) + nick;

    return nick;
}

void User::LoadHash(QHash<QString, QVariant> hash)
{
    libirc::User::LoadHash(hash);
    UNSERIALIZE_CCHAR(ChannelPrefix);
    UNSERIALIZE_STRING(ServerName);
    UNSERIALIZE_CCHAR(CUMode);
}

QHash<QString, QVariant> User::ToHash()
{
    QHash<QString, QVariant> hash = libirc::User::ToHash();
    SERIALIZE_CCHAR(ChannelPrefix);
    SERIALIZE(ServerName);
    SERIALIZE_CCHAR(CUMode);
    return hash;
}


