//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "channel.h"

using namespace libirc;

Channel::Channel(const QString &name)
{
    this->_name = name;
}

QString Channel::GetName() const
{
    return this->_name;
}

QString Channel::GetTopic() const
{
    return this->_topic;
}

void Channel::SetTopicTime(const QDateTime &time)
{
    this->_topicTime = time;
}

QDateTime Channel::GetTopicTime() const
{
    return this->_topicTime;
}

QString Channel::GetTopicUser() const
{
    return this->_topicUser;
}

void Channel::SetTopicUser(const QString &user)
{
    this->_topicUser = user;
}

void Channel::LoadHash(const QHash<QString, QVariant> &hash)
{
    UNSERIALIZE_STRING(_topic);
    UNSERIALIZE_STRING(_name);
    UNSERIALIZE_DATETIME(_topicTime);
    UNSERIALIZE_STRING(_topicUser);
}

QHash<QString, QVariant> Channel::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(_topic);
    SERIALIZE(_topicTime);
    SERIALIZE(_topicUser);
    SERIALIZE(_name);
    return hash;
}

void Channel::SetName(const QString &name)
{
    this->_name = name;
}

void Channel::SetTopic(const QString &topic)
{
    this->_topic = topic;
}

