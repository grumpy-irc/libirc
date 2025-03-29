//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2025

#include <QtNetwork>
#include <QAbstractSocket>
#include <QDebug>
#include "network.h"
#include "server.h"
#include "channel.h"
#include "parser.h"
#include "networkmodehelp.h"
#include "generic.h"
#include "../libirc/serveraddress.h"
#include "../libirc/error_code.h"
#include <algorithm> // Add this include for std::sort
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#define fromTime_t fromSecsSinceEpoch
#endif

using namespace libircclient;

Network::Network(libirc::ServerAddress &server, const QString &name, const Encoding &enc) : libirc::Network(name)
{
    this->initialize();
    this->hostname = server.GetHost();
    if (server.GetNick().isEmpty())
        this->localUser.SetNick("GrumpyUser");
    else
        this->localUser.SetNick(server.GetNick());
    if (!server.GetRealname().isEmpty())
        this->localUser.SetRealname(server.GetRealname());
    if (!server.GetIdent().isEmpty())
        this->localUser.SetIdent(server.GetIdent());
    if (!server.GetPassword().isEmpty())
        this->SetPassword(server.GetPassword());
    this->usingSSL = server.UsingSSL();
    this->port = server.GetPort();
    this->channelsToJoin.clear();
    this->encoding = enc;
    if (!server.GetSuffix().isEmpty())
    {
        QList<QString> channels_join = server.GetSuffix().split(",");
        foreach (QString channel, channels_join)
        {
            if (!channel.startsWith("#"))
                channel = "#" + channel;
            if (!channel.contains(" ") && !this->channelsToJoin.contains(channel))
                this->channelsToJoin.append(channel);
        }
    }
}

Network::Network(const QHash<QString, QVariant> &hash) : libirc::Network("")
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
    this->_loggedIn = false;
    //delete this->network_thread;
    //delete this->socket;

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
        this->TransferRaw("QUIT :" + reason, Priority_RealTime);
        if (this->socket)
            this->socket->close();
    }
    if (this->socket)
    {
        this->socket->deleteLater();
        this->socket = nullptr;
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

void libircclient::Network::SetAway(bool away, const QString &message)
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

void Network::SetDefaultNick(const QString &nick)
{
    if (!this->IsConnected())
        this->localUser.SetNick(nick);
}

void Network::SetDefaultIdent(const QString &ident)
{
    if (!this->IsConnected())
        this->localUser.SetIdent(ident);
}

void Network::SetDefaultUsername(const QString &realname)
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

#define CTCP_SEPARATOR QString((char)1)

int Network::SendMessage(const QString &text, const QString &target, Priority priority)
{
    this->TransferRaw("PRIVMSG " + target + " :" + text, priority);
    return SUCCESS;
}

int Network::SendAction(const QString &text, Channel *channel, Priority priority)
{
    return this->SendAction(text, channel->GetName(), priority);
}

int Network::SendAction(const QString &text, const QString &target, Priority priority)
{
    this->TransferRaw(QString("PRIVMSG ") + target + " :" + CTCP_SEPARATOR + "ACTION " + text + CTCP_SEPARATOR, priority);
    return SUCCESS;
}

int Network::SendNotice(const QString &text, User *user, Priority priority)
{
    return this->SendNotice(text, user->GetNick(), priority);
}

int Network::SendNotice(const QString &text, Channel *channel, Priority priority)
{
    return this->SendNotice(text, channel->GetName(), priority);
}

int Network::SendNotice(const QString &text, const QString &target, Priority priority)
{
    this->TransferRaw(QString("NOTICE ") + target + " :" + text, priority);
    return SUCCESS;
}

int Network::SendMessage(const QString &text, Channel *channel, Priority priority)
{
    return this->SendMessage(text, channel->GetName(), priority);
}

int Network::SendMessage(const QString &text, User *user, Priority priority)
{
    return this->SendMessage(text, user->GetNick(), priority);
}

void Network::RequestPart(const QString &channel_name, Priority priority)
{
    this->TransferRaw("PART " + channel_name, priority);
}

void Network::RequestPart(Channel *channel, Priority priority)
{
    this->TransferRaw("PART " + channel->GetName(), priority);
}

void Network::RequestNick(const QString &nick, Priority priority)
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

void Network::RequestCapability(const QString &capability)
{
    if (!this->_capabilitiesRequested.contains(capability))
        this->_capabilitiesRequested.append(capability);
}

void Network::DisableCapability(const QString &capability)
{
    if (this->_capabilitiesRequested.contains(capability))
        this->_capabilitiesRequested.removeAll(capability);
}

bool Network::CapabilityRequested(const QString &capability)
{
    return this->_capabilitiesRequested.contains(capability);
}

bool Network::CapabilityEnabled(const QString &capability)
{
    return this->_capabilitiesSubscribed.contains(capability);
}

bool Network::CapabilitySupported(const QString &capability)
{
    return this->_capabilitiesSupported.contains(capability);
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

void Network::_st_SetNick(const QString &nick)
{
    this->localUser.SetNick(nick);
}

int Network::GetTimeout() const
{
    return this->pingTimeout;
}

int Network::SendCtcp(const QString &name, const QString &text, const QString &target, Priority priority)
{
    if (!text.isEmpty())
        this->TransferRaw(QString("PRIVMSG ") + target + " :" + CTCP_SEPARATOR + name + " " + text + CTCP_SEPARATOR, priority);
    else
        this->TransferRaw(QString("PRIVMSG ") + target + " :" + CTCP_SEPARATOR + name + CTCP_SEPARATOR, priority);
    return SUCCESS;
}

void Network::SetHelpForMode(char mode, const QString &message)
{
    if (this->ChannelModeHelp.contains(mode))
        this->ChannelModeHelp[mode] = message;
    else
        this->ChannelModeHelp.insert(mode, message);
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

void Network::SetPassword(const QString &Password)
{
    this->password = Password;
}

void Network::RequestJoin(const QString &name, Priority priority)
{
    this->TransferRaw("JOIN " + name, priority);
}

void Network::OnPingSend()
{
    this->TransferRaw("PING :" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()), libircclient::Priority_RealTime);
}

void Network::OnCapSupportTimeout()
{
    // Prevent this from running multiple times
    this->capTimeout.stop();
    emit this->Event_CAP_Timeout();
    this->DisableIRCv3Support();
}

void Network::OnReceive(const QByteArray &data)
{
    if (data.length() == 0)
        return;

    emit this->Event_RawIncoming(data);

    this->processIncomingRawData(data);
}

void Network::closeError(const QString &error, int code)
{
    // Delete the socket first to prevent neverending loop
    // for some reason when you call the destructor Qt emits
    // some errors again causing program to hang
    if (this->socket == nullptr)
        return;
    QTcpSocket *temp = this->socket;
    this->socket = nullptr;
    temp->close();
    temp->deleteLater();
    this->deleteTimers();
    emit this->Event_NetworkFailure(error, code);
    emit this->Event_Disconnected();
}

void Network::updateSelfAway(Parser *parser, bool status, const QString &text)
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
    if (this->socket == nullptr)
        return;
    emit this->Event_ConnectionFailure(er);
    this->closeError(Generic::ErrorCode2String(er), 1);
}

void Network::OnReceive()
{
    if (!this->IsConnected())
        return;
    // We need to keep checking if socket is not NULL because following calls from
    // processIncomingRawData can initiate disconnect, which would delete it and change
    // it to NULL
    while (this->socket && this->socket->canReadLine())
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
    {
        int index = this->channelUserPrefixes.indexOf(first_symbol);
        if (index < this->CUModes.count())
            return this->CUModes[index];
    }
    return 0;
}

int Network::PositionOfUCPrefix(char prefix)
{
    // Check if there is such a prefix
    if (!this->channelUserPrefixes.contains(prefix))
        return -1;

    return this->channelUserPrefixes.indexOf(prefix);
}

void Network::SetChannelUserPrefixes(const QList<char> &data)
{
    this->channelUserPrefixes = data;
}

void Network::SetCModes(const QList<char> &data)
{
    this->CModes = data;
}

QList<char> Network::GetChannelUserPrefixes()
{
    return this->channelUserPrefixes;
}

bool Network::HasCap(const QString &cap)
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

void Network::SetCPModes(const QList<char> &data)
{
    this->CPModes = data;
}

void Network::SetCRModes(const QList<char> &data)
{
    this->CRModes = data;
}

QList<char> Network::GetCRModes()
{
    return this->CRModes;
}

QList<char> Network::GetSTATUSMSGModes()
{
    return this->STATUSMSG_Modes;
}

void Network::SetSTATUSMSGModes(const QList<char> &data)
{
    this->STATUSMSG_Modes = data;
}

void Network::SetCUModes(const QList<char> &data)
{
    this->CUModes = data;
}

void Network::SetCCModes(const QList<char> &data)
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
    std::sort(unsorted_ints.begin(), unsorted_ints.end());
    QList<char> sorted_list;
    foreach (int mode, unsorted_ints)
        sorted_list.append(mask[mode]);
    return sorted_list;
}

QList<char> Network::ModeHelper_GetSortedChannelPrefixes(const QList<char> &unsorted_list)
{
    return SortingHelper(this->channelUserPrefixes, unsorted_list);
}

QList<char> Network::ModeHelper_GetSortedCUModes(const QList<char> &unsorted_list)
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

void Network::LoadHash(const QHash<QString, QVariant> &hash)
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
    UNSERIALIZE_CHARLIST(STATUSMSG_Modes);
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
    if (hash.contains("encoding"))
        this->encoding = static_cast<Encoding> (hash["encoding"].toInt());
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
    hash.insert("STATUSMSG_Modes", serializeList(this->STATUSMSG_Modes));
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
    hash.insert("encoding", static_cast<int>(this->encoding));
    return hash;
}

// This is a slot so don't change the signature to const QList<QSslError> &errors
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
    return nullptr;
}

QList<Channel *> Network::GetChannels()
{
    return this->channels;
}

Encoding Network::GetEncoding()
{
    return this->encoding;
}

QList<char> Network::GetCUModes()
{
    return this->CUModes;
}

bool Network::ContainsChannel(const QString &channel_name)
{
    return this->GetChannel(channel_name) != nullptr;
}

long long Network::GetLag()
{
    return this->lastPingResponseTimeInMs;
}

long long Network::GetBytesSent()
{
    return this->bytesSent;
}

long long Network::GetBytesReceived()
{
    return this->bytesRcvd;
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
    }
    this->standardLogin();
}

void Network::processIncomingRawData(QByteArray data)
{
    this->lastPing = QDateTime::currentDateTime();
    QString l;
    switch (this->encoding)
    {
        case EncodingDefault:
            l = QString(data);
            break;
        case EncodingASCII:
        case EncodingLatin:
            l = QString::fromLatin1(data.data());
            break;
        case EncodingUTF8:
            l = QString::fromUtf8(data.data());
            break;
        case EncodingUTF16:
        {
            #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QStringDecoder decoder(QStringDecoder::Encoding::Utf16);
            l = decoder.decode(data);
            #else
            l = QTextCodec::codecForName("UTF-16")->toUnicode(data);
            #endif
        }
            break;
        default:
            l = QString(data);
            break;
    }
    // let's try to parse this IRC command
    Parser parser(l);
    if (!parser.IsValid())
    {
        emit this->Event_Invalid(data);
        return;
    }
    bool self_command = false;
    if (parser.GetSourceUserInfo() != nullptr)
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
            // Example (unreal ircd): :irc2.tm-irc.org 004 GrumpyUser irc2.tm-irc.org UnrealIRCd-4.0.17 iowrsxzdHtIDZRqpWGTSB lvhopsmntikraqbeIzMQNRTOVKDdGLPZSCcf
            if (parser.GetParameters().count() > 1)
            {
                this->server->SetName(parser.GetParameters()[1]);
                this->server->SetVersion(parser.GetParameters()[2]);
                this->ChannelModeHelp = NetworkModeHelp::GetChannelModeHelp(this->server->GetVersion());
                this->UserModeHelp = NetworkModeHelp::GetUserModeHelp(this->server->GetVersion());
            }
            this->autoJoin();
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
                    emit this->Event_Broken(&parser, "Channel struct not in memory");
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
        case IRC_NUMERIC_RAW_CHGHOST:
            this->processChangeHost(parser);
            break;
        case IRC_NUMERIC_RAW_PONG:
        {
            // Get the parameters and check how long it took
            bool ok = true;
            qint64 last_ping = parser.GetText().toLongLong(&ok);
            if (!ok)
            {
                // The signal is emitted only once we are done with calculations, or in case calculation fails
                // otherwise it wouldn't be precise enough
                emit this->Event_PONG(&parser);
                break;
            }
            QDateTime lpdt = QDateTime::fromMSecsSinceEpoch(last_ping);
            this->lastPingResponseTimeInMs = lpdt.msecsTo(QDateTime::currentDateTime());
            emit this->Event_PONG(&parser);
        }
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
        case IRC_NUMERIC_WHOISUSER:
            emit this->Event_WhoisGeneric(&parser);
            this->processWhoisUser(parser);
            break;
        case IRC_NUMERIC_WHOISIDLE:
            emit this->Event_WhoisGeneric(&parser);
            this->processWhoisIdle(parser);
            break;
        case IRC_NUMERIC_WHOISOPERATOR:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisOperator(&parser);
            break;
        case IRC_NUMERIC_WHOISREGNICK:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisRegNick(&parser);
            break;
        case IRC_NUMERIC_WHOISCHANNELS:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisChannels(&parser);
            break;
        case IRC_NUMERIC_WHOISSERVER:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisServer(&parser);
            break;
        case IRC_NUMERIC_ENDOFWHOIS:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisEnd(&parser);
            break;
        case IRC_NUMERIC_AWAY:
            emit this->Event_RplAway(&parser);
            break;
        case IRC_NUMERIC_WHOISSECURE:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisSecure(&parser);
            break;
        case IRC_NUMERIC_WHOISSPECIAL:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisSpecial(&parser);
            break;
        case IRC_NUMERIC_WHOISHOST:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisHost(&parser);
            break;
        case IRC_NUMERIC_WHOISMODES:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisModes(&parser);
            break;
        case IRC_NUMERIC_WHOISACCOUNT:
            emit this->Event_WhoisGeneric(&parser);
            emit this->Event_WhoisAccount(&parser);
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
                emit this->Event_Broken(&parser, "Channel struct not in memory");
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
        case IRC_NUMERIC_ENDOFWHO:
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
            if (autoIdentify)
            {
                Identify();
            }
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
        emit this->Event_Broken(parser, "Malformed NAMRPL");
        return;
    }

    Channel *channel = this->GetChannel(parser->GetParameters()[2]);
    if (channel == NULL)
    {
        emit this->Event_Broken(parser, "Channel struct not in memory");
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
        } else if (info.startsWith("NETWORK="))
        {
            this->networkName = info.mid(8);
            continue;
        } else if (info.startsWith("STATUSMSG="))
        {
            this->STATUSMSG_Modes = CLFromStr(info.mid(10));
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
    Channel *channel = nullptr;
    QString gecos;
    User *user = nullptr;
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
#ifdef QT_VERSION
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            user->Hops = gecos.mid(0, gecos.indexOf(" ")).toInt();
#else
            user->Hops = gecos.midRef(0, gecos.indexOf(" ")).toInt();
#endif
#endif
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

    // https://tools.ietf.org/id/draft-oakley-irc-ctcp-01.html
    if (text.startsWith(CTCP_SEPARATOR))
    {
        // This is a CTCP message
        text = text.mid(1);
        if (text.endsWith(CTCP_SEPARATOR))
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
        emit this->Event_Broken(parser, "Channel struct not in memory");
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
        emit this->Event_Broken(parser, "Channel struct not in memory");
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
            emit this->Event_Broken(parser, "Channel struct not in memory");
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
        emit this->Event_Broken(parser, "Channel struct not in memory");
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
        if (channel == nullptr)
        {
            emit this->Event_Broken(parser, "No channel");
            return;
        }
        parameters.removeFirst();
        if (parameters.isEmpty())
        {
            emit this->Event_Broken(parser, "Invalid mode");
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
                if (user == nullptr)
                {
                    emit this->Event_Broken(parser, "Invalid user");
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
        emit this->Event_Broken(parser, "Invalid MODETIME");
        return;
    }
    Channel *channel = this->GetChannel(parameters[1]);
    if (!channel)
    {
        emit this->Event_Broken(parser, "Channel struct not in memory");
        return;
    }
    channel->SetMTime(QDateTime::fromTime_t(parameters[2].toUInt()));
    emit this->Event_CreationTime(parser);
}

void Network::processJoin(Parser *parser, bool self_command)
{
    Channel *channel_p = nullptr;
    if (parser->GetParameters().count() < 1 && parser->GetText().isEmpty())
    {
        emit this->Event_Broken(parser, "Malformed JOIN");
        return;
    }
    // On some extremely old servers channel is passed as text and on some as parameter
    // we don't need to do this if we are using IRCv.3 protocol
    QString channel_name = parser->GetText();
    if (parser->GetParameters().count() > 0)
        channel_name = parser->GetParameters()[0];
    if (!channel_name.startsWith(this->channelPrefix))
    {
        emit this->Event_Broken(parser, "Malformed JOIN");
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
        } else
        {
            channel_p = new Channel(channel_name, this);
            this->channels.append(channel_p);
            emit this->Event_SelfJoin(channel_p);
        }
    }
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
        emit this->Event_Broken(parser, "Wrong number of parameters for CAP message");
        return;
    }
    // Obviously this network is supporting IRCv3
    this->capTimeout.stop();
    if (!this->SupportsIRCv3())
        this->EnableIRCv3Support();
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

void Network::processWhoisUser(Parser &parser)
{
    libircclient::User user;
    QList<QString> parameters = parser.GetParameters();
    if (parameters.count() > 1)
        user.SetNick(parameters[1]);
    if (parameters.count() > 3)
    {
        user.SetIdent(parameters[2]);
        user.SetHost(parameters[3]);
    }
    user.SetRealname(parser.GetText());
    emit this->Event_WhoisUser(&parser, &user);
}

void Network::processWhoisIdle(Parser &parser)
{
    unsigned int idle_time = 0;
    QDateTime signon_time;
    QList<QString> parameters = parser.GetParameters();

    if (parameters.count() < 4)
    {
        emit this->Event_WhoisIdle(&parser, idle_time, signon_time);
        return;
    }

    idle_time = parameters[2].toUInt();
    signon_time = QDateTime::fromTime_t(parameters[3].toUInt());

    emit this->Event_WhoisIdle(&parser, idle_time, signon_time);
}

void Network::processChangeHost(Parser &parser)
{
    QString new_host, new_ident, old_host, old_ident, nick;
    if (parser.GetParameters().count() < 2)
    {
        // wrong amount of parameters
        qDebug() << "IRC PARSER: Malformed CHGHOST: " + parser.GetRaw();
        return;
    }

    new_ident = parser.GetParameters()[0];
    new_host = parser.GetParameters()[1];
    old_ident = parser.GetSourceUserInfo()->GetIdent();
    old_host = parser.GetSourceUserInfo()->GetHost();
    nick = parser.GetSourceUserInfo()->GetNick();

    if (nick.toLower() == this->GetNick().toLower())
    {
        // our own hostname / ident was changed
        this->localUser.SetIdent(new_ident);
        this->localUser.SetHost(new_host);
        emit this->Event_SelfCHGHOST(&parser, old_host, old_ident, new_host, new_ident);
    }
    // Change the hosts in every channel this user is in
    foreach (Channel *channel, this->channels)
        channel->ChangeHost(nick, new_host, new_ident);
    emit this->Event_CHGHOST(&parser, old_host, old_ident, new_host, new_ident);
}

void Network::standardLogin()
{
    if (this->_loggedIn)
        return;
    this->_loggedIn = true;
    this->TransferRaw("USER " + this->localUser.GetIdent() + " * * :" + this->localUser.GetRealname());
    this->TransferRaw("NICK " + this->localUser.GetNick());
    this->lastPing = QDateTime::currentDateTime();
    this->timerPingSend = new QTimer(this);
    connect(this->timerPingSend, SIGNAL(timeout()), this, SLOT(OnPingSend()));
    this->timerPingTimeout = new QTimer(this);
    this->timerPingSend->start(this->pingRate);
    connect(this->timerPingTimeout, SIGNAL(timeout()), this, SLOT(OnPing()));
    this->timerPingTimeout->start(2000);
    emit this->Event_Connected();
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
        this->timerPingSend = nullptr;
    }
    if (this->timerPingTimeout)
    {
        this->timerPingTimeout->stop();
        this->timerPingTimeout->deleteLater();
        this->timerPingTimeout = nullptr;
    }
    this->capTimeout.stop();
    this->senderTimer.stop();
}

void Network::initialize()
{
    this->bytesSent = 0;
    this->bytesRcvd = 0;
    this->_loggedIn = false;
    this->socket = nullptr;
    this->resetCap();
    this->_enableCap = true;
    this->_capGraceTime = 20;
    this->localUser.SetIdent("libirc");
    this->localUser.SetRealname("https://github.com/grumpy-irc/libirc");
    this->pingTimeout = 60;
    this->timerPingTimeout = nullptr;
    this->timerPingSend = nullptr;
    this->scheduling = true;
    this->pingRate = 20000;
    this->defaultQuit = "GrumpyChat libirc: https://github.com/grumpy-irc/libirc";
    this->channelPrefix = '#';
    this->autoRejoin = false;
    this->autoIdentify = true;
    this->identifyString = "PRIVMSG NickServ identify $nickname $password";
    this->alternateNickNumber = 0;
    this->server = new Server();
    this->ResolveOnSelfChanges = true;
    this->ResolveOnNickConflicts = true;
    this->loggedIn = false;
    // This is overriden for every server that is following IRC standards
    this->channelUserPrefixes << '~' << '&' << '@' << '%' << '+';
    this->CUModes << 'q' << 'a' << 'o' << 'h' << 'v';
    this->CModes << 'i' << 'm';
    this->STATUSMSG_Modes << '@' << '+';
    connect(&this->capTimeout, SIGNAL(timeout()), this, SLOT(OnCapSupportTimeout()));
    connect(&this->senderTimer, SIGNAL(timeout()), this, SLOT(OnSend()));
    this->ChannelModeHelp = NetworkModeHelp::GetChannelModeHelp("unknown");
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
    this->_capabilitiesRequested << "away-notify" << "extended-join" << "multi-prefix" << "chghost" << "server-time";
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

void Network::scheduleDelivery(const QByteArray &data, libircclient::Priority priority)
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
        // This will never happen because we already handled this priority level in top of this
        // it's here just to silence clang
        case Priority_RealTime:
            break;
    }
    this->mutex.unlock();
}

void Network::autoJoin()
{
    while(!this->channelsToJoin.isEmpty())
    {
        this->RequestJoin(this->channelsToJoin.at(0), Priority_Low);
        this->channelsToJoin.removeAt(0);
    }
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
    //QString line(packet);
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
    if (!this->hprFIFO.empty())
    {
        item = this->hprFIFO.first();
        this->hprFIFO.removeFirst();
    } else if (!this->mprFIFO.empty())
    {
        item = this->mprFIFO.first();
        this->mprFIFO.removeFirst();
    } else if (!this->lprFIFO.empty())
    {
        item = this->lprFIFO.first();
        this->lprFIFO.removeFirst();
    }
    this->mutex.unlock();
    return item;
}
