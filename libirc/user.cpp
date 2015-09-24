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

