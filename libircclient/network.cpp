//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QtNetwork>
#include <QAbstractSocket>
#include <QDebug>
#include "network.h"
#include "server.h"
#include "channel.h"
#include "parser.h"
#include "generic.h"
#include "../libirc/serveraddress.h"
#include "../libirc/error_code.h"

using namespace libircclient;

Network::Network(libirc::ServerAddress &server, QString name) : libirc::Network(name)
{
    this->initialize();
    this->hostname = server.GetHost();
    if (server.GetNick().isEmpty())
        this->localUser.SetNick("GrumpyUser");
    else
        this->localUser.SetNick(server.GetNick());
    this->usingSSL = server.UsingSSL();
    this->port = server.GetPort();
}

Network::Network(QHash<QString, QVariant> hash) : libirc::Network("")
{
    this->initialize();
    this->LoadHash(hash);
}

Network::~Network()
{
    delete this->server;
    this->deleteTimers();
    delete this->socket;
    this->freemm();
}

void Network::Connect()
{
    if (this->IsConnected())
        return;
    this->deleteTimers();
    this->scheduling = true;
    //delete this->network_thread;
    delete this->socket;

    //this->network_thread = new NetworkThread(this);
    if (!this->IsSSL())
        this->socket = new QTcpSocket();
    else
        this->socket = new QSslSocket();

    connect(this->socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnError(QAbstractSocket::SocketError)));
    connect(this->socket, SIGNAL(readyRead()), this, SLOT(OnReceive()));
    connect(this->socket, SIGNAL(disconnected()), this, SLOT(OnDisconnect()));

    if (!this->IsSSL())
    {
        connect(this->socket, SIGNAL(connected()), this, SLOT(OnConnected()));
        this->socket->connectToHost(this->hostname, this->port);
    } else
    {
        ((QSslSocket*)this->socket)->ignoreSslErrors();
        connect(((QSslSocket*)this->socket), SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(OnSslHandshakeFailure(QList<QSslError>)));
        connect(((QSslSocket*)this->socket), SIGNAL(encrypted()), this, SLOT(OnConnected()));
        ((QSslSocket*)this->socket)->connectToHostEncrypted(this->hostname, this->port);
        /*if (this->socket && !((QSslSocket*)this->socket)->waitForEncrypted())
        {
            this->closeError("SSL handshake failed: " + this->socket->errorString(), EHANDSHAKE);
        }*/
    }
    this->senderTimer.start(this->MSDelayOnEmpty);
}

void Network::Reconnect()
{
    this->Disconnect();
    this->Connect();
}

void Network::Disconnect(QString reason)
{
    if (reason.isEmpty())
        reason = this->defaultQuit;
    if (this->IsConnected())
    {
        this->scheduling = false;
        this->TransferRaw("QUIT :" + reason);
        if (this->socket)
            this->socket->close();
    }
    if (this->socket)
    {
        this->socket->deleteLater();
        this->socket = NULL;
    }
    this->deleteTimers();
    this->freemm();
}

bool libircclient::Network::IsAway() const
{
    return this->localUser.IsAway;
}

bool Network::IsConnected()
{
    if (!this->socket)
        return false;

    return this->socket->isOpen();
}

void libircclient::Network::SetAway(bool away, QString message)
{
    this->awayMessage = message;
    if (!away)
    {
        this->TransferRaw("AWAY");
    }
    else
    {
        this->TransferRaw("AWAY :" + message);
    }
}

void Network::SetDefaultNick(QString nick)
{
    if (!this->IsConnected())
        this->localUser.SetNick(nick);
}

void Network::SetDefaultIdent(QString ident)
{
    if (!this->IsConnected())
        this->localUser.SetIdent(ident);
}

void Network::SetDefaultUsername(QString realname)
{
    if (!this->IsConnected())
        this->localUser.SetRealname(realname);
}

bool Network::IsSSL()
{
    return this->usingSSL;
}

QString Network::GetNick()
{
    return this->localUser.GetNick();
}

QString Network::GetHost()
{
    return this->localUser.GetHost();
}

QString Network::GetServerVersion()
{
    return this->server->GetVersion();
}

unsigned int Network::GetPort()
{
    return this->port;
}

QString Network::GetIdent()
{
    return this->localUser.GetIdent();
}

void Network::TransferRaw(QString raw, libircclient::Priority priority)
{
    if (!this->IsConnected())
        return;

    // Remove garbage for security reasons
    raw.replace("\r", "").replace("\n", "");

    QByteArray data = QString(raw + "\n").toUtf8();
    emit this->Event_RawOutgoing(data);
    if (this->scheduling)
    {
        this->scheduleDelivery(data, priority);
    }
    else
    {
        this->bytesSent += data.size();
        this->socket->write(data);
        this->socket->flush();
    }
}

#define SEPARATOR QString((char)1)

int Network::SendMessage(QString text, QString target, Priority priority)
{
    this->TransferRaw("PRIVMSG " + target + " :" + text, priority);
    return SUCCESS;
}

int Network::SendAction(QString text, Channel *channel, Priority priority)
{
    return this->SendAction(text, channel->GetName(), priority);
}

int Network::SendAction(QString text, QString target, Priority priority)
{
    this->TransferRaw(QString("PRIVMSG ") + target + " :" + SEPARATOR + "ACTION " + text + SEPARATOR, priority);
    return SUCCESS;
}

int Network::SendNotice(QString text, User *user, Priority priority)
{
    return this->SendNotice(text, user->GetNick(), priority);
}

int Network::SendNotice(QString text, Channel *channel, Priority priority)
{
    return this->SendNotice(text, channel->GetName(), priority);
}

int Network::SendNotice(QString text, QString target, Priority priority)
{
    this->TransferRaw(QString("NOTICE ") + target + " :" + text, priority);
    return SUCCESS;
}

int Network::SendMessage(QString text, Channel *channel, Priority priority)
{
    return this->SendMessage(text, channel->GetName(), priority);
}

int Network::SendMessage(QString text, User *user, Priority priority)
{
    return this->SendMessage(text, user->GetNick(), priority);
}

void Network::RequestPart(QString channel_name, Priority priority)
{
    this->TransferRaw("PART " + channel_name, priority);
}

void Network::RequestPart(Channel *channel, Priority priority)
{
    this->TransferRaw("PART " + channel->GetName(), priority);
}

void Network::RequestNick(QString nick, Priority priority)
{
    this->TransferRaw("NICK " + nick, priority);
}

void Network::Identify(QString Nickname, QString Password, Priority priority)
{
    if (Nickname.isEmpty())
        Nickname = this->localUser.GetNick();
    if (Password.isEmpty())
        Password = this->password;
    QString ident_line = this->identifyString;
    ident_line.replace("$nickname", Nickname).replace("$password", Password);
    this->TransferRaw(ident_line, priority);
}

bool libircclient::Network::SupportsIRCv3() const
{
    return this->_enableCap;
}

void Network::EnableIRCv3Support()
{
    this->_enableCap = true;
}

void Network::DisableIRCv3Support()
{
    this->_enableCap = false;
}

QList<QString> Network::GetSupportedCaps()
{
    return this->_capabilitiesSupported;
}

QList<QString> Network::GetSubscribedCaps()
{
    return this->_capabilitiesSubscribed;
}

QString Network::GetServerAddress()
{
    return this->hostname;
}

QString Network::GetHelpForMode(char mode, QString missing)
{
    if (this->ChannelModeHelp.contains(mode))
        return this->ChannelModeHelp[mode];
    return missing;
}

void Network::_st_SetNick(QString nick)
{
    this->localUser.SetNick(nick);
}

int Network::GetTimeout() const
{
    return this->pingTimeout;
}

int Network::SendCtcp(QString name, QString text, QString target, Priority priority)
{
    if (!text.isEmpty())
        this->TransferRaw(QString("PRIVMSG ") + target + " :" + SEPARATOR + name + " " + text + SEPARATOR, priority);
    else
        this->TransferRaw(QString("PRIVMSG ") + target + " :" + SEPARATOR + name + SEPARATOR, priority);
    return SUCCESS;
}

void Network::OnPing()
{
    // Check ping timeout
    if (QDateTime::currentDateTime() > this->lastPing.addSecs(this->pingTimeout))
    {
        emit this->Event_Timeout();
        this->closeError("Timeout", ETIMEDOUT);
    }
}

void Network::SetPassword(QString Password)
{
    this->password = Password;
}

void Network::RequestJoin(QString name, Priority priority)
{
    this->TransferRaw("JOIN " + name, priority);
}

void Network::OnPingSend()
{
    this->TransferRaw("PING :" + this->GetServerAddress(), libircclient::Priority_High);
}

void Network::OnCapSupportTimeout()
{
    // Prevent this from running multiple times
    this->capTimeout.stop();
    emit this->Event_CAP_Timeout();
    this->DisableIRCv3Support();
    if (this->IsConnected())
        this->standardLogin();
}

void Network::OnReceive(QByteArray data)
{
    if (data.length() == 0)
        return;

    emit this->Event_RawIncoming(data);

    this->processIncomingRawData(data);
}

void Network::closeError(QString error, int code)
{
    // Delete the socket first to prevent neverending loop
    // for some reason when you call the destructor Qt emits
    // some errors again causing program to hang
    if (this->socket == NULL)
        return;
    QTcpSocket *temp = this->socket;
    this->socket = NULL;
    temp->close();
    temp->deleteLater();
    this->deleteTimers();
    emit this->Event_NetworkFailure(error, code);
    emit this->Event_Disconnected();
}

void Network::updateSelfAway(Parser *parser, bool status, QString text)
{
    // we need to scan all channels and update the status
    foreach (Channel *channel, this->channels)
    {
        if (channel->ContainsUser(this->GetNick()))
        {
            User *user = channel->GetUser(this->GetNick());
            if (user->IsAway != status)
            {
                user->IsAway = status;
                user->AwayMs = text;
                emit this->Event_UserAwayStatusChange(parser, channel, user);
            }
        }
    }
}

void Network::OnError(QAbstractSocket::SocketError er)
{
    if (this->socket == NULL)
        return;
    emit this->Event_ConnectionFailure(er);
    this->closeError(Generic::ErrorCode2String(er), 1);
}

void Network::OnReceive()
{
    if (!this->IsConnected())
        return;
    while (this->socket->canReadLine())
    {
        QByteArray line = this->socket->readLine();
        if (line.length() == 0)
            return;

        emit this->Event_RawIncoming(line);

        this->processIncomingRawData(line);
    }
}

void Network::OnDisconnect()
{
    this->closeError("Disconnected", EDISCONNECTED);
}

User *Network::GetLocalUserInfo()
{
    return &this->localUser;
}

char Network::StartsWithCUPrefix(QString user_name)
{
    if (user_name.isEmpty())
        return 0;
    char first_symbol = user_name[0].toLatin1();
    if (this->channelUserPrefixes.contains(first_symbol))
        return this->CUModes[this->channelUserPrefixes.indexOf(first_symbol)];
    return 0;
}

int Network::PositionOfUCPrefix(char prefix)
{
    // Check if there is such a prefix
    if (!this->channelUserPrefixes.contains(prefix))
        return -1;

    return this->channelUserPrefixes.indexOf(prefix);
}

void Network::SetChannelUserPrefixes(QList<char> data)
{
    this->channelUserPrefixes = data;
}

void Network::SetCModes(QList<char> data)
{
    this->CModes = data;
}

QList<char> Network::GetChannelUserPrefixes()
{
    return this->channelUserPrefixes;
}

bool Network::HasCap(QString cap)
{
    return this->_capabilitiesSupported.contains(cap);
}

QList<char> Network::GetCModes()
{
    return this->CModes;
}

QList<char> Network::GetCPModes()
{
    return this->CPModes;
}

void Network::SetCPModes(QList<char> data)
{
    this->CPModes = data;
}

void Network::SetCRModes(QList<char> data)
{
    this->CRModes = data;
}

QList<char> Network::GetCRModes()
{
    return this->CRModes;
}

void Network::SetCUModes(QList<char> data)
{
    this->CUModes = data;
}

void Network::SetCCModes(QList<char> data)
{
    this->CCModes = data;
}

UMode Network::GetLocalUserMode()
{
    return this->localUserMode;
}

//! This function performs a sort of a list of random chars using a mask list, that contains these chars that are sorted
//! it removes duplicates
static QList<char> SortingHelper(QList<char> mask, QList<char> list)
{
    // Hash sort, memory expensive but pretty fast
    QHash<int, char> hash;
    foreach (char mode, list)
    {
        int index = mask.indexOf(mode);
        if (hash.contains(index))
            continue;
        hash.insert(index, mode);
    }
    // Now that we have all modes in a hash table, we can just easily sort them
    QList<int> unsorted_ints = hash.keys();
    qSort(unsorted_ints);
    QList<char> sorted_list;
    foreach (int mode, unsorted_ints)
        sorted_list.append(mask[mode]);
    return sorted_list;
}

QList<char> Network::ModeHelper_GetSortedChannelPrefixes(QList<char> unsorted_list)
{
    return SortingHelper(this->channelUserPrefixes, unsorted_list);
}

QList<char> Network::ModeHelper_GetSortedCUModes(QList<char> unsorted_list)
{
    return SortingHelper(this->CUModes, unsorted_list);
}

QList<char> Network::ParameterModes()
{
    QList<char> results;
    results.append(this->CUModes);
    results.append(this->CRModes);
    results.append(this->CPModes);
    return results;
}

QList<char> Network::GetCCModes()
{
    return this->CCModes;
}

static QVariant serializeList(QList<char> data)
{
    QList<QVariant> result;
    foreach (char x, data)
        result.append(QVariant(QChar(x)));
    return QVariant(result);
}

void Network::LoadHash(QHash<QString, QVariant> hash)
{
    libirc::Network::LoadHash(hash);
    UNSERIALIZE_STRING(awayMessage);
    UNSERIALIZE_STRING(hostname);
    UNSERIALIZE_UINT(port);
    UNSERIALIZE_INT(pingTimeout);
    UNSERIALIZE_INT(pingRate);
    UNSERIALIZE_STRING(defaultQuit);
    UNSERIALIZE_BOOL(autoRejoin);
    UNSERIALIZE_BOOL(autoIdentify);
    UNSERIALIZE_STRING(identifyString);
    UNSERIALIZE_CHARLIST(CModes);
    UNSERIALIZE_CHARLIST(CCModes);
    UNSERIALIZE_CHARLIST(CUModes);
    UNSERIALIZE_CHARLIST(CPModes);
    UNSERIALIZE_CHARLIST(CRModes);
    UNSERIALIZE_BOOL(_enableCap);
    UNSERIALIZE_CHARLIST(channelUserPrefixes);
    UNSERIALIZE_STRING(password);
    UNSERIALIZE_STRING(alternateNick);
    if (hash.contains("server"))
        this->server = new Server(hash["server"].toHash());
    if (hash.contains("users"))
    {
        foreach (QVariant user, hash["user"].toList())
            this->users.append(new User(user.toHash()));
    }
    if (hash.contains("channels"))
    {
        foreach (QVariant channel, hash["channels"].toList())
            this->channels.append(new Channel(channel.toHash()));
    }
    if (hash.contains("localUserMode"))
        this->localUserMode = UMode(hash["localUserMode"].toHash());
    if (hash.contains("localUser"))
        this->localUser = User(hash["localUser"].toHash());
    UNSERIALIZE_STRINGLIST(_capabilitiesSubscribed);
    UNSERIALIZE_STRINGLIST(_capabilitiesRequested);
    UNSERIALIZE_STRINGLIST(_capabilitiesSupported);
}

QHash<QString, QVariant> Network::ToHash()
{
    QHash<QString, QVariant> hash = libirc::Network::ToHash();
    SERIALIZE(awayMessage);
    SERIALIZE(hostname);
    SERIALIZE(port);
    SERIALIZE(pingTimeout);
    SERIALIZE(pingRate);
    SERIALIZE(defaultQuit);
    SERIALIZE(_enableCap);
    SERIALIZE(autoRejoin);
    SERIALIZE(autoIdentify);
    SERIALIZE(identifyString);
    SERIALIZE(password);
    SERIALIZE(alternateNick);
    SERIALIZE(_capabilitiesSupported);
    SERIALIZE(_capabilitiesSubscribed);
    SERIALIZE(_capabilitiesRequested);
    hash.insert("CCModes", serializeList(this->CCModes));
    hash.insert("CModes", serializeList(this->CModes));
    hash.insert("CPModes", serializeList(this->CPModes));
    hash.insert("CUModes", serializeList(this->CUModes));
    hash.insert("channelUserPrefixes", serializeList(this->channelUserPrefixes));
    hash.insert("CRModes", serializeList(this->CRModes));
    hash.insert("localUserMode", this->localUserMode.ToHash());
    SERIALIZE(channelPrefix);
    hash.insert("server", this->server->ToHash());
    hash.insert("localUser", this->localUser.ToHash());
    SERIALIZE(lastPing);
    QList<QVariant> users_x, channels_x;
    foreach (Channel *ch, this->channels)
        channels_x.append(QVariant(ch->ToHash()));
    hash.insert("channels", QVariant(channels_x));
    foreach (User *user, this->users)
        users_x.append(QVariant(user->ToHash()));
    hash.insert("users", QVariant(users_x));
    return hash;
}

void Network::OnSslHandshakeFailure(QList<QSslError> errors)
{
    bool temp = false;
    emit this->Event_SSLFailure(errors, &temp);
    if (!temp)
        ((QSslSocket*)this->socket)->ignoreSslErrors();
    else
        this->closeError("Requested by hook", EHANDSHAKE);
}

Channel *Network::GetChannel(QString channel_name)
{
    channel_name = channel_name.toLower();
    foreach(Channel *channel, this->channels)
    {
        if (channel->GetName().toLower() == channel_name)
        {
            return channel;
        }
    }
    return NULL;
}

QList<Channel *> Network::GetChannels()
{
    return this->channels;
}

QList<char> Network::GetCUModes()
{
    return this->CUModes;
}

bool Network::ContainsChannel(QString channel_name)
{
    return this->GetChannel(channel_name) != NULL;
}

void Network::_st_ClearChannels()
{
    qDeleteAll(this->channels);
    this->channels.clear();
}

Channel *Network::_st_InsertChannel(Channel *channel)
{
    Channel *cx = new Channel(channel);
    cx->SetNetwork(this);
    this->channels.append(cx);
    return cx;
}

void Network::OnConnected()
{
    // We just connected to an IRC network
    if (this->_enableCap)
    {
        // IRCv3 protocol is enabled, let's verify if ircd supports it
        this->resetCap();
        this->capTimeout.start(this->_capGraceTime * 1000);
        // There is one issue with this command though. For whatever reasons IRCv3 people believe that client should
        // send CAP END to finish negotiation, and unless client does that, it isn't allowed to login to network.

        // That would be fine if CAP was actually any IRC standard, which it isn't, so any ircd server may not support it.
        // Standards allow ircd's to ignore unknown commands, which means that in extreme case, we send CAP to ircd
        // which never respond and we are waiting forever. In order to prevent this capTimeout is implemented with its
        // own timer that disable IRCv3 support on server in case that it fails to respons within given grace time.
        this->TransferRaw("CAP LS");
    } else
    {
        this->standardLogin();
    }
}

static void DebugInvalid(QString message, Parser *parser)
{
    qDebug() << "IRC PARSER: " + message + ": " + parser->GetRaw();
}

void Network::processIncomingRawData(QByteArray data)
{
    this->lastPing = QDateTime::currentDateTime();
    QString l(data);
    // let's try to parse this IRC command
    Parser parser(l);
    if (!parser.IsValid())
    {
        emit this->Event_Invalid(data);
        return;
    }
    bool self_command = false;
    if (parser.GetSourceUserInfo() != NULL)
        self_command = parser.GetSourceUserInfo()->GetNick().toLower() == this->GetNick().toLower();
    // This is a fixup for our own hostname as seen by the server, it may actually change runtime
    // based on cloak mechanisms used by a server, so when it happens we need to update it
    if (self_command && !parser.GetSourceUserInfo()->GetHost().isEmpty() && parser.GetSourceUserInfo()->GetHost() != this->localUser.GetHost())
        this->localUser.SetHost(parser.GetSourceUserInfo()->GetHost());
    bool known = true;
    switch (parser.GetNumeric())
    {
        case IRC_NUMERIC_RAW_PING:
            if (parser.GetParameters().count() == 0)
                this->TransferRaw("PONG", Priority_RealTime);
            else
                this->TransferRaw("PONG :" + parser.GetParameters()[0], Priority_RealTime);
            break;
        case IRC_NUMERIC_MYINFO:
            // Process the information about network
            if (parser.GetParameters().count() < 4)
                break;
            this->server->SetName(parser.GetParameters()[1]);
            this->server->SetVersion(parser.GetParameters()[2]);
            emit this->Event_MyInfo(&parser);
            break;

        case IRC_NUMERIC_RAW_JOIN:
            this->processJoin(&parser, self_command);
            break;

        case IRC_NUMERIC_RAW_PRIVMSG:
            this->processPrivMsg(&parser);
            break;

        case IRC_NUMERIC_RAW_NOTICE:
            emit this->Event_NOTICE(&parser);
            break;

        case IRC_NUMERIC_RAW_NICK:
            this->processNick(&parser, self_command);
            break;

        case IRC_NUMERIC_RAW_QUIT:
        {
            // Remove the user from all channels
            foreach (Channel *channel, this->channels)
            {
                if (channel->ContainsUser(parser.GetSourceUserInfo()->GetNick()))
                {
                    channel->RemoveUser(parser.GetSourceUserInfo()->GetNick());
                    emit this->Event_PerChannelQuit(&parser, channel);
                }
            }
            emit this->Event_Quit(&parser);
        }   break;
        case IRC_NUMERIC_RAW_PART:
        {
            if (parser.GetParameters().count() < 1)
            {
                qDebug() << "IRC PARSER: Invalid PART: " + parser.GetRaw();
                break;
            }
            Channel *channel = this->GetChannel(parser.GetParameters()[0]);
            if (self_command)
            {
                if (!channel)
                {
                    DebugInvalid("Channel struct not in memory", &parser);
                }
                else
                {
                    emit this->Event_SelfPart(&parser, channel);
                    this->channels.removeOne(channel);
                    emit this->Event_Part(&parser, channel);
                    delete channel;
                    break;
                }
            }
            else if (channel)
            {
                channel->RemoveUser(parser.GetSourceUserInfo()->GetNick());
            }
            emit this->Event_Part(&parser, channel);
        }   break;
        case IRC_NUMERIC_RAW_KICK:
            this->processKick(&parser);
            break;
        case IRC_NUMERIC_RAW_PONG:
            break;
        case IRC_NUMERIC_RAW_MODE:
            this->processMode(&parser);
            break;
        case IRC_NUMERIC_ISUPPORT:
            this->processInfo(&parser);
            break;
        case IRC_NUMERIC_NAMREPLY:
            this->processNamrpl(&parser);
            break;
        case IRC_NUMERIC_ENDOFNAMES:
            emit this->Event_EndOfNames(&parser);
            break;
        case IRC_NUMERIC_RAW_TOPIC:
            this->processTopic(&parser);
            break;
        case IRC_NUMERIC_TOPICINFO:
        {
            if (parser.GetParameters().count() < 2)
            {
                qDebug() << "IRC PARSER: Invalid TOPICINFO: " + parser.GetRaw();
                break;
            }
            Channel *channel = this->GetChannel(parser.GetParameters()[1]);
            if (!channel)
            {
                DebugInvalid("Channel struct not in memory", &parser);
                break;
            }
            channel->SetTopic(parser.GetText());
            emit this->Event_TOPICInfo(&parser, channel);
        }
            break;
        case IRC_NUMERIC_TOPICWHOTIME:
            this->processTopicWhoTime(&parser);
            break;
        case IRC_NUMERIC_NICKUSED:
            this->process433(&parser);
            break;
        case IRC_NUMERIC_MOTD:
            emit this->Event_MOTD(&parser);
            break;
        case IRC_NUMERIC_MOTDBEGIN:
            emit this->Event_MOTDBegin(&parser);
            break;
        case IRC_NUMERIC_MOTDEND:
            emit this->Event_MOTDEnd(&parser);
            break;
        case IRC_NUMERIC_WHOREPLY:
            this->processWho(&parser);
            break;
        case IRC_NUMERIC_WHOEND:
            // 315
            emit this->Event_EndOfWHO(&parser);
            break;
        case IRC_NUMERIC_MODEINFO:
            this->processMdIn(&parser);
            break;
        case IRC_NUMERIC_CREATIONTIME:
            this->processMTime(&parser);
            break;
        case IRC_NUMERIC_WELCOME:
            emit this->Event_Welcome(&parser);
            this->loggedIn = true;
            break;
        case IRC_NUMERIC_EXCEPTION:
            this->processPMode(&parser, 'e');
            break;
        case IRC_NUMERIC_BAN:
            this->processPMode(&parser, 'b');
            break;
        case IRC_NUMERIC_ENDOFBANS:
            emit this->Event_EndOfBans(&parser);
            break;
        case IRC_NUMERIC_UNAWAY:
            this->localUser.IsAway = false;
            // Update the status of our own user in every channel
            this->updateSelfAway(&parser, false, "");
            emit this->Event_UnAway(&parser);
            break;
        case IRC_NUMERIC_NOWAWAY:
            this->localUser.IsAway = true;
            this->updateSelfAway(&parser, true, this->awayMessage);
            emit this->Event_NowAway(&parser);
            break;
        case IRC_NUMERIC_RAW_CAP:
            this->processCap(&parser);
            break;
        case IRC_NUMERIC_RAW_AWAY:
            this->processAway(&parser, self_command);
            break;
        case IRC_NUMERIC_ERR_INVALIDCAPCMD:
            // If we are negotiating cap handshake right now, the ircd is clearly broken
            // let's disable it
            if (this->capTimeout.isActive())
            {
                this->capTimeout.stop();
                this->DisableIRCv3Support();
                // This may not work but we really should finish negotiation right here
                this->TransferRaw("CAP END");
                this->standardLogin();
            }
            break;
        case IRC_NUMERIC_UNKNOWN:
            // If we are negotiating cap handshake, server doesn't support it
            if (this->capTimeout.isActive())
            {
                this->capTimeout.stop();
                this->DisableIRCv3Support();
                this->standardLogin();
            }
            emit this->Event_NUMERIC_UNKNOWN(&parser);
            break;
        case IRC_NUMERIC_NICKISNOTAVAILABLE:
            this->process433(&parser);
            break;
        case IRC_NUMERIC_RAW_INVITE:
            emit this->Event_INVITE(&parser);
            break;
        default:
            known = false;
            break;
    }
    if (!known)
        emit this->Event_Unknown(&parser);
    emit this->Event_Parse(&parser);
}

void Network::processNamrpl(Parser *parser)
{
    // 353 GrumpyUser = #support :GrumpyUser petan|home @+petan %wm-bot &OperBot
    // Server sent us an initial list of users that are in the channel
    if (parser->GetParameters().size() < 3)
    {
        DebugInvalid("Malformed NAMRPL", parser);
        return;
    }

    Channel *channel = this->GetChannel(parser->GetParameters()[2]);
    if (channel == NULL)
    {
        DebugInvalid("Unknown channel", parser);
        return;
    }

    // Now insert new users one by one

    foreach (QString user, parser->GetText().split(" "))
    {
        if (user.isEmpty())
            continue;
        char cumode = this->StartsWithCUPrefix(user);
        QList<char> cumodes;
        QList<char> prefixes;
        while (cumode != 0)
        {
            char prefix = user[0].toLatin1();
            cumodes << cumode;
            prefixes << prefix;
            user = user.mid(1);
            cumode = this->StartsWithCUPrefix(user);
        }
        User ux;
        ux.SetNick(user);
        ux.CUModes = cumodes;
        ux.ChannelPrefixes = prefixes;
        channel->InsertUser(&ux);
    }
}

static QList<char> CLFromStr(QString string)
{
    QList<char> lt;
    foreach (QChar ch, string)
    {
        lt.append(ch.toLatin1());
    }
    return lt;
}

void Network::processInfo(Parser *parser)
{
    // WATCHOPTS=A SILENCE=15 MODES=12 CHANTYPES=# PREFIX=(qaohv)~&@%+ CHANMODES=beI,kfL,lj,psmntirRcOAQKVCuzNSMTGZ NETWORK=tm-irc CASEMAPPING=ascii EXTBAN=~,qjncrRa ELIST=MNUCT STATUSMSG=~&@%+
    foreach (QString info, parser->GetParameters())
    {
        if (info.startsWith("PREFIX"))
        {
            QString cu, prefix;
            if (info.length() < 8 || !info.contains("("))
                goto broken_prefix;
            prefix = info.mid(8);
            if (prefix.contains("(") || !prefix.contains(")"))
                goto broken_prefix;
            cu = prefix.mid(0, prefix.indexOf(")"));
            prefix = prefix.mid(prefix.indexOf(")") + 1);
            if (cu.length() != prefix.length())
                goto broken_prefix;
            this->channelUserPrefixes.clear();
            this->CUModes.clear();
            foreach (QChar CUMode, cu)
                this->CUModes.append(CUMode.toLatin1());
            foreach (QChar pr, prefix)
                this->channelUserPrefixes.append(pr.toLatin1());

            continue;
            broken_prefix:
                qDebug() << "IRC PARSER: broken prefix: " + parser->GetRaw();
                break;
        }
        else if (info.startsWith("NETWORK="))
        {
            this->networkName = info.mid(8);
            continue;
        } else if (info.startsWith("CHANMODES="))
        {
            QString input = info.mid(10);
            QList<QString> groups = input.split(',');
            if (groups.count() > 0)
                this->CPModes = CLFromStr(groups[0]);
            if (groups.count() > 1)
                this->CRModes = CLFromStr(groups[1]);
            if (groups.count() > 2)
                this->CCModes = CLFromStr(groups[2]);
            if (groups.count() > 3)
                this->CModes = CLFromStr(groups[3]);
        }
    }
    emit this->Event_ISUPPORT(parser);
}

void Network::processWho(Parser *parser)
{
    // GrumpyUser1 #support grumpy hidden-715465F6.net.upcbroadband.cz hub.tm-irc.org GrumpyUser1 H :0 GrumpyUser                    |
    //             <channel> <user> <host>                             <server>       <nick>     <H|G>[*][@|+] :<hopcount> <real name>
    QStringList parameters = parser->GetParameters();
    bool is_away;
    Channel *channel = NULL;
    QString gecos;
    User *user = NULL;
    if (parameters.count() < 7)
        goto finish;
    if (!parameters[1].startsWith(this->channelPrefix))
        goto finish;

    // Find a channel related to this message and update the user details
    channel = this->GetChannel(parameters[1]);
    if (!channel)
        goto finish;
    user = channel->GetUser(parameters[5]);
    if (!user)
        goto finish;
    gecos = parser->GetText();
    if (gecos.contains(" "))
    {
        user->Hops = gecos.mid(0, gecos.indexOf(" ")).toInt();
        gecos = gecos.mid(gecos.indexOf(" ") + 1);
    }
    user->SetRealname(gecos);
    user->SetIdent(parameters[2]);
    is_away = parameters[6].contains("G");
    user->SetHost(parameters[3]);
    if (user->IsAway != is_away)
    {
        user->IsAway = is_away;
        emit this->Event_UserAwayStatusChange(parser, channel, user);
    }
    user->ServerName = parameters[4];

    finish:
        emit this->Event_WHO(parser, channel, user);
}

void Network::processPrivMsg(Parser *parser)
{
    if (parser->GetParameters().count() < 1)
    {
        qDebug() << "IRC PARSER: Malformed PRIVMSG: " + parser->GetRaw();
        return;
    }

    QString text = parser->GetText();
    if (text.startsWith(SEPARATOR))
    {
        // This is a CTCP message
        text = text.mid(1);
        if (text.endsWith(SEPARATOR))
            text = text.mid(0, text.size() - 1);

        // These are usually split by space
        QString command = text;
        QString parameters;
        if (text.contains(" "))
        {
            parameters = command.mid(command.indexOf(" ") + 1);
            command = command.mid(0, command.indexOf(" "));
        }
        emit this->Event_CTCP(parser, command, parameters);
    } else
    {
        emit this->Event_PRIVMSG(parser);
    }
}

void Network::processMdIn(Parser *parser)
{
    QStringList pl = parser->GetParameters();
    if (pl.size() < 3)
    {
        qDebug() << "IRC PARSER: Invalid MODEINFO: " + parser->GetRaw();
        return;
    }
    Channel *channel = this->GetChannel(pl[1]);
    if (!channel)
    {
        DebugInvalid("Channel struct not in memory", parser);
        return;
    }
    channel->SetMode(pl[2]);
    emit this->Event_ModeInfo(parser, channel);
}

void Network::processTopic(Parser *parser)
{
    if (parser->GetParameters().count() < 1)
    {
        qDebug() << "IRC PARSER: Invalid TOPIC: " + parser->GetRaw();
        return;
    }
    Channel *channel = this->GetChannel(parser->GetParameters()[0]);
    if (!channel)
    {
        DebugInvalid("Channel struct not in memory", parser);
        return;
    }
    QString topic = channel->GetTopic();
    channel->SetTopic(parser->GetText());
    emit this->Event_TOPIC(parser, channel, topic);
}

void Network::processKick(Parser *parser)
{
    if (parser->GetParameters().count() < 2)
    {
        qDebug() << "IRC PARSER: Invalid KICK: " + parser->GetRaw();
        return;
    }
    bool self_command = parser->GetParameters()[1].toLower() == this->GetNick().toLower();
    Channel *channel = this->GetChannel(parser->GetParameters()[0]);
    if (self_command)
    {
        if (!channel)
        {
            DebugInvalid("Channel struct not in memory", parser);
        }
        else
        {
            emit this->Event_SelfKick(parser, channel);
            this->channels.removeOne(channel);
            emit this->Event_Kick(parser, channel);
            delete channel;
            return;
        }
    }
    else if (channel)
    {
        channel->RemoveUser(parser->GetParameters()[1]);
    }
    emit this->Event_Kick(parser, channel);
}

void Network::processTopicWhoTime(Parser *parser)
{
    if (parser->GetParameters().count() < 2)
    {
        qDebug() << "IRC PARSER: Invalid TOPICWHOTIME: " + parser->GetRaw();
        return;
    }
    QStringList parameters = parser->GetParameters();
    Channel *channel = this->GetChannel(parameters[1]);
    if (!channel)
    {
        DebugInvalid("Channel struct not in memory", parser);
        return;
    }
    channel->SetTopicUser(parameters[2]);
    channel->SetTopicTime(QDateTime::fromTime_t(parameters[3].toUInt()));
    emit this->Event_TOPICWhoTime(parser, channel);
}

void Network::processPMode(Parser *parser, char mode)
{
    // :hub.tm-irc.org 367 petan|work1 #test test!*@* petan|work1 1448444377
    //                                Channel  ban        SetBy      Time

    QList<QString> parameters = parser->GetParameters();
    if (parameters.count() < 5)
    {
        emit this->Event_Broken(parser, "Invalid PMODE");
        return;
    }
    libircclient::Channel *channel = this->GetChannel(parameters[1]);
    if (!channel)
    {
        emit this->Event_Broken(parser, "Unknown channel");
        return;
    }
    QString string = QString(QChar(mode));
    ChannelPMode temp(string);
    temp.SetBy = User(parameters[3]);
    temp.Parameter = parameters[2];
    temp.SetOn = QDateTime::fromTime_t(parameters[4].toUInt());
    if (channel->SetPMode(temp))
        emit this->Event_CPMInserted(parser, temp, channel);
    emit this->Event_PMode(parser, mode);
}

void Network::processMode(Parser *parser)
{
    if (parser->GetParameters().count() < 1)
    {
        emit this->Event_Broken(parser, "Invalid mode");
        return;
    }
    QString entity = parser->GetParameters()[0];
    if (entity.toLower() == this->localUser.GetNick().toLower())
    {
        // Someone changed our own UMode
        QString mode = parser->GetText();
        if (mode.isEmpty() && parser->GetParameters().count() > 1)
            mode = parser->GetParameters()[1];
        this->localUserMode.SetMode(mode);
    } else if (entity.startsWith(this->channelPrefix))
    {
        // Someone changed a channel mode
        // Get a channel first
        QStringList parameters = parser->GetParameters();
        Channel *channel = this->GetChannel(parameters[0]);
        if (channel == NULL)
        {
            qDebug() << "IRC PARSER: No channel: " + parser->GetRaw();
            return;
        }
        parameters.removeFirst();
        if (parameters.isEmpty())
        {
            qDebug() << "IRC PARSER: Invalid MODE: " + parser->GetRaw();
            return;
        }
        QString mode = parameters[0];
        parameters.removeFirst();
        QList<char> parameter_modes = this->ParameterModes();
        Mode new_mode(mode);
        // remove the parameter modes, as we can't apply them to local channel mode
        new_mode.ResetModes(parameter_modes);
        channel->SetMode(new_mode.ToString());
        emit this->Event_ChannelModeChanged(parser, channel);
        // now that we updated the static mode, we need to update all respective bans, users and similar stuff
        QList<libirc::SingleMode> modes = libirc::SingleMode::ToModeList(mode, parameters, parameter_modes);
        foreach (libirc::SingleMode sm, modes)
        {
            if (this->CUModes.contains(sm.Get()))
            {
                // User mode was changed
                User *user = channel->GetUser(sm.Parameter);
                if (user == NULL)
                {
                    qDebug() << "IRC PARSER: Invalid user: " + parser->GetRaw();
                    continue;
                }
                // User mode was changed, the trick here is that some irc daemons allow users to have multiple modes
                // so we need to figure if this user mode is higher level mode than mode that user currently posses
                char current_mode = user->GetHighestCUMode();
                if (sm.IsIncluding())
                {
                    if (!current_mode || !this->CUModes.contains(current_mode) || this->CUModes.indexOf(current_mode) > this->CUModes.indexOf(sm.Get()))
                    {
                        // yes this mode is higher, so let's update their current mode and also change their channel symbol
                        user->CUModes.insert(0, sm.Get());
                        user->ChannelPrefixes.insert(0, this->channelUserPrefixes[this->CUModes.indexOf(sm.Get())]);
                        emit this->Event_ChannelUserModeChanged(parser, channel, user);
                    } else
                    {
                        // this mode is not higher but we still want to track it, however we don't know which index it's meant to be
                        // which we need to figure out first
                        QList<char> temp = user->CUModes;
                        temp << sm.Get();
                        user->CUModes = this->ModeHelper_GetSortedCUModes(temp);
                        temp = user->ChannelPrefixes;
                        temp << this->channelUserPrefixes[this->CUModes.indexOf(sm.Get())];
                        user->ChannelPrefixes = this->ModeHelper_GetSortedChannelPrefixes(temp);
                        emit this->Event_ChannelUserSubmodeChanged(parser, channel, user);
                    }
                } else
                {
                    // The mode is revoked, however that matters only if user actually posses the mode, some irc servers
                    // let you revoke mode of user who never even had it
                    // so we check if user actually have that
                    if (sm.Get() == current_mode)
                    {
                        //! \todo Some networks allow users to have more than 1 user mode, although they don't easily actually share
                        //! that information. We should however maintain a list of modes this user posses
                        user->CUModes.removeAt(0);
                        user->ChannelPrefixes.removeAt(0);
                        emit this->Event_ChannelUserModeChanged(parser, channel, user);
                    }
                    else if (user->CUModes.contains(sm.Get()))
                    {
                        // User mode was changed, but not the highest one
                        user->ChannelPrefixes.removeAll(this->channelUserPrefixes[this->CUModes.indexOf(sm.Get())]);
                        user->CUModes.removeAll(sm.Get());
                        emit this->Event_ChannelUserSubmodeChanged(parser, channel, user);
                    }
                }
            } else if (this->CPModes.contains(sm.Get()))
            {
                // Ban / Exception / Invite
                ChannelPMode channel_mode(QString(QChar(sm.Get())));
                if (parser->GetSourceUserInfo())
                    channel_mode.SetBy = User(parser->GetSourceUserInfo());
                channel_mode.SetOn = QDateTime::currentDateTime();
                channel_mode.Parameter = sm.Parameter;
                if (!sm.IsIncluding())
                {
                    // Remove existing one
                    if (channel->RemovePMode(channel_mode))
                        emit this->Event_CPMRemoved(parser, channel_mode, channel);
                } else
                {
                    if (channel->SetPMode(channel_mode))
                        emit this->Event_CPMInserted(parser, channel_mode, channel);
                }
            }
        }
        //! \todo Bans / exemption and others
    } else
    {
        // Someone changed UMode of another user, this is not supported on majority of servers, unless you are services
    }
    emit this->Event_Mode(parser);
}

void Network::processMTime(Parser *parser)
{
    QStringList parameters = parser->GetParameters();
    if (parameters.size() < 3)
    {
        qDebug() << "IRC PARSER: Invalid MODETIME: " + parser->GetRaw();
        return;
    }
    Channel *channel = this->GetChannel(parameters[1]);
    if (!channel)
    {
        DebugInvalid("Channel struct not in memory", parser);
        return;
    }
    channel->SetMTime(QDateTime::fromTime_t(parameters[2].toUInt()));
    emit this->Event_CreationTime(parser);
}

void Network::processJoin(Parser *parser, bool self_command)
{
    Channel *channel_p = NULL;
    if (parser->GetParameters().count() < 1 && parser->GetText().isEmpty())
    {
        // broken
        DebugInvalid("Malformed JOIN", parser);
        return;
    }
    // On some extremely old servers channel is passed as text and on some as parameter
    // we don't need to do this if we are using IRCv.3 protocol
    QString channel_name = parser->GetText();
    if (parser->GetParameters().count() > 0)
        channel_name = parser->GetParameters()[0];
    if (!channel_name.startsWith(this->channelPrefix))
    {
        DebugInvalid("Malformed JOIN", parser);
        return;
    }
    // Check if the person who joined the channel isn't us
    if (self_command)
    {
        // Yes, we joined a new channel
        if (this->ContainsChannel(channel_name))
        {
            // what the fuck?? we joined the channel which we are already in
            qDebug() << "Server told us we just joined a channel we are already in: " + channel_name + " network: " + this->GetHost();
            goto join_emit;
        }
        channel_p = new Channel(channel_name, this);
        this->channels.append(channel_p);
        emit this->Event_SelfJoin(channel_p);
    }
join_emit:
    if (!channel_p)
        channel_p = this->GetChannel(channel_name);
    if (!channel_p)
    {
        qDebug() << "Server told us that user " + parser->GetSourceUserInfo()->ToString() + " joined channel " + channel_name + " which we are not in";
        return;
    }
    // Insert this user to a channel
    User *temp = parser->GetSourceUserInfo();
    if (this->_enableCap && !parser->GetText().isEmpty())
    {
        // We can use some features of IRCv3 to prefill the user name
        temp->SetRealname(parser->GetText());
    }
    User *user = channel_p->InsertUser(temp);
    emit this->Event_Join(parser, user, channel_p);
}

void Network::processNick(Parser *parser, bool self_command)
{
    QString new_nick;
    if (parser->GetParameters().count() < 1)
    {
        if (parser->GetText().isEmpty())
        {
            // wrong amount of parameters
            qDebug() << "IRC PARSER: Malformed NICK: " + parser->GetRaw();
            return;
        }
        else
        {
            new_nick = parser->GetText();
        }
    }
    else
    {
        new_nick = parser->GetParameters()[0];
    }
    // Precache the nicks to save hundreds of function calls
    QString old_nick = parser->GetSourceUserInfo()->GetNick();

    if (self_command)
    {
        // our own nick was changed
        this->localUser.SetNick(new_nick);
        emit this->Event_SelfNICK(parser, old_nick, new_nick);
    }
    // Change the nicks in every channel this user is in
    foreach (Channel *channel, this->channels)
        channel->ChangeNick(old_nick, new_nick);
    emit this->Event_NICK(parser, old_nick, new_nick);
}

void Network::processAway(Parser *parser, bool self_command)
{
    bool is_away = !parser->GetText().isEmpty();
    QString message = parser->GetText();
    if (self_command)
    {
        this->localUser.IsAway = is_away;
    }
    // Update away status for every user in every channel
    foreach (Channel *channel, this->channels)
    {
        if (channel->ContainsUser(parser->GetSourceUserInfo()->GetNick()))
        {
            User *user = channel->GetUser(parser->GetSourceUserInfo()->GetNick());
            if (user->IsAway != is_away)
            {
                user->IsAway = is_away;
                user->AwayMs = message;
                emit this->Event_UserAwayStatusChange(parser, channel, user);
            }
        }
    }
    emit this->Event_AWAY(parser);
}

void Network::processCap(Parser *parser)
{
    QStringList params = parser->GetParameters();
    if (params.size() < 2)
    {
        DebugInvalid("Wrong number of parameters for CAP message", parser);
        return;
    }
    QString cap = params[1].toUpper();
    if (cap == "LS")
    {
        if (params.size() > 2 && params[2] == "*")
            this->capProcessingMultilineLS = true;
        else
            this->capProcessingMultilineLS = false;
        // List of supported caps
        this->_capabilitiesSupported = Generic::UniqueMerge(this->_capabilitiesSupported, parser->GetText().split(" "));
        if (!this->capProcessingMultilineLS && !this->capAutoRequestFinished)
            this->processAutoCap();
    } else if (cap == "ACK" || cap == "NAK")
    {
        if (cap == "ACK")
        {
            this->_capabilitiesSubscribed = Generic::UniqueMerge(this->_capabilitiesSubscribed, parser->GetText().split(" "));
            emit this->Event_CAP_ACK(parser);
        }
        else
        {
            emit this->Event_CAP_NAK(parser);
        }

        // We don't really care if server approved or rejected the change, we just continue here
        if (this->capProcessingChangeRequest)
        {
            this->capProcessingChangeRequest = false;

            if (!this->capAutoRequestFinished)
            {
                this->capAutoRequestFinished = true;
                // finish the login
                this->TransferRaw("CAP END");
                this->standardLogin();
            }
        }
    }
    emit this->Event_CAP(parser);
}

void Network::standardLogin()
{
    this->capTimeout.stop();
    this->_loggedIn = true;
    this->TransferRaw("USER " + this->localUser.GetIdent() + " 8 * :" + this->localUser.GetRealname());
    this->TransferRaw("NICK " + this->localUser.GetNick());
    this->lastPing = QDateTime::currentDateTime();
    this->timerPingSend = new QTimer(this);
    connect(this->timerPingSend, SIGNAL(timeout()), this, SLOT(OnPingSend()));
    this->timerPingTimeout = new QTimer(this);
    this->timerPingSend->start(this->pingRate);
    connect(this->timerPingTimeout, SIGNAL(timeout()), this, SLOT(OnPing()));
    this->timerPingTimeout->start(2000);
}

void Network::process433(Parser *parser)
{
    if (this->loggedIn)
    {
        emit this->Event_NickCollision(parser);
        return;
    }
    // Try to get some alternative nick
    bool no_alternative = this->alternateNick.isEmpty();
    if (this->originalNick == this->alternateNick || this->GetNick() == this->alternateNick)
        no_alternative = true;
    if (no_alternative)
    {
        // We need to request original nick with a suffix number
        this->alternateNickNumber++;
        if (this->originalNick.isEmpty())
            this->originalNick = this->GetNick();
        QString nick = this->originalNick + QString::number(this->alternateNickNumber);
        this->localUser.SetNick(nick);
        this->RequestNick(nick);
    }
    else
    {
        this->RequestNick(this->alternateNick);
        this->localUser.SetNick(this->alternateNick);
    }
    emit this->Event_NickCollision(parser);
}

void Network::deleteTimers()
{
    if (this->timerPingSend)
    {
        this->timerPingSend->stop();
        this->timerPingSend->deleteLater();
        this->timerPingSend = NULL;
    }
    if (this->timerPingTimeout)
    {
        this->timerPingTimeout->stop();
        this->timerPingTimeout->deleteLater();
        this->timerPingTimeout = NULL;
    }
    this->capTimeout.stop();
    this->senderTimer.stop();
}

void Network::initialize()
{
    this->bytesSent = 0;
    this->bytesRcvd = 0;
    this->_loggedIn = false;
    this->socket = NULL;
    this->resetCap();
    this->_enableCap = true;
    this->_capGraceTime = 20;
    this->localUser.SetIdent("grumpy");
    this->localUser.SetRealname("GrumpyIRC");
    this->pingTimeout = 60;
    this->timerPingTimeout = NULL;
    this->timerPingSend = NULL;
    this->scheduling = true;
    this->pingRate = 20000;
    this->defaultQuit = "Grumpy IRC";
    this->channelPrefix = '#';
    this->autoRejoin = false;
    this->identifyString = "PRIVMSG NickServ identify $nickname $password";
    this->alternateNickNumber = 0;
    this->server = new Server();
    this->ResolveOnSelfChanges = true;
    this->ResolveOnNickConflicts = true;
    this->loggedIn = false;
    // This is overriden for every server that is following IRC standards
    this->channelUserPrefixes << '@' << '+';
    this->CUModes << 'o' << 'v';
    this->CModes << 'i' << 'm';
    connect(&this->capTimeout, SIGNAL(timeout()), this, SLOT(OnCapSupportTimeout()));
    connect(&this->senderTimer, SIGNAL(timeout()), this, SLOT(OnSend()));
    this->ChannelModeHelp.insert('m', "Moderated - will suppress all messages from people who don't have voice (+v) or higher.");
    this->ChannelModeHelp.insert('t', "Topic changes restricted - only allow privileged users to change the topic.");
    this->MSDelayOnEmpty = 300;
    this->MSDelayOnOpen = 2000;
    this->MSWait = 800;
    this->senderTime = QDateTime::currentDateTime();
}

void Network::freemm()
{
    qDeleteAll(this->channels);
    this->channels.clear();
    qDeleteAll(this->users);
    this->users.clear();
    this->mutex.lock();
    this->lprFIFO.clear();
    this->mprFIFO.clear();
    this->hprFIFO.clear();
    this->mutex.unlock();
}

void Network::resetCap()
{
    this->capAutoRequestFinished = false;
    this->capProcessingChangeRequest = false;
    this->capProcessingMultilineLS = false;
    this->_capabilitiesRequested.clear();
    this->_capabilitiesSubscribed.clear();
    this->_capabilitiesSupported.clear();
    this->_capabilitiesRequested << "away-notify" << "extended-join" << "multi-prefix";
}

void Network::processAutoCap()
{
    if (!this->_enableCap)
    {
        if (!this->loggedIn)
        {
            this->TransferRaw("CAP END");
            this->standardLogin();
        }
        return;
    }
    // We finished processing LS of all caps, so let's subscribe to all these that we want to have
    QString requested_list;
    foreach (QString capability, this->_capabilitiesRequested)
    {
        if (!this->HasCap(capability))
        {
            emit this->Event_CAP_RequestedCapNotSupported(capability);
            continue;
        }
        // Request the capability
        requested_list += capability + " ";
    }
    requested_list = requested_list.trimmed();
    if (requested_list.isEmpty())
    {
        // There is nothing to request, finish the request and connect to network
        this->TransferRaw("CAP END");
        this->standardLogin();
    } else
    {
        // Request the caps
        this->TransferRaw("CAP REQ :" + requested_list);
        this->capProcessingChangeRequest = true;
    }
}

void Network::scheduleDelivery(QByteArray data, libircclient::Priority priority)
{
    if (priority == Priority_RealTime)
    {
        if (!this->socket)
            return;
        this->bytesSent += data.size();
        this->socket->write(data);
        this->socket->flush();
        return;
    }
    this->mutex.lock();
    switch (priority)
    {
        case Priority_High:
            this->hprFIFO.append(data);
            break;
        case Priority_Normal:
            this->mprFIFO.append(data);
            break;
        case Priority_Low:
            this->lprFIFO.append(data);
            break;
    }
    this->mutex.unlock();
}

void Network::OnSend()
{
    if (QDateTime::currentDateTime() < this->senderTime)
        return;
    if (!this->socket)
        return;
    QByteArray packet = this->getDataToSend();
    if (packet.isEmpty())
    {
        //this->pseudoSleep(this->MSDelayOnEmpty);
        return;
    }
    QString line(packet);
    this->bytesSent += packet.size();
    this->socket->write(packet);
    this->socket->flush();
    this->pseudoSleep(this->MSWait);
}

void Network::pseudoSleep(unsigned int msec)
{
    this->senderTime = QDateTime::currentDateTime().addMSecs(msec);
}

QByteArray Network::getDataToSend()
{
    QByteArray item;
    this->mutex.lock();
    if (this->hprFIFO.size())
    {
        item = this->hprFIFO.first();
        this->hprFIFO.removeFirst();
    } else if (this->mprFIFO.size())
    {
        item = this->mprFIFO.first();
        this->mprFIFO.removeFirst();
    } else if (this->lprFIFO.size())
    {
        item = this->lprFIFO.first();
        this->lprFIFO.removeFirst();
    }
    this->mutex.unlock();
    return item;
}
