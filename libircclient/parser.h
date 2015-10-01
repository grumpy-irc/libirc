//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015


#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QList>
#include "libircclient_global.h"
#include "user.h"

#define IRC_NUMERIC_UNKNOWN    -200
#define IRC_NUMERIC_PING_CHECK -1
#define IRC_NUMERIC_PRIVMSG    -2
#define IRC_NUMERIC_NOTICE     -3
#define IRC_NUMERIC_KICK       -4
#define IRC_NUMERIC_QUIT       -5
#define IRC_NUMERIC_PART       -6
#define IRC_NUMERIC_JOIN       -7
#define IRC_NUMERIC_WELCOME    1
#define IRC_NUMERIC_YOURHOST   2
#define IRC_NUMERIC_CREATED    3
#define IRC_NUMERIC_MYINFO     4
#define IRC_NUMERIC_BOUNCE     5
#define IRC_NUMERIC_ISUPPORT   5
#ifndef LIBIRC_STRICT
#define IRC_NUMERIC_MAP        6 // Unreal specific
#define IRC_NUMERIC_MAPEND     7 // Unreal specific
#endif


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
            QList<QString> GetParameters();

        private:
            void obtainNumeric();
            User *user;
            QString source;
            bool _valid;
            int _numeric;
            QList<QString> parameters;
            QString command;
            QString parameterLine;

    };
}

#endif // PARSER_H
