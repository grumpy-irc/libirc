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
#include "network.h"
#include "server.h"
#include "channel.h"
#include "user.h"
#include "../libirc/error_code.h"

using namespace libircclient;

Network::Network(QString Name, QString Hostname, QString Nickname, bool SSL, QString Password, unsigned int Port) : libirc::Network(Name)
{
    this->socket = NULL;
	this->hostname = Hostname;
	this->currentIdent = "grumpy";
	this->currentNick = "GrumpyUser";
	this->usingSSL = SSL;
    this->port = Port;
    this->network_thread = NULL;
}

Network::~Network()
{
    delete this->socket;
}

void Network::Connect()
{
    if (this->IsConnected())
        return;
}

void Network::Reconnect()
{
    this->Disconnect();
    this->Connect();
}

void Network::Disconnect()
{

}

bool Network::IsConnected()
{
    if (!this->socket)
        return false;

    return this->socket->isOpen();
}

void Network::TransferRaw(QString raw)
{
    if (!this->IsConnected())
        return;

    QByteArray data = QString(raw + "\n").toUtf8();
    emit this->RawOutgoing(data);
    this->socket->write(data);
    this->socket->flush();
}

int Network::SendMessage(QString text, Channel *channel)
{

    return SUCCESS;
}

int Network::SendMessage(QString text, User *user)
{

    return SUCCESS;
}

void Network::processIncomingRawData(QByteArray data)
{
    emit this->RawIncoming(data);
}

