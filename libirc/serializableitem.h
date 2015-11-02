//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SERIALIZABLEITEM_H
#define SERIALIZABLEITEM_H

#include <QVariant>
#include <QHash>
#include <QString>
#include <QMutex>
#include "libirc_global.h"

#define SERIALIZE(variable_name)          hash.insert(#variable_name, QVariant(variable_name))
#define SERIALIZE_CCHAR(variable_name)    hash.insert(#variable_name, QVariant(QChar(variable_name)))
#define UNSERIALIZE_BOOL(variable_name)       if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toBool(); }
#define UNSERIALIZE_HASH(variable_name)       if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toHash(); }
#define UNSERIALIZE_INT(variable_name)        if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toInt(); }
#define UNSERIALIZE_UINT(variable_name)       if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toUInt(); }
#define UNSERIALIZE_STRING(variable_name)     if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toString(); }
#define UNSERIALIZE_LIST(variable_name)       if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toList(); }
#define UNSERIALIZE_CCHAR(variable_name)      if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toChar().toLatin1(); }
#define UNSERIALIZE_DATETIME(variable_name)   if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toDateTime(); }
#define UNSERIALIZE_ULONGLONG(variable_name)  if (hash.contains(#variable_name)) { variable_name = hash[#variable_name].toULongLong(); }

namespace libirc
{
    class LIBIRCSHARED_EXPORT SerializableItem
    {
        public:
            static const unsigned long long LIBIRC_UNKNOWN_RPC_ID;

            SerializableItem();
            virtual ~SerializableItem();
            virtual QHash<QString, QVariant> ToHash();
            virtual void LoadHash(QHash<QString, QVariant> hash);
            virtual void RPC(int function, QList<QVariant> parameters);
            virtual bool SupportsRPC() { return false; }
            virtual unsigned long long __rpc_GetID();
        protected:
            static QHash<unsigned long long, SerializableItem*> __rpc_cache;
            static QMutex __rpc_lock;

            static unsigned long long __rpc_currentID;
            unsigned long long __rpc_id;
    };
}

#endif // SERIALIZABLEITEM_H
