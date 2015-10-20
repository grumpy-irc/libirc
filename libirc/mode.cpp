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

using namespace libirc;

Mode::Mode()
{

}

Mode::Mode(QString mode_string)
{
    this->SetMode(mode_string);
}

Mode::~Mode()
{

}

void Mode::SetMode(QString mode_string, bool reset)
{
    int position = 0;
    bool including = true;
    while (position < mode_string.size())
    {
        char sx = mode_string.at(position).toLatin1();
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
        position++;
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

void Mode::LoadHash(QHash<QString, QVariant> hash)
{
    if (hash.contains("included_modes"))
    {
        foreach (QVariant mx, hash["included_modes"].toList())
            this->included_modes.append(mx.toChar().toLatin1());
        foreach (QVariant mx, hash["excluded_modes"].toList())
            this->excluded_modes.append(mx.toChar().toLatin1());
    }
}

QHash<QString, QVariant> Mode::ToHash()
{
    QHash<QString, QVariant> hash;
    QList<QVariant> included, excluded;
    foreach (char m, this->included_modes)
        included.append(QVariant(m));
    foreach (char m, this->excluded_modes)
        excluded.append(QVariant(m));
    hash.insert("included_modes", QVariant(included));
    hash.insert("excluded_modes", QVariant(excluded));
    return hash;
}
