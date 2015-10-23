//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "mode.h"
#include "user.h"

using namespace libircclient;

Mode::Mode()
{

}

Mode::Mode(QHash<QString, QVariant> hash)
{
    this->LoadHash(hash);
}

UMode::UMode()
{

}

UMode::UMode(QHash<QString, QVariant> hash)
{
    this->LoadHash(hash);
}

CMode::CMode()
{

}

CMode::CMode(QHash<QString, QVariant> hash)
{
    this->LoadHash(hash);
}

CMode::CMode(QString mode_string)
{

}

ChannelPMode::ChannelPMode()
{
    this->SetBy = NULL;
}

ChannelPMode::~ChannelPMode()
{
    delete this->SetBy;
}

ChannelException::ChannelException()
{

}
