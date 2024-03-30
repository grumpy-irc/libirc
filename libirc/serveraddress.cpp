//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "serveraddress.h"
#include "irc_standards.h"

using namespace libirc;

ServerAddress::ServerAddress(const QString &url)
{
    this->_host = "";
    this->_original = url;
    this->_port = IRC_STANDARD_PORT;
    this->_ssl = false;
    this->_ipv6 = false;
    this->_valid = false;
    QString temp = url.toLower();
    temp = temp.trimmed();
    if (temp.contains(" "))
        return;
    if (temp.startsWith("irc://"))
    {
        temp = temp.mid(6);
    } else if (temp.startsWith("ircs://"))
    {
        this->_port = IRC_STANDARD_PORT_SSL;
        temp = temp.mid(7);
        this->_ssl = true;
    }
    // get the suffix
    if (temp.contains("/"))
    {
        this->_suffix = temp.mid(temp.indexOf("/") + 1);
        temp = temp.mid(0, temp.indexOf("/"));
    }
    // IPv6 support, must be processed before port because that one uses colons as well
    if (temp.startsWith("["))
    {
        this->_ipv6 = true;
        temp = temp.mid(1);
        if (!temp.contains("]"))
            return;
        this->_host = temp.mid(0, temp.indexOf("]"));
        temp = temp.mid(temp.indexOf("]") + 1);
        if (temp.contains(":"))
        {
            temp = temp.mid(temp.indexOf(":") + 1);
            this->_port = temp.toUInt();
        }
    } else
    {
        if (!temp.contains(":"))
        {
            // we are done here
            this->_host = temp;
        } else
        {
            this->_host = temp.mid(0, temp.indexOf(":"));
            this->_port = temp.mid(temp.indexOf(":") + 1).toUInt();
        }
    }
    this->_valid = true;
}

ServerAddress::ServerAddress(const QString &Host, bool SSL, unsigned int Port, const QString &Nick, const QString &Password)
{
    this->_host = Host;
    this->_ssl = SSL;
    this->_valid = true;
    this->_port = Port;
    this->_password = Password;
    this->_ipv6 = false;
    this->_nick = Nick;
}

ServerAddress::ServerAddress(const QHash<QString, QVariant> &hash)
{
    this->_valid = false;
    this->_port = IRC_STANDARD_PORT;
    this->_ssl = false;
    this->_ipv6 = false;
    this->LoadHash(hash);
}

bool ServerAddress::IsValid()
{
    return this->_valid;
}

QString ServerAddress::GetHost()
{
    return this->_host;
}

bool ServerAddress::UsingSSL()
{
    return this->_ssl;
}

unsigned int ServerAddress::GetPort()
{
    return this->_port;
}

void ServerAddress::SetHost(const QString &host)
{
    this->_host = host;
    this->_valid = true;
}

void ServerAddress::SetPassword(const QString &pw)
{
    this->_password = pw;
}

void ServerAddress::SetPort(unsigned int port)
{
    this->_port = port;
}

void ServerAddress::SetSSL(bool ssl)
{
    this->_ssl = ssl;
}

void ServerAddress::SetRealname(const QString &name)
{
    this->_realname = name;
}

QString ServerAddress::GetRealname()
{
    return this->_realname;
}

void ServerAddress::LoadHash(const QHash<QString, QVariant> &hash)
{
    UNSERIALIZE_UINT(_port);
    UNSERIALIZE_STRING(_password);
    UNSERIALIZE_STRING(_nick);
    UNSERIALIZE_STRING(_host);
    UNSERIALIZE_STRING(_suffix);
    UNSERIALIZE_BOOL(_ssl);
    UNSERIALIZE_BOOL(_valid);
    UNSERIALIZE_BOOL(_ipv6);
    UNSERIALIZE_STRING(_original);
    UNSERIALIZE_STRING(_realname);
}

QHash<QString, QVariant> ServerAddress::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(_port);
    SERIALIZE(_password);
    SERIALIZE(_nick);
    SERIALIZE(_host);
    SERIALIZE(_suffix);
    SERIALIZE(_ssl);
    SERIALIZE(_valid);
    SERIALIZE(_ipv6);
    SERIALIZE(_original);
    SERIALIZE(_realname);
    return hash;
}

QString ServerAddress::GetOriginal()
{
    return this->_original;
}

void ServerAddress::SetNick(const QString &nick)
{
    this->_nick = nick;
}

QString ServerAddress::GetNick()
{
    return this->_nick;
}

QString ServerAddress::GetPassword()
{
    return this->_password;
}

QString ServerAddress::GetSuffix()
{
    return this->_suffix;
}

void ServerAddress::SetSuffix(const QString &suffix)
{
    this->_suffix = suffix;
}

bool ServerAddress::IsIPv6()
{
    return this->_ipv6;
}

