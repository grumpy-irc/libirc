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

#include "serializableitem.h"

namespace libirc
{
    class LIBIRCSHARED_EXPORT User : public SerializableItem
    {
        public:
            User()=default;
            User(User *user);
            User(const QString &source);
            ~User() override = default;
            virtual QString GetHost() const;
            virtual void SetHost(const QString &host);
            virtual QString GetNick() const;
            virtual void SetNick(const QString &nick);
            virtual QString GetIdent() const;
            virtual void SetIdent(const QString &ident);
            virtual QString ToString() const;
            virtual void SetRealname(const QString &user);
            virtual QString GetRealname() const;
            //! Returns whether User object is null, this can be verified for functions that
            //! return a copy of object instead of pointer (pass by value), where it's not
            //! possible to return NULL literally.
            bool IsNull();
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;

        protected:
            QString _host;
            QString _ident;
            QString _nick;
            QString _username;

    };
}

#endif // USER_H
