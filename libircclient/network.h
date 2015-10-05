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

    class User;
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
            QString GetNick() { return this->currentNick; }
            QString GetHost() { return this->hostname; }
            QString GetIdent() { return this->currentIdent; }
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

        signals:
            void Event_RawOutgoing(QByteArray data);
            void Event_RawIncoming(QByteArray data);
            void Event_Invalid(QByteArray data);
            void Event_ConnectionFailure(QAbstractSocket::SocketError reason);
            void Event_Parse(Parser *parser);
            void Event_SelfJoin(Channel *chan);
            void Event_Join(Parser *parser);
            //! Server gave us some unknown command
            void Event_Unknown(Parser *parser);
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
            QString currentNick;
            QString currentIdent;
            QString realname;
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
            void processIncomingRawData(QByteArray data);
            void deleteTimers();
            Server *server;
            QList<User*> users;
            QList<Channel*> channels;
            QDateTime lastPing;
            QTimer *timerPingTimeout;
            QTimer *timerPingSend;
    };
}

#endif // NETWORK_H
