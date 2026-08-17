//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "parser.h"
#include <QStringList>

using namespace libircclient;

static QStringList Trim(QStringList list)
{
    QStringList result;
    foreach (QString item, list)
        if (!item.isEmpty())
            result << item;
    return result;
}

static QString unescapeTagValue(const QString &value)
{
    QString result;
    bool escaping = false;
    foreach (QChar ch, value)
    {
        if (escaping)
        {
            if (ch == ':')
                result += ';';
            else if (ch == 's')
                result += ' ';
            else if (ch == 'r')
                result += '\r';
            else if (ch == 'n')
                result += '\n';
            else
                result += ch;
            escaping = false;
        }
        else if (ch == '\\')
        {
            escaping = true;
        }
        else
        {
            result += ch;
        }
    }
    if (escaping)
        result += '\\';
    return result;
}

static QDateTime parseServerTime(const QString &time_string)
{
    QDateTime timestamp = QDateTime::fromString(time_string, Qt::ISODateWithMs);
    if (!timestamp.isValid())
        timestamp = QDateTime::fromString(time_string, Qt::ISODate);
    if (!timestamp.isValid())
        timestamp = QDateTime::fromString(time_string, "yyyy-MM-ddTHH:mm:ss.zzzZ");
    return timestamp;
}

Parser::Parser(QString incoming_text)
{
    // remove all garbage from incoming text
    incoming_text.replace("\r", "");
    incoming_text.replace("\n", "");
    this->original_text = incoming_text;
    this->_valid = false;
    this->user = nullptr;
    if (incoming_text.startsWith("@"))
    {
        if (!incoming_text.contains(" "))
            return;
        QString tag_string = incoming_text.mid(1, incoming_text.indexOf(" ") - 1);
        incoming_text = incoming_text.mid(incoming_text.indexOf(" ") + 1);
        this->parseTags(tag_string);
        if (this->tags.contains("time"))
            this->timestamp = parseServerTime(this->tags["time"]);
    }
    // the incoming text must be prefixed with colon, otherwise it's not from a server and we don't relay client messages
    if (!incoming_text.startsWith(":"))
    {
        if (!this->timestamp.isValid())
            this->timestamp = QDateTime::currentDateTime();
        // this is an exception though
        if (incoming_text.startsWith("PING :"))
        {
            this->text = incoming_text;
            this->_numeric = IRC_NUMERIC_RAW_PING;
            this->parameterLine = incoming_text.mid(6);
            this->parameters = this->parameterLine.split(" ");
            this->_valid = true;
            return;
        }
        if (!incoming_text.startsWith(":"))
            return;
    } else
    {
        if (!this->timestamp.isValid())
            this->timestamp = QDateTime::currentDateTime();
    }
    this->text = incoming_text;
    QString temp = incoming_text.mid(1);
    // get the source
    if (!temp.contains(" "))
        return;
    this->source = temp.mid(0, temp.indexOf(" "));
    temp = temp.mid(temp.indexOf(" ") + 1);
    // first of all, extract the text if there is some
    if (temp.contains(" :"))
    {
        this->message_text = temp.mid(temp.indexOf(" :") + 2);
        temp = temp.mid(0, temp.indexOf(" :"));
    }
    // extract the command, it's a first standalone word
    if (temp.contains(" "))
    {
        this->command = temp.mid(0, temp.indexOf(" "));
        temp = temp.mid(temp.indexOf(" ") + 1);
    }
    else
    {
        this->command = temp;
        temp = "";
    }
    this->parameterLine = temp;
    this->parameters = Trim(this->parameterLine.split(" "));
    this->_valid = true;
    this->obtainNumeric();
    if (this->source.contains("@"))
        this->user = new User(this->source);
    else
        this->user = new User(this->source + "!@");
}

void Parser::parseTags(QString tag_string)
{
    foreach (QString tag, tag_string.split(";"))
    {
        if (tag.isEmpty())
            continue;
        QString key = tag;
        QString value;
        if (tag.contains("="))
        {
            key = tag.mid(0, tag.indexOf("="));
            value = unescapeTagValue(tag.mid(tag.indexOf("=") + 1));
        }
        this->tags.insert(key, value);
    }
}

Parser::~Parser()
{
    delete this->user;
}

int Parser::GetNumeric()
{
    return this->_numeric;
}

void Parser::obtainNumeric()
{
    int numeric_code = this->command.toInt();
    if (numeric_code == 0)
    {
        this->_numeric = IRC_NUMERIC_INVALID;
    }
    else
    {
        this->_numeric = numeric_code;
        return;
    }

    // Convert text command to numeric
    if (this->command == "PING")
        this->_numeric = IRC_NUMERIC_RAW_PING;
    else if (this->command == "JOIN")
        this->_numeric = IRC_NUMERIC_RAW_JOIN;
    else if (this->command == "NICK")
        this->_numeric = IRC_NUMERIC_RAW_NICK;
    else if (this->command == "PONG")
        this->_numeric = IRC_NUMERIC_RAW_PONG;
    else if (this->command == "NOTICE")
        this->_numeric = IRC_NUMERIC_RAW_NOTICE;
    else if (this->command == "MODE")
        this->_numeric = IRC_NUMERIC_RAW_MODE;
    else if (this->command == "PRIVMSG")
        this->_numeric = IRC_NUMERIC_RAW_PRIVMSG;
    else if (this->command == "KICK")
        this->_numeric = IRC_NUMERIC_RAW_KICK;
    else if (this->command == "TOPIC")
        this->_numeric = IRC_NUMERIC_RAW_TOPIC;
    else if (this->command == "PART")
        this->_numeric = IRC_NUMERIC_RAW_PART;
    else if (this->command == "CTCP")
        this->_numeric = IRC_NUMERIC_RAW_CTCP;
    else if (this->command == "QUIT")
        this->_numeric = IRC_NUMERIC_RAW_QUIT;
    else if (this->command == "AWAY")
        this->_numeric = IRC_NUMERIC_RAW_AWAY;
    else if (this->command == "CAP")
        this->_numeric = IRC_NUMERIC_RAW_CAP;
    else if (this->command == "METADATA")
        this->_numeric = IRC_NUMERIC_RAW_METADATA;
    else if (this->command == "INVITE")
        this->_numeric = IRC_NUMERIC_RAW_INVITE;
    else if (this->command == "CHGHOST")
        this->_numeric = IRC_NUMERIC_RAW_CHGHOST;
}

bool Parser::IsValid()
{
    return this->_valid;
}

QString Parser::GetParameterLine()
{
    return this->parameterLine;
}

QString Parser::GetRaw()
{
    return this->text;
}

QString Parser::GetOriginalRaw()
{
    return this->original_text;
}

QString Parser::GetText()
{
    return this->message_text;
}

QList<QString> Parser::GetParameters()
{
    return this->parameters;
}

QDateTime Parser::GetTimestamp()
{
    return this->timestamp;
}

QHash<QString, QString> Parser::GetTags()
{
    return this->tags;
}

QString Parser::GetTag(const QString &tag)
{
    return this->tags.value(tag);
}

User *Parser::GetSourceUserInfo()
{
    return this->user;
}

QString Parser::GetSourceInfo()
{
    return this->source;
}
