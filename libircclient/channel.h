//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef CHANNEL_H
#define CHANNEL_H

#include "libircclient_global.h"
#include <QString>
#include <QSet>
#include <QList>
#include "mode.h"
#include "../libirc/channel.h"

namespace libircclient
{
    class User;
    class Network;
    class LIBIRCCLIENTSHARED_EXPORT Channel : public libirc::Channel
    {
        public:
            Channel(QHash<QString, QVariant> hash);
            Channel(QString name, Network *network = NULL);
            Channel(Channel *channel);
            Channel(const Channel &channel);
            ~Channel();
            /*!
             * \brief InsertUser Use this to insert a new user to channel, if user already exists it's updated according to information for new user
             * \param user Pointer to user object, this function creates a copy, so the object passed here can be temporary
             * \return Pointer to newly created user or existing user in case there is already this user in a channel
             */
            User *InsertUser(User *user);
            void RemoveUser(QString user);
            void ChangeNick(QString old_nick, QString new_nick);
            bool ContainsUser(QString user);
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            void SendMessage(QString text);
            void SetNetwork(Network *network);
            void ClearUsers();
            QHash<QString, User *> GetUsers() const;
            User *GetUser(QString user);
            QDateTime GetMTime();
            void SetMTime(QDateTime tm);
            QList<ChannelPMode> GetBans();
            QList<ChannelPMode> GetExceptions();
            void RemovePMode(libirc::SingleMode mode);
            void RemovePMode(ChannelPMode mode);
            void SetPMode(ChannelPMode mode);
            CMode GetMode();
            void SetMode(QString mode);
            void Part();
        /*signals:
            void Event_UserInserted(User *user);
            void Event_UserRemoved(QString user);
            void Event_NickChanged(QString old_nick, QString new_nick); */
        protected:
            QList<ChannelPMode> filteredList(char filter);
#ifdef LIBIRC_HASH
            QSet<ChannelPMode> _localPModes;
#else
            QList<ChannelPMode> _localPModes;
#endif
            CMode _localMode;
            QDateTime _localModeDateTime;
            QHash<QString, User*> _users;
            Network *_net;
        private:
            void deepCopy(const Channel *source);
    };
}

#endif // CHANNEL_H
