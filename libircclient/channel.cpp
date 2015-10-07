//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "channel.h"
#include "user.h"
#include "network.h"

using namespace libircclient;

Channel::Channel(QString name, Network *network) : libirc::Channel(name)
{
    this->_net = network;
}

Channel::~Channel()
{
    qDeleteAll(this->users);
}

User *Channel::InsertUser(User *user)
{
    if (this->ContainsUser(user->GetNick()))
        return this->GetUser(user->GetNick());
    User *ux = new User(user);
    this->users.append(ux);
    return ux;
}

void Channel::RemoveUser(QString user)
{
    user = user.toLower();
    int i = 0;
    while (i < this->users.count())
    {
        if (this->users[i]->GetNick().toLower() == user)
        {
            delete this->users[i];
            this->users.removeAt(i);
            return;
        }
        i++;
    }
}

void Channel::ChangeNick(QString old_nick, QString new_nick)
{
    User *user = this->GetUser(old_nick);
    if (!user)
        return;

    user->SetNick(new_nick);
}

bool Channel::ContainsUser(QString user)
{
    user = user.toLower();
    foreach (User *ux, this->users)
    {
        if (ux->GetNick().toLower() == user)
            return true;
    }
    return false;
}

void Channel::SendMessage(QString text)
{
    if (!this->_net)
        return;
    this->_net->SendMessage(text, this);
}

User *Channel::GetUser(QString user)
{
    user = user.toLower();
    foreach (User *xx, this->users)
    {
        if (xx->GetNick().toLower() == user)
            return xx;
    }
    return NULL;
}

CMode Channel::GetMode()
{
    return this->localMode;
}

void Channel::SetMode(QString mode)
{
    this->localMode.SetMode(mode);
}

void Channel::Part()
{
    if (this->_net)
        this->_net->Part(this);
}

