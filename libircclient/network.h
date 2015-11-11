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
#include <QSslSocket>
#include <QThread>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QTimer>
#include "libircclient_global.h"
#include "../libirc/irc_standards.h"

class QTcpSocket;

#ifndef ETIMEDOUT
#define ETIMEDOUT     10
#endif
#define EHANDSHAKE    20
#define EDISCONNECTED 30

namespace libirc
{
    class ServerAddress;
}

namespace libircclient
{
    enum Priority
    {
        Priority_High = 2,
        Priority_Normal = 1,
        Priority_Low = 0
    };

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
            friend class Network_SenderThread;

            Network(libirc::ServerAddress &server, QString name);
            Network(QHash<QString, QVariant> hash);
            virtual ~Network();
            virtual void Connect();
            virtual void Reconnect();
            virtual void Disconnect(QString reason = "");
            virtual bool IsConnected();
            //! This function can be used to change the default nickname that will be requested upon connection to server
            //! subsequent calls of this function while on active IRC connection will be ignored.
            virtual void SetDefaultNick(QString nick);
            virtual void SetDefaultIdent(QString ident);
            virtual void SetDefaultUsername(QString realname);
            virtual bool IsSSL();
            QString GetNick();
            QString GetHost();
            QString GetIdent();
            QString GetServerAddress();
            virtual void SetPassword(QString Password);
            virtual void TransferRaw(QString raw, Priority priority = Priority_Normal);
            virtual int SendMessage(QString text, Channel *channel, Priority priority = Priority_Normal);
            virtual int SendMessage(QString text, User *user, Priority priority = Priority_Normal);
            virtual int SendMessage(QString text, QString target, Priority priority = Priority_Normal);
            virtual int SendAction(QString text, Channel *channel, Priority priority = Priority_Normal);
            virtual int SendAction(QString text, QString target, Priority priority = Priority_Normal);
            virtual int GetTimeout() const;
            virtual void RequestPart(QString channel_name, Priority priority = Priority_Normal);
            virtual void RequestPart(Channel *channel, Priority priority = Priority_Normal);
            virtual void RequestNick(QString nick, Priority priority = Priority_Normal);
            virtual void Identify(QString Nickname = "", QString Password = "", Priority priority = Priority_Normal);
            virtual bool ContainsChannel(QString channel_name);
            //////////////////////////////////////////////////////////////////////////////////////////
            // Synchronization tools
            //! This will update the nick in operating memory, it will not request it from server and may cause troubles
            //! if not properly called. This is only used for resynchronization.
            virtual void _st_SetNick(QString nick);
            //! This will delete all internal memory structures related to channels this user is in.
            //! Use only for synchronization purposes, while implementing something like grumpyd
            //! calling this function on live IRC network connection will have unpredictable result
            void _st_ClearChannels();
            /*!
             * \brief InsertChannel Inserts a deep copy of provided channel to local list of channels
             * \param channel Source which is copied, can be safely deleted
             * \return The copy of input object which was inserted to local vector of channels same as result of GetChannel
             */
            Channel *_st_InsertChannel(libircclient::Channel *channel);
            //////////////////////////////////////////////////////////////////////////////////////////
            virtual Channel *GetChannel(QString channel_name);
            virtual QList<Channel *> GetChannels();
            virtual User *GetLocalUserInfo();
            /*!
             * \brief StartsWithCUPrefix checks the user name whether it starts with a CUMode prefix (such as @)
             * \param user_name Name of user including the prefix (@channel_op)
             * \return 0 in case there is no prefix, otherwise it returns the respective CUMode (o in case of @)
             */
            virtual char StartsWithCUPrefix(QString user_name);
            /*!
             * \brief PositionOfChannelPrefix returns a position of UCP or negative number in case it's not in there
             * \param prefix
             * \return
             */
            virtual int PositionOfUCPrefix(char prefix);
            virtual void SetChannelUserPrefixes(QList<char> data);
            virtual void SetCModes(QList<char> data);
            virtual QList<char> GetChannelUserPrefixes();
            virtual QList<char> GetCModes();
            virtual QList<char> GetCPModes();
            virtual void SetCPModes(QList<char> data);
            virtual void SetCRModes(QList<char> data);
            virtual QList<char> GetCCModes();
            virtual QList<char> GetCRModes();
            virtual void SetCUModes(QList<char> data);
            virtual QList<char> GetCUModes();
            virtual void SetCCModes(QList<char> data);
            virtual UMode GetLocalUserMode();
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            bool ResolveOnNickConflicts;
            QHash<char, QString> ChannelModeHelp;
            QHash<char, QString> UserModeHelp;

        signals:
            void Event_RawOutgoing(QByteArray data);
            void Event_RawIncoming(QByteArray data);
            void Event_Invalid(QByteArray data);
            void Event_ConnectionFailure(QAbstractSocket::SocketError reason);
            void Event_ConnectionError(QString error, int code);
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
            void Event_MOTDEnd(libircclient::Parser *parser);
            void Event_MOTDBegin(libircclient::Parser *parser);
            void Event_MOTD(libircclient::Parser *parser);
            void Event_Mode(libircclient::Parser *parser);
            void Event_NickCollision(libircclient::Parser *parser);
            void Event_WhoisInfo(libircclient::Parser *parser);
            void Event_INFO(libircclient::Parser *parser);
            void Event_PRIVMSG(libircclient::Parser *parser);
            void Event_CTCP(libircclient::Parser *parser, QString ctcp, QString parameters);
            void Event_EndOfNames(libircclient::Parser *parser);
            void Event_NOTICE(libircclient::Parser *parser);
            void Event_NICK(libircclient::Parser *parser, QString old_nick, QString new_nick);
            void Event_SelfNICK(libircclient::Parser *parser, QString old_nick, QString new_nick);
            //! IRC_NUMERIC_MYINFO
            void Event_MyInfo(libircclient::Parser *parser);
            //! Emitted when someone changes the topic
            void Event_TOPIC(libircclient::Parser *parser, libircclient::Channel * chan, QString old_topic);
            //! Retrieved after channel is joined as part of info
            void Event_TOPICInfo(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_TOPICWhoTime(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_SSLFailure(QList<QSslError> error_l, bool *fail);
            //! Server gave us some unknown command
            void Event_Unknown(libircclient::Parser *parser);
            void Event_Timeout();
            void Event_Connected();
            void Event_Disconnected();
            void Event_WHO(libircclient::Parser *parser, libircclient::Channel *channel, libircclient::User *user);
            void Event_EndOfWHO(libircclient::Parser *parser);
            void Event_ModeInfo(libircclient::Parser *parser);
            void Event_CreationTime(libircclient::Parser *parser);

        protected slots:
            virtual void OnSslHandshakeFailure(QList<QSslError> errors);
            virtual void OnError(QAbstractSocket::SocketError er);
            virtual void OnReceive();
            virtual void OnDisconnect();
            virtual void OnConnected();
            virtual void OnSend();
            virtual void OnPing();
            virtual void OnPingSend();

        protected:
            virtual void OnReceive(QByteArray data);
            virtual void closeError(QString error, int code);
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
            void process433(Parser *parser);
            void processInfo(Parser *parser);
			void processWho(Parser *parser);
            void processPrivMsg(Parser *parser);
            void processMode(Parser *parser);
            void processMdIn(Parser *parser);
            void processTopic(Parser *parser);
            void processKick(Parser *parser, bool self_command);
            void processTopicWhoTime(Parser *parser);
            void processMTime(Parser *parser);
            void processJoin(Parser *parser, bool self_command);
            void processNick(Parser *parser, bool self_command);
            void deleteTimers();
            void initialize();
            void pseudoSleep(unsigned int msec);
            QByteArray getDataToSend();
            void scheduleDelivery(QByteArray data, libircclient::Priority priority);

            /////////////////////////////////////
            // This probably doesn't need syncing
            unsigned int MSDelayOnEmpty;
            unsigned int MSDelayOnOpen;
            unsigned int MSWait;
            QDateTime senderTime;
            QTimer senderTimer;
            QMutex mutex;
            QList<QByteArray> hprFIFO;
            QList<QByteArray> mprFIFO;
            QList<QByteArray> lprFIFO;
            /////////////////////////////////////

            //! List of symbols that are used to prefix users
            QList<char> channelUserPrefixes;
            QList<char> CModes;
            QList<char> CPModes;
            QList<char> CRModes;
            QList<char> CUModes;
            QList<char> CCModes;
            QString originalNick;
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
