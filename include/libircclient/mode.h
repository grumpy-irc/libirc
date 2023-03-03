//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef MODE_H
#define MODE_H

#include "libircclient_global.h"
#include <QString>
#include <QDateTime>
#include <QChar>
#include "user.h"
#include "../libirc/mode.h"

namespace libircclient
{
	class LIBIRCCLIENTSHARED_EXPORT Mode : public libirc::Mode
    {
        public:
            Mode()=default;
            Mode(const QHash<QString, QVariant> &hash);
            Mode(const QString &mode) : libirc::Mode(mode) {}
    };

    class LIBIRCCLIENTSHARED_EXPORT UMode : public Mode
    {
        public:
            UMode()=default;
            UMode(const QHash<QString, QVariant> &hash);
            UMode(const QString &mode_string) : Mode(mode_string) {}
    };

    class LIBIRCCLIENTSHARED_EXPORT CMode : public Mode
    {
        public:
            CMode()=default;
            CMode(const QHash<QString, QVariant> &hash);
            CMode(const QString &mode_string) : Mode(mode_string) {};
    };

    class LIBIRCCLIENTSHARED_EXPORT ChannelPMode : public libirc::SingleMode
    {
        public:
            ChannelPMode(const QString &mode);
            ChannelPMode(const QHash<QString, QVariant> &mode);
            ~ChannelPMode() override=default;
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;
            char Symbol;
            QDateTime SetOn;
            User SetBy;
            bool operator==(const ChannelPMode& m) const { return m.EqualTo(*this); }
            bool operator!=(const ChannelPMode& m) const { return !m.EqualTo(*this); }
            bool EqualTo(const ChannelPMode& m) const;
    };

    uint qHash(const ChannelPMode& mode, uint seed = 0);
}

#endif // MODE_H
