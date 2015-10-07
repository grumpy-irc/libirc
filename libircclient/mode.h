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
#include "../libirc/mode.h"

namespace libircclient
{
    class User;

	class LIBIRCCLIENTSHARED_EXPORT Mode : public libirc::Mode
    {
        public:
            Mode();
            Mode(QString mode) : libirc::Mode(mode) {}
    };

    class LIBIRCCLIENTSHARED_EXPORT UMode : public Mode
    {
        public:
            UMode();
            UMode(QString mode_string) : Mode(mode_string) {}
    };

    class LIBIRCCLIENTSHARED_EXPORT CMode : public Mode
    {
        public:
            CMode();
            CMode(QString mode_string);
    };

    class LIBIRCCLIENTSHARED_EXPORT ChannelPMode
    {
        public:
            ChannelPMode();
            virtual ~ChannelPMode();
            char Symbol;
            QString Parameter;
            QDateTime SetOn;
            User *SetBy;
    };

    class LIBIRCCLIENTSHARED_EXPORT ChannelBan : public ChannelPMode
    {
        public:
            ChannelBan();
    };

    class LIBIRCCLIENTSHARED_EXPORT ChannelInvite : public ChannelPMode
    {
        public:
            ChannelInvite();
    };

    class LIBIRCCLIENTSHARED_EXPORT ChannelException : public ChannelPMode
    {
        public:
            ChannelException();
    };
}

#endif // MODE_H
