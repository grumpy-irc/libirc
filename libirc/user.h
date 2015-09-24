//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef LIBIRCUSER_H
#define LIBIRCUSER_H

#include "libirc_global.h"
#include <QString>

namespace libirc
{
    class LIBIRCSHARED_EXPORT User
    {
        public:
            User();
            virtual ~User() {}
            virtual QString GetHost() const;
            virtual void SetHost(const QString &host);
            virtual QString GetIdent() const;
            virtual void SetIdent(QString ident);

        protected:
            QString _host;
            QString _ident;
            QString _nick;
            QString _username;

    };
}

#endif // USER_H
