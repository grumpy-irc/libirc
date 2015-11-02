//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "serializableitem.h"
#include <limits>

using namespace libirc;

const unsigned long long SerializableItem::LIBIRC_UNKNOWN_RPC_ID = std::numeric_limits<unsigned long long>::max();
unsigned long long SerializableItem::__rpc_currentID = 0;
QMutex SerializableItem::__rpc_lock;
QHash<unsigned long long, SerializableItem*> SerializableItem::__rpc_cache;

SerializableItem::SerializableItem()
{
    this->__rpc_id = LIBIRC_UNKNOWN_RPC_ID;
}

libirc::SerializableItem::~SerializableItem()
{
    if (this->__rpc_id == SerializableItem::LIBIRC_UNKNOWN_RPC_ID)
        return;
    // remove this instance from cache if there is some
    __rpc_lock.lock();
    if (__rpc_cache.contains(this->__rpc_id))
        __rpc_cache.remove(this->__rpc_id);
    __rpc_lock.unlock();
}

QHash<QString, QVariant> SerializableItem::ToHash()
{
    QHash<QString, QVariant> hash;
    if (this->SupportsRPC())
    {
        // We call a dummy get id just so that it's generated in case there is none
        this->__rpc_GetID();
        SERIALIZE(__rpc_id);
    }
    return hash;
}

void SerializableItem::LoadHash(QHash<QString, QVariant> hash)
{
    if (!hash.contains("__rpc_id"))
        return;
    UNSERIALIZE_ULONGLONG(__rpc_id);
    __rpc_lock.lock();
    __rpc_cache.insert(__rpc_id, this);
    __rpc_lock.unlock();
}

void SerializableItem::RPC(int function, QList<QVariant> parameters)
{
    Q_UNUSED(function);
    Q_UNUSED(parameters);
}

unsigned long long SerializableItem::__rpc_GetID()
{
    if (this->__rpc_id == SerializableItem::LIBIRC_UNKNOWN_RPC_ID)
    {
        __rpc_lock.lock();
        this->__rpc_id = SerializableItem::__rpc_currentID++;
        __rpc_cache.insert(__rpc_id, this);
        __rpc_lock.unlock();
    }
    return this->__rpc_id;
}

