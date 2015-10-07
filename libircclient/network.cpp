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
#include <QDebug>
#include "network.h"
#include "server.h"
#include "channel.h"
#include "parser.h"
#include "user.h"
#include "../libirc/serveraddress.h"
#include "../libirc/error_code.h"

using namespace libircclient;

Network::Network(libirc::ServerAddress &server, QString name) : libirc::Network(name)
{
    this->socket = NULL;
    this->hostname = server.GetHost();
    this->localUser.SetIdent("grumpy");
    if (server.GetNick().isEmpty())
        this->localUser.SetNick("GrumpyUser");
    else
        this->localUser.SetNick(server.GetNick());
    this->localUser.SetRealname("GrumpyIRC");
    this->usingSSL = server.UsingSSL();
    this->pingTimeout = 60;
    this->port = server.GetPort();
    this->timerPingTimeout = NULL;
    this->timerPingSend = NULL;
    this->pingRate = 20000;
    this->defaultQuit = "Grumpy IRC";
    this->channelPrefix = '#';
    this->autoRejoin = false;
    this->identifyString = "PRIVMSG NickServ identify $nickname $password";
    this->server = new Server();
}

Network::~Network()
{
    delete this->server;
    this->deleteTimers();
    delete this->socket;
    qDeleteAll(this->channels);
    qDeleteAll(this->users);
}

void Network::Connect()
{
    if (this->IsConnected())
        return;
    //delete this->network_thread;
    delete this->socket;
    //this->network_thread = new NetworkThread(this);
    this->socket = new QTcpSocket();
    connect(this->socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnError(QAbstractSocket::SocketError)));
    connect(this->socket, SIGNAL(readyRead()), this, SLOT(OnReceive()));
    connect(this->socket, SIGNAL(connected()), this, SLOT(OnConnected()));
    this->socket->connectToHost(this->hostname, this->port);
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
        this->TransferRaw("QUIT :" + reason);
        this->socket->close();
    }
    delete this->socket;
    this->socket = NULL;
    this->deleteTimers();
}

bool Network::IsConnected()
{
    if (!this->socket)
        return false;

    return this->socket->isOpen();
}

QString Network::GetNick()
{
    return this->localUser.GetNick();
}

QString Network::GetHost()
{
    return this->localUser.GetHost();
}

QString Network::GetIdent()
{
    return this->localUser.GetIdent();
}

void Network::TransferRaw(QString raw)
{
    if (!this->IsConnected())
        return;

    // Remove garbage for security reasons
    raw.replace("\r", "").replace("\n", "");

    QByteArray data = QString(raw + "\n").toUtf8();
    emit this->Event_RawOutgoing(data);
    this->socket->write(data);
    this->socket->flush();
}

int Network::SendMessage(QString text, QString target)
{
    this->TransferRaw("PRIVMSG " + target + " :" + text);
    return SUCCESS;
}

int Network::SendMessage(QString text, Channel *channel)
{
    return this->SendMessage(text, channel->GetName());
}

int Network::SendMessage(QString text, User *user)
{
    return this->SendMessage(text, user->GetNick());
}

void Network::Part(QString channel_name)
{
    this->TransferRaw("PART " + channel_name);
}

void Network::Part(Channel *channel)
{
    this->TransferRaw("PART " + channel->GetName());
}

void Network::Identify(QString Nickname, QString Password)
{
    if (Nickname.isEmpty())
        Nickname = this->localUser.GetNick();
    if (Password.isEmpty())
        Password = this->password;
    QString ident_line = this->identifyString;
    ident_line.replace("$nickname", Nickname).replace("$password", Password);
    this->TransferRaw(ident_line);
}

QString Network::GetServerAddress()
{
    return this->hostname;
}

int Network::GetTimeout() const
{
    return this->pingTimeout;
}

void Network::OnPing()
{
    // Check ping timeout
    if (QDateTime::currentDateTime() > this->lastPing.addSecs(this->pingTimeout))
    {
        emit this->Event_Timeout();
        this->Disconnect();
    }
}

void Network::SetPassword(QString Password)
{
    this->password = Password;
}

void Network::OnPingSend()
{
    this->TransferRaw("PING :" + this->GetHost());
}

void Network::OnError(QAbstractSocket::SocketError er)
{
    emit this->Event_ConnectionFailure(er);
    this->Disconnect();
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

User *Network::GetLocalUserInfo()
{
    return &this->localUser;
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

bool Network::ContainsChannel(QString channel_name)
{
    return this->GetChannel(channel_name) != NULL;
}

void Network::OnConnected()
{
    // We just connected to an IRC network
    this->TransferRaw("USER " + this->localUser.GetIdent() + " 8 * :" + this->localUser.GetRealname());
    this->TransferRaw("NICK " + this->localUser.GetNick());
    this->lastPing = QDateTime::currentDateTime();
    this->deleteTimers();
    this->timerPingSend = new QTimer(this);
    connect(this->timerPingSend, SIGNAL(timeout()), this, SLOT(OnPingSend()));
    this->timerPingTimeout = new QTimer(this);
    this->timerPingSend->start(this->pingRate);
    connect(this->timerPingTimeout, SIGNAL(timeout()), this, SLOT(OnPing()));
    this->timerPingTimeout->start(2000);
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
    bool known = false;
    switch (parser.GetNumeric())
    {
        case IRC_NUMERIC_PING_CHECK:
            known = true;
            if (parser.GetParameters().count() == 0)
                this->TransferRaw("PONG");
            else
                this->TransferRaw("PONG :" + parser.GetParameters()[0]);
            break;
        case IRC_NUMERIC_MYINFO:
            known = true;
            // Process the information about network
            if (parser.GetParameters().count() < 4)
                break;
            this->server->SetName(parser.GetParameters()[0]);
            this->server->SetVersion(parser.GetParameters()[1]);
            break;

        case IRC_NUMERIC_JOIN:
        {
            known = true;
            Channel *channel_p = NULL;
            if (parser.GetParameters().count() < 1)
            {
                // broken
                qDebug() << "IRC PARSER: Malformed JOIN " + parser.GetRaw();
                break;
            }
            // Check if the person who joined the channel isn't us
            if (self_command)
            {
                // Yes, we joined a new channel
                QString channel = parser.GetParameters()[0];
                if (this->ContainsChannel(channel))
                {
                    // what the fuck?? we joined the channel which we are already in
                    qDebug() << "Server told us we just joined a channel we are already in: " + channel + " network: " + this->GetHost();
                    goto join_emit;
                }
                channel_p = new Channel(channel, this);
                this->channels.append(channel_p);
                emit this->Event_SelfJoin(channel_p);
            }
        join_emit:
            if (!channel_p)
                channel_p = this->GetChannel(parser.GetParameters()[0]);
            if (!channel_p)
            {
                qDebug() << "Server told us that user " + parser.GetSourceUserInfo()->ToString() + " joined channel " + parser.GetParameters()[0] + " which we are not in";
                break;
            }
            // Insert this user to a channel
            User *user = channel_p->InsertUser(parser.GetSourceUserInfo());
            emit this->Event_Join(&parser, user, this->GetChannel(parser.GetParameters()[0]));
        }   break;
        case IRC_NUMERIC_PRIVMSG:
            known = true;
            emit this->Event_PRIVMSG(&parser);
            break;

        case IRC_NUMERIC_NOTICE:
            known = true;
            emit this->Event_NOTICE(&parser);
            break;

        case IRC_NUMERIC_NICK:
        {
            known = true;
            if (parser.GetParameters().count() < 1)
            {
                // wrong amount of parameters
                qDebug() << "IRC PARSER: Malformed NICK: " + parser.GetRaw();
                break;
            }
            // Precache the nicks to save hundreds of function calls
            QString old_nick = parser.GetSourceUserInfo()->GetNick();
            QString new_nick = parser.GetParameters()[0];
            if (self_command)
            {
                // our own nick was changed
                this->localUser.SetNick(new_nick);
                emit this->Event_SelfNICK(&parser, old_nick, new_nick);
            }
            // Change the nicks in every channel this user is in
            foreach (Channel *channel, this->channels)
                channel->ChangeNick(old_nick, new_nick);
            emit this->Event_NICK(&parser, old_nick, new_nick);
        }   break;
        case IRC_NUMERIC_QUIT:
        {
            known = true;
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
        case IRC_NUMERIC_PART:
        {
            known = true;
            if (parser.GetParameters().count() < 1)
            {
                qDebug() << "IRC PARSER: Invalid PART: " + parser.GetRaw();
                break;
            }
            Channel *channel = this->GetChannel(parser.GetParameters()[0]);
            if (channel)
                channel->RemoveUser(parser.GetSourceUserInfo()->GetNick());
            emit this->Event_Part(&parser, channel);
        }   break;
        case IRC_NUMERIC_KICK:
        {
            known = true;
            if (parser.GetParameters().count() < 2)
            {
                qDebug() << "IRC PARSER: Invalid KICK: " + parser.GetRaw();
                break;
            }
            Channel *channel = this->GetChannel(parser.GetParameters()[0]);
            if (channel)
                channel->RemoveUser(parser.GetParameters()[1]);
            emit this->Event_Kick(&parser, channel);
        }   break;
        case IRC_NUMERIC_PONG:
            known = true;
            break;
        case IRC_NUMERIC_MODE:
        {
            known = true;
            if (parser.GetParameters().count() < 1)
            {
                qDebug() << "IRC PARSER: Invalid MODE: " + parser.GetRaw();
                break;
            }
            QString entity = parser.GetParameters()[0];
            if (entity.toLower() == this->localUser.GetNick().toLower())
            {
                // Someone changed our own UMode
                this->localUserMode.SetMode(parser.GetText());
            } else if (entity.startsWith(this->channelPrefix))
            {
                // Someone changed a channel mode
            } else
            {
                // Someone changed UMode of another user, this is not supported on majority of servers, unless you are services
            }
            emit this->Event_Mode(&parser);
        }
            break;
    }
    if (!known)
        emit this->Event_Unknown(&parser);
    emit this->Event_Parse(&parser);
}

void Network::deleteTimers()
{
    if (this->timerPingSend)
    {
        this->timerPingSend->stop();
        delete this->timerPingSend;
        this->timerPingSend = NULL;
    }
    if (this->timerPingTimeout)
    {
        this->timerPingTimeout->stop();
        delete this->timerPingTimeout;
        this->timerPingTimeout = NULL;
    }
}
