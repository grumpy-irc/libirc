//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "serveraddress.h"
#include "irc_standards.h"

using namespace libirc;

ServerAddress::ServerAddress(QString url)
{
    this->host = "";
    this->original = url;
    this->port = IRC_STANDARD_PORT;
    this->ssl = false;
    this->valid = false;
    QString temp = url.toLower();
    temp = temp.trimmed();
    if (temp.contains(" "))
        return;
    if (temp.startsWith("irc://"))
    {
        temp = temp.mid(6);
    } else if (temp.startsWith("ircs://"))
    {
        temp = temp.mid(7);
        this->ssl = true;
    }
    // get the suffix
    if (temp.contains("/"))
    {
        this->suffix = temp.mid(temp.indexOf("/") + 1);
        temp = temp.mid(0, temp.indexOf("/"));
    }
    // IPv6 support, must be processed before port because that one uses colons as well
    if (temp.startsWith("["))
    {
        this->ipv6 = true;
        temp = temp.mid(1);
        if (!temp.contains("]"))
            return;
        this->host = temp.mid(0, temp.indexOf("]"));
        temp = temp.mid(temp.indexOf("]") + 1);
        if (temp.contains(":"))
        {
            temp = temp.mid(temp.indexOf(":") + 1);
            this->port = temp.toUInt();
        }
    } else
    {
        if (!temp.contains(":"))
        {
            // we are done here
            this->host = temp;
        } else
        {
            this->host = temp.mid(0, temp.indexOf(":"));
            this->port = temp.mid(temp.indexOf(":") + 1).toUInt();
        }
    }
    this->valid = true;
}

ServerAddress::ServerAddress(QString Host, bool SSL, unsigned int Port, QString Nick, QString Password)
{
    this->host = Host;
    this->ssl = SSL;
    this->valid = true;
    this->port = Port;
    this->password = Password;
    this->nick = Nick;
}

ServerAddress::~ServerAddress()
{

}

bool ServerAddress::IsValid()
{
    return this->valid;
}

QString ServerAddress::GetHost()
{
    return this->host;
}

bool ServerAddress::UsingSSL()
{
    return this->ssl;
}

unsigned int ServerAddress::GetPort()
{
    return this->port;
}

QString ServerAddress::GetOriginal()
{
    return this->original;
}

void ServerAddress::SetNick(QString Nick)
{
    this->nick = Nick;
}

QString ServerAddress::GetNick()
{
    return this->nick;
}

QString ServerAddress::GetPassword()
{
    return this->password;
}

QString ServerAddress::GetSuffix()
{
    return this->suffix;
}

bool ServerAddress::IsIPv6()
{
    return this->ipv6;
}

