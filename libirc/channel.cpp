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

using namespace libirc;

Channel::Channel(QString name)
{
    this->_name = name;
}

Channel::~Channel()
{

}

QString Channel::GetName() const
{
    return this->_name;
}

QString Channel::GetTopic() const
{
    return this->_topic;
}

void Channel::SetName(QString name)
{
    this->_name = name;
}

void Channel::SetTopic(QString topic)
{
    this->_topic = topic;
}

