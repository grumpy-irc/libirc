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

ChannelPMode::ChannelPMode(QString mode) : libirc::SingleMode(mode)
{

}

ChannelPMode::ChannelPMode(QHash<QString, QVariant> mode) : libirc::SingleMode(mode)
{
    this->LoadHash(mode);
}

ChannelPMode::~ChannelPMode()
{

}

void ChannelPMode::LoadHash(QHash<QString, QVariant> hash)
{
    SingleMode::LoadHash(hash);
    UNSERIALIZE_DATETIME(SetOn);
    if (hash.contains("SetBy"))
        this->SetBy.LoadHash(hash["SetBy"].toHash());
}

QHash<QString, QVariant> ChannelPMode::ToHash()
{
    QHash<QString, QVariant> hash = SingleMode::ToHash();
    hash.insert("SetBy", this->SetBy.ToHash());
    SERIALIZE(SetOn);
    return hash;
}

bool libircclient::ChannelPMode::EqualTo(const ChannelPMode& m) const
{
    return m.mode == this->mode && m.Parameter == this->Parameter;
}

uint libircclient::qHash(const ChannelPMode & mode, uint seed)
{
    return qHash(mode.Parameter, seed) ^ mode.Get();
}
