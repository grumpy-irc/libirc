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

Channel::Channel(QHash<QString, QVariant> hash) : libirc::Channel("")
{
    this->_net = NULL;
    this->LoadHash(hash);
}

Channel::Channel(QString name, Network *network) : libirc::Channel(name)
{
    this->_net = network;
    this->_localModeDateTime = QDateTime::currentDateTime();
}

Channel::Channel(Channel *channel) : libirc::Channel(channel->_name)
{
    this->deepCopy(channel);
}

Channel::Channel(const Channel &channel) : libirc::Channel(channel._name)
{
    this->deepCopy(&channel);
}

Channel::~Channel()
{
    qDeleteAll(this->_users.values());
    this->_users.clear();
}

User *Channel::InsertUser(User *user)
{
    User *ux;

    QString luser = user->GetNick().toLower();

    if (this->_users.contains(luser))
    {
        ux = this->_users[luser];
        ux->ChannelPrefix = user->ChannelPrefix;
        ux->CUMode = user->CUMode;
        ux->SetHost(user->GetHost());
        ux->SetIdent(user->GetIdent());
        ux->ServerName = user->ServerName;
        ux->SetRealname(user->GetRealname());
    }
    else
    {
        ux = new User(user);
        this->_users.insert(luser, ux);
    }

    //emit this->Event_UserInserted(ux);
    return ux;
}

void Channel::RemoveUser(QString user)
{
    user = user.toLower();
    if (!this->_users.contains(user))
        return;
    delete this->_users[user];
    this->_users.remove(user);
}

void Channel::ChangeNick(QString old_nick, QString new_nick)
{
    User *user = this->GetUser(old_nick);
    if (!user)
        return;

    //emit this->Event_NickChanged(old_nick, new_nick);
    user->SetNick(new_nick);
    this->_users.remove(old_nick.toLower());
    this->_users.insert(new_nick.toLower(), user);
}

bool Channel::ContainsUser(QString user)
{
    return this->_users.contains(user.toLower());
}

void Channel::LoadHash(QHash<QString, QVariant> hash)
{
    libirc::Channel::LoadHash(hash);
    UNSERIALIZE_DATETIME(_localModeDateTime);
    if (hash.contains("localMode"))
        this->_localMode = CMode(hash["localMode"].toHash());
    if (hash.contains("users"))
    {
        QHash<QString, QVariant> users_x = hash["users"].toHash();
        foreach (QString user, users_x.keys())
            this->_users.insert(user, new User(users_x[user].toHash()));
    }
}

QHash<QString, QVariant> Channel::ToHash()
{
    QHash<QString, QVariant> hash = libirc::Channel::ToHash();
    SERIALIZE(_localModeDateTime);
    hash.insert("localMode", QVariant(this->_localMode.ToHash()));
    QHash<QString, QVariant> users_l;
    foreach (QString user, this->_users.keys())
        users_l.insert(user, this->_users[user]->ToHash());
    hash.insert("users", QVariant(users_l));
    return hash;
}

void Channel::SendMessage(QString text)
{
    if (!this->_net)
        return;
    this->_net->SendMessage(text, this);
}

void Channel::SetNetwork(Network *network)
{
    this->_net = network;
}

void Channel::ClearUsers()
{
    qDeleteAll(this->_users);
    this->_users.clear();
}

QHash<QString, User *> Channel::GetUsers() const
{
    return this->_users;
}

User *Channel::GetUser(QString user)
{
    user = user.toLower();
    if (this->_users.contains(user))
        return this->_users[user];

    return NULL;
}

CMode Channel::GetMode()
{
    return this->_localMode;
}

void Channel::SetMode(QString mode)
{
    this->_localMode.SetMode(mode);
}

void Channel::Part()
{
    if (this->_net)
        this->_net->RequestPart(this);
}

void Channel::deepCopy(const Channel *source)
{
    this->_net = source->_net;
    this->_topicTime = source->_topicTime;
    this->_name = source->_name;
    this->_topic = source->_topic;
    this->_localModeDateTime = source->_localModeDateTime;
    this->_topicUser = source->_topicUser;
    this->_localMode = source->_localMode;
    foreach (QString user, source->_users.keys())
        this->_users.insert(user, new User(source->_users[user]));
}

