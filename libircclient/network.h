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
#include <QTcpSocket>
#include "libircclient_global.h"
#include "networkthread.h"

class QTcpSocket;

namespace libircclient
{
    enum Encoding
    {
        EncodingUTF8
    };

    class User;
    class Server;
    class Channel;

    class LIBIRCCLIENTSHARED_EXPORT Network : public libirc::Network
    {
		Q_OBJECT

        public:
            Network(QString Name, QString Hostname, QString Nickname = "", bool SSL = false, QString Password = "", unsigned int Port = 6667);
            virtual ~Network();
			void Connect();
			void Reconnect();
			void Disconnect();
			bool IsConnected();
            QString GetNick() { return this->currentNick; }
            QString GetHost() { return this->hostname; }
            void TransferRaw(QString raw);
            int SendMessage(QString text, Channel *channel);
            int SendMessage(QString text, User *user);

        signals:
            void RawOutgoing(QByteArray data);
            void RawIncoming(QByteArray data);
            void Connected();
            void Disconnected();

        protected:
            bool usingSSL;
            QString currentNick;
            QString currentIdent;
            QString realname;
            QTcpSocket *socket;
            QString hostname;
            unsigned int port;
       private:
            void processIncomingRawData(QByteArray data);
            QList<User> users;
            QList<Channel> channels;
            friend class NetworkThread;
            NetworkThread *network_thread;
    };
}

#endif // NETWORK_H
