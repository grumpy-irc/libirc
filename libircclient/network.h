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
            QList<char> GetCUModes();
            User *GetLocalUserInfo();
            /*!
             * \brief StartsWithCUPrefix checks the user name whether it starts with a CUMode prefix (such as @)
             * \param user_name Name of user including the prefix (@channel_op)
             * \return 0 in case there is no prefix, otherwise it returns the respective CUMode (o in case of @)
             */
            char StartsWithCUPrefix(QString user_name);
            /*!
             * \brief PositionOfChannelPrefix returns a position of UCP or negative number in case it's not in there
             * \param prefix
             * \return
             */
            int PositionOfUCPrefix(char prefix);
            QHash<char, QString> ChannelModeHelp;
            QHash<char, QString> UserModeHelp;
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();

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
            //! Emitted before the channel is removed from memory on part of a channel you were in
            void Event_SelfPart(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_Part(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_SelfKick(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_Kick(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_ServerMode(libircclient::Parser *parser);
            void Event_ChannelMode(libircclient::Parser *parser);
            void Event_MOTD(libircclient::Parser *parser);
            void Event_Mode(libircclient::Parser *parser);
            void Event_WhoisInfo(libircclient::Parser *parser);
            void Event_INFO(libircclient::Parser *parser);
            void Event_PRIVMSG(libircclient::Parser *parser);
            void Event_EndOfNames(libircclient::Parser *parser);
            void Event_NOTICE(libircclient::Parser *parser);
            void Event_NICK(libircclient::Parser *parser, QString old_nick, QString new_nick);
            void Event_SelfNICK(libircclient::Parser *parser, QString old_nick, QString new_nick);
            //! Emitted when someone changes the topic
            void Event_TOPIC(libircclient::Parser *parser, libircclient::Channel * chan, QString old_topic);
            //! Retrieved after channel is joined as part of info
            void Event_TOPICInfo(libircclient::Parser *parser, libircclient::Channel *chan);
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
            void processIncomingRawData(QByteArray data);
            void processNamrpl(Parser *parser);
            void processInfo(Parser *parser);
            void deleteTimers();
            //! List of symbols that are used to prefix users
            QList<char> channelUserPrefixes;
            QList<char> CModes;
            QList<char> CPModes;
            QList<char> CRModes;
            QList<char> CUModes;
            QList<char> CCModes;
            UMode localUserMode;
            QString alternateNick;
            int alternateNickNumber;
            char channelPrefix;
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
