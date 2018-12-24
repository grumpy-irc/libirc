//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "mode.h"
#include <QDebug>

using namespace libirc;

QList<SingleMode> SingleMode::ToModeList(const QString &mode_string, QList<QString> parameters, const QList<char> &parameter_modes)
{
    QList<SingleMode> modes;
    int position = 0;
    char prefix = MODE_INCLUDE;
    while (position < mode_string.size())
    {
        char sx = mode_string.at(position++).toLatin1();
        if (sx == MODE_EXCLUDE || sx == MODE_INCLUDE)
        {
            prefix = sx;
        }
        else if (parameter_modes.contains(sx))
        {
            if (parameters.isEmpty())
            {
                qDebug() << "Invalid mode: " + mode_string + " missing parameters";
                return modes;
            }
            SingleMode mode(QString(QChar(prefix)) + QString(QChar(sx)));
            mode.Parameter = parameters.first();
            parameters.removeAt(0);
            modes.append(mode);
        } else
        {
            modes.append(SingleMode((QString(QChar(prefix)) + QString(QChar(sx)))));
        }
    }
    return modes;
}

Mode::Mode(const QString &mode_string)
{
    this->SetMode(mode_string);
}

void Mode::SetMode(const QString &mode_string, bool reset)
{
    int position = 0;
    bool including = true;
    while (position < mode_string.size())
    {
        char sx = mode_string.at(position++).toLatin1();
        if (sx == MODE_EXCLUDE)
            including = false;
        else if (sx == MODE_INCLUDE)
            including = true;
        else if (including)
            this->IncludeMode(sx);
        else if (!including)
        {
            if (reset)
            {
                this->ResetMode(sx);
            } else
                this->ExcludeMode(sx);
        }
    }
}

bool Mode::Includes(char mode)
{
    return this->included_modes.contains(mode);
}

bool Mode::Excludes(char mode)
{
    return this->excluded_modes.contains(mode);
}

bool Mode::IsEmpty()
{
    return (this->excluded_modes.isEmpty() && this->included_modes.isEmpty());
}

void Mode::IncludeMode(char mode)
{
    if (this->included_modes.contains(mode))
        return;
    if (this->excluded_modes.contains(mode))
        this->excluded_modes.removeAll(mode);
    this->included_modes.append(mode);
}

void Mode::ExcludeMode(char mode)
{
    if (this->excluded_modes.contains(mode))
        return;
    if (this->included_modes.contains(mode))
        this->included_modes.removeAll(mode);
    this->excluded_modes.append(mode);
}

void Mode::ResetMode(char mode)
{
    if (this->included_modes.contains(mode))
        this->included_modes.removeOne(mode);
    if (this->excluded_modes.contains(mode))
        this->excluded_modes.removeOne(mode);
}

void Mode::ResetModes(QList<char> modes)
{
    foreach (char mode, modes)
        this->ResetMode(mode);
}

QList<char> Mode::GetExcluding()
{
    return this->excluded_modes;
}

QList<char> Mode::GetIncluding()
{
    return this->included_modes;
}

QString Mode::ToString()
{
    QString mode;
    if (!this->included_modes.isEmpty())
    {
        mode += MODE_INCLUDE;
        foreach (char sx, this->included_modes)
            mode += sx;
    }
    if (!this->excluded_modes.isEmpty())
    {
        mode += MODE_EXCLUDE;
        foreach (char sx, this->excluded_modes)
            mode += sx;
    }
    return mode;
}

void Mode::LoadHash(const QHash<QString, QVariant> &hash)
{
    if (hash.contains("excluded_modes"))
    {
        foreach (QVariant mx, hash["excluded_modes"].toList())
            this->excluded_modes.append(mx.toChar().toLatin1());
    }
    if (hash.contains("included_modes"))
    {
        foreach (QVariant mx, hash["included_modes"].toList())
            this->included_modes.append(mx.toChar().toLatin1());
    }
    UNSERIALIZE_STRING(Parameter);
}

QHash<QString, QVariant> Mode::ToHash()
{
    QHash<QString, QVariant> hash = SerializableItem::ToHash();
    QList<QVariant> included, excluded;
    SERIALIZE(Parameter);
    foreach (char m, this->included_modes)
        included.append(QVariant(m));
    foreach (char m, this->excluded_modes)
        excluded.append(QVariant(m));
    hash.insert("included_modes", QVariant(included));
    hash.insert("excluded_modes", QVariant(excluded));
    return hash;
}

SingleMode::SingleMode(QString mode)
{
    if (mode.size() > 2)
    {
        this->valid = false;
        return;
    }
    if (mode.size() == 2)
    {
        char px = mode[0].toLatin1();
        if (px == MODE_EXCLUDE || px == MODE_INCLUDE)
        {
            this->valid = true;
            this->mode = mode[1].toLatin1();
            this->including = px == MODE_INCLUDE;
        } else
        {
            this->valid = false;
            return;
        }
    } else
    {
        this->valid = true;
        this->including = true;
        this->mode = mode[0].toLatin1();
    }
}

SingleMode::SingleMode(const QHash<QString, QVariant> &hash)
{
    this->including = false;
    this->valid = false;
    this->mode = 0;
    this->LoadHash(hash);
}

bool SingleMode::IsIncluding()
{
    return this->including;
}

bool SingleMode::IsValid()
{
    return this->valid;
}

QString libirc::SingleMode::ToString() const
{
    QString prefix;
    if (this->including)
        prefix = "+";
    else
        prefix = "-";
    QString params;
    if (!this->Parameter.isEmpty())
        params = " " + this->Parameter;
    return prefix + QString(this->mode) + params;
}

char SingleMode::Get() const
{
    return this->mode;
}

void SingleMode::LoadHash(const QHash<QString, QVariant> &hash)
{
    UNSERIALIZE_CCHAR(mode);
    UNSERIALIZE_BOOL(including);
    UNSERIALIZE_BOOL(valid);
    UNSERIALIZE_STRING(Parameter);
}

QHash<QString, QVariant> SingleMode::ToHash()
{
    QHash<QString, QVariant> hash = SerializableItem::ToHash();
    SERIALIZE(valid);
    SERIALIZE_CCHAR(mode);
    SERIALIZE(including);
    SERIALIZE(Parameter);
    return hash;
}
