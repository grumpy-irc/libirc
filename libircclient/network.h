//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef NETWORK_H
#define NETWORK_H

#include "../libirc/network.h"
#include "user.h"
#include "mode.h"
#include <QList>
#include <QString>
#include <QDateTime>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QTimer>
#include "libircclient_global.h"
#include "../libirc/irc_standards.h"

class QTcpSocket;

namespace libirc
{
    class ServerAddress;
}

namespace libircclient
{
    enum Encoding
    {
        EncodingUTF8
    };

    class Server;
    class Channel;
    class Parser;

    class LIBIRCCLIENTSHARED_EXPORT Network : public libirc::Network
    {
        Q_OBJECT

        public:
            Network(libirc::ServerAddress &server, QString name);
            virtual ~Network();
            void Connect();
            void Reconnect();
            void Disconnect(QString reason = "");
            bool IsConnected();
            QString GetNick();
            QString GetHost();
            QString GetIdent();
            QString GetServerAddress();
            void SetPassword(QString Password);
            void TransferRaw(QString raw);
            int SendMessage(QString text, Channel *channel);
            int SendMessage(QString text, User *user);
            int SendMessage(QString text, QString target);
            int GetTimeout() const;
            void Part(QString channel_name);
            void Part(Channel *channel);
            void Identify(QString Nickname = "", QString Password = "");
            bool ContainsChannel(QString channel_name);
            Channel *GetChannel(QString channel_name);
            QList<Channel *> GetChannels();
            User *GetLocalUserInfo();

        signals:
            void Event_RawOutgoing(QByteArray data);
            void Event_RawIncoming(QByteArray data);
            void Event_Invalid(QByteArray data);
            void Event_ConnectionFailure(QAbstractSocket::SocketError reason);
            void Event_Parse(libircclient::Parser *parser);
            void Event_SelfJoin(libircclient::Channel *chan);
            void Event_Join(libircclient::Parser *parser, libircclient::User *user, libircclient::Channel *chan);
            /*!
             * \brief Event_PerChannelQuit Emitted when a user quit the network for every single channel this user was in
             *                             so that it's extremely simple to render the information in related scrollbacks
             * \param parser   Pointer to parser of IRC raw message
             * \param chan     Pointer to channel this user just left
             */
            void Event_PerChannelQuit(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_Quit(libircclient::Parser *parser);
            void Event_Part(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_Kick(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_ServerMode(libircclient::Parser *parser);
            void Event_ChannelMode(libircclient::Parser *parser);
            void Event_MOTD(libircclient::Parser *parser);
            void Event_Mode(libircclient::Parser *parser);
            void Event_WhoisInfo(libircclient::Parser *parser);
            void Event_PRIVMSG(libircclient::Parser *parser);
            void Event_NOTICE(libircclient::Parser *parser);
            void Event_NICK(libircclient::Parser *parser, QString old_nick, QString new_nick);
            void Event_SelfNICK(libircclient::Parser *parser, QString old_nick, QString new_nick);
            //! Server gave us some unknown command
            void Event_Unknown(libircclient::Parser *parser);
            void Event_Timeout();
            void Event_Connected();
            void Event_Disconnected();

        private slots:
            void OnPing();
            void OnPingSend();
            void OnError(QAbstractSocket::SocketError er);
            void OnReceive();
            void OnConnected();

        protected:
            bool usingSSL;
            QTcpSocket *socket;
            QString hostname;
            unsigned int port;
            int pingTimeout;
            int pingRate;
            QString defaultQuit;
            bool autoRejoin;
            bool autoIdentify;
            QString identifyString;
            QString password;
        private:
            UMode localUserMode;
            char channelPrefix;
            void processIncomingRawData(QByteArray data);
            void deleteTimers();
            Server *server;
            QList<User*> users;
            QList<Channel*> channels;
            User localUser;
            QDateTime lastPing;
            QTimer *timerPingTimeout;
            QTimer *timerPingSend;
    };
}

#endif // NETWORK_H
