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
            Mode();
            Mode(QHash<QString, QVariant> hash);
            Mode(QString mode) : libirc::Mode(mode) {}
    };

    class LIBIRCCLIENTSHARED_EXPORT UMode : public Mode
    {
        public:
            UMode();
            UMode(QHash<QString, QVariant> hash);
            UMode(QString mode_string) : Mode(mode_string) {}
    };

    class LIBIRCCLIENTSHARED_EXPORT CMode : public Mode
    {
        public:
            CMode();
            CMode(QHash<QString, QVariant> hash);
            CMode(QString mode_string);
    };

    class LIBIRCCLIENTSHARED_EXPORT ChannelPMode : public libirc::SingleMode
    {
        public:
            ChannelPMode(QString mode);
            ChannelPMode(QHash<QString, QVariant> mode);
            virtual ~ChannelPMode();
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            char Symbol;
            QDateTime SetOn;
            User SetBy;
    };
}

#endif // MODE_H
