//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019


#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QList>
#include <QDateTime>
#include "libircclient_global.h"
#include "user.h"
#include "../libirc/irc_numerics.h"

namespace libircclient
{
    class LIBIRCCLIENTSHARED_EXPORT Parser
    {
        public:
            Parser(QString incoming_text);
            ~Parser();
            int GetNumeric();
            bool IsValid();
            User *GetSourceUserInfo();
            QString GetSourceInfo();
            QString GetParameterLine();
            //! Returns RAW data as received from server, with some alterations to remove some CAP extras (such as server-time)
            //! this RAW text follows IRC RFC preceeding IRCv3
            QString GetRaw();
            //! Returns true RAW data as received from server with no alterations, with IRCv3 caps this text doesn't follow standard convention
            QString GetOriginalRaw();
            QString GetText();
            QList<QString> GetParameters();
            QDateTime GetTimestamp();

        private:
            void obtainNumeric();
            User *user;
            QString source;
            QString text;
            QString original_text;
            QString message_text;
            bool _valid;
            int _numeric;
            QList<QString> parameters;
            QString command;
            QString parameterLine;
            QDateTime timestamp;

    };
}

#endif // PARSER_H
