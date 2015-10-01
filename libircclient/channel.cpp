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
#include "network.h"

using namespace libircclient;

Channel::Channel(QString name, Network *network) : libirc::Channel(name)
{
    this->_net = network;
}

void Channel::SendMessage(QString text)
{
    if (!this->_net)
        return;
    this->_net->SendMessage(text, this);
}

