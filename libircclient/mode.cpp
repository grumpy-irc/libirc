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
#include <QHash>
#include "user.h"

using namespace libircclient;

Mode::Mode(const QHash<QString, QVariant> &hash)
{
    this->LoadHash(hash);
}

UMode::UMode(const QHash<QString, QVariant> &hash)
{
    this->LoadHash(hash);
}

CMode::CMode(const QHash<QString, QVariant> &hash)
{
    this->LoadHash(hash);
}

ChannelPMode::ChannelPMode(const QString &mode) : libirc::SingleMode(mode)
{

}

ChannelPMode::ChannelPMode(const QHash<QString, QVariant> &mode) : libirc::SingleMode(mode)
{
    this->LoadHash(mode);
}

void ChannelPMode::LoadHash(const QHash<QString, QVariant> &hash)
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
    // this overflow on Qt4 for some reasons
    //
    //return qHash(QString(mode.Parameter + mode.Get()), seed);
    //
    if (mode.Parameter.isEmpty())
        return qHash(QString(mode.Get())) * mode.Get();
    return qHash(mode.Parameter, seed) ^ mode.Get();
}
