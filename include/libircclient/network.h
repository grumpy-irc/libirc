//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef NETWORK_H
#define NETWORK_H

#include "../libirc/network.h"
#include "priority.h"
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
    enum Encoding
    {
        // Default by Qt
        EncodingDefault = 0,
        EncodingASCII = 1,
        EncodingUTF8 = 2,
        EncodingUTF16 = 3,
        //EncodingUTF32 = 4,
        EncodingLatin = 5
    };

    class Server;
    class Channel;
    class Parser;

    class LIBIRCCLIENTSHARED_EXPORT Network : public libirc::Network
    {
        Q_OBJECT

        public:
            friend class Network_SenderThread;

            Network(libirc::ServerAddress &server, const QString &name, const Encoding &enc = EncodingDefault);
            Network(const QHash<QString, QVariant> &hash);
             ~Network() override;
            virtual void Connect();
            virtual void Reconnect();
            virtual void Disconnect(QString reason = "");
            bool IsAway() const;
            virtual bool IsConnected();
            virtual void SetAway(bool away, const QString &message = "");
            //! This function can be used to change the default nickname that will be requested upon connection to server
            //! subsequent calls of this function while on active IRC connection will be ignored.
            virtual void    SetDefaultNick(const QString &nick);
            virtual void    SetDefaultIdent(const QString &ident);
            virtual void    SetDefaultUsername(const QString &realname);
            virtual bool    IsSSL();
            virtual QString GetNick();
            virtual QString GetHost();
            virtual QString GetServerVersion();
            virtual int     GetTimeout() const;
            unsigned int    GetPort();
            virtual QString GetIdent();
            virtual QString GetServerAddress();
            virtual User    *GetLocalUserInfo();
            virtual void    SetHelpForMode(char mode, const QString &message);
            virtual QString GetHelpForMode(char mode, QString missing);
            virtual Channel *GetChannel(QString channel_name);
            virtual QList<Channel *> GetChannels();
            virtual Encoding GetEncoding();
            virtual void SetPassword(const QString &Password);
            virtual void RequestJoin(const QString &name, Priority priority = Priority_Normal);
            virtual void TransferRaw(QString raw, Priority priority = Priority_Normal);
            virtual int SendMessage(const QString &text, Channel *channel, Priority priority = Priority_Normal);
            virtual int SendMessage(const QString &text, User *user, Priority priority = Priority_Normal);
            virtual int SendMessage(const QString &text, const QString &target, Priority priority = Priority_Normal);
            virtual int SendAction(const QString &text, Channel *channel, Priority priority = Priority_Normal);
            virtual int SendAction(const QString &text, const QString &target, Priority priority = Priority_Normal);
            virtual int SendNotice(const QString &text, User *user, Priority priority = Priority_Normal);
            virtual int SendNotice(const QString &text, Channel *channel, Priority priority = Priority_Normal);
            virtual int SendNotice(const QString &text, const QString &target, Priority priority = Priority_Normal);
            virtual int SendCtcp(const QString &name, const QString &text, const QString &target, Priority priority = Priority_Normal);
            virtual void RequestPart(const QString &channel_name, Priority priority = Priority_Normal);
            virtual void RequestPart(Channel *channel, Priority priority = Priority_Normal);
            virtual void RequestNick(const QString &nick, Priority priority = Priority_Normal);
            virtual void Identify(QString Nickname = "", QString Password = "", Priority priority = Priority_Normal);
            // IRCv3
            virtual bool SupportsIRCv3() const;
            virtual void EnableIRCv3Support();
            virtual void DisableIRCv3Support();
            //! Requests IRCv3 capability, this needs to be called before connection is initiated
            virtual void RequestCapability(const QString &capability);
            //! Remove a capability from list of requested caps, meaning we will not ask for it after connecting to server
            virtual void DisableCapability(const QString &capability);
            //! Returns true in case this capability is on a list of caps we want to enabled after connection to network
            virtual bool CapabilityRequested(const QString &capability);
            virtual bool CapabilityEnabled(const QString &capability);
            //! Works only after connection is established - will return true in case network supports this capability
            virtual bool CapabilitySupported(const QString &capability);
            virtual QList<QString> GetSupportedCaps();
            virtual QList<QString> GetSubscribedCaps();
            virtual bool ContainsChannel(const QString &channel_name);
            //! Returns a network lag in MS, measured from last PONG response
            virtual long long GetLag();
            virtual long long GetBytesSent();
            virtual long long GetBytesReceived();
            //////////////////////////////////////////////////////////////////////////////////////////
            // Synchronization tools
            //! This will update the nick in operating memory, it will not request it from server and may cause troubles
            //! if not properly called. This is only used for resynchronization.
            virtual void _st_SetNick(const QString &nick);
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
            virtual void SetChannelUserPrefixes(const QList<char> &data);
            virtual void SetCModes(const QList<char> &data);
            virtual QList<char> GetChannelUserPrefixes();
            virtual bool HasCap(const QString &cap);
            virtual QList<char> GetCModes();
            virtual QList<char> GetCPModes();
            virtual void SetCPModes(const QList<char> &data);
            virtual void SetCRModes(const QList<char> &data);
            virtual QList<char> GetCCModes();
            virtual QList<char> GetCRModes();
            virtual QList<char> GetSTATUSMSGModes();
            virtual void SetSTATUSMSGModes(const QList<char> &data);
            virtual void SetCUModes(const QList<char> &data);
            virtual QList<char> GetCUModes();
            virtual void SetCCModes(const QList<char> &data);
            virtual UMode GetLocalUserMode();
            virtual QList<char> ModeHelper_GetSortedChannelPrefixes(const QList<char> &unsorted_list);
            virtual QList<char> ModeHelper_GetSortedCUModes(const QList<char> &unsorted_list);
            QList<char> ParameterModes();
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;
            //! This will automatically fix your own identification data in case they change
            //! For example if server changes your hostname (cloak) system will recognize
            //! the change and update localUser accordingly.
            //!
            //! There is a small CPU penalty for this, so this feature can be turned off
            //! in case that this information isn't important for you
            bool ResolveOnSelfChanges;
            bool ResolveOnNickConflicts;
            QHash<char, QString> ChannelModeHelp;
            QHash<char, QString> UserModeHelp;

        signals:
            // Primitives
            void Event_RawOutgoing(QByteArray data);
            void Event_RawIncoming(QByteArray data);
            void Event_Invalid(QByteArray data);
            void Event_ConnectionFailure(QAbstractSocket::SocketError reason);
            void Event_ConnectionError(QString error, int code);
            void Event_Parse(libircclient::Parser *parser);
            void Event_SSLFailure(QList<QSslError> error_l, bool *fail);
            //! Server gave us some unknown command
            void Event_Unknown(libircclient::Parser *parser);
            void Event_Timeout();
            void Event_Connected();
            void Event_Disconnected();
            void Event_Broken(libircclient::Parser *parser, QString reason);
            void Event_NetworkFailure(QString reason, int failure);
            //! Emitted when server sent us IRC_NUMERIC_UNKNOWN
            void Event_NUMERIC_UNKNOWN(libircclient::Parser *parser);

            // Channel related
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
            //! Emitted when someone changes the topic
            void Event_TOPIC(libircclient::Parser *parser, libircclient::Channel * chan, QString old_topic);
            //! Retrieved after channel is joined as part of info
            void Event_TOPICInfo(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_TOPICWhoTime(libircclient::Parser *parser, libircclient::Channel *chan);
            void Event_ModeInfo(libircclient::Parser *parser, libircclient::Channel *channel);
            //! When user's channel mode was changed, but the changed mode was lower priority than the one which
            //! user already possesed.
            //! This change is very minor and probably doesn't reflect any real change

            //! Exampe: user who had owner ~ and halfop % (effectively being ~%) had halfop removed
            void Event_ChannelUserSubmodeChanged(libircclient::Parser *parser, libircclient::Channel *channel, libircclient::User *user);
            void Event_ChannelModeChanged(libircclient::Parser *parser, libircclient::Channel *channel);
            void Event_ChannelUserModeChanged(libircclient::Parser *parser, libircclient::Channel *channel, libircclient::User *user);
            void Event_CreationTime(libircclient::Parser *parser);
            void Event_EndOfBans(libircclient::Parser *parser);
            void Event_EndOfExcepts(libircclient::Parser *parser);
            void Event_EndOfInvites(libircclient::Parser *parser);
            void Event_CPMInserted(libircclient::Parser *parser, libircclient::ChannelPMode mode, libircclient::Channel *channel);
            void Event_CPMRemoved(libircclient::Parser *parser, libircclient::ChannelPMode mode, libircclient::Channel *channel);
            void Event_INVITE(libircclient::Parser *parser);

            // Server related
            void Event_PONG(libircclient::Parser *parser);
            void Event_EndOfNames(libircclient::Parser *parser);
            void Event_ServerMode(libircclient::Parser *parser);
            void Event_MOTDEnd(libircclient::Parser *parser);
            void Event_MOTDBegin(libircclient::Parser *parser);
            void Event_MOTD(libircclient::Parser *parser);
            void Event_Mode(libircclient::Parser *parser);
            void Event_NickCollision(libircclient::Parser *parser);
            void Event_INFO(libircclient::Parser *parser);
            //! IRC_NUMERIC_MYINFO
            void Event_MyInfo(libircclient::Parser *parser);
            void Event_Welcome(libircclient::Parser *parser);
            void Event_ISUPPORT(libircclient::Parser *parser);

            // Whois
            //! Emitted for all WHOIS events, in case you don't want to attach to individual replies
            void Event_WhoisGeneric(libircclient::Parser *parser);
            void Event_WhoisUser(libircclient::Parser *parser, libircclient::User *user);
            void Event_WhoisOperator(libircclient::Parser *parser);
            void Event_WhoisIdle(libircclient::Parser *parser, unsigned int seconds_idle, QDateTime signon_time);
            void Event_WhoisRegNick(libircclient::Parser *parser);
            void Event_WhoisChannels(libircclient::Parser *parser);
            void Event_WhoisServer(libircclient::Parser *parser);
            void Event_WhoisEnd(libircclient::Parser *parser);
            void Event_WhoisSpecial(libircclient::Parser *parser);
            void Event_WhoisAccount(libircclient::Parser *parser);
            void Event_WhoisSecure(libircclient::Parser *parser);
            void Event_WhoisHost(libircclient::Parser *parser);
            void Event_WhoisModes(libircclient::Parser *parser);

            // Messaging
            void Event_PRIVMSG(libircclient::Parser *parser);
            void Event_CTCP(libircclient::Parser *parser, QString ctcp, QString parameters);
            void Event_NOTICE(libircclient::Parser *parser);

            // Users
            void Event_UserAwayStatusChange(libircclient::Parser *parser, libircclient::Channel *channel, libircclient::User *user);
            void Event_NICK(libircclient::Parser *parser, QString old_nick, QString new_nick);
            void Event_SelfNICK(libircclient::Parser *parser, QString old_nick, QString new_nick);
            void Event_WHO(libircclient::Parser *parser, libircclient::Channel *channel, libircclient::User *user);
            void Event_EndOfWHO(libircclient::Parser *parser);
            void Event_PMode(libircclient::Parser *parser, char mode);
            void Event_UnAway(libircclient::Parser *parser);
            void Event_NowAway(libircclient::Parser *parser);
            void Event_AWAY(libircclient::Parser *parser);
            void Event_RplAway(libircclient::Parser *parser);
            void Event_SelfCHGHOST(libircclient::Parser *parser, QString old_host, QString old_ident, QString new_host, QString new_ident);
            void Event_CHGHOST(libircclient::Parser *parser, QString old_host, QString old_ident, QString new_host, QString new_ident);

            // IRCv3
            void Event_CAP(libircclient::Parser *parser);
            void Event_CAP_ACK(libircclient::Parser *parser);
            void Event_CAP_NAK(libircclient::Parser *parser);
            void Event_CAP_Timeout();
            void Event_CAP_RequestedCapNotSupported(QString name);

        protected slots:
            virtual void OnSslHandshakeFailure(QList<QSslError> errors);
            virtual void OnError(QAbstractSocket::SocketError er);
            virtual void OnReceive();
            virtual void OnDisconnect();
            virtual void OnConnected();
            virtual void OnSend();
            virtual void OnPing();
            virtual void OnPingSend();
            virtual void OnCapSupportTimeout();

        protected:
            virtual void OnReceive(const QByteArray &data);
            virtual void closeError(const QString &error, int code);
            bool usingSSL;
            QTcpSocket *socket;
            //! These capabilities will be automatically requested from a server if it supports them
            QList<QString> _capabilitiesRequested;
            QList<QString> _capabilitiesSupported;
            QList<QString> _capabilitiesSubscribed;
            bool _loggedIn;
            int _capGraceTime;
            bool _enableCap;
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
            void updateSelfAway(Parser *parser, bool status, const QString &text);
            void processIncomingRawData(QByteArray data);
            void processNamrpl(Parser *parser);
            void process433(Parser *parser);
            void processInfo(Parser *parser);
            void processWho(Parser *parser);
            void processPrivMsg(Parser *parser);
            void processMode(Parser *parser);
            void processMdIn(Parser *parser);
            void processTopic(Parser *parser);
            void processKick(Parser *parser);
            void processTopicWhoTime(Parser *parser);
            void processPMode(Parser *parser, char mode);
            void processMTime(Parser *parser);
            void processJoin(Parser *parser, bool self_command);
            void processNick(Parser *parser, bool self_command);
            void processAway(Parser *parser, bool self_command);
            void processCap(Parser *parser);
            void processWhoisUser(Parser &parser);
            void processWhoisIdle(Parser &parser);
            void processChangeHost(Parser &parser);
            void standardLogin();
            void deleteTimers();
            void initialize();
            void freemm();
            void resetCap();
            void processAutoCap();
            void pseudoSleep(unsigned int msec);
            QByteArray getDataToSend();
            void scheduleDelivery(const QByteArray &data, libircclient::Priority priority);
            void autoJoin();

            /////////////////////////////////////
            // This probably doesn't need syncing
            unsigned int MSDelayOnEmpty;
            unsigned int MSDelayOnOpen;
            unsigned int MSWait;
            QTimer capTimeout;
            bool capProcessingMultilineLS;
            bool capProcessingChangeRequest;
            bool capAutoRequestFinished;
            bool loggedIn;
            bool scheduling;
            QDateTime senderTime;
            QTimer senderTimer;
            QMutex mutex;
            unsigned long long bytesSent;
            unsigned long long bytesRcvd;
            QList<QByteArray> hprFIFO;
            QList<QByteArray> mprFIFO;
            QList<QByteArray> lprFIFO;
            /////////////////////////////////////

            //! List of symbols that are used to prefix users
            QList<char> channelUserPrefixes;
            //! Channel modes with no parameters
            QList<char> CModes;
            //! Channel parameter modes (+b, +I)
            QList<char> CPModes;
            //! Channel secret modes (+k)
            QList<char> CRModes;
            //! Channel user modes (+o, +v)
            QList<char> CUModes;
            //! Channel numeric modes (+l)
            QList<char> CCModes;
            //! https://tools.ietf.org/html/draft-hardy-irc-isupport-00#section-4.18
            QList<char> STATUSMSG_Modes;
            QString originalNick;
            UMode localUserMode;
            QString alternateNick;
            int alternateNickNumber;
            QString awayMessage;
            char channelPrefix;
            Server *server;
            QList<User*> users;
            Encoding encoding = EncodingDefault;
            QList<Channel*> channels;
            User localUser;
            QDateTime lastPing;
            QTimer *timerPingTimeout;
            QTimer *timerPingSend;
            //! List of channels to join after connection to network
            QList<QString> channelsToJoin;
            long long lastPingResponseTimeInMs = 0;
    };
}

#endif // NETWORK_H
