//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018


#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QList>
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
            QString GetRaw();
            QString GetText();
            QList<QString> GetParameters();

        private:
            void obtainNumeric();
            User *user;
            QString source;
            QString text;
            QString message_text;
            bool _valid;
            int _numeric;
            QList<QString> parameters;
            QString command;
            QString parameterLine;

    };
}

#endif // PARSER_H
