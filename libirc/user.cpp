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

using namespace libirc;

User::User()
{

}

User::User(User *user)
{
    this->_host = user->_host;
    this->_ident = user->_ident;
    this->_nick = user->_nick;
    this->_username = user->_username;
}

User::User(QString source)
{
    QString temp = source;
    if (source.contains("!"))
    {
        this->_nick = temp.mid(0, temp.indexOf("!"));
        temp = temp.mid(temp.indexOf("!") + 1);
    }
    else if (source.contains("@"))
    {
        this->_nick = source.mid(0, source.indexOf("@"));
        this->_host = source.mid(source.indexOf("@") + 1);
    }
    else
    {
        this->_nick = source;
        return;
    }
    if (temp.contains("@"))
    {
        this->_ident = temp.mid(0, temp.indexOf("@"));
        this->_host = temp.mid(temp.indexOf("@") + 1);
    }
}

QString User::GetHost() const
{
    return this->_host;
}

QString User::GetIdent() const
{
    return this->_ident;
}

void User::SetIdent(QString ident)
{
    this->_ident = ident;
}

void User::SetHost(const QString &host)
{
    this->_host = host;
}

QString User::GetNick() const
{
    return this->_nick;
}

void User::SetNick(const QString &nick)
{
    this->_nick = nick;
}

QString libirc::User::ToString() const
{
    return this->_nick + "!" + this->_ident + "@" + this->_host;
}

void User::SetRealname(QString user)
{
    this->_username = user;
}

QString User::GetRealname() const
{
    return this->_username;
}

bool User::IsNull()
{
    return this->_nick.isNull();
}

void User::LoadHash(QHash<QString, QVariant> hash)
{
    UNSERIALIZE_STRING(_username);
    UNSERIALIZE_STRING(_host);
    UNSERIALIZE_STRING(_nick);
    UNSERIALIZE_STRING(_ident);
}

QHash<QString, QVariant> User::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(_username);
    SERIALIZE(_nick);
    SERIALIZE(_ident);
    SERIALIZE(_host);
    return hash;
}
