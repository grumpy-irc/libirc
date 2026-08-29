//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef LIBIRCCHANNEL_H
#define LIBIRCCHANNEL_H

#include <QDateTime>
#include "serializableitem.h"

namespace libirc
{
    class LIBIRCSHARED_EXPORT Channel : public SerializableItem
    {
        public:
            Channel(const QString &name);
             ~Channel() override=default;
            virtual QString GetName() const;
            virtual void SetName(const QString &name);
            virtual void SetTopic(const QString &topic);
            virtual QString GetTopic() const;
            virtual void SetTopicTime(const QDateTime &time);
            virtual QDateTime GetTopicTime() const;
            virtual QString GetTopicUser() const;
            virtual void SetTopicUser(const QString &user);
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;

        protected:
            QString _name;
            QString _topic;
            QDateTime _topicTime;
            QString _topicUser;
    };
}

#endif // CHANNEL_H
