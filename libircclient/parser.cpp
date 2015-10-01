//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "parser.h"

using namespace libircclient;

Parser::Parser(QString incoming_text)
{
    this->_valid = false;
    this->user = NULL;
    // the incoming text must be prefixed with colon, otherwise it's not from a server and we don't relay client messages
    if (!incoming_text.startsWith(":"))
    {
        // this is an exception though
        if (incoming_text.startsWith("PING :"))
        {
            this->_numeric = IRC_NUMERIC_PING_CHECK;
            this->parameterLine = incoming_text.mid(6);
            this->parameters = this->parameterLine.split(" ");
            this->_valid = true;
            return;
        }
        return;
    }
    QString temp = incoming_text.mid(1);
    // get the source
    if (!temp.contains(" "))
        return;
    this->source = temp.mid(0, temp.indexOf(" "));
    temp = temp.mid(temp.indexOf(" ") + 1);
    if (temp.contains(" :"))
    {
        this->parameterLine = temp.mid(temp.indexOf(" :") + 2);
        this->command = temp.mid(0, temp.indexOf(" "));
    }
    else if (temp.contains(" "))
    {
        this->parameterLine = temp.mid(temp.indexOf(" ") + 1);
        this->command = temp.mid(0, temp.indexOf(" "));
    }
    else
    {
        this->command = temp;
    }
    this->parameters = this->parameterLine.split(" ");
    this->_valid = true;
    this->obtainNumeric();
    if (this->source.contains("@"))
        this->user = new User(this->source);
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
        this->_numeric = IRC_NUMERIC_UNKNOWN;
    }
    else
    {
        this->_numeric = numeric_code;
        return;
    }

    // Convert text command to numeric
    if (this->command == "PING")
        this->_numeric = IRC_NUMERIC_PING_CHECK;
    else if (this->command == "JOIN")
        this->_numeric = IRC_NUMERIC_JOIN;
    else if (this->command == "PRIVMSG")
        this->_numeric = IRC_NUMERIC_PRIVMSG;
    else if (this->command == "KICK")
        this->_numeric = IRC_NUMERIC_KICK;
    else if (this->command == "PART")
        this->_numeric = IRC_NUMERIC_PART;
    else if (this->command == "QUIT")
        this->_numeric = IRC_NUMERIC_QUIT;
}

bool Parser::IsValid()
{
    return this->_valid;
}

QString Parser::GetParameterLine()
{
    return this->parameterLine;
}

QList<QString> Parser::GetParameters()
{
    return this->parameters;
}

User *Parser::GetSourceUserInfo()
{
    return this->user;
}

QString Parser::GetSourceInfo()
{
    return this->source;
}

