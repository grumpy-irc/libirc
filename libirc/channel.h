//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef LIBIRCCHANNEL_H
#define LIBIRCCHANNEL_H

#include <QDateTime>
#include "serializableitem.h"

namespace libirc
{
    class LIBIRCSHARED_EXPORT Channel : public SerializableItem
    {
        public:
            Channel(QString name);
            virtual ~Channel();
            virtual QString GetName() const;
            virtual void SetName(QString name);
            virtual void SetTopic(QString topic);
            virtual QString GetTopic() const;
            virtual void SetTopicTime(QDateTime time);
            virtual QString GetTopicUser() const;
            virtual void SetTopicUser(QString user);
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();

        protected:
            QString _name;
            QString _topic;
            QDateTime _topicTime;
            QString _topicUser;
    };
}

#endif // CHANNEL_H
